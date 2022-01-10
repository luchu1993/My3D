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

}

}

