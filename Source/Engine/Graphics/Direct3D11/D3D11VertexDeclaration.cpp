//
// Created by luchu on 2022/2/20.
//

#include "Graphics/Graphics.h"
#include "Graphics/GraphicsImpl.h"
#include "Graphics/ShaderVariation.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/VertexDeclaration.h"
#include "IO/Log.h"


namespace My3D
{
    static const DXGI_FORMAT d3dElementFormats[] =
    {
        DXGI_FORMAT_R32_SINT,
        DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_R32G32_FLOAT,
        DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        DXGI_FORMAT_R8G8B8A8_UINT,
        DXGI_FORMAT_R8G8B8A8_UNORM
    };

    VertexDeclaration::VertexDeclaration(Graphics* graphics, ShaderVariation* vertexShader, VertexBuffer** vertexBuffers)
        : inputLayout_(nullptr)
    {
        PODVector<D3D11_INPUT_ELEMENT_DESC> elementDescs;
        unsigned prevBufferDescs = 0;


        for (unsigned i = 0; i < MAX_VERTEX_STREAMS; ++i)
        {
            if (!vertexBuffers[i])
                continue;

            const PODVector<VertexElement>& srcElements = vertexBuffers[i]->GetElements();
            bool isExisting = false;

            for (unsigned j = 0; j < srcElements.Size(); ++j)
            {
                const VertexElement& srcElement = srcElements[j];
                const char* semanticName = ShaderVariation::elementSemanticNames[srcElement.semantic_];

                // Override existing element if necessary
                for (unsigned k = 0; k < prevBufferDescs; ++k)
                {
                    if (elementDescs[k].SemanticName == semanticName && elementDescs[k].SemanticIndex == srcElement.index_)
                    {
                        isExisting = true;
                        elementDescs[k].InputSlot = i;
                        elementDescs[k].AlignedByteOffset = srcElement.offset_;
                        elementDescs[k].InputSlotClass = srcElement.perInstance_ ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
                        elementDescs[k].InstanceDataStepRate = srcElement.perInstance_ ? 1 : 0;
                        break;
                    }
                }

                if (isExisting)
                    continue;

                D3D11_INPUT_ELEMENT_DESC newDesc;
                newDesc.SemanticName = semanticName;
                newDesc.SemanticIndex = srcElement.index_;
                newDesc.Format = d3dElementFormats[srcElement.type_];
                newDesc.InputSlot = (UINT)i;
                newDesc.AlignedByteOffset = srcElement.offset_;
                newDesc.InputSlotClass = srcElement.perInstance_ ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
                newDesc.InstanceDataStepRate = srcElement.perInstance_ ? 1 : 0;
                elementDescs.Push(newDesc);
            }

            prevBufferDescs = elementDescs.Size();
        }

        if (elementDescs.Empty())
            return;

        const PODVector<unsigned char>& byteCode = vertexShader->GetByteCode();

        HRESULT hr = graphics->GetImpl()->GetDevice()->CreateInputLayout(&elementDescs[0], (UINT)elementDescs.Size(), &byteCode[0],
                                                                         byteCode.Size(), (ID3D11InputLayout**)&inputLayout_);
        if (FAILED(hr))
        {
            MY3D_SAFE_RELEASE(inputLayout_);
            MY3D_LOGERRORF("Failed to create input layout for shader %s due to missing vertex element(s) (HRESULT %x)",
                             vertexShader->GetFullName().CString(), (unsigned)hr);
        }
    }

    VertexDeclaration::~VertexDeclaration()
    {
        MY3D_SAFE_RELEASE(inputLayout_);
    }
}
