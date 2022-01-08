//
// Created by luchu on 2022/1/8.
//

#include "Timer.h"

#ifdef PLATFORM_MSVC
#include <windows.h>
#include <mmsystem.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include <ctime>

namespace My3D
{
static unsigned Tick()
{
#ifdef PLATFORM_MSVC
    return (unsigned) timeGetTime();
#else
    struct timeval time;
    gettimeofday(&time, nullptr);
    return (unsigned)(time.tv_sec * 1000 + time.tv_usec / 1000);
#endif
}

Time::Time(Context *context)
    : Object(context)
{

}

Time::~Time() noexcept = default;

unsigned Time::GetSystemTime()
{
    return Tick();
}

String Time::GetTimeStamp()
{
    std::time_t sysTime = std::time(nullptr);

    char dateTime[CONVERSION_BUFFER_LENGTH];
    strftime(dateTime, sizeof(dateTime), "%F-%T", std::localtime(&sysTime));
    return String(dateTime);
}

void Time::Sleep(unsigned int mSec)
{
#ifdef PLATFORM_MSVC
    ::Sleep(mSec);
#else
    timespec time{static_cast<time_t>(mSec / 1000), static_cast<long>((mSec % 1000) * 1000000)};
    nanosleep(&time, nullptr);
#endif
}

}
