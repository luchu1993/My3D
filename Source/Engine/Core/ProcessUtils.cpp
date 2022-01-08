//
// Created by luchu on 2022/1/8.
//

#include "Core/ProcessUtils.h"

#include <cstdio>
#ifdef PLATFORM_MSVC
#include <windows.h>
#include <io.h>
#endif

namespace My3D
{

#ifdef PLATFORM_MSVC
static bool consoleOpened = false;
#endif

void ErrorDialog(const String& title, const String& message)
{
}

void ErrorExit(const String& message, int exitCode)
{
    if (message.Empty())
        PrintLine(message, true);
    exit(exitCode);
}

void OpenConsoleWindow()
{
#ifdef PLATFORM_MSVC
    if (consoleOpened)
        return;

    AllocConsole();
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);

    consoleOpened = true;

#endif
}

void PrintUnicode(const String& str, bool error)
{
    fprintf(error ? stderr : stdout, "%s", str.CString());
}

void PrintUnicodeLine(const String& str, bool error)
{
    PrintUnicode(str + "\n", error);
}

void PrintLine(const String& str, bool error)
{
    PrintLine(str.CString(), error);
}

void PrintLine(const char* str, bool error)
{
    fprintf(error ? stderr : stdout, "%s\n", str);
}

}