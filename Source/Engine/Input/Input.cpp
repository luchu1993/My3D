//
// Created by luchu on 2022/1/23.
//

#include "SDL.h"

#include "Input/Input.h"
#include "Core/Context.h"
#include "Launch/Engine.h"
#include "Graphics/GraphicsEvents.h"
#include "Graphics/Graphics.h"
#include "Core/CoreEvents.h"
#include "IO/Log.h"

extern "C" int SDL_AddTouch(SDL_TouchID touchID, SDL_TouchDeviceType type, const char* name);

namespace My3D
{
    /// Convert SDL keycode if necessary.
    Key ConvertSDLKeyCode(int keySym, int scanCode)
    {
        if (scanCode == SCANCODE_AC_BACK)
            return KEY_ESCAPE;
        else
            return (Key)SDL_tolower(keySym);
    }

    void JoystickState::Initialize(unsigned int numButtons, unsigned int numAxes, unsigned int numHats)
    {
        buttons_.Resize(numButtons);
        buttonPress_.Resize(numButtons);
        axes_.Resize(numAxes);
        hats_.Resize(numHats);

        Reset();
    }

    void JoystickState::Reset()
    {
        for (unsigned i = 0; i < buttons_.Size(); ++i)
        {
            buttons_[i] = false;
            buttonPress_[i] = false;
        }
        for (unsigned i = 0; i < axes_.Size(); ++i)
            axes_[i] = 0.0f;
        for (unsigned i = 0; i < hats_.Size(); ++i)
            hats_[i] = HAT_CENTER;
    }

    Input::Input(Context *context)
        : Base(context)
        , mouseButtonDown_(0)
        , mouseButtonPress_(0)
        , mouseMoveWheel_(0)
        , inputScale_(Vector2::ONE)
        , windowID_(0)
        , toggleFullscreen_(true)
        , mouseVisible_(false)
        , touchEmulation_(false)
        , inputFocus_(false)
        , minimized_(false)
        , initialized_(false)
    {
        context_->RequireSDL(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);

        SubscribeToEvent(E_SCREENMODE, MY3D_HANDLER(Input, HandleScreenMode));
    }

    Input::~Input()
    {
        context_->ReleaseSDL();
    }

    void Input::Update()
    {
        assert(initialized_);

        bool mouseMoved = false;
        if (mouseMove_ != IntVector2::ZERO)
            mouseMoved = true;

        SDL_Event evt;
        while (SDL_PollEvent(&evt))
            HandleSDLEvent(&evt);
    }

    void Input::SetToggleFullscreen(bool enable)
    {

    }

    void Input::SetMouseVisible(bool enable, bool suppressEvent)
    {

    }

    void Input::SetTouchEmulation(bool enable)
    {
        if (enable != touchEmulation_)
        {
            if (enable)
            {
                // Touch emulation needs the mouse visible
                if (!mouseVisible_)
                    SetMouseVisible(true);

                // Add a virtual touch device the first time we are enabling emulated touch
                if (!SDL_GetNumTouchDevices())
                    SDL_AddTouch(0, SDL_TOUCH_DEVICE_INDIRECT_RELATIVE, "Emulated Touch");
            }
            else
                ResetTouches();

            touchEmulation_ = enable;
        }
    }

#ifdef _WIN32
// On Windows repaint while the window is actively being resized.
    int Win32_ResizingEventWatcher(void* data, SDL_Event* event)
    {
        if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED)
        {
            SDL_Window* win = SDL_GetWindowFromID(event->window.windowID);
            if (win == (SDL_Window*)data)
            {
                if (auto* ctx = (Context*)SDL_GetWindowData(win, "URHO3D_CONTEXT"))
                {
                    if (auto* graphics = ctx->GetSubsystem<Graphics>())
                    {
                        if (graphics->IsInitialized())
                        {
                            graphics->OnWindowResized();
                            ctx->GetSubsystem<Engine>()->RunFrame();
                        }
                    }
                }
            }
        }
        return 0;
    }
#endif

    void Input::Initialize()
    {
        auto* graphics = GetSubsystem<Graphics>();
        if (!graphics || !graphics->IsInitialized())
            return;

        graphics_ = graphics;

        // In external window mode only visible mouse is supported
        if (graphics_->GetExternalWindow())
            mouseVisible_ = true;

        // Set the initial activation
        initialized_ = true;

        GainFocus();

        ResetJoysticks();
        ResetState();

        SubscribeToEvent(E_BEGINFRAME, MY3D_HANDLER(Input, HandleBeginFrame));

#ifdef _WIN32
        // Register callback for resizing in order to repaint.
        if (SDL_Window* window = graphics_->GetWindow())
        {
            SDL_SetWindowData(window, "URHO3D_CONTEXT", GetContext());
            SDL_AddEventWatch(Win32_ResizingEventWatcher, window);
        }
#endif
        MY3D_LOGINFO("Initialized input");
    }

    SDL_JoystickID Input::OpenJoystick(unsigned int index)
    {
        SDL_Joystick* joystick = SDL_JoystickOpen(index);
        if (!joystick)
        {
            MY3D_LOGERRORF("Cannot open joystick #%d", index);
            return -1;
        }

        // Create joystick state for the new joystick
        int joystickID = SDL_JoystickInstanceID(joystick);
        JoystickState& state = joysticks_[joystickID];
        state.joystick_ = joystick;
        state.joystickID_ = joystickID;
        state.name_ = SDL_JoystickName(joystick);
        if (SDL_IsGameController(index))
            state.controller_ = SDL_GameControllerOpen(index);

        auto numButtons = (unsigned)SDL_JoystickNumButtons(joystick);
        auto numAxes = (unsigned)SDL_JoystickNumAxes(joystick);
        auto numHats = (unsigned)SDL_JoystickNumHats(joystick);

        // When the joystick is a controller, make sure there's enough axes & buttons for the standard controller mappings
        if (state.controller_)
        {
            if (numButtons < SDL_CONTROLLER_BUTTON_MAX)
                numButtons = SDL_CONTROLLER_BUTTON_MAX;
            if (numAxes < SDL_CONTROLLER_AXIS_MAX)
                numAxes = SDL_CONTROLLER_AXIS_MAX;
        }

        state.Initialize(numButtons, numAxes, numHats);

        return joystickID;
    }

    void Input::ResetJoysticks()
    {
        joysticks_.Clear();

        // Open each detected joystick automatically on startup
        auto size = static_cast<unsigned>(SDL_NumJoysticks());
        for (unsigned i = 0; i < size; ++i)
            OpenJoystick(i);
    }

    void Input::GainFocus()
    {

    }

    void Input::LoseFocus()
    {

    }

    void Input::ResetState()
    {
        keyDown_.Clear();
        keyPress_.Clear();
        scancodeDown_.Clear();
        scancodePress_.Clear();

        /// \todo Check if resetting joystick state on input focus loss is even necessary
        for (HashMap<SDL_JoystickID, JoystickState>::Iterator i = joysticks_.Begin(); i != joysticks_.End(); ++i)
            i->second_.Reset();

        ResetTouches();

        // Use SetMouseButton() to reset the state so that mouse events will be sent properly
        SetMouseButton(MOUSEB_LEFT, false, 0);
        SetMouseButton(MOUSEB_RIGHT, false, 0);
        SetMouseButton(MOUSEB_MIDDLE, false, 0);

        mouseMove_ = IntVector2::ZERO;
        mouseMoveWheel_ = 0;
        mouseButtonPress_ = MOUSEB_NONE;
    }

    void Input::ResetTouches()
    {

    }

    bool Input::GetQualifierDown(Qualifier qualifier) const
    {
        if (qualifier == QUAL_SHIFT)
            return GetKeyDown(KEY_LSHIFT) || GetKeyDown(KEY_RSHIFT);
        if (qualifier == QUAL_CTRL)
            return GetKeyDown(KEY_LCTRL) || GetKeyDown(KEY_RCTRL);
        if (qualifier == QUAL_ALT)
            return GetKeyDown(KEY_LALT) || GetKeyDown(KEY_RALT);

        return false;
    }

    QualifierFlags Input::GetQualifiers() const
    {
        QualifierFlags ret;
        if (GetQualifierDown(QUAL_SHIFT))
            ret |= QUAL_SHIFT;
        if (GetQualifierDown(QUAL_CTRL))
            ret |= QUAL_CTRL;
        if (GetQualifierDown(QUAL_ALT))
            ret |= QUAL_ALT;

        return ret;
    }

    void Input::SetMouseButton(MouseButton button, bool newState, int clicks)
    {
        if (newState)
        {
            if (!(mouseButtonDown_ & button))
                mouseButtonPress_ |= button;

            mouseButtonDown_ |= button;
        }
        else
        {
            if (!(mouseButtonDown_ & button))
                return;

            mouseButtonDown_ &= ~button;
        }

        using namespace MouseButtonDown;

        VariantMap& eventData = GetEventDataMap();
        eventData[P_BUTTON] = button;
        eventData[P_BUTTONS] = (unsigned)mouseButtonDown_;
        eventData[P_QUALIFIERS] = (unsigned)GetQualifiers();
        eventData[P_CLICKS] = clicks;
        SendEvent(newState ? E_MOUSEBUTTONDOWN : E_MOUSEBUTTONUP, eventData);
    }

    bool Input::GetKeyDown(Key key) const
    {
        return keyDown_.Contains(SDL_tolower(key));
    }

    bool Input::GetKeyPress(Key key) const
    {
        return keyPress_.Contains(SDL_tolower(key));
    }

    void Input::SetKey(Key key, Scancode scancode, bool newState)
    {
        bool repeat = false;

        if (newState)
        {
            scancodeDown_.Insert(scancode);
            scancodePress_.Insert(scancode);

            if (!keyDown_.Contains(key))
            {
                keyDown_.Insert(key);
                keyPress_.Insert(key);
            }
            else
                repeat = true;
        }
        else
        {
            scancodeDown_.Erase(scancode);

            if (!keyDown_.Erase(key))
                return;
        }

        using namespace KeyDown;

        VariantMap& eventData = GetEventDataMap();
        eventData[P_KEY] = key;
        eventData[P_SCANCODE] = scancode;
        eventData[P_BUTTONS] = (unsigned)mouseButtonDown_;
        eventData[P_QUALIFIERS] = (unsigned)GetQualifiers();
        if (newState)
            eventData[P_REPEAT] = repeat;
        SendEvent(newState ? E_KEYDOWN : E_KEYUP, eventData);

        if ((key == KEY_RETURN || key == KEY_RETURN2 || key == KEY_KP_ENTER) && newState && !repeat && toggleFullscreen_ &&
            (GetKeyDown(KEY_LALT) || GetKeyDown(KEY_RALT)))
            graphics_->ToggleFullscreen();
    }

    void Input::HandleScreenMode(StringHash eventType, VariantMap &eventData)
    {
        if (!initialized_)
            Initialize();

        SDL_Window* window = graphics_->GetWindow();
        windowID_ = SDL_GetWindowID(window);
    }

    void Input::HandleBeginFrame(StringHash eventType, VariantMap &eventData)
    {
        // Update input right at the beginning of the frame
        SendEvent(E_INPUTBEGIN);
        Update();
        SendEvent(E_INPUTEND);
    }

    void Input::HandleSDLEvent(void *sdlEvent)
    {
        SDL_Event& evt = *static_cast<SDL_Event*>(sdlEvent);

        // Possibility for custom handling or suppression of default handling for the SDL event
        {
            using namespace SDLRawInput;

            VariantMap eventData = GetEventDataMap();
            eventData[P_SDLEVENT] = &evt;
            eventData[P_CONSUMED] = false;
            SendEvent(E_SDLRAWINPUT, eventData);

            if (eventData[P_CONSUMED].GetBool())
                return;
        }

        switch (evt.type)
        {
            case SDL_KEYDOWN:
                SetKey(ConvertSDLKeyCode(evt.key.keysym.sym, evt.key.keysym.scancode), (Scancode)evt.key.keysym.scancode, true);
                break;

            case SDL_KEYUP:
                SetKey(ConvertSDLKeyCode(evt.key.keysym.sym, evt.key.keysym.scancode), (Scancode)evt.key.keysym.scancode, false);
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (!touchEmulation_)
                {
                    const auto mouseButton = static_cast<MouseButton>(1u << (evt.button.button - 1u));  // NOLINT(misc-misplaced-widening-cast)
                    SetMouseButton(mouseButton, true, evt.button.clicks);
                }
                else
                {
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    x = (int)(x * inputScale_.x_);
                    y = (int)(y * inputScale_.y_);

                    SDL_Event event;
                    event.type = SDL_FINGERDOWN;
                    event.tfinger.touchId = 0;
                    event.tfinger.fingerId = evt.button.button - 1;
                    event.tfinger.pressure = 1.0f;
                    event.tfinger.x = (float)x / (float)graphics_->GetWidth();
                    event.tfinger.y = (float)y / (float)graphics_->GetHeight();
                    event.tfinger.dx = 0;
                    event.tfinger.dy = 0;
                    SDL_PushEvent(&event);
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (!touchEmulation_)
                {
                    const auto mouseButton = static_cast<MouseButton>(1u << (evt.button.button - 1u));  // NOLINT(misc-misplaced-widening-cast)
                    SetMouseButton(mouseButton, false, evt.button.clicks);
                }
                else
                {
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    x = (int)(x * inputScale_.x_);
                    y = (int)(y * inputScale_.y_);

                    SDL_Event event;
                    event.type = SDL_FINGERUP;
                    event.tfinger.touchId = 0;
                    event.tfinger.fingerId = evt.button.button - 1;
                    event.tfinger.pressure = 0.0f;
                    event.tfinger.x = (float)x / (float)graphics_->GetWidth();
                    event.tfinger.y = (float)y / (float)graphics_->GetHeight();
                    event.tfinger.dx = 0;
                    event.tfinger.dy = 0;
                    SDL_PushEvent(&event);
                }
                break;
        }
    }
}