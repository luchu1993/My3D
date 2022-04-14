//
// Created by luchu on 2022/2/25.
//

#include "Core/WorkQueue.h"
#include "Container/Sort.h"
#include "Graphics/Camera.h"
#include "Graphics/DebugRenderer.h"
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
#include "Graphics/VertexBuffer.h"
#include "Graphics/Material.h"
#include "Graphics/OcclusionBuffer.h"
#include "Resource/ResourceCache.h"
#include "Scene/Scene.h"
#include "IO/Log.h"
#include "IO/FileSystem.h"


namespace My3D
{
    /// Frustum octree query for shadowcasters.
    class ShadowCasterOctreeQuery : public FrustumOctreeQuery
    {
    public:
        /// Construct with frustum and query parameters.
        ShadowCasterOctreeQuery(PODVector<Drawable*>& result, const Frustum& frustum, unsigned char drawableFlags = DRAWABLE_ANY,
            unsigned viewMask = DEFAULT_VIEWMASK) : FrustumOctreeQuery(result, frustum, drawableFlags, viewMask)
        {
        }

        /// Intersection test for drawables.
        void TestDrawables(Drawable** start, Drawable** end, bool inside) override
        {
            while (start != end)
            {
                Drawable* drawable = *start++;

                if (drawable->GetCastShadows() && (drawable->GetDrawableFlags() & drawableFlags_) &&
                    (drawable->GetViewMask() & viewMask_))
                {
                    if (inside || frustum_.IsInsideFast(drawable->GetWorldBoundingBox()))
                        result_.Push(drawable);
                }
            }
        }
    };

    /// Frustum octree query for zones and occluders.
    class ZoneOccluderOctreeQuery : public FrustumOctreeQuery
    {
    public:
        /// Construct with frustum and query parameters.
        ZoneOccluderOctreeQuery(PODVector<Drawable*>& result, const Frustum& frustum, unsigned char drawableFlags = DRAWABLE_ANY,
            unsigned viewMask = DEFAULT_VIEWMASK) : FrustumOctreeQuery(result, frustum, drawableFlags, viewMask)
        {
        }

        /// Intersection test for drawables.
        void TestDrawables(Drawable** start, Drawable** end, bool inside) override
        {
            while (start != end)
            {
                Drawable* drawable = *start++;
                unsigned char flags = drawable->GetDrawableFlags();

                if ((flags == DRAWABLE_ZONE || (flags == DRAWABLE_GEOMETRY && drawable->IsOccluder())) &&
                    (drawable->GetViewMask() & viewMask_))
                {
                    if (inside || frustum_.IsInsideFast(drawable->GetWorldBoundingBox()))
                        result_.Push(drawable);
                }
            }
        }
    };

    /// Frustum octree query with occlusion.
    class OccludedFrustumOctreeQuery : public FrustumOctreeQuery
    {
    public:
        /// Construct with frustum, occlusion buffer and query parameters.
        OccludedFrustumOctreeQuery(PODVector<Drawable*>& result, const Frustum& frustum, OcclusionBuffer* buffer,
            unsigned char drawableFlags = DRAWABLE_ANY, unsigned viewMask = DEFAULT_VIEWMASK)
            : FrustumOctreeQuery(result, frustum, drawableFlags, viewMask)
            , buffer_(buffer)
        {
        }

        /// Intersection test for an octant.
        Intersection TestOctant(const BoundingBox& box, bool inside) override
        {
            if (inside)
                return buffer_->IsVisible(box) ? INSIDE : OUTSIDE;
            else
            {
                Intersection result = frustum_.IsInside(box);
                if (result != OUTSIDE && !buffer_->IsVisible(box))
                    result = OUTSIDE;
                return result;
            }
        }

        /// Intersection test for drawables. Note: drawable occlusion is performed later in worker threads.
        void TestDrawables(Drawable** start, Drawable** end, bool inside) override
        {
            while (start != end)
            {
                Drawable* drawable = *start++;

                if ((drawable->GetDrawableFlags() & drawableFlags_) && (drawable->GetViewMask() & viewMask_))
                {
                    if (inside || frustum_.IsInsideFast(drawable->GetWorldBoundingBox()))
                        result_.Push(drawable);
                }
            }
        }

        /// Occlusion buffer.
        OcclusionBuffer* buffer_;
    };

