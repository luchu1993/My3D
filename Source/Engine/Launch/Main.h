//
// Created by luchu on 2022/1/1.
//

#pragma once

#include "Core/ProcessUtils.h"

#ifdef PLATFORM_MSVC
#include <windows.h>
#include <crtdbg.h>
#endif

// Define a platform specific main function, which in turn executes the user-defined function

#ifdef PLATFORM_MSVC
    #ifdef _DEBUG
        #define MY3D_DEFINE_MAIN(function) \
        int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, PSTR cmdLine, int showCmd) \
        { \
            _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); \
            My3D::ParseArguments(GetCommandLineW()); \
            return function; \
        }
    #else
        int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd) \
        { \
            My3D::ParseArguments(GetCommandLineW()); \
            return function; \
        }
    #endif
#else

#define MY3D_DEFINE_MAIN(function) \
int main(int argc, char* argv[]) \
{\
    return function;\
}

#endif
