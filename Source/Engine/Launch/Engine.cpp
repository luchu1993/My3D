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
#include "Graphics/Graphics.h"
#include "IO/FileSystem.h"
#include "Input/Input.h"
#include "Core/ProcessUtils.h"
#include "Core/WorkQueue.h"
#include "Resource/ResourceCache.h"


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
    context_->RegisterSubsystem<Time>();
    context_->RegisterSubsystem<WorkQueue>();
    context_->RegisterSubsystem<Input>();
    context_->RegisterSubsystem<FileSystem>();
    context_->RegisterSubsystem<ResourceCache>();

    SubscribeToEvent(E_EXITREQUESTED, MY3D_HANDLER(Engine, HandleExitRequested));
}

bool Engine::Initialize(const VariantMap& parameters)
{
    if (initialized_)
        return true;
    // Set headless mode
    headless_ = GetParameter(parameters, EP_HEADLESS, false).GetBool();
    if (!headless_)
    {
        context_->RegisterSubsystem<Graphics>();
    }

    // Start logging
    auto* log = GetSubsystem<Log>();
    if (log)
    {
        if (HasParameter(parameters, EP_LOG_LEVEL))
            log->SetLevel(GetParameter(parameters, EP_LOG_LEVEL).GetInt());

        log->SetQuiet(GetParameter(parameters, EP_LOG_QUIET, false).GetBool());
        log->Open(GetParameter(parameters, EP_LOG_NAME, "My3D.log").GetString());
    }

    GetSubsystem<Time>()->SetTimerPeriod(1);

    // Configure max FPS
    if (!GetParameter(parameters, EP_FRAME_LIMITER, false).GetBool())
        SetMaxFps(0);

    // Set amount of worker threads according to the available physical CPU cores.
    unsigned numThreads = GetNumPhysicalCPUs() - 1;
    if (numThreads)
    {
        GetSubsystem<WorkQueue>()->CreateThreads(numThreads);
        MY3D_LOGINFOF("Created %u worker thread%s", numThreads, numThreads > 1 ? "s" : "");
    }

    // Add resource paths
    if (!InitializeResourceCache(parameters, false))
        return false;

    auto* cache = GetSubsystem<ResourceCache>();
    auto* fileSystem = GetSubsystem<FileSystem>();

    // Initialize graphics & audio output
    if (!headless_)
    {
        auto* graphics = GetSubsystem<Graphics>();

        graphics->SetWindowTitle(GetParameter(parameters, EP_WINDOW_TITLE, "My3D").GetString());

        if (HasParameter(parameters, EP_WINDOW_POSITION_X) && HasParameter(parameters, EP_WINDOW_POSITION_Y))
        {
            graphics->SetWindowPosition
            (
                GetParameter(parameters, EP_WINDOW_POSITION_X).GetInt(),
                GetParameter(parameters, EP_WINDOW_POSITION_Y).GetInt()
            );
        }

        if (!graphics->SetMode(
                GetParameter(parameters, EP_WINDOW_WIDTH, 0).GetInt(),
                GetParameter(parameters, EP_WINDOW_HEIGHT, 0).GetInt(),
                GetParameter(parameters, EP_FULL_SCREEN, false).GetBool(),
                GetParameter(parameters, EP_BORDERLESS, false).GetBool(),
                GetParameter(parameters, EP_WINDOW_RESIZABLE, false).GetBool(),
                GetParameter(parameters, EP_HIGH_DPI, true).GetBool(),
                GetParameter(parameters, EP_VSYNC, false).GetBool(),
                GetParameter(parameters, EP_TRIPLE_BUFFER, false).GetBool(),
                GetParameter(parameters, EP_MULTI_SAMPLE, 1).GetInt(),
                GetParameter(parameters, EP_MONITOR, 0).GetInt(),
                GetParameter(parameters, EP_REFRESH_RATE, 0).GetInt()
        ))
            return false;

        // Initialize input
        if (HasParameter(parameters, EP_TOUCH_EMULATION))
            GetSubsystem<Input>()->SetTouchEmulation(GetParameter(parameters, EP_TOUCH_EMULATION).GetBool());
    }

    frameTimer_.Reset();

    MY3D_LOGINFO("Initialized engine");
    initialized_ = true;
    return true;
}

bool Engine::InitializeResourceCache(const VariantMap& parameters, bool removeOld /*= true*/)
{
    return true;
}

void Engine::RunFrame()
{
    assert(initialized_);

    if (exiting_)
        return;

    auto* time = GetSubsystem<Time>();
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
