//
// Created by luchu on 2022/2/20.
//

#include "Graphics/Graphics.h"
#include "Graphics/Shader.h"
#include "Graphics/ShaderVariation.h"


namespace My3D
{
    ShaderParameter::ShaderParameter(const String& name, unsigned  glType, int location)
        : name_(name)
        , glType_(glType)
        , location_(location)
    {
    }

    ShaderParameter::ShaderParameter(ShaderType type, const String& name, unsigned offset, unsigned size, unsigned buffer)
        : type_(type)
        , name_(name)
        , offset_(offset)
        , size_(size)
        , buffer_(buffer)
    {
    }

    ShaderParameter::ShaderParameter(ShaderType type, const String& name, unsigned reg, unsigned regCount)
        : type_{type}
        , name_{name}
    {
    }

    ShaderVariation::ShaderVariation(Shader *owner, ShaderType type)
        : GPUObject(owner->GetSubsystem<Graphics>())
        , owner_(owner)
        , type_(type)
    {
    }

    ShaderVariation::~ShaderVariation()
    {
        Release();
    }

    void ShaderVariation::SetName(const String &name)
    {
        name_ = name;
    }

    Shader* ShaderVariation::GetOwner() const
    {
        return owner_;
    }
}
