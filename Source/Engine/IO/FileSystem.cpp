//
// Created by luchu on 2022/1/13.
//

#include "Core/Context.h"
#include "Core/Thread.h"
#include "IO/FileSystem.h"
#include "IO/File.h"
#include "IO/Log.h"
#include "IO/IOEvents.h"
#include "Core/CoreEvents.h"

#include <sys/stat.h>
#include <cstdio>


namespace My3D
{

FileSystem::FileSystem(Context *context)
    : Object(context)
{
    SubscribeToEvent(E_BEGINFRAME, MY3D_HANDLER(FileSystem, HandleBeginFrame));
    // Subscribe to console command
    SetExecuteConsoleCommands(true);
}

void FileSystem::SetExecuteConsoleCommands(bool enable)
{
    if (enable == executeConsoleCommands_)
        return;

    executeConsoleCommands_ = enable;
}

}