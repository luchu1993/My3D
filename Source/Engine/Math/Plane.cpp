//
// Created by luchu on 2022/1/9.
//

#include "Math/Plane.h"

namespace My3D
{

// Static initialization order can not be relied on, so do not use Vector3 constants
const Plane Plane::UP(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f));

}