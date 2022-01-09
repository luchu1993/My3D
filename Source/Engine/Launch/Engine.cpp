//
// Created by luchu on 2022/1/1.
//

#include "Launch/Engine.h"
#include "Core/Context.h"
#include "IO/Log.h"
#include "Core/Timer.h"

#include <cassert>


namespace My3D
{
Engine::Engine(Context *context)
    : Base(context)
    , timeStep_(0.0f)
    , minFps_(10)
    , maxFps_(200)
    , autoExit_(true)
    , initialized_(false)
    , exiting_(false)
{
    // Register self as a subsystem
    context_->RegisterSubsystem(this);

#ifdef MY3D_LOGGING
    context_->RegisterSubsystem<Log>();
#endif

    context->RemoveSubsystem<Time>();
}

bool Engine::Initialize()
{
    if (initialized_)
        return true;

    initialized_ = true;
    return true;
}

void Engine::RunFrame()
{
    assert(initialized_);

    if (exiting_)
        return;

    Update();
    Render();
    ApplyFrameLimit();
}

void Engine::Render()
{

}

void Engine::ApplyFrameLimit()
{
    if (!initialized_)
        return;
}

void Engine::Exit()
{
    DoExit();
}

void Engine::DoExit()
{
    exiting_ = true;
}

void Engine::Update()
{

}

}
