#include "Context.h"


namespace My3D
{
    SharedPtr<Object> Context::CreateObject(StringHash objectType)
    {

    }

    void Context::RegisterFactory(ObjectFactory* factory)
    {

    }

    void Context::RegisterFactory(ObjectFactory* factory, const char* category)
    {

    }

    void Context::RegisterSubsystem(Object* object)
    {

    }

    void Context::RemoveSubsystem(StringHash objectType)
    {

    }

    bool Context::RequireSDL(unsigned sdlFlags)
    {

    }

    void Context::ReleaseSDL()
    {

    }

    /// Return subsystem by type
    Object* Context::GetSubsystem(StringHash type) const
    {

    }
}