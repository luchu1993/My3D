//
// Created by luchu on 2022/1/22.
//

#include "Core/Context.h"
#include "Scene/Node.h"
#include "Resource/XMLFile.h"
#include "Scene/SceneEvents.h"
#include "Scene/Scene.h"
#include "Scene/Component.h"


namespace My3D
{
    Node::Node(Context *context)
        : Animatable(context)
        , worldTransform_(Matrix3x4::IDENTITY)
        , dirty_(false)
        , enabled_(true)
        , enabledPrev_(true)
        , networkUpdate_(false)
        , parent_(nullptr)
        , scene_(nullptr)
        , id_(0)
        , position_(Vector3::ZERO)
        , rotation_(Quaternion::IDENTITY)
        , worldRotation_(Quaternion::IDENTITY)
    {
        impl_ = new NodeImpl();
    }

    Node::~Node()
    {
        RemoveAllChildren();
        RemoveAllComponents();

        // Remove from the scene
        if (scene_)
            scene_->NodeRemoved(this);
    }

    void Node::RegisterObject(Context *context)
    {
        context->RegisterFactory<Node>();

        MY3D_ACCESSOR_ATTRIBUTE("Is Enabled", IsEnabled, SetEnabled, bool, true, AM_DEFAULT);
        MY3D_ACCESSOR_ATTRIBUTE("Name", GetName, SetName, String, String::EMPTY, AM_DEFAULT);
        MY3D_ACCESSOR_ATTRIBUTE("Tags", GetTags, SetTags, StringVector, Variant::emptyStringVector, AM_DEFAULT);
        MY3D_ACCESSOR_ATTRIBUTE("Position", GetPosition, SetPosition, Vector3, Vector3::ZERO, AM_FILE);
        MY3D_ACCESSOR_ATTRIBUTE("Rotation", GetRotation, SetRotation, Quaternion, Quaternion::IDENTITY, AM_FILE);
        MY3D_ACCESSOR_ATTRIBUTE("Scale", GetScale, SetScale, Vector3, Vector3::ONE, AM_DEFAULT);
    }

    bool Node::Load(Deserializer &source)
    {
        return false;
    }

    bool Node::Save(Serializer &dest) const
    {
        return false;
    }

    bool Node::LoadXML(const XMLElement &source)
    {
        return false;
    }

    bool Node::SaveXML(XMLElement &dest) const
    {
        return false;
    }

    void Node::ApplyAttributes()
    {

    }

    bool Node::SaveXML(Serializer& dest, const String& indentation) const
    {
        SharedPtr<XMLFile> xml(new XMLFile(context_));
        XMLElement rootElem = xml->CreateRoot("node");
        if (!SaveXML(rootElem))
            return false;

        return xml->Save(dest, indentation);
    }

    void Node::SetName(const String& name)
    {
        if (name != impl_->name_)
        {
            impl_->name_ = name;
            impl_->nameHash_ = name;

            MarkNetworkUpdate();

            // Send change event
            if (scene_)
            {
                using namespace NodeNameChanged;

                VariantMap& eventData = GetEventDataMap();
                eventData[P_SCENE] = scene_;
                eventData[P_NODE] = this;

                scene_->SendEvent(E_NODENAMECHANGED, eventData);
            }
        }
    }

    void Node::SetTags(const StringVector& tags)
    {
        RemoveAllTags();
        AddTags(tags);
        // MarkNetworkUpdate() already called in RemoveAllTags() / AddTags()
    }

    void Node::AddTag(const String& tag)
    {
        // Check if tag empty or already added
        if (tag.Empty() || HasTag(tag))
            return;

        // Add tag
        impl_->tags_.Push(tag);

        // Cache
        if (scene_)
        {
            scene_->NodeTagAdded(this, tag);

            // Send event
            using namespace NodeTagAdded;
            VariantMap& eventData = GetEventDataMap();
            eventData[P_SCENE] = scene_;
            eventData[P_NODE] = this;
            eventData[P_TAG] = tag;
            scene_->SendEvent(E_NODETAGADDED, eventData);
        }
        // Sync
        MarkNetworkUpdate();
    }

    void Node::AddTags(const String& tags, char separator)
    {
        StringVector tagVector = tags.Split(separator);
        AddTags(tagVector);
    }

    void Node::AddTags(const StringVector& tags)
    {
        // This is OK, as MarkNetworkUpdate() early-outs when called multiple times
        for (unsigned i = 0; i < tags.Size(); ++i)
            AddTag(tags[i]);
    }

    bool Node::RemoveTag(const String& tag)
    {
        bool removed = impl_->tags_.Remove(tag);

        // Nothing to do
        if (!removed)
            return false;

        // Scene cache update
        if (scene_)
        {
            scene_->NodeTagRemoved(this, tag);
            // Send event
            using namespace NodeTagRemoved;
            VariantMap& eventData = GetEventDataMap();
            eventData[P_SCENE] = scene_;
            eventData[P_NODE] = this;
            eventData[P_TAG] = tag;
            scene_->SendEvent(E_NODETAGREMOVED, eventData);
        }

        // Sync
        MarkNetworkUpdate();
        return true;
    }

    void Node::RemoveAllTags()
    {
        // Clear old scene cache
        if (scene_)
        {
            for (unsigned i = 0; i < impl_->tags_.Size(); ++i)
            {
                scene_->NodeTagRemoved(this, impl_->tags_[i]);

                // Send event
                using namespace NodeTagRemoved;
                VariantMap& eventData = GetEventDataMap();
                eventData[P_SCENE] = scene_;
                eventData[P_NODE] = this;
                eventData[P_TAG] = impl_->tags_[i];
                scene_->SendEvent(E_NODETAGREMOVED, eventData);
            }
        }

        impl_->tags_.Clear();

        // Sync
        MarkNetworkUpdate();
    }

    bool Node::HasTag(const String& tag) const
    {
        return impl_->tags_.Contains(tag);
    }

    bool Node::IsReplicated() const
    {
        return Scene::IsReplicatedID(id_);
    }

    bool Node::IsChildOf(Node* node) const
    {
        Node* parent = parent_;
        while (parent)
        {
            if (parent == node)
                return true;
            parent = parent->parent_;
        }
        return false;
    }

    void Node::SetEnabled(bool enable)
    {

    }

    void Node::OnAttributeAnimationAdded()
    {
    }

    void Node::OnAttributeAnimationRemoved()
    {

    }

    void Node::GetComponents(PODVector<Component*>& dest, StringHash type, bool recursive) const
    {
        dest.Clear();

        if (!recursive)
        {
            for (Vector<SharedPtr<Component> >::ConstIterator i = components_.Begin(); i != components_.End(); ++i)
            {
                if ((*i)->GetType() == type)
                    dest.Push(*i);
            }
        }
        else
            GetComponentsRecursive(dest, type);
    }

    void Node::GetComponentsRecursive(PODVector<Component*>& dest, StringHash type) const
    {
        for (Vector<SharedPtr<Component> >::ConstIterator i = components_.Begin(); i != components_.End(); ++i)
        {
            if ((*i)->GetType() == type)
                dest.Push(*i);
        }
        for (Vector<SharedPtr<Node> >::ConstIterator i = children_.Begin(); i != children_.End(); ++i)
            (*i)->GetComponentsRecursive(dest, type);
    }

    Component* Node::GetComponent(StringHash type, bool recursive) const
    {
        for (Vector<SharedPtr<Component> >::ConstIterator i = components_.Begin(); i != components_.End(); ++i)
        {
            if ((*i)->GetType() == type)
                return *i;
        }

        if (recursive)
        {
            for (Vector<SharedPtr<Node> >::ConstIterator i = children_.Begin(); i != children_.End(); ++i)
            {
                Component* component = (*i)->GetComponent(type, true);
                if (component)
                    return component;
            }
        }

        return nullptr;
    }

    void Node::SetID(unsigned id)
    {
        id_ = id;
    }

    void Node::SetScene(Scene* scene)
    {
        scene_ = scene;
    }

    void Node::ResetScene()
    {
        SetID(0);
        SetScene(nullptr);
    }

    void Node::RemoveComponent(Component* component)
    {
        for (Vector<SharedPtr<Component> >::Iterator i = components_.Begin(); i != components_.End(); ++i)
        {
            if (*i == component)
            {
                RemoveComponent(i);

                // Mark node dirty in all replication states
                MarkReplicationDirty();
                return;
            }
        }
    }

    void Node::RemoveComponent(Vector<SharedPtr<Component> >::Iterator i)
    {
        // Send node change event. Do not send when already being destroyed
        if (Refs() > 0 && scene_)
        {
            using namespace ComponentRemoved;

            VariantMap& eventData = GetEventDataMap();
            eventData[P_SCENE] = scene_;
            eventData[P_NODE] = this;
            eventData[P_COMPONENT] = (*i).Get();

            scene_->SendEvent(E_COMPONENTREMOVED, eventData);
        }

        RemoveListener(*i);
        if (scene_)
            scene_->ComponentRemoved(*i);
        (*i)->SetNode(nullptr);
        components_.Erase(i);
    }

    void Node::RemoveComponent(StringHash type)
    {
        for (Vector<SharedPtr<Component> >::Iterator i = components_.Begin(); i != components_.End(); ++i)
        {
            if ((*i)->GetType() == type)
            {
                RemoveComponent(i);

                // Mark node dirty in all replication states
                MarkReplicationDirty();
                return;
            }
        }
    }

    void Node::RemoveComponents(StringHash type)
    {
        unsigned numRemoved = 0;

        for (unsigned i = components_.Size() - 1; i < components_.Size(); --i)
        {
            if (components_[i]->GetType() == type)
            {
                RemoveComponent(components_.Begin() + i);
                ++numRemoved;
            }
        }

        // Mark node dirty in all replication states
        if (numRemoved)
            MarkReplicationDirty();
    }

    void Node::RemoveComponents(bool removeReplicated, bool removeLocal)
    {
        unsigned numRemoved = 0;

        for (unsigned i = components_.Size() - 1; i < components_.Size(); --i)
        {
            bool remove = false;
            Component* component = components_[i];

            if (component->IsReplicated() && removeReplicated)
                remove = true;
            else if (!component->IsReplicated() && removeLocal)
                remove = true;

            if (remove)
            {
                RemoveComponent(components_.Begin() + i);
                ++numRemoved;
            }
        }

        // Mark node dirty in all replication states
        if (numRemoved)
            MarkReplicationDirty();
    }

    void Node::RemoveAllComponents()
    {
        RemoveComponents(true, true);
    }

    void Node::MarkReplicationDirty()
    {

    }

    void Node::AddListener(Component* component)
    {
        if (!component)
            return;

        // Check for not adding twice
        for (Vector<WeakPtr<Component> >::Iterator i = listeners_.Begin(); i != listeners_.End(); ++i)
        {
            if (*i == component)
                return;
        }

        listeners_.Push(WeakPtr<Component>(component));
        // If the node is currently dirty, notify immediately
        if (dirty_)
            component->OnMarkedDirty(this);
    }

    void Node::MarkDirty()
    {
        Node *cur = this;
        for (;;)
        {
            // Precondition:
            // a) whenever a node is marked dirty, all its children are marked dirty as well.
            // b) whenever a node is cleared from being dirty, all its parents must have been
            //    cleared as well.
            // Therefore if we are recursing here to mark this node dirty, and it already was,
            // then all children of this node must also be already dirty, and we don't need to
            // reflag them again.
            if (cur->dirty_)
                return;
            cur->dirty_ = true;

            // Notify listener components first, then mark child nodes
            for (Vector<WeakPtr<Component> >::Iterator i = cur->listeners_.Begin(); i != cur->listeners_.End();)
            {
                Component *c = *i;
                if (c)
                {
                    c->OnMarkedDirty(cur);
                    ++i;
                }
                    // If listener has expired, erase from list (swap with the last element to avoid O(n^2) behavior)
                else
                {
                    *i = cur->listeners_.Back();
                    cur->listeners_.Pop();
                }
            }

            // Tail call optimization: Don't recurse to mark the first child dirty, but
            // instead process it in the context of the current function. If there are more
            // than one child, then recurse to the excess children.
            Vector<SharedPtr<Node> >::Iterator i = cur->children_.Begin();
            if (i != cur->children_.End())
            {
                Node *next = *i;
                for (++i; i != cur->children_.End(); ++i)
                    (*i)->MarkDirty();
                cur = next;
            }
            else
                return;
        }
    }

    void Node::RemoveListener(Component* component)
    {
        for (Vector<WeakPtr<Component> >::Iterator i = listeners_.Begin(); i != listeners_.End(); ++i)
        {
            if (*i == component)
            {
                listeners_.Erase(i);
                return;
            }
        }
    }

    void Node::AddChild(Node* node, unsigned index)
    {
        // Check for illegal or redundant parent assignment
        if (!node || node == this || node->parent_ == this)
            return;
        // Check for possible cyclic parent assignment
        if (IsChildOf(node))
            return;

        // Keep a shared ptr to the node while transferring
        SharedPtr<Node> nodeShared(node);
        Node* oldParent = node->parent_;
        if (oldParent)
        {
            // If old parent is in different scene, perform the full removal
            if (oldParent->GetScene() != scene_)
                oldParent->RemoveChild(node);
            else
            {
                if (scene_)
                {
                    // Otherwise do not remove from the scene during reparenting, just send the necessary change event
                    using namespace NodeRemoved;

                    VariantMap& eventData = GetEventDataMap();
                    eventData[P_SCENE] = scene_;
                    eventData[P_PARENT] = oldParent;
                    eventData[P_NODE] = node;

                    scene_->SendEvent(E_NODEREMOVED, eventData);
                }

                oldParent->children_.Remove(nodeShared);
            }
        }

        // Add to the child vector, then add to the scene if not added yet
        children_.Insert(index, nodeShared);
        if (scene_ && node->GetScene() != scene_)
            scene_->NodeAdded(node);

        node->parent_ = this;
        node->MarkDirty();
        node->MarkNetworkUpdate();
        // If the child node has components, also mark network update on them to ensure they have a valid NetworkState
        for (Vector<SharedPtr<Component> >::Iterator i = node->components_.Begin(); i != node->components_.End(); ++i)
            (*i)->MarkNetworkUpdate();

        // Send change event
        if (scene_)
        {
            using namespace NodeAdded;

            VariantMap& eventData = GetEventDataMap();
            eventData[P_SCENE] = scene_;
            eventData[P_PARENT] = this;
            eventData[P_NODE] = node;

            scene_->SendEvent(E_NODEADDED, eventData);
        }
    }

    void Node::RemoveChild(Node* node)
    {
        if (!node)
            return;

        for (Vector<SharedPtr<Node> >::Iterator i = children_.Begin(); i != children_.End(); ++i)
        {
            if (*i == node)
            {
                RemoveChild(i);
                return;
            }
        }
    }

    void Node::RemoveAllChildren()
    {
        RemoveChildren(true, true, true);
    }

    void Node::RemoveChildren(bool removeReplicated, bool removeLocal, bool recursive)
    {
        unsigned numRemoved = 0;

        for (unsigned i = children_.Size() - 1; i < children_.Size(); --i)
        {
            bool remove = false;
            Node* childNode = children_[i];

            if (recursive)
                childNode->RemoveChildren(removeReplicated, removeLocal, true);
            if (childNode->IsReplicated() && removeReplicated)
                remove = true;
            else if (!childNode->IsReplicated() && removeLocal)
                remove = true;

            if (remove)
            {
                RemoveChild(children_.Begin() + i);
                ++numRemoved;
            }
        }

        // Mark node dirty in all replication states
        if (numRemoved)
            MarkReplicationDirty();
    }

    void Node::RemoveChild(Vector<SharedPtr<Node> >::Iterator i)
    {
        // Keep a shared pointer to the child about to be removed, to make sure the erase from container completes first. Otherwise
        // it would be possible that other child nodes get removed as part of the node's components' cleanup, causing a re-entrant
        // erase and a crash
        SharedPtr<Node> child(*i);

        // Send change event. Do not send when this node is already being destroyed
        if (Refs() > 0 && scene_)
        {
            using namespace NodeRemoved;

            VariantMap& eventData = GetEventDataMap();
            eventData[P_SCENE] = scene_;
            eventData[P_PARENT] = this;
            eventData[P_NODE] = child;

            scene_->SendEvent(E_NODEREMOVED, eventData);
        }

        child->parent_ = nullptr;
        child->MarkDirty();
        child->MarkNetworkUpdate();
        if (scene_)
            scene_->NodeRemoved(child);

        children_.Erase(i);
    }

    void Node::SetPosition(const Vector3& position)
    {
        position_ = position;
        MarkDirty();

        MarkNetworkUpdate();
    }

    void Node::SetRotation(const Quaternion& rotation)
    {
        rotation_ = rotation;
        MarkDirty();

        MarkNetworkUpdate();
    }

    void Node::SetDirection(const Vector3& direction)
    {
        SetRotation(Quaternion(Vector3::FORWARD, direction));
    }

    void Node::SetScale(float scale)
    {
        SetScale(Vector3(scale, scale, scale));
    }

    void Node::SetScale(const Vector3& scale)
    {
        scale_ = scale;
        // Prevent exact zero scale e.g. from momentary edits as this may cause division by zero
        // when decomposing the world transform matrix
        if (scale_.x_ == 0.0f)
            scale_.x_ = M_EPSILON;
        if (scale_.y_ == 0.0f)
            scale_.y_ = M_EPSILON;
        if (scale_.z_ == 0.0f)
            scale_.z_ = M_EPSILON;

        MarkDirty();
        MarkNetworkUpdate();
    }
}