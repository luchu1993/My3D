//
// Created by luchu on 2022/1/13.
//

#include "Core/Object.h"

namespace My3D
{

/// Log message event
MY3D_EVENT(E_LOGMESSAGE, LogMessage)
{
    MY3D_PARAM(P_MESSAGE, Message); // String
    MY3D_PARAM(P_LEVEL, Level);     // int
}

/// Async system command execution finished.
MY3D_EVENT(E_ASYNCEXECFINISHED, AsyncExecFinished)
{
    MY3D_PARAM(P_REQUESTID, RequestID);          // unsigned
    MY3D_PARAM(P_EXITCODE, ExitCode);            // int
}

}
