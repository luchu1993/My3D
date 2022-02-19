//
// Created by luchu on 2022/2/18.
//

#include "Resource/Image.h"
#include "IO/Log.h"
#include "Core/Context.h"
#include "IO/FileSystem.h"
#include "IO/File.h"
#include "Resource/Decompress.h"

#include <SDL_surface.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((unsigned)(ch0) | ((unsigned)(ch1) << 8) | ((unsigned)(ch2) << 16) | ((unsigned)(ch3) << 24))
#endif

#define FOURCC_DXT1 (MAKEFOURCC('D','X','T','1'))
#define FOURCC_DXT2 (MAKEFOURCC('D','X','T','2'))
#define FOURCC_DXT3 (MAKEFOURCC('D','X','T','3'))
#define FOURCC_DXT4 (MAKEFOURCC('D','X','T','4'))
#define FOURCC_DXT5 (MAKEFOURCC('D','X','T','5'))
#define FOURCC_DX10 (MAKEFOURCC('D','X','1','0'))

#define FOURCC_ETC1 (MAKEFOURCC('E','T','C','1'))
#define FOURCC_ETC2 (MAKEFOURCC('E','T','C','2'))
#define FOURCC_ETC2A (MAKEFOURCC('E','T','2','A'))


namespace My3D
{
    bool CompressedLevel::Decompress(unsigned char *dest) const
    {
        if (!data_)
            return false;

        switch (format_)
        {
            case CF_DXT1:
            case CF_DXT3:
            case CF_DXT5:
                DecompressImageDXT(dest, data_, width_, height_, depth_, format_);
                return true;

            // ETC2 format is compatible with ETC1, so we just use the same function.
            case CF_ETC1:
            case CF_ETC2_RGB:
                DecompressImageETC(dest, data_, width_, height_, false);
                return true;
            case CF_ETC2_RGBA:
                DecompressImageETC(dest, data_, width_, height_, true);
                return true;

            case CF_PVRTC_RGB_2BPP:
            case CF_PVRTC_RGBA_2BPP:
            case CF_PVRTC_RGB_4BPP:
            case CF_PVRTC_RGBA_4BPP:
                DecompressImagePVRTC(dest, data_, width_, height_, format_);
                return true;

            default:
                // Unknown format
                return false;
        }
    }

    Image::Image(Context* context)
        : Resource(context)
    {
    }

    Image::~Image() = default;

    void Image::RegisterObject(Context* context)
    {
        context->RegisterFactory<Image>();
    }

    bool Image::BeginLoad(Deserializer &source)
    {
        // Check for DDS, KTX or PVR compressed format
        String fileID = source.ReadFileID();

        if (fileID == "DDS ")
        {

        }
        return true;
    }

    bool Image::Save(Serializer& dest) const
    {
        if (IsCompressed())
        {
            MY3D_LOGERROR("Can not save compressed image " + GetName());
            return false;
        }

        if (!data_)
        {
            MY3D_LOGERROR("Can not save zero-sized image " + GetName());
            return false;
        }

        int len;
        unsigned char* png = stbi_write_png_to_mem(data_.Get(), 0, width_, height_, components_, &len);
        bool success = dest.Write(png, (unsigned)len) == (unsigned)len;
        free(png);

        return success;
    }

    bool Image::SaveFile(const String& fileName) const
    {
        return false;
    }
}