//
// Created by luchu on 2022/2/13.
//

#include "Container/Sort.h"
#include "Graphics/Octree.h"
#include "Core/Context.h"
#include "Core/CoreEvents.h"
#include "Core/Timer.h"
#include "Core/Thread.h"
#include "Core/WorkQueue.h"
#include "Graphics/Graphics.h"
#include "IO/Log.h"
#include "Scene/Scene.h"
#include "Scene/SceneEvents.h"


namespace My3D
{
    static const float DEFAULT_OCTREE_SIZE = 1000.0f;
    static const int DEFAULT_OCTREE_LEVELS = 8;

    void UpdateDrawablesWork(const WorkItem* item, unsigned threadIndex)
    {
        const FrameInfo& frame = *(reinterpret_cast<FrameInfo*>(item->aux_));
        auto** start = reinterpret_cast<Drawable**>(item->start_);
        auto** end = reinterpret_cast<Drawable**>(item->end_);

        while (start != end)
        {
            Drawable* drawable = *start;
            if (drawable)
                drawable->Update(frame);
            ++start;
        }
    }

    inline bool CompareRayQueryResults(const RayQueryResult& lhs, const RayQueryResult& rhs)
    {
        return lhs.distance_ < rhs.distance_;
    }

    Octant::Octant(const BoundingBox &box, unsigned int level, Octant *parent, Octree *root, unsigned int index)
        : level_(level)
        , parent_(parent)
        , root_(root)
        , index_(index)
    {
        Initialize(box);
    }

    Octant::~Octant()
    {
        if (root_)
        {
            for (auto i = drawables_.Begin(); i != drawables_.End(); ++i)
            {
                (*i)->SetOctant(root_);
                root_->drawables_.Push(*i);
                root_->QueueUpdate(*i);
            }
            drawables_.Clear();
            numDrawables_ = 0;
        }

        for (unsigned i = 0; i < NUM_OCTANTS; ++i)
            DeleteChild(i);
    }

    Octant *Octant::GetOrCreateChild(unsigned int index)
    {
        if (children_[index])
            return children_[index];

        Vector3 newMin = worldBoundingBox_.min_;
        Vector3 newMax = worldBoundingBox_.max_;
        Vector3 oldCenter = worldBoundingBox_.Center();

        if (index & 1u)
            newMin.x_ = oldCenter.x_;
        else
            newMax.x_ = oldCenter.x_;

        if (index & 2u)
            newMin.y_ = oldCenter.y_;
        else
            newMax.y_ = oldCenter.y_;

        if (index & 4u)
            newMin.z_ = oldCenter.z_;
        else
            newMax.z_ = oldCenter.z_;

        children_[index] = new Octant(BoundingBox(newMin, newMax), level_ + 1, this, root_, index);
        return children_[index];
    }

    void Octant::DeleteChild(unsigned index)
    {
        assert(index < NUM_OCTANTS);
        delete children_[index];
        children_[index] = nullptr;
    }

    void Octant::InsertDrawable(Drawable *drawable)
    {
        const BoundingBox& box = drawable->GetWorldBoundingBox();
        // If root octant, insert all non-occludees here, so that octant occlusion does not hide the drawable.
        // Also if drawable is outside the root octant bounds, insert to root
        bool insertHere;
        if (this == root_)
            insertHere = !drawable->IsOccludee() || cullingBox_.IsInside(box) != INSIDE || CheckDrawableFit(box);
        else
            insertHere = CheckDrawableFit(box);

        if (insertHere)
        {
            Octant* oldOctant = drawable->octant_;
            if (oldOctant != this)
            {
                // Add first, then remove, because drawable count going to zero deletes the octree branch in question
                AddDrawable(drawable);
                if (oldOctant)
                    oldOctant->RemoveDrawable(drawable, false);
            }
        }
        else
        {
            Vector3 boxCenter = box.Center();
            unsigned x = boxCenter.x_ < center_.x_ ? 0 : 1;
            unsigned y = boxCenter.y_ < center_.y_ ? 0 : 2;
            unsigned z = boxCenter.z_ < center_.z_ ? 0 : 4;

            GetOrCreateChild(x + y + z)->InsertDrawable(drawable);
        }
    }

    bool Octant::CheckDrawableFit(const BoundingBox &box) const
    {
        Vector3 boxSize = box.Size();
        // If max split level, size always OK, otherwise check that box is at least half size of octant
        if (level_ >= root_->GetNumLevels() || boxSize.x_ >= halfSize_.x_ || boxSize.y_ >= halfSize_.y_ ||
            boxSize.z_ >= halfSize_.z_)
            return true;
        // Also check if the box can not fit a child octant's culling box, in that case size OK (must insert here)
        else
        {
            if (box.min_.x_ <= worldBoundingBox_.min_.x_ - 0.5f * halfSize_.x_ ||
                box.max_.x_ >= worldBoundingBox_.max_.x_ + 0.5f * halfSize_.x_ ||
                box.min_.y_ <= worldBoundingBox_.min_.y_ - 0.5f * halfSize_.y_ ||
                box.max_.y_ >= worldBoundingBox_.max_.y_ + 0.5f * halfSize_.y_ ||
                box.min_.z_ <= worldBoundingBox_.min_.z_ - 0.5f * halfSize_.z_ ||
                box.max_.z_ >= worldBoundingBox_.max_.z_ + 0.5f * halfSize_.z_)
                return true;
        }
        // Bounding box too small, should create a child octant
        return false;
    }

    void Octant::Initialize(const BoundingBox &box)
    {
        worldBoundingBox_ = box;
        center_ = box.Center();
        halfSize_ = 0.5f * box.Size();
        cullingBox_ = BoundingBox(worldBoundingBox_.min_ - halfSize_, worldBoundingBox_.max_ + halfSize_);
    }

    void Octant::GetDrawablesInternal(OctreeQuery &query, bool inside) const
    {
        if (this != root_)
        {
            Intersection res = query.TestOctant(cullingBox_, inside);
            if (res == INSIDE)
                inside = true;
            else if (res == OUTSIDE)
            {
                // Fully outside, so cull this octant, its children & drawables
                return;
            }
        }

        if (drawables_.Size())
        {
            auto** start = const_cast<Drawable**>(&drawables_[0]);
            Drawable** end = start + drawables_.Size();
            query.TestDrawables(start, end, inside);
        }

        for (auto child : children_)
        {
            if (child)
                child->GetDrawablesInternal(query, inside);
        }
    }

    void Octant::GetDrawablesInternal(RayOctreeQuery& query) const
    {
        float octantDist = query.ray_.HitDistance(cullingBox_);
        if (octantDist >= query.maxDistance_)
            return;

        if (drawables_.Size())
        {
            auto** start = const_cast<Drawable**>(&drawables_[0]);
            Drawable** end = start + drawables_.Size();

            while (start != end)
            {
                Drawable* drawable = *start++;

                if ((drawable->GetDrawableFlags() & query.drawableFlags_) && (drawable->GetViewMask() & query.viewMask_))
                    drawable->ProcessRayQuery(query, query.result_);
            }
        }

        for (auto child : children_)
        {
            if (child)
                child->GetDrawablesInternal(query);
        }
    }

    void Octant::GetDrawablesOnlyInternal(RayOctreeQuery& query, PODVector<Drawable*>& drawables) const
    {
        float octantDist = query.ray_.HitDistance(cullingBox_);
        if (octantDist >= query.maxDistance_)
            return;

        if (drawables_.Size())
        {
            auto** start = const_cast<Drawable**>(&drawables_[0]);
            Drawable** end = start + drawables_.Size();

            while (start != end)
            {
                Drawable* drawable = *start++;

                if ((drawable->GetDrawableFlags() & query.drawableFlags_) && (drawable->GetViewMask() & query.viewMask_))
                    drawables.Push(drawable);
            }
        }

        for (auto child : children_)
        {
            if (child)
                child->GetDrawablesOnlyInternal(query, drawables);
        }
    }

    void Octant::ResetRoot()
    {
        root_ = nullptr;

        // The whole octree is being destroyed, just detach the drawables
        for (PODVector<Drawable*>::Iterator i = drawables_.Begin(); i != drawables_.End(); ++i)
            (*i)->SetOctant(nullptr);

        for (auto& child : children_)
        {
            if (child)
                child->ResetRoot();
        }
    }

    Octree::Octree(Context *context)
        : Component(context)
        , Octant(BoundingBox(-DEFAULT_OCTREE_SIZE, DEFAULT_OCTREE_SIZE), 0, nullptr, this)
        , numLevels_(DEFAULT_OCTREE_LEVELS)
    {
        // If the engine is running headless, subscribe to RenderUpdate events for manually updating the octree
        // to allow raycasts and animation update
        if (!GetSubsystem<Graphics>())
            SubscribeToEvent(E_RENDERUPDATE, MY3D_HANDLER(Octree, HandleRenderUpdate));
    }

    Octree::~Octree()
    {
        // Reset root pointer from all child octants now so that they do not move their drawables to root
        drawableUpdates_.Clear();
        ResetRoot();
    }

    void Octree::SetSize(const BoundingBox& box, unsigned numLevels)
    {
        // If drawables exist, they are temporarily moved to the root
        for (unsigned i = 0; i < NUM_OCTANTS; ++i)
            DeleteChild(i);

        Initialize(box);
        numDrawables_ = drawables_.Size();
        numLevels_ = Max(numLevels, 1U);
    }

    void Octree::Update(const FrameInfo& frame)
    {
        if (!Thread::IsMainThread())
        {
            MY3D_LOGERROR("Octree::Update() can not be called from worker threads");
            return;
        }
    }

    void Octree::AddManualDrawable(Drawable* drawable)
    {
        if (!drawable || drawable->GetOctant())
            return;

        AddDrawable(drawable);
    }

    void Octree::RemoveManualDrawable(Drawable* drawable)
    {
        if (!drawable)
            return;

        Octant* octant = drawable->GetOctant();
        if (octant && octant->GetRoot() == this)
            octant->RemoveDrawable(drawable);
    }

    void Octree::GetDrawables(OctreeQuery& query) const
    {
        query.result_.Clear();
        GetDrawablesInternal(query, false);
    }

    void Octree::Raycast(RayOctreeQuery& query) const
    {
        query.result_.Clear();
        GetDrawablesInternal(query);
        Sort(query.result_.Begin(), query.result_.End(), CompareRayQueryResults);
    }

    void Octree::RaycastSingle(RayOctreeQuery& query) const
    {
        query.result_.Clear();
        rayQueryDrawables_.Clear();
        GetDrawablesOnlyInternal(query, rayQueryDrawables_);

        // Sort by increasing hit distance to AABB
        for (PODVector<Drawable*>::Iterator i = rayQueryDrawables_.Begin(); i != rayQueryDrawables_.End(); ++i)
        {
            Drawable* drawable = *i;
            drawable->SetSortValue(query.ray_.HitDistance(drawable->GetWorldBoundingBox()));
        }

        Sort(rayQueryDrawables_.Begin(), rayQueryDrawables_.End(), CompareDrawables);

        // Then do the actual test according to the query, and early-out as possible
        float closestHit = M_INFINITY;
        for (PODVector<Drawable*>::Iterator i = rayQueryDrawables_.Begin(); i != rayQueryDrawables_.End(); ++i)
        {
            Drawable* drawable = *i;
            if (drawable->GetSortValue() < Min(closestHit, query.maxDistance_))
            {
                unsigned oldSize = query.result_.Size();
                drawable->ProcessRayQuery(query, query.result_);
                if (query.result_.Size() > oldSize)
                    closestHit = Min(closestHit, query.result_.Back().distance_);
            }
            else
                break;
        }

        if (query.result_.Size() > 1)
        {
            Sort(query.result_.Begin(), query.result_.End(), CompareRayQueryResults);
            query.result_.Resize(1);
        }
    }

    void Octree::QueueUpdate(Drawable* drawable)
    {
        Scene* scene = GetScene();
        if (scene && scene->IsThreadedUpdate())
        {
            MutexLock lock(octreeMutex_);
            threadedDrawableUpdates_.Push(drawable);
        }
        else
            drawableUpdates_.Push(drawable);

        drawable->updateQueued_ = true;
    }

    void Octree::CancelUpdate(Drawable* drawable)
    {
        // This doesn't have to take into account scene being in threaded update, because it is called only
        // when removing a drawable from octree, which should only ever happen from the main thread.
        drawableUpdates_.Remove(drawable);
        drawable->updateQueued_ = false;
    }

    void Octree::HandleRenderUpdate(StringHash eventType, VariantMap &eventData)
    {
        // When running in headless mode, update the Octree manually during the RenderUpdate event
        Scene* scene = GetScene();
        if (!scene || !scene->IsUpdateEnabled())
            return;

        using namespace RenderUpdate;

        FrameInfo frame;
        frame.frameNumber_ = GetSubsystem<Time>()->GetFrameNumber();
        frame.timeStep_ = eventData[P_TIMESTEP].GetFloat();
        frame.camera_ = nullptr;

        Update(frame);
    }
}
