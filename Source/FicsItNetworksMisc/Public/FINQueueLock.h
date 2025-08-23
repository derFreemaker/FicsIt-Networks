#pragma once

#include "CoreMinimal.h"
#include "CoreMiscDefines.h"
#include "Event.h"
#include "Queue.h"

class FICSITNETWORKSMISC_API FFINQueueLock {
	FCriticalSection Mutex;
	bool IsLocked = false;
	
	TQueue<uint32> WaitingQueue;
	TMap<uint32, FCriticalSection*> Map;

public:
	FFINQueueLock() = default;

	UE_NONCOPYABLE(FFINQueueLock)

	void Lock();
	bool TryLock();

	void Unlock();
};

class FICSITNETWORKSMISC_API FFINQueueScopeLock {
	FFINQueueLock &Lock;

public:
	explicit FFINQueueScopeLock(FFINQueueLock &Lock) : Lock(Lock) {
		Lock.Lock();
	}

	~FFINQueueScopeLock() {
		Lock.Unlock();
	}

	UE_NONCOPYABLE(FFINQueueScopeLock)
};
