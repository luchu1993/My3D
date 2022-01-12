#include "Launch/Application.h"
#include "Launch/Engine.h"
#include "Core/ProcessUtils.h"

#include <cstdlib>


namespace My3D
{

Application::Application(Context *context)
    : Base(context)
    , exitCode_(EXIT_SUCCESS)
{
    engineParameters_ = Engine::ParseParameters(GetArguments());
    engine_ = new Engine(context);
}

Application::~Application() = default;

int Application::Run()
{
    Setup();
    if (exitCode_)
        return exitCode_;

    if (!engine_->Initialize(engineParameters_))
    {
        ErrorExit();
        return exitCode_;
    }

    Start();
    if (exitCode_)
        return exitCode_;

    while (!engine_->IsExiting())
    {
        engine_->RunFrame();
    }

    Stop();
    return exitCode_;
}

void Application::ErrorExit(const String &message)
{
    engine_->Exit();
    exitCode_ = EXIT_FAILURE;
}

}