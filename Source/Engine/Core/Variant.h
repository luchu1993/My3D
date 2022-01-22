//
// Created by luchu on 2022/1/9.
//

#pragma once

#include "Container/HashMap.h"
#include "Container/Ptr.h"
#include "Core/StringHash.h"
#include "Math/Vector4.h"
#include "Math/Matrix2.h"
#include "Math/Matrix4.h"
#include "Math/Quaternion.h"
#include "Math/Sphere.h"
#include "Math/Plane.h"
#include "Math/Ray.h"
#include "Math/BoundingBox.h"
#include "Math/Color.h"



namespace My3D
{
    /// VariantType
    enum VariantType
    {
        VAR_NONE = 0,
        VAR_INT,
        VAR_BOOL,
        VAR_FLOAT,
        VAR_VECTOR2,
        VAR_VECTOR3,
        VAR_VECTOR4,
        VAR_QUATERNION,
        VAR_COLOR,
        VAR_STRING,
        VAR_BUFFER,
        VAR_VOIDPTR,
        VAR_RESOURCEREF,
        VAR_RESOURCEREFLIST,
        VAR_VARIANTVECTOR,
        VAR_VARIANTMAP,
        VAR_INTRECT,
        VAR_INTVECTOR2,
        VAR_PTR,
        VAR_MATRIX2,
        VAR_MATRIX3,
        VAR_MATRIX3X4,
        VAR_MATRIX4,
        VAR_DOUBLE,
        VAR_STRINGVECTOR,
        VAR_RECT,
        VAR_INTVECTOR3,
        VAR_INT64,
        // Add new types here
        VAR_CUSTOM_HEAP,
        VAR_CUSTOM_STACK,
        MAX_VAR_TYPES
    };

    class Variant;
    class VectorBuffer;
    /// Vector of variants
    using VariantVector = Vector<Variant>;
    /// Vector of string
    using StringVector = Vector<String>;
    /// Map of variants
    using VariantMap = HashMap<StringHash, Variant>;
    /// Size of variant value. 16 bytes on 32-bit platform, 32 bytes on 64-bit platform.
    static const unsigned VARIANT_VALUE_SIZE = sizeof(void*) * 4;
    /// Union for the possible variant values. Objects exceeding the VARIANT_VALUE_SIZE are allocated on the heap.
    union VariantValue
    {
        unsigned char storage_[VARIANT_VALUE_SIZE];
        int int_;
        bool bool_;
        float float_;
        double double_;
        long long int64_;
        void* voidPtr_;
        WeakPtr<RefCounted> weakPtr_;
        Vector2 vector2_;
        Vector3 vector3_;
        Vector4 vector4_;
        Rect rect_;
        IntVector2 intVector2_;
        IntVector3 intVector3_;
        Matrix2* matrix2_;
        Matrix3* matrix3_;
        Matrix3x4* matrix3x4_;
        Matrix4* matrix4_;
        Color color_;
        String string_;
        StringVector stringVector_;
        VariantVector variantVector_;
        VariantMap variantMap_;
        PODVector<unsigned char> buffer_;

        /// Construct uninitialized.
        VariantValue() { }
        /// Non-copyable.
        VariantValue(const VariantValue& value) = delete;
        VariantValue& operator=(const VariantType&) = delete;
        /// Destruct.
        ~VariantValue() { }
    };

    static_assert(sizeof(VariantValue) == VARIANT_VALUE_SIZE, "Unexpected size of VariantValue");

