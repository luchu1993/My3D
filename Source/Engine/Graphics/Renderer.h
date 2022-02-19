//
// Created by luchu on 2022/2/3.
//

#include "Core/Mutex.h"
#include "Container/HashSet.h"
#include "Graphics/Viewport.h"
#include "Graphics/Drawable.h"
#include "Graphics/GraphicsDefs.h"
#include "Math/Color.h"


namespace My3D
{
    class Geometry;
    class Drawable;
    class Light;
    class Material;
    class Pass;
    class Technique;
    class Octree;
    class Graphics;
    class RenderPath;
    class RenderSurface;
    class ResourceCache;
    class Scene;
    class Skeleton;
    class OcclusionBuffer;
    class Technique;
    class Texture;
    class Texture2D;
    class TextureCube;
    class View;
    class Zone;
    struct BatchQueue;

    static const int SHADOW_MIN_PIXELS = 64;
    static const int INSTANCING_BUFFER_DEFAULT_SIZE = 1024;

    /// Light vertex shader variations.
    enum LightVSVariation
    {
        LVS_DIR = 0,
        LVS_SPOT,
        LVS_POINT,
        LVS_SHADOW,
        LVS_SPOTSHADOW,
        LVS_POINTSHADOW,
        LVS_SHADOWNORMALOFFSET,
        LVS_SPOTSHADOWNORMALOFFSET,
        LVS_POINTSHADOWNORMALOFFSET,
        MAX_LIGHT_VS_VARIATIONS
    };

    /// Per-vertex light vertex shader variations.
    enum VertexLightVSVariation
    {
        VLVS_NOLIGHTS = 0,
        VLVS_1LIGHT,
        VLVS_2LIGHTS,
        VLVS_3LIGHTS,
        VLVS_4LIGHTS,
        MAX_VERTEXLIGHT_VS_VARIATIONS
    };

    /// Light pixel shader variations.
    enum LightPSVariation
    {
        LPS_NONE = 0,
        LPS_SPOT,
        LPS_POINT,
        LPS_POINTMASK,
        LPS_SPEC,
        LPS_SPOTSPEC,
        LPS_POINTSPEC,
        LPS_POINTMASKSPEC,
        LPS_SHADOW,
        LPS_SPOTSHADOW,
        LPS_POINTSHADOW,
        LPS_POINTMASKSHADOW,
        LPS_SHADOWSPEC,
        LPS_SPOTSHADOWSPEC,
        LPS_POINTSHADOWSPEC,
        LPS_POINTMASKSHADOWSPEC,
        MAX_LIGHT_PS_VARIATIONS
    };

    /// Deferred light volume vertex shader variations.
    enum DeferredLightVSVariation
    {
        DLVS_NONE = 0,
        DLVS_DIR,
        DLVS_ORTHO,
        DLVS_ORTHODIR,
        MAX_DEFERRED_LIGHT_VS_VARIATIONS
    };

    /// Deferred light volume pixels shader variations.
    enum DeferredLightPSVariation
    {
        DLPS_NONE = 0,
        DLPS_SPOT,
        DLPS_POINT,
        DLPS_POINTMASK,
        DLPS_SPEC,
        DLPS_SPOTSPEC,
        DLPS_POINTSPEC,
        DLPS_POINTMASKSPEC,
        DLPS_SHADOW,
        DLPS_SPOTSHADOW,
        DLPS_POINTSHADOW,
        DLPS_POINTMASKSHADOW,
        DLPS_SHADOWSPEC,
        DLPS_SPOTSHADOWSPEC,
        DLPS_POINTSHADOWSPEC,
        DLPS_POINTMASKSHADOWSPEC,
        DLPS_SHADOWNORMALOFFSET,
        DLPS_SPOTSHADOWNORMALOFFSET,
        DLPS_POINTSHADOWNORMALOFFSET,
        DLPS_POINTMASKSHADOWNORMALOFFSET,
        DLPS_SHADOWSPECNORMALOFFSET,
        DLPS_SPOTSHADOWSPECNORMALOFFSET,
        DLPS_POINTSHADOWSPECNORMALOFFSET,
        DLPS_POINTMASKSHADOWSPECNORMALOFFSET,
        DLPS_ORTHO,
        DLPS_ORTHOSPOT,
        DLPS_ORTHOPOINT,
        DLPS_ORTHOPOINTMASK,
        DLPS_ORTHOSPEC,
        DLPS_ORTHOSPOTSPEC,
        DLPS_ORTHOPOINTSPEC,
        DLPS_ORTHOPOINTMASKSPEC,
        DLPS_ORTHOSHADOW,
        DLPS_ORTHOSPOTSHADOW,
        DLPS_ORTHOPOINTSHADOW,
        DLPS_ORTHOPOINTMASKSHADOW,
        DLPS_ORTHOSHADOWSPEC,
        DLPS_ORTHOSPOTSHADOWSPEC,
        DLPS_ORTHOPOINTSHADOWSPEC,
        DLPS_ORTHOPOINTMASKSHADOWSPEC,
        DLPS_ORTHOSHADOWNORMALOFFSET,
        DLPS_ORTHOSPOTSHADOWNORMALOFFSET,
        DLPS_ORTHOPOINTSHADOWNORMALOFFSET,
        DLPS_ORTHOPOINTMASKSHADOWNORMALOFFSET,
        DLPS_ORTHOSHADOWSPECNORMALOFFSET,
        DLPS_ORTHOSPOTSHADOWSPECNORMALOFFSET,
        DLPS_ORTHOPOINTSHADOWSPECNORMALOFFSET,
        DLPS_ORTHOPOINTMASKSHADOWSPECNORMALOFFSET,
        MAX_DEFERRED_LIGHT_PS_VARIATIONS
    };

