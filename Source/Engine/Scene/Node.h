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
    class SceneResolver;
    class Connection;

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
        /// Return the node's last own enabled state. May be different than the value returned by IsEnabled when SetDeepEnabled has been used.
        bool IsEnabledSelf() const { return enabledPrev_; }
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

        /// Set position in parent space. If the scene node is on the root level (is child of the scene itself), this is same as world space.
        void SetPosition(const Vector3& position);
        /// Set position in world space.
        void SetWorldPosition(const Vector3& position);
        /// Set rotation in parent space.
        void SetRotation(const Quaternion& rotation);
        /// Set rotation in world space.
        void SetWorldRotation(const Quaternion& rotation);
        /// Set forward direction in parent space. Positive Z axis equals identity rotation.
        void SetDirection(const Vector3& direction);
        /// Set forward direction in world space.
        void SetWorldDirection(const Vector3& direction);
        /// Set uniform scale in parent space.
        void SetScale(float scale);
        /// Set uniform scale in world space.
        void SetWorldScale(float scale);
        /// Set scale in parent space.
        void SetScale(const Vector3& scale);
        /// Set scale in world space.
        void SetWorldScale(const Vector3& scale);
        /// Set both position and rotation in parent space as an atomic operation. This is faster than setting position and rotation separately.
        void SetTransform(const Vector3& position, const Quaternion& rotation);
        /// Set position, rotation, and uniform scale in parent space as an atomic operation.
        void SetTransform(const Vector3& position, const Quaternion& rotation, float scale);
        /// Set position, rotation, and scale in parent space as an atomic operation.
        void SetTransform(const Vector3& position, const Quaternion& rotation, const Vector3& scale);
        /// Set node transformation in parent space as an atomic operation.
        void SetTransform(const Matrix3x4& matrix);
        /// Return position in parent space.
        const Vector3& GetPosition() const { return position_; }
        /// Return rotation in parent space.
        const Quaternion& GetRotation() const { return rotation_; }
        /// Return scale in parent space.
        const Vector3& GetScale() const { return scale_; }
        /// Return forward direction in parent space. Positive Z axis equals identity rotation.
        Vector3 GetDirection() const { return rotation_ * Vector3::FORWARD; }
        /// Return up direction in parent space. Positive Y axis equals identity rotation.
        Vector3 GetUp() const { return rotation_ * Vector3::UP; }
        /// Return right direction in parent space. Positive X axis equals identity rotation.
        Vector3 GetRight() const { return rotation_ * Vector3::RIGHT; }
        /// Return parent space transform matrix.
        Matrix3x4 GetTransform() const { return Matrix3x4(position_, rotation_, scale_); }
        /// Return position in world space.
        Vector3 GetWorldPosition() const
        {
            if (dirty_)
                UpdateWorldTransform();

            return worldTransform_.Translation();
        }
        /// Return rotation in world space.
        Quaternion GetWorldRotation() const
        {
            if (dirty_)
                UpdateWorldTransform();

            return worldRotation_;
        }
        /// Return direction in world space.
        Vector3 GetWorldDirection() const
        {
            if (dirty_)
                UpdateWorldTransform();

            return worldRotation_ * Vector3::FORWARD;
        }
        /// Return node's up vector in world space.
        Vector3 GetWorldUp() const
        {
            if (dirty_)
                UpdateWorldTransform();

            return worldRotation_ * Vector3::UP;
        }
        /// Return node's right vector in world space.
        Vector3 GetWorldRight() const
        {
            if (dirty_)
                UpdateWorldTransform();

            return worldRotation_ * Vector3::RIGHT;
        }
        /// Return scale in world space.
        Vector3 GetWorldScale() const
        {
            if (dirty_)
                UpdateWorldTransform();

            return worldTransform_.Scale();
        }
        /// Return world space transform matrix.
        const Matrix3x4& GetWorldTransform() const
        {
            if (dirty_)
                UpdateWorldTransform();

            return worldTransform_;
        }
        /// Set position in parent space silently without marking the node & child nodes dirty. Used by animation code.
        void SetPositionSilent(const Vector3& position) { position_ = position; }
        /// Set position in parent space silently without marking the node & child nodes dirty. Used by animation code.
        void SetRotationSilent(const Quaternion& rotation) { rotation_ = rotation; }
        /// Set scale in parent space silently without marking the node & child nodes dirty. Used by animation code.
        void SetScaleSilent(const Vector3& scale) { scale_ = scale; }
        /// Set local transform silently without marking the node & child nodes dirty. Used by animation code.
        void SetTransformSilent(const Vector3& position, const Quaternion& rotation, const Vector3& scale);
        /// Convert a local space position to world space.
        Vector3 LocalToWorld(const Vector3& position) const;
        /// Convert a local space position or rotation to world space.
        Vector3 LocalToWorld(const Vector4& vector) const;
        /// Convert a local space position or rotation to world space.
        Vector2 LocalToWorld2D(const Vector2& vector) const;
        /// Convert a world space position to local space.
        Vector3 WorldToLocal(const Vector3& position) const;
        /// Convert a world space position or rotation to local space.
        Vector3 WorldToLocal(const Vector4& vector) const;
        /// Convert a world space position or rotation to local space.
        Vector2 WorldToLocal2D(const Vector2& vector) const;

        /// Return whether is a direct or indirect child of specified node.
        bool IsChildOf(Node* node) const;
        /// Return parent scene node.
        Node* GetParent() const { return parent_; }
        /// Return scene.
        Scene* GetScene() const { return scene_; }
        /// Return whether transform has changed and world transform needs recalculation.
        bool IsDirty() const { return dirty_; }
        /// Return number of child scene nodes.
        unsigned GetNumChildren(bool recursive = false) const;
        /// Return immediate child scene nodes.
        const Vector<SharedPtr<Node> >& GetChildren() const { return children_; }
        /// Return child scene nodes, optionally recursive.
        void GetChildren(PODVector<Node*>& dest, bool recursive = false) const;
        /// Return child scene nodes, optionally recursive.
        PODVector<Node*> GetChildren(bool recursive) const;
        /// Return child scene nodes with a specific component.
        void GetChildrenWithComponent(PODVector<Node*>& dest, StringHash type, bool recursive = false) const;
        /// Return child scene nodes with a specific component.
        PODVector<Node*> GetChildrenWithComponent(StringHash type, bool recursive = false) const;
        /// Return child scene nodes with a specific tag.
        void GetChildrenWithTag(PODVector<Node*>& dest, const String& tag, bool recursive = false) const;
        /// Return child scene nodes with a specific tag.
        PODVector<Node*> GetChildrenWithTag(const String& tag, bool recursive = false) const;
        /// Return child scene node by index.
        Node* GetChild(unsigned index) const;
        /// Return child scene node by name.
        Node* GetChild(const String& name, bool recursive = false) const;
        /// Return child scene node by name.
        Node* GetChild(const char* name, bool recursive = false) const;
        /// Return child scene node by name hash.
        Node* GetChild(StringHash nameHash, bool recursive = false) const;
        /// Return number of non-local components.
        unsigned GetNumNetworkComponents() const;
        /// Return number of components.
        unsigned GetNumComponents() const { return components_.Size(); }
        /// Return all components.
        const Vector<SharedPtr<Component>>& GetComponents() const { return components_; }
        /// Return all components of type. Optionally recursive.
        void GetComponents(PODVector<Component*>& dest, StringHash type, bool recursive = false) const;
        /// Return component by type. If there are several, returns the first.
        Component* GetComponent(StringHash type, bool recursive = false) const;
        /// Return component in parent node. If there are several, returns the first. May optional traverse up to the root node.
        Component* GetParentComponent(StringHash type, bool fullTraversal = false) const;
        /// Return whether has a specific component.
        bool HasComponent(StringHash type) const;
        /// Return listener components.
        Vector<WeakPtr<Component>> GetListeners() const { return listeners_; }
        /// Return a user variable.
        const Variant& GetVar(StringHash key) const;
        /// Return all user variables.
        const VariantMap& GetVars() const { return vars_; }
        /// Set ID. Called by Scene.
        void SetID(unsigned id);
        /// Set scene. Called by Scene.
        void SetScene(Scene* scene);
        /// Reset scene, ID and owner. Called by Scene.
        void ResetScene();
        /// Set network position attribute.
        void SetNetPositionAttr(const Vector3& value);
        /// Set network rotation attribute.
        void SetNetRotationAttr(const PODVector<unsigned char>& value);
        /// Set network parent attribute.
        void SetNetParentAttr(const PODVector<unsigned char>& value);
        /// Return network position attribute.
        const Vector3& GetNetPositionAttr() const;
        /// Return network rotation attribute.
        const PODVector<unsigned char>& GetNetRotationAttr() const;
        /// Return network parent attribute.
        const PODVector<unsigned char>& GetNetParentAttr() const;
        /// Return the depended on nodes to order network updates.
        const PODVector<Node*>& GetDependencyNodes() const { return impl_->dependencyNodes_; }
        /// Prepare network update by comparing attributes and marking replication states dirty as necessary.
        void PrepareNetworkUpdate();
        /// Clean up all references to a network connection that is about to be removed.
        void CleanupConnection(Connection* connection);
        /// Mark node dirty in scene replication states.
        void MarkReplicationDirty();
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
        /// Adjust index order of an existing component in this node.
        void ReorderComponent(Component* component, unsigned index);
        /// Clone scene node, components and child nodes. Return the clone.
        Node* Clone(CreateMode mode = REPLICATED);
        /// Remove from the parent node. If no other shared pointer references exist, causes immediate deletion.
        void Remove();
        /// Assign to a new parent scene node. Retains the world transform.
        void SetParent(Node* parent);
        /// Set a user variable.
        void SetVar(StringHash key, const Variant& value);
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
        /// Create a component to this node (with specified ID if provided).
        Component* CreateComponent(StringHash type, CreateMode mode = REPLICATED, unsigned id = 0);
        /// Create a component to this node if it does not exist already.
        Component* GetOrCreateComponent(StringHash type, CreateMode mode = REPLICATED, unsigned id = 0);
        /// Clone a component from another node using its create mode. Return the clone if successful or null on failure.
        Component* CloneComponent(Component* component, unsigned id = 0);
        /// Clone a component from another node and specify the create mode. Return the clone if successful or null on failure.
        Component* CloneComponent(Component* component, CreateMode mode, unsigned id = 0);
        /// Create a child node with specific ID.
        Node* CreateChild(unsigned id, CreateMode mode, bool temporary = false);
        /// Add a pre-created component. Using this function from application code is discouraged, as component operation without an owner node may not be well-defined in all cases. Prefer CreateComponent() instead.
        void AddComponent(Component* component, unsigned id, CreateMode mode);
        /// Calculate number of non-temporary child nodes.
        unsigned GetNumPersistentChildren() const;
        /// Calculate number of non-temporary components.
        unsigned GetNumPersistentComponents() const;
        /// Load components and optionally load child nodes.
        bool Load(Deserializer& source, SceneResolver& resolver, bool loadChildren = true, bool rewriteIDs = false, CreateMode mode = REPLICATED);
        /// Load components from XML data and optionally load child nodes.
        bool LoadXML(const XMLElement& source, SceneResolver& resolver, bool loadChildren = true, bool rewriteIDs = false, CreateMode mode = REPLICATED);


        /// Template version of creating a component.
        template <typename T> T* CreateComponent(CreateMode mode = REPLICATED, unsigned id = 0);
        /// Template version of getting or creating a component.
        template <typename T> T* GetOrCreateComponent(CreateMode mode = REPLICATED, unsigned id = 0);
        /// Template version of removing a component.
        template <typename T> void RemoveComponent();
        /// Template version of removing all components of specific type.
        template <typename T> void RemoveComponents();
        /// Return first component derived from class.
        template <typename T> T* GetDerivedComponent(bool recursive = false) const;
        /// Return first component derived from class in the parent node, or if fully traversing then the first node up the tree with one.
        template <typename T> T* GetParentDerivedComponent(bool fullTraversal = false) const;
        /// Return components derived from class.
        template <typename T> void GetDerivedComponents(PODVector<T*>& dest, bool recursive = false, bool clearVector = true) const;
        /// Template version of returning child nodes with a specific component.
        template <typename T> void GetChildrenWithComponent(PODVector<Node*>& dest, bool recursive = false) const;
        /// Template version of returning a component by type.
        template <typename T> T* GetComponent(bool recursive = false) const;
        /// Template version of returning a parent's component by type.
        template <typename T> T* GetParentComponent(bool fullTraversal = false) const;
        /// Template version of returning all components of type.
        template <typename T> void GetComponents(PODVector<T*>& dest, bool recursive = false) const;
        /// Template version of checking whether has a specific component.
        template <typename T> bool HasComponent() const;

    protected:
        /// Handle attribute animation added.
        void OnAttributeAnimationAdded() override;
        /// Handle attribute animation removed.
        void OnAttributeAnimationRemoved() override;
        /// Find target of an attribute animation from object hierarchy by name.
        Animatable* FindAttributeAnimationTarget(const String& name, String& outName) override;

    private:
        /// Set enabled/disabled state with optional recursion. Optionally affect the remembered enable state.
        void SetEnabled(bool enable, bool recursive, bool storeSelf);
        /// Create component, allowing UnknownComponent if actual type is not supported. Leave typeName empty if not known.
        Component* SafeCreateComponent(const String& typeName, StringHash type, CreateMode mode, unsigned id);
        /// Recalculate the world transform.
        void UpdateWorldTransform() const;
        /// Remove a component from this node with the specified iterator.
        void RemoveComponent(Vector<SharedPtr<Component> >::Iterator i);
        /// Remove child node by iterator.
        void RemoveChild(Vector<SharedPtr<Node> >::Iterator i);
        /// Return child nodes recursively.
        void GetChildrenRecursive(PODVector<Node*>& dest) const;
        /// Return child nodes with a specific component recursively.
        void GetChildrenWithComponentRecursive(PODVector<Node*>& dest, StringHash type) const;
        /// Return child nodes with a specific tag recursively.
        void GetChildrenWithTagRecursive(PODVector<Node*>& dest, const String& tag) const;
        /// Return specific components recursively.
        void GetComponentsRecursive(PODVector<Component*>& dest, StringHash type) const;
        /// Clone node recursively.
        Node* CloneRecursive(Node* parent, SceneResolver& resolver, CreateMode mode);
        /// Handle attribute animation update event.
        void HandleAttributeAnimationUpdate(StringHash eventType, VariantMap& eventData);
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

    template <typename T> T* Node::CreateComponent(CreateMode mode, unsigned id)
    {
        return static_cast<T*>(CreateComponent(T::GetTypeStatic(), mode, id));
    }

    template <typename T> T* Node::GetOrCreateComponent(CreateMode mode, unsigned id)
    {
        return static_cast<T*>(GetOrCreateComponent(T::GetTypeStatic(), mode, id));
    }

    template <typename T> void Node::RemoveComponent() { RemoveComponent(T::GetTypeStatic()); }

    template <typename T> void Node::RemoveComponents() { RemoveComponents(T::GetTypeStatic()); }

    template <typename T> void Node::GetChildrenWithComponent(PODVector<Node*>& dest, bool recursive) const
    {
        GetChildrenWithComponent(dest, T::GetTypeStatic(), recursive);
    }

    template <typename T> T* Node::GetComponent(bool recursive) const { return static_cast<T*>(GetComponent(T::GetTypeStatic(), recursive)); }

    template <typename T> T* Node::GetParentComponent(bool fullTraversal) const { return static_cast<T*>(GetParentComponent(T::GetTypeStatic(), fullTraversal)); }

    template <typename T> void Node::GetComponents(PODVector<T*>& dest, bool recursive) const
    {
        GetComponents(reinterpret_cast<PODVector<Component*>&>(dest), T::GetTypeStatic(), recursive);
    }

    template <typename T> bool Node::HasComponent() const { return HasComponent(T::GetTypeStatic()); }

    template <typename T> T* Node::GetDerivedComponent(bool recursive) const
    {
        for (Vector<SharedPtr<Component> >::ConstIterator i = components_.Begin(); i != components_.End(); ++i)
        {
            auto* component = dynamic_cast<T*>(i->Get());
            if (component)
                return component;
        }

        if (recursive)
        {
            for (Vector<SharedPtr<Node> >::ConstIterator i = children_.Begin(); i != children_.End(); ++i)
            {
                T* component = (*i)->GetDerivedComponent<T>(true);
                if (component)
                    return component;
            }
        }

        return nullptr;
    }

    template <typename T> T* Node::GetParentDerivedComponent(bool fullTraversal) const
    {
        Node* current = GetParent();
        while (current)
        {
            T* soughtComponent = current->GetDerivedComponent<T>();
            if (soughtComponent)
                return soughtComponent;

            if (fullTraversal)
                current = current->GetParent();
            else
                break;
        }
        return 0;
    }

    template <typename T> void Node::GetDerivedComponents(PODVector<T*>& dest, bool recursive, bool clearVector) const
    {
        if (clearVector)
            dest.Clear();

        for (Vector<SharedPtr<Component> >::ConstIterator i = components_.Begin(); i != components_.End(); ++i)
        {
            auto* component = dynamic_cast<T*>(i->Get());
            if (component)
                dest.Push(component);
        }

        if (recursive)
        {
            for (Vector<SharedPtr<Node> >::ConstIterator i = children_.Begin(); i != children_.End(); ++i)
                (*i)->GetDerivedComponents<T>(dest, true, false);
        }
    }
}
