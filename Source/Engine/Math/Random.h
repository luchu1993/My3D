//
// Created by luchu on 2022/1/24.
//

#pragma once

#include "My3D.h"

namespace My3D
{
/// Set the random seed. The default seed is 1.
MY3D_API void SetRandomSeed(unsigned seed);
/// Return the current random seed.
MY3D_API unsigned GetRandomSeed();
/// Return a random number between 0-32767. Should operate similarly to MSVC rand().
MY3D_API int Rand();
/// Return a standard normal distributed number.
MY3D_API float RandStandardNormal();
}
