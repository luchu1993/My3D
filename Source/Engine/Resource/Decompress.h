//
// Created by luchu on 2022/2/18.
//

#pragma once

#include "Resource/Image.h"

namespace My3D
{
    /// Decompress a DXT compressed image to RGBA.
    MY3D_API void DecompressImageDXT(unsigned char* rgba, const void* blocks, int width, int height, int depth, CompressedFormat format);
    /// Decompress an ETC1/ETC2 compressed image to RGBA.
    MY3D_API void DecompressImageETC(unsigned char* dstImage, const void* blocks, int width, int height, bool hasAlpha);
    /// Decompress a PVRTC compressed image to RGBA.
    MY3D_API void DecompressImagePVRTC(unsigned char* rgba, const void* blocks, int width, int height, CompressedFormat format);
    /// Flip a compressed block vertically.
    MY3D_API void FlipBlockVertical(unsigned char* dest, const unsigned char* src, CompressedFormat format);
    /// Flip a compressed block horizontally.
    MY3D_API void FlipBlockHorizontal(unsigned char* dest, const unsigned char* src, CompressedFormat format);
}
