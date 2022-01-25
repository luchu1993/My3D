
#include "SDL.h"

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
            if (SDL_Init(0) != 0)
            {
                MY3D_LOGERRORF("Failed to initialise SDL: %s", SDL_GetError());
                return false;
            }
        }

        Uint32 remainingFlags = sdlFlags & ~SDL_WasInit(0);
        if (remainingFlags != 0)
        {
            if (SDL_InitSubSystem(remainingFlags) != 0)
            {
                MY3D_LOGERRORF("Failed to initialise SDL subsystem: %s", SDL_GetError());
                return false;
            }
        }

        return true;
    }

    void Context::ReleaseSDL()
    {
        --sdlInitCounter;
        if (sdlInitCounter == 0)
        {
            MY3D_LOGDEBUG("Quitting SDL");
            SDL_QuitSubSystem(SDL_INIT_EVERYTHING);
            SDL_Quit();
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

    AttributeHandle Context::RegisterAttribute(StringHash objectType, const AttributeInfo& attr)
    {
        // None or pointer types can not be supported
        if (attr.type_ == VAR_NONE || attr.type_ == VAR_VOIDPTR || attr.type_ == VAR_PTR
            || attr.type_ == VAR_CUSTOM_HEAP || attr.type_ == VAR_CUSTOM_STACK)
        {
            MY3D_LOGWARNING("Attempt to register unsupported attribute type " + Variant::GetTypeName(attr.type_) + " to class " +
                              GetTypeName(objectType));
            return AttributeHandle();
        }

        AttributeHandle handle;

        Vector<AttributeInfo>& objectAttributes = attributes_[objectType];
        objectAttributes.Push(attr);
        handle.attributeInfo_ = &objectAttributes.Back();

        if (attr.mode_ & AM_NET)
        {
            Vector<AttributeInfo>& objectNetworkAttributes = networkAttributes_[objectType];
            objectNetworkAttributes.Push(attr);
            handle.networkAttributeInfo_ = &objectNetworkAttributes.Back();
        }
        return handle;
    }

    void RemoveNamedAttribute(HashMap<StringHash, Vector<AttributeInfo>>& attributes, StringHash objectType, const char* name)
    {
        auto it = attributes.Find(objectType);
        if (it == attributes.End())
            return;

        Vector<AttributeInfo>& infos = it->second_;
        for (auto i = infos.Begin(); i != infos.End(); ++i)
        {
            if (!i->name_.Compare(name, true))
            {
                infos.Erase(i);
                break;
            }
        }

        // If the vector became empty, erase the object type from the map
        if (infos.Empty())
            attributes.Erase(it);
    }

    void Context::RemoveAttribute(StringHash objectType, const char *name)
    {
        RemoveNamedAttribute(attributes_, objectType, name);
        RemoveNamedAttribute(networkAttributes_, objectType, name);
    }

    void Context::RemoveAllAttributes(StringHash objectType)
    {
        attributes_.Erase(objectType);
        networkAttributes_.Erase(objectType);
    }

    void Context::UpdateAttributeDefaultValue(StringHash objectType, const char *name, const Variant &defaultValue)
    {
        AttributeInfo* info = GetAttribute(objectType, name);
        if (info)
            info->defaultValue_ = defaultValue;
    }

    void Context::CopyBaseAttributes(StringHash baseType, StringHash derivedType)
    {
        // Prevent endless loop if mistakenly copying attributes from same class as derived
        if (baseType == derivedType)
        {
            MY3D_LOGWARNING("Attempt to copy base attributes to itself for class " + GetTypeName(baseType));
            return;
        }

        const Vector<AttributeInfo>* baseAttributes = GetAttributes(baseType);
        if (baseAttributes)
        {
            for (unsigned i = 0; i < baseAttributes->Size(); ++i)
            {
                const AttributeInfo& attr = baseAttributes->At(i);
                attributes_[derivedType].Push(attr);
                if (attr.mode_ & AM_NET)
                    networkAttributes_[derivedType].Push(attr);
            }
        }
    }

    AttributeInfo *Context::GetAttribute(StringHash objectType, const char *name)
    {
        auto it = attributes_.Find(objectType);
        if (it == attributes_.End())
            return nullptr;

        Vector<AttributeInfo>& infos = it->second_;

        for (auto& info : infos)
        {
            if (!info.name_.Compare(name, true))
                return &info;
        }

        return nullptr;
    }
}