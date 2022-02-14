//
// Created by luchu on 2022/2/13.
//

#pragma once

#include "Math/BoundingBox.h"
#include "Scene/Component.h"


namespace My3D
{
    static const unsigned DRAWABLE_UNDEFINED = 0x0;
    static const unsigned DRAWABLE_GEOMETRY = 0x1;
    static const unsigned DRAWABLE_LIGHT = 0x2;
    static const unsigned DRAWABLE_ZONE = 0x4;
    static const unsigned DRAWABLE_GEOMETRY2D = 0x8;
    static const unsigned DRAWABLE_ANY = 0xff;
    static const unsigned DEFAULT_VIEWMASK = M_MAX_UNSIGNED;
    static const unsigned DEFAULT_LIGHTMASK = M_MAX_UNSIGNED;
    static const unsigned DEFAULT_SHADOWMASK = M_MAX_UNSIGNED;
    static const unsigned DEFAULT_ZONEMASK = M_MAX_UNSIGNED;
    static const int MAX_VERTEX_LIGHTS = 4;
    static const float ANIMATION_LOD_BASESCALE = 2500.0f;

    class Camera;
    class File;
    class Geometry;
    class Light;
    class Material;
    class OcclusionBuffer;
    class Octant;
    class RayOctreeQuery;
    class Zone;
    struct RayQueryResult;
    struct WorkItem;

    /// Geometry update type.
    enum UpdateGeometryType
    {
        UPDATE_NONE = 0,
        UPDATE_MAIN_THREAD,
        UPDATE_WORKER_THREAD
    };

    /// Rendering frame update parameters.
    struct FrameInfo
    {
        /// Frame number.
        unsigned frameNumber_;
        /// Time elapsed since last frame.
        float timeStep_;
        /// Viewport size.
        IntVector2 viewSize_;
        /// Camera being used.
        Camera* camera_;
    };

    /// Base class for visible component
    class MY3D_API Drawable : public Component
    {
        MY3D_OBJECT(Drawable, Component)

        friend class Octant;
        friend class Octree;
        friend void UpdateDrawablesWork(const WorkItem* item, unsigned threadIndex);

    public:
        /// Construct
        Drawable(Context* context, unsigned char drawableFlags);
        /// Destruct
        ~Drawable() override;
        /// Register object attributes. Drawable must be registered first.
        static void RegisterObject(Context* context);
        /// Handle enabled/disabled state change.
        void OnSetEnabled() override;
        /// Process octree raycast. May be called from a worker thread.
        virtual void ProcessRayQuery(const RayOctreeQuery& query, PODVector<RayQueryResult>& results);
        /// Update before octree reinsertion. Is called from a worker thread.
        virtual void Update(const FrameInfo& frame) { }
        /// Calculate distance and prepare batches for rendering. May be called from worker thread(s), possibly re-entrantly.
        virtual void UpdateBatches(const FrameInfo& frame);
        /// Prepare geometry for rendering.
        virtual void UpdateGeometry(const FrameInfo& frame) { }
        /// Return whether a geometry update is necessary, and if it can happen in a worker thread.
        virtual UpdateGeometryType GetUpdateGeometryType() { return UPDATE_NONE; }
        /// Return local space bounding box. May not be applicable or properly updated on all drawables.
        const BoundingBox& GetBoundingBox() const { return boundingBox_; }
        /// Return world-space bounding box.
        const BoundingBox& GetWorldBoundingBox();
        /// Return drawable flags.
        unsigned char GetDrawableFlags() const { return drawableFlags_; }
        /// Return draw distance.
        float GetDrawDistance() const { return drawDistance_; }
        /// Return shadow draw distance.
        float GetShadowDistance() const { return shadowDistance_; }
        /// Return LOD bias.
        float GetLodBias() const { return lodBias_; }
        /// Return view mask.
        unsigned GetViewMask() const { return viewMask_; }
        /// Return light mask.
        unsigned GetLightMask() const { return lightMask_; }
        /// Return shadow mask.
        unsigned GetShadowMask() const { return shadowMask_; }
        /// Return zone mask.
        unsigned GetZoneMask() const { return zoneMask_; }
        /// Return maximum number of per-pixel lights.
        unsigned GetMaxLights() const { return maxLights_; }
        /// Return shadowcaster flag.
        bool GetCastShadows() const { return castShadows_; }
        /// Return occluder flag.
        bool IsOccluder() const { return occluder_; }
        /// Return occludee flag.
        bool IsOccludee() const { return occludee_; }

protected:
        /// Handle node being assigned.
        void OnNodeSet(Node* node) override;
        /// Handle scene being assigned.
        void OnSceneSet(Scene* scene) override;
        /// Handle node transform being dirtied.
        void OnMarkedDirty(Node* node) override;
        /// Recalculate the world-space bounding box.
        virtual void OnWorldBoundingBoxUpdate() = 0;

        /// Handle removal from octree.
        virtual void OnRemoveFromOctree() { }
        /// Add to octree.
        void AddToOctree();
        /// Remove from octree.
        void RemoveFromOctree();
        /// Move into another octree octant.
        void SetOctant(Octant* octant) { octant_ = octant; }

        /// World-space bounding box.
        BoundingBox worldBoundingBox_;
        /// Local-space bounding box.
        BoundingBox boundingBox_;
        /// Drawable flags.
        unsigned char drawableFlags_;
        /// Bounding box dirty flag.
        bool worldBoundingBoxDirty_;
        /// Shadowcaster flag.
        bool castShadows_;
        /// Occluder flag.
        bool occluder_;
        /// Occludee flag.
        bool occludee_;
        /// Octree update queued flag.
        bool updateQueued_;
        /// Zone inconclusive or dirtied flag.
        bool zoneDirty_;
        /// Octree octant.
        Octant* octant_;
        /// Current zone.
        Zone* zone_;
        /// View mask.
        unsigned viewMask_;
        unsigned lightMask_;
        /// Shadow mask.
        unsigned shadowMask_;
        /// Zone mask.
        unsigned zoneMask_;
        /// Last visible frame number.
        unsigned viewFrameNumber_;
        /// Current distance to camera.
        float distance_;
        /// LOD scaled distance.
        float lodDistance_;
        /// Draw distance.
        float drawDistance_;
        /// Shadow distance.
        float shadowDistance_;
        /// Current sort value.
        float sortValue_;
        /// Current minimum view space depth.
        float minZ_;
        /// Current maximum view space depth.
        float maxZ_;
        /// LOD bias.
        float lodBias_;
        /// Base pass flags, bit per batch.
        unsigned basePassFlags_;
        /// Maximum per-pixel lights.
        unsigned maxLights_;
        /// List of cameras from which is seen on the current frame.
        PODVector<Camera*> viewCameras_;
        /// First per-pixel light added this frame.
        Light* firstLight_;
        /// Per-pixel lights affecting this drawable.
        PODVector<Light*> lights_;
        /// Per-vertex lights affecting this drawable.
        PODVector<Light*> vertexLights_;
    };
}
