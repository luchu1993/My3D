//
// Created by luchu on 2022/1/1.
//

#include "Container/VectorBase.h"

namespace My3D
{

unsigned char* VectorBase::AllocateBuffer(unsigned int size)
{
    return new unsigned char[size];
}

}

