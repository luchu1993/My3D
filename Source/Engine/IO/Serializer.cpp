//
// Created by luchu on 2022/1/13.
//

#include "IO/Serializer.h"
#include "Math/MathDefs.h"


namespace My3D
{
    Serializer::~Serializer() = default;

    bool Serializer::WriteInt64(long long value)
    {
        return Write(&value, sizeof(value)) == sizeof(value);
    }

    bool Serializer::WriteInt(int value)
    {
        return Write(&value, sizeof(value)) == sizeof(value);
    }

    bool Serializer::WriteShort(short value)
    {
        return Write(&value, sizeof(value)) == sizeof(value);
    }

    bool Serializer::WriteByte(signed char value)
    {
        return Write(&value, sizeof(value)) == sizeof(value);
    }

    bool Serializer::WriteUInt64(unsigned long long int value)
    {
        return Write(&value, sizeof(value)) == sizeof(value);
    }

    bool Serializer::WriteUInt(unsigned int value)
    {
        return Write(&value, sizeof(value)) == sizeof(value);
    }

    bool Serializer::WriteUShort(unsigned short value)
    {
        return Write(&value, sizeof(value)) == sizeof(value);
    }

    bool Serializer::WriteUByte(unsigned char value)
    {
        return Write(&value, sizeof(value)) == sizeof(value);
    }

    bool Serializer::WriteBool(bool value)
    {
        return WriteUByte((unsigned char)(value ? 1 : 0)) == 1;
    }

    bool Serializer::WriteFloat(float value)
    {
        return Write(&value, sizeof(value)) == sizeof(value);
    }

    bool Serializer::WriteDouble(double value)
    {
        return Write(&value, sizeof(value)) == sizeof(value);
    }

    bool Serializer::WriteIntVector2(const IntVector2 &value)
    {
        return Write(value.Data(), sizeof(value)) == sizeof(value);
    }

    bool Serializer::WriteIntVector3(const IntVector3 &value)
    {
        return Write(value.Data(), sizeof(value)) == sizeof(value);
    }

    bool Serializer::WriteRect(const Rect &value)
    {
        return Write(value.Data(), sizeof(value)) == sizeof(value);
    }

    bool Serializer::WriteVector2(const Vector2 &value)
    {
        return Write(value.Data(), sizeof(value)) == sizeof(value);
    }

    bool Serializer::WriteVector3(const Vector3 &value)
    {
        return Write(value.Data(), sizeof(value)) == sizeof(value);
    }

    bool Serializer::WritePackedVector3(const Vector3 &value, float maxAbsCoord)
    {
        short coords[3];
        float v = M_MAX_SHORT / maxAbsCoord;
        coords[0] = (short) Round(Clamp(value.x_, -maxAbsCoord, maxAbsCoord) * v);
        coords[1] = (short) Round(Clamp(value.y_, -maxAbsCoord, maxAbsCoord) * v);
        coords[2] = (short) Round(Clamp(value.z_, -maxAbsCoord, maxAbsCoord) * v);

        return Write(&coords[0], sizeof(coords)) == sizeof(coords);
    }

    bool Serializer::WriteVector4(const Vector4& value)
    {
        return Write(value.Data(), sizeof(value)) == sizeof(value);
    }

    bool Serializer::WriteQuaternion(const Quaternion& value)
    {
        return Write(value.Data(), sizeof(value)) == sizeof(value);
    }

    bool Serializer::WritePackedQuaternion(const Quaternion& value)
    {
        short coords[4];
        Quaternion norm = value.Normalized();

        coords[0] = (short)Round(Clamp(norm.w_, -1.0f, 1.0f) * M_MAX_SHORT);
        coords[1] = (short)Round(Clamp(norm.x_, -1.0f, 1.0f) * M_MAX_SHORT);
        coords[2] = (short)Round(Clamp(norm.y_, -1.0f, 1.0f) * M_MAX_SHORT);
        coords[3] = (short)Round(Clamp(norm.z_, -1.0f, 1.0f) * M_MAX_SHORT);
        return Write(&coords[0], sizeof(coords)) == sizeof(coords);
    }
    bool Serializer::WriteMatrix3(const Matrix3& value)
    {
        return Write(value.Data(), sizeof value) == sizeof value;
    }

    bool Serializer::WriteMatrix3x4(const Matrix3x4& value)
    {
        return Write(value.Data(), sizeof value) == sizeof value;
    }

    bool Serializer::WriteMatrix4(const Matrix4& value)
    {
        return Write(value.Data(), sizeof value) == sizeof value;
    }

    bool Serializer::WriteColor(const Color& value)
    {
        return Write(value.Data(), sizeof value) == sizeof value;
    }

    bool Serializer::WriteBoundingBox(const BoundingBox &value)
    {
        bool success = true;
        success &= WriteVector3(value.min_);
        success &= WriteVector3(value.max_);
        return success;
    }

    bool Serializer::WriteString(const String &value)
    {
        const char* chars = value.CString();
        unsigned length = String::CStringLength(chars);
        return Write(chars, length + 1) == length + 1;
    }

    bool Serializer::WriteFileID(const String &value)
    {
        bool success = true;
        unsigned length = Min(value.Length(), 4U);

        success &= Write(value.CString(), length) == length;
        for (unsigned  i = value.Length(); i < 4; ++i)
            success &= WriteByte(' ');
        return success;
    }

    bool Serializer::WriteStringHash(const StringHash &value)
    {
        return WriteUInt(value.Value());
    }

    bool Serializer::WriteBuffer(const PODVector<unsigned char> &value)
    {
        bool success = true;
        unsigned size = value.Size();

        success &= WriteVLE(size);
        if (size)
            success &= Write(&value[0], size) == size;
        return success;
    }

    bool Serializer::WriteResourceRef(const ResourceRef &value)
    {
        return false;
    }

    bool Serializer::WriteResourceRefList(const ResourceRefList &value)
    {
        return false;
    }

    bool Serializer::WriteVLE(unsigned int value)
    {
        return false;
    }

    bool Serializer::WriteLine(const String &value)
    {
        bool success = true;
        success &= Write(value.CString(), value.Length()) == value.Length();
        success &= WriteUByte(13);
        success &= WriteUByte(10);
        return success;
    }
}