    void CheckVisibilityWork(const WorkItem* item, unsigned threadIndex)
    {
        auto* view = reinterpret_cast<View*>(item->aux_);
        auto** start = reinterpret_cast<Drawable**>(item->start_);
        auto** end = reinterpret_cast<Drawable**>(item->end_);
        OcclusionBuffer* buffer = view->occlusionBuffer_;
        const Matrix3x4& viewMatrix = view->cullCamera_->GetView();
        Vector3 viewZ = Vector3(viewMatrix.m20_, viewMatrix.m21_, viewMatrix.m22_);
        Vector3 absViewZ = viewZ.Abs();
        unsigned cameraViewMask = view->cullCamera_->GetViewMask();
        bool cameraZoneOverride = view->cameraZoneOverride_;
        PerThreadSceneResult& result = view->sceneResults_[threadIndex];

        while (start != end)
        {
            Drawable* drawable = *start++;

            if (!buffer || !drawable->IsOccludee() || buffer->IsVisible(drawable->GetWorldBoundingBox()))
            {
                drawable->UpdateBatches(view->frame_);
                // If draw distance non-zero, update and check it
                float maxDistance = drawable->GetDrawDistance();
                if (maxDistance > 0.0f)
                {
                    if (drawable->GetDistance() > maxDistance)
                        continue;
                }

                drawable->MarkInView(view->frame_);

                // For geometries, find zone, clear lights and calculate view space Z range
                if (drawable->GetDrawableFlags() & DRAWABLE_GEOMETRY)
                {
                    Zone* drawableZone = drawable->GetZone();
                    if (!cameraZoneOverride &&
                        (drawable->IsZoneDirty() || !drawableZone || (drawableZone->GetViewMask() & cameraViewMask) == 0))
                        view->FindZone(drawable);

                    const BoundingBox& geomBox = drawable->GetWorldBoundingBox();
                    Vector3 center = geomBox.Center();
                    Vector3 edge = geomBox.Size() * 0.5f;

                    // Do not add "infinite" objects like skybox to prevent shadow map focusing behaving erroneously
                    if (edge.LengthSquared() < M_LARGE_VALUE * M_LARGE_VALUE)
                    {
                        float viewCenterZ = viewZ.DotProduct(center) + viewMatrix.m23_;
                        float viewEdgeZ = absViewZ.DotProduct(edge);
                        float minZ = viewCenterZ - viewEdgeZ;
                        float maxZ = viewCenterZ + viewEdgeZ;
                        drawable->SetMinMaxZ(viewCenterZ - viewEdgeZ, viewCenterZ + viewEdgeZ);
                        result.minZ_ = Min(result.minZ_, minZ);
                        result.maxZ_ = Max(result.maxZ_, maxZ);
                    }
                    else
                        drawable->SetMinMaxZ(M_LARGE_VALUE, M_LARGE_VALUE);

                    result.geometries_.Push(drawable);
                }
                else if (drawable->GetDrawableFlags() & DRAWABLE_LIGHT)
                {
                    auto* light = static_cast<Light*>(drawable);
                    // Skip lights with zero brightness or black color
                    if (!light->GetEffectiveColor().Equals(Color::BLACK))
                        result.lights_.Push(light);
                }
            }
        }
    }


    View::View(Context* context)
        : Object(context)
        , graphics_(GetSubsystem<Graphics>())
        , renderer_(GetSubsystem<Renderer>())
    {
        // Create octree query and scene results vector for each thread
        unsigned numThreads = GetSubsystem<WorkQueue>()->GetNumThreads() + 1; // Worker threads + main thread
        tempDrawables_.Resize(numThreads);
        sceneResults_.Resize(numThreads);
    }

    bool View::Define(RenderSurface* renderTarget, Viewport* viewport)
    {
        sourceView_ = nullptr;
        renderPath_ = viewport->GetRenderPath();

        renderTarget_ = renderTarget;
        drawDebug_ = viewport->GetDrawDebug();

        // Validate the rect and calculate size. If zero rect, use whole rendertarget size
        int rtWidth = renderTarget ? renderTarget->GetWidth() : graphics_->GetWidth();
        int rtHeight = renderTarget ? renderTarget->GetHeight() : graphics_->GetHeight();
        const IntRect& rect = viewport->GetRect();

        if (rect != IntRect::ZERO)
        {
            viewRect_.left_ = Clamp(rect.left_, 0, rtWidth - 1);
            viewRect_.top_ = Clamp(rect.top_, 0, rtHeight - 1);
            viewRect_.right_ = Clamp(rect.right_, viewRect_.left_ + 1, rtWidth);
            viewRect_.bottom_ = Clamp(rect.bottom_, viewRect_.top_ + 1, rtHeight);
        }
        else
            viewRect_ = IntRect(0, 0, rtWidth, rtHeight);

        viewSize_ = viewRect_.Size();
        rtSize_ = IntVector2(rtWidth, rtHeight);

    #ifdef MY3D_OPENGL
        if (renderTarget_)
        {
            viewRect_.bottom_ = rtHeight - viewRect_.top_;
            viewRect_.top_ = viewRect_.bottom_ - viewSize_.y_;
        }
    #endif

        scene_ = viewport->GetScene();
        cullCamera_ = viewport->GetCullCamera();
        camera_ = viewport->GetCamera();

        if (!cullCamera_)
            cullCamera_ = camera_;
        else
        {
            // If view specifies a culling camera (view preparation sharing), check if already prepared
            sourceView_ = renderer_->GetPreparedView(cullCamera_);
            if (sourceView_ && sourceView_->scene_ == scene_ && sourceView_->renderPath_ == renderPath_)
            {
                // Copy properties needed later in rendering
                deferred_ = sourceView_->deferred_;
                deferredAmbient_ = sourceView_->deferredAmbient_;
                useLitBase_ = sourceView_->useLitBase_;
                hasScenePasses_ = sourceView_->hasScenePasses_;
                noStencil_ = sourceView_->noStencil_;
                lightVolumeCommand_ = sourceView_->lightVolumeCommand_;
                forwardLightsCommand_ = sourceView_->forwardLightsCommand_;
                octree_ = sourceView_->octree_;
                return true;
            }
            else
            {
                // Mismatch in scene or renderpath, fall back to unique view preparation
                sourceView_ = nullptr;
            }
        }

        // Set default passes
        gBufferPassIndex_ = M_MAX_UNSIGNED;
        basePassIndex_ = Technique::GetPassIndex("base");
        alphaPassIndex_ = Technique::GetPassIndex("alpha");
        lightPassIndex_ = Technique::GetPassIndex("light");
        litBasePassIndex_ = Technique::GetPassIndex("litbase");
        litAlphaPassIndex_ = Technique::GetPassIndex("litalpha");

        deferred_ = false;
        deferredAmbient_ = false;
        useLitBase_ = false;
        hasScenePasses_ = false;
        noStencil_ = false;
        lightVolumeCommand_ = nullptr;
        forwardLightsCommand_ = nullptr;

        scenePasses_.Clear();
        geometriesUpdated_ = false;

    #ifdef MY3D_OPENGL
        for (unsigned i = 0; i < renderPath_->commands_.Size(); ++i)
        {
            const RenderPathCommand& command = renderPath_->commands_[i];
            if (!command.enabled_)
                continue;
            if (command.depthStencilName_.Length())
            {
                // Using a readable depth texture will disable light stencil optimizations on OpenGL, as for compatibility reasons
                // we are using a depth format without stencil channel
                noStencil_ = true;
                break;
            }
        }
    #endif

        // Make sure that all necessary batch queues exist
        for (unsigned i = 0; i < renderPath_->commands_.Size(); ++i)
        {
            RenderPathCommand& command = renderPath_->commands_[i];
            if (!command.enabled_)
                continue;

            if (command.type_ == CMD_SCENEPASS)
            {
                hasScenePasses_ = true;

                ScenePassInfo info{};
                info.passIndex_ = command.passIndex_ = Technique::GetPassIndex(command.pass_);
                info.allowInstancing_ = command.sortMode_ != SORT_BACKTOFRONT;
                info.markToStencil_ = !noStencil_ && command.markToStencil_;
                info.vertexLights_ = command.vertexLights_;

                // Check scenepass metadata for defining custom passes which interact with lighting
                if (!command.metadata_.Empty())
                {
                    if (command.metadata_ == "gbuffer")
                        gBufferPassIndex_ = command.passIndex_;
                    else if (command.metadata_ == "base" && command.pass_ != "base")
                    {
                        basePassIndex_ = command.passIndex_;
                        litBasePassIndex_ = Technique::GetPassIndex("lit" + command.pass_);
                    }
                    else if (command.metadata_ == "alpha" && command.pass_ != "alpha")
                    {
                        alphaPassIndex_ = command.passIndex_;
                        litAlphaPassIndex_ = Technique::GetPassIndex("lit" + command.pass_);
                    }
                }

                HashMap<unsigned, BatchQueue>::Iterator j = batchQueues_.Find(info.passIndex_);
                if (j == batchQueues_.End())
                    j = batchQueues_.Insert(Pair<unsigned, BatchQueue>(info.passIndex_, BatchQueue()));
                info.batchQueue_ = &j->second_;
                SetQueueShaderDefines(*info.batchQueue_, command);

                scenePasses_.Push(info);
            }
            // Allow a custom forward light pass
            else if (command.type_ == CMD_FORWARDLIGHTS && !command.pass_.Empty())
                lightPassIndex_ = command.passIndex_ = Technique::GetPassIndex(command.pass_);
        }

        octree_ = nullptr;
        // Get default zone first in case we do not have zones defined
        cameraZone_ = farClipZone_ = renderer_->GetDefaultZone();

        if (hasScenePasses_)
        {
            if (!scene_ || !cullCamera_ || !cullCamera_->IsEnabledEffective())
                return false;

            // If scene is loading scene content asynchronously, it is incomplete and should not be rendered
            if (scene_->IsAsyncLoading() && scene_->GetAsyncLoadMode() > LOAD_RESOURCES_ONLY)
                return false;

            octree_ = scene_->GetComponent<Octree>();
            if (!octree_)
                return false;

            // Do not accept view if camera projection is illegal
            // (there is a possibility of crash if occlusion is used and it can not clip properly)
            if (!cullCamera_->IsProjectionValid())
                return false;
        }

        // Go through commands to check for deferred rendering and other flags
        for (unsigned i = 0; i < renderPath_->commands_.Size(); ++i)
        {
            const RenderPathCommand& command = renderPath_->commands_[i];
            if (!command.enabled_)
                continue;

            // Check if ambient pass and G-buffer rendering happens at the same time
            if (command.type_ == CMD_SCENEPASS && command.outputs_.Size() > 1)
            {
                if (CheckViewportWrite(command))
                    deferredAmbient_ = true;
            }
            else if (command.type_ == CMD_LIGHTVOLUMES)
            {
                lightVolumeCommand_ = &command;
                deferred_ = true;
            }
            else if (command.type_ == CMD_FORWARDLIGHTS)
            {
                forwardLightsCommand_ = &command;
                useLitBase_ = command.useLitBase_;
            }
        }

        drawShadows_ = renderer_->GetDrawShadows();
        materialQuality_ = renderer_->GetMaterialQuality();
        maxOccluderTriangles_ = renderer_->GetMaxOccluderTriangles();
        minInstances_ = renderer_->GetMinInstances();

        // Set possible quality overrides from the camera
        // Note that the culling camera is used here (its settings are authoritative) while the render camera
        // will be just used for the final view & projection matrices
        unsigned viewOverrideFlags = cullCamera_ ? cullCamera_->GetViewOverrideFlags() : VO_NONE;
        if (viewOverrideFlags & VO_LOW_MATERIAL_QUALITY)
            materialQuality_ = QUALITY_LOW;
        if (viewOverrideFlags & VO_DISABLE_SHADOWS)
            drawShadows_ = false;
        if (viewOverrideFlags & VO_DISABLE_OCCLUSION)
            maxOccluderTriangles_ = 0;

        // Occlusion buffer has constant width. If resulting height would be too large due to aspect ratio, disable occlusion
        if (viewSize_.y_ > viewSize_.x_ * 4)
            maxOccluderTriangles_ = 0;

        return true;
    }

