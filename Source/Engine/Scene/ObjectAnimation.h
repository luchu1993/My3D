//
// Created by luchu on 2022/2/2.
//

#pragma once

#include "Resource/Resource.h"
#include "Scene/AnimationDefs.h"

namespace My3D
{
    class ValueAnimation;
    class ValueAnimationInfo;
    class XMLElement;

    /// Object animation class, an object animation include one or more attribute animations and theirs wrap mode and speed for an Animatable object.
    class MY3D_API ObjectAnimation : public Resource
    {
        MY3D_OBJECT(ObjectAnimation, Resource);

    public:
        /// Construct
        explicit ObjectAnimation(Context* context);
        /// Destruct
        ~ObjectAnimation() override;
        /// Register object factory
        static void RegisterObject(Context* context);
        /// Load resource from stream. May be called from a worker thread. Return true if successful.
        bool BeginLoad(Deserializer& source) override;
        /// Save resource. Return true if successful.
        bool Save(Serializer& dest) const override;
        /// Load from XML data. Return true if successful.
        bool LoadXML(const XMLElement& source);
        /// Save as XML data. Return true if successful.
        bool SaveXML(XMLElement& dest) const;
        /// Add attribute animation, attribute name can in following format: "attribute" or "#0/#1/attribute" or ""#0/#1/@component#1/attribute.
        void AddAttributeAnimation(const String& name, ValueAnimation* attributeAnimation, WrapMode wrapMode = WM_LOOP, float speed = 1.0f);
        /// Remove attribute animation, attribute name can in following format: "attribute" or "#0/#1/attribute" or ""#0/#1/@component#1/attribute.
        void RemoveAttributeAnimation(const String& name);
        /// Remove attribute animation.
        void RemoveAttributeAnimation(ValueAnimation* attributeAnimation);
        /// Return attribute animation by name.
        ValueAnimation* GetAttributeAnimation(const String& name) const;
        /// Return attribute animation wrap mode by name.
        WrapMode GetAttributeAnimationWrapMode(const String& name) const;
        /// Return attribute animation speed by name.
        float GetAttributeAnimationSpeed(const String& name) const;
        /// Return all attribute animations infos.
        const HashMap<String, SharedPtr<ValueAnimationInfo> >& GetAttributeAnimationInfos() const { return attributeAnimationInfos_; }
        /// Return attribute animation info by name.
        ValueAnimationInfo* GetAttributeAnimationInfo(const String& name) const;

    private:
        /// Send attribute animation added event.
        void SendAttributeAnimationAddedEvent(const String& name);
        /// Send attribute animation remove event.
        void SendAttributeAnimationRemovedEvent(const String& name);
        /// Name to attribute animation info mapping.
        HashMap<String, SharedPtr<ValueAnimationInfo> > attributeAnimationInfos_;
    };
}
