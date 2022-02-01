//
// Created by luchu on 2022/2/1.
//

#pragma once

#include "Core/Object.h"

namespace My3D
{
    /// Variable timestep scene update.
    MY3D_EVENT(E_SCENEUPDATE, SceneUpdate)
    {
        MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
        MY3D_PARAM(P_TIMESTEP, TimeStep);            // float
    }

    /// Scene subsystem update.
    MY3D_EVENT(E_SCENESUBSYSTEMUPDATE, SceneSubsystemUpdate)
    {
        MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
        MY3D_PARAM(P_TIMESTEP, TimeStep);            // float
    }

    /// A serializable's temporary state has changed.
    MY3D_EVENT(E_TEMPORARYCHANGED, TemporaryChanged)
    {
        MY3D_PARAM(P_SERIALIZABLE, Serializable);    // Serializable pointer
    }
}
