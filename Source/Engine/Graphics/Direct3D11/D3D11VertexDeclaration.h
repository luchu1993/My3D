//
// Created by luchu on 2022/2/20.
//

#pragma once

#include "Container/RefCounted.h"


namespace My3D
{
    class Graphics;
    class ShaderVariation;
    class VertexBuffer;

    /// Vertex declaration.
    class MY3D_API VertexDeclaration : public RefCounted
    {
    public:
        /// Construct with vertex buffers and element masks to base declaration on.
        VertexDeclaration(Graphics* graphics, ShaderVariation* vertexShader, VertexBuffer** buffers);
        /// Destruct.
        ~VertexDeclaration() override;

        /// Return input layout object corresponding to the declaration.
        void* GetInputLayout() const { return inputLayout_; }

    private:
        /// Input layout object.
        void* inputLayout_;
    };
}
