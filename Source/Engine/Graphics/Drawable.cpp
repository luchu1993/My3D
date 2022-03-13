//
// Created by luchu on 2022/2/13.
//

#include "Core/Context.h"
#include "Graphics/Camera.h"
#include "Graphics/Geometry.h"
#include "Graphics/Drawable.h"
#include "Graphics/Octree.h"
#include "Graphics/Material.h"
#include "Graphics/Batch.h"
#include "Graphics/Renderer.h"
#include "Scene/Scene.h"
#include "Container/Sort.h"
#include "IO/File.h"
#include "IO/Log.h"


namespace My3D
{
    const char* GEOMETRY_CATEGORY = "Geometry";

    SourceBatch::SourceBatch() = default;

    SourceBatch::SourceBatch(const SourceBatch& batch) = default;

    SourceBatch::~SourceBatch() = default;

    SourceBatch& SourceBatch::operator =(const SourceBatch& rhs)= default;

    Drawable::Drawable(Context* context, unsigned char drawableFlags)
        : Component(context)
        , boundingBox_(0.0f, 0.0f)
        , drawableFlags_(drawableFlags)
        , worldBoundingBoxDirty_(true)
        , castShadows_(false)
        , occluder_(false)
        , occludee_(true)
        , updateQueued_(false)
        , zoneDirty_(false)
        , octant_(nullptr)
        , zone_(nullptr)
        , viewMask_(DEFAULT_VIEWMASK)
        , lightMask_(DEFAULT_LIGHTMASK)
        , shadowMask_(DEFAULT_SHADOWMASK)
        , zoneMask_(DEFAULT_ZONEMASK)
        , viewFrameNumber_(0)
        , distance_(0.0f)
        , lodDistance_(0.0f)
        , drawDistance_(0.0f)
        , shadowDistance_(0.0f)
        , sortValue_(0.0f)
        , minZ_(0.0f)
        , maxZ_(0.0f)
        , lodBias_(1.0f)
        , basePassFlags_(0)
        , maxLights_(0)
        , firstLight_(nullptr)
    {
        if (drawableFlags == DRAWABLE_UNDEFINED || drawableFlags > DRAWABLE_ANY)
        {
            MY3D_LOGERROR("Drawable with undefined drawableFlags");
        }
    }

    Drawable::~Drawable()
    {
        RemoveFromOctree();
    }

    void Drawable::RegisterObject(Context* context)
    {
        MY3D_ATTRIBUTE("Max Lights", int, maxLights_, 0, AM_DEFAULT);
        MY3D_ATTRIBUTE("View Mask", int, viewMask_, DEFAULT_VIEWMASK, AM_DEFAULT);
        MY3D_ATTRIBUTE("Light Mask", int, lightMask_, DEFAULT_LIGHTMASK, AM_DEFAULT);
        MY3D_ATTRIBUTE("Shadow Mask", int, shadowMask_, DEFAULT_SHADOWMASK, AM_DEFAULT);
        MY3D_ACCESSOR_ATTRIBUTE("Zone Mask", GetZoneMask, SetZoneMask, unsigned, DEFAULT_ZONEMASK, AM_DEFAULT);
    }

    void Drawable::OnSetEnabled()
    {
        bool enabled = IsEnabledEffective();

        if (enabled && !octant_)
            AddToOctree();
        else if (!enabled && octant_)
            RemoveFromOctree();
    }

    void Drawable::ProcessRayQuery(const RayOctreeQuery& query, PODVector<RayQueryResult>& results)
    {
        float distance = query.ray_.HitDistance(GetWorldBoundingBox());
        if (distance < query.maxDistance_)
        {
            RayQueryResult result;
            result.position_ = query.ray_.origin_ + distance * query.ray_.direction_;
            result.normal_ = -query.ray_.direction_;
            result.distance_ = distance;
            result.drawable_ = this;
            result.node_ = GetNode();
            result.subObject_ = M_MAX_UNSIGNED;
            results.Push(result);
        }
    }

    void Drawable::UpdateBatches(const FrameInfo& frame)
    {
        const BoundingBox& worldBoundingBox = GetWorldBoundingBox();
        const Matrix3x4& worldTransform = node_->GetWorldTransform();
        distance_ = frame.camera_->GetDistance(worldBoundingBox.Center());

        for (unsigned i = 0; i < batches_.Size(); ++i)
        {
            batches_[i].distance_ = distance_;
            batches_[i].worldTransform_ = &worldTransform;
        }

        float scale = worldBoundingBox.Size().DotProduct(DOT_SCALE);
        float newLodDistance = frame.camera_->GetLodDistance(distance_, scale, lodBias_);

        if (newLodDistance != lodDistance_)
            lodDistance_ = newLodDistance;
    }

    Geometry* Drawable::GetLodGeometry(unsigned batchIndex, unsigned level)
    {
        // By default return the visible batch geometry
        if (batchIndex < batches_.Size())
            return batches_[batchIndex].geometry_;
        else
            return nullptr;
    }

    bool Drawable::DrawOcclusion(OcclusionBuffer* buffer)
    {
        return true;
    }

    void Drawable::DrawDebugGeometry(DebugRenderer* debug, bool depthTest)
    {

    }

    void Drawable::SetDrawDistance(float distance)
    {
        drawDistance_ = distance;
        MarkNetworkUpdate();
    }

    void Drawable::SetShadowDistance(float distance)
    {
        shadowDistance_ = distance;
        MarkNetworkUpdate();
    }

    void Drawable::SetLodBias(float bias)
    {
        lodBias_ = Max(bias, M_EPSILON);
        MarkNetworkUpdate();
    }

    void Drawable::SetViewMask(unsigned mask)
    {
        viewMask_ = mask;
        MarkNetworkUpdate();
    }

    void Drawable::SetLightMask(unsigned mask)
    {
        lightMask_ = mask;
        MarkNetworkUpdate();
    }

    void Drawable::SetShadowMask(unsigned mask)
    {
        shadowMask_ = mask;
        MarkNetworkUpdate();
    }

    void Drawable::SetZoneMask(unsigned mask)
    {
        zoneMask_ = mask;
        // Mark dirty to reset cached zone
        OnMarkedDirty(node_);
        MarkNetworkUpdate();
    }

    void Drawable::SetMaxLights(unsigned num)
    {
        maxLights_ = num;
        MarkNetworkUpdate();
    }

    void Drawable::SetCastShadows(bool enable)
    {
        castShadows_ = enable;
        MarkNetworkUpdate();
    }

    void Drawable::SetOccluder(bool enable)
    {
        occluder_ = enable;
        MarkNetworkUpdate();
    }

    void Drawable::SetOccludee(bool enable)
    {
        if (enable != occludee_)
        {
            occludee_ = enable;
            // Reinsert to octree to make sure octant occlusion does not erroneously hide this drawable
            if (octant_ && !updateQueued_)
                octant_->GetRoot()->QueueUpdate(this);
            MarkNetworkUpdate();
        }
    }

    void Drawable::MarkForUpdate()
    {
        if (!updateQueued_ && octant_)
            octant_->GetRoot()->QueueUpdate(this);
    }

    const BoundingBox& Drawable::GetWorldBoundingBox()
    {
        if (worldBoundingBoxDirty_)
        {
            OnWorldBoundingBoxUpdate();
            worldBoundingBoxDirty_ = false;
        }

        return worldBoundingBox_;
    }

    bool Drawable::IsInView() const
    {
        // Note: in headless mode there is no renderer subsystem and no view frustum tests are performed, so return
        // always false in that case
        auto* renderer = GetSubsystem<Renderer>();
        return renderer && viewFrameNumber_ == renderer->GetFrameInfo().frameNumber_ && !viewCameras_.Empty();
    }

    bool Drawable::IsInView(Camera* camera) const
    {
        auto* renderer = GetSubsystem<Renderer>();
        return renderer && viewFrameNumber_ == renderer->GetFrameInfo().frameNumber_ && (!camera || viewCameras_.Contains(camera));
    }

    bool Drawable::IsInView(const FrameInfo& frame, bool anyCamera) const
    {
        return viewFrameNumber_ == frame.frameNumber_ && (anyCamera || viewCameras_.Contains(frame.camera_));
    }

    void Drawable::SetZone(Zone* zone, bool temporary)
    {
        zone_ = zone;

        // If the zone assignment was temporary (inconclusive) set the dirty flag so that it will be re-evaluated on the next frame
        zoneDirty_ = temporary;
    }

    void Drawable::SetSortValue(float value)
    {
        sortValue_ = value;
    }

    void Drawable::MarkInView(const FrameInfo& frame)
    {
        if (frame.frameNumber_ != viewFrameNumber_)
        {
            viewFrameNumber_ = frame.frameNumber_;
            viewCameras_.Resize(1);
            viewCameras_[0] = frame.camera_;
        }
        else
            viewCameras_.Push(frame.camera_);

        basePassFlags_ = 0;
        firstLight_ = nullptr;
        lights_.Clear();
        vertexLights_.Clear();
    }

    void Drawable::MarkInView(unsigned frameNumber)
    {
        if (frameNumber != viewFrameNumber_)
        {
            viewFrameNumber_ = frameNumber;
            viewCameras_.Clear();
        }
    }

    void Drawable::LimitLights()
    {
        // Maximum lights value 0 means unlimited
        if (!maxLights_ || lights_.Size() <= maxLights_)
            return;

        // If more lights than allowed, move to vertex lights and cut the list
        const BoundingBox& box = GetWorldBoundingBox();
        for (unsigned i = 0; i < lights_.Size(); ++i)
            lights_[i]->SetIntensitySortValue(box);

        Sort(lights_.Begin(), lights_.End(), CompareDrawables);
        vertexLights_.Insert(vertexLights_.End(), lights_.Begin() + maxLights_, lights_.End());
        lights_.Resize(maxLights_);
    }

    void Drawable::LimitVertexLights(bool removeConvertedLights)
    {
        if (removeConvertedLights)
        {
            for (unsigned i = vertexLights_.Size() - 1; i < vertexLights_.Size(); --i)
            {
                if (!vertexLights_[i]->GetPerVertex())
                    vertexLights_.Erase(i);
            }
        }

        if (vertexLights_.Size() <= MAX_VERTEX_LIGHTS)
            return;

        const BoundingBox& box = GetWorldBoundingBox();
        for (unsigned i = 0; i < vertexLights_.Size(); ++i)
            vertexLights_[i]->SetIntensitySortValue(box);

        Sort(vertexLights_.Begin(), vertexLights_.End(), CompareDrawables);
        vertexLights_.Resize(MAX_VERTEX_LIGHTS);
    }

    void Drawable::OnNodeSet(Node* node)
    {
        if (node)
            node->AddListener(this);
    }

    void Drawable::OnSceneSet(Scene* scene)
    {
        if (scene)
            AddToOctree();
        else
            RemoveFromOctree();
    }

    void Drawable::OnMarkedDirty(Node* node)
    {
        worldBoundingBoxDirty_ = true;
        if (!updateQueued_ && octant_)
            octant_->GetRoot()->QueueUpdate(this);

        // Mark zone assignment dirty when transform changes
        if (node == node_)
            zoneDirty_ = true;
    }

    void Drawable::AddToOctree()
    {
        // Do not add to octree when disabled
        if (!IsEnabledEffective())
            return;

        Scene* scene = GetScene();
        if (scene)
        {
            auto* octree = scene->GetComponent<Octree>();
            if (octree)
                octree->InsertDrawable(this);
            else
                MY3D_LOGERROR("No Octree component in scene, drawable will not render");
        }
        else
        {
            // We have a mechanism for adding detached nodes to an octree manually, so do not log this error
            //URHO3D_LOGERROR("Node is detached from scene, drawable will not render");
        }
    }

    void Drawable::RemoveFromOctree()
    {
        if (octant_)
        {
            Octree* octree = octant_->GetRoot();
            if (updateQueued_)
                octree->CancelUpdate(this);

            // Perform subclass specific deinitialization if necessary
            OnRemoveFromOctree();

            octant_->RemoveDrawable(this);
        }
    }
}