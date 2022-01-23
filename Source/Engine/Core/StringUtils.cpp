//
// Created by luchu on 2022/1/9.
//

#include "Core/StringUtils.h"


namespace My3D
{
    unsigned CountElements(const char* buffer, char separator)
    {
        if (!buffer)
            return 0;

        const char* endPos = buffer + String::CStringLength(buffer);
        const char* pos = buffer;
        unsigned ret = 0;

        while (pos < endPos)
        {
            if (*pos != separator)
                break;
            ++pos;
        }

        while (pos < endPos)
        {
            const char* start = pos;

            while (start < endPos)
            {
                if (*start == separator)
                    break;

                ++start;
            }

            if (start == endPos)
            {
                ++ret;
                break;
            }

            const char* end = start;

            while (end < endPos)
            {
                if (*end != separator)
                    break;

                ++end;
            }

            ++ret;
            pos = end;
        }

        return ret;
    }

    bool ToBool(const String& source)
    {
        return ToBool(source.CString());
    }

    bool ToBool(const char* source)
    {
        unsigned length = String::CStringLength(source);

        for (unsigned i = 0; i < length; ++i)
        {
            auto c = (char)tolower(source[i]);
            if (c == 't' || c == 'y' || c == '1')
                return true;
            else if (c != ' ' && c != '\t')
                break;
        }

        return false;
    }

    int ToInt(const String& source, int base)
    {
        return ToInt(source.CString(), base);
    }

    int ToInt(const char* source, int base)
    {
        if (!source)
            return 0;

        // Shield against runtime library assert by converting illegal base values to 0 (autodetect)
        if (base < 2 || base > 36)
            base = 0;

        return (int)strtol(source, nullptr, base);
    }

    long long ToInt64(const char* source, int base)
    {
        if (!source)
            return 0;

        // Shield against runtime library assert by converting illegal base values to 0 (autodetect)
        if (base < 2 || base > 36)
            base = 0;

        return strtoll(source, nullptr, base);
    }

    long long ToInt64(const String& source, int base)
    {
        return ToInt64(source.CString(), base);
    }

    unsigned ToUInt(const String& source, int base)
    {
        return ToUInt(source.CString(), base);
    }

    unsigned long long ToUInt64(const char* source, int base)
    {
        if (!source)
            return 0;

        // Shield against runtime library assert by converting illegal base values to 0 (autodetect)
        if (base < 2 || base > 36)
            base = 0;

        return strtoull(source, nullptr, base);
    }

    unsigned long long ToUInt64(const String& source, int base)
    {
        return ToUInt64(source.CString(), base);
    }

    unsigned ToUInt(const char* source, int base)
    {
        if (!source)
            return 0;

        if (base < 2 || base > 36)
            base = 0;

        return (unsigned)strtoul(source, nullptr, base);
    }

    float ToFloat(const String& source)
    {
        return ToFloat(source.CString());
    }

    float ToFloat(const char* source)
    {
        if (!source)
            return 0;

        return (float)strtod(source, nullptr);
    }

    double ToDouble(const String& source)
    {
        return ToDouble(source.CString());
    }

    double ToDouble(const char* source)
    {
        if (!source)
            return 0;

        return strtod(source, nullptr);
    }

    Color ToColor(const String& source)
    {
        return ToColor(source.CString());
    }

    Color ToColor(const char* source)
    {
        Color ret;

        unsigned elements = CountElements(source, ' ');
        if (elements < 3)
            return ret;

        auto* ptr = (char*)source;
        ret.r_ = (float)strtod(ptr, &ptr);
        ret.g_ = (float)strtod(ptr, &ptr);
        ret.b_ = (float)strtod(ptr, &ptr);
        if (elements > 3)
            ret.a_ = (float)strtod(ptr, &ptr);

        return ret;
    }

    IntVector2 ToIntVector2(const String& source)
    {
        return ToIntVector2(source.CString());
    }

    IntVector2 ToIntVector2(const char* source)
    {
        IntVector2 ret(IntVector2::ZERO);

        unsigned elements = CountElements(source, ' ');
        if (elements < 2)
            return ret;

        auto* ptr = (char*)source;
        ret.x_ = (int)strtol(ptr, &ptr, 10);
        ret.y_ = (int)strtol(ptr, &ptr, 10);

        return ret;
    }

    IntVector3 ToIntVector3(const String& source)
    {
        return ToIntVector3(source.CString());
    }

