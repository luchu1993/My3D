//
// Created by luchu on 2022/1/2.
//

#pragma once

#include "My3D.h"


#ifdef PLATFORM_MSVC
using ThreadID = unsigned;
#else
#include <pthread.h>
using ThreadID = pthread_t;
#endif


namespace My3D
{
    /// Operating system thead
	class MY3D_API Thread
    {
	public:
        /// Construct. Does not start the thread yet.
        Thread();
        /// Destruct. If running, stop and wait for thread to finish.
        virtual ~Thread();
        // Set the current thread as the main thread
        static void SetMainThread();
        // Function to run in the thread
        virtual void ThreadFunction() = 0;
        // Start running the thread.
        bool Run();
        // Set the running flag to false and wait for the thread to finish.
        void Stop();
        // Set thread priority
        void SetPriority(int priority);
        // Return whether thread exists
        bool IsStarted() const { return handle_ != nullptr; }
        // Return the current thread's ID
        static ThreadID GetCurrentThreadID();
        // Return whether is executing int main thread
        static bool IsMainThread();

	protected:
        // Thread handle
        void* handle_;
        // Running flag
        volatile bool shouldRun_;
        // Main thread's thread ID
        static ThreadID mainThreadID;
    };
}