    /// Variable that supports a fixed set of types
    class MY3D_API Variant
    {
    public:
        /// Construct
        Variant() = default;
        /// Construct from integer
        Variant(int value) { *this = value; }
        /// Construct from 64 bit integer
        Variant(long long value) { *this = value; }
        /// Construct from unsigned integer
        Variant(unsigned  value) { *this = value; }
        /// Construct from unsigned long
        Variant(unsigned long long value) { *this = value; }
        /// Construct from a string hash (convert to unsigned)
        Variant(const StringHash& value) { *this = value.Value(); }
        /// Construct from a bool
        Variant(bool value) { *this = value; }
        /// Construct from a float
        Variant(float value) { *this = value; }
        /// Construct from a double
        Variant(double value) { *this = value; }
        /// Construct from Vector2
        Variant(const Vector2& value) { *this = value; }
        /// Construct from Vector3
        Variant(const Vector3& value) { *this = value; }
        /// Construct from Vector4
        Variant(const Vector4& value) { *this = value; }
        /// Construct from a quaternion.
        Variant(const Quaternion& value) { *this = value; }
        /// Construct from a color
        Variant(const Color& value) { *this = value; }
        /// Construct from a string
        Variant(const String& value) { *this = value; }
        /// Construct from a C string
        Variant(const char* value) { *this = value; }
        /// Construct from a buffer
        Variant(const PODVector<unsigned char>& value) { *this = value; }
        /// Construct from a pointer
        Variant(void* value) { *this = value; }
        /// Construct from a variant vector
        Variant(const VariantVector& value) { *this = value; }
        /// Construct from a variant map.
        Variant(const VariantMap& value) { *this = value; }
        /// Construct from a string vector
        Variant(const StringVector& value) { *this = value; }
        /// Construct from a rect
        Variant(const Rect& value) { *this = value; }
        /// Construct from an integer rect.
        Variant(const IntRect& value) { *this = value; }
        /// Construct from an IntVector2
        Variant(const IntVector2& value) { *this = value; }
        /// Construct from an IntVector3
        Variant(const IntVector3& value) { *this = value; }
        /// Construct from a Matrix2
        Variant(const Matrix2& value) { *this = value; }
        /// Construct from a Matrix3
        Variant(const Matrix3& value) { *this = value; }
        /// Construct from a Matrix3x4
        Variant(const Matrix3x4& value) { *this = value; }
        /// Construct from a Matrix4
        Variant(const Matrix4& value) { *this = value; }
        /// Construct from type and value
        Variant(const String& type, const String& value)
        {
            FromString(type, value);
        }
        /// Construct from type and value
        Variant(VariantType type, const String& value)
        {
            FromString(type, value);
        }
        /// Construct from type and value
        Variant(const char* type, const char* value)
        {
            FromString(type, value);
        }
        /// Construct from type and value.
        Variant(VariantType type, const char* value)
        {
            FromString(type, value);
        }
        /// Copy-construct from another variant.
        Variant(const Variant& value)
        {
            *this = value;
        }
        /// Destruct.
        ~Variant()
        {
            SetType(VAR_NONE);
        }
        /// Reset to empty.
        void Clear()
        {
            SetType(VAR_NONE);
        }
        /// Assign from another variant.
        Variant& operator =(const Variant& rhs);
        /// Assign from an integer.
        Variant& operator =(int rhs)
        {
            SetType(VAR_INT);
            value_.int_ = rhs;
            return *this;
        }
        /// Assign from 64 bit integer.
        Variant& operator =(long long rhs)
        {
            SetType(VAR_INT64);
            value_.int64_ = rhs;
            return *this;
        }
        /// Assign from unsigned 64 bit integer.
        Variant& operator =(unsigned long long rhs)
        {
            SetType(VAR_INT64);
            value_.int64_ = static_cast<long long>(rhs);
            return *this;
        }
        /// Assign from an unsigned integer.
        Variant& operator =(unsigned rhs)
        {
            SetType(VAR_INT);
            value_.int_ = (int)rhs;
            return *this;
        }
        /// Assign from a StringHash (convert to integer).
        Variant& operator =(const StringHash& rhs)
        {
            SetType(VAR_INT);
            value_.int_ = (int)rhs.Value();
            return *this;
        }
        /// Assign from a bool.
        Variant& operator =(bool rhs)
        {
            SetType(VAR_BOOL);
            value_.bool_ = rhs;
            return *this;
        }
        /// Assign from a float.
        Variant& operator =(float rhs)
        {
            SetType(VAR_FLOAT);
            value_.float_ = rhs;
            return *this;
        }
        /// Assign from a double.
        Variant& operator =(double rhs)
        {
            SetType(VAR_DOUBLE);
            value_.double_ = rhs;
            return *this;
        }
        /// Assign from a Vector2.
        Variant& operator =(const Vector2& rhs)
        {
            SetType(VAR_VECTOR2);
            value_.vector2_ = rhs;
            return *this;
        }
        /// Assign from a Vector3.
        Variant& operator =(const Vector3& rhs)
        {
            SetType(VAR_VECTOR3);
            value_.vector3_ = rhs;
            return *this;
        }
        /// Assign from a Vector4.
        Variant& operator =(const Vector4& rhs)
        {
            SetType(VAR_VECTOR4);
            value_.vector4_ = rhs;
            return *this;
        }
        /// Assign from a color.
        Variant& operator =(const Color& rhs)
        {
            SetType(VAR_COLOR);
            value_.color_ = rhs;
            return *this;
        }
        /// Assign from a string.
        Variant& operator =(const String& rhs)
        {
            SetType(VAR_STRING);
            value_.string_ = rhs;
            return *this;
        }
        /// Assign from a C string.
        Variant& operator =(const char* rhs)
        {
            SetType(VAR_STRING);
            value_.string_ = rhs;
            return *this;
        }
        /// Assign from a buffer.
        Variant& operator =(const PODVector<unsigned char>& rhs)
        {
            SetType(VAR_BUFFER);
            value_.buffer_ = rhs;
            return *this;
        }
        /// Assign from a void pointer.
        Variant& operator =(void* rhs)
        {
            SetType(VAR_VOIDPTR);
            value_.voidPtr_ = rhs;
            return *this;
        }
        /// Assign from a variant vector.
        Variant& operator =(const VariantVector& rhs)
        {
            SetType(VAR_VARIANTVECTOR);
            value_.variantVector_ = rhs;
            return *this;
        }
        /// Assign from a string vector.
        Variant& operator =(const StringVector& rhs)
        {
            SetType(VAR_STRINGVECTOR);
            value_.stringVector_ = rhs;
            return *this;
        }

        /// Assign from a variant map.
        Variant& operator =(const VariantMap& rhs)
        {
            SetType(VAR_VARIANTMAP);
            value_.variantMap_ = rhs;
            return *this;
        }

        /// Assign from a rect.
        Variant& operator =(const Rect& rhs)
        {
            SetType(VAR_RECT);
            value_.rect_ = rhs;
            return *this;
        }
        /// Assign from an IntVector2.
        Variant& operator =(const IntVector2& rhs)
        {
            SetType(VAR_INTVECTOR2);
            value_.intVector2_ = rhs;
            return *this;
        }
        /// Assign from an IntVector3.
        Variant& operator =(const IntVector3& rhs)
        {
            SetType(VAR_INTVECTOR3);
            value_.intVector3_ = rhs;
            return *this;
        }
        /// Assign from a RefCounted pointer. The object will be stored internally in a WeakPtr so that its expiration can be detected safely.
        Variant& operator =(RefCounted* rhs)
        {
            SetType(VAR_PTR);
            value_.weakPtr_ = rhs;
            return *this;
        }
        /// Assign from a Matrix2.
        Variant& operator =(const Matrix2& rhs)
        {
            SetType(VAR_MATRIX2);
            *value_.matrix2_ = rhs;
            return *this;
        }
        /// Assign from a Matrix3.
        Variant& operator =(const Matrix3& rhs)
        {
            SetType(VAR_MATRIX3);
            *value_.matrix3_ = rhs;
            return *this;
        }
        /// Assign from a Matrix3x4.
        Variant& operator =(const Matrix3x4& rhs)
        {
            SetType(VAR_MATRIX3X4);
            *value_.matrix3x4_ = rhs;
            return *this;
        }
        /// Assign from a Matrix4.
        Variant& operator =(const Matrix4& rhs)
        {
            SetType(VAR_MATRIX4);
            *value_.matrix4_ = rhs;
            return *this;
        }
        /// Test for equality with another variant.
        bool operator ==(const Variant& rhs) const;
        /// Test for equality with an integer. To return true, both the type and value must match.
        bool operator ==(int rhs) const { return type_ == VAR_INT && value_.int_ == rhs; }
        /// Test for equality with an unsigned 64 bit integer. To return true, both the type and value must match.
        bool operator ==(unsigned rhs) const { return type_ == VAR_INT && value_.int_ == static_cast<int>(rhs); }
        /// Test for equality with an 64 bit integer. To return true, both the type and value must match.
        bool operator ==(long long rhs) const { return type_ == VAR_INT64 && value_.int64_ == rhs; }
        /// Test for equality with an unsigned integer. To return true, both the type and value must match.
        bool operator ==(unsigned long long rhs) const
        {
            return type_ == VAR_INT64 && value_.int64_ == static_cast<long long>(rhs);
        }
        /// Test for equality with a bool. To return true, both the type and value must match.
        bool operator ==(bool rhs) const { return type_ == VAR_BOOL && value_.bool_ == rhs; }
        /// Test for equality with a float. To return true, both the type and value must match.
        bool operator ==(float rhs) const { return type_ == VAR_FLOAT && value_.float_ == rhs; }
        /// Test for equality with a double. To return true, both the type and value must match.
        bool operator ==(double rhs) const { return type_ == VAR_DOUBLE && value_.double_ == rhs; }
        /// Test for equality with a Vector2. To return true, both the type and value must match.
        bool operator ==(const Vector2& rhs) const
        {
            return type_ == VAR_VECTOR2 && value_.vector2_ == rhs;
        }
        /// Test for equality with a Vector3. To return true, both the type and value must match.
        bool operator ==(const Vector3& rhs) const
        {
            return type_ == VAR_VECTOR3 && value_.vector3_ == rhs;
        }
        /// Test for equality with a Vector4. To return true, both the type and value must match.
        bool operator ==(const Vector4& rhs) const
        {
            return type_ == VAR_VECTOR4 && value_.vector4_ == rhs;
        }
        /// Test for equality with a color. To return true, both the type and value must match.
        bool operator ==(const Color& rhs) const
        {
            return type_ == VAR_COLOR && value_.color_ == rhs;
        }
        /// Test for equality with a string. To return true, both the type and value must match.
        bool operator ==(const String& rhs) const
        {
            return type_ == VAR_STRING && value_.string_ == rhs;
        }
        /// Test for equality with a void pointer. To return true, both the type and value must match, with the exception that a RefCounted pointer is also allowed.
        bool operator ==(void* rhs) const
        {
            if (type_ == VAR_VOIDPTR)
                return value_.voidPtr_ == rhs;
            else if (type_ == VAR_PTR)
                return value_.weakPtr_ == rhs;
            else
                return false;
        }
        /// Test for equality with a variant vector. To return true, both the type and value must match.
        bool operator ==(const VariantVector& rhs) const
        {
            return type_ == VAR_VARIANTVECTOR && value_.variantVector_ == rhs;
        }
        /// Test for equality with a string vector. To return true, both the type and value must match.
        bool operator ==(const StringVector& rhs) const
        {
            return type_ == VAR_STRINGVECTOR && value_.stringVector_ == rhs;
        }
        /// Test for equality with a variant map. To return true, both the type and value must match.
        bool operator ==(const VariantMap& rhs) const
        {
            return type_ == VAR_VARIANTMAP && value_.variantMap_ == rhs;
        }
        /// Test for equality with a rect. To return true, both the type and value must match.
        bool operator ==(const Rect& rhs) const
        {
            return type_ == VAR_RECT && value_.rect_ == rhs;
        }
        /// Test for equality with an IntVector2. To return true, both the type and value must match.
        bool operator ==(const IntVector2& rhs) const
        {
            return type_ == VAR_INTVECTOR2 && value_.intVector2_ == rhs;
        }

        /// Test for equality with an IntVector3. To return true, both the type and value must match.
        bool operator ==(const IntVector3& rhs) const
        {
            return type_ == VAR_INTVECTOR3 && value_.intVector3_ == rhs;
        }
        /// Test for equality with a StringHash. To return true, both the type and value must match.
        bool operator ==(const StringHash& rhs) const { return type_ == VAR_INT && static_cast<unsigned>(value_.int_) == rhs.Value(); }
        /// Test for equality with a RefCounted pointer. To return true, both the type and value must match, with the exception that void pointer is also allowed.
        bool operator ==(RefCounted* rhs) const
        {
            if (type_ == VAR_PTR)
                return value_.weakPtr_ == rhs;
            else if (type_ == VAR_VOIDPTR)
                return value_.voidPtr_ == rhs;
            else
                return false;
        }
        /// Test for equality with a Matrix3. To return true, both the type and value must match.
        bool operator ==(const Matrix3& rhs) const
        {
            return type_ == VAR_MATRIX3 && *value_.matrix3_ == rhs;
        }
        /// Test for equality with a Matrix3x4. To return true, both the type and value must match.
        bool operator ==(const Matrix3x4& rhs) const
        {
            return type_ == VAR_MATRIX3X4 && *value_.matrix3x4_ == rhs;
        }
        /// Test for equality with a Matrix4. To return true, both the type and value must match.
        bool operator ==(const Matrix4& rhs) const
        {
            return type_ == VAR_MATRIX4 && *value_.matrix4_ == rhs;
        }
        /// Test for equality with a buffer. To return true, both the type and value must match.
        bool operator ==(const PODVector<unsigned char>& rhs) const;

