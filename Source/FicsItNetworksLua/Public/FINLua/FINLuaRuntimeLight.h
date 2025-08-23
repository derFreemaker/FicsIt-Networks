#pragma once

#include "CoreMinimal.h"
#include "CoreMiscDefines.h"
#include "Runnable.h"
#include "RunnableThread.h"
#include "SingleThreadRunnable.h"
#include "Memory/MemoryArena.h"

#include "FINQueueLock.h"
#include "LuaUtil.h"

#define FIN_LUA_SIMULATE_TICK 0

class UFINKernelSystem;
struct FFINLuaRuntimeLight;

struct FICSITNETWORKSLUA_API FFINLuaTerminate {};

struct FICSITNETWORKSLUA_API FFINLuaThread : public FRunnable, public FSingleThreadRunnable {
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
	friend struct FFINLuaRuntimeLight;
	
	static FFINLuaThread& GetFromLuaState(lua_State *L) {
		return **static_cast<FFINLuaThread**>(lua_getextraspace(L));
	}

private:
	FFINLuaRuntimeLight *const Runtime;
	FMemoryArena *Arena;

	lua_State *L = nullptr;
	lua_State *LuaThread = nullptr;
	EState State = EState::Uninitialized;
	bool StopRequested = false;

	FCriticalSection ThreadMutex;

	FFINQueueLock SyncMutex;

	TOptional<FString> ErrorMessage;
	TOptional<FString>& Code;
public:
	FFINLuaThread(FFINLuaRuntimeLight *const Runtime, FMemoryArena *Arena, TOptional<FString>& Code) : Runtime(Runtime), Arena(Arena), Code(Code) {
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
	FFINLuaRuntimeLight* GetRuntime() const;

	EState GetStatus();
	const TOptional<FString>& GetErrorMessage();

	void FactoryTick();
};

class UFINLuaProcessorLight;

struct FICSITNETWORKSLUA_API FFINLuaRuntimeLight {
	static FFINLuaRuntimeLight& GetFromLuaState(lua_State *L) {
		return *FFINLuaThread::GetFromLuaState(L).GetRuntime();
	}

private:
	//TODO: implement with real Arena allocator
	FMallocArena Arena;

	FFINLuaThread Thread;
	FRunnableThread *RunnableThread = nullptr;

	UFINLuaProcessorLight &Processor;

	TOptional<FString> Code;
public:
	FFINLuaRuntimeLight(UFINLuaProcessorLight &Processor) : Thread(this, &Arena, Code), Processor(Processor) {}
	~FFINLuaRuntimeLight();

private:
	UE_NONCOPYABLE(FFINLuaRuntimeLight)

public:
	UFINKernelSystem& GetKernel() const;

	FFINLuaThread::EState GetStatus();

	void SetCode(const TOptional<FString> &NewCode);

	bool Start();
	void Stop();

	void Tick();

	TOptional<FString> GetError();
};
