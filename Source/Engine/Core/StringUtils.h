//
// Created by luchu on 2022/1/9.
//

#pragma once

#include "Core/Variant.h"


namespace My3D
{
/// Parse a bool from a string. Check for the first non-empty character (converted to lowercase) being either 't', 'y' or '1'.
MY3D_API bool ToBool(const String& source);
/// Parse a bool from a C string. Check for the first non-empty character (converted to lowercase) being either 't', 'y' or '1'.
MY3D_API bool ToBool(const char* source);
/// Parse a float from a string.
MY3D_API float ToFloat(const String& source);
/// Parse a float from a C string.
MY3D_API float ToFloat(const char* source);
/// Parse a double from a string.
MY3D_API double ToDouble(const String& source);
/// Parse a double from a C string.
MY3D_API double ToDouble(const char* source);
/// Parse an integer from a string. Assumed to be decimal by default (base 10). Use base 0 to autodetect from string.
MY3D_API int ToInt(const String& source, int base = 10);
/// Parse an integer from a C string. Assumed to be decimal by default (base 10). Use base 0 to autodetect from string.
MY3D_API int ToInt(const char* source, int base = 10);
/// Parse an unsigned integer from a string. Assumed to be decimal by default (base 10). Use base 0 to autodetect from string.
MY3D_API unsigned ToUInt(const String& source, int base = 10);
/// Parse an unsigned integer from a C string. Assumed to be decimal by default (base 10). Use base 0 to autodetect from string.
MY3D_API unsigned ToUInt(const char* source, int base = 10);
/// Parse an 64 bit integer from a string. Assumed to be decimal by default (base 10). Use base 0 to autodetect from string.
MY3D_API long long ToInt64(const String& source, int base = 10);
/// Parse an 64 bit integer from a C string. Assumed to be decimal by default (base 10). Use base 0 to autodetect from string.
MY3D_API long long ToInt64(const char* source, int base = 10);
/// Parse an unsigned 64 bit integer from a string. Assumed to be decimal by default (base 10). Use base 0 to autodetect from string.
MY3D_API unsigned long long ToUInt64(const String& source, int base = 10);
/// Parse an unsigned 64 bit integer from a C string. Assumed to be decimal by default (base 10). Use base 0 to autodetect from string.
MY3D_API unsigned long long ToUInt64(const char* source, int base = 10);
/// Parse a Color from a string.
MY3D_API Color ToColor(const String& source);
/// Parse a Color from a C string.
MY3D_API Color ToColor(const char* source);
/// Parse an IntVector2 from a string.
MY3D_API IntVector2 ToIntVector2(const String& source);
/// Parse an IntVector2 from a C string.
MY3D_API IntVector2 ToIntVector2(const char* source);
/// Parse an IntVector3 from a string.
MY3D_API IntVector3 ToIntVector3(const String& source);
/// Parse an IntVector3 from a C string.
MY3D_API IntVector3 ToIntVector3(const char* source);
/// Parse a Rect from a string.
MY3D_API Rect ToRect(const String& source);
/// Parse a Rect from a C string.
MY3D_API Rect ToRect(const char* source);
/// Parse a Vector2 from a string.
MY3D_API Vector2 ToVector2(const String& source);
/// Parse a Vector2 from a C string.
MY3D_API Vector2 ToVector2(const char* source);
/// Parse a Vector3 from a string.
MY3D_API Vector3 ToVector3(const String& source);
/// Parse a Vector3 from a C string.
MY3D_API Vector3 ToVector3(const char* source);
/// Parse a Vector4 from a string.
MY3D_API Vector4 ToVector4(const String& source, bool allowMissingCoords = false);
/// Parse a Vector4 from a C string.
MY3D_API Vector4 ToVector4(const char* source, bool allowMissingCoords = false);
/// Parse a float, Vector or Matrix variant from a string. Return empty variant on illegal input.
MY3D_API Variant ToVectorVariant(const String& source);
/// Parse a float, Vector or Matrix variant from a C string. Return empty variant on illegal input.
MY3D_API Variant ToVectorVariant(const char* source);
/// Parse a Matrix3 from a string.
MY3D_API Matrix3 ToMatrix3(const String& source);
/// Parse a Matrix3 from a C string.
MY3D_API Matrix3 ToMatrix3(const char* source);
/// Parse a Matrix3x4 from a string.
MY3D_API Matrix3x4 ToMatrix3x4(const String& source);
/// Parse a Matrix3x4 from a C string.
MY3D_API Matrix3x4 ToMatrix3x4(const char* source);
/// Parse a Matrix4 from a string.
MY3D_API Matrix4 ToMatrix4(const String& source);
/// Parse a Matrix4 from a C string.
MY3D_API Matrix4 ToMatrix4(const char* source);
/// Convert a string to a byte buffer.
MY3D_API void StringToBuffer(PODVector<unsigned char>& dest, const String& source);
/// Convert a C string to a byte buffer.
MY3D_API void StringToBuffer(PODVector<unsigned char>& dest, const char* source);
/// Convert a byte buffer to a string.
MY3D_API void BufferToString(String& dest, const void* data, unsigned size);
/// Convert a pointer to string (returns hexadecimal).
MY3D_API String ToString(void* value);
/// Convert an unsigned integer to string as hexadecimal.
MY3D_API String ToStringHex(unsigned value);

/// Return an index to a string list corresponding to the given string, or a default value if not found. The string list must be empty-terminated.
MY3D_API unsigned GetStringListIndex(const String& value, const String* strings, unsigned defaultIndex, bool caseSensitive = false);
/// Return an index to a string list corresponding to the given C string, or a default value if not found. The string list must be empty-terminated.
MY3D_API unsigned GetStringListIndex(const char* value, const String* strings, unsigned defaultIndex, bool caseSensitive = false);
/// Return an index to a C string list corresponding to the given C string, or a default value if not found. The string list must be empty-terminated.
MY3D_API unsigned GetStringListIndex(const char* value, const char** strings, unsigned defaultIndex, bool caseSensitive = false);

}