    /// High-level rendering subsystem. Manages drawing of 3D views.
    class MY3D_API Renderer : public Object
    {
        MY3D_OBJECT(Renderer, Object)
    public:
        using ShadowMapFilter = void(Object::*)(View* view, Texture2D* shadowMap, float blurScale);

        /// Construct.
        explicit Renderer(Context* context);
        /// Destruct.
        ~Renderer() override;

        /// Return the frame update parameters.
        const FrameInfo& GetFrameInfo() const { return frame_; }

        /// Update for rendering. Called by HandleRenderUpdate().
        void Update(float timeStep);
        /// Render. Called by Engine.
        void Render();
        /// Add debug geometry to the debug renderer.
        void DrawDebugGeometry(bool depthTest);
        /// Queue a render surface's viewports for rendering. Called by the surface, or by View.
        void QueueRenderSurface(RenderSurface* renderTarget);
        /// Queue a viewport for rendering. Null surface means backbuffer.
        void QueueViewport(RenderSurface* renderTarget, Viewport* viewport);

        /// Return whether HDR rendering is enabled.
        bool GetHDRRendering() const { return hdrRendering_; }
        /// Return whether specular lighting is enabled.
        bool GetSpecularLighting() const { return specularLighting_; }
        /// Return whether drawing shadows is enabled.
        bool GetDrawShadows() const { return drawShadows_; }
        /// Return default texture max. anisotropy level.
        int GetTextureAnisotropy() const { return textureAnisotropy_; }
        /// Return default texture filtering mode.
        TextureFilterMode GetTextureFilterMode() const { return textureFilterMode_; }
        /// Return texture quality level.
        MaterialQuality GetTextureQuality() const { return textureQuality_; }
        /// Return material quality level.
        MaterialQuality GetMaterialQuality() const { return materialQuality_; }

    private:
        /// Initialize when screen mode initially set.
        void Initialize();
        /// Handle screen mode event.
        void HandleScreenMode(StringHash eventType, VariantMap& eventData);
        /// Handle render update event.
        void HandleRenderUpdate(StringHash eventType, VariantMap& eventData);

        /// Graphics subsystem.
        WeakPtr<Graphics> graphics_;
        /// Backbuffer viewports.
        Vector<SharedPtr<Viewport>> viewports_;
        /// Render surface viewports queued for update.
        Vector<Pair<WeakPtr<RenderSurface>, WeakPtr<Viewport>>> queuedViewports_;
        /// Frame info for rendering.
        FrameInfo frame_;
        /// Texture anisotropy level.
        int textureAnisotropy_{4};
        /// Texture filtering mode.
        TextureFilterMode textureFilterMode_{FILTER_TRILINEAR};
        /// Texture quality level.
        MaterialQuality textureQuality_{QUALITY_HIGH};
        /// Material quality level.
        MaterialQuality materialQuality_{QUALITY_HIGH};
        /// HDR rendering flag.
        bool hdrRendering_{};
        /// Specular lighting flag.
        bool specularLighting_{true};
        /// Draw shadows flag.
        bool drawShadows_{true};
        /// Shadow map reuse flag.
        bool reuseShadowMaps_{true};
        /// Dynamic instancing flag.
        bool dynamicInstancing_{true};
        /// Number of extra instancing data elements.
        int numExtraInstancingBufferElements_{};
        /// Threaded occlusion rendering flag.
        bool threadedOcclusion_{};
        /// Shaders need reloading flag.
        bool shadersDirty_{true};
        /// Initialized flag.
        bool initialized_{};
        /// Flag for views needing reset.
        bool resetViews_{};
    };
}
