//
// Created by luchu on 2022/2/20.
//

#pragma once

#if defined(MY3D_OPENGL)
//#error OpenGL Graphics API does not have VertexDeclaration class, remove this header file in your build to fix this error
#else
#include "Direct3D11/D3D11VertexDeclaration.h"
#endif