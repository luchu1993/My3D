//
// Created by luchu on 2022/1/8.
//

#pragma once


#include "My3D.h"
#include "Core/Object.h"


namespace My3D
{

/// Low-resolution operating system timer
class MY3D_API Timer
{
public:
    /// Construct
    Timer();
    /// Return elapsed milliseconds and optionally reset
    unsigned GetMSec(bool reset);
    /// Reset the timer
    void Reset();

private:
    /// Staring clock value in milliseconds
    unsigned startTime_{};
};

/// High-resolution operating system timer
class MY3D_API HiresTimer
{
    friend class Time;

public:
    /// Construct
    HiresTimer();
    /// Return elapsed microseconds and optional reset
    long long GetUSec(bool reset);
    /// Reset the timer
    void Reset();
    /// Return if high-resolution timer is supported
    static bool IsSupported() { return supported; }
    /// High-resolution timer frequency if supported
    static long long GetFrequency() { return frequency; }

private:
    /// Starting clock value in CPU ticks
    long long startTime_{};
    /// High-resolution timer support flag
    static bool supported;
    /// High-resolution timer frequency
    static long long frequency;
};

/// Time and frame counter subsystem
class MY3D_API Time : public Object
{
    MY3D_OBJECT(Time, Object)
public:
    /// Construct
    explicit Time(Context* context);
    /// Destruct
    ~Time() noexcept override;
    /// Begin new frame
    void BeginFrame(float timeStep);
    /// End frame
    void EndFrame();
    /// Set the low-resolution timer period in milliseconds. 0 resets to the default period
    void SetTimerPeriod(unsigned mSec);
    /// Return current frame timestep as seconds
    float GetTimeStep() const { return timeStep_; }
    /// Return frame number
    unsigned GetFrameNumber() const { return frameNumber_; }
    /// Return current low-resolution timer period in milliseconds
    unsigned GetTimerPeriod() const { return timerPeriod_; }
    /// Return elapsed time from program start as seconds
    float GetElapsedTime();
    /// Return current frames per second
    float GetFramesPerSecond() const;
    /// Get system time as milliseconds
    static unsigned GetSystemTime();
    /// Get system time as seconds since 1970.1.1
    static unsigned GetTimeSinceEpoch();
    /// Get a date/time stamp as a string
    static String GetTimeStamp();
    /// Sleep for a number of milliseconds
    static void Sleep(unsigned mSec);

private:
    /// Elapsed time since program start
    Timer elapsedTime_;
    /// Frame number
    unsigned frameNumber_;
    /// Timestep in seconds
    float timeStep_;
    /// Low-resolution timer period
    unsigned timerPeriod_;
};

}
