//
// Created by luchu on 2022/1/1.
//

#pragma once

#include "My3D.h"
#include "Core/StringHash.h"
#include "Container/RefCounted.h"
#include "Container/Ptr.h"


#include <cassert>


namespace My3D
{

class Context;


class MY3D_API TypeInfo
{
public:
    TypeInfo(const char* typeName, const TypeInfo* baseTypeInfo);
    ~TypeInfo() = default;

    bool IsTypeOf(StringHash type) const;
    bool IsTypeOf(const TypeInfo* typeInfo) const;

    template<typename T>
    bool IsTypeOf() const { return IsTypeOf(T::GetTypeInfoStatic()); }

    StringHash GetType() const { return type_; }
    const String& GetTypeName() const { return typeName_; }
    const TypeInfo* GetBaseTypeInfo() const { return baseTypeInfo_; }

private:
    StringHash type_;
    String typeName_;
    const TypeInfo* baseTypeInfo_;
};


#define MY3D_OBJECT(typeName, baseTypeName) \
    public: \
        using Type = typeName; \
        using Base = baseTypeName; \
        virtual My3D::StringHash GetType() const override { return GetTypeInfoStatic()->GetType(); } \
        virtual const My3D::String& GetTypeName() const override { return GetTypeInfoStatic()->GetTypeName(); } \
        virtual const My3D::TypeInfo* GetTypeInfo() const override { return GetTypeInfoStatic(); } \
        static My3D::StringHash GetTypeStatic() { return GetTypeInfoStatic()->GetType(); } \
        static const My3D::String& GetTypeNameStatic() { return GetTypeInfoStatic()->GetTypeName(); } \
        static const My3D::TypeInfo* GetTypeInfoStatic() { static const My3D::TypeInfo typeInfoStatic(#typeName, Base::GetTypeInfoStatic()); return &typeInfoStatic; }


class MY3D_API Object : public RefCounted
{
    friend class Context;

public:
    explicit Object(Context* context);
    virtual ~Object() override;

    virtual StringHash GetType() const = 0;
    virtual const String& GetTypeName() const = 0;
    virtual const TypeInfo* GetTypeInfo() const = 0;

    static const TypeInfo* GetTypeInfoStatic() { return nullptr; }
    bool IsInstanceOf(StringHash type) const;
    bool InInstanceOf(const TypeInfo* typeInfo) const;

    template<typename T> bool IsInstanceOf() const { return IsInstanceOf(T::GetStaticTypeInfo()); }
    template<typename T> T* Cast() { return IsInstanceOf<T>() ?  static_cast<T*>(this) : nullptr; }
    template<typename T> const T* Cast() const { return IsInstanceOf<T>() ?  static_cast<const T*>(this) : nullptr; }

    // virtual void OnEvent(Object* sender, StringHash eventType, VariantMap& eventData);

    Context* GetContext() const { return context_; }

    Object* GetSubSystem(StringHash type) const;
    template<typename T> T* GetSubSystem() const;

protected:
    Context* context_;
    bool blockEvents_;
};

template<typename T>
T* Object::GetSubSystem() const { return static_cast<T*>( GetSubSystem(T::GetTypeStatic()) );}

/// Base class for object factories
class MY3D_API ObjectFactory : public RefCounted
{
public:
    /// Construct
    explicit ObjectFactory(Context* context)
        : context_(context)
    {
        assert(context);
    }
    /// Create an object. Implemented im templated subclasses
    virtual SharedPtr<Object> CreateObject() = 0;
    /// Return execution context
    Context* GetContext() const { return context_; }
    /// Return type info of objects created by this factory
    const TypeInfo* GetTypeInfo() const { return typeInfo_; }
    /// Return type hash of objects created by this factory
    StringHash GetType() const { return typeInfo_->GetType(); }
    /// Return type name of objects created by this factory
    const String& GetTypeName() const { return typeInfo_->GetTypeName(); }

protected:
    /// Execution context
    Context* context_;
    /// Type info
    const TypeInfo* typeInfo_{};
};

/// Template implementation of object factory
template <typename T> class ObjectFactoryImpl : public ObjectFactory
{
public:
    /// Construct
    explicit ObjectFactoryImpl(Context* context)
        : ObjectFactory(context)
    {
        typeInfo_ = T::GetTypeInfoStatic();
    }

    /// Create an object of specific type
    SharedPtr<Object> CreateObject() override
    {
        return SharedPtr<Object>(new T(context_));
    }
};

}

