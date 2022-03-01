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

    };
}
