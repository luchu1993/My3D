//
// Created by luchu on 2022/1/22.
//

#pragma once

#include "Container/HashSet.h"
#include "Scene/Serializable.h"
#include "Scene/ValueAnimationInfo.h"


namespace My3D
{
    class Animatable;
    class ValueAnimation;
    class ObjectAnimation;

    /// Attribute animation instance
    class AttributeAnimationInfo : public ValueAnimationInfo
    {
    public:
        /// Construct
        AttributeAnimationInfo(Animatable* animatable, const AttributeInfo& attributeInfo, ValueAnimation* attributeAnimation, WrapMode wrapMode, float speed);
        /// Copy construct
        AttributeAnimationInfo(const AttributeAnimationInfo& other);
        /// Destruct
        ~AttributeAnimationInfo() override;
        /// Return attribute information
        const AttributeInfo& GetAttributeInfo() const { return attributeInfo_; }

    private:
        /// Apply new animation value to the target object. Called by update.
        void ApplyValue(const Variant& newValue) override;

    private:
        /// Attribute information
        const AttributeInfo& attributeInfo_;
    };

    /// Base class for animatable object, an animatable object can be set animation on it's attributes, or can be set an object animation to it.
    class MY3D_API Animatable : public Serializable
    {
        MY3D_OBJECT(Animatable, Serializable)

    public:
        /// Construct
        explicit Animatable(Context* context);
        /// Destruct
        ~Animatable() override;
        /// Register object factory
        static void RegisterObject(Context* context);

        /// Load from XML data. Return true if successful
        bool LoadXML(const XMLElement& source) override;
        /// Save as XML data. Return true if successful
        bool SaveXML(XMLElement& dest) const override;
        /// Set automatic update of animation, default true
        void SetAnimationEnabled(bool enable);
        /// Set time position of all attribute animations or an object animation manually. Automatic update should be disabled in this case.
        void SetAnimationTime(float time);

        /// Set object animation
        void SetObjectAnimation(ObjectAnimation* objectAnimation);
        /// Set attribute animation
        void SetAttributeAnimation(const String& name, ValueAnimation* attributeAnimation, WrapMode wrapMode = WM_LOOP, float speed = 1.0f);
        /// Set attribute animation wrap mode
        void SetAttributeAnimationWrapMode(const String& name, WrapMode wrapMode);
        /// Set attribute animation speed
        void SetAttributeAnimationSpeed(const String& name, float speed);
        /// Set attribute animation time position manually. Automatic update should be disabled in this case.
        void SetAttributeAnimationTime(const String& name, float time);
        /// Remove object animation. Same as calling SetObjectAnimation with a null pointer
        void RemoveObjectAnimation();
        /// Remove attribute animation. Same as calling SetAttributeAnimation with a null pointer.
        void RemoveAttributeAnimation(const String& name);

        /// Return animation enabled
        bool GetAnimationEnabled() const { return animationEnabled_; }
        /// Return object animation
        ObjectAnimation* GetObjectAnimation() const;

        /// Set object animation attribute.
        void SetObjectAnimationAttr(const ResourceRef& value);
        /// Return object animation attribute.
        ResourceRef GetObjectAnimationAttr() const;

    protected:
        /// Handle attribute animation added.
        virtual void OnAttributeAnimationAdded() = 0;
        /// Handle attribute animation removed.
        virtual void OnAttributeAnimationRemoved() = 0;
        /// Find target of an attribute animation from object hierarchy by name.
        virtual Animatable* FindAttributeAnimationTarget(const String& name, String& outName);
        /// Set object attribute animation internal.
        void SetObjectAttributeAnimation(const String& name, ValueAnimation* attributeAnimation, WrapMode wrapMode, float speed);
        /// Handle object animation added.
        void OnObjectAnimationAdded(ObjectAnimation* objectAnimation);
        /// Handle object animation removed.
        void OnObjectAnimationRemoved(ObjectAnimation* objectAnimation);
        /// Return attribute animation info.
        AttributeAnimationInfo* GetAttributeAnimationInfo(const String& name) const;
        /// Handle attribute animation added.
        void HandleAttributeAnimationAdded(StringHash eventType, VariantMap& eventData);
        /// Handle attribute animation removed.
        void HandleAttributeAnimationRemoved(StringHash eventType, VariantMap& eventData);

        /// Animation enabled
        bool animationEnabled_;
        /// Animation
        SharedPtr<ObjectAnimation> objectAnimation_;
        /// Animated network attribute set
        HashSet<const AttributeInfo*> animatedNetworkAttributes_;
        /// Attribute animation infos
        HashMap<String, SharedPtr<AttributeAnimationInfo>> attributeAnimationInfos_;
    };

}

