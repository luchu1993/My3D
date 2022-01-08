//
// Created by luchu on 2022/1/7.
//
#pragma once

#ifdef PLATFORM_MSVC
    #ifdef BUILD_ENGINE
        #define MY3D_API __declspec(dllexport)
    #else
        #define MY3D_API __declspec(dllimport)
    #endif
#else
    #define MY3D_API
#endif

#ifdef PLATFORM_MSVC
    #pragma warning(disable: 4251)
    #pragma warning(disable: 4275)
    #pragma warning(disable: 4996)
#endif

