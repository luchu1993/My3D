//
// Created by luchu on 2022/1/1.
//

#pragma once

#include "My3D.h"
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
    void DoExit();

    float timeStep_;
    unsigned minFps_;
    unsigned maxFps_;
    bool autoExit_;
    bool initialized_;
    bool exiting_;
};

}