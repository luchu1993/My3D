//
// Created by luchu on 2022/1/1.
//

#include "Core/StringHash.h"
#include "Math/MathDefs.h"
#include <cstdio>


namespace My3D
{

const StringHash StringHash::ZERO;


StringHash::StringHash(const char* str) noexcept : value_(Calculate(str)) { }

StringHash::StringHash(const String& str) noexcept : value_(Calculate(str.CString())) { }

unsigned StringHash::Calculate(const char *str, unsigned int hash)
{
    if (!str) return hash;

    while (*str)
    {
        hash = SDBMHash(hash, (unsigned char) *str++);
    }

    return hash;
}

String StringHash::ToString() const
{
    char tempBuffer[CONVERSION_BUFFER_LENGTH];
    sprintf(tempBuffer, "%08X", value_);
    return String(tempBuffer);
}

}
