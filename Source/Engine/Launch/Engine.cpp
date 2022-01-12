//
// Created by luchu on 2022/1/1.
//

#include "Launch/Engine.h"
#include "Core/Context.h"
#include "IO/Log.h"
#include "Core/StringUtils.h"
#include "Core/Timer.h"
#include "Input/InputEvents.h"
#include "Core/CoreEvents.h"
#include "Launch/EngineDefs.h"

#include <cassert>


namespace My3D
{
extern const char* logLevelPrefixes[];

Engine::Engine(Context *context)
    : Base(context)
    , timeStep_(0.0f)
    , timeStepSmoothing_(2)
    , minFps_(10)
    , maxFps_(200)
    , maxInactiveFps_(60)
    , pauseMinimized_(false)
    , autoExit_(true)
    , initialized_(false)
    , exiting_(false)
{
    // Register self as a subsystem
    context_->RegisterSubsystem(this);
#ifdef MY3D_LOGGING
    context_->RegisterSubsystem<Log>();
#endif
    context->RegisterSubsystem<Time>();

    SubscribeToEvent(E_EXITREQUESTED, MY3D_HANDLER(Engine, HandleExitRequested));
}

bool Engine::Initialize(const VariantMap& parameters)
{
    if (initialized_)
        return true;

    GetSubSystem<Time>()->SetTimerPeriod(1);
    frameTimer_.Reset();

    MY3D_LOGINFO("Initialized engine");
    initialized_ = true;
    return true;
}

void Engine::RunFrame()
{
    assert(initialized_);

    if (exiting_)
        return;

    auto* time = GetSubSystem<Time>();
    time->BeginFrame(timeStep_);

    Update();

    Render();
    ApplyFrameLimit();

    time->EndFrame();
}

void Engine::Render()
{

}

void Engine::ApplyFrameLimit()
{
    if (!initialized_)
        return;

    unsigned maxFps = maxFps_;

    long long elapsed = 0;
    if (maxFps)
    {
        long long targetMax = 1000000LL / maxFps;
        for (;;)
        {
            elapsed = frameTimer_.GetUSec(false);
            if (elapsed >= targetMax)
                break;
            // Sleep if 1 ms or more off the frame limiting goal
            if (targetMax - elapsed >= 1000LL)
            {
                auto sleepTime = static_cast<unsigned>((targetMax - elapsed) / 1000LL);
                Time::Sleep(sleepTime);
            }
        }
    }

    elapsed = frameTimer_.GetUSec(true);

    if (minFps_)
    {
        long long targetMin = 1000000LL / minFps_;
        if (elapsed > targetMin)
            elapsed = targetMin;
    }

    /// Perform timestep smoothing
    timeStep_ = 0.0f;
    lastTimeSteps_.Push(elapsed / 1000000.0f);
    if (lastTimeSteps_.Size() > timeStepSmoothing_)
    {
        lastTimeSteps_.Erase(0, lastTimeSteps_.Size() - timeStepSmoothing_);
        for (unsigned  i = 0; i < lastTimeSteps_.Size(); ++i)
            timeStep_ += lastTimeSteps_[i];
        timeStep_ /= lastTimeSteps_.Size();
    }
    else
        timeStep_ = lastTimeSteps_.Back();
}

void Engine::Exit()
{
    DoExit();
}

void Engine::Update()
{
    using namespace Update;

    // Logic update event
    VariantMap& eventData = GetEventDataMap();
    eventData[P_TIMESTEP] = timeStep_;
    SendEvent(E_UPDATE, eventData);

    // Logic post-update event
    SendEvent(E_POSTUPDATE, eventData);

    // Rendering update event
    SendEvent(E_RENDERUPDATE, eventData);

    // Post-render update event
    SendEvent(E_POSTRENDERUPDATE, eventData);
}

void Engine::HandleExitRequested(StringHash eventType, VariantMap &eventData)
{
    DoExit();
}

void Engine::DoExit()
{
    exiting_ = true;
}

    VariantMap Engine::ParseParameters(const Vector<String> &arguments)
    {
        VariantMap ret;

        for (unsigned i = 0; i < arguments.Size(); ++i)
        {
            if (arguments[i].Length() > 1 && arguments[i][0] == '-')
            {
                String argument = arguments[i].Substring(1).ToLower();
                String value = i + 1 < arguments.Size() ? arguments[i + 1] : String::EMPTY;

                if (argument == "log" && !value.Empty())
                {
                    unsigned logLevel = GetStringListIndex(value.CString(), logLevelPrefixes, M_MAX_UNSIGNED);
                    if (logLevel != M_MAX_UNSIGNED)
                    {
                        ret[EP_LOG_LEVEL] = logLevel;
                        ++i;
                    }
                }
            }
        }

        return ret;
    }

    bool Engine::HasParameter(const VariantMap &parameters, const String &parameter)
    {
        StringHash nameHash(parameter);
        return parameters.Find(nameHash) != parameters.End();
    }

    const Variant &
    Engine::GetParameter(const VariantMap &parameters, const String &parameter, const Variant &defaultValue)
    {
        StringHash nameHash(parameter);
        auto it = parameters.Find(nameHash);
        return it != parameters.End() ? it->second_ : defaultValue;
    }

}
