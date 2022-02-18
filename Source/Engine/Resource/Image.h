//
// Created by luchu on 2022/2/18.
//

#pragma once

#include "Container/ArrayPtr.h"
#include "Resource/Resource.h"

struct SDL_Surface;

namespace My3D
{
    /// Supported compressed image formats.
    enum CompressedFormat
    {
        CF_NONE = 0,
        CF_RGBA,
        CF_DXT1,
        CF_DXT3,
        CF_DXT5,
        CF_ETC1,
        CF_ETC2_RGB,
        CF_ETC2_RGBA,
        CF_PVRTC_RGB_2BPP,
        CF_PVRTC_RGBA_2BPP,
        CF_PVRTC_RGB_4BPP,
        CF_PVRTC_RGBA_4BPP,
    };

    /// Compressed image mip level.
    struct CompressedLevel
    {
        /// Decompress to RGBA. The destination buffer required is width * height * 4 bytes. Return true if successful.
        bool Decompress(unsigned char* dest) const;

        /// Compressed image data.
        unsigned char* data_{};
        /// Compression format.
        CompressedFormat format_{CF_NONE};
        /// Width.
        int width_{};
        /// Height.
        int height_{};
        /// Depth.
        int depth_{};
        /// Block size in bytes.
        unsigned blockSize_{};
        /// Total data size in bytes.
        unsigned dataSize_{};
        /// Row size in bytes.
        unsigned rowSize_{};
        /// Number of rows.
        unsigned rows_{};
    };

    /// Image resource
    class MY3D_API Image : public Resource
    {
        MY3D_OBJECT(Image, Resource)

    public:
        /// Construct empty
        explicit Image(Context* context);
        /// Destruct.
        ~Image() override;
        /// Register object factory.
        static void RegisterObject(Context* context);

        /// Load resource from stream. May be called from a worker thread. Return true if successful.
        bool BeginLoad(Deserializer& source) override;
        /// Save the image to a stream. Regardless of original format, the image is saved as png. Compressed image data is not supported. Return true if successful.
        bool Save(Serializer& dest) const override;
        /// Save the image to a file. Format of the image is determined by file extension. JPG is saved with maximum quality.
        bool SaveFile(const String& fileName) const override;

        /// Return width.
        int GetWidth() const { return width_; }
        /// Return height.
        int GetHeight() const { return height_; }
        /// Return depth.
        int GetDepth() const { return depth_; }
        /// Return number of color components.
        unsigned GetComponents() const { return components_; }
        /// Return pixel data.
        unsigned char* GetData() const { return data_; }
        /// Return whether is compressed.
        bool IsCompressed() const { return compressedFormat_ != CF_NONE; }
        /// Return compressed format.
        CompressedFormat GetCompressedFormat() const { return compressedFormat_; }
        /// Return number of compressed mip levels. Returns 0 if the image is has not been loaded from a source file containing multiple mip levels.
        unsigned GetNumCompressedLevels() const { return numCompressedLevels_; }

    private:
        /// Decode an image using stb_image.
        static unsigned char* GetImageData(Deserializer& source, int& width, int& height, unsigned& components);
        /// Free an image file's pixel data.
        static void FreeImageData(unsigned char* pixelData);

        /// Width.
        int width_{};
        /// Height.
        int height_{};
        /// Depth.
        int depth_{};
        /// Number of color components.
        unsigned components_{};
        /// Number of compressed mip levels.
        unsigned numCompressedLevels_{};
        /// Cubemap status if DDS.
        bool cubemap_{};
        /// Texture array status if DDS.
        bool array_{};
        /// Data is sRGB.
        bool sRGB_{};
        /// Compressed format.
        CompressedFormat compressedFormat_{CF_NONE};
        /// Pixel data.
        SharedArrayPtr<unsigned char> data_;
        /// Precalculated mip level image.
        SharedPtr<Image> nextLevel_;
        /// Next texture array or cube map image.
        SharedPtr<Image> nextSibling_;
    };
}
