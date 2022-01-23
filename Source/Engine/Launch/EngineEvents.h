//
// Created by luchu on 2022/1/23.
//

#pragma once

#include "Core/Object.h"

namespace My3D
{

/// A command has been entered on the console.
MY3D_EVENT(E_CONSOLECOMMAND, ConsoleCommand)
{
    MY3D_PARAM(P_COMMAND, Command);              // String
    MY3D_PARAM(P_ID, Id);                        // String
}

}
