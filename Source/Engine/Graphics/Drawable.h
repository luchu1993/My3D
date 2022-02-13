//
// Created by luchu on 2022/2/13.
//

#pragma once

#include "Math/BoundingBox.h"
#include "Scene/Component.h"


namespace My3D
{
    class Camera;
    class Octant;
    class Zone;
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

        /// Return local space bounding box. May not be applicable or properly updated on all drawables.
        const BoundingBox& GetBoundingBox() const { return boundingBox_; }

        /// Return world-space bounding box.
        const BoundingBox& GetWorldBoundingBox();
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
        /// Occluder flag.
        bool occluder_;
        /// Occludee flag.
        bool occludee_;
        /// Zone inconclusive or dirtied flag.
        bool zoneDirty_;
        /// Octree octant.
        Octant* octant_;
        /// Current zone.
        Zone* zone_;
    };
}
