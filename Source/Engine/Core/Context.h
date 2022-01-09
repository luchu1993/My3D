#pragma once

#include "My3D.h"
#include "Core/Object.h"
#include "Core/Variant.h"
#include "Core/StringHash.h"
#include "Container/String.h"
#include "Container/RefCounted.h"
#include "Container/Ptr.h"
#include "Container/HashMap.h"


namespace My3D
{

    /// Tracking structure for event receivers
    class MY3D_API EventReceiverGroup : public RefCounted
    {
    public:
        /// Construct
        EventReceiverGroup()
            : inSend_(false)
            , dirty_(false)
        {
        }

        /// Begin send event.
        void BeginSendEvent();
        /// End event send. Clean up if necessary
        void EndSendEvent();
        /// Add receiver.  Leave holes during send, which requires later cleanup.
        void Add(Object* object);
        /// Remove receiver
        void Remove(Object* object);
        /// Receivers. My contains holes during sending
        PODVector<Object*> receivers_;

    private:
        /// "In send" recursion counter
        unsigned inSend_;
        /// Cleanup required flag
        bool dirty_;
    };

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
    /// Return global variable based on key.
    const Variant& GetGlobalVar(StringHash key) const;
    /// Return all global variables.
    const VariantMap& GetGlobalVars() const { return globalVars_; }
    /// Set global variable with the respective key and value.
    void SetGlobalVar(StringHash key, const Variant& value);
    /// Event receivers for non-specific events
    HashMap<StringHash, SharedPtr<EventReceiverGroup>> eventReceivers_;

private:
    /// Object factories
    HashMap<StringHash, SharedPtr<ObjectFactory>> factories_;
    /// Subsystems
    HashMap<StringHash, SharedPtr<Object>> subsystems_;
    /// Object categories
    HashMap<String, Vector<StringHash>> objectCategories_;
    /// Variant map for global variables that can persist throughout application execution.
    VariantMap globalVars_;
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

