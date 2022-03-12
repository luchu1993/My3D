//
// Created by luchu on 2022/2/13.
//

#include "Graphics/Drawable.h"
#include "Graphics/Octree.h"
#include "Graphics/Material.h"


namespace My3D
{
    const char* GEOMETRY_CATEGORY = "Geometry";

    SourceBatch::SourceBatch() = default;

    SourceBatch::SourceBatch(const SourceBatch& batch) = default;

    SourceBatch::~SourceBatch() = default;

    SourceBatch& SourceBatch::operator =(const SourceBatch& rhs)= default;

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

    const BoundingBox& Drawable::GetWorldBoundingBox()
    {
        if (worldBoundingBoxDirty_)
        {
            OnWorldBoundingBoxUpdate();
            worldBoundingBoxDirty_ = false;
        }

        return worldBoundingBox_;
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
}