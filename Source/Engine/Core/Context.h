#pragma once

#include "My3D.h"
#include "Container/RefCounted.h"



namespace My3D
{

class MY3D_API Context : public RefCounted
{
public:
    Context() = default;
    ~Context() override = default;
};

}

