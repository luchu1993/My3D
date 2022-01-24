#include "Launch/Application.h"
#include "Math/Vector2.h"
#include "Container/Vector.h"
#include "IO/Log.h"
#include "IO/FileSystem.h"
#include "Core/Variant.h"
#include "Core/CoreEvents.h"
#include "Input/InputEvents.h"
#include "Input/InputConstants.h"
#include "Launch/EngineDefs.h"


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
        engineParameters_[EP_WINDOW_TITLE] = GetTypeName();
        engineParameters_[EP_LOG_NAME] = GetSubsystem<FileSystem>()->GetAppPreferencesDir("my3d", "logs") + GetTypeName() + ".log";
        engineParameters_[EP_FULL_SCREEN] = false;
        engineParameters_[EP_HEADLESS] = false;

        engineParameters_[EP_WINDOW_WIDTH] = 1920;
        engineParameters_[EP_WINDOW_HEIGHT] = 1080;

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
