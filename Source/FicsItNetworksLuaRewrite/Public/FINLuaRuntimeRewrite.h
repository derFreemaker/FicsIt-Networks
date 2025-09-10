#pragma once

#include "CoreMinimal.h"
#include "CoreMiscDefines.h"
#include "Runnable.h"
#include "RunnableThread.h"
#include "SingleThreadRunnable.h"
#include "Memory/MemoryArena.h"

#include "FINLuaUtilRewrite.h"
#include "FINQueueLock.h"

struct FICSITNETWORKSLUAREWRITE_API FFINLuaPanic {
	FString Message;
};

#define FIN_LUA_SIMULATE_TICK 0

class UFINKernelSystem;
struct FFINLuaRuntimeRewrite;

struct FICSITNETWORKSLUAREWRITE_API FFINLuaTerminate {};

struct FICSITNETWORKSLUAREWRITE_API FFINLuaThread : public FRunnable, public FSingleThreadRunnable {
	enum class EState {
		Uninitialized,
		Waiting,
		Running,
		Stopped,
		TickStop,
		Suspended,
		Finished,
		Crashed,
	};

	friend void TickHookF(lua_State *L, lua_Debug *Ar);
	friend struct FFINLuaRuntimeRewrite;
	
	static FFINLuaThread& GetFromLuaState(lua_State *L) {
		return **static_cast<FFINLuaThread**>(lua_getextraspace(L));
	}

private:
	FFINLuaRuntimeRewrite *const Runtime;
	FMemoryArena *Arena;

	lua_State *L = nullptr;
	lua_State *LuaThread = nullptr;
	EState State = EState::Uninitialized;
	bool StopRequested = false;

	FRWLock ThreadMutex;

	FFINQueueLock SyncMutex;

	TOptional<FString> ErrorMessage;
	TOptional<FString>& Code;
public:
	FFINLuaThread(FFINLuaRuntimeRewrite *const Runtime, FMemoryArena *Arena, TOptional<FString>& Code) : Runtime(Runtime), Arena(Arena), Code(Code) {
		// we unlock in each tick so that when we do an action which needs to be sync with the factory tick we can align the thread
		SyncMutex.Lock();
	}

	UE_NONCOPYABLE(FFINLuaThread)

private:
	int ResumeLua();
	bool HandleIfStopRequested();

	// FRunnable
	friend class FRunnable;

	virtual bool Init() override;

	virtual uint32 Run() override;

	virtual void Stop() override;

	virtual void Exit() override;

	virtual FSingleThreadRunnable* GetSingleThreadInterface() override {
		return this;
	}

	// FRunnable

	// FSingleThreadRunnable
	friend class FSingleThreadRunnable;

	virtual void Tick() override;

	// FSingleThreadRunnable

public:
	FFINLuaRuntimeRewrite* GetRuntime() const;

	EState GetStatus();
	const TOptional<FString>& GetErrorMessage();

	void FactoryTick();
};

class UFINLuaProcessorRewrite;

struct FICSITNETWORKSLUAREWRITE_API FFINLuaRuntimeRewrite {
	static FFINLuaRuntimeRewrite& GetFromLuaState(lua_State *L) {
		return *FFINLuaThread::GetFromLuaState(L).GetRuntime();
	}

private:
	//TODO: implement with real Arena allocator
	FMallocArena Arena;

	FFINLuaThread Thread;
	FRunnableThread *RunnableThread = nullptr;

	UFINLuaProcessorRewrite &Processor;

	TOptional<FString> Code;
public:
	FFINLuaRuntimeRewrite(UFINLuaProcessorRewrite &Processor) : Thread(this, &Arena, Code), Processor(Processor) {}
	~FFINLuaRuntimeRewrite();

private:
	UE_NONCOPYABLE(FFINLuaRuntimeRewrite)

public:
	UFINKernelSystem& GetKernel() const;

	FFINLuaThread::EState GetStatus();

	void SetCode(const TOptional<FString> &NewCode);

	bool Start();
	void Stop();

	void Tick();

	TOptional<FString> GetError();
};
