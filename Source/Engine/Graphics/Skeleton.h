//
// Created by luchu on 2022/5/8.
//

#pragma once

#include "Container/Vector.h"
#include "Scene/Node.h"
#include "Math/BoundingBox.h"


namespace My3D
{

    enum BoneCollisionShape : unsigned char
    {
        BONECOLLISION_NONE = 0x0,
        BONECOLLISION_SPHERE = 0x1,
        BONECOLLISION_BOX = 0x2,
    };
    MY3D_FLAGSET(BoneCollisionShape, BoneCollisionShapeFlags);

    /// Bone in a skeleton
    struct Bone
    {
        /// Bone name
        String name_;
        /// Bone name hash
        StringHash nameHash_;
        /// Parent bone index
        unsigned parentIndex_ {};
        /// Reset position
        Vector3 initialPosition_ {Vector3::ZERO};
        /// Reset rotation
        Quaternion initialRotation_ {Quaternion::IDENTITY};
        /// Reset scale
        Vector3 initialScale_ {Vector3::ONE};
        /// Offset matrix
        Matrix3x4 offsetMatrix_;
        /// Animation enable flag
        bool animated_ {true};
        /// Supported collision types.
        BoneCollisionShapeFlags collisionMask_ = BONECOLLISION_NONE;
        /// Radius
        float radius_ {};
        /// Local-space bounding box.
        BoundingBox boundingBox_;
        /// Scene node
        WeakPtr<Node> node_;
    };

    /// Hierarchical collection of bones.
    class MY3D_API Skeleton
    {
        /// Construct an empty skeleton.
        Skeleton();
        /// Destruct.
        ~Skeleton();

        /// Read from a stream. Return true if successful.
        bool Load(Deserializer& source);
        /// Write to a stream. Return true if successful.
        bool Save(Serializer& dest) const;
        /// Define from another skeleton.
        void Define(const Skeleton& src);
        /// Set root bone's index.
        void SetRootBoneIndex(unsigned index);
        /// Clear bones.
        void ClearBones();
        /// Reset all animating bones to initial positions.
        void Reset();

        /// Return all bones.
        const Vector<Bone>& GetBones() const { return bones_; }

        /// Return modifiable bones.
        Vector<Bone>& GetModifiableBones() { return bones_; }

        /// Return number of bones.
        /// @property
        unsigned GetNumBones() const { return bones_.Size(); }

        /// Return root bone.
        /// @property
        Bone* GetRootBone();
        /// Return index of the bone by name. Return M_MAX_UNSIGNED if not found.
        unsigned GetBoneIndex(const String& boneName) const;
        /// Return index of the bone by name hash. Return M_MAX_UNSIGNED if not found.
        unsigned GetBoneIndex(const StringHash& boneNameHash) const;
        /// Return index of the bone by the bone pointer. Return M_MAX_UNSIGNED if not found.
        unsigned GetBoneIndex(const Bone* bone) const;
        /// Return parent of the given bone. Return null for root bones.
        Bone* GetBoneParent(const Bone* bone);
        /// Return bone by index.
        /// @property{get_bones}
        Bone* GetBone(unsigned index);
        /// Return bone by name.
        Bone* GetBone(const String& name);
        /// Return bone by name.
        Bone* GetBone(const char* name);
        /// Return bone by name hash.
        Bone* GetBone(const StringHash& boneNameHash);

        /// Reset all animating bones to initial positions without marking the nodes dirty. Requires the node dirtying to be performed later.
        void ResetSilent();

    private:
        /// Bones.
        Vector<Bone> bones_;
        /// Root bone index.
        unsigned rootBoneIndex_;
    };
}
