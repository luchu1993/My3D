//
// Created by luchu on 2022/1/23.
//

#pragma once

#include "Core/Object.h"

namespace My3D
{
    /// File entry within the package file
    struct PackageEntry
    {
        /// Offset from the beginning
        unsigned offset_;
        /// File size
        unsigned size_;
        /// File checksum
        unsigned checksum_;
    };

    /// Stores files of a directory tree sequentially for convenient access.
    class MY3D_API PackageFile : public Object
    {
        MY3D_OBJECT(PackageFile, Object)

    public:
        /// Construct
        explicit PackageFile(Context* context);
        /// Construct and open
        PackageFile(Context* context, const String& fileName, unsigned startOffset = 0);
        /// Destruct
        ~PackageFile() override;

        /// Open the package file. Return true if successful
        bool Open(const String& fileName, unsigned startOffset = 0);
        /// Check if a file exists within the package file. This will be case-insensitive on Windows and case-sensitive on other platforms.
        bool Exists(const String& fileName) const;
        /// Return the file entry corresponding to the name, or null if not found. This will be case-insensitive on Windows and case-sensitive on other platforms.
        const PackageEntry* GetEntry(const String& fileName) const;
        /// return all file entries
        const HashMap<String, PackageEntry>& GetEntries() const { return entries_; }
        /// Return the package file name
        const String& GetName() const { return fileName_; }
        /// Return number of files
        unsigned GetNumFiles() const { return entries_.Size(); }
        /// Return total size of the package file
        unsigned GetTotalSize() const { return totalSize_; }
        /// Return total data size from all the file entries in the package file
        unsigned GetTotalDataSize() const { return totalDataSize_; }
        /// Return checksum of the package file contents
        unsigned GetCheckSum() const { return checksum_; }
        /// Return whether the files are compressed
        bool IsCompressed() const { return compressed_; }
        /// Return list of file names in the package
        Vector<String> GetEntryNames() const { return entries_.Keys(); }

    private:
        // File entries
        HashMap<String, PackageEntry> entries_;
        /// File name
        String fileName_;
        /// Package file name hash
        StringHash nameHash_;
        /// Package file total size
        unsigned totalSize_;
        /// Total data size in the package using each entry's actual size if it is a compressed package file.
        unsigned totalDataSize_;
        /// Package file checksum
        unsigned checksum_;
        /// Compressed flag
        bool compressed_;
    };
}