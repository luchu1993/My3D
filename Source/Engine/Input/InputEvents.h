//
// Created by luchu on 2022/1/11.
//

#pragma once

#include "Core/Object.h"


namespace My3D
{
    /// Mouse button pressed
    MY3D_EVENT(E_MOUSEBUTTONDOWN, MouseButtonDown)
    {
        MY3D_PARAM(P_BUTTON, Button);           // int
        MY3D_PARAM(P_BUTTONS, Buttons);         // int
        MY3D_PARAM(P_QUALIFIERS, Qualifiers);   // int
        MY3D_PARAM(P_CLICKS, Clicks);           // int
    }

    /// Mouse button released.
    MY3D_EVENT(E_MOUSEBUTTONUP, MouseButtonUp)
    {
        MY3D_PARAM(P_BUTTON, Button);                // int
        MY3D_PARAM(P_BUTTONS, Buttons);              // int
        MY3D_PARAM(P_QUALIFIERS, Qualifiers);        // int
    }

    /// Key pressed.
    MY3D_EVENT(E_KEYDOWN, KeyDown)
    {
        MY3D_PARAM(P_KEY, Key);                      // int
        MY3D_PARAM(P_SCANCODE, Scancode);            // int
        MY3D_PARAM(P_BUTTONS, Buttons);              // int
        MY3D_PARAM(P_QUALIFIERS, Qualifiers);        // int
        MY3D_PARAM(P_REPEAT, Repeat);                // bool
    }

    /// Key released.
    MY3D_EVENT(E_KEYUP, KeyUp)
    {
        MY3D_PARAM(P_KEY, Key);                      // int
        MY3D_PARAM(P_SCANCODE, Scancode);            // int
        MY3D_PARAM(P_BUTTONS, Buttons);              // int
        MY3D_PARAM(P_QUALIFIERS, Qualifiers);        // int
    }

    /// Application exit requested
    MY3D_EVENT(E_EXITREQUESTED, ExitRequested)
    {
    }

    /// Raw SDL input event.
    MY3D_EVENT(E_SDLRAWINPUT, SDLRawInput)
    {
        MY3D_PARAM(P_SDLEVENT, SDLEvent);           // SDL_Event*
        MY3D_PARAM(P_CONSUMED, Consumed);           // bool
    }

    /// Input handling begins.
    MY3D_EVENT(E_INPUTBEGIN, InputBegin)
    {
    }

    /// Input handling ends.
    MY3D_EVENT(E_INPUTEND, InputEnd)
    {
    }
}
