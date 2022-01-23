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

    /// initialize engine
    bool Initialize(const VariantMap& parameters);
    /// Run one frame
    void RunFrame();
    /// Set minimum frames per second.  If FPS goes lower than this, time will appear to slow down.
    void SetMinFps(unsigned fps) { minFps_ = Max(0u, fps); }
    /// Set maximum frames per second.
    void SetMaxFps(unsigned fps) { maxFps_ = Max(0u, fps); }
    /// Set maximum frames per second when the application does not have input focus.
    void SetMaxInactiveFps(unsigned fps) { maxInactiveFps_ = Max(0u, fps); }
    /// Set how many frames to average for timestep smoothing. Default is 2. 1 disables smoothing.
    void SetTimeStepSmoothing(unsigned frames) { timeStepSmoothing_ = Clamp(frames, 1u, 20u); }
    /// Set whether to exit automatically on exit request (window close button).
    void SetAutoExit(bool enable) { autoExit_ = enable; }
    /// Override timestep of the next frame. Should be called in between RunFrame() calls.
    void SetNextTimeStep(float seconds) { timeStep_ = Max(seconds, 0.0f); }
    /// Get timestep of the next frame
    float GetNextTimeStep() const { return timeStep_; }
    /// Return the maximum frames per second
    unsigned GetMinFps() const { return minFps_; }
    /// Return the minimum frames per second
    unsigned GetMaxFps() const { return maxFps_; }
    /// Return the maximum frames per second when the application does not have input focus.
    unsigned GetMaxInactiveFps() const { return maxInactiveFps_; }
    /// Return how many frames to average for timestep smoothing
    unsigned GetTimeStepSmoothing() const { return timeStepSmoothing_; }
    /// Return whether to exit automatically on exit request
    bool GetAutoExit() const { return autoExit_; }
    /// Return whether engine has been initialized
    bool IsInitialized() const { return initialized_; }
    /// Return whether exit has been requested
    bool IsExiting() const { return exiting_; }
    /// Send frame update events
    void Update();
    /// Render after frame update
    void Render();
    /// Get the timestep for the next frame and sleep for frame limiting if necessary
    void ApplyFrameLimit();
    /// Close the graphics window and set the exit flag.
    void Exit();

    /// Parse the engine startup parameters map from command line arguments
    static VariantMap ParseParameters(const Vector<String>& arguments);
    /// Return whether startup parameters contains a specific parameter
    static bool HasParameter(const VariantMap& parameters, const String& parameter);
    /// Get an engine startup parameter. which default value if missing
    static const Variant& GetParameter(const VariantMap& parameters, const String& parameter, const Variant& defaultValue = Variant::EMPTY);

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
    unsigned maxInactiveFps_;
    /// Pause when minimized flag
    bool pauseMinimized_;
    /// Auto-exit flag
    bool autoExit_;
    /// Initialized flag
    bool initialized_;
    /// Exiting flag
    bool exiting_;
    /// Headless mode flag.
    bool headless_;
};

}