    void View::Update(const FrameInfo &frame)
    {
        // No need to update if using another prepared view
        if (sourceView_)
            return;

        frame_.camera_ = cullCamera_;
        frame_.timeStep_ = frame.timeStep_;
        frame_.frameNumber_ = frame.frameNumber_;
        frame_.viewSize_ = viewSize_;

        using namespace BeginViewUpdate;

        SendViewEvent(E_BEGINVIEWUPDATE);

        int maxSortedInstances = renderer_->GetMaxSortedInstances();

        // Clear buffers, geometry, light, occluder & batch list
        renderTargets_.Clear();
        geometries_.Clear();
        lights_.Clear();
        zones_.Clear();
        occluders_.Clear();
        activeOccluders_ = 0;
        vertexLightQueues_.Clear();
        for (HashMap<unsigned, BatchQueue>::Iterator i = batchQueues_.Begin(); i != batchQueues_.End(); ++i)
            i->second_.Clear(maxSortedInstances);

        if (hasScenePasses_ && (!cullCamera_ || !octree_))
        {
            SendViewEvent(E_ENDVIEWUPDATE);
            return;
        }

        // Set automatic aspect ratio if required
        if (cullCamera_ && cullCamera_->GetAutoAspectRatio())
            cullCamera_->SetAspectRatioInternal((float)frame_.viewSize_.x_ / (float)frame_.viewSize_.y_);

        GetDrawables();
        GetBatches();
        renderer_->StorePreparedView(this, cullCamera_);

        SendViewEvent(E_ENDVIEWUPDATE);
    }

    void View::Render()
    {
        SendViewEvent(E_BEGINVIEWRENDER);

        if (hasScenePasses_ && (!octree_ || !camera_))
        {
            SendViewEvent(E_ENDVIEWRENDER);
            return;
        }

        UpdateGeometries();

        // Allocate screen buffers as necessary
        AllocateScreenBuffers();
        SendViewEvent(E_VIEWBUFFERSREADY);

        // Forget parameter sources from the previous view
        graphics_->ClearParameterSources();

        if (renderer_->GetDynamicInstancing() && graphics_->GetInstancingSupport())
            PrepareInstancingBuffer();

        // It is possible, though not recommended, that the same camera is used for multiple main views. Set automatic aspect ratio
        // to ensure correct projection will be used
        if (camera_ && camera_->GetAutoAspectRatio())
            camera_->SetAspectRatioInternal((float)(viewSize_.x_) / (float)(viewSize_.y_));

#ifdef MY3D_OPENGL
        if (renderTarget_)
        {
            // On OpenGL, flip the projection if rendering to a texture so that the texture can be addressed in the same way
            // as a render texture produced on Direct3D9
            // Note that the state of the FlipVertical mode is toggled here rather than enabled
            // The reason for this is that we want the mode to be the opposite of what the user has currently set for the
            // camera when rendering to texture for OpenGL
            // This mode is returned to the original state by toggling it again below, after the render
            if (camera_)
                camera_->SetFlipVertical(!camera_->GetFlipVertical());
        }
#endif

        // Render
        ExecuteRenderPathCommands();

        // Reset state after commands
        graphics_->SetFillMode(FILL_SOLID);
        graphics_->SetLineAntiAlias(false);
        graphics_->SetClipPlane(false);
        graphics_->SetColorWrite(true);
        graphics_->SetDepthBias(0.0f, 0.0f);
        graphics_->SetScissorTest(false);
        graphics_->SetStencilTest(false);

        // Draw the associated debug geometry now if enabled
        if (drawDebug_ && octree_ && camera_)
        {
            auto* debug = octree_->GetComponent<DebugRenderer>();
            if (debug && debug->IsEnabledEffective() && debug->HasContent())
            {
                // If used resolve from backbuffer, blit first to the backbuffer to ensure correct depth buffer on OpenGL
                // Otherwise use the last rendertarget and blit after debug geometry
                if (usedResolve_ && currentRenderTarget_ != renderTarget_)
                {
                    BlitFramebuffer(currentRenderTarget_->GetParentTexture(), renderTarget_, false);
                    currentRenderTarget_ = renderTarget_;
                    lastCustomDepthSurface_ = nullptr;
                }

                graphics_->SetRenderTarget(0, currentRenderTarget_);
                for (unsigned i = 1; i < MAX_RENDERTARGETS; ++i)
                    graphics_->SetRenderTarget(i, (RenderSurface*)nullptr);

                // If a custom depth surface was used, use it also for debug rendering
                graphics_->SetDepthStencil(lastCustomDepthSurface_ ? lastCustomDepthSurface_ : GetDepthStencil(currentRenderTarget_));

                IntVector2 rtSizeNow = graphics_->GetRenderTargetDimensions();
                IntRect viewport = (currentRenderTarget_ == renderTarget_) ? viewRect_ : IntRect(0, 0, rtSizeNow.x_,
                                                                                                 rtSizeNow.y_);
                graphics_->SetViewport(viewport);

                debug->SetView(camera_);
                debug->Render();
            }
        }

#ifdef MY3D_OPENGL
        if (renderTarget_)
        {
            // Restores original setting of FlipVertical when flipped by code above.
            if (camera_)
                camera_->SetFlipVertical(!camera_->GetFlipVertical());
        }
#endif
        // Run framebuffer blitting if necessary. If scene was resolved from backbuffer, do not touch depth
        // (backbuffer should contain proper depth already)
        if (currentRenderTarget_ != renderTarget_)
            BlitFramebuffer(currentRenderTarget_->GetParentTexture(), renderTarget_, !usedResolve_);

        SendViewEvent(E_ENDVIEWRENDER);
    }

