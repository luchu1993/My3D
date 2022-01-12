//
// Created by luchu on 2022/1/8.
//

#pragma once

#include "Container/String.h"
#include <cstdlib>


namespace My3D
{
/// Display an error dialog with the specified title and message.
MY3D_API void ErrorDialog(const String& title, const String& message);
/// Exit the application with an error message to the console.
MY3D_API void ErrorExit(const String& message = String::EMPTY, int exitCode = EXIT_FAILURE);
/// Open a console window.
MY3D_API void OpenConsoleWindow();
/// Print Unicode text to the console. Will not be printed to the MSVC output window.
MY3D_API void PrintUnicode(const String& str, bool error = false);
/// Print Unicode text to the console with a newline appended. Will not be printed to the MSVC output window.
MY3D_API void PrintUnicodeLine(const String& str, bool error = false);
/// Print ASCII text to the console with a newline appended. Uses printf() to allow printing into the MSVC output window.
MY3D_API void PrintLine(const String& str, bool error = false);
/// Print ASCII text to the console with a newline appended. Uses printf() to allow printing into the MSVC output window.
MY3D_API void PrintLine(const char* str, bool error = false);
/// Parse arguments from the command line. First argument is by default assumed to be the executable name and is skipped.
MY3D_API const Vector<String>& ParseArguments(const String& cmdLine, bool skipFirstArgument = true);
/// Parse arguments from the command line.
MY3D_API const Vector<String>& ParseArguments(const char* cmdLine);
/// Parse arguments from a wide char command line.
MY3D_API const Vector<String>& ParseArguments(const WString& cmdLine);
/// Parse arguments from a wide char command line.
MY3D_API const Vector<String>& ParseArguments(const wchar_t* cmdLine);
/// Parse arguments from argc & argv.
MY3D_API const Vector<String>& ParseArguments(int argc, char** argv);
/// Return previously parsed arguments.
MY3D_API const Vector<String>& GetArguments();
}
