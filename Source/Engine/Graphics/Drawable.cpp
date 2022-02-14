//
// Created by luchu on 2022/2/13.
//

#include "Graphics/Drawable.h"
#include "Graphics/Octree.h"

namespace My3D
{
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

}