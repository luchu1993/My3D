//
// Created by luchu on 2022/1/11.
//

#pragma once

#include "Core/Object.h"

/// Frame begin event
MY3D_EVENT(E_BEGINFRAME, BeginFrame)
{
    MY3D_PARAM(P_FRAMENUMBER, FrameNumber);
    MY3D_PARAM(P_TIMESTEP, TimeStep);
}

/// Application-wide logic update event
MY3D_EVENT(E_UPDATE, Update)
{
    MY3D_PARAM(P_TIMESTEP, TimeStep);
}

/// Application-wide logic post-update event
MY3D_EVENT(E_POSTUPDATE, PostUpdate)
{
    MY3D_PARAM(P_TIMESTEP, TimeStep);
}

/// Render update event
MY3D_EVENT(E_RENDERUPDATE, RenderUpdate)
{
    MY3D_PARAM(P_TIMESTEP, TimeStep);
}

/// Post-render update event
MY3D_EVENT(E_POSTRENDERUPDATE, PostRenderUpdate)
{
    MY3D_PARAM(P_TIMESTEP, TimeStep);
}

/// Frame end event
MY3D_EVENT(E_ENDFRAME, EndFrame)
{
}