//
// Created by luchu on 2022/1/1.
//

#include "Object.h"

namespace My3D
{

TypeInfo::TypeInfo(const char *typeName, const My3D::TypeInfo *baseTypeInfo)
    : type_(typeName)
    , typeName_(typeName)
    , baseTypeInfo_(baseTypeInfo)
{
}

bool TypeInfo::IsTypeOf(StringHash type) const
{
    const TypeInfo* current = this;
    while (current)
    {
        if (current->GetType() == type)
            return true;
        current = current->GetBaseTypeInfo();
    }

    return false;
}

bool TypeInfo::IsTypeOf(const TypeInfo *typeInfo) const
{
    if (typeInfo == nullptr)
        return false;

    const TypeInfo* current = this;
    while (current)
    {
        if (current == typeInfo || current->GetType() == typeInfo->GetType())
            return true;
        current = current->GetBaseTypeInfo();
    }

    return false;
}

Object::Object(Context* context)
    : context_(context)
    , blockEvents_(false)
{

}

Object::~Object()
{

}

}