    IntVector3 ToIntVector3(const char* source)
    {
        IntVector3 ret(IntVector3::ZERO);

        unsigned elements = CountElements(source, ' ');
        if (elements < 3)
            return ret;

        auto* ptr = (char*)source;
        ret.x_ = (int)strtol(ptr, &ptr, 10);
        ret.y_ = (int)strtol(ptr, &ptr, 10);
        ret.z_ = (int)strtol(ptr, &ptr, 10);

        return ret;
    }

    Rect ToRect(const String& source)
    {
        return ToRect(source.CString());
    }

    Rect ToRect(const char* source)
    {
        Rect ret(Rect::ZERO);

        unsigned elements = CountElements(source, ' ');
        if (elements < 4)
            return ret;

        auto* ptr = (char*)source;
        ret.min_.x_ = (float)strtod(ptr, &ptr);
        ret.min_.y_ = (float)strtod(ptr, &ptr);
        ret.max_.x_ = (float)strtod(ptr, &ptr);
        ret.max_.y_ = (float)strtod(ptr, &ptr);

        return ret;
    }

    Vector2 ToVector2(const String& source)
    {
        return ToVector2(source.CString());
    }

    Vector2 ToVector2(const char* source)
    {
        Vector2 ret(Vector2::ZERO);

        unsigned elements = CountElements(source, ' ');
        if (elements < 2)
            return ret;

        auto* ptr = (char*)source;
        ret.x_ = (float)strtod(ptr, &ptr);
        ret.y_ = (float)strtod(ptr, &ptr);

        return ret;
    }

    Vector3 ToVector3(const String& source)
    {
        return ToVector3(source.CString());
    }

    Vector3 ToVector3(const char* source)
    {
        Vector3 ret(Vector3::ZERO);

        unsigned elements = CountElements(source, ' ');
        if (elements < 3)
            return ret;

        auto* ptr = (char*)source;
        ret.x_ = (float)strtod(ptr, &ptr);
        ret.y_ = (float)strtod(ptr, &ptr);
        ret.z_ = (float)strtod(ptr, &ptr);

        return ret;
    }

    Vector4 ToVector4(const String& source, bool allowMissingCoords)
    {
        return ToVector4(source.CString(), allowMissingCoords);
    }

    Vector4 ToVector4(const char* source, bool allowMissingCoords)
    {
        Vector4 ret(Vector4::ZERO);

        unsigned elements = CountElements(source, ' ');
        auto* ptr = (char*)source;

        if (!allowMissingCoords)
        {
            if (elements < 4)
                return ret;

            ret.x_ = (float)strtod(ptr, &ptr);
            ret.y_ = (float)strtod(ptr, &ptr);
            ret.z_ = (float)strtod(ptr, &ptr);
            ret.w_ = (float)strtod(ptr, &ptr);

            return ret;
        }
        else
        {
            if (elements > 0)
                ret.x_ = (float)strtod(ptr, &ptr);
            if (elements > 1)
                ret.y_ = (float)strtod(ptr, &ptr);
            if (elements > 2)
                ret.z_ = (float)strtod(ptr, &ptr);
            if (elements > 3)
                ret.w_ = (float)strtod(ptr, &ptr);

            return ret;
        }
    }

    Variant ToVectorVariant(const String& source)
    {
        return ToVectorVariant(source.CString());
    }

    Variant ToVectorVariant(const char* source)
    {
        Variant ret;
        unsigned elements = CountElements(source, ' ');

        switch (elements)
        {
            case 1:
                ret.FromString(VAR_FLOAT, source);
                break;

            case 2:
                ret.FromString(VAR_VECTOR2, source);
                break;

            case 3:
                ret.FromString(VAR_VECTOR3, source);
                break;

            case 4:
                ret.FromString(VAR_VECTOR4, source);
                break;

            case 9:
                ret.FromString(VAR_MATRIX3, source);
                break;

            case 12:
                ret.FromString(VAR_MATRIX3X4, source);
                break;

            case 16:
                ret.FromString(VAR_MATRIX4, source);
                break;

            default:
                // Illegal input. Return variant remains empty
                break;
        }

        return ret;
    }

    Matrix3 ToMatrix3(const String& source)
    {
        return ToMatrix3(source.CString());
    }

