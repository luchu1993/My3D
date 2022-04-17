//
// Created by luchu on 2022/4/15.
//

#include "Container/Vector.h"
#include "Core/Context.h"
#include "Core/CoreEvents.h"
#include "Graphics/Graphics.h"
#include "Graphics/DebugRenderer.h"
#include "Graphics/Camera.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/ShaderVariation.h"


namespace My3D
{
    extern const char* SUBSYSTEM_CATEGORY;

    // Cap the amount of lines to prevent crash when eg. debug rendering large heightfields
    static const unsigned MAX_LINES = 1000000;
    // Cap the amount of triangles to prevent crash.
    static const unsigned MAX_TRIANGLES = 100000;

    DebugRenderer::DebugRenderer(Context *context)
        : Component(context)
        , lineAntiAlias_(false)
    {
        vertexBuffer_ = new VertexBuffer(context_);
        SubscribeToEvent(E_ENDFRAME, MY3D_HANDLER(DebugRenderer, HandleEndFrame));
    }

    DebugRenderer::~DebugRenderer() = default;

    void DebugRenderer::RegisterObject(Context* context)
    {
        context->RegisterFactory<DebugRenderer>(SUBSYSTEM_CATEGORY);
        MY3D_ACCESSOR_ATTRIBUTE("Line Antialias", GetLineAntiAlias, SetLineAntiAlias, bool, false, AM_DEFAULT);
    }

    void DebugRenderer::SetLineAntiAlias(bool enable)
    {
        if (enable != lineAntiAlias_)
        {
            lineAntiAlias_ = enable;
            MarkNetworkUpdate();
        }
    }

    void DebugRenderer::AddLine(const Vector3& start, const Vector3& end, const Color& color, bool depthTest)
    {
        AddLine(start, end, color.ToUInt(), depthTest);
    }

    void DebugRenderer::AddLine(const Vector3& start, const Vector3& end, unsigned color, bool depthTest)
    {
        if (lines_.Size() + noDepthLines_.Size() >= MAX_LINES)
            return;

        if (depthTest)
            lines_.Push(DebugLine(start, end, color));
        else
            noDepthLines_.Push(DebugLine(start, end, color));
    }

    void DebugRenderer::AddTriangle(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Color& color, bool depthTest)
    {
        AddTriangle(v1, v2, v3, color.ToUInt(), depthTest);
    }

    void DebugRenderer::AddTriangle(const Vector3& v1, const Vector3& v2, const Vector3& v3, unsigned color, bool depthTest)
    {
        if (triangles_.Size() + noDepthTriangles_.Size() >= MAX_TRIANGLES)
            return;

        if (depthTest)
            triangles_.Push(DebugTriangle(v1, v2, v3, color));
        else
            noDepthTriangles_.Push(DebugTriangle(v1, v2, v3, color));
    }

    void DebugRenderer::AddTriangleMesh(const void* vertexData, unsigned vertexSize, const void* indexData,
                                        unsigned indexSize, unsigned indexStart, unsigned indexCount, const Matrix3x4& transform, const Color& color, bool depthTest)
    {
        AddTriangleMesh(vertexData, vertexSize, 0, indexData, indexSize, indexStart, indexCount, transform, color, depthTest);
    }

    void DebugRenderer::AddTriangleMesh(const void* vertexData, unsigned vertexSize, unsigned vertexStart, const void* indexData,
                                        unsigned indexSize, unsigned indexStart, unsigned indexCount, const Matrix3x4& transform, const Color& color, bool depthTest)
    {
        unsigned uintColor = color.ToUInt();
        const auto* srcData = ((const unsigned char*)vertexData) + vertexStart;

        // 16-bit indices
        if (indexSize == sizeof(unsigned short))
        {
            const unsigned short* indices = ((const unsigned short*)indexData) + indexStart;
            const unsigned short* indicesEnd = indices + indexCount;

            while (indices < indicesEnd)
            {
                Vector3 v0 = transform * *((const Vector3*)(&srcData[indices[0] * vertexSize]));
                Vector3 v1 = transform * *((const Vector3*)(&srcData[indices[1] * vertexSize]));
                Vector3 v2 = transform * *((const Vector3*)(&srcData[indices[2] * vertexSize]));

                AddLine(v0, v1, uintColor, depthTest);
                AddLine(v1, v2, uintColor, depthTest);
                AddLine(v2, v0, uintColor, depthTest);

                indices += 3;
            }
        }
        else
        {
            const unsigned* indices = ((const unsigned*)indexData) + indexStart;
            const unsigned* indicesEnd = indices + indexCount;

            while (indices < indicesEnd)
            {
                Vector3 v0 = transform * *((const Vector3*)(&srcData[indices[0] * vertexSize]));
                Vector3 v1 = transform * *((const Vector3*)(&srcData[indices[1] * vertexSize]));
                Vector3 v2 = transform * *((const Vector3*)(&srcData[indices[2] * vertexSize]));

                AddLine(v0, v1, uintColor, depthTest);
                AddLine(v1, v2, uintColor, depthTest);
                AddLine(v2, v0, uintColor, depthTest);

                indices += 3;
            }
        }
    }

    void DebugRenderer::AddCircle(const Vector3& center, const Vector3& normal, float radius, const Color& color, int steps, bool depthTest)
    {
        Quaternion orientation;
        orientation.FromRotationTo(Vector3::UP, normal.Normalized());
        Vector3 p = orientation * Vector3(radius, 0, 0) + center;
        unsigned uintColor = color.ToUInt();

        for(int i = 1; i <= steps; ++i)
        {
            const float angle = (float)i / (float)steps * 360.0f;
            Vector3 v(radius * Cos(angle), 0, radius * Sin(angle));
            Vector3 c = orientation * v + center;
            AddLine(p, c, uintColor, depthTest);
            p = c;
        }

        p = center + normal * (radius / 4.0f);
        AddLine(center, p, uintColor, depthTest);
    }

    void DebugRenderer::AddCross(const Vector3& center, float size, const Color& color, bool depthTest)
    {
        unsigned uintColor = color.ToUInt();

        float halfSize = size / 2.0f;
        for (int i = 0; i < 3; ++i)
        {
            float start[3] = { center.x_, center.y_, center.z_ };
            float end[3] = { center.x_, center.y_, center.z_ };
            start[i] -= halfSize;
            end[i] += halfSize;
            AddLine(Vector3(start), Vector3(end), uintColor, depthTest);
        }
    }

    void DebugRenderer::AddQuad(const Vector3& center, float width, float height, const Color& color, bool depthTest)
    {
        unsigned uintColor = color.ToUInt();

        Vector3 v0(center.x_ - width / 2, center.y_, center.z_ - height / 2);
        Vector3 v1(center.x_ + width / 2, center.y_, center.z_ - height / 2);
        Vector3 v2(center.x_ + width / 2, center.y_, center.z_ + height / 2);
        Vector3 v3(center.x_ - width / 2, center.y_, center.z_ + height / 2);
        AddLine(v0, v1, uintColor, depthTest);
        AddLine(v1, v2, uintColor, depthTest);
        AddLine(v2, v3, uintColor, depthTest);
        AddLine(v3, v0, uintColor, depthTest);
    }

    void DebugRenderer::Render()
    {
        if (!HasContent())
            return;

        auto* graphics = GetSubsystem<Graphics>();
        // Engine does not render when window is closed or device is lost
        assert(graphics && graphics->IsInitialized() && !graphics->IsDeviceLost());

        ShaderVariation* vs = graphics->GetShader(VS, "Basic", "VERTEXCOLOR");
        ShaderVariation* ps = graphics->GetShader(PS, "Basic", "VERTEXCOLOR");

        unsigned numVertices = (lines_.Size() + noDepthLines_.Size()) * 2 + (triangles_.Size() + noDepthTriangles_.Size()) * 3;
        // Resize the vertex buffer if too small or much too large
        if (vertexBuffer_->GetVertexCount() < numVertices || vertexBuffer_->GetVertexCount() > numVertices * 2)
            vertexBuffer_->SetSize(numVertices, MASK_POSITION | MASK_COLOR, true);

        auto* dest = (float*)vertexBuffer_->Lock(0, numVertices, true);
        if (!dest)
            return;

        for (unsigned i = 0; i < lines_.Size(); ++i)
        {
            const DebugLine& line = lines_[i];

            dest[0] = line.start_.x_;
            dest[1] = line.start_.y_;
            dest[2] = line.start_.z_;
            ((unsigned&)dest[3]) = line.color_;
            dest[4] = line.end_.x_;
            dest[5] = line.end_.y_;
            dest[6] = line.end_.z_;
            ((unsigned&)dest[7]) = line.color_;

            dest += 8;
        }

        for (unsigned i = 0; i < noDepthLines_.Size(); ++i)
        {
            const DebugLine& line = noDepthLines_[i];

            dest[0] = line.start_.x_;
            dest[1] = line.start_.y_;
            dest[2] = line.start_.z_;
            ((unsigned&)dest[3]) = line.color_;
            dest[4] = line.end_.x_;
            dest[5] = line.end_.y_;
            dest[6] = line.end_.z_;
            ((unsigned&)dest[7]) = line.color_;

            dest += 8;
        }

        for (unsigned i = 0; i < triangles_.Size(); ++i)
        {
            const DebugTriangle& triangle = triangles_[i];

            dest[0] = triangle.v1_.x_;
            dest[1] = triangle.v1_.y_;
            dest[2] = triangle.v1_.z_;
            ((unsigned&)dest[3]) = triangle.color_;

            dest[4] = triangle.v2_.x_;
            dest[5] = triangle.v2_.y_;
            dest[6] = triangle.v2_.z_;
            ((unsigned&)dest[7]) = triangle.color_;

            dest[8] = triangle.v3_.x_;
            dest[9] = triangle.v3_.y_;
            dest[10] = triangle.v3_.z_;
            ((unsigned&)dest[11]) = triangle.color_;

            dest += 12;
        }

        for (unsigned i = 0; i < noDepthTriangles_.Size(); ++i)
        {
            const DebugTriangle& triangle = noDepthTriangles_[i];

            dest[0] = triangle.v1_.x_;
            dest[1] = triangle.v1_.y_;
            dest[2] = triangle.v1_.z_;
            ((unsigned&)dest[3]) = triangle.color_;

            dest[4] = triangle.v2_.x_;
            dest[5] = triangle.v2_.y_;
            dest[6] = triangle.v2_.z_;
            ((unsigned&)dest[7]) = triangle.color_;

            dest[8] = triangle.v3_.x_;
            dest[9] = triangle.v3_.y_;
            dest[10] = triangle.v3_.z_;
            ((unsigned&)dest[11]) = triangle.color_;

            dest += 12;
        }

        vertexBuffer_->Unlock();

        graphics->SetBlendMode(lineAntiAlias_ ? BLEND_ALPHA : BLEND_REPLACE);
        graphics->SetColorWrite(true);
        graphics->SetCullMode(CULL_NONE);
        graphics->SetDepthWrite(true);
        graphics->SetLineAntiAlias(lineAntiAlias_);
        graphics->SetScissorTest(false);
        graphics->SetStencilTest(false);
        graphics->SetShaders(vs, ps);
        graphics->SetShaderParameter(VSP_MODEL, Matrix3x4::IDENTITY);
        graphics->SetShaderParameter(VSP_VIEW, view_);
        graphics->SetShaderParameter(VSP_VIEWINV, view_.Inverse());
        graphics->SetShaderParameter(VSP_VIEWPROJ, gpuProjection_ * view_);
        graphics->SetShaderParameter(PSP_MATDIFFCOLOR, Color(1.0f, 1.0f, 1.0f, 1.0f));
        graphics->SetVertexBuffer(vertexBuffer_);

        unsigned start = 0;
        unsigned count = 0;
        if (lines_.Size())
        {
            count = lines_.Size() * 2;
            graphics->SetDepthTest(CMP_LESSEQUAL);
            graphics->Draw(LINE_LIST, start, count);
            start += count;
        }
        if (noDepthLines_.Size())
        {
            count = noDepthLines_.Size() * 2;
            graphics->SetDepthTest(CMP_ALWAYS);
            graphics->Draw(LINE_LIST, start, count);
            start += count;
        }

        graphics->SetBlendMode(BLEND_ALPHA);
        graphics->SetDepthWrite(false);

        if (triangles_.Size())
        {
            count = triangles_.Size() * 3;
            graphics->SetDepthTest(CMP_LESSEQUAL);
            graphics->Draw(TRIANGLE_LIST, start, count);
            start += count;
        }
        if (noDepthTriangles_.Size())
        {
            count = noDepthTriangles_.Size() * 3;
            graphics->SetDepthTest(CMP_ALWAYS);
            graphics->Draw(TRIANGLE_LIST, start, count);
        }

        graphics->SetLineAntiAlias(false);
    }

    void DebugRenderer::SetView(Camera* camera)
    {
        if (!camera)
            return;

        view_ = camera->GetView();
        projection_ = camera->GetProjection();
        gpuProjection_ = camera->GetGPUProjection();
        frustum_ = camera->GetFrustum();
    }

    bool DebugRenderer::HasContent() const
    {
        return !(lines_.Empty() && noDepthLines_.Empty() && triangles_.Empty() && noDepthTriangles_.Empty());
    }

    void DebugRenderer::HandleEndFrame(StringHash eventType, VariantMap &eventData)
    {
        // When the amount of debug geometry is reduced, release memory
        unsigned linesSize = lines_.Size();
        unsigned noDepthLinesSize = noDepthLines_.Size();
        unsigned trianglesSize = triangles_.Size();
        unsigned noDepthTrianglesSize = noDepthTriangles_.Size();

        lines_.Clear();
        noDepthLines_.Clear();
        triangles_.Clear();
        noDepthTriangles_.Clear();

        if (lines_.Capacity() > linesSize * 2)
            lines_.Reserve(linesSize);
        if (noDepthLines_.Capacity() > noDepthLinesSize * 2)
            noDepthLines_.Reserve(noDepthLinesSize);
        if (triangles_.Capacity() > trianglesSize * 2)
            triangles_.Reserve(trianglesSize);
        if (noDepthTriangles_.Capacity() > noDepthTrianglesSize * 2)
            noDepthTriangles_.Reserve(noDepthTrianglesSize);
    }
}