#include "FINLuaRuntimeRewrite.h"

#include <string>

#include "FicsItLogLibrary.h"
#include "FILLogContainer.h"
#include "FINLuaProcessorRewrite.h"

void TickHookF(lua_State *L, lua_Debug *Ar) {
	auto &Thread = FFINLuaThread::GetFromLuaState(L);

	{
		FScopeLock Lock(&Thread.ThreadMutex);
		if (Thread.State != FFINLuaThread::EState::Running) {
			// UE_LOG(LogFicsItNetworksLua, Display, TEXT("Lua Thread: tick canceled"));
			return;
		}

		Thread.State = FFINLuaThread::EState::TickStop;
	}

	// UE_LOG(LogFicsItNetworksLua, Display, TEXT("Lua Thread: tick..."));
	lua_yieldafterinstruction(L, true);
}

bool FFINLuaThread::HandleIfStopRequested() {
	FScopeLock Lock(&ThreadMutex);
	if (!StopRequested) {
		return false;
	}

	State = EState::Stopped;
	lua_yieldafterinstruction(L, false);
	// UE_LOG(LogFicsItNetworksLua, Display, TEXT("Lua Thread: stopped"));
	return true;
}

int FFINLuaThread::ResumeLua() {
	int Status;
	int NResults = 0;

#if FIN_LUA_SIMULATE_TICK
	while (true) {
#endif

		if (HandleIfStopRequested()) {
			return 0;
		}

		{
			FScopeLock Lock(&ThreadMutex);
			State = EState::Running;
		}

		try {
			// UE_LOG(LogFicsItNetworksLua, Display, TEXT("Lua Thread: resume..."));
			Status = lua_resume(LuaThread, L, 0, &NResults);
			// UE_LOG(LogFicsItNetworksLua, Display, TEXT("Lua Thread: resume finished"));
		} catch (FFINLuaPanic Panic) {
			FScopeLock Lock(&ThreadMutex);
			State = EState::Crashed;
			ErrorMessage = Panic.Message;
			return 0;
		}

		// UE_LOG(LogFicsItNetworksLua, Display, TEXT("Lua Thread: status %d"), Status);

		if (HandleIfStopRequested()) {
			return Status;
		}

#if FIN_LUA_SIMULATE_TICK
		if (Status == LUA_YIELD && GetStatus() == EState::TickStop) {
			lua_yieldafterinstruction(L, false);
			UE_LOG(LogFicsItNetworksLua, Display, TEXT("Lua Thread: Tick every 100'000'000 instructions"));
			continue;
		}

		break;
	}
#endif

	{
		FScopeLock Lock(&ThreadMutex);
		switch (Status) {
			case LUA_OK: {
				State = EState::Finished;
				lua_pop(LuaThread, NResults);
				break;
			}

			case LUA_YIELD: {
				if (State == EState::TickStop) {
					State = EState::Suspended;
					// UE_LOG(LogFicsItNetworksLua, Display, TEXT("Lua Thread: suspended"));
				} else {
					lua_pop(L, NResults);
					State = EState::Crashed;
					luaL_traceback(L, LuaThread, "illegal lua yield", 0);
					ErrorMessage = FINLua::luaFIN_toFString(L, -1);
					lua_pop(L, 1);
					break;
				}
				break;
			}

			case LUA_ERRRUN:
			case LUA_ERRMEM:
			case LUA_ERRERR:
			case LUA_ERRSYNTAX: {
				State = EState::Crashed;
				const char *Message = lua_tostring(LuaThread, -1);
				luaL_traceback(L, LuaThread, Message, 0);
				ErrorMessage = FINLua::luaFIN_toFString(L, -1);
				lua_pop(L, 2);
				break;
			}

			default: {
				checkf(false, TEXT("unreachable"))
				break;
			}
		}
	}

	return Status;
}

FFINLuaRuntimeRewrite* FFINLuaThread::GetRuntime() const {
	return Runtime;
}

FFINLuaThread::EState FFINLuaThread::GetStatus() {
	FScopeLock Lock(&ThreadMutex);
	return State;
}

const TOptional<FString>& FFINLuaThread::GetErrorMessage() {
	FScopeLock Lock(&ThreadMutex);
	return ErrorMessage;
}

void FFINLuaThread::FactoryTick() {
	SyncMutex.Unlock();

	// we allow for all queue threads to execute their sync actions
	SyncMutex.Lock();
}

void LuaWarnF(void *Ud, const char *Msg, int ToCont) {
	UFILogLibrary::Log(FIL_Verbosity_Warning, UTF8_TO_TCHAR(Msg));
}

int LuaPanicF(lua_State *L) {
	const FString Message = FINLua::luaFIN_toFString(L, -1);
	UE_LOG(LogFicsItNetworksLuaRewrite, Warning, TEXT("Lua Thread: panic with message: %s"), *Message);
	throw FFINLuaPanic(Message);
}

static void* LuaArenaAllocFunction(void *Ud, void *Ptr, SIZE_T OldSize, SIZE_T NewSize) {
	const auto Arena = static_cast<FMemoryArena*>(Ud);

	if (NewSize == 0) {
		if (Ptr) {
			Arena->Free(Ptr);
		}
		return nullptr;
	}

	// we don't have any alignment information from lua
	constexpr SIZE_T Alignment = 0;

	if (Ptr == nullptr) {
		return Arena->Alloc(NewSize, Alignment);
	}

	return ArenaRealloc(Arena, Ptr, OldSize, NewSize, Alignment).GetPointer();
}