    Matrix3 ToMatrix3(const char* source)
    {
        Matrix3 ret(Matrix3::ZERO);

        unsigned elements = CountElements(source, ' ');
        if (elements < 9)
            return ret;

        auto* ptr = (char*)source;
        ret.m00_ = (float)strtod(ptr, &ptr);
        ret.m01_ = (float)strtod(ptr, &ptr);
        ret.m02_ = (float)strtod(ptr, &ptr);
        ret.m10_ = (float)strtod(ptr, &ptr);
        ret.m11_ = (float)strtod(ptr, &ptr);
        ret.m12_ = (float)strtod(ptr, &ptr);
        ret.m20_ = (float)strtod(ptr, &ptr);
        ret.m21_ = (float)strtod(ptr, &ptr);
        ret.m22_ = (float)strtod(ptr, &ptr);

        return ret;
    }

    Matrix3x4 ToMatrix3x4(const String& source)
    {
        return ToMatrix3x4(source.CString());
    }

    Matrix3x4 ToMatrix3x4(const char* source)
    {
        Matrix3x4 ret(Matrix3x4::ZERO);

        unsigned elements = CountElements(source, ' ');
        if (elements < 12)
            return ret;

        auto* ptr = (char*)source;
        ret.m00_ = (float)strtod(ptr, &ptr);
        ret.m01_ = (float)strtod(ptr, &ptr);
        ret.m02_ = (float)strtod(ptr, &ptr);
        ret.m03_ = (float)strtod(ptr, &ptr);
        ret.m10_ = (float)strtod(ptr, &ptr);
        ret.m11_ = (float)strtod(ptr, &ptr);
        ret.m12_ = (float)strtod(ptr, &ptr);
        ret.m13_ = (float)strtod(ptr, &ptr);
        ret.m20_ = (float)strtod(ptr, &ptr);
        ret.m21_ = (float)strtod(ptr, &ptr);
        ret.m22_ = (float)strtod(ptr, &ptr);
        ret.m23_ = (float)strtod(ptr, &ptr);

        return ret;
    }

    Matrix4 ToMatrix4(const String& source)
    {
        return ToMatrix4(source.CString());
    }

    Matrix4 ToMatrix4(const char* source)
    {
        Matrix4 ret(Matrix4::ZERO);

        unsigned elements = CountElements(source, ' ');
        if (elements < 16)
            return ret;

        auto* ptr = (char*)source;
        ret.m00_ = (float)strtod(ptr, &ptr);
        ret.m01_ = (float)strtod(ptr, &ptr);
        ret.m02_ = (float)strtod(ptr, &ptr);
        ret.m03_ = (float)strtod(ptr, &ptr);
        ret.m10_ = (float)strtod(ptr, &ptr);
        ret.m11_ = (float)strtod(ptr, &ptr);
        ret.m12_ = (float)strtod(ptr, &ptr);
        ret.m13_ = (float)strtod(ptr, &ptr);
        ret.m20_ = (float)strtod(ptr, &ptr);
        ret.m21_ = (float)strtod(ptr, &ptr);
        ret.m22_ = (float)strtod(ptr, &ptr);
        ret.m23_ = (float)strtod(ptr, &ptr);
        ret.m30_ = (float)strtod(ptr, &ptr);
        ret.m31_ = (float)strtod(ptr, &ptr);
        ret.m32_ = (float)strtod(ptr, &ptr);
        ret.m33_ = (float)strtod(ptr, &ptr);

        return ret;
    }

    String ToString(void* value)
    {
        return ToStringHex((unsigned)(size_t)value);
    }

