//
// Created by luchu on 2022/1/8.
//

#include "Timer.h"
#include "Core/CoreEvents.h"

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

bool HiresTimer::supported(false);
long long HiresTimer::frequency(1000);

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

static long long HiresTick()
{
#ifdef PLATFORM_MSVC
    if (HiresTimer::IsSupported())
    {
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        return counter.QuadPart;
    }
    else
        return timeGetTime();
#else
    struct timeval time{};
    gettimeofday(&time, nullptr);
    return time.tv_sec * 1000000LL + time.tv_usec;
#endif
}

Timer::Timer() { Reset(); }

unsigned Timer::GetMSec(bool reset)
{
    unsigned currentTime = Tick();
    unsigned elapsedTime = currentTime - startTime_;

    if (reset)
        startTime_ = currentTime;

    return elapsedTime;
}

void Timer::Reset()
{
    startTime_ = Tick();
}

Time::Time(Context *context)
    : Base(context)
    , frameNumber_(0)
    , timeStep_(0.0f)
    , timerPeriod_(0)
{
#ifdef PLATFORM_MSVC
    LARGE_INTEGER frequency;
    if (QueryPerformanceFrequency(&frequency))
    {
        HiresTimer::frequency = frequency.QuadPart;
        HiresTimer::supported = true;
    }
#else
    HiresTimer::frequency = 1000000;
    HiresTimer::supported = true;
#endif
}

Time::~Time() noexcept = default;

unsigned Time::GetSystemTime()
{
    return Tick();
}

unsigned Time::GetTimeSinceEpoch()
{
    return (unsigned) time(nullptr);
}

String Time::GetTimeStamp()
{
    std::time_t sysTime = std::time(nullptr);

    char dateTime[CONVERSION_BUFFER_LENGTH];
    strftime(dateTime, sizeof(dateTime), "%F %T", std::localtime(&sysTime));
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

void Time::BeginFrame(float timeStep)
{
    ++frameNumber_;
    if (!frameNumber_)
        ++frameNumber_;

    timeStep_ = timeStep;

    // Frame begin event
    using namespace BeginFrame;
    VariantMap& eventData = GetEventDataMap();
    eventData[P_FRAMENUMBER] = frameNumber_;
    eventData[P_TIMESTEP] = timeStep_;

    SendEvent(E_BEGINFRAME, eventData);
}

void Time::EndFrame()
{
    SendEvent(E_ENDFRAME);
}

void Time::SetTimerPeriod(unsigned int mSec)
{
#ifdef PLATFORM_MSVC
    if (timerPeriod_ > 0)
        timeEndPeriod(timerPeriod_);

    timerPeriod_ = mSec;

    if (timerPeriod_ > 0)
        timeBeginPeriod(timerPeriod_);
#endif
}

float Time::GetFramesPerSecond() const
{
    return 1.0f / timeStep_;
}

float Time::GetElapsedTime()
{
    return elapsedTime_.GetMSec(false) / 1000.0f;
}

HiresTimer::HiresTimer()
{
    Reset();
}

long long HiresTimer::GetUSec(bool reset)
{
    long long currentTime = HiresTick();
    long long elapsedTime = currentTime - startTime_;

    if (elapsedTime < 0)
        elapsedTime = 0;

    if (reset)
        startTime_ = currentTime;

    return (elapsedTime * 1000000LL) / frequency;
}

void HiresTimer::Reset()
{
    startTime_ = HiresTick();
}

}
