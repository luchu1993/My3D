//
// Created by luchu on 2022/1/9.
//


#include "Math/BoundingBox.h"

namespace My3D
{

    String BoundingBox::ToString() const
    {
        return min_.ToString() + " - " + max_.ToString();
    }
}