    String ToStringHex(unsigned value)
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%08x", value);
        return String(tempBuffer);
    }

    void BufferToString(String& dest, const void* data, unsigned size)
    {
        // Precalculate needed string size
        const auto* bytes = (const unsigned char*)data;
        unsigned length = 0;
        for (unsigned i = 0; i < size; ++i)
        {
            // Room for separator
            if (i)
                ++length;

            // Room for the value
            if (bytes[i] < 10)
                ++length;
            else if (bytes[i] < 100)
                length += 2;
            else
                length += 3;
        }

        dest.Resize(length);
        unsigned index = 0;

        // Convert values
        for (unsigned i = 0; i < size; ++i)
        {
            if (i)
                dest[index++] = ' ';

            if (bytes[i] < 10)
            {
                dest[index++] = '0' + bytes[i];
            }
            else if (bytes[i] < 100)
            {
                dest[index++] = (char)('0' + bytes[i] / 10);
                dest[index++] = (char)('0' + bytes[i] % 10);
            }
            else
            {
                dest[index++] = (char)('0' + bytes[i] / 100);
                dest[index++] = (char)('0' + bytes[i] % 100 / 10);
                dest[index++] = (char)('0' + bytes[i] % 10);
            }
        }
    }

    void StringToBuffer(PODVector<unsigned char>& dest, const String& source)
    {
        StringToBuffer(dest, source.CString());
    }

    void StringToBuffer(PODVector<unsigned char>& dest, const char* source)
    {
        if (!source)
        {
            dest.Clear();
            return;
        }

        unsigned size = CountElements(source, ' ');
        dest.Resize(size);

        bool inSpace = true;
        unsigned index = 0;
        unsigned value = 0;

        // Parse values
        const char* ptr = source;
        while (*ptr)
        {
            if (inSpace && *ptr != ' ')
            {
                inSpace = false;
                value = (unsigned)(*ptr - '0');
            }
            else if (!inSpace && *ptr != ' ')
            {
                value *= 10;
                value += *ptr - '0';
            }
            else if (!inSpace && *ptr == ' ')
            {
                dest[index++] = (unsigned char)value;
                inSpace = true;
            }

            ++ptr;
        }

        // Write the final value
        if (!inSpace && index < size)
            dest[index] = (unsigned char)value;
    }

    void Variant::SetType(VariantType newType)
    {
        if (type_ == newType)
            return;

        switch (type_)
        {
            case VAR_STRING:
                value_.string_.~String();
                break;

            case VAR_BUFFER:
                value_.buffer_.~PODVector<unsigned char>();
                break;

            case VAR_VARIANTVECTOR:
                value_.variantVector_.~VariantVector();
                break;

            case VAR_STRINGVECTOR:
                value_.stringVector_.~StringVector();
                break;

            case VAR_VARIANTMAP:
                value_.variantMap_.~VariantMap();
                break;

            case VAR_PTR:
                value_.weakPtr_.~WeakPtr<RefCounted>();
                break;

            case VAR_MATRIX3:
                delete value_.matrix3_;
                break;

            case VAR_MATRIX3X4:
                delete value_.matrix3x4_;
                break;

            case VAR_MATRIX4:
                delete value_.matrix4_;
                break;

            default:
                break;
        }

        type_ = newType;

        switch (type_)
        {
            case VAR_STRING:
                new(&value_.string_) String();
                break;

            case VAR_BUFFER:
                new(&value_.buffer_) PODVector<unsigned char>();
                break;

            case VAR_VARIANTVECTOR:
                new(&value_.variantVector_) VariantVector();
                break;

            case VAR_STRINGVECTOR:
                new(&value_.stringVector_) StringVector();
                break;

            case VAR_VARIANTMAP:
                new(&value_.variantMap_) VariantMap();
                break;

            case VAR_PTR:
                new(&value_.weakPtr_) WeakPtr<RefCounted>();
                break;

            case VAR_MATRIX3:
                value_.matrix3_ = new Matrix3();
                break;

            case VAR_MATRIX3X4:
                value_.matrix3x4_ = new Matrix3x4();
                break;

            case VAR_MATRIX4:
                value_.matrix4_ = new Matrix4();
                break;

            default:
                break;
        }
    }

    unsigned GetStringListIndex(const String& value, const String* strings, unsigned defaultIndex, bool caseSensitive)
    {
        return GetStringListIndex(value.CString(), strings, defaultIndex, caseSensitive);
    }

    unsigned GetStringListIndex(const char* value, const String* strings, unsigned defaultIndex, bool caseSensitive)
    {
        unsigned i = 0;
        while (!strings[i].Empty())
        {
            if (!strings[i].Compare(value, caseSensitive))
                return i;
            ++i;
        }
        return defaultIndex;
    }

    unsigned GetStringListIndex(const char* value, const char** strings, unsigned defaultIndex, bool caseSensitive)
    {
        unsigned i = 0;

        while (strings[i])
        {
            if (!String::Compare(value, strings[i], caseSensitive))
                return i;
            ++i;
        }

        return defaultIndex;
    }

    String ToString(const char* formatString, ...)
    {
        String ret;
        va_list args;
                va_start(args, formatString);
        ret.AppendWithFormatArgs(formatString, args);
                va_end(args);
        return ret;
    }

    bool IsAlpha(unsigned ch)
    {
        return ch < 256 && isalpha(ch) != 0;
    }

    bool IsDigit(unsigned ch)
    {
        return ch < 256 && isdigit(ch) != 0;
    }

    unsigned ToUpper(unsigned ch)
    {
        return (unsigned)toupper(ch);
    }

    unsigned ToLower(unsigned ch)
    {
        return (unsigned)tolower(ch);
    }

}