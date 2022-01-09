//
// Created by luchu on 2022/1/2.
//

#include "Core/Thread.h"

#ifdef PLATFORM_MSVC
#include <windows.h>
#else
#include <pthread.h>
#endif


namespace My3D
{

#ifdef PLATFORM_MSVC
static DWORD WINAPI ThreadFunctionStatic(void* data)
{
	Thread* thread = static_cast<Thread*>(data);
	thread->ThreadFunction();
	return 0;
}

#else
static void* ThreadFunctionStatic(void* data)
{
	auto* thread = static_cast<Thread*>(data);
	thread->ThreadFunction();
	pthread_exit((void*)nullptr);
	return nullptr;
}
#endif

ThreadID Thread::mainThreadID;

Thread::Thread()
: handle_(nullptr)
, shouldRun_(false)
{
}

Thread::~Thread()
{
	Stop();
}

bool Thread::Run()
{
	if (handle_)
		return false;
	shouldRun_ = true;
#ifdef PLATFORM_MSVC
	handle_ = CreateThread(nullptr, 0, ThreadFunctionStatic, this, 0, nullptr);
#else
	handle_ = new pthread_t;
	pthread_attr_t type;
	pthread_attr_init(&type);
	pthread_attr_setdetachstate(&type, PTHREAD_CREATE_JOINABLE);
	pthread_create((pthread_t*)handle_, &type, ThreadFunctionStatic, this);
#endif
	return handle_ != nullptr;
}

void Thread::Stop()
{
	if (!handle_)
		return;

	shouldRun_ = false;
#ifdef PLATFORM_MSVC
	WaitForSingleObject((HANDLE)handle_, INFINITE);
	CloseHandle((HANDLE)handle_);
#else
	auto* thread = (pthread_t*)handle_;
	if (thread)
		pthread_join(*thread, nullptr);
	delete thread;
#endif
	handle_ = nullptr;
}

void Thread::SetPriority(int priority)
{
#ifdef PLATFORM_MSVC
	if (handle_)
		SetThreadPriority((HANDLE)handle_, priority);
#else
	auto* thread = (pthread_t*)handle_;
	if (thread)
		pthread_setschedprio(*thread, priority);
#endif
}

void Thread::SetMainThread()
{
	mainThreadID = GetCurrentThreadID();
}

ThreadID Thread::GetCurrentThreadID()
{
#ifdef PLATFORM_MSVC
	return GetCurrentThreadId();
#else
	return pthread_self();
#endif
}

bool Thread::IsMainThread()
{
	return GetCurrentThreadID() == mainThreadID;
}

}