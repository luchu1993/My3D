//
// Created by luchu on 2022/1/18.
//

#pragma once

#include "Graphics/GraphicsDefs.h"
#include "Core/Object.h"
#include "Core/Mutex.h"
#include "Container/ArrayPtr.h"


struct SDL_Window;

namespace My3D
{
    class File;
    class Texture;
    class GraphicsImpl;
    class RenderSurface;
    class GPUObject;
    class Vector3;
    class Vector4;
    class VertexBuffer;
    class IndexBuffer;

    /// CPU-side scratch buffer for vertex data updates
    struct ScratchBuffer
    {
        ScratchBuffer() : size_(0), reserved_(false) { }

        /// Buffer data
        SharedArrayPtr<unsigned char> data_;
        /// Data size
        unsigned size_;
        /// Reserved flag
        bool reserved_;
    };

    /// Screen mode parameters
    struct ScreenModeParams
    {
        /// Whether to use fullscreen mode
        bool fullscreen_{};
        /// Whether to hide window borders. Window is always borderless in fullscreen mode
        bool borderless_{};
        /// Whether the window is resizable
        bool resizable_{};
        /// Whether the high DPI is enabled.
        bool highDPI_{};
        /// whether the vertical synchronization is used.
        bool vsync_{};
        /// Whether the triple bufferization is used.
        bool tripleBuffer_{};
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
                   && highDPI_ == rhs.highDPI_
                   // && vsync_ == rhs.vsync_
                   && tripleBuffer_ == rhs.tripleBuffer_
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
        /// Set window modes to be rotated by ToggleFullscreen. Apply primary window settings immeditally.
        /// Window may be maximized if requested and possible. Return true if successful.
        bool SetWindowModes(const WindowModeParams& windowMode, const WindowModeParams& secondaryWindowMode, bool maximize = false);
        /// Set default window modes. Return true if successful.
        bool SetDefaultWindowModes(int width, int height, const ScreenModeParams& params);
        /// Set default window modes. Deprecated. Return true if successful.
        bool SetMode(int width, int height, bool fullscreen, bool borderless, bool resizable, bool highDPI, bool vsync, bool tripleBuffer, int multiSample, int monitor, int refreshRate);
        /// Set screen resolution only. Deprecated. Return true if successful.
        bool SetMode(int width, int height);
        /// Set whether the main window uses sRGB conversion on write.
        void SetSRGB(bool enable);
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
        /// Clear any or all of render target, depth buffer and stencil buffer
        void Clear(ClearTargetFlags flags, const Color& color = Color::TRANSPARENT_BLACK, float depth = 1.0f, unsigned stencil = 0);
        void SetVertexBuffer(VertexBuffer* buffer);
        /// Set multiple vertex buffers.
        bool SetVertexBuffers(const PODVector<VertexBuffer*>& buffers, unsigned instanceOffset = 0);
        /// Set multiple vertex buffers.
        bool SetVertexBuffers(const Vector<SharedPtr<VertexBuffer> >& buffers, unsigned instanceOffset = 0);
        /// Set index buffer.
        void SetIndexBuffer(IndexBuffer* buffer);
        /// Return current rendertarget width and height.
        IntVector2 GetRenderTargetDimensions() const;
        /// Reset all rendertargets, depth-stencil surface and viewport.
        void ResetRenderTargets();
        /// Reset specific rendertarget.
        void ResetRenderTarget(unsigned index);
        /// Window was resized through user interaction. Called by Input subsystem.
        void OnWindowResized();
        /// Window was moved through user interaction. Called by Input subsystem.
        void OnWindowMoved();
        /// Maximize the window.
        void Maximize();
        /// Minimize the window.
        void Minimize();
        /// Raises window if it was minimized.
        void Raise() const;
        /// Add a GPU object to keep track of. Called by GPUObject.
        void AddGPUObject(GPUObject* object);
        /// Remove a GPU object. Called by GPUObject.
        void RemoveGPUObject(GPUObject* object);
        /// Reserve a CPU-side scratch buffer
        void* ReserveScratchBuffer(unsigned size);
        /// Free a CPU-side scratch buffer
        void FreeScratchBuffer(void* buffer);
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
        /// Return multisample mode (1 = no multisampling).
        int GetMultiSample() const { return screenParams_.multiSample_; }
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
        /// Return whether triple buffering is enabled.
        bool GetTripleBuffer() const { return screenParams_.tripleBuffer_; }
        /// Return whether teh main window using sRGB conversion on write
        bool GetSRGB() const { return sRGB_; }
        /// Return whether the GPU command buffer is flushed each frame.
        bool GetFlushGPU() const { return flushGPU_; }
        /// Return allowed screen orientations
        const String& GetOrientations() const { return orientations_; }
        /// Return whether sRGB conversion on texture sampling is supported.
        bool GetSRGBSupport() const { return sRGBSupport_; }
        /// Return whether sRGB conversion on rendertarget writing is supported.
        bool GetSRGBWriteSupport() const { return sRGBWriteSupport_; }
        /// Return supported fullscreen resolutions (third component is refreshRate). Will be empty if listing the resolutions is not supported on the platform (e.g. Web).
        PODVector<IntVector3> GetResolutions(int monitor) const;
        /// Return index of the best resolution for requested width, height and refresh rate.
        unsigned FindBestResolutionIndex(int monitor, int width, int height, int refreshRate) const;
        /// Return supported multisampling levels.
        PODVector<int> GetMultiSampleLevels() const;
        /// Return the desktop resolution.
        /// @property
        IntVector2 GetDesktopResolution(int monitor) const;
        /// Return the number of currently connected monitors.
        int GetMonitorCount() const;
        /// Returns the index of the display containing the center of the window on success or a negative error code on failure.
        int GetCurrentMonitor() const;
        /// Returns true if window is maximized or runs in full screen mode.
        bool GetMaximized() const;
        /// Return display dpi information: (hdpi, vdpi, ddpi). On failure returns zero vector.
        Vector3 GetDisplayDPI(int monitor=0) const;
        /// Return current vertex buffer by index.
        VertexBuffer* GetVertexBuffer(unsigned index) const;

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
        /// Reset cached rendering state.
        void ResetCachedState();
        /// Initialize texture unit mappings.
        void SetTextureUnitMappings();

        /// Mutex for accessing the GPU objects vector from several threads.
        Mutex gpuObjectMutex_;
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
        /// Light pre-pass rendering support flag.
        bool lightPrepassSupport_{};
        /// Deferred rendering support flag.
        bool deferredSupport_{};
        /// Anisotropic filtering support flag.
        bool anisotropySupport_{};
        /// DXT format support flag.
        bool dxtTextureSupport_{};
        /// ETC1 format support flag.
        bool etcTextureSupport_{};
        /// ETC2 format support flag.
        bool etc2TextureSupport_{};
        /// PVRTC formats support flag.
        bool pvrtcTextureSupport_{};
        /// Hardware shadow map depth compare support flag.
        bool hardwareShadowSupport_{};
        /// Instancing support flag.
        bool instancingSupport_{};
        /// sRGB conversion on read support flag.
        bool sRGBSupport_{};
        /// sRGB conversion on write support flag.
        bool sRGBWriteSupport_{};
        /// Shadow map dummy color texture format.
        unsigned dummyColorFormat_{};
        /// Shadow map depth texture format.
        unsigned shadowMapFormat_{};
        /// Shadow map 24-bit depth texture format.
        unsigned hiresShadowMapFormat_{};
        /// Vertex buffers in use.
        VertexBuffer* vertexBuffers_[MAX_VERTEX_STREAMS]{};
        /// Index buffer in use.
        IndexBuffer* indexBuffer_{};
        /// Allowed screen orientations.
        String orientations_;
        /// Graphics API name
        String apiName_;
        /// Rendertargets in use.
        RenderSurface* renderTargets_[MAX_RENDERTARGETS]{};
        /// Depth-stencil surface in use.
        RenderSurface* depthStencil_{};
        /// Viewport coordinates.
        IntRect viewport_;
        /// Largest scratch buffer request this frame.
        unsigned maxScratchBufferRequest_{};
        /// GPU objects.
        PODVector<GPUObject*> gpuObjects_;
        /// Scratch buffers.
        Vector<ScratchBuffer> scratchBuffers_;
        /// Textures in use.
        Texture* textures_[MAX_TEXTURE_UNITS]{};
        /// Texture unit mappings.
        HashMap<String, TextureUnit> textureUnits_;
    };

    /// Register Graphics library objects
    void MY3D_API RegisterGraphicsLibrary(Context* context);
}
