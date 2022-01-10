#include "Context.h"
#include "IO/Log.h"
#include "Core/Thread.h"


namespace My3D
{
    static int sdlInitCounter = 0;

    void EventReceiverGroup::BeginSendEvent()
    {
        ++inSend_;
    }

    void EventReceiverGroup::EndSendEvent()
    {
        assert(inSend_ > 0);
        --inSend_;

        if (inSend_ == 0 && dirty_)
        {
            auto it = receivers_.Begin();
            while (it != receivers_.End())
            {
                if (*it == nullptr)
                {
                    it = receivers_.Erase(it);
                }
                else
                    ++it;
            }

            dirty_ = false;
        }
    }

    void EventReceiverGroup::Add(Object *object)
    {
        if (object)
            receivers_.Push(object);
    }

    void EventReceiverGroup::Remove(Object *object)
    {
        if (inSend_ > 0)
        {
            auto it = receivers_.Find(object);
            if (it != receivers_.End())
            {
                (*it) = nullptr;
                dirty_ = true;
            }
        }
        else
            receivers_.Remove(object);
    }

    Context::Context()
        : eventHandler_(nullptr)
    {
        Thread::SetMainThread();
    }

    Context::~Context()
    {
        subsystems_.Clear();
        factories_.Clear();

        // Delete allocated event data maps
        for (auto it = eventDataMaps_.Begin(); it != eventDataMaps_.End(); ++it)
            delete *it;
        eventDataMaps_.Clear();
    }

    bool Context::RequireSDL(unsigned sdlFlags)
    {
        ++sdlInitCounter;
        if (sdlInitCounter == 1)
        {
            MY3D_LOGDEBUG("Initialize SDL");
        }

        return true;
    }

    void Context::ReleaseSDL()
    {
        --sdlInitCounter;
        if (sdlInitCounter == 0)
        {
            MY3D_LOGDEBUG("Quitting SDL");
        }

        if (sdlInitCounter < 0)
            MY3D_LOGERROR("Too many calls to Context::ReleaseSDL");
    }

    SharedPtr<Object> Context::CreateObject(StringHash objectType)
    {
        auto it = factories_.Find(objectType);
        if (it != factories_.End())
            return it->second_->CreateObject();
        else
            return SharedPtr<Object>();
    }

    const String& Context::GetTypeName(StringHash objectType) const
    {
        auto it = factories_.Find(objectType);
        return it != factories_.End() ? it->second_->GetTypeName() : String::EMPTY;
    }

    void Context::RegisterFactory(ObjectFactory* factory)
    {
        if (!factory)
            return;
        factories_[factory->GetType()] = factory;
    }

    void Context::RegisterFactory(ObjectFactory* factory, const char* category)
    {
        if (!factory)
            return;

        RegisterFactory(factory);
        if (String::CStringLength(category))
            objectCategories_[category].Push(factory->GetType());
    }

    void Context::RegisterSubsystem(Object* object)
    {
        if (!object)
            return;

        subsystems_[object->GetType()] = object;
    }

    void Context::RemoveSubsystem(StringHash objectType)
    {
        auto it = subsystems_.Find(objectType);
        if (it != subsystems_.End())
            subsystems_.Erase(it);
    }

    Object* Context::GetSubsystem(StringHash type) const
    {
        auto it = subsystems_.Find(type);
        if (it != subsystems_.End())
            return it->second_;
        else
            return nullptr;
    }

    const Variant& Context::GetGlobalVar(StringHash key) const
    {
        auto it = globalVars_.Find(key);
        return it != globalVars_.End() ? it->second_ : Variant::EMPTY;
    }

    void Context::SetGlobalVar(StringHash key, const Variant& value)
    {
        globalVars_[key] = value;
    }

    VariantMap& Context::GetEventDataMap()
    {
        unsigned nestingLevel = eventSenders_.Size();
        while (eventDataMaps_.Size() < nestingLevel + 1)
        {
            eventDataMaps_.Push(new VariantMap());
        }

        VariantMap& ret = *eventDataMaps_[nestingLevel];
        ret.Clear();
        return ret;
    }

    Object* Context::GetEventSender() const
    {
        if (!eventSenders_.Empty())
            return eventSenders_.Back();
        else
            return nullptr;
    }

    void Context::AddEventReceiver(Object *receiver, StringHash eventType)
    {
        auto& group = eventReceivers_[eventType];
        if (!group)
            group = new EventReceiverGroup();
        group->Add(receiver);
    }

    void Context::AddEventReceiver(Object *receiver, Object *sender, StringHash eventType)
    {
        auto& group = specificEventReceivers_[sender][eventType];
        if (!group)
            group = new EventReceiverGroup();
        group->Add(receiver);
    }

    void Context::RemoveEventSender(Object *sender)
    {
        auto i = specificEventReceivers_.Find(sender);
        if (i != specificEventReceivers_.End())
        {
            for (const auto& receivers : i->second_)
            {
                for (const auto &receiver : receivers.second_->receivers_)
                {
                    if (receiver)
                        receiver->RemoveEventSender(sender);
                }
            }
            specificEventReceivers_.Erase(i);
        }
    }

    void Context::RemoveEventReceiver(Object *receiver, StringHash eventType)
    {
        EventReceiverGroup* group = GetEventReceivers(eventType);
        if (group)
            group->Remove(receiver);
    }

    void Context::RemoveEventReceiver(Object *receiver, Object *sender, StringHash eventType)
    {
        EventReceiverGroup* group = GetEventReceivers(sender, eventType);
        if (group)
            group->Remove(receiver);
    }

    void Context::BeginSendEvent(Object *sender, StringHash eventType)
    {
        eventSenders_.Push(sender);
    }

    void Context::EndSendEvent()
    {
        eventSenders_.Pop();
    }
}