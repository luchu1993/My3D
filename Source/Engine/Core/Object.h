//
// Created by luchu on 2022/1/1.
//

#pragma once

#include "My3D.h"
#include "Core/StringHash.h"
#include "Container/RefCounted.h"
#include "Container/Ptr.h"
#include "Container/LinkedList.h"
#include "Core/Variant.h"

#include <functional>
#include <cassert>
#include <utility>


namespace My3D
{

class Context;
class EventHandler;


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
    /// Construct
    explicit Object(Context* context);
    /// Destruct. Clean up self from event sender & receiver structures
    ~Object() override;
    /// Return type hash
    virtual StringHash GetType() const = 0;
    /// Return type name
    virtual const String& GetTypeName() const = 0;
    /// Return type info
    virtual const TypeInfo* GetTypeInfo() const = 0;
    /// Return event
    static const TypeInfo* GetTypeInfoStatic() { return nullptr; }
    /// Check current instance is type of specified type
    bool IsInstanceOf(StringHash type) const;
    /// Check current instance is type of specified type
    bool IsInstanceOf(const TypeInfo* typeInfo) const;
    /// Check current instance is type of specified type
    template<typename T> bool IsInstanceOf() const { return IsInstanceOf(T::GetStaticTypeInfo()); }
    /// Cast the object to specified most derived class
    template<typename T> T* Cast() { return IsInstanceOf<T>() ?  static_cast<T*>(this) : nullptr; }
    /// Cast to the object to specified most derived class
    template<typename T> const T* Cast() const { return IsInstanceOf<T>() ?  static_cast<const T*>(this) : nullptr; }
    /// Return object category
    const String& GetCategory() const;

    /// Handle event
    virtual void OnEvent(Object* sender, StringHash eventType, VariantMap& eventData);
    /// Return active event sender.
    Object* GetEventSender() const;
    /// Return active event handler
    EventHandler* GetEventHandler() const;
    /// Return a preallocated map for event data
    VariantMap& GetEventDataMap() const;
    /// Subscribe to an event that can be sent by an sender
    void SubscribeToEvent(StringHash eventType, EventHandler* handler);
    /// Subscribe to a specific sender's event
    void SubscribeToEvent(Object* sender, StringHash eventType, EventHandler* handler);
    /// Subscribe to an event that can be sent by an sender
    void SubscribeToEvent(StringHash eventType, const std::function<void(StringHash, VariantMap&)>&function, void* userData= nullptr);
    /// Unsubscribe from an event
    void UnsubscribeFromEvent(StringHash eventType);
    /// Unsubscribe from a specific sender's event
    void UnsubscribeFromEvent(Object* sender, StringHash eventType);
    /// Unsubscribe from a specific sender's events
    void UnsubscribeFromEvents(Object* sender);
    /// Unsubscribe from all events
    void UnsubscribeFromAllEvents();
    /// Unsubscribe from all events except those listed, and optionally only those with userdata (script registered events)
    void UnsubscribeFromAllEventsExcept(const PODVector<StringHash>& exceptions, bool onlyUserData);
    /// Return whether has subscribed to an event without specific sender
    bool HasSubscribedToEvent(StringHash eventType) const;
    /// Return whether has subscribed to a specific sender's event
    bool HasSubscribedToEvent(Object* sender, StringHash eventType) const;
    /// Send event to all subscribers
    void SendEvent(StringHash eventType);
    /// Send event with parameters to all subscribers
    void SendEvent(StringHash eventType, VariantMap& eventData);
    /// Send event with variadic parameter pairs to all subscribers.
    template<typename... Args> void SendEvent(StringHash eventType, Args... args)
    {
        SendEvent(eventType, GetEventDataMap().Populate(args...));
    }
    /// Block object from sending and receiving events
    void SetBlockEvents(bool block) { blockEvents_ = block; }
    /// Return sending and receiving events blocking status
    bool GetBlockEvents() const { return blockEvents_; }

    /// Return execution context
    Context* GetContext() const { return context_; }

    /// Return global variable based on key
    const Variant& GetGlobalVar(StringHash key) const;
    /// Return all global variables
    const VariantMap& GetGlobalVars() const;
    /// Set global variable with the respective key an value
    void SetGlobalVar(StringHash key, const Variant& value);

    /// Return subsystem by type
    Object* GetSubsystem(StringHash type) const;
    /// Template version of returning a subsystem
    template<typename T> T* GetSubsystem() const;

protected:
    /// Execution context
    Context* context_;

private:
    /// Find the first event handler with no specific sender
    EventHandler* FindEventHandler(StringHash eventType, EventHandler** previous = nullptr) const;
    /// Find the first event handler with specific handler
    EventHandler* FindSpecificEventHandler(Object* sender, EventHandler** previous = nullptr) const;
    /// Find the first event handler with specific handler and event type
    EventHandler* FindSpecificEventHandler(Object* sender, StringHash eventType, EventHandler** previous = nullptr) const;
    /// Remove event handlers related to a specific sender
    void RemoveEventSender(Object* sender);
    /// Event handlers.
    LinkedList<EventHandler> eventHandlers_;
    /// Block object from sending and receiving any events
    bool blockEvents_;
};

template<typename T>
T* Object::GetSubsystem() const { return static_cast<T*>( GetSubsystem(T::GetTypeStatic()) );}

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

/// Internal helper class for invoking event handler functions
class MY3D_API EventHandler : public LinkedListNode
{
public:
    /// Construct with specified receiver and userdata
    explicit EventHandler(Object* receiver, void* userData = nullptr)
        : receiver_(receiver)
        , sender_(nullptr)
        , userData_(userData)
    {
    }
    /// Destruct
    virtual ~EventHandler() = default;
    /// Set sender and event type
    void SetSenderAndEventType(Object* sender, StringHash eventType)
    {
        sender_ = sender;
        eventType_ = eventType;
    }
    /// Invoke event handler function
    virtual void Invoke(VariantMap& eventData) = 0;
    /// Return a unique copy of the event handler
    virtual EventHandler* Clone() const = 0;
    /// Return event receiver
    Object* GetReceiver() const { return receiver_; }
    /// Return event sender. Null if the handler is non-specific
    Object* GetSender() const { return sender_; }
    /// Return event type
    StringHash GetEventType() const { return eventType_; }
    /// Return userdata
    void* GetUserData() const { return userData_; }

protected:
    /// Event receiver
    Object* receiver_;
    /// Event sender
    Object* sender_;
    /// Event type
    StringHash eventType_;
    /// Userdata
    void* userData_;
};

/// Template implementation of the event handler invoke helper
template <typename T> class EventHandlerImpl : public EventHandler
{
public:
    using HandlerFunctionPtr = void (T::*)(StringHash, VariantMap&);
    /// Construct with receiver and function pointers and userdata
    EventHandlerImpl(T* receiver, HandlerFunctionPtr function, void* userData = nullptr)
        : EventHandler(receiver, userData)
        , function_(function)
    {
        assert(receiver);
        assert(function);
    }
    /// Invoke event handler function
    virtual void Invoke(VariantMap& eventData) override
    {
        auto* receiver = static_cast<T*>(receiver_);
        (receiver->*function_)(eventType_, eventData);
    }
    /// Return a unique copy of the event handler
    EventHandler* Clone() const override
    {
        return new EventHandlerImpl(static_cast<T*>(receiver_), function_, userData_);
    }
private:
    /// Class-specific pointer to handler function
    HandlerFunctionPtr function_;
};

/// Lambda implementation of the event handler invoke helper
class EventHandler11Impl : public EventHandler
{
public:
    using HandlerFunctionPtr = std::function<void(StringHash, VariantMap&)>;
    /// Construct with receiver and function pointer and userdata
    explicit EventHandler11Impl(HandlerFunctionPtr function, void* userData = nullptr)
        : EventHandler(nullptr, userData)
        , function_(std::move(function))
    {
        assert(function_);
    }
    /// Invoke event handler function
    virtual void Invoke(VariantMap& eventData) override
    {
        function_(eventType_, eventData);
    }
    /// Return a unique copy of the event handler
    EventHandler* Clone() const override
    {
        return new EventHandler11Impl(function_, userData_);
    }
private:
    /// Class-specific pointer to handler function
    HandlerFunctionPtr function_;
};

/// Describe an event's hash ID and begin a namespace in which to define its parameters
#define MY3D_EVENT(eventID, eventName) static const My3D::StringHash eventID(#eventName); namespace eventName
/// Describe an event's parameter hash ID. Should be used inside an event namespace.
#define MY3D_PARAM(paramID, paramName) static const My3D::StringHash paramID(#paramName);
/// Convenience macro to construct an EventHandler that points to a receiver object and its member function
#define MY3D_HANDLER(className, function) (new My3D::EventHandlerImpl<className>(this, &className::function))
/// Convenience macro to construct an EventHandler that points to a receiver object and its member function, and also defines a userdata pointer.
#define MY3D_HANDLER_USERDATA(className, function, userData) (new My3D::EventHandlerImpl<className>(this, &className::function, userData))

}

