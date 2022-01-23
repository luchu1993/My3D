//
// Created by luchu on 2022/1/11.
//

#pragma once

#include "Core/Object.h"
#include "Core/Mutex.h"
#include "Container/FlagSet.h"
#include "Container/HashSet.h"
#include "Container/List.h"
#include "Input/InputEvents.h"
#include "Input/InputConstants.h"


namespace My3D
{
class Deserializer;
class Graphics;
class Serializer;

/// Input state for a joystick
struct JoystickState
{
    /// Initialize the number of buttons, axes and hats and set them to neutral state.
    void Initialize(unsigned numButtons, unsigned numAxes, unsigned numHats);
    /// Reset button, axis and hat states to neutral.
    void Reset();

    /// Return whether is a game controller. Game controllers will use standardized axis and button mappings.
    bool IsController() const { return controller_ != nullptr; }
    /// Return number of buttons.
    unsigned GetNumButtons() const { return buttons_.Size(); }
    /// Return number of axes.
    unsigned GetNumAxes() const { return axes_.Size(); }
    /// Return number of hats.
    unsigned GetNumHats() const { return hats_.Size(); }
    /// Check if a button is held down.
    bool GetButtonDown(unsigned index) const { return index < buttons_.Size() ? buttons_[index] : false; }
    /// Check if a button has been pressed on this frame.
    bool GetButtonPress(unsigned index) const { return index < buttonPress_.Size() ? buttonPress_[index] : false; }
    /// Return axis position.
    float GetAxisPosition(unsigned index) const { return index < axes_.Size() ? axes_[index] : 0.0f; }
    /// Return hat position.
    int GetHatPosition(unsigned index) const { return index < hats_.Size() ? hats_[index] : int(HAT_CENTER); }

    /// SDL joystick.
    SDL_Joystick* joystick_{};
    /// SDL joystick instance ID.
    SDL_JoystickID joystickID_{};
    /// SDL game controller.
    SDL_GameController* controller_{};
    /// Joystick name.
    String name_;
    /// Button up/down state.
    PODVector<bool> buttons_;
    /// Button pressed on this frame.
    PODVector<bool> buttonPress_;
    /// Axis position from -1 to 1.
    PODVector<float> axes_;
    /// POV hat bits.
    PODVector<int> hats_;
};

/// Input subsystem. Converts operating system window messages to input state and events.
class MY3D_API Input : public Object
{
    MY3D_OBJECT(Input, Object)

public:
    /// Construct
    explicit Input(Context* context);
    /// Destruct
    ~Input() override;
    /// Poll for window message. Called by HandleBeginFrame()
    void Update();
    /// Set whether ALT-ENTER fullscreen toggle is enabled.
    void SetToggleFullscreen(bool enable);
    /// Set whether the operating system mouse cursor is visible. When not visible (default), is kept centered to prevent leaving the window. Mouse visibility event can be suppressed-- this also recalls any unsuppressed SetMouseVisible which can be returned by ResetMouseVisible().
    void SetMouseVisible(bool enable, bool suppressEvent = false);
    /// Set touch emulation by mouse. Only available on desktop platforms. When enabled, actual mouse events are no longer sent and the mouse cursor is forced visible.
    void SetTouchEmulation(bool enable);

private:
    /// Initialize when screen mode initially set.
    void Initialize();
    /// Open a joystick and return its ID. Return -1 if no joystick.
    SDL_JoystickID OpenJoystick(unsigned index);
    /// Setup internal joystick structures.
    void ResetJoysticks();
    /// Prepare input state for application gaining input focus.
    void GainFocus();
    /// Prepare input state for application losing input focus.
    void LoseFocus();
    /// Clear input state.
    void ResetState();
    /// Clear touch states and send touch end events.
    void ResetTouches();
    /// Handle a mouse button change.
    void SetMouseButton(MouseButton button, bool newState, int clicks);
    /// Handle a key change.
    void SetKey(Key key, Scancode scancode, bool newState);
    /// Check if a key is held down.
    bool GetKeyDown(Key key) const;
    /// Check if a key has been pressed on this frame.
    bool GetKeyPress(Key key) const;
    /// Check if a key is held down by sca
    /// Check if a mouse button has been pressed on this frame.
    bool GetMouseButtonPress(MouseButtonFlags button) const;
    /// Check if a qualifier key is held down.
    bool GetQualifierDown(Qualifier qualifier) const;
    /// Return the currently held down qualifiers.
    QualifierFlags GetQualifiers() const;
    /// Handle screen mode event.
    void HandleScreenMode(StringHash eventType, VariantMap& eventData);
    /// Handle frame start event.
    void HandleBeginFrame(StringHash eventType, VariantMap& eventData);
    /// Handle touch events from the controls of screen joystick(s).
    void HandleScreenJoystickTouch(StringHash eventType, VariantMap& eventData);
    /// Handle SDL event.
    void HandleSDLEvent(void* sdlEvent);

    /// Graphics subsystem.
    WeakPtr<Graphics> graphics_;
    /// Touch emulation mode flag.
    bool touchEmulation_;
    /// Key down state.
    HashSet<int> keyDown_;
    /// Key pressed state.
    HashSet<int> keyPress_;
    /// Key down state by scancode.
    HashSet<int> scancodeDown_;
    /// Key pressed state by scancode.
    HashSet<int> scancodePress_;
    /// Opened joysticks.
    HashMap<SDL_JoystickID, JoystickState> joysticks_;
    /// Mouse buttons' down state.
    MouseButtonFlags mouseButtonDown_;
    /// Mouse buttons' pressed state.
    MouseButtonFlags mouseButtonPress_;
    /// Operating system mouse cursor visible flag.
    bool mouseVisible_;
    /// Mouse movement since last frame.
    IntVector2 mouseMove_;
    /// Mouse wheel movement since last frame.
    int mouseMoveWheel_;
    /// Input coordinate scaling. Non-unity when window and backbuffer have different sizes (e.g. Retina display).
    Vector2 inputScale_;
    /// SDL window ID.
    unsigned windowID_;
    /// Fullscreen toggle flag.
    bool toggleFullscreen_;
    /// Input focus flag.
    bool inputFocus_;
    /// Minimized flag.
    bool minimized_;
    /// Initialized flag.
    bool initialized_;
};
}