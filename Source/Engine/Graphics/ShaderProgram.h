//
// Created by luchu on 2022/2/20.
//

#pragma once

// Note: ShaderProgram class is purposefully API-specific. It should not be used by client applications.

#if defined(MY3D_OPENGL)
#include "OpenGL/OGLShaderProgram.h"
#else
#include "Direct3D11/D3D11ShaderProgram.h"
#endif
