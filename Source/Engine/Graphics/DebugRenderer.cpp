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