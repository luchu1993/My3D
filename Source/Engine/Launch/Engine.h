//
// Created by luchu on 2022/1/1.
//

#pragma once

#include "My3D.h"
#include "Core/Timer.h"
#include "Core/Object.h"


namespace My3D
{

class MY3D_API Engine : public Object
{
    MY3D_OBJECT(Engine, Object)

public:
    explicit Engine(Context* context);
    ~Engine() override = default;

    // initialize engine
    bool Initialize();

    // Run one frame
    void RunFrame();

    float GetNextTimeStep() const { return timeStep_; }

    unsigned GetMinFps() const { return minFps_; }

    unsigned GetMaxFps() const { return maxFps_; }

    bool GetAutoExit() const { return autoExit_; }

    bool IsInitialized() const { return initialized_; }

    bool IsExiting() const { return exiting_; }

    void Update();

    void Render();

    void ApplyFrameLimit();

    void Exit();

private:
    /// Handle exit requested event. Auto-exited if enabled
    void HandleExitRequested(StringHash eventType, VariantMap& eventData);
    void DoExit();

    /// Frame update timer
    HiresTimer frameTimer_;
    /// Previous timesteps for smoothing
    PODVector<float> lastTimeSteps_;
    /// Next frame timestep in seconds
    float timeStep_;
    /// How many frames to average for the smoothed timestep
    unsigned timeStepSmoothing_;
    /// Maximum frames per second
    unsigned maxFps_;
    /// Minimum frames per second
    unsigned minFps_;
    /// Maximum frames per second when the application does not have input focus
    unsigned maxInteractiveFps_;
    /// Pause when minimized flag
    bool pauseMinimized_;
    /// Auto-exit flag
    bool autoExit_;
    /// Initialized flag
    bool initialized_;
    /// Exiting flag
    bool exiting_;
};

}