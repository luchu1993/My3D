//
// Created by luchu on 2022/2/21.
//

#pragma once

#include "Graphics/Geometry.h"
#include "Graphics/Graphics.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/VertexBuffer.h"
#include "IO/Log.h"
#include "Math/Ray.h"


namespace My3D
{
    Geometry::Geometry(Context* context)
        : Object(context)
        , primitiveType_(TRIANGLE_LIST)
        , indexStart_(0)
        , indexCount_(0)
        , vertexStart_(0)
        , vertexCount_(0)
        , rawVertexSize_(0)
        , rawIndexSize_(0)
        , lodDistance_(0.0f)
    {
        SetNumVertexBuffers(1);
    }

    Geometry::~Geometry() = default;

    bool Geometry::SetNumVertexBuffers(unsigned num)
    {
        if (num >= MAX_VERTEX_STREAMS)
        {
            MY3D_LOGERROR("Too many vertex streams");
            return false;
        }

        vertexBuffers_.Resize(num);

        return true;
    }
}