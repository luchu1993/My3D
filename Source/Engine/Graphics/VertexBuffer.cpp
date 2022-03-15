//
// Created by luchu on 2022/1/21.
//

#include "Graphics/VertexBuffer.h"
#include "Graphics/Graphics.h"
#include "Graphics/GraphicsImpl.h"
#include "IO/Log.h"


namespace My3D
{
    VertexBuffer::VertexBuffer(Context *context, bool forceHeadless)
        : Object(context)
        , GPUObject(forceHeadless ? nullptr : GetSubsystem<Graphics>())
    {
        UpdateOffsets();
        if (!graphics_)
            shadowed_ = true;
    }

    VertexBuffer::~VertexBuffer()
    {
        Release();
    }

    const VertexElement* VertexBuffer::GetElement(VertexElementSemantic semantic, unsigned char index) const
    {
        for (const auto& element : elements_)
        {
            if (element.semantic_ == semantic && element.index_ == index)
                return &element;
        }

        return nullptr;
    }

    const VertexElement* VertexBuffer::GetElement(VertexElementType type, VertexElementSemantic semantic, unsigned char index) const
    {
        for (const auto& element : elements_)
        {
            if (element.type_ == type && element.semantic_ == semantic && element.index_ == index)
                return &element;
        }
        return nullptr;
    }

    const VertexElement* VertexBuffer::GetElement(const PODVector<VertexElement> &elements, VertexElementType type, VertexElementSemantic semantic, unsigned char index)
    {
        for (auto const& element : elements)
        {
            if (element.type_ == type && element.semantic_ == semantic && element.index_ == index)
                return &element;
        }

        return nullptr;
    }

    PODVector<VertexElement> VertexBuffer::GetElements(unsigned elementMask)
    {
        PODVector<VertexElement> ret;

        for (unsigned i = 0; i < MAX_LEGACY_VERTEX_ELEMENTS; ++i)
        {
            if (elementMask & (1u << i))
                ret.Push(LEGACY_VERTEXELEMENTS[i]);
        }

        return ret;
    }

    bool VertexBuffer::HasElement(const PODVector<VertexElement> &elements, VertexElementType type, VertexElementSemantic semantic, unsigned char index)
    {
        return GetElement(elements, type, semantic, index) != nullptr;
    }

    unsigned VertexBuffer::GetElementOffset(VertexElementSemantic semantic, unsigned char index) const
    {
        const VertexElement* element = GetElement(semantic, index);
        return element ? element->offset_ : M_MAX_UNSIGNED;
    }

    unsigned VertexBuffer::GetElementOffset(VertexElementType type, VertexElementSemantic semantic, unsigned char index)
    {
        const VertexElement* element = GetElement(type, semantic, index);
        return element ? element->offset_ : M_MAX_UNSIGNED;
    }

    unsigned VertexBuffer::GetElementOffset(const PODVector<VertexElement> &elements, VertexElementType type, VertexElementSemantic semantic, unsigned char index)
    {
        const VertexElement* element = GetElement(elements, type, semantic, index);
        return element ? element->offset_ : M_MAX_UNSIGNED;
    }

    unsigned int VertexBuffer::GetVertexSize(const PODVector<VertexElement> &elements)
    {
        unsigned size = 0;
        for (unsigned i = 0; i < elements.Size(); ++i)
            size += ELEMENT_TYPESIZES[elements[i].type_];

        return size;
    }

    void VertexBuffer::UpdateOffsets()
    {
        unsigned elementOffset = 0;
        elementHash_ = 0;

        for (auto& element : elements_)
        {
            element.offset_ = elementOffset;
            elementOffset += ELEMENT_TYPESIZES[element.type_];
            elementHash_ <<= 6;
            elementHash_ += (((int)element.type_ + 1) * ((int)element.semantic_ + 1) + element.index_);
        }

        vertexSize_ = elementOffset;
    }

    void VertexBuffer::SetShadowed(bool enable)
    {
        if (!graphics_)
            enable = true;

        if (shadowed_ != enable)
        {
            if (enable && vertexSize_ && vertexCount_)
                shadowData_ = new unsigned char [vertexCount_ * vertexSize_];
            else
                shadowData_.Reset();

            shadowed_ = true;
        }
    }

    bool VertexBuffer::SetSize(unsigned int vertexCount, unsigned int elementMask, bool dynamic)
    {
        return SetSize(vertexCount, GetElements(elementMask), dynamic);
    }

    bool VertexBuffer::SetSize(unsigned int vertexCount, const PODVector<VertexElement> &elements, bool dynamic)
    {
        Unlock();
        vertexCount_ = vertexCount;
        elements_ = elements;
        dynamic_ = dynamic;

        UpdateOffsets();

        if (shadowed_ && vertexCount_ && vertexSize_)
            shadowData_ = new unsigned char[vertexCount_ * vertexSize_];
        else
            shadowData_.Reset();

        return Create();
    }

    void VertexBuffer::UpdateOffsets(PODVector<VertexElement> &elements)
    {
        unsigned elementOffset = 0;
        for (auto& element : elements)
        {
            element.offset_ = elementOffset;
            elementOffset += ELEMENT_TYPESIZES[element.type_];
        }
    }
}