//
// Created by luchu on 2022/2/8.
//

#pragma once

#include "Scene/Animatable.h"

namespace My3D
{
    class DebugRenderer;
    class Node;
    class Scene;

    /// Autoremove is used by some components for automatic removal from the scene hierarchy upon completion of an action, for example sound or particle effect.
    enum AutoRemoveMode
    {
        REMOVE_DISABLED = 0,
        REMOVE_COMPONENT,
        REMOVE_NODE
    };

    /// Base class for components. Components can be created to scene nodes.
    class MY3D_API Component : public Animatable
    {
        MY3D_OBJECT(Component, Animatable)

        friend class Scene;
        friend class Node;

    public:
        /// Construct.
        explicit Component(Context* context);
        /// Destruct.
        ~Component() override;
        /// Handle enabled/disabled state change.
        virtual void OnSetEnabled() { }
        /// Save as binary data. Return true if successful.
        bool Save(Serializer& dest) const override;
        /// Save as XML data. Return true if successful.
        bool SaveXML(XMLElement& dest) const override;
        /// Mark for attribute check on the next network update.
        void MarkNetworkUpdate() override;
        /// Return the depended on nodes to order network updates.
        virtual void GetDependencyNodes(PODVector<Node*>& dest);
        /// Visualize the component as debug geometry.
        virtual void DrawDebugGeometry(DebugRenderer* debug, bool depthTest);

        /// Set enabled/disabled state.
        /// @property
        void SetEnabled(bool enable);
        /// Remove from the scene node. If no other shared pointer references exist, causes immediate deletion.
        void Remove();
        /// Return ID.
        unsigned GetID() const { return id_; }
        /// Return whether the component is replicated or local to a scene.
        bool IsReplicated() const;
        /// Return scene node.
        Node* GetNode() const { return node_; }
        /// Return the scene the node belongs to.
        Scene* GetScene() const;
        /// Return whether is enabled.
        bool IsEnabled() const { return enabled_; }
        /// Return whether is effectively enabled (node is also enabled).
        bool IsEnabledEffective() const;

        /// Return component in the same scene node by type. If there are several, returns the first.
        Component* GetComponent(StringHash type) const;
        /// Return components in the same scene node by type.
        void GetComponents(PODVector<Component*>& dest, StringHash type) const;
        /// Template version of returning a component in the same scene node by type.
        template <class T> T* GetComponent() const;
        /// Template version of returning components in the same scene node by type.
        template <class T> void GetComponents(PODVector<T*>& dest) const;

    protected:
        /// Handle attribute animation added.
        void OnAttributeAnimationAdded() override;
        /// Handle attribute animation removed.
        void OnAttributeAnimationRemoved() override;
        /// Handle scene node being assigned at creation.
        virtual void OnNodeSet(Node* node);
        /// Handle scene being assigned. This may happen several times during the component's lifetime. Scene-wide subsystems and events are subscribed to here.
        virtual void OnSceneSet(Scene* scene);
        /// Handle scene node transform dirtied.
        virtual void OnMarkedDirty(Node* node);
        /// Handle scene node enabled status changing.
        virtual void OnNodeSetEnabled(Node* node);
        /// Set ID. Called by Scene.
        void SetID(unsigned id);
        /// Set scene node. Called by Node when creating the component.
        void SetNode(Node* node);
        /// Handle scene attribute animation update event.
        void HandleAttributeAnimationUpdate(StringHash eventType, VariantMap& eventData);
    private:
        /// Scene node.
        Node* node_;
        /// Unique ID within the scene.
        unsigned id_;
        /// Network update queued flag.
        bool networkUpdate_;
        /// Enabled flag.
        bool enabled_;
    };

    template <typename T> T* Component::GetComponent() const { return static_cast<T*>(GetComponent(T::GetTypeStatic())); }

    template <typename T> void Component::GetComponents(PODVector<T*>& dest) const
    {
        GetComponents(reinterpret_cast<PODVector<Component*>&>(dest), T::GetTypeStatic());
    }

}
