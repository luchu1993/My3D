#include "Launch/Application.h"
#include "Math/Vector2.h"
#include "Container/Vector.h"
#include "IO/Log.h"
#include "Core/Variant.h"
#include "Core/CoreEvents.h"
#include "Input/InputEvents.h"
#include "Input/InputConstants.h"


using namespace My3D;


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
        MY3D_LOGINFO("[HelloWorld] Setup My3D Engine!");
    }

    void Start() override
    {
        MY3D_LOGINFO("[HelloWorld] Start My3D Engine!");

        SubscribeToEvent(E_KEYDOWN, MY3D_HANDLER(HelloWorld, HandleKeyDown));
    }

    void Stop() override
    {
        MY3D_LOGINFO("[HelloWorld] Stop My3D Engine!");
    }

    void HandleKeyDown(StringHash eventType, VariantMap& eventData)
    {
        using namespace KeyUp;
        int key = eventData[P_KEY].GetInt();

        // Close console (if open) or exit when ESC is pressed
        if (key == KEY_ESCAPE)
        {
            engine_->Exit();
        }
    }
};


MY3D_DEFINE_APPLICATION_MAIN(HelloWorld)
