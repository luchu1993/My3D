#include "Context.h"
#include "IO/Log.h"
#include "Core/Thread.h"


namespace My3D
{

    static int sdlInitCounter = 0;

    Context::Context()
    {
        Thread::SetMainThread();
    }

    Context::~Context()
    {
        subsystems_.Clear();
        factories_.Clear();
    }


    SharedPtr<Object> Context::CreateObject(StringHash objectType)
    {
        auto it = factories_.Find(objectType);
        if (it != factories_.End())
            return it->second_->CreateObject();
        else
            return SharedPtr<Object>();
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

    /// Return subsystem by type
    Object* Context::GetSubsystem(StringHash type) const
    {
        auto it = subsystems_.Find(type);
        if (it != subsystems_.End())
            return it->second_;
        else
            return nullptr;
    }
}