//
// Created by luchu on 2022/1/13.
//

#include "IO/File.h"
#include "IO/Log.h"
#include "IO/FileSystem.h"


namespace My3D
{
#ifdef PLATFORM_MSVC
static const wchar_t* openMode[] =
{
    L"rb",
    L"wb",
    L"r+b",
    L"w+b"
};
#else
static const char* openMode[] =
{
    "rb",
    "wb",
    "r+b",
    "w+b"
};
#endif

File::File(Context *context)
    : Object(context)
    , mode_(FILE_READ)
    , handle_(nullptr)
    , readBufferOffset_(0)
    , readBufferSize_(0)
    , offset_(0)
    , checksum_(0)
    , compressed_(false)
    , readSyncNeeded_(false)
    , writeSyncNeeded_(false)
{
}

File::~File()
{
    Close();
}

bool File::Open(const String& fileName, FileMode mode)
{
    return OpenInternal(fileName, mode);
}

bool File::Open(PackageFile* package, const String& fileName)
{

}

bool File::OpenInternal(const String &fileName, FileMode mode, bool fromPackage)
{
    Close();

    compressed_ = false;
    readSyncNeeded_ = false;
    writeSyncNeeded_ = false;

    auto* fileSystem = GetSubSystem<FileSystem>();
}

}