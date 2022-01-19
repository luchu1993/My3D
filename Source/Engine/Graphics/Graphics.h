//
// Created by luchu on 2022/1/18.
//

#pragma once

#include "Core/Object.h"

struct SDL_Window;

namespace My3D
{
    class GraphicsImpl;

    /// Screen mode parameters
    struct ScreenModeParams
    {
        /// Whether to use fullscreen mode
        bool fullscreen_{};
        /// Whether to hide window borders. Window is always borderless in fullscreen mode
        bool borderless_{};
        /// Whether the window is resizable
        bool resizable_{};
        /// whether the vertical synchronization is used.
        bool vsync_{};
        /// Level of multi-sampling
        int multiSample_{1};
        /// Monitor for fullscreen mode.
        int monitor_{};
        /// Refresh rate. 0 to pick automatically
        int refreshRate_{};
        /// Compare contents except vsync flag.
        bool EqualsExceptVSync(const ScreenModeParams& rhs) const
        {
            return fullscreen_ == rhs.fullscreen_
                   && borderless_ == rhs.borderless_
                   && resizable_ == rhs.resizable_
                   // && vsync_ == rhs.vsync_
                   && multiSample_ == rhs.multiSample_
                   && monitor_ == rhs.monitor_
                   && refreshRate_ == rhs.refreshRate_;
        }

        /// Compare for equality with another parameter set.
        bool operator ==(const ScreenModeParams& rhs) const
        {
            return vsync_ == rhs.vsync_ && EqualsExceptVSync(rhs);
        }

        /// Compare for inequality with another parameter set.
        bool operator !=(const ScreenModeParams& rhs) const { return !(*this == rhs); }
    };

    /// Window mode parameters.
    struct WindowModeParams
    {
        /// Width of the window. 0 to pick automatically.
        int width_{};
        /// Height of the window. 0 to pick automatically.
        int height_{};
        /// Screen mode parameters.
        ScreenModeParams screenParams_;
    };

    class MY3D_API Graphics : public Object
    {
        MY3D_OBJECT(Graphics, Object)

    public:
        /// Construct
        explicit Graphics(Context* context);
        /// Destruct
        ~Graphics() override;

        /// Set external window handle.
        void SetExternalWindow(void* window);
        /// Set window title
        void SetWindowTitle(const String& windowTitle);
        /// Set window position
        void SetWindowPosition(const IntVector2& position);
        /// Set windows position
        void SetWindowPosition(int x, int y);
        /// Set screen mode
        bool SetScreenMode(int width, int height, const ScreenModeParams& params, bool maximize = false);
        /// Set screen mode
        bool SetScreenMode(int width, int height);
        /// Set whether to flush the GPU command buffer to prevent multiple frames being queued and uneven frame timesteps. Default off, may decrease performance if enabled. Not currently implemented on OpenGL.
        void SetFlushGPU(bool enable);
        /// Toggle between full screen and windowed mode.
        bool ToggleFullscreen();
        /// Close the window
        void Close();
        /// Begin frame rendering.
        bool BeginFrame();
        /// End frame rendering and swap buffers.
        void EndFrame();
        /// Maximize the window.
        void Maximize();
        /// Minimize the window.
        void Minimize();
        /// Raises window if it was minimized.
        void Raise() const;
        /// Return whether rendering initialized
        bool IsInitialized() const;
        /// Return graphics implementation, which holds the actual API-specific resources
        GraphicsImpl* GetImpl() const { return impl_; }
        /// Return OS-specific external window handle.
        void* GetExternalWindow() const { return externalWindow_; }
        /// Return SDL window
        SDL_Window* GetWindow() const { return window_; }
        /// Return window title
        const String& GetWindowTitle() const { return windowTitle_; }
        /// Return graphics API name
        const String& GetApiName() const { return apiName_; }
        /// Return window position
        IntVector2 GetWindowPosition() const;
        /// Return window width in pixels
        int GetWidth() const { return width_; }
        /// Return window height in pixels
        int GetHeight() const { return height_; }
        /// Return screen mode parameters
        const ScreenModeParams& GetScreenModeParams() const { return screenParams_; }
        /// Return window size in pixels
        IntVector2 GetSize() const { return IntVector2(width_, height_); }
        /// Return whether window is fullscreen
        bool GetFullScreen() const { return screenParams_.fullscreen_; }
        /// Return whether window is borderless
        bool GetBorderless() const { return screenParams_.borderless_; }
        /// Return whether window is resizable
        bool GetResizable() const { return screenParams_.resizable_; }
        /// Return whether window is high DPI
        bool GetHighDPI() const { return false; }
        /// Return whether vertical sync is on
        bool GetVSync() const { return screenParams_.vsync_; }
        /// Return refresh rate when using vsync in fullscreen
        int GetRefreshRate() const { return screenParams_.refreshRate_; }
        /// Return the current monitor index
        int GetMonitor() const { return screenParams_.monitor_; }
        /// Return whether teh main window using sRGB conversion on write
        bool GetSRGB() const { return sRGB_; }
        /// Return allowed screen orientations
        const String& GetOrientations() const { return orientations_; }
        /// Return supported fullscreen resolutions (third component is refreshRate). Will be empty if listing the resolutions is not supported on the platform (e.g. Web).
        PODVector<IntVector3> GetResolutions(int monitor) const;
        /// Return index of the best resolution for requested width, height and refresh rate.
        unsigned FindBestResolutionIndex(int monitor, int width, int height, int refreshRate) const;
        /// Return supported multisampling levels.
        PODVector<int> GetMultiSampleLevels() const;

    private:
        /// Create the application window.
        bool OpenWindow(int width, int height, bool resizable, bool borderless);
        /// Adjust parameters according to the platform.
        void AdjustScreenMode(int& newWidth, int& newHeight, ScreenModeParams& params, bool& maximize) const;
        /// Called when scree mode is successfully changed by the backend
        void OnScreenModeChanged();
        /// Adjust the window for new resolution and fullscreen mode
        void AdjustWindow(int& newWidth, int& newHeight, bool& newFullscreen, bool& newBorderless, int& monitor);
        /// Create Direct3D11 device and swap chain.
        bool CreateDevice(int width, int height);
        /// Update Direct3D11 swap chain state for a new mode and create views for the back-buffer and default depth buffer.
        bool UpdateSwapChain(int width, int height);
        /// Check supported rendering features.
        void CheckFeatureSupport();
        /// Implementation
        GraphicsImpl* impl_;
        /// SDL window
        SDL_Window* window_{};
        /// Window title
        String windowTitle_;
        /// External window, null if not in use.
        void* externalWindow_{};
        /// Most recently applied window mode.
        WindowModeParams primaryWindowMode_;
        /// Secondary window mode to be applied on Graphics::ToggleFullscreen
        WindowModeParams secondaryWindowMode_;
        /// Window width in pixels
        int width_{};
        /// Window height in pixels
        int height_{};
        /// Window position
        IntVector2 position_;
        /// Screen mode parameters
        ScreenModeParams screenParams_;
        /// Flush GPU command buffer flag.
        bool flushGPU_{};
        /// sRGB conversion on write flag for teh main window
        bool sRGB_{};
        /// Allowed screen orientations.
        String orientations_;
        /// Graphics API name
        String apiName_;
    };

    /// Register Graphics library objects
    void MY3D_API RegisterGraphicsLibrary(Context* context);
}
