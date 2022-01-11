//
// Created by luchu on 2022/1/11.
//

#pragma once

#include "Core/Object.h"


namespace My3D
{
    /// Mouse button pressed
    MY3D_EVENT(E_MOUSEBUTTONDOWN, NouseButtonDown)
    {
        MY3D_PARAM(P_BUTTON, Button);           // int
        MY3D_PARAM(P_BUTTONS, Buttons);         // int
        MY3D_PARAM(P_QUALIFIERS, Qualifiers);   // int
        MY3D_PARAM(P_CLICKS, Clicks);           // int
    }

    /// Application exit requested
    MY3D_EVENT(E_EXITREQUESTED, ExitRequested)
    {
    }
}
