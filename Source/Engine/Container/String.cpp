//
// Created by luchu on 2022/1/1.
//

#include "Container/String.h"
#include "Container/Swap.h"
#include "IO/Log.h"

#include <cstdio>


namespace My3D
{

char String::endZero = '\0';
const String String::EMPTY;
const unsigned String::NPOS;
const unsigned String::MIN_CAPACITY;


int String::Compare(const char *lhs, const char *rhs, bool casSensitive)
{
    if (!lhs || !rhs)
        return lhs ? 1 : (rhs? -1 : 0);

    if (casSensitive)
        return strcmp(lhs, rhs);

    for (;;)
    {
        char l = (char) tolower(*lhs);
        char r = (char) tolower(*rhs);
        if (!l || !r)
            return l ? 1 : (r ? -1 : 0);
        if (l < r)
            return -1;
        if (l > r)
            return 1;

        ++lhs;
        ++rhs;
    }
}

void String::Swap(String &str)
{
    My3D::Swap(length_, str.length_);
    My3D::Swap(capacity_, str.capacity_);
    My3D::Swap(buffer_, str.buffer_);
}

void String::Clear()
{
    Resize(0);
}

void String::Compact()
{

}

void String::Resize(unsigned int newLength)
{
    if (!capacity_)
    {
        if (!newLength)
            return;

        capacity_ = newLength + 1;
        if (capacity_ < MIN_CAPACITY)
            capacity_ = MIN_CAPACITY;
        buffer_ = new char[capacity_];
    }
    else
    {
        if (newLength && capacity_ < newLength + 1)
        {
            while (capacity_ < newLength + 1)
                capacity_ += (capacity_ + 1) >> 1u;

            auto* newBuffer = new char[capacity_];
            if (length_)
                CopyChars(newBuffer, buffer_, length_);
            delete[] buffer_;
            buffer_ = newBuffer;
        }
    }

    buffer_[newLength] = 0;
    length_ = newLength;
}

String::String(int value)
    : length_(0)
    , capacity_(0)
    , buffer_(&endZero)
{
    char tempBuffer[CONVERSION_BUFFER_LENGTH];
    sprintf(tempBuffer, "%d", value);
    *this = tempBuffer;
}

String::String(short value) :
        length_(0),
        capacity_(0),
        buffer_(&endZero)
{
    char tempBuffer[CONVERSION_BUFFER_LENGTH];
    sprintf(tempBuffer, "%d", value);
    *this = tempBuffer;
}

String::String(long value)
    : length_(0)
    , capacity_(0)
    , buffer_(&endZero)
{
    char tempBuffer[CONVERSION_BUFFER_LENGTH];
    sprintf(tempBuffer, "%ld", value);
    *this = tempBuffer;
}

String::String(long long value)
    : length_(0)
    , capacity_(0)
    , buffer_(&endZero)
{
    char tempBuffer[CONVERSION_BUFFER_LENGTH];
    sprintf(tempBuffer, "%lld", value);
    *this = tempBuffer;
}

String::String(unsigned value)
    : length_(0)
    , capacity_(0)
    , buffer_(&endZero)
{
    char tempBuffer[CONVERSION_BUFFER_LENGTH];
    sprintf(tempBuffer, "%u", value);
    *this = tempBuffer;
}

String::String(unsigned short value)
    : length_(0)
    , capacity_(0)
    , buffer_(&endZero)
{
    char tempBuffer[CONVERSION_BUFFER_LENGTH];
    sprintf(tempBuffer, "%u", value);
    *this = tempBuffer;
}

String::String(unsigned long value)
    : length_(0)
    , capacity_(0)
    , buffer_(&endZero)
{
    char tempBuffer[CONVERSION_BUFFER_LENGTH];
    sprintf(tempBuffer, "%lu", value);
    *this = tempBuffer;
}

String::String(unsigned long long value)
    : length_(0)
    , capacity_(0)
    , buffer_(&endZero)
{
    char tempBuffer[CONVERSION_BUFFER_LENGTH];
    sprintf(tempBuffer, "%llu", value);
    *this = tempBuffer;
}

String::String(float value)
    : length_(0)
    , capacity_(0)
    , buffer_(&endZero)
{
    char tempBuffer[CONVERSION_BUFFER_LENGTH];
    sprintf(tempBuffer, "%g", value);
    *this = tempBuffer;
}

String::String(double value)
    : length_(0)
    , capacity_(0)
    , buffer_(&endZero)
{
    char tempBuffer[CONVERSION_BUFFER_LENGTH];
    sprintf(tempBuffer, "%.15g", value);
    *this = tempBuffer;
}

String::String(bool value)
    : length_(0)
    , capacity_(0)
    , buffer_(&endZero)
{
    if (value)
        *this = "true";
    else
        *this = "false";
}


String::String(char value)
    : length_(0)
    , capacity_(0)
    , buffer_(&endZero)
{
    Resize(1);
    buffer_[0] = value;
}

String::String(char value, unsigned length)
    : length_(0)
    , capacity_(0)
    , buffer_(&endZero)
{
    Resize(length);
    for (unsigned i = 0; i < length; ++i)
        buffer_[i] = value;
}

String& String::AppendWithFormat(const char *formatString, ...)
{
    va_list args;
    va_start(args, formatString);
    AppendWithFormatArgs(formatString, args);
    va_end(args);

    return *this;
}

String& String::AppendWithFormatArgs(const char *formatString, va_list args)
{
    int pos = 0, lastPos = 0;
    auto length = (int)strlen(formatString);

    while (true)
    {
        // Scan the format string and find %a argument where a is one of d, f, s ...
        while (pos < length && formatString[pos] != '%') pos++;
        Append(formatString + lastPos, (unsigned)(pos - lastPos));
        if (pos >= length)
            return *this;

        char format = formatString[pos + 1];
        pos += 2;
        lastPos = pos;

        switch (format)
        {
            case 'd':
            case 'i':
            {
                int arg = va_arg(args, int);
                Append(String(arg));
                break;
            }
            case 'u':
            {
                unsigned arg = va_arg(args, unsigned);
                Append(String(arg));
                break;
            }
            case 'l':
            {
                unsigned long arg = va_arg(args, unsigned long);
                Append(String(arg));
                break;
            }
            case 'f':
            {
                double arg = va_arg(args, double);
                Append(String(arg));
                break;
            }
            case 'c':
            {
                int arg = va_arg(args, int);
                Append((char)arg);
                break;
            }
            case 's':
            {
                char* arg = va_arg(args, char*);
                Append(arg);
                break;
            }
            case 'x':
            {
                char buf[CONVERSION_BUFFER_LENGTH];
                int arg = va_arg(args, int);
                int arglen = ::sprintf(buf, "%x", arg);
                Append(buf,(unsigned) arglen);
                break;
            }
            case 'p':
            {
                char buf[CONVERSION_BUFFER_LENGTH];
                void* arg = va_arg(args, void*);
                int arglen = ::sprintf(buf, "%p", arg);
                Append(buf, (unsigned)arglen);
                break;
            }
            case '%':
            {
                Append("%", 1);
                break;
            }

            default:
                MY3D_LOGWARNINGF("Unsupported format specifier: '%c'", format);
                break;
        }
    }
}

String& String::Append(const String &str)
{
    return *this += str;
}

String& String::Append(const char *str, unsigned int length)
{
    if (str)
    {
        unsigned oldLength = length_;
        Resize(oldLength + length);
        CopyChars(&buffer_[oldLength], str, length);
    }

    return *this;
}

String& String::Append(char c)
{
    return *this += c;
}

String& String::Append(const char *str)
{
    return *this += str;
}

}