        /// Test for inequality with another variant.
        bool operator !=(const Variant& rhs) const { return !(*this == rhs); }
        /// Test for inequality with an integer.
        bool operator !=(int rhs) const { return !(*this == rhs); }
        /// Test for inequality with an unsigned integer.
        bool operator !=(unsigned rhs) const { return !(*this == rhs); }
        /// Test for inequality with an 64 bit integer.
        bool operator !=(long long rhs) const { return !(*this == rhs); }
        /// Test for inequality with an unsigned 64 bit integer.
        bool operator !=(unsigned long long rhs) const { return !(*this == rhs); }
        /// Test for inequality with a bool.
        bool operator !=(bool rhs) const { return !(*this == rhs); }
        /// Test for inequality with a float.
        bool operator !=(float rhs) const { return !(*this == rhs); }
        /// Test for inequality with a double.
        bool operator !=(double rhs) const { return !(*this == rhs); }
        /// Test for inequality with a Vector2.
        bool operator !=(const Vector2& rhs) const { return !(*this == rhs); }
        /// Test for inequality with a Vector3.
        bool operator !=(const Vector3& rhs) const { return !(*this == rhs); }
        /// Test for inequality with an Vector4.
        bool operator !=(const Vector4& rhs) const { return !(*this == rhs); }
        /// Test for inequality with a variant vector.
        bool operator !=(const VariantVector& rhs) const { return !(*this == rhs); }
        /// Test for inequality with a string vector.
        bool operator !=(const StringVector& rhs) const { return !(*this == rhs); }
        /// Test for inequality with a variant map.
        bool operator !=(const VariantMap& rhs) const { return !(*this == rhs); }
        /// Test for inequality with a rect.
        bool operator !=(const Rect& rhs) const { return !(*this == rhs); }
        /// Test for inequality with a string.
        bool operator !=(const String& rhs) const { return !(*this == rhs); }
        /// Test for inequality with a buffer.
        bool operator !=(const PODVector<unsigned char>& rhs) const { return !(*this == rhs); }
        /// Test for inequality with a pointer.
        bool operator !=(void* rhs) const { return !(*this == rhs); }
        /// Test for inequality with an IntVector2.
        bool operator !=(const IntVector2& rhs) const { return !(*this == rhs); }
        /// Test for inequality with an IntVector3.
        bool operator !=(const IntVector3& rhs) const { return !(*this == rhs); }
        /// Test for inequality with a StringHash.
        bool operator !=(const StringHash& rhs) const { return !(*this == rhs); }
        /// Test for inequality with a RefCounted pointer.
        bool operator !=(RefCounted* rhs) const { return !(*this == rhs); }
        /// Test for inequality with a Matrix3.
        bool operator !=(const Matrix3& rhs) const { return !(*this == rhs); }
        /// Test for inequality with a Matrix3x4.
        bool operator !=(const Matrix3x4& rhs) const { return !(*this == rhs); }
        /// Test for inequality with a Matrix4.
        bool operator !=(const Matrix4& rhs) const { return !(*this == rhs); }
        /// Return void pointer or null on type mismatch. RefCounted pointer will be converted.
        void* GetVoidPtr() const
        {
            if (type_ == VAR_VOIDPTR)
                return value_.voidPtr_;
            else if (type_ == VAR_PTR)
                return value_.weakPtr_;
            else
                return nullptr;
        }
        /// Return int or zero on type mismatch. Floats and doubles are converted.
        int GetInt() const
        {
            if (type_ == VAR_INT)
                return value_.int_;
            else if (type_ == VAR_FLOAT)
                return static_cast<int>(value_.float_);
            else if (type_ == VAR_DOUBLE)
                return static_cast<int>(value_.double_);
            else
                return 0;
        }
        /// Return 64 bit int or zero on type mismatch. Floats and doubles are converted.
        long long GetInt64() const
        {
            if (type_ == VAR_INT64)
                return value_.int64_;
            else if (type_ == VAR_INT)
                return value_.int_;
            else if (type_ == VAR_FLOAT)
                return static_cast<long long>(value_.float_);
            else if (type_ == VAR_DOUBLE)
                return static_cast<long long>(value_.double_);
            else
                return 0;
        }
        /// Return unsigned 64 bit int or zero on type mismatch. Floats and doubles are converted.
        unsigned long long GetUInt64() const
        {
            if (type_ == VAR_INT64)
                return static_cast<unsigned long long>(value_.int64_);
            else if (type_ == VAR_INT)
                return static_cast<unsigned long long>(value_.int_);
            else if (type_ == VAR_FLOAT)
                return static_cast<unsigned long long>(value_.float_);
            else if (type_ == VAR_DOUBLE)
                return static_cast<unsigned long long>(value_.double_);
            else
                return 0;
        }
        /// Return unsigned int or zero on type mismatch. Floats and doubles are converted.
        unsigned GetUInt() const
        {
            if (type_ == VAR_INT)
                return static_cast<unsigned>(value_.int_);
            else if (type_ == VAR_FLOAT)
                return static_cast<unsigned>(value_.float_);
            else if (type_ == VAR_DOUBLE)
                return static_cast<unsigned>(value_.double_);
            else
                return 0;
        }
        /// Return StringHash or zero on type mismatch.
        StringHash GetStringHash() const { return StringHash(GetUInt()); }
        /// Return bool or false on type mismatch.
        bool GetBool() const { return type_ == VAR_BOOL ? value_.bool_ : false; }
        /// Return float or zero on type mismatch. Ints and doubles are converted.
        float GetFloat() const
        {
            if (type_ == VAR_FLOAT)
                return value_.float_;
            else if (type_ == VAR_DOUBLE)
                return static_cast<float>(value_.double_);
            else if (type_ == VAR_INT)
                return static_cast<float>(value_.int_);
            else if (type_ == VAR_INT64)
                return static_cast<float>(value_.int64_);
            else
                return 0.0f;
        }
        /// Return double or zero on type mismatch. Ints and floats are converted.
        double GetDouble() const
        {
            if (type_ == VAR_DOUBLE)
                return value_.double_;
            else if (type_ == VAR_FLOAT)
                return static_cast<double>(value_.float_);
            else if (type_ == VAR_INT)
                return static_cast<double>(value_.int_);
            else if (type_ == VAR_INT64)
                return static_cast<double>(value_.int64_);
            else
                return 0.0;
        }
        /// Return Vector2 or zero on type mismatch.
        const Vector2& GetVector2() const { return type_ == VAR_VECTOR2 ? value_.vector2_ : Vector2::ZERO; }
        /// Return Vector3 or zero on type mismatch.
        const Vector3& GetVector3() const { return type_ == VAR_VECTOR3 ? value_.vector3_ : Vector3::ZERO; }
        /// Return Vector4 or zero on type mismatch.
        const Vector4& GetVector4() const { return type_ == VAR_VECTOR4 ? value_.vector4_ : Vector4::ZERO; }
        /// Return color or default on type mismatch. Vector4 is aliased to Color if necessary.
        const Color& GetColor() const { return (type_ == VAR_COLOR || type_ == VAR_VECTOR4) ? value_.color_ : Color::WHITE; }
        /// Return string or empty on type mismatch.
        const String& GetString() const { return type_ == VAR_STRING ? value_.string_ : String::EMPTY; }
        /// Return buffer or empty on type mismatch.
        const PODVector<unsigned char>& GetBuffer() const
        {
            return type_ == VAR_BUFFER ? value_.buffer_ : emptyBuffer;
        }
        /// Return a variant vector or empty on type mismatch.
        const VariantVector& GetVariantVector() const
        {
            return type_ == VAR_VARIANTVECTOR ? value_.variantVector_ : emptyVariantVector;
        }
        /// Return a string vector or empty on type mismatch.
        const StringVector& GetStringVector() const
        {
            return type_ == VAR_STRINGVECTOR ? value_.stringVector_ : emptyStringVector;
        }
        /// Return a variant map or empty on type mismatch.
        const VariantMap& GetVariantMap() const
        {
            return type_ == VAR_VARIANTMAP ? value_.variantMap_ : emptyVariantMap;
        }
        /// Return a rect or empty on type mismatch.
        const Rect& GetRect() const { return type_ == VAR_RECT ? value_.rect_ : Rect::ZERO; }
        /// Return an IntVector2 or empty on type mismatch.
        const IntVector2& GetIntVector2() const
        {
            return type_ == VAR_INTVECTOR2 ? value_.intVector2_ : IntVector2::ZERO;
        }
        /// Return an IntVector3 or empty on type mismatch.
        const IntVector3& GetIntVector3() const
        {
            return type_ == VAR_INTVECTOR3 ? value_.intVector3_ : IntVector3::ZERO;
        }
        /// Return a RefCounted pointer or null on type mismatch. Will return null if holding a void pointer, as it can not be safely verified that the object is a RefCounted.
        RefCounted* GetPtr() const
        {
            return type_ == VAR_PTR ? value_.weakPtr_ : nullptr;
        }
        /// Return a Matrix3 or identity on type mismatch.
        const Matrix3& GetMatrix3() const
        {
            return type_ == VAR_MATRIX3 ? *value_.matrix3_ : Matrix3::IDENTITY;
        }
        /// Return a Matrix3x4 or identity on type mismatch.
        const Matrix3x4& GetMatrix3x4() const
        {
            return type_ == VAR_MATRIX3X4 ? *value_.matrix3x4_ : Matrix3x4::IDENTITY;
        }
        /// Return a Matrix4 or identity on type mismatch.
        const Matrix4& GetMatrix4() const
        {
            return type_ == VAR_MATRIX4 ? *value_.matrix4_ : Matrix4::IDENTITY;
        }
        /// Return value's type.
        VariantType GetType() const { return type_; }
        /// Return value's type name.
        String GetTypeName() const;
        /// Convert value to string. Pointers are returned as null, and VariantBuffer or VariantMap are not supported and return empty.
        String ToString() const;
        /// Return true when the variant value is considered zero according to its actual type.
        bool IsZero() const;
        /// Return true when the variant is empty (i.e. not initialized yet).
        bool IsEmpty() const { return type_ == VAR_NONE; }
        /// Return true when the variant stores custom type.
        bool IsCustom() const { return type_ == VAR_CUSTOM_HEAP || type_ == VAR_CUSTOM_STACK; }
        /// Return the value, template version.
        template <class T> T Get() const;
        /// Return a pointer to a modifiable buffer or null on type mismatch.
        PODVector<unsigned char>* GetBufferPtr()
        {
            return type_ == VAR_BUFFER ? &value_.buffer_ : nullptr;
        }
        /// Set from typename and value strings. Pointers will be set to null, and VariantBuffer or VariantMap types are not supported.
        void FromString(const String& type, const String& value);
        /// Set from typename and value strings. Pointers will be set to null, and VariantBuffer or VariantMap types are not supported.
        void FromString(const char* type, const char* value);
        /// Set from type and value string. Pointers will be set to null, and VariantBuffer or VariantMap types are not supported.
        void FromString(VariantType type, const String& value);
        /// Set from type and value string. Pointers will be set to null, and VariantBuffer or VariantMap types are not supported.
        void FromString(VariantType type, const char* value);

        /// Return name for variant type
        static String GetTypeName(VariantType type);
        /// Return variant type from type name
        static VariantType GetTypeFromName(const String& typeName);
        /// Return variant type form type name
        static VariantType GetTypeFromName(const char* typeName);

        /// Empty variant.
        static const Variant EMPTY;
        /// Empty buffer.
        static const PODVector<unsigned char> emptyBuffer;
        /// Empty variant map.
        static const VariantMap emptyVariantMap;
        /// Empty variant vector.
        static const VariantVector emptyVariantVector;
        /// Empty string vector.
        static const StringVector emptyStringVector;
    private:
        /// Set new type and allocate/deallocate memory as necessary
        void SetType(VariantType newType);
        /// Variant type
        VariantType type_ = VAR_NONE;
        /// Variant value
        VariantValue value_;
    };

    /// Return variant type from type.
    template <typename T> VariantType GetVariantType();
    // Return variant type from concrete types
    template <> inline VariantType GetVariantType<int>() { return VAR_INT; }
    template <> inline VariantType GetVariantType<unsigned>() { return VAR_INT; }
    template <> inline VariantType GetVariantType<long long>() { return VAR_INT64; }
    template <> inline VariantType GetVariantType<unsigned long long>() { return VAR_INT64; }
    template <> inline VariantType GetVariantType<bool>() { return VAR_BOOL; }
    template <> inline VariantType GetVariantType<float>() { return VAR_FLOAT; }
    template <> inline VariantType GetVariantType<double>() { return VAR_DOUBLE; }
    template <> inline VariantType GetVariantType<Vector2>() { return VAR_VECTOR2; }
    template <> inline VariantType GetVariantType<Vector3>() { return VAR_VECTOR3; }
    template <> inline VariantType GetVariantType<Vector4>() { return VAR_VECTOR4; }
    template <> inline VariantType GetVariantType<Color>() { return VAR_COLOR; }
    template <> inline VariantType GetVariantType<String>() { return VAR_STRING; }
    template <> inline VariantType GetVariantType<StringHash>() { return VAR_INT; }
    template <> inline VariantType GetVariantType<PODVector<unsigned char> >() { return VAR_BUFFER; }
    template <> inline VariantType GetVariantType<VariantVector>() { return VAR_VARIANTVECTOR; }
    template <> inline VariantType GetVariantType<StringVector>() { return VAR_STRINGVECTOR; }
    template <> inline VariantType GetVariantType<VariantMap>() { return VAR_VARIANTMAP; }
    template <> inline VariantType GetVariantType<Rect>() { return VAR_RECT; }
    template <> inline VariantType GetVariantType<IntVector2>() { return VAR_INTVECTOR2; }
    template <> inline VariantType GetVariantType<IntVector3>() { return VAR_INTVECTOR3; }
    template <> inline VariantType GetVariantType<Matrix3>() { return VAR_MATRIX3; }
    template <> inline VariantType GetVariantType<Matrix3x4>() { return VAR_MATRIX3X4; }
    template <> inline VariantType GetVariantType<Matrix4>() { return VAR_MATRIX4; }

    // Specializations of Variant::Get<T>
    template <> MY3D_API int Variant::Get<int>() const;
    template <> MY3D_API unsigned Variant::Get<unsigned>() const;
    template <> MY3D_API long long Variant::Get<long long>() const;
    template <> MY3D_API unsigned long long Variant::Get<unsigned long long>() const;
    template <> MY3D_API StringHash Variant::Get<StringHash>() const;
    template <> MY3D_API bool Variant::Get<bool>() const;
    template <> MY3D_API float Variant::Get<float>() const;
    template <> MY3D_API double Variant::Get<double>() const;
    template <> MY3D_API const Vector2& Variant::Get<const Vector2&>() const;
    template <> MY3D_API const Vector3& Variant::Get<const Vector3&>() const;
    template <> MY3D_API const Vector4& Variant::Get<const Vector4&>() const;
    template <> MY3D_API const Color& Variant::Get<const Color&>() const;
    template <> MY3D_API const String& Variant::Get<const String&>() const;
    template <> MY3D_API const Rect& Variant::Get<const Rect&>() const;
    template <> MY3D_API const IntVector2& Variant::Get<const IntVector2&>() const;
    template <> MY3D_API const IntVector3& Variant::Get<const IntVector3&>() const;
    template <> MY3D_API const PODVector<unsigned char>& Variant::Get<const PODVector<unsigned char>&>() const;
    template <> MY3D_API void* Variant::Get<void*>() const;
    template <> MY3D_API RefCounted* Variant::Get<RefCounted*>() const;
    template <> MY3D_API const Matrix3& Variant::Get<const Matrix3&>() const;
    template <> MY3D_API const Matrix3x4& Variant::Get<const Matrix3x4&>() const;
    template <> MY3D_API const Matrix4& Variant::Get<const Matrix4&>() const;

    template <> MY3D_API VariantVector Variant::Get<VariantVector>() const;
    template <> MY3D_API StringVector Variant::Get<StringVector>() const;
    template <> MY3D_API VariantMap Variant::Get<VariantMap>() const;
    template <> MY3D_API Vector2 Variant::Get<Vector2>() const;
    template <> MY3D_API Vector3 Variant::Get<Vector3>() const;
    template <> MY3D_API Vector4 Variant::Get<Vector4>() const;
    template <> MY3D_API Color Variant::Get<Color>() const;
    template <> MY3D_API String Variant::Get<String>() const;
    template <> MY3D_API Rect Variant::Get<Rect>() const;
    template <> MY3D_API IntVector2 Variant::Get<IntVector2>() const;
    template <> MY3D_API IntVector3 Variant::Get<IntVector3>() const;
    template <> MY3D_API PODVector<unsigned char> Variant::Get<PODVector<unsigned char> >() const;
    template <> MY3D_API Matrix3 Variant::Get<Matrix3>() const;
    template <> MY3D_API Matrix3x4 Variant::Get<Matrix3x4>() const;
    template <> MY3D_API Matrix4 Variant::Get<Matrix4>() const;
}



