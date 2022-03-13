//
// Created by luchu on 2022/2/25.
//

#include "Core/WorkQueue.h"
#include "Graphics/Camera.h"
#include "Graphics/Geometry.h"
#include "Graphics/Graphics.h"
#include "Graphics/GraphicsEvents.h"
#include "Graphics/GraphicsImpl.h"
#include "Graphics/GraphicsDefs.h"
#include "Graphics/Material.h"
#include "Graphics/Octree.h"
#include "Graphics/OctreeQuery.h"
#include "Graphics/Renderer.h"
#include "Graphics/RenderPath.h"
#include "Graphics/ShaderVariation.h"
#include "Graphics/Technique.h"
#include "Graphics/Texture2D.h"
#include "Graphics/View.h"
#include "IO/Log.h"
#include "Resource/ResourceCache.h"
#include "Scene/Scene.h"


namespace My3D
{

    View::View(Context* context)
        : Object(context)
        , graphics_(GetSubsystem<Graphics>())
        , renderer_(GetSubsystem<Renderer>())
    {

    }

    Graphics* View::GetGraphics() const
    {
        return graphics_;
    }

    Renderer* View::GetRenderer() const
    {
        return renderer_;
    }

    void View::SetGlobalShaderParameters()
    {
        graphics_->SetShaderParameter(VSP_DELTATIME, frame_.timeStep_);
        graphics_->SetShaderParameter(PSP_DELTATIME, frame_.timeStep_);

        if (scene_)
        {
            float elapsedTime = scene_->GetElapsedTime();
            graphics_->SetShaderParameter(VSP_ELAPSEDTIME, elapsedTime);
            graphics_->SetShaderParameter(PSP_ELAPSEDTIME, elapsedTime);
        }

        SendViewEvent(E_VIEWGLOBALSHADERPARAMETERS);
    }

    void View::SetCameraShaderParameters(Camera* camera)
    {
        if (!camera)
            return;
        Matrix3x4 cameraEffectiveTransform = camera->GetEffectiveWorldTransform();

        graphics_->SetShaderParameter(VSP_CAMERAPOS, cameraEffectiveTransform.Translation());
        graphics_->SetShaderParameter(VSP_VIEWINV, cameraEffectiveTransform);
        graphics_->SetShaderParameter(VSP_VIEW, camera->GetView());
        graphics_->SetShaderParameter(PSP_CAMERAPOS, cameraEffectiveTransform.Translation());

        float nearClip = camera->GetNearClip();
        float farClip = camera->GetFarClip();
        graphics_->SetShaderParameter(VSP_NEARCLIP, nearClip);
        graphics_->SetShaderParameter(VSP_FARCLIP, farClip);
        graphics_->SetShaderParameter(PSP_NEARCLIP, nearClip);
        graphics_->SetShaderParameter(PSP_FARCLIP, farClip);

        Vector4 depthMode = Vector4::ZERO;
        if (camera->IsOrthographic())
        {
            depthMode.x_ = 1.0f;
#ifdef MY3D_OPENGL
            depthMode.z_ = 0.5f;
            depthMode.w_ = 0.5f;
#else
            depthMode.z_ = 1.0f;
#endif
        }
        else
            depthMode.w_ = 1.0f / camera->GetFarClip();

        graphics_->SetShaderParameter(VSP_DEPTHMODE, depthMode);

        Vector4 depthReconstruct
                (farClip / (farClip - nearClip), -nearClip / (farClip - nearClip), camera->IsOrthographic() ? 1.0f : 0.0f,
                 camera->IsOrthographic() ? 0.0f : 1.0f);
        graphics_->SetShaderParameter(PSP_DEPTHRECONSTRUCT, depthReconstruct);

        Vector3 nearVector, farVector;
        camera->GetFrustumSize(nearVector, farVector);
        graphics_->SetShaderParameter(VSP_FRUSTUMSIZE, farVector);

        Matrix4 projection = camera->GetGPUProjection();
#ifdef MY3D_OPENGL
        // Add constant depth bias manually to the projection matrix due to glPolygonOffset() inconsistency
        float constantBias = 2.0f * graphics_->GetDepthConstantBias();
        projection.m22_ += projection.m32_ * constantBias;
        projection.m23_ += projection.m33_ * constantBias;
#endif

        graphics_->SetShaderParameter(VSP_VIEWPROJ, projection * camera->GetView());

        // If in a scene pass and the command defines shader parameters, set them now
        if (passCommand_)
            SetCommandShaderParameters(*passCommand_);
    }

    void View::SetCommandShaderParameters(const RenderPathCommand& command)
    {
        const HashMap<StringHash, Variant>& parameters = command.shaderParameters_;
        for (HashMap<StringHash, Variant>::ConstIterator k = parameters.Begin(); k != parameters.End(); ++k)
            graphics_->SetShaderParameter(k->first_, k->second_);
    }

    void View::SendViewEvent(StringHash eventType)
    {
        using namespace BeginViewRender;

        VariantMap& eventData = GetEventDataMap();

        eventData[P_VIEW] = this;
        eventData[P_SURFACE] = renderTarget_;
        eventData[P_TEXTURE] = (renderTarget_ ? renderTarget_->GetParentTexture() : nullptr);
        eventData[P_SCENE] = scene_;
        eventData[P_CAMERA] = cullCamera_;

        renderer_->SendEvent(eventType, eventData);
    }
}