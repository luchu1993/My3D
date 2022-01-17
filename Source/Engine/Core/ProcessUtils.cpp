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
static String currentLine;
static Vector<String> arguments;

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
    FILE* out = error ? stderr : stdout;
    fprintf(out, "%s", str.CString());
    fflush(out);
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

const Vector<String>& GetArguments()
{
    return arguments;
}

const Vector<String>& ParseArguments(const String& cmdLine, bool skipFirstArgment)
{
    arguments.Clear();
    unsigned cmdStart = 0, cmdEnd = 0;
    bool inCmd = false;
    bool inQuote = false;

    for (unsigned i = 0; i < cmdLine.Length(); ++i)
    {
        if (cmdLine[i] == '\"')
            inQuote = !inQuote;
        if (cmdLine[i] == ' ' && !inQuote)
        {
            if (inCmd)
            {
                inCmd = false;
                cmdEnd = i;
                if (!skipFirstArgment)
                    arguments.Push(cmdLine.Substring(cmdStart, cmdEnd - cmdStart));
                skipFirstArgment = false;
            }
        }
        else 
        {
            if (!inCmd)
            {
                inCmd = true;
                cmdStart = i;
            }
        }
    }

    if (inCmd)
    {
        cmdEnd = cmdLine.Length();
        if (!skipFirstArgment)
            arguments.Push(cmdLine.Substring(cmdStart, cmdEnd - cmdStart));
    }

    // Strip double quotes from the arguments
    for (unsigned i = 0; i < arguments.Size(); ++i)
        arguments[i].Replace("\"", "");
    
    return arguments;
}

const Vector<String>& ParseArguments(const char* cmdLine)
{
    return ParseArguments(String(cmdLine));
}

const Vector<String>& ParseArguments(const WString& cmdLine)
{
    return ParseArguments(String(cmdLine));
}

const Vector<String>& ParseArguments(const wchar_t* cmdLine)
{
    return ParseArguments(String(cmdLine));
}

const Vector<String>& ParseArguments(int argc, char** argv)
{
    String cmdLine;
    for (int i = 0; i < argc; ++i)
        cmdLine.AppendWithFormat("\"%s\"", (const char*)argv[i]);
    return ParseArguments(cmdLine);
}

}