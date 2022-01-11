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
    /// Return current frame timestep as seconds
    float GetTimeStep() const { return timeStep_; }
    /// Return frame number
    unsigned GetFrameNumber() const { return frameNumber_; }
    /// Return elapsed time from program start as seconds
    float GetElapsedTime();
    /// Return current frames per second
    float GetFramesPerSecond() const;
    /// Get system time as milliseconds
    static unsigned GetSystemTime();
    /// Get system time as seconds since 1970.1.1
    /// Get a date/time stamp as a string
    static String GetTimeStamp();
    /// Sleep for a number of milliseconds
    static void Sleep(unsigned mSec);

private:
    /// Frame number
    unsigned frameNumber_;
    /// Timestep in seconds
    float timeStep_;
};

}
