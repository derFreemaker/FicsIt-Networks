#pragma once

#include "FINQueueLock.h"

#include "FicsItNetworksMisc.h"

void FFINQueueLock::Lock() {
	const auto CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
	FCriticalSection ThreadMutex;

	{
		FScopeLock Lock(&Mutex);

		if (!IsLocked && WaitingQueue.IsEmpty()) {
			IsLocked = true;
			return;
		}

		WaitingQueue.Enqueue(CurrentThreadId);

		// ThreadMutex = static_cast<FCriticalSection*>(FMemory::Malloc(sizeof(FCriticalSection), alignof(FCriticalSection)));
		// lock mutex so we can wait for it later
		ThreadMutex.Lock();
		Map.Add(CurrentThreadId, &ThreadMutex);
	}

	// wait for current locking Thread to unlock
	ThreadMutex.Lock();
	// FMemory::Free(ThreadMutex);
}

bool FFINQueueLock::TryLock() {
	{
		FScopeLock Lock(&Mutex);
		if (IsLocked) {
			return false;
		}
	}

	Lock();
	return true;
}

void FFINQueueLock::Unlock() {
	FScopeLock Lock(&Mutex);

	uint32 NextThreadId = 0;
	if (!WaitingQueue.Dequeue(NextThreadId)) {
		IsLocked = false;
		return;
	}

	// unlock mutex different thread is waiting on
	Map.FindChecked(NextThreadId)->Unlock();
	Map.Remove(NextThreadId);
}
