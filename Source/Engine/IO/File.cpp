//
// Created by luchu on 2022/1/13.
//

#include "IO/File.h"
#include "IO/Log.h"
#include "IO/PackageFile.h"
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

File::File(Context *context, const String &fileName, FileMode mode)
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
    Open(fileName, mode);
}

File::File(Context *context, PackageFile *package, const String &fileName)
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
    Open(package, fileName);
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
    if (!package)
        return false;

    const PackageEntry* entry = package->GetEntry(fileName);
    if (!entry)
        return false;

    bool success = OpenInternal(package->GetName(), FILE_READ, true);
    if (!success)
    {
        MY3D_LOGERROR("Could not open package file " + fileName);
        return false;
    }

    name_ = fileName;
    offset_ = entry->offset_;
    checksum_ = entry->checksum_;
    size_ = entry->size_;
    compressed_ = package->IsCompressed();

    // Seek to beginning of package entry's file data
    SeekInternal(offset_);
    return true;
}

unsigned int File::Read(void *dest, unsigned int size)
{
    if (!IsOpen())
    {
        return 0;
    }

    if (mode_ == FILE_WRITE)
    {
        MY3D_LOGERROR("File not opened for reading");
        return 0;
    }

    if (size + position_ > size_)
        size = size_ - position_;

    if (!size)
        return 0;

    if (compressed_)
    {
        unsigned sizeLeft = size;
        auto* destPtr = (unsigned char*) dest;

        while (sizeLeft)
        {
            if (!readBuffer_ || readBufferOffset_ >= readBufferSize_)
            {
                unsigned char blockHeaderBytes[4];
                ReadInternal(blockHeaderBytes, sizeof blockHeaderBytes);

            }

            unsigned copySize = Min((readBufferSize_ - readBufferOffset_), sizeLeft);
            memcpy(destPtr, readBuffer_.Get() + readBufferOffset_, copySize);
            destPtr += copySize;
            sizeLeft -= copySize;
            readBufferOffset_ += copySize;
            position_ += copySize;
        }
    }

    writeSyncNeeded_ = true;
    position_ += size;
    return size;
}

unsigned int File::Seek(unsigned int position)
{
    if (!IsOpen())
    {
        // If file not open, do not log the error further here to prevent spamming the stderr stream
        return 0;
    }

    position_ = position;
    readSyncNeeded_ = false;
    writeSyncNeeded_ = false;
    return position_;
}

unsigned int File::Write(const void *data, unsigned int size)
{
    if (!IsOpen())
    {
        // If file not open, do not log the error further here to prevent spamming the stderr stream
        return 0;
    }

    if (mode_ == FILE_READ)
    {
        MY3D_LOGERROR("File not opened for writing");
        return 0;
    }

    if (!size)
        return 0;


    // Need to reassign the position due to internal buffering when transitioning from reading to writing
    if (writeSyncNeeded_)
    {
        fseek((FILE*)handle_, (long)position_ + offset_, SEEK_SET);
        writeSyncNeeded_ = false;
    }

    if (fwrite(data, size, 1, (FILE*)handle_) != 1)
    {
        // Return to the position where the write began
        fseek((FILE*)handle_, (long)position_ + offset_, SEEK_SET);
        MY3D_LOGERROR("Error while writing to file " + GetName());
        return 0;
    }

    readSyncNeeded_ = true;
    position_ += size;
    if (position_ > size_)
        size_ = position_;

    return size;
}

    unsigned int File::GetChecksum()
    {
        if (offset_ || checksum_)
            return checksum_;

        return checksum_;
    }

void File::Close()
{
    readBuffer_.Reset();
    inputBuffer_.Reset();

    if (handle_)
    {
        fclose((FILE*)handle_);
        handle_ = nullptr;
        position_ = 0;
        size_ = 0;
        offset_ = 0;
        checksum_ = 0;
    }
}

bool File::IsOpen() const
{
    return handle_ != nullptr;
}

void File::Flush()
{
    if (handle_)
        fflush((FILE*)handle_);
}

bool File::OpenInternal(const String &fileName, FileMode mode, bool fromPackage)
{
    Close();

    compressed_ = false;
    readSyncNeeded_ = false;
    writeSyncNeeded_ = false;

    auto* fileSystem = GetSubsystem<FileSystem>();
    if (fileSystem && !fileSystem->CheckAccess(GetPath(fileName)))
    {
        MY3D_LOGERRORF("Access denied to %s", fileName.CString());
        return false;
    }

    if (fileName.Empty())
    {
        MY3D_LOGERRORF("Could not open file with empty name");
        return false;
    }

#ifdef PLATFORM_MSVC
    handle_ = _wfopen(GetWideNativePath(fileName).CString(), openMode[mode]);
#else
    handle_ = fopen(GetNativePath(fileName).CString(), openMode[mode]);
#endif

    if (!handle_)
    {
        MY3D_LOGERRORF("Could not open file %s", fileName.CString());
        return false;
    }

    if (!fromPackage)
    {
        fseek((FILE*)handle_, 0, SEEK_END);
        long size = ftell((FILE*)handle_);
        fseek((FILE*)handle_, 0, SEEK_SET);
        if (size > M_MAX_UNSIGNED)
        {
            MY3D_LOGERRORF("Could not open file %s which is larger than 4GB", fileName.CString());
            Close();
            size_ = 0;
            return false;
        }
        size_ = (unsigned)size;
        offset_ = 0;
    }

    name_ = fileName;
    mode_ = mode;
    position_ = 0;
    checksum_ = 0;

    return true;
}

bool File::ReadInternal(void *dest, unsigned int size)
{
    return fread(dest, size, 1, (FILE*)handle_) == 1;
}

void File::SeekInternal(unsigned int newPosition)
{
    fseek((FILE*)handle_, newPosition, SEEK_SET);
}

}