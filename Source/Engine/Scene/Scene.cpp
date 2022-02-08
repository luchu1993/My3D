//
// Created by luchu on 2022/1/22.
//

#include "Core/Context.h"
#include "Core/CoreEvents.h"
#include "IO/Log.h"
#include "Resource/ResourceCache.h"
#include "Resource/ResourceEvents.h"
#include "Resource/XMLFile.h"
#include "Scene/Component.h"
#include "Scene/ObjectAnimation.h"
#include "Scene/Scene.h"
#include "Scene/SceneEvents.h"
#include "Scene/ValueAnimation.h"


namespace My3D
{
    const char* SCENE_CATEGORY = "Scene";
    const char* LOGIC_CATEGORY = "Logic";
    const char* SUBSYSTEM_CATEGORY = "Subsystem";

    Scene::Scene(Context *context)
        : Node(context)
        , replicatedNodeID_(FIRST_REPLICATED_ID)
        , replicatedComponentID_(FIRST_REPLICATED_ID)
        , localNodeID_(FIRST_LOCAL_ID)
        , localComponentID_(FIRST_LOCAL_ID)
        , checksum_(0)
        , timeScale_(1.0f)
        , elapsedTime_(0)
        , updateEnabled_(true)
        , asyncLoading_(false)
        , threadedUpdate_(false)
    {
        // Assign an ID to self so that nodes can refer to this node as a parent
        SetID(GetFreeNodeID(REPLICATED));
        NodeAdded(this);
    }

    Scene::~Scene()
    {
        // Remove root-level components first, so that scene subsystems such as the octree destroy themselves. This will speed up
        // the removal of child nodes' components
        RemoveAllComponents();
        RemoveAllChildren();

        // Remove scene reference and owner from all nodes that still exist
        for (HashMap<unsigned, Node*>::Iterator i = replicatedNodes_.Begin(); i != replicatedNodes_.End(); ++i)
            i->second_->ResetScene();
        for (HashMap<unsigned, Node*>::Iterator i = localNodes_.Begin(); i != localNodes_.End(); ++i)
            i->second_->ResetScene();
    }

    unsigned Scene::GetFreeNodeID(CreateMode mode)
    {
        if (mode == REPLICATED)
        {
            for (;;)
            {
                unsigned ret = replicatedNodeID_;
                if (replicatedNodeID_ < LAST_REPLICATED_ID)
                    ++replicatedNodeID_;
                else
                    replicatedNodeID_ = FIRST_REPLICATED_ID;

                if (!replicatedNodes_.Contains(ret))
                    return ret;
            }
        }
        else
        {
            for (;;)
            {
                unsigned ret = localNodeID_;
                if (localNodeID_ < LAST_LOCAL_ID)
                    ++localNodeID_;
                else
                    localNodeID_ = FIRST_LOCAL_ID;

                if (!localNodes_.Contains(ret))
                    return ret;
            }
        }
    }

    unsigned Scene::GetFreeComponentID(CreateMode mode)
    {
        if (mode == REPLICATED)
        {
            for (;;)
            {
                unsigned ret = replicatedComponentID_;
                if (replicatedComponentID_ < LAST_REPLICATED_ID)
                    ++replicatedComponentID_;
                else
                    replicatedComponentID_ = FIRST_REPLICATED_ID;

                if (!replicatedComponents_.Contains(ret))
                    return ret;
            }
        }
        else
        {
            for (;;)
            {
                unsigned ret = localComponentID_;
                if (localComponentID_ < LAST_LOCAL_ID)
                    ++localComponentID_;
                else
                    localComponentID_ = FIRST_LOCAL_ID;

                if (!localComponents_.Contains(ret))
                    return ret;
            }
        }
    }

    void Scene::NodeAdded(Node* node)
    {
        if (!node || node->GetScene() == this)
            return;

        // Remove from old scene first
        Scene* oldScene = node->GetScene();
        if (oldScene)
            oldScene->NodeRemoved(node);

        node->SetScene(this);

        // If the new node has an ID of zero (default), assign a replicated ID now
        unsigned id = node->GetID();
        if (!id)
        {
            id = GetFreeNodeID(REPLICATED);
            node->SetID(id);
        }

        // If node with same ID exists, remove the scene reference from it and overwrite with the new node
        if (IsReplicatedID(id))
        {
            HashMap<unsigned, Node*>::Iterator i = replicatedNodes_.Find(id);
            if (i != replicatedNodes_.End() && i->second_ != node)
            {
                MY3D_LOGWARNING("Overwriting node with ID " + String(id));
                NodeRemoved(i->second_);
            }

            replicatedNodes_[id] = node;

            MarkNetworkUpdate(node);
            MarkReplicationDirty(node);
        }
        else
        {
            HashMap<unsigned, Node*>::Iterator i = localNodes_.Find(id);
            if (i != localNodes_.End() && i->second_ != node)
            {
                MY3D_LOGWARNING("Overwriting node with ID " + String(id));
                NodeRemoved(i->second_);
            }
            localNodes_[id] = node;
        }

        // Cache tag if already tagged.
        if (!node->GetTags().Empty())
        {
            const StringVector& tags = node->GetTags();
            for (unsigned i = 0; i < tags.Size(); ++i)
                taggedNodes_[tags[i]].Push(node);
        }

        // Add already created components and child nodes now
        const Vector<SharedPtr<Component>>& components = node->GetComponents();
        for (Vector<SharedPtr<Component> >::ConstIterator i = components.Begin(); i != components.End(); ++i)
            ComponentAdded(*i);
        const Vector<SharedPtr<Node> >& children = node->GetChildren();
        for (Vector<SharedPtr<Node> >::ConstIterator i = children.Begin(); i != children.End(); ++i)
            NodeAdded(*i);
    }

    void Scene::NodeTagAdded(Node* node, const String& tag)
    {
        taggedNodes_[tag].Push(node);
    }

    void Scene::NodeTagRemoved(Node* node, const String& tag)
    {
        taggedNodes_[tag].Remove(node);
    }

    void Scene::NodeRemoved(Node* node)
    {
        if (!node || node->GetScene() != this)
            return;

        unsigned id = node->GetID();
        if (Scene::IsReplicatedID(id))
        {
            replicatedNodes_.Erase(id);
            MarkReplicationDirty(node);
        }
        else
            localNodes_.Erase(id);

        node->ResetScene();

        // Remove node from tag cache
        if (!node->GetTags().Empty())
        {
            const StringVector& tags = node->GetTags();
            for (unsigned i = 0; i < tags.Size(); ++i)
                taggedNodes_[tags[i]].Remove(node);
        }

        // Remove components and child nodes as well
        const Vector<SharedPtr<Component>>& components = node->GetComponents();
        for (Vector<SharedPtr<Component>>::ConstIterator i = components.Begin(); i != components.End(); ++i)
            ComponentRemoved(*i);
        const Vector<SharedPtr<Node>>& children = node->GetChildren();
        for (Vector<SharedPtr<Node>>::ConstIterator i = children.Begin(); i != children.End(); ++i)
            NodeRemoved(*i);
    }

    void Scene::ComponentAdded(Component* component)
    {
        if (!component)
            return;

        unsigned id = component->GetID();

        // If the new component has an ID of zero (default), assign a replicated ID now
        if (!id)
        {
            id = GetFreeComponentID(REPLICATED);
            component->SetID(id);
        }

        if (IsReplicatedID(id))
        {
            HashMap<unsigned, Component*>::Iterator i = replicatedComponents_.Find(id);
            if (i != replicatedComponents_.End() && i->second_ != component)
            {
                MY3D_LOGWARNING("Overwriting component with ID " + String(id));
                ComponentRemoved(i->second_);
            }

            replicatedComponents_[id] = component;
        }
        else
        {
            HashMap<unsigned, Component*>::Iterator i = localComponents_.Find(id);
            if (i != localComponents_.End() && i->second_ != component)
            {
                MY3D_LOGWARNING("Overwriting component with ID " + String(id));
                ComponentRemoved(i->second_);
            }

            localComponents_[id] = component;
        }

        component->OnSceneSet(this);
    }

    void Scene::ComponentRemoved(Component* component)
    {
        if (!component)
            return;

        unsigned id = component->GetID();
        if (Scene::IsReplicatedID(id))
            replicatedComponents_.Erase(id);
        else
            localComponents_.Erase(id);

        component->SetID(0);
        component->OnSceneSet(nullptr);
    }

    void Scene::MarkNetworkUpdate(Node* node)
    {
        if (node)
        {
            if (!threadedUpdate_)
                networkUpdateNodes_.Insert(node->GetID());
            else
            {
                MutexLock lock(sceneMutex_);
                networkUpdateNodes_.Insert(node->GetID());
            }
        }
    }

    void Scene::MarkNetworkUpdate(Component* component)
    {
        if (component)
        {
            if (!threadedUpdate_)
                networkUpdateComponents_.Insert(component->GetID());
            else
            {
                MutexLock lock(sceneMutex_);
                networkUpdateComponents_.Insert(component->GetID());
            }
        }
    }

    void Scene::MarkReplicationDirty(Node* node)
    {

    }

    void Scene::Update(float timeStep)
    {
        if (asyncLoading_)
        {
            UpdateAsyncLoading();
            // If only preloading resources, scene update can continue
            if (asyncProgress_.mode_ > LOAD_RESOURCES_ONLY)
                return;
        }

        timeStep *= timeScale_;

        using namespace SceneUpdate;
        VariantMap& eventData = GetEventDataMap();
        eventData[P_SCENE] = this;
        eventData[P_TIMESTEP] = timeStep;

        // Update variable timestep logic
        SendEvent(E_SCENEUPDATE, eventData);

        // Update scene attribute animation.
        SendEvent(E_ATTRIBUTEANIMATIONUPDATE, eventData);

        // Update scene subsystems. If a physics world is present, it will be updated, triggering fixed timestep logic updates
        SendEvent(E_SCENESUBSYSTEMUPDATE, eventData);

        // Post-update variable timestep logic
        SendEvent(E_SCENEPOSTUPDATE, eventData);

        // Note: using a float for elapsed time accumulation is inherently inaccurate. The purpose of this value is
        // primarily to update material animation effects, as it is available to shaders. It can be reset by calling
        // SetElapsedTime()
        elapsedTime_ += timeStep;
    }

    void Scene::UpdateAsyncLoading()
    {

    }

    void Scene::HandleUpdate(StringHash eventType, VariantMap& eventData)
    {
        if (!updateEnabled_)
            return;

        using namespace Update;
        Update(eventData[P_TIMESTEP].GetFloat());
    }
}