//
// Created by luchu on 2022/2/25.
//

#pragma once

#include "Container/HashSet.h"
#include "Container/List.h"
#include "Graphics/Batch.h"
#include "Graphics/Light.h"


namespace My3D
{

    class Camera;
    class DebugRenderer;
    class Light;
    class Drawable;
    class Graphics;
    class Octree;
    class Renderer;
    class RenderPath;
    class RenderSurface;
    class Technique;
    class Texture2D;
    class Viewport;
    class Zone;
    struct RenderPathCommand;
    struct WorkItem;

    /// Intermediate light processing result.
    struct LightQueryResult
    {
        /// Light.
        Light* light_;
        /// Lit geometries.
        PODVector<Drawable*> litGeometries_;
        /// Shadow casters.
        PODVector<Drawable*> shadowCasters_;
        /// Shadow cameras.
        Camera* shadowCameras_[MAX_LIGHT_SPLITS];
        /// Shadow caster start indices.
        unsigned shadowCasterBegin_[MAX_LIGHT_SPLITS];
        /// Shadow caster end indices.
        unsigned shadowCasterEnd_[MAX_LIGHT_SPLITS];
        /// Combined bounding box of shadow casters in light projection space. Only used for focused spot lights.
        BoundingBox shadowCasterBox_[MAX_LIGHT_SPLITS];
        /// Shadow camera near splits (directional lights only).
        float shadowNearSplits_[MAX_LIGHT_SPLITS];
        /// Shadow camera far splits (directional lights only).
        float shadowFarSplits_[MAX_LIGHT_SPLITS];
        /// Shadow map split count.
        unsigned numSplits_;
    };

    /// Scene render pass info.
    struct ScenePassInfo
    {
        /// Pass index.
        unsigned passIndex_;
        /// Allow instancing flag.
        bool allowInstancing_;
        /// Mark to stencil flag.
        bool markToStencil_;
        /// Vertex light flag.
        bool vertexLights_;
        /// Batch queue.
        BatchQueue* batchQueue_;
    };

    /// Per-thread geometry, light and scene range collection structure.
    struct PerThreadSceneResult
    {
        /// Geometry objects.
        PODVector<Drawable*> geometries_;
        /// Lights.
        PODVector<Light*> lights_;
        /// Scene minimum Z value.
        float minZ_;
        /// Scene maximum Z value.
        float maxZ_;
    };

    static const unsigned MAX_VIEWPORT_TEXTURES = 2;

    /// Internal structure for 3D rendering work. Created for each backbuffer and texture viewport, but not for shadow cameras.
    class MY3D_API View : public Object
    {
        friend void CheckVisibilityWork(const WorkItem* item, unsigned threadIndex);
        friend void ProcessLightWork(const WorkItem* item, unsigned threadIndex);

        MY3D_OBJECT(View, Object)

    public:
        /// Construct.
        explicit View(Context* context);
        /// Destruct.
        ~View() override = default;

        /// Define with rendertarget and viewport. Return true if successful.
        bool Define(RenderSurface* renderTarget, Viewport* viewport);
        /// Update and cull objects and construct rendering batches.
        void Update(const FrameInfo& frame);
        /// Render batches.
        void Render();

        /// Return graphics subsystem.
        Graphics* GetGraphics() const;
        /// Return renderer subsystem.
        Renderer* GetRenderer() const;

        /// Set global (per-frame) shader parameters. Called by Batch and internally by View.
        void SetGlobalShaderParameters();
        /// Set camera-specific shader parameters. Called by Batch and internally by View.
        void SetCameraShaderParameters(Camera* camera);
        /// Set command's shader parameters if any. Called internally by View.
        void SetCommandShaderParameters(const RenderPathCommand& command);
        /// Set G-buffer offset and inverse size shader parameters. Called by Batch and internally by View.
        void SetGBufferShaderParameters(const IntVector2& texSize, const IntRect& viewRect);

        /// Draw a fullscreen quad. Shaders and renderstates must have been set beforehand. Quad will be drawn to the middle of depth range, similarly to deferred directional lights.
        void DrawFullscreenQuad(bool setIdentityProjection = false);

        /// Get a named texture from the rendertarget list or from the resource cache, to be either used as a rendertarget or texture binding.
        Texture* FindNamedTexture(const String& name, bool isRenderTarget, bool isVolumeMap = false);

    private:
        /// Send a view update or render related event through the Renderer subsystem. The parameters are the same for all of them.
        void SendViewEvent(StringHash eventType);
        /// Return the drawable's zone, or camera zone if it has override mode enabled.
        Zone* GetZone(Drawable* drawable)
        {
            if (cameraZoneOverride_)
                return cameraZone_;
            Zone* drawableZone = drawable->GetZone();
            return drawableZone ? drawableZone : cameraZone_;
        }

        /// Graphics subsystem.
        WeakPtr<Graphics> graphics_;
        /// Renderer subsystem.
        WeakPtr<Renderer> renderer_;
        /// Scene to use.
        Scene* scene_{};
        /// Octree to use.
        Octree* octree_{};
        /// Viewport (rendering) camera.
        Camera* camera_{};
        /// Culling camera. Usually same as the viewport camera.
        Camera* cullCamera_{};
        /// Shared source view. Null if this view is using its own culling.
        WeakPtr<View> sourceView_;
        /// Zone the camera is inside, or default zone if not assigned.
        Zone* cameraZone_{};
        /// Zone at far clip plane.
        Zone* farClipZone_{};
        /// Occlusion buffer for the main camera.
        OcclusionBuffer* occlusionBuffer_{};
        /// Destination color rendertarget.
        RenderSurface* renderTarget_{};
        /// Substitute rendertarget for deferred rendering. Allocated if necessary.
        RenderSurface* substituteRenderTarget_{};
        /// Texture(s) for sampling the viewport contents. Allocated if necessary.
        Texture* viewportTextures_[MAX_VIEWPORT_TEXTURES]{};
        /// Color rendertarget active for the current renderpath command.
        RenderSurface* currentRenderTarget_{};
        /// Last used custom depth render surface.
        RenderSurface* lastCustomDepthSurface_{};
        /// Texture containing the latest viewport texture.
        Texture* currentViewportTexture_{};
        /// Dummy texture for D3D9 depth only rendering.
        Texture* depthOnlyDummyTexture_{};
        /// Viewport rectangle.
        IntRect viewRect_;
        /// Viewport size.
        IntVector2 viewSize_;
        /// Destination rendertarget size.
        IntVector2 rtSize_;
        /// Information of the frame being rendered.
        FrameInfo frame_{};
        /// View aspect ratio.
        float aspectRatio_{};
        /// Minimum Z value of the visible scene.
        float minZ_{};
        /// Maximum Z value of the visible scene.
        float maxZ_{};
        /// Material quality level.
        int materialQuality_{};
        /// Maximum number of occluder triangles.
        int maxOccluderTriangles_{};
        /// Minimum number of instances required in a batch group to render as instanced.
        int minInstances_{};
        /// Highest zone priority currently visible.
        int highestZonePriority_{};
        /// Geometries updated flag.
        bool geometriesUpdated_{};
        /// Camera zone's override flag.
        bool cameraZoneOverride_{};
        /// Draw shadows flag.
        bool drawShadows_{};
        /// Deferred flag. Inferred from the existence of a light volume command in the renderpath.
        bool deferred_{};
        /// Deferred ambient pass flag. This means that the destination rendertarget is being written to at the same time as albedo/normal/depth buffers, and needs to be RGBA on OpenGL.
        bool deferredAmbient_{};
        /// Forward light base pass optimization flag. If in use, combine the base pass and first light for all opaque objects.
        bool useLitBase_{};
        /// Has scene passes flag. If no scene passes, view can be defined without a valid scene or camera to only perform quad rendering.
        bool hasScenePasses_{};
        /// Whether is using a custom readable depth texture without a stencil channel.
        bool noStencil_{};
        /// Draw debug geometry flag. Copied from the viewport.
        bool drawDebug_{};
        /// Renderpath.
        RenderPath* renderPath_{};
        /// Per-thread octree query results.
        Vector<PODVector<Drawable*> > tempDrawables_;
        /// Per-thread geometries, lights and Z range collection results.
        Vector<PerThreadSceneResult> sceneResults_;
        /// Visible zones.
        PODVector<Zone*> zones_;
        /// Visible geometry objects.
        PODVector<Drawable*> geometries_;
        /// Geometry objects that will be updated in the main thread.
        PODVector<Drawable*> nonThreadedGeometries_;
        /// Geometry objects that will be updated in worker threads.
        PODVector<Drawable*> threadedGeometries_;
        /// Occluder objects.
        PODVector<Drawable*> occluders_;
        /// Lights.
        PODVector<Light*> lights_;
        /// Number of active occluders.
        unsigned activeOccluders_{};

        /// Drawables that limit their maximum light count.
        HashSet<Drawable*> maxLightsDrawables_;
        /// Rendertargets defined by the renderpath.
        HashMap<StringHash, Texture*> renderTargets_;
        /// Intermediate light processing results.
        Vector<LightQueryResult> lightQueryResults_;
        /// Info for scene render passes defined by the renderpath.
        PODVector<ScenePassInfo> scenePasses_;
        /// Per-pixel light queues.
        Vector<LightBatchQueue> lightQueues_;
        /// Per-vertex light queues.
        HashMap<unsigned long long, LightBatchQueue> vertexLightQueues_;
        /// Batch queues by pass index.
        HashMap<unsigned, BatchQueue> batchQueues_;
        /// Index of the GBuffer pass.
        unsigned gBufferPassIndex_{};
        /// Index of the opaque forward base pass.
        unsigned basePassIndex_{};
        /// Index of the alpha pass.
        unsigned alphaPassIndex_{};
        /// Index of the forward light pass.
        unsigned lightPassIndex_{};
        /// Index of the litbase pass.
        unsigned litBasePassIndex_{};
        /// Index of the litalpha pass.
        unsigned litAlphaPassIndex_{};
        /// Pointer to the light volume command if any.
        const RenderPathCommand* lightVolumeCommand_{};
        /// Pointer to the forwardlights command if any.
        const RenderPathCommand* forwardLightsCommand_{};
        /// Pointer to the current commmand if it contains shader parameters to be set for a render pass.
        const RenderPathCommand* passCommand_{};
        /// Flag for scene being resolved from the backbuffer.
        bool usedResolve_{};
    };
}
