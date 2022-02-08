//
// Created by luchu on 2022/1/22.
//

#pragma once

#include "IO/VectorBuffer.h"
#include "Math/Matrix3x4.h"
#include "Scene/Animatable.h"


namespace My3D
{
    class Node;
    class Scene;
    class Component;

    /// Component and child node creation mode for networking.
    enum CreateMode
    {
        REPLICATED = 0,
        LOCAL = 1
    };

    /// Transform space for translations and rotations.
    enum TransformSpace
    {
        TS_LOCAL = 0,
        TS_PARENT,
        TS_WORLD
    };

    /// Internal implementation structure for less performance-critical Node variables.
    struct MY3D_API NodeImpl
    {
        /// Nodes this node depends on for network updates.
        PODVector<Node*> dependencyNodes_;
        /// Name.
        String name_;
        /// Tag strings.
        StringVector tags_;
        /// Name hash.
        StringHash nameHash_;
        /// Attribute buffer for network updates.
        mutable VectorBuffer attrBuffer_;
    };

    /// Scene node that may contain components and child nodes.
    class MY3D_API Node : public Animatable
    {
        MY3D_OBJECT(Node, Animatable)

    public:
        /// Construct
        explicit Node(Context* context);
        /// Destruct. Any child nodes are detached
        ~Node() override;
        /// Register object factory
        static void RegisterObject(Context* context);

        /// Load from binary data. Return true if successful.
        bool Load(Deserializer& source) override;
        /// Load from XML data. Return true if successful.
        bool LoadXML(const XMLElement& source) override;
        /// Save as binary data. Return true if successful.
        bool Save(Serializer& dest) const override;
        /// Save as XML data. Return true if successful.
        bool SaveXML(XMLElement& dest) const override;
        /// Apply attribute changes that can not be applied immediately recursively to child nodes and components.
        void ApplyAttributes() override;

        /// Save to an XML file. Return true if successful.
        bool SaveXML(Serializer& dest, const String& indentation = "\t") const;
        /// Set name of the scene node. Names are not required to be unique.
        void SetName(const String& name);
        /// Set tags. Old tags are overwritten.
        void SetTags(const StringVector& tags);
        /// Add a tag.
        void AddTag(const String& tag);
        /// Add tags with the specified separator (; by default).
        void AddTags(const String& tags, char separator = ';');
        /// Add tags.
        void AddTags(const StringVector& tags);
        /// Remove tag. Return true if existed.
        bool RemoveTag(const String& tag);
        /// Remove all tags.
        void RemoveAllTags();
        /// Set enabled/disabled state without recursion. Components in a disabled node become effectively disabled regardless of their own enable/disable state.
        void SetEnabled(bool enable);
        /// Return whether is enabled. Disables nodes effectively disable all their components.
        bool IsEnabled() const { return enabled_; }
        /// Return ID.
        unsigned GetID() const { return id_; }
        /// Return whether the node is replicated or local to a scene.
        bool IsReplicated() const;
        /// Return name.
        const String& GetName() const { return impl_->name_; }
        /// Return name hash.
        StringHash GetNameHash() const { return impl_->nameHash_; }
        /// Return all tags.
        const StringVector& GetTags() const { return impl_->tags_; }
        /// Return whether has a specific tag.
        bool HasTag(const String& tag) const;
        /// Return whether is a direct or indirect child of specified node.
        bool IsChildOf(Node* node) const;
        /// Return parent scene node.
        Node* GetParent() const { return parent_; }
        /// Return scene.
        Scene* GetScene() const { return scene_; }
        /// Return whether transform has changed and world transform needs recalculation.
        bool IsDirty() const { return dirty_; }
        /// Return immediate child scene nodes.
        const Vector<SharedPtr<Node> >& GetChildren() const { return children_; }
        /// Set position in parent space. If the scene node is on the root level (is child of the scene itself), this is same as world space.
        void SetPosition(const Vector3& position);
        /// Return all components.
        const Vector<SharedPtr<Component>>& GetComponents() const { return components_; }
        /// Return all components of type. Optionally recursive.
        void GetComponents(PODVector<Component*>& dest, StringHash type, bool recursive = false) const;
        /// Return component by type. If there are several, returns the first.
        Component* GetComponent(StringHash type, bool recursive = false) const;
        /// Set ID. Called by Scene.
        void SetID(unsigned id);
        /// Set scene. Called by Scene.
        void SetScene(Scene* scene);
        /// Reset scene, ID and owner. Called by Scene.
        void ResetScene();
        /// Remove a component from this node.
        void RemoveComponent(Component* component);
        /// Remove the first component of specific type from this node.
        void RemoveComponent(StringHash type);
        /// Remove components that match criteria.
        void RemoveComponents(bool removeReplicated, bool removeLocal);
        /// Remove all components of specific type.
        void RemoveComponents(StringHash type);
        /// Remove all components from this node.
        void RemoveAllComponents();
        /// Mark node dirty in scene replication states.
        void MarkReplicationDirty();
        /// Add listener component that is notified of node being dirtied. Can either be in the same node or another.
        void AddListener(Component* component);
        /// Remove listener component.
        void RemoveListener(Component* component);
        /// Mark node and child nodes to need world transform recalculation. Notify listener components.
        void MarkDirty();
        /// Add a child scene node at a specific index. If index is not explicitly specified or is greater than current children size, append the new child at the end.
        void AddChild(Node* node, unsigned index = M_MAX_UNSIGNED);
        /// Remove a child scene node.
        void RemoveChild(Node* node);
        /// Remove all child scene nodes.
        void RemoveAllChildren();
        /// Remove child scene nodes that match criteria.
        void RemoveChildren(bool removeReplicated, bool removeLocal, bool recursive);

    protected:
        /// Handle attribute animation added.
        void OnAttributeAnimationAdded() override;
        /// Handle attribute animation removed.
        void OnAttributeAnimationRemoved() override;

    private:
        /// Set enabled/disabled state with optional recursion. Optionally affect the remembered enable state.
        void SetEnabled(bool enable, bool recursive, bool storeSelf);
        /// Remove a component from this node with the specified iterator.
        void RemoveComponent(Vector<SharedPtr<Component> >::Iterator i);
        /// Remove child node by iterator.
        void RemoveChild(Vector<SharedPtr<Node> >::Iterator i);
        /// Return specific components recursively.
        void GetComponentsRecursive(PODVector<Component*>& dest, StringHash type) const;
        /// World-space transform matrix.
        mutable Matrix3x4 worldTransform_;
        /// World transform needs update flag.
        mutable bool dirty_;
        /// Enabled flag.
        bool enabled_;
        /// Last SetEnabled flag before any SetDeepEnabled.
        bool enabledPrev_;

    protected:
        /// Network update queued flag.
        bool networkUpdate_;

    private:
        /// Parent scene node.
        Node* parent_;
        /// Scene (root node).
        Scene* scene_;
        /// Unique ID within the scene.
        unsigned id_;
        /// Position.
        Vector3 position_;
        /// Rotation.
        Quaternion rotation_;
        /// Scale.
        Vector3 scale_;
        /// World-space rotation.
        mutable Quaternion worldRotation_;
        /// Components.
        Vector<SharedPtr<Component> > components_;
        /// Child scene nodes.
        Vector<SharedPtr<Node> > children_;
        /// Node listeners.
        Vector<WeakPtr<Component> > listeners_;
        /// Pointer to implementation.
        UniquePtr<NodeImpl> impl_;

    protected:
        /// User variables.
        VariantMap vars_;
    };
}
