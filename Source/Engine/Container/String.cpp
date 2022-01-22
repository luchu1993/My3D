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

void String::Reserve(unsigned newCapacity)
{
    if (newCapacity < length_ + 1)
        newCapacity = length_ + 1;
    if (newCapacity == capacity_)
        return;

    auto* newBuffer = new char[newCapacity];
    // Move the existing data to the new buffer, then delete the old buffer
    CopyChars(newBuffer, buffer_, length_ + 1);
    if (capacity_)
        delete[] buffer_;

    capacity_ = newCapacity;
    buffer_ = newBuffer;
}

void String::Compact()
{
    if (capacity_)
        Reserve(length_ + 1);
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

String::String(const WString& str)
    : length_(0)
    , capacity_(0)
    , buffer_(&endZero)
{

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

String String::Replaced(char replaceThis, char replaceWith, bool caseSensitive) const
{
    String ret(*this);
    ret.Replace(replaceThis, replaceWith, caseSensitive);
    return ret;
}

String String::Replaced(const String& replaceThis, const String& replaceWith, bool caseSensitive) const
{
    String ret(*this);
    ret.Replace(replaceThis, replaceWith, caseSensitive);
    return ret;
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

unsigned String::Find(char c, unsigned startPos, bool caseSensitive) const
{
    if (caseSensitive)
    {
        for (unsigned i = startPos; i < length_; ++i)
        {
            if (buffer_[i] == c)
                return i;
        }
    }
    else
    {
        c = (char)tolower(c);
        for (unsigned i = startPos; i < length_; ++i)
        {
            if (tolower(buffer_[i]) == c)
                return i;
        }
    }

    return NPOS;
}

unsigned String::Find(const String& str, unsigned startPos, bool caseSensitive) const
{
    if (!str.length_ || str.length_ > length_)
        return NPOS;

    char first = str.buffer_[0];
    if (!caseSensitive)
        first = (char)tolower(first);

    for (unsigned i = startPos; i <= length_ - str.length_; ++i)
    {
        char c = buffer_[i];
        if (!caseSensitive)
            c = (char)tolower(c);

        if (c == first)
        {
            unsigned skip = NPOS;
            bool found = true;
            for (unsigned j = 1; j < str.length_; ++j)
            {
                c = buffer_[i + j];
                char d = str.buffer_[j];
                if (!caseSensitive)
                {
                    c = (char)tolower(c);
                    d = (char)tolower(d);
                }

                if (skip == NPOS && c == first)
                    skip = i + j - 1;

                if (c != d)
                {
                    found = false;
                    if (skip != NPOS)
                        i = skip;
                    break;
                }
            }
            if (found)
                return i;
        }
    }

    return NPOS;
}

unsigned String::FindLast(char c, unsigned startPos, bool caseSensitive) const
{
    if (startPos >= length_)
        startPos = length_ - 1;

    if (caseSensitive)
    {
        for (unsigned i = startPos; i < length_; --i)
        {
            if (buffer_[i] == c)
                return i;
        }
    }
    else
    {
        c = (char)tolower(c);
        for (unsigned i = startPos; i < length_; --i)
        {
            if (tolower(buffer_[i]) == c)
                return i;
        }
    }

    return NPOS;
}

unsigned String::FindLast(const String& str, unsigned startPos, bool caseSensitive) const
{
    if (!str.length_ || str.length_ > length_)
        return NPOS;
    if (startPos > length_ - str.length_)
        startPos = length_ - str.length_;

    char first = str.buffer_[0];
    if (!caseSensitive)
        first = (char)tolower(first);

    for (unsigned i = startPos; i < length_; --i)
    {
        char c = buffer_[i];
        if (!caseSensitive)
            c = (char)tolower(c);

        if (c == first)
        {
            bool found = true;
            for (unsigned j = 1; j < str.length_; ++j)
            {
                c = buffer_[i + j];
                char d = str.buffer_[j];
                if (!caseSensitive)
                {
                    c = (char)tolower(c);
                    d = (char)tolower(d);
                }

                if (c != d)
                {
                    found = false;
                    break;
                }
            }
            if (found)
                return i;
        }
    }

    return NPOS;
}

bool String::StartsWith(const String& str, bool caseSensitive) const
{
    return Find(str, 0, caseSensitive) == 0;
}

bool String::EndsWith(const String& str, bool caseSensitive) const
{
    unsigned pos = FindLast(str, Length() - 1, caseSensitive);
    return pos != NPOS && pos == Length() - str.Length();
}

int String::Compare(const String& str, bool caseSensitive) const
{
    return Compare(CString(), str.CString(), caseSensitive);
}

int String::Compare(const char* str, bool caseSensitive) const
{
    return Compare(CString(), str, caseSensitive);
}

String String::ToLower() const
{
    String ret(*this);
    for (unsigned i = 0; i < ret.length_; ++i)
        ret[i] = (char) tolower(buffer_[i]);

    return ret;
}

String String::ToUpper() const
{
    String ret(*this);
    for (unsigned i = 0; i < ret.length_; ++i)
        ret[i] = (char) toupper(buffer_[i]);

    return ret;
}

String String::Trimmed() const
{
    unsigned trimStart = 0;
    unsigned trimEnd = length_;

    while (trimStart < trimEnd)
    {
        char c = buffer_[trimStart];
        if (c != ' ' & c != 9)
            break;
        ++trimStart;
    }

    while (trimEnd > trimStart)
    {
        char c = buffer_[trimEnd - 1];
        if (c != ' ' && c != 9)
            break;
        --trimEnd;
    }

    return Substring(trimStart, trimEnd - trimStart);
}

String String::Substring(unsigned int pos) const
{
    if (pos < length_)
    {
        String ret;
        ret.Resize(length_ - pos);
        CopyChars(ret.buffer_, buffer_ + pos, ret.length_);

        return ret;
    }
    else
        return String();
}

String String::Substring(unsigned int pos, unsigned int length) const
{
    if (pos < length_)
    {
        String ret;
        if (pos + length > length)
            length = length_ - pos;
        ret.Resize(length);
        CopyChars(ret.buffer_, buffer_ + pos, ret.length_);

        return ret;
    }
    else
        return String();
}

void String::Replace(char replaceThis, char replaceWith, bool caseSensitive)
{
    if (caseSensitive)
    {
        for (int i = 0; i < length_; ++i)
        {
            if (buffer_[i] == replaceThis)
                buffer_[i] = replaceWith;
        }
    }
    else 
    {
        replaceThis = (char) tolower(replaceThis);
        for (unsigned i = 0; i < length_; ++i)
        {
            if (tolower(buffer_[i]) == replaceThis)
                buffer_[i] = replaceWith;
        }
    }
}

void String::Replace(const String& replaceThis, const String& replaceWith, bool caseSensitive)
{

}

void String::Replace(unsigned pos, unsigned length, const String& replaceWith)
{

}

void String::Replace(unsigned pos, unsigned length, const char* replaceWith)
{
    if (pos + length >= length_)
        return;
    
    Replace(pos, length, replaceWith, CStringLength(replaceWith));
}

String::Iterator String::Replace(const Iterator& start, const Iterator& end, const String& replaceWith)
{
    unsigned pos = (unsigned)(start - Begin());
    if (pos >= length_)
        return End();

    auto length = (unsigned)(end - start);
    Replace(pos, length, replaceWith);

    return Begin() + pos;
}

void String::Replace(unsigned pos, unsigned length, const char* srcStart, unsigned srcLength)
{
    unsigned delta = srcLength - length;
    if (pos + length < length_)
    {
        if (delta < 0)
        {
            MoveRange(pos + srcLength, pos + length, length_ - pos - length);
            Resize(length_ + delta);
        }
        if (delta > 0)
        {
            Resize(length_ + delta);
            MoveRange(pos + srcLength, pos + length, length_ - pos - length_ - delta);
        }
    }
    else 
        Resize(length + delta);

    CopyChars(buffer_ + pos, srcStart, srcLength);
}

void String::SetUTF8FromLatin1(const char* str)
{
    char temp[7];
    Clear();

    if (!str) return;
}

void String::SetUTF8FromWChar(const wchar_t* str)
{
    char temp[7];
    Clear();

    if (!str) return;
}

WString::WString()
    : length_(0)
    , buffer_(nullptr)
{
}

WString::WString(const String& str)
    : length_(0)
    , buffer_(nullptr)
{
#ifdef PLATFORM_MSVC
#else
    Resize(str.LengthUTF8());

    unsigned byteOffset = 0;
    wchar_t* dest = buffer_;
    while (byteOffset < str.Length())
        *dest++ = (wchar_t)str.NextUTF8Char(byteOffset);
#endif
}

WString::~WString() noexcept
{
    delete[] buffer_;
}
}

