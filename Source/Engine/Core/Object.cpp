//
// Created by luchu on 2022/1/1.
//

#include "Core/Object.h"
#include "Core/Context.h"
#include "Core/Thread.h"
#include "IO/Log.h"
#include "Container/HashSet.h"



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
    context_->RemoveEventSender(this);
}

void Object::RemoveEventSender(Object *sender)
{
    EventHandler* handler = eventHandlers_.First();
    EventHandler* previous = nullptr;

    while (handler)
    {
        if (handler->GetSender() == sender)
        {
            EventHandler* next = eventHandlers_.Next(handler);
            eventHandlers_.Erase(handler, previous);
            handler = next;
        }
        else
        {
            previous = handler;
            handler = eventHandlers_.Next(handler);
        }
    }
}

Object* Object::GetSubSystem(StringHash type) const
{
    return context_->GetSubsystem(type);
}

void Object::OnEvent(Object *sender, StringHash eventType, VariantMap &eventData)
{
    if (blockEvents_)
        return;

    Context* context = context_;
    EventHandler* specific = nullptr;
    EventHandler* nonSpecific = nullptr;

    EventHandler* handler = eventHandlers_.First();
    while (handler)
    {
        if (handler->GetEventType() == eventType)
        {
            if (!handler->GetSender())
                nonSpecific = handler;
            else if (handler->GetSender() == sender)
            {
                specific = handler;
                specific = handler;
                break;
            }
        }

        handler = eventHandlers_.Next(handler);
    }

    // Specific event handlers have priority
    if (specific)
    {
        context->SetEventHandler(specific);
        specific->Invoke(eventData);
        context->SetEventHandler(nullptr);
        return;
    }

    if (nonSpecific)
    {
        context->SetEventHandler(nonSpecific);
        nonSpecific->Invoke(eventData);
        context->SetEventHandler(nullptr);
    }
}

bool Object::IsInstanceOf(StringHash type) const
{
    return GetTypeInfo()->IsTypeOf(type);
}

bool Object::IsInstanceOf(const TypeInfo* typeInfo) const
{
    return GetTypeInfo()->IsTypeOf(typeInfo);
}

Object* Object::GetEventSender() const
{
    return context_->GetEventSender();
}

EventHandler* Object::GetEventHandler() const
{
    return context_->GetEventHandler();
}

VariantMap& Object::GetEventDataMap() const
{
    return context_->GetEventDataMap();
}

void Object::SendEvent(StringHash eventType)
{
    VariantMap noEventData;
    SendEvent(eventType, noEventData);
}

void Object::SendEvent(StringHash eventType, VariantMap &eventData)
{
    if (!Thread::IsMainThread())
    {
        MY3D_LOGERROR("Sending events is only supported from the main thread");
        return;
    }

    if (blockEvents_)
        return;

    // Make a weak pointer to self to check for destruction during event handling
    WeakPtr<Object> self(this);
    Context* context = context_;
    HashSet<Object*> processed;

    context_->BeginSendEvent(this, eventType);

    // Check first the specific event receivers
    // Note: group is held alive with a shared ptr, as it may get destroyed along with the sender
    SharedPtr<EventReceiverGroup> group(context->GetEventReceivers(this, eventType));
    if (group)
    {
        group->BeginSendEvent();
        unsigned numReceivers = group->receivers_.Size();
        for (unsigned i = 0; i < numReceivers; ++i)
        {
            Object* receiver = group->receivers_[i];
            if (!receiver)
                continue;
            receiver->OnEvent(this, eventType, eventData);
            if (self.Expired())
            {
                group->EndSendEvent();
                context_->EndSendEvent();
                return;
            }

            processed.Insert(receiver);
        }
        group->EndSendEvent();
    }

    // Then the non-specific receivers
    group = context->GetEventReceivers(eventType);
    if (group)
    {
        group->BeginSendEvent();
        if (processed.Empty())
        {
            unsigned numReceivers = group->receivers_.Size();
            for (unsigned i = 0; i < numReceivers; ++i)
            {
                Object* receiver = group->receivers_[i];
                if (!receiver)
                    continue;
                receiver->OnEvent(this, eventType, eventData);
                if (self.Expired())
                {
                    group->EndSendEvent();
                    context_->EndSendEvent();
                    return;
                }
            }
        }
        else
        {
            unsigned numReceivers = group->receivers_.Size();
            for (unsigned i = 0; i < numReceivers; ++i)
            {
                Object* receiver = group->receivers_[i];
                if (!receiver || processed.Contains(receiver))
                    continue;
                receiver->OnEvent(this, eventType, eventData);
                if (self.Expired())
                {
                    group->EndSendEvent();
                    context_->EndSendEvent();
                    return;
                }
            }
        }

        group->EndSendEvent();
    }

    context_->EndSendEvent();
}

const String& Object::GetCategory() const
{
    return String::EMPTY;
}

void Object::SubscribeToEvent(StringHash eventType, EventHandler *handler)
{
    if (!handler)
        return;

    handler->SetSenderAndEventType(nullptr, eventType);
    // Remove old event handler first
    EventHandler* previous;
    EventHandler* oldHandler = FindSpecificEventHandler(nullptr, eventType, &previous);
    if (oldHandler)
    {
        eventHandlers_.Erase(oldHandler, previous);
        eventHandlers_.InsertFront(handler);
    }
    else
    {
        eventHandlers_.InsertFront(handler);
        context_->AddEventReceiver(this, eventType);
    }
}

void Object::SubscribeToEvent(Object *sender, StringHash eventType, EventHandler *handler)
{
    if (!sender || !handler)
    {
        delete handler;
        return;
    }

    handler->SetSenderAndEventType(sender, eventType);
    EventHandler* previous;
    EventHandler* oldHandler = FindSpecificEventHandler(sender, eventType, &previous);
    if (oldHandler)
    {
        eventHandlers_.Erase(oldHandler, previous);
        eventHandlers_.InsertFront(handler);
    }
    else
    {
        eventHandlers_.InsertFront(handler);
        context_->AddEventReceiver(this, sender, eventType);
    }
}

void Object::SubscribeToEvent(StringHash eventType,
                              const std::function<void(StringHash, VariantMap&)> &function, void *userData)
{
    SubscribeToEvent(eventType, new EventHandler11Impl(function, userData));
}

bool Object::HasSubscribedToEvent(StringHash eventType) const
{
    return FindEventHandler(eventType, nullptr) != nullptr;
}

bool Object::HasSubscribedToEvent(Object *sender, StringHash eventType) const
{
    if (!sender)
        return false;
    else
        return FindSpecificEventHandler(sender, eventType) != nullptr;
}

const Variant& Object::GetGlobalVar(StringHash key) const
{
    return context_->GetGlobalVar(key);
}

const VariantMap& Object::GetGlobalVars() const
{
    return context_->GetGlobalVars();
}

void Object::SetGlobalVar(StringHash key, const Variant &value)
{
    context_->SetGlobalVar(key, value);
}

EventHandler* Object::FindSpecificEventHandler(Object *sender, EventHandler **previous) const
{
    EventHandler* handler = eventHandlers_.First();
    if (previous)
        *previous = nullptr;

    while (handler)
    {
        if (handler->GetSender() == sender)
            return handler;
        if (previous)
            *previous = handler;
        handler = eventHandlers_.Next(handler);
    }

    return nullptr;
}

EventHandler* Object::FindSpecificEventHandler(Object *sender, StringHash eventType, EventHandler **previous) const
{
    EventHandler* handler = eventHandlers_.First();
    if (previous)
        *previous = nullptr;

    while (handler)
    {
        if (handler->GetSender() == sender && handler->GetEventType() == eventType)
            return handler;
        if (previous)
            *previous = handler;
        handler = eventHandlers_.Next(handler);
    }

    return nullptr;
}

EventHandler* Object::FindEventHandler(StringHash eventType, EventHandler **previous) const
{
    EventHandler* handler = eventHandlers_.First();
    if (previous)
        *previous = nullptr;

    while (handler)
    {
        if (handler->GetEventType() == eventType)
            return handler;
        if (previous)
            *previous = handler;
        handler = eventHandlers_.Next(handler);
    }

    return nullptr;
}

void Object::UnsubscribeFromEvent(StringHash eventType)
{
    for (;;)
    {
        EventHandler* previous;
        EventHandler* handler = FindEventHandler(eventType, &previous);
        if (handler)
        {
            if (handler->GetSender())
                context_->RemoveEventReceiver(this, handler->GetSender(), eventType);
            else
                context_->RemoveEventReceiver(this, eventType);
            eventHandlers_.Erase(handler, previous);
        }
        else
            break;
    }
}

void Object::UnsubscribeFromEvent(Object *sender, StringHash eventType)
{
    if (!sender)
        return;

    EventHandler* previous;
    EventHandler* handler = FindSpecificEventHandler(sender, eventType, &previous);
    if (handler)
    {
        context_->RemoveEventReceiver(this, handler->GetSender(),eventType);
        eventHandlers_.Erase(handler, previous);
    }
}

void Object::UnsubscribeFromEvents(Object *sender)
{
    if (!sender)
        return;

    for (;;)
    {
        EventHandler* previous;
        EventHandler* handler = FindSpecificEventHandler(sender, &previous);
        if (handler)
        {
            context_->RemoveEventReceiver(this, handler->GetSender(), handler->GetEventType());
            eventHandlers_.Erase(handler, previous);
        }
        else
            break;
    }
}

void Object::UnsubscribeFromAllEvents()
{
    for (;;)
    {
        EventHandler* handler = eventHandlers_.First();
        if (handler)
        {
            if (handler->GetSender())
                context_->RemoveEventReceiver(this, handler->GetSender(), handler->GetEventType());
            else
                context_->RemoveEventReceiver(this, handler->GetEventType());
            eventHandlers_.Erase(handler);
        }
        else
            break;
    }
}

void Object::UnsubscribeFromAllEventsExcept(const PODVector<StringHash> &exceptions, bool onlyUserData)
{
    EventHandler* previous = nullptr;
    EventHandler* handler = eventHandlers_.First();

    while (handler)
    {
        EventHandler* next = eventHandlers_.Next(handler);
        if ((!onlyUserData || handler->GetUserData()) && !exceptions.Contains(handler->GetEventType()))
        {
            if (handler->GetSender())
                context_->RemoveEventReceiver(this, handler->GetSender(), handler->GetEventType());
            else
                context_->RemoveEventReceiver(this, handler->GetEventType());
            eventHandlers_.Erase(handler, previous);
        }
        else
            previous = handler;

        handler = next;
    }
}

}

