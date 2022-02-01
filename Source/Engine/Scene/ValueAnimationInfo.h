//
// Created by luchu on 2022/1/22.
//

#pragma once

#include "Container/Ptr.h"
#include "Container/RefCounted.h"
#include "Container/Vector.h"
#include "Scene/AnimationDefs.h"


namespace My3D
{
    class Object;
    class ValueAnimation;
    class Variant;
    struct VAnimEventFrame;

    /// Base class for a value animation instance, which includes animation runtime information and updates the target object's value automatically
    class MY3D_API ValueAnimationInfo : public RefCounted
    {
    public:
        /// Construct without target object
        ValueAnimationInfo(ValueAnimation* animation, WrapMode wrapMode, float speed);
        /// Construct with target object
        ValueAnimationInfo(Object* target, ValueAnimation* animation, WrapMode wrapMode, float speed);
        /// Copy construct
        ValueAnimationInfo(const ValueAnimationInfo& other);
        /// Destruct
        ~ValueAnimationInfo() override;

        /// Advance time position and apply. Return true when the animation is finished. No-op when the target object is not defined.
        bool Update(float timeStep);
        /// Set time position and apply. Return true when the animation is finished. No-op when the target object is not defined.
        bool SetTime(float time);

    protected:
        /// Target object.
        WeakPtr<Object> target_;
        /// Attribute animation.
        SharedPtr<ValueAnimation> animation_;
        /// Wrap mode.
        WrapMode wrapMode_;
        /// Animation speed.
        float speed_;
        /// Current time.
        float currentTime_;
        /// Last scaled time.
        float lastScaledTime_;
    };

}

