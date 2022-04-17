//
// Created by luchu on 2022/3/15.
//

#include "Core/WorkQueue.h"
#include "Graphics/Camera.h"
#include "Graphics/OcclusionBuffer.h"
#include "IO/Log.h"


namespace My3D
{
    enum ClipMask : unsigned
    {
        CLIPMASK_X_POS = 0x1,
        CLIPMASK_X_NEG = 0x2,
        CLIPMASK_Y_POS = 0x4,
        CLIPMASK_Y_NEG = 0x8,
        CLIPMASK_Z_POS = 0x10,
        CLIPMASK_Z_NEG = 0x20,
    };
    MY3D_FLAGSET(ClipMask, ClipMaskFlags);

    OcclusionBuffer::OcclusionBuffer(Context* context)
        : Object(context)
    {
    }

    OcclusionBuffer::~OcclusionBuffer() = default;

    unsigned OcclusionBuffer::GetUseTimer()
    {
        return useTimer_.GetMSec(false);
    }
}