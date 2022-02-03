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

        /// Set position in parent space. If the scene node is on the root level (is child of the scene itself), this is same as world space.
        void SetPosition(const Vector3& position);

    protected:
        /// Handle attribute animation added.
        void OnAttributeAnimationAdded() override;
        /// Handle attribute animation removed.
        void OnAttributeAnimationRemoved() override;

    private:
        /// Enabled flag.
        bool enabled_;
    };
}
