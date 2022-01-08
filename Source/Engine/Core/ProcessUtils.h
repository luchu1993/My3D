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
}