//TODO: temporary
int LuaPrint(lua_State *L) {
	const int args = lua_gettop(L);
	std::string log;
	for (int i = 1; i <= args; ++i) {
		size_t s_len = 0;
		const char *s = luaL_tolstring(L, i, &s_len);
		if (!s)
			luaL_argerror(L, i, "is not valid type");
		log += std::string(s, s_len) + " ";
	}
	if (log.length() > 0)
		log = log.erase(log.length() - 1);

	const auto &Runtime = FFINLuaRuntimeRewrite::GetFromLuaState(L);
	Runtime.GetKernel().GetLog()->PushLogEntry(FIL_Verbosity_Info, UTF8_TO_TCHAR(log.c_str()));

	return 0;
}

static int LuaResume(lua_State *L);

static int LuaResumeResume(lua_State *L, int, lua_KContext) {
	return LuaResume(L);
}

static int LuaResume(lua_State *L) {
	const int r = luaB_coresume(L);

	if (FFINLuaThread::GetFromLuaState(L).GetStatus() != FFINLuaThread::EState::Running) {
		return lua_yieldk(L, 0, NULL, &LuaResumeResume);
	}
	
	return r;
}

bool FFINLuaThread::Init() {
	FScopeLock Lock(&ThreadMutex);

	StopRequested = false;
	ErrorMessage.Reset();

	L = lua_newstate(LuaArenaAllocFunction, Arena);
	if (!L) {
		return false;
	}
	*static_cast<FFINLuaThread**>(lua_getextraspace(L)) = this;

	luaL_openlibs(L);

	lua_pushcfunction(L, LuaPrint);
	lua_setglobal(L, "print");

	lua_getglobal(L, "coroutine");
	lua_pushcfunction(L, LuaResume);
	lua_setfield(L, -2, "resume");
	lua_pop(L, 1);

	lua_setwarnf(L, LuaWarnF, this);
	lua_atpanic(L, LuaPanicF);

	LuaThread = lua_newthread(L);

	if (!Code) {
		return false;
	}
	const FTCHARToUTF8 CodeConv(**Code, Code->Len());
	const auto LoadStatus = luaL_loadbufferx(LuaThread, CodeConv.Get(), CodeConv.Length(), "=EEPROM", "t");
	if (LoadStatus != LUA_OK) {
		check(lua_isstring(LuaThread, -1));
		ErrorMessage = FINLua::luaFIN_toFString(LuaThread, -1);
		return false;
	}

#if FIN_LUA_SIMULATE_TICK
	lua_sethook(LuaThread, TickHookF, LUA_MASKCOUNT, 100'000'000);
#endif

	State = EState::Waiting;
	return true;
}

uint32 FFINLuaThread::Run() {
	return ResumeLua();
}

void FFINLuaThread::Stop() {
	FScopeLock Lock(&ThreadMutex);

	StopRequested = true;

	if (State != EState::Uninitialized &&
		State != EState::Finished &&
		State != EState::Stopped &&
		State != EState::Crashed) {
		lua_yieldafterinstruction(L, true);
	}
	// UE_LOG(LogFicsItNetworksLua, Display, TEXT("Lua Thread: requested stop"));
}

void FFINLuaThread::Exit() {
	FScopeLock Lock(&ThreadMutex);

	lua_closethread(LuaThread, L);
	LuaThread = nullptr;

	lua_close(L);
	L = nullptr;
}

void FFINLuaThread::Tick() {
	lua_sethook(LuaThread, TickHookF, LUA_MASKCOUNT, 2500);
	ResumeLua();
}

FFINLuaRuntimeRewrite::~FFINLuaRuntimeRewrite() {
	Stop();
}

UFINKernelSystem& FFINLuaRuntimeRewrite::GetKernel() const {
	return *Processor.GetKernel();
}

FFINLuaThread::EState FFINLuaRuntimeRewrite::GetStatus() {
	return Thread.GetStatus();
}

void FFINLuaRuntimeRewrite::SetCode(const TOptional<FString> &NewCode) {
	FScopeLock Lock(&Thread.ThreadMutex);
	Code = NewCode;
}

bool FFINLuaRuntimeRewrite::Start() {
	// UE_LOG(LogFicsItNetworksLua, Display, TEXT("Lua Runtime: trying to start..."));

	RunnableThread = FRunnableThread::Create(&Thread, TEXT("LuaThread"));
	if (!RunnableThread) {
		return false;
	}

	// UE_LOG(LogFicsItNetworksLua, Display, TEXT("Lua Runtime: started"));
	return true;
}

void FFINLuaRuntimeRewrite::Stop() {
	if (RunnableThread) {
		// UE_LOG(LogFicsItNetworksLua, Display, TEXT("Lua Runtime: waiting for thread stop"));
		RunnableThread->Kill(true);
		RunnableThread = nullptr;
		// UE_LOG(LogFicsItNetworksLua, Display, TEXT("Lua Runtime: thread stopped"));
	}
}

void FFINLuaRuntimeRewrite::Tick() {
	const auto Status = GetStatus();
	if (Status == FFINLuaThread::EState::Running || Status == FFINLuaThread::EState::TickStop) {
		return;
	}

	Thread.FactoryTick();
}

TOptional<FString> FFINLuaRuntimeRewrite::GetError() {
	return Thread.GetErrorMessage();
}