    Graphics* View::GetGraphics() const
    {
        return graphics_;
    }

    Renderer* View::GetRenderer() const
    {
        return renderer_;
    }

    View* View::GetSourceView() const
    {
        return sourceView_;
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

    void View::SetGBufferShaderParameters(const IntVector2& texSize, const IntRect& viewRect)
    {
        auto texWidth = (float)texSize.x_;
        auto texHeight = (float)texSize.y_;
        float widthRange = 0.5f * viewRect.Width() / texWidth;
        float heightRange = 0.5f * viewRect.Height() / texHeight;

    #ifdef MY3D_OPENGL
        Vector4 bufferUVOffset(((float)viewRect.left_) / texWidth + widthRange,
        1.0f - (((float)viewRect.top_) / texHeight + heightRange), widthRange, heightRange);
    #else
        const Vector2& pixelUVOffset = Graphics::GetPixelUVOffset();
        Vector4 bufferUVOffset((pixelUVOffset.x_ + (float)viewRect.left_) / texWidth + widthRange,
                               (pixelUVOffset.y_ + (float)viewRect.top_) / texHeight + heightRange, widthRange, heightRange);
    #endif
        graphics_->SetShaderParameter(VSP_GBUFFEROFFSETS, bufferUVOffset);

        float invSizeX = 1.0f / texWidth;
        float invSizeY = 1.0f / texHeight;
        graphics_->SetShaderParameter(PSP_GBUFFERINVSIZE, Vector2(invSizeX, invSizeY));
    }

    void View::DrawFullscreenQuad(bool setIdentityProjection)
    {
        Geometry* geometry = renderer_->GetQuadGeometry();

        // If no camera, no choice but to use identity projection
        if (!camera_)
            setIdentityProjection = true;

        if (setIdentityProjection)
        {
            Matrix3x4 model = Matrix3x4::IDENTITY;
            Matrix4 projection = Matrix4::IDENTITY;
        #ifdef MY3D_OPENGL
            if (camera_ && camera_->GetFlipVertical())
            projection.m11_ = -1.0f;
            model.m23_ = 0.0f;
        #else
            model.m23_ = 0.5f;
        #endif

            graphics_->SetShaderParameter(VSP_MODEL, model);
            graphics_->SetShaderParameter(VSP_VIEWPROJ, projection);
        }
        else
            graphics_->SetShaderParameter(VSP_MODEL, Light::GetFullscreenQuadTransform(camera_));

        graphics_->SetCullMode(CULL_NONE);
        graphics_->ClearTransformSources();

        geometry->Draw(graphics_);
    }


    void View::GetDrawables()
    {
        if (!octree_ || !cullCamera_)
            return;

        auto* queue = GetSubsystem<WorkQueue>();
        PODVector<Drawable*>& tempDrawables = tempDrawables_[0];

        // Get zones and occluders first
        {
            ZoneOccluderOctreeQuery query(tempDrawables, cullCamera_->GetFrustum(),
                DRAWABLE_GEOMETRY | DRAWABLE_ZONE, cullCamera_->GetViewMask());
            octree_->GetDrawables(query);
        }

        highestZonePriority_ = M_MIN_INT;
        int bestPriority = M_MIN_INT;
        Node* cameraNode = cullCamera_->GetNode();
        Vector3 cameraPos = cameraNode->GetWorldPosition();

        for (PODVector<Drawable*>::ConstIterator i = tempDrawables.Begin(); i != tempDrawables.End(); ++i)
        {
            Drawable* drawable = *i;
            unsigned char flags = drawable->GetDrawableFlags();

            if (flags & DRAWABLE_ZONE)
            {
                auto* zone = static_cast<Zone*>(drawable);
                zones_.Push(zone);
                int priority = zone->GetPriority();
                if (priority > highestZonePriority_)
                    highestZonePriority_ = priority;
                if (priority > bestPriority && zone->IsInside(cameraPos))
                {
                    cameraZone_ = zone;
                    bestPriority = priority;
                }
            }
            else
                occluders_.Push(drawable);
        }

        // Determine the zone at far clip distance. If not found, or camera zone has override mode, use camera zone
        cameraZoneOverride_ = cameraZone_->GetOverride();
        if (!cameraZoneOverride_)
        {
            Vector3 farClipPos = cameraPos + cameraNode->GetWorldDirection() * Vector3(0.0f, 0.0f, cullCamera_->GetFarClip());
            bestPriority = M_MIN_INT;

            for (PODVector<Zone*>::Iterator i = zones_.Begin(); i != zones_.End(); ++i)
            {
                int priority = (*i)->GetPriority();
                if (priority > bestPriority && (*i)->IsInside(farClipPos))
                {
                    farClipZone_ = *i;
                    bestPriority = priority;
                }
            }
        }

        if (farClipZone_ == renderer_->GetDefaultZone())
            farClipZone_ = cameraZone_;

        // If occlusion in use, get & render the occluders
        occlusionBuffer_ = nullptr;
        if (maxOccluderTriangles_ > 0)
        {
            UpdateOccluders(occluders_, cullCamera_);
            if (occluders_.Size())
            {
                occlusionBuffer_ = renderer_->GetOcclusionBuffer(cullCamera_);
                DrawOccluders(occlusionBuffer_, occluders_);
            }
        }
        else
            occluders_.Clear();

        // Get lights and geometries. Coarse occlusion for octants is used at this point
        if (occlusionBuffer_)
        {
            OccludedFrustumOctreeQuery query(tempDrawables, cullCamera_->GetFrustum(), occlusionBuffer_,
                DRAWABLE_GEOMETRY | DRAWABLE_LIGHT, cullCamera_->GetViewMask());
            octree_->GetDrawables(query);
        }
        else
        {
            FrustumOctreeQuery query(tempDrawables, cullCamera_->GetFrustum(),
                DRAWABLE_GEOMETRY | DRAWABLE_LIGHT, cullCamera_->GetViewMask());
            octree_->GetDrawables(query);
        }

        // Check drawable occlusion, find zones for moved drawables and collect geometries & lights in worker threads
        {
            for (unsigned i = 0; i < sceneResults_.Size(); ++i)
            {
                PerThreadSceneResult& result = sceneResults_[i];

                result.geometries_.Clear();
                result.lights_.Clear();
                result.minZ_ = M_INFINITY;
                result.maxZ_ = 0.0f;
            }

            int numWorkItems = queue->GetNumThreads() + 1; // Worker threads + main thread
            int drawablesPerItem = tempDrawables.Size() / numWorkItems;

            PODVector<Drawable*>::Iterator start = tempDrawables.Begin();
            // Create a work item for each thread
            for (int i = 0; i < numWorkItems; ++i)
            {
                SharedPtr<WorkItem> item = queue->GetFreeItem();
                item->priority_ = M_MAX_UNSIGNED;
                item->workFunction_ = CheckVisibilityWork;
                item->aux_ = this;

                PODVector<Drawable*>::Iterator end = tempDrawables.End();
                if (i < numWorkItems - 1 && end - start > drawablesPerItem)
                    end = start + drawablesPerItem;

                item->start_ = &(*start);
                item->end_ = &(*end);
                queue->AddWorkItem(item);

                start = end;
            }

            queue->Complete(M_MAX_UNSIGNED);
        }

        // Combine lights, geometries & scene Z range from the threads
        geometries_.Clear();
        lights_.Clear();
        minZ_ = M_INFINITY;
        maxZ_ = 0.0f;

        if (sceneResults_.Size() > 1)
        {
            for (unsigned i = 0; i < sceneResults_.Size(); ++i)
            {
                PerThreadSceneResult& result = sceneResults_[i];
                geometries_.Push(result.geometries_);
                lights_.Push(result.lights_);
                minZ_ = Min(minZ_, result.minZ_);
                maxZ_ = Max(maxZ_, result.maxZ_);
            }
        }
        else
        {
            // If just 1 thread, copy the results directly
            PerThreadSceneResult& result = sceneResults_[0];
            minZ_ = result.minZ_;
            maxZ_ = result.maxZ_;
            Swap(geometries_, result.geometries_);
            Swap(lights_, result.lights_);
        }

        if (minZ_ == M_INFINITY)
            minZ_ = 0.0f;

        // Sort the lights to brightest/closest first, and per-vertex lights first so that per-vertex base pass can be evaluated first
        for (unsigned i = 0; i < lights_.Size(); ++i)
        {
            Light* light = lights_[i];
            light->SetIntensitySortValue(cullCamera_->GetDistance(light->GetNode()->GetWorldPosition()));
            light->SetLightQueue(nullptr);
        }

        Sort(lights_.Begin(), lights_.End(), CompareLights);
    }

    void View::SetQueueShaderDefines(BatchQueue& queue, const RenderPathCommand& command)
    {
        String vsDefines = command.vertexShaderDefines_.Trimmed();
        String psDefines = command.pixelShaderDefines_.Trimmed();
        if (vsDefines.Length() || psDefines.Length())
        {
            queue.hasExtraDefines_ = true;
            queue.vsExtraDefines_ = vsDefines;
            queue.psExtraDefines_ = psDefines;
            queue.vsExtraDefinesHash_ = StringHash(vsDefines);
            queue.psExtraDefinesHash_ = StringHash(psDefines);
        }
        else
            queue.hasExtraDefines_ = false;
    }

    void View::AddBatchToQueue(BatchQueue& queue, Batch& batch, Technique* tech, bool allowInstancing, bool allowShadows)
    {
        if (!batch.material_)
            batch.material_ = renderer_->GetDefaultMaterial();

        // Convert to instanced if possible
        if (allowInstancing && batch.geometryType_ == GEOM_STATIC && batch.geometry_->GetIndexBuffer())
            batch.geometryType_ = GEOM_INSTANCED;

        if (batch.geometryType_ == GEOM_INSTANCED)
        {
            BatchGroupKey key(batch);

            HashMap<BatchGroupKey, BatchGroup>::Iterator i = queue.batchGroups_.Find(key);
            if (i == queue.batchGroups_.End())
            {
                // Create a new group based on the batch
                // In case the group remains below the instancing limit, do not enable instancing shaders yet
                BatchGroup newGroup(batch);
                newGroup.geometryType_ = GEOM_STATIC;
                renderer_->SetBatchShaders(newGroup, tech, allowShadows, queue);
                newGroup.CalculateSortKey();
                i = queue.batchGroups_.Insert(MakePair(key, newGroup));
            }

            int oldSize = i->second_.instances_.Size();
            i->second_.AddTransforms(batch);
            // Convert to using instancing shaders when the instancing limit is reached
            if (oldSize < minInstances_ && (int)i->second_.instances_.Size() >= minInstances_)
            {
                i->second_.geometryType_ = GEOM_INSTANCED;
                renderer_->SetBatchShaders(i->second_, tech, allowShadows, queue);
                i->second_.CalculateSortKey();
            }
        }
        else
        {
            renderer_->SetBatchShaders(batch, tech, allowShadows, queue);
            batch.CalculateSortKey();

            // If batch is static with multiple world transforms and cannot instance, we must push copies of the batch individually
            if (batch.geometryType_ == GEOM_STATIC && batch.numWorldTransforms_ > 1)
            {
                unsigned numTransforms = batch.numWorldTransforms_;
                batch.numWorldTransforms_ = 1;
                for (unsigned i = 0; i < numTransforms; ++i)
                {
                    // Move the transform pointer to generate copies of the batch which only refer to 1 world transform
                    queue.batches_.Push(batch);
                    ++batch.worldTransform_;
                }
            }
            else
                queue.batches_.Push(batch);
        }
    }

    void View::PrepareInstancingBuffer()
    {
        // Prepare instancing buffer from the source view
        /// \todo If rendering the same view several times back-to-back, would not need to refill the buffer
        if (sourceView_)
        {
            sourceView_->PrepareInstancingBuffer();
            return;
        }

        unsigned totalInstances = 0;

        for (HashMap<unsigned, BatchQueue>::Iterator i = batchQueues_.Begin(); i != batchQueues_.End(); ++i)
            totalInstances += i->second_.GetNumInstances();

        for (Vector<LightBatchQueue>::Iterator i = lightQueues_.Begin(); i != lightQueues_.End(); ++i)
        {
            for (unsigned j = 0; j < i->shadowSplits_.Size(); ++j)
                totalInstances += i->shadowSplits_[j].shadowBatches_.GetNumInstances();
            totalInstances += i->litBaseBatches_.GetNumInstances();
            totalInstances += i->litBatches_.GetNumInstances();
        }

        if (!totalInstances || !renderer_->ResizeInstancingBuffer(totalInstances))
            return;

        VertexBuffer* instancingBuffer = renderer_->GetInstancingBuffer();
        unsigned freeIndex = 0;
        void* dest = instancingBuffer->Lock(0, totalInstances, true);
        if (!dest)
            return;

        const unsigned stride = instancingBuffer->GetVertexSize();
        for (HashMap<unsigned, BatchQueue>::Iterator i = batchQueues_.Begin(); i != batchQueues_.End(); ++i)
            i->second_.SetInstancingData(dest, stride, freeIndex);

        for (Vector<LightBatchQueue>::Iterator i = lightQueues_.Begin(); i != lightQueues_.End(); ++i)
        {
            for (unsigned j = 0; j < i->shadowSplits_.Size(); ++j)
                i->shadowSplits_[j].shadowBatches_.SetInstancingData(dest, stride, freeIndex);
            i->litBaseBatches_.SetInstancingData(dest, stride, freeIndex);
            i->litBatches_.SetInstancingData(dest, stride, freeIndex);
        }

        instancingBuffer->Unlock();
    }

    void View::SetupLightVolumeBatch(Batch& batch)
    {
        Light* light = batch.lightQueue_->light_;
        LightType type = light->GetLightType();
        Vector3 cameraPos = camera_->GetNode()->GetWorldPosition();
        float lightDist;

        graphics_->SetBlendMode(light->IsNegative() ? BLEND_SUBTRACT : BLEND_ADD);
        graphics_->SetDepthBias(0.0f, 0.0f);
        graphics_->SetDepthWrite(false);
        graphics_->SetFillMode(FILL_SOLID);
        graphics_->SetLineAntiAlias(false);
        graphics_->SetClipPlane(false);

        if (type != LIGHT_DIRECTIONAL)
        {
            if (type == LIGHT_POINT)
                lightDist = Sphere(light->GetNode()->GetWorldPosition(), light->GetRange() * 1.25f).Distance(cameraPos);
            else
                lightDist = light->GetFrustum().Distance(cameraPos);

            // Draw front faces if not inside light volume
            if (lightDist < camera_->GetNearClip() * 2.0f)
            {
                renderer_->SetCullMode(CULL_CW, camera_);
                graphics_->SetDepthTest(CMP_GREATER);
            }
            else
            {
                renderer_->SetCullMode(CULL_CCW, camera_);
                graphics_->SetDepthTest(CMP_LESSEQUAL);
            }
        }
        else
        {
            // In case the same camera is used for multiple views with differing aspect ratios (not recommended)
            // refresh the directional light's model transform before rendering
            light->GetVolumeTransform(camera_);
            graphics_->SetCullMode(CULL_NONE);
            graphics_->SetDepthTest(CMP_ALWAYS);
        }

        graphics_->SetScissorTest(false);
        if (!noStencil_)
            graphics_->SetStencilTest(true, CMP_NOTEQUAL, OP_KEEP, OP_KEEP, OP_KEEP, 0, light->GetLightMask());
        else
            graphics_->SetStencilTest(false);
    }

    bool View::NeedRenderShadowMap(const LightBatchQueue& queue)
    {
        // Must have a shadow map, and either forward or deferred lit batches
        return queue.shadowMap_ && (!queue.litBatches_.IsEmpty() || !queue.litBaseBatches_.IsEmpty() || !queue.volumeBatches_.Empty());
    }

    void View::RenderShadowMap(const LightBatchQueue &queue)
    {
        Texture2D* shadowMap = queue.shadowMap_;
        graphics_->SetTexture(TU_SHADOWMAP, nullptr);

        graphics_->SetFillMode(FILL_SOLID);
        graphics_->SetClipPlane(false);
        graphics_->SetStencilTest(false);

        // Set shadow depth bias
        BiasParameters parameters = queue.light_->GetShadowBias();

        // The shadow map is a depth stencil texture
        if (shadowMap->GetUsage() == TEXTURE_DEPTHSTENCIL)
        {
            graphics_->SetColorWrite(false);
            graphics_->SetDepthStencil(shadowMap);
            graphics_->SetRenderTarget(0, shadowMap->GetRenderSurface()->GetLinkedRenderTarget());
            // Disable other render targets
            for (unsigned i = 1; i < MAX_RENDERTARGETS; ++i)
                graphics_->SetRenderTarget(i, (RenderSurface*) nullptr);
            graphics_->SetViewport(IntRect(0, 0, shadowMap->GetWidth(), shadowMap->GetHeight()));
            graphics_->Clear(CLEAR_DEPTH);
        }
        else // if the shadow map is a color rendertarget
        {
            graphics_->SetColorWrite(true);
            graphics_->SetRenderTarget(0, shadowMap);
            // Disable other render targets
            for (unsigned i = 1; i < MAX_RENDERTARGETS; ++i)
                graphics_->SetRenderTarget(i, (RenderSurface*) nullptr);
            graphics_->SetDepthStencil(renderer_->GetDepthStencil(shadowMap->GetWidth(), shadowMap->GetHeight(),
                                                                  shadowMap->GetMultiSample(), shadowMap->GetAutoResolve()));
            graphics_->SetViewport(IntRect(0, 0, shadowMap->GetWidth(), shadowMap->GetHeight()));
            graphics_->Clear(CLEAR_DEPTH | CLEAR_COLOR, Color::WHITE);

            parameters = BiasParameters(0.0f, 0.0f);
        }

        // Render each of the splits
        for (unsigned i = 0; i < queue.shadowSplits_.Size(); ++i)
        {
            const ShadowBatchQueue &shadowQueue = queue.shadowSplits_[i];

            float multiplier = 1.0f;
            // For directional light cascade splits, adjust depth bias according to the far clip ratio of the splits
            if (i > 0 && queue.light_->GetLightType() == LIGHT_DIRECTIONAL) {
                multiplier =
                        Max(shadowQueue.shadowCamera_->GetFarClip() /
                            queue.shadowSplits_[0].shadowCamera_->GetFarClip(), 1.0f);
                multiplier = 1.0f + (multiplier - 1.0f) * queue.light_->GetShadowCascade().biasAutoAdjust_;
                // Quantize multiplier to prevent creation of too many rasterizer states on D3D11
                multiplier = (int) (multiplier * 10.0f) / 10.0f;
            }

            // Perform further modification of depth bias on OpenGL ES, as shadow calculations' precision is limited
            graphics_->SetDepthBias(multiplier * parameters.constantBias_, multiplier * parameters.slopeScaledBias_);

            if (!shadowQueue.shadowBatches_.IsEmpty())
            {
                graphics_->SetViewport(shadowQueue.shadowViewport_);
                shadowQueue.shadowBatches_.Draw(this, shadowQueue.shadowCamera_, false, false, true);
            }
        }

        // Scale filter blur amount to shadow map viewport size so that different shadow map resolutions don't behave differently
        float blurScale = queue.shadowSplits_[0].shadowViewport_.Width() / 1024.0f;
        renderer_->ApplyShadowMapFilter(this, shadowMap, blurScale);

        // reset some parameters
        graphics_->SetColorWrite(true);
        graphics_->SetDepthBias(0.0f, 0.0f);
    }

    RenderSurface* View::GetDepthStencil(RenderSurface* renderTarget)
    {
        // If using the backbuffer, return the backbuffer depth-stencil
        if (!renderTarget)
            return nullptr;
        // Then check for linked depth-stencil
        RenderSurface* depthStencil = renderTarget->GetLinkedDepthStencil();
        // Finally get one from Renderer
        if (!depthStencil)
            depthStencil = renderer_->GetDepthStencil(renderTarget->GetWidth(), renderTarget->GetHeight(),
                                                      renderTarget->GetMultiSample(), renderTarget->GetAutoResolve());
        return depthStencil;
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