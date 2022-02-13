//
// Created by luchu on 2022/2/13.
//

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
                // TODO
                // root_->QueueUpdate(*i);
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

    void Octree::Update(const FrameInfo& frame)
    {

    }
}
