//
// Created by luchu on 2022/5/8.
//

#include "IO/Log.h"
#include "Graphics/Skeleton.h"


namespace My3D
{
    Skeleton::Skeleton() : rootBoneIndex_(M_MAX_UNSIGNED) {}

    Skeleton::~Skeleton() = default;

    bool Skeleton::Load(Deserializer &source)
    {
        ClearBones();

        if (source.IsEof())
            return false;

        unsigned bones = source.ReadUInt();
        bones_.Reserve(bones);

        for (unsigned i = 0; i < bones; ++i)
        {
            Bone newBone;
            newBone.name_ = source.ReadString();
            newBone.nameHash_ = newBone.name_;
            newBone.parentIndex_ = source.ReadUInt();
            newBone.initialPosition_ = source.ReadVector3();
            newBone.initialRotation_ = source.ReadQuaternion();
            newBone.initialScale_ = source.ReadVector3();
            source.Read(&newBone.offsetMatrix_.m00_, sizeof(Matrix3x4));

            // Read bone collision data
            newBone.collisionMask_ = BoneCollisionShapeFlags(source.ReadUByte());
            if (newBone.collisionMask_ & BONECOLLISION_SPHERE)
                newBone.radius_ = source.ReadFloat();
            if (newBone.collisionMask_ & BONECOLLISION_BOX)
                newBone.boundingBox_ = source.ReadBoundingBox();

            if (newBone.parentIndex_ == i)
                rootBoneIndex_ = i;

            bones_.Push(newBone);
        }

        return true;
    }

    bool Skeleton::Save(Serializer& dest) const
    {
        if (!dest.WriteUInt(bones_.Size()))
            return false;

        for (unsigned i = 0; i < bones_.Size(); ++i)
        {
            const Bone& bone = bones_[i];
            dest.WriteString(bone.name_);
            dest.WriteUInt(bone.parentIndex_);
            dest.WriteVector3(bone.initialPosition_);
            dest.WriteQuaternion(bone.initialRotation_);
            dest.WriteVector3(bone.initialScale_);
            dest.Write(bone.offsetMatrix_.Data(), sizeof(Matrix3x4));

            // Collision info
            dest.WriteUByte(bone.collisionMask_);
            if (bone.collisionMask_ & BONECOLLISION_SPHERE)
                dest.WriteFloat(bone.radius_);
            if (bone.collisionMask_ & BONECOLLISION_BOX)
                dest.WriteBoundingBox(bone.boundingBox_);
        }

        return true;
    }

    void Skeleton::Define(const Skeleton& src)
    {
        ClearBones();

        bones_ = src.bones_;
        // Make sure we clear node references, if they exist
        // (AnimatedModel will create new nodes on its own)
        for (Vector<Bone>::Iterator i = bones_.Begin(); i != bones_.End(); ++i)
            i->node_.Reset();
        rootBoneIndex_ = src.rootBoneIndex_;
    }

    void Skeleton::SetRootBoneIndex(unsigned index)
    {
        if (index < bones_.Size())
            rootBoneIndex_ = index;
        else
            MY3D_LOGERROR("Root bone index out of bounds");
    }

    void Skeleton::ClearBones()
    {
        bones_.Clear();
        rootBoneIndex_ = M_MAX_UNSIGNED;
    }

    void Skeleton::Reset()
    {
        for (Vector<Bone>::Iterator i = bones_.Begin(); i != bones_.End(); ++i)
        {
            if (i->animated_ && i->node_)
                i->node_->SetTransform(i->initialPosition_, i->initialRotation_, i->initialScale_);
        }
    }

    void Skeleton::ResetSilent()
    {
        for (Vector<Bone>::Iterator i = bones_.Begin(); i != bones_.End(); ++i)
        {
            if (i->animated_ && i->node_)
                i->node_->SetTransformSilent(i->initialPosition_, i->initialRotation_, i->initialScale_);
        }
    }


    Bone* Skeleton::GetRootBone()
    {
        return GetBone(rootBoneIndex_);
    }

    unsigned Skeleton::GetBoneIndex(const StringHash& boneNameHash) const
    {
        const unsigned numBones = bones_.Size();
        for (unsigned i = 0; i < numBones; ++i)
        {
            if (bones_[i].nameHash_ == boneNameHash)
                return i;
        }

        return M_MAX_UNSIGNED;
    }

    unsigned Skeleton::GetBoneIndex(const Bone* bone) const
    {
        if (bones_.Empty() || bone < &bones_.Front() || bone > &bones_.Back())
            return M_MAX_UNSIGNED;

        return static_cast<unsigned>(bone - &bones_.Front());
    }

    unsigned Skeleton::GetBoneIndex(const String& boneName) const
    {
        return GetBoneIndex(StringHash(boneName));
    }

    Bone* Skeleton::GetBoneParent(const Bone* bone)
    {
        if (GetBoneIndex(bone) == bone->parentIndex_)
            return nullptr;
        else
            return GetBone(bone->parentIndex_);
    }

    Bone* Skeleton::GetBone(unsigned index)
    {
        return index < bones_.Size() ? &bones_[index] : nullptr;
    }

    Bone* Skeleton::GetBone(const String& name)
    {
        return GetBone(StringHash(name));
    }

    Bone* Skeleton::GetBone(const char* name)
    {
        return GetBone(StringHash(name));
    }

    Bone* Skeleton::GetBone(const StringHash& boneNameHash)
    {
        const unsigned index = GetBoneIndex(boneNameHash);
        return index < bones_.Size() ? &bones_[index] : nullptr;
    }
}