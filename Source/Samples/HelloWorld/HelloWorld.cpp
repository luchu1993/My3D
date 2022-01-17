#include "Launch/Application.h"
#include "Math/Vector2.h"
#include "Container/HashMap.h"
#include "Container/Vector.h"
#include "IO/Log.h"
#include "Input/Input.h"
#include "Core/Variant.h"
#include "Container/HashSet.h"
#include "Core/CoreEvents.h"

#include "SDL.h"

using namespace My3D;


class CustomKey
{
public:
    CustomKey() = default;
    explicit CustomKey(float v) : value(v) { }

    unsigned ToHash() const { return (unsigned) value; }
    bool operator ==(const CustomKey& rhs) const { return value == rhs.value; }

    float value;
};


class HelloWorld : public Application
{
    MY3D_OBJECT(HelloWorld, Application)
public:
    explicit HelloWorld(Context* context)
        : Base(context)
    {

    }

    void Setup() override
    {
        MY3D_LOGINFO("Setup My3D Engine!");
        SDL_Init(SDL_INIT_VIDEO);
        window_ = SDL_CreateWindow("HelloWorld", 200, 200, 800, 600, 0);

        SubscribeToEvent(E_UPDATE, MY3D_HANDLER(HelloWorld, HandleUpdate));

    }

    void Start() override
    {
        MY3D_LOGINFO("Start My3D Engine!");
    }

    void Stop() override
    {
        SDL_DestroyWindow(window_);
        MY3D_LOGINFO("Stop My3D Engine!");
    }

    void HandleUpdate(StringHash eventType, VariantMap& eventData)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_KEYDOWN:
                {
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        engine_->Exit();
                    }
                }
            }
        }
    }

private:
    SDL_Window* window_;
};


MY3D_DEFINE_APPLICATION_MAIN(HelloWorld)
