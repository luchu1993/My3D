//
// Created by luchu on 2022/1/7.
//
#pragma once

#ifdef MY3D_SHARED
    #ifdef PLATFORM_MSVC
        #ifdef BUILD_ENGINE
            #define MY3D_API __declspec(dllexport)
        #else
            #define MY3D_API __declspec(dllimport)
        #endif
    #else
        #define MY3D_API
    #endif
#else
    #define MY3D_API
#endif
