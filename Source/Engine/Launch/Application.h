//
// Created by luchu on 2022/1/1.
//

#pragma once

#include "My3D.h"
#include "Core/Object.h"
#include "Core/Context.h"
#include "Container/Ptr.h"
#include "Launch/Main.h"
#include "Launch/Engine.h"


namespace My3D {

class MY3D_API Application : public Object
{
    MY3D_OBJECT(Application, Object)

public:
    explicit Application(Context *context);
    ~Application () override;

    /// Setup before engine initialization.
    virtual void Setup() {}
    /// setup after engine initialization.
    virtual void Start() {}
    /// cleanup after the main loop
    virtual void Stop() {}
    /// Initialize the engine and run main loop.
    int Run();
    /// Show error message.
    void ErrorExit(const String &message = String::EMPTY);

protected:
    SharedPtr<Engine> engine_;
    String startupErrors_;
    int exitCode_;
};


#define MY3D_DEFINE_APPLICATION_MAIN(className) \
int RunApplication()                            \
{                                               \
    My3D::SharedPtr<My3D::Context> context(new My3D::Context()); \
    My3D::SharedPtr<className> application(new className(context)); \
    return application->Run();                   \
}                                               \
MY3D_DEFINE_MAIN(RunApplication())

}