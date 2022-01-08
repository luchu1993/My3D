#pragma once

#include "My3D.h"
#include "Core/Object.h"
#include "Core/StringHash.h"
#include "Container/String.h"
#include "Container/RefCounted.h"
#include "Container/Ptr.h"
#include "Container/HashMap.h"



namespace My3D
{

/// My3D execution context. Provides access to subsystems, object factories and attributes.
class MY3D_API Context : public RefCounted
{
    friend class Object;

public:
    /// Construct
    Context();
    /// Destruct
    ~Context() override;
    /// Create an object by type. Return pointer to it or null if factory not found
    template<typename T> inline SharedPtr<T> CreateObject()
    {
        return StaticCast<T>(CreateObject(T::GetTypeStatic()));
    }
    /// Create an object by type hash. Return pointer to it or null if no factory found.
    SharedPtr<Object> CreateObject(StringHash objectType);
    /// Register a factory for an object type.
    void RegisterFactory(ObjectFactory* factory);
    /// Register a factory for an object type and specify the object category
    void RegisterFactory(ObjectFactory* factory, const char* category);
    /// Register a subsystem
    void RegisterSubsystem(Object* object);
    /// Remove a subsystem
    void RemoveSubsystem(StringHash objectType);
    /// Initialises the specified SDL systems, if not already.
    bool RequireSDL(unsigned sdlFlags);
    /// Indicate the you are done with using SDL. Must be called after using RequireSDL()
    void ReleaseSDL();
    /// Template version of registering an object factory
    template<typename T> void RegisterFactory();
    /// Template version of registering an object factory with category.
    template<typename T> void RegisterFactory(const char* category);
    /// Template version of registering an subsystem
    template<typename T> T* RegisterSubsystem();
    /// Template verison of removing a subsystem
    template<typename T> void RemoveSubsystem();
    /// Return subsystem by type
    Object* GetSubsystem(StringHash type) const;
    /// Template version of return a subsystem
    template<typename T> T* GetSubSystem() const;

private:
    /// Object factories
    HashMap<StringHash, SharedPtr<ObjectFactory>> factories_;
    /// Subsystems
    HashMap<StringHash, SharedPtr<Object>> subsystems_;
    /// Object categories
    HashMap<String, Vector<StringHash>> objectCategories_;
};

template<typename T> void Context::RegisterFactory()
{
    RegisterFactory(new ObjectFactoryImpl<T>(this));
}

template <typename T> void Context::RegisterFactory(const char* category)
{
    RegisterFactory(new ObjectFactoryImpl<T>(this), category);
}

template <typename T> T* Context::RegisterSubsystem()
{
    auto* subsystem = new T(this);
    RegisterSubsystem(subsystem);
    return subsystem;
}

template <typename T> void Context::RemoveSubsystem()
{
    RemoveSubsystem(T::GetTypeStatic());
}

template <typename T> T* Context::GetSubSystem() const
{
    return static_cast<T*>(GetSubsystem(T::GetTypeStatic()));
}

}

