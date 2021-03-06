#pragma once

#include "Core/Attribute.h"
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
    /// Receivers. May contains holes during sending
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

    /// Initialises the specified SDL systems, if not already.
    bool RequireSDL(unsigned sdlFlags);
    /// Indicate the you are done with using SDL. Must be called after using RequireSDL()
    void ReleaseSDL();

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
    /// Return object type name from hash.
    const String& GetTypeName(StringHash objectType) const;
    /// Template version of registering an object factory
    template<typename T> void RegisterFactory();
    /// Template version of registering an object factory with category.
    template<typename T> void RegisterFactory(const char* category);

    /// Register a subsystem
    void RegisterSubsystem(Object* object);
    /// Remove a subsystem
    void RemoveSubsystem(StringHash objectType);
    /// Template version of registering an subsystem
    template<typename T> T* RegisterSubsystem();
    /// Template version of removing a subsystem
    template<typename T> void RemoveSubsystem();
    /// Return subsystem by type
    Object* GetSubsystem(StringHash type) const;
    /// Template version of return a subsystem
    template<typename T> T* GetSubsystem() const;
    /// Return global variable based on key.

    const Variant& GetGlobalVar(StringHash key) const;
    /// Return all global variables.
    const VariantMap& GetGlobalVars() const { return globalVars_; }
    /// Set global variable with the respective key and value.
    void SetGlobalVar(StringHash key, const Variant& value);

    /// Return a preallocated map for event data. Used for optimization to avoid constant re-allocation of event data maps.
    VariantMap& GetEventDataMap();
    /// Return active event sender.
    Object* GetEventSender() const;
    /// Return active event handler.
    EventHandler* GetEventHandler() const { return eventHandler_; }
    /// Return event receiver for a sender and event type
    EventReceiverGroup* GetEventReceivers(Object* sender, StringHash eventType) const
    {
        auto i = specificEventReceivers_.Find(sender);
        if (i != specificEventReceivers_.End())
        {
            auto j = i->second_.Find(eventType);
            return j != i->second_.End() ? j->second_ : nullptr;
        }
        else
            return nullptr;
    }
    /// Return event receivers for an event type
    EventReceiverGroup* GetEventReceivers(StringHash eventType) const
    {
        auto i = eventReceivers_.Find(eventType);
        return i != eventReceivers_.End() ? i->second_ : nullptr;
    }

    /// Register object attribute.
    AttributeHandle RegisterAttribute(StringHash objectType, const AttributeInfo& attr);
    /// Remove object attribute.
    void RemoveAttribute(StringHash objectType, const char* name);
    /// Remove all object attributes.
    void RemoveAllAttributes(StringHash objectType);
    /// Update object attribute's default value.
    void UpdateAttributeDefaultValue(StringHash objectType, const char* name, const Variant& defaultValue);
    /// Copy base class attributes to derived class.
    void CopyBaseAttributes(StringHash baseType, StringHash derivedType);
    /// Return a specific attribute description for an object, or null if not found
    AttributeInfo* GetAttribute(StringHash objectType, const char* name);
    /// Return attribute descriptions for an object type, or null if none defined.
    const Vector<AttributeInfo>* GetAttributes(StringHash type) const
    {
        auto it = attributes_.Find(type);
        return it != attributes_.End() ? &it->second_ : nullptr;
    }
    /// Return network replication attribute descriptions for an object type, or null if none defined.
    const Vector<AttributeInfo>* GetNetworkAttributes(StringHash type) const
    {
        auto it = networkAttributes_.Find(type);
        return it != networkAttributes_.End() ? &it->second_ : nullptr;
    }
    /// Return all registered attributes.
    const HashMap<StringHash, Vector<AttributeInfo> >& GetAllAttributes() const { return attributes_; }
    /// Template version of registering an object attribute.
    template <typename T> AttributeHandle RegisterAttribute(const AttributeInfo& attr);
    /// Template version of removing an object attribute.
    template <typename T> void RemoveAttribute(const char* name);
    /// Template version of removing all object attributes.
    template <typename T> void RemoveAllAttributes();
    /// Template version of updating an object attribute's default value.
    template <typename T> void UpdateAttributeDefaultValue(const char* name, const Variant& defaultValue);
    /// Template version of copying base class attributes to derived class.
    template <typename T, typename U> void CopyBaseAttributes();
    /// Template version of returning a specific attribute description
    template<typename T> AttributeInfo* GetAttribute(const char* name);

private:
    /// Add event receiver
    void AddEventReceiver(Object* receiver, StringHash eventType);
    /// Add event receiver for specific event
    void AddEventReceiver(Object* receiver, Object* sender, StringHash eventType);
    /// Remove an event sender from all receiver. called on its destruction
    void RemoveEventSender(Object* sender);
    /// Remove event receiver from specific events
    void RemoveEventReceiver(Object* receiver, Object* sender, StringHash eventType);
    /// Remove event receiver from no-specific events
    void RemoveEventReceiver(Object* receiver, StringHash eventType);
    /// Begin send event
    void BeginSendEvent(Object* sender, StringHash eventType);
    /// End event send. Clean up event receivers removed in the meanwhile
    void EndSendEvent();
    /// Set current event handler. Called by object.
    void SetEventHandler(EventHandler* handler) { eventHandler_ = handler; }

    /// Object factories
    HashMap<StringHash, SharedPtr<ObjectFactory>> factories_;
    /// Subsystems
    HashMap<StringHash, SharedPtr<Object>> subsystems_;
    /// Object categories
    HashMap<String, Vector<StringHash>> objectCategories_;
    /// Variant map for global variables that can persist throughout application execution.
    VariantMap globalVars_;
    /// Event receivers for non-specific events
    HashMap<StringHash, SharedPtr<EventReceiverGroup>> eventReceivers_;
    /// Event receivers for specific events
    HashMap<Object*, HashMap<StringHash, SharedPtr<EventReceiverGroup>>> specificEventReceivers_;
    /// Event sender stack
    PODVector<Object*> eventSenders_;
    /// Event data stack
    PODVector<VariantMap*> eventDataMaps_;
    /// Active event handler.
    EventHandler* eventHandler_;

    /// Attribute descriptions per object type
    HashMap<StringHash, Vector<AttributeInfo>> attributes_;
    /// Network replication attribute descriptions per object type
    HashMap<StringHash, Vector<AttributeInfo>> networkAttributes_;
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

template <typename T> T* Context::GetSubsystem() const
{
    return static_cast<T*>(GetSubsystem(T::GetTypeStatic()));
}

template <typename T> AttributeHandle Context::RegisterAttribute(const AttributeInfo& attr)
{
    return RegisterAttribute(T::GetTypeStatic(), attr);
}

template <typename T> void Context::RemoveAttribute(const char *name)
{
    RemoveAttribute(T::GetTypeStatic(), name);
}

template <typename T> void Context::RemoveAllAttributes()
{
    RemoveAllAttributes(T::GetTypeStatic());
}

template <typename T> AttributeInfo* Context::GetAttribute(const char* name)
{
    return GetAttribute(T::GetTypeStatic(), name);
}

template <typename T> void Context::UpdateAttributeDefaultValue(const char *name, const Variant& defaultValue)
{
    UpdateAttributeDefaultValue(T::GetTypeStatic(), name, defaultValue);
}

template <typename T, typename U> void Context::CopyBaseAttributes()
{
    CopyBaseAttributes(T::GetTypeStatic(), U::GetTypeStatic());
}

}

