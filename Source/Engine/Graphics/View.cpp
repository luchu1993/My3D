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
#include "Graphics/OcclusionBuffer.h"
#include "Resource/ResourceCache.h"
#include "Scene/Scene.h"


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

    void ProcessLightWork(const WorkItem* item, unsigned threadIndex)
    {
        auto* view = reinterpret_cast<View*>(item->aux_);
        auto* query = reinterpret_cast<LightQueryResult*>(item->start_);

        view->ProcessLight(*query, threadIndex);
    }

    void UpdateDrawableGeometriesWork(const WorkItem* item, unsigned threadIndex)
    {
        const FrameInfo& frame = *(reinterpret_cast<FrameInfo*>(item->aux_));
        auto** start = reinterpret_cast<Drawable**>(item->start_);
        auto** end = reinterpret_cast<Drawable**>(item->end_);

        while (start != end)
        {
            Drawable* drawable = *start++;
            // We may leave null pointer holes in the queue if a drawable is found out to require a main thread update
            if (drawable)
                drawable->UpdateGeometry(frame);
        }
    }

    void SortBatchQueueFrontToBackWork(const WorkItem* item, unsigned threadIndex)
    {
        auto* queue = reinterpret_cast<BatchQueue*>(item->start_);

        queue->SortFrontToBack();
    }

    void SortBatchQueueBackToFrontWork(const WorkItem* item, unsigned threadIndex)
    {
        auto* queue = reinterpret_cast<BatchQueue*>(item->start_);

        queue->SortBackToFront();
    }

    void SortLightQueueWork(const WorkItem* item, unsigned threadIndex)
    {
        auto* start = reinterpret_cast<LightBatchQueue*>(item->start_);
        start->litBaseBatches_.SortFrontToBack();
        start->litBatches_.SortFrontToBack();
    }

    void SortShadowQueueWork(const WorkItem* item, unsigned threadIndex)
    {
        auto* start = reinterpret_cast<LightBatchQueue*>(item->start_);
        for (unsigned i = 0; i < start->shadowSplits_.Size(); ++i)
            start->shadowSplits_[i].shadowBatches_.SortFrontToBack();
    }

    StringHash ParseTextureTypeXml(ResourceCache* cache, const String& filename);

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

    void View::GetBatches()
    {
        if (!octree_ || !cullCamera_)
            return;

        nonThreadedGeometries_.Clear();
        threadedGeometries_.Clear();

        ProcessLights();
        GetLightBatches();
        GetBaseBatches();
    }

    void View::ProcessLights()
    {
        // Process lit geometries and shadow casters for each light
        auto* queue = GetSubsystem<WorkQueue>();
        lightQueryResults_.Resize(lights_.Size());

        for (unsigned i = 0; i < lightQueryResults_.Size(); ++i)
        {
            SharedPtr<WorkItem> item = queue->GetFreeItem();
            item->priority_ = M_MAX_UNSIGNED;
            item->workFunction_ = ProcessLightWork;
            item->aux_ = this;

            LightQueryResult& query = lightQueryResults_[i];
            query.light_ = lights_[i];

            item->start_ = &query;
            queue->AddWorkItem(item);
        }

        // Ensure all lights have been processed before proceeding
        queue->Complete(M_MAX_UNSIGNED);
    }

    void View::GetLightBatches()
    {
        BatchQueue* alphaQueue = batchQueues_.Contains(alphaPassIndex_) ? &batchQueues_[alphaPassIndex_] : nullptr;

        // Build light queue and lit batches
        {
            // Preallocate light queues: per-pixel lights which have lit geometries
            unsigned numLightQueues = 0;
            unsigned usedLightQueues = 0;

            for (auto& query : lightQueryResults_)
            {
                if (!query.light_->GetPerVertex() && query.litGeometries_.Size())
                    ++numLightQueues;
            }

            lightQueues_.Resize(numLightQueues);
            maxLightsDrawables_.Clear();
            unsigned maxSortedInstances = renderer_->GetMaxSortedInstances();

            for (auto& query : lightQueryResults_)
            {
                // If light has no affected geometries, no need ot process further.
                if (query.litGeometries_.Empty())
                    continue;

                Light* light = query.light_;

                // Per-pixel light
                if (!light->GetPerVertex())
                {
                    unsigned shadowSplits = query.numSplits_;

                    // Initialize light queue and store it to the light so that it can be found later.
                    LightBatchQueue& lightQueue = lightQueues_[usedLightQueues++];
                    light->SetLightQueue(&lightQueue);
                    lightQueue.light_ = light;
                    lightQueue.negative_ = light->IsNegative();
                    lightQueue.shadowMap_ = nullptr;
                    lightQueue.litBaseBatches_.Clear(maxSortedInstances);
                    lightQueue.litBatches_.Clear(maxSortedInstances);

                    if (forwardLightsCommand_)
                    {
                        SetQueueShaderDefines(lightQueue.litBaseBatches_, *forwardLightsCommand_);
                        SetQueueShaderDefines(lightQueue.litBatches_, *forwardLightsCommand_);
                    }
                    else
                    {
                        lightQueue.litBaseBatches_.hasExtraDefines_ = false;
                        lightQueue.litBatches_.hasExtraDefines_ = false;
                    }
                    lightQueue.volumeBatches_.Clear();

                    // Allocate shadow map now
                    if (shadowSplits > 0)
                    {
                        lightQueue.shadowMap_ = renderer_->GetShadowMap(light, cullCamera_, (unsigned) viewSize_.x_, (unsigned) viewSize_.y_);
                        // If did not manage to get a shadow map, convert the light to unshadowed
                        if (!lightQueue.shadowMap_)
                            shadowSplits = 0;
                    }

                    // Setup shadow batch queues
                    lightQueue.shadowSplits_.Resize(shadowSplits);
                    for (unsigned  j = 0; j < shadowSplits; ++j)
                    {
                        ShadowBatchQueue& shadowQueue = lightQueue.shadowSplits_[j];
                        Camera* shadowCamera = query.shadowCameras_[j];
                        shadowQueue.shadowCamera_ = shadowCamera;
                        shadowQueue.nearSplit_ = query.shadowNearSplits_[j];
                        shadowQueue.farSplit_ = query.shadowFarSplits_[j];
                        shadowQueue.shadowBatches_.Clear(maxSortedInstances);

                        // Setup the shadow split viewport and finalize shadow camera parameters
                        shadowQueue.shadowViewport_ = GetShadowMapViewport(light, j, lightQueue.shadowMap_);
                        FinalizeShadowCamera(shadowCamera, light, shadowQueue.shadowViewport_, query.shadowCasterBox_[j]);

                        // Loop through shadow casters
                        for (auto k = query.shadowCasters_.Begin() + query.shadowCasterBegin_[j];
                            k < query.shadowCasters_.Begin() + query.shadowCasterEnd_[j]; ++k)
                        {
                            Drawable* drawable = *k;
                            // If drawable is not in actual view frustum, mark it in view here and check its geometry update type
                            if (!drawable->IsInView(frame_, true))
                            {
                                drawable->MarkInView(frame_.frameNumber_);
                                UpdateGeometryType type = drawable->GetUpdateGeometryType();
                                if (type == UPDATE_MAIN_THREAD)
                                    nonThreadedGeometries_.Push(drawable);
                                else
                                    threadedGeometries_.Push(drawable);
                            }

                            const Vector<SourceBatch>& batches = drawable->GetBatches();

                            for (auto const& srcBatch : batches)
                            {
                                Technique* tech = GetTechnique(drawable, srcBatch.material_);
                                if (!srcBatch.geometry_ || !srcBatch.numWorldTransforms_ || !tech)
                                    continue;

                                Pass* pass = tech->GetPass(Technique::shadowPassIndex);
                                // Skip if material has no shadow pass
                                if (!pass)
                                    continue;

                                Batch destBatch(srcBatch);
                                destBatch.pass_ = pass;
                                destBatch.zone_ = nullptr;

                                AddBatchToQueue(shadowQueue.shadowBatches_, destBatch, tech);
                            }
                        }
                    }

                    // Process lit geometries
                    for (auto drawable : query.litGeometries_)
                    {
                        drawable->AddLight(light);

                        // If drawable limits maximum lights, only record the light, and check maximum count / build batches later
                        if (!drawable->GetMaxLights())
                            GetLitBatches(drawable, lightQueue, alphaQueue);
                        else
                            maxLightsDrawables_.Insert(drawable);
                    }

                    // In deferred modes, store the light volume batch now. Since light mask 8 lowest bits are output ot the stencil,
                    // lights that have all zeroes in the low 8 bits can be skipped; they would not affect geometry anyway.
                    if (deferred_ && (light->GetLightMask() & 0xffu) != 0)
                    {
                        Batch volumeBatch;
                        volumeBatch.geometry_ = renderer_->GetLightGeometry(light);
                        volumeBatch.geometryType_ = GEOM_STATIC;
                        volumeBatch.worldTransform_ = &light->GetVolumeTransform(cullCamera_);
                        volumeBatch.numWorldTransforms_ = 1;
                        volumeBatch.lightQueue_ = &lightQueue;
                        volumeBatch.distance_ = light->GetDistance();
                        volumeBatch.material_ = nullptr;
                        volumeBatch.pass_ = nullptr;
                        volumeBatch.zone_ = nullptr;
                        renderer_->SetLightVolumeBatchShaders(volumeBatch, cullCamera_, lightVolumeCommand_->vertexShaderName_,
                            lightVolumeCommand_->pixelShaderName_, lightVolumeCommand_->vertexShaderDefines_,
                            lightVolumeCommand_->pixelShaderDefines_);
                        lightQueue.volumeBatches_.Push(volumeBatch);
                    }
                }
                // Per-vertex light
                else
                {
                    // Add the vertex light to lit drawables. It will be processed later during base pass batch generation.
                    for (Drawable* drawable : query.litGeometries_)
                        drawable->AddVertexLight(light);
                }
            }
        }

        // Process drawables with limited per-pixel light count
        if (maxLightsDrawables_.Size())
        {
            for (auto drawable : maxLightsDrawables_)
            {
                drawable->LimitLights();
                const PODVector<Light*>& lights = drawable->GetLights();

                for (auto light : lights)
                {
                    // Find the correct light queue again
                    LightBatchQueue* queue = light->GetLightQueue();
                    if (queue)
                        GetLitBatches(drawable, *queue, alphaQueue);
                }
            }
        }
    }

    void View::GetBaseBatches()
    {
        for (auto drawable : geometries_)
        {
            UpdateGeometryType type = drawable->GetUpdateGeometryType();
            if (type == UPDATE_MAIN_THREAD)
                nonThreadedGeometries_.Push(drawable);
            else
                threadedGeometries_.Push(drawable);


            const Vector<SourceBatch>& batches = drawable->GetBatches();
            bool vertexLightsProcessed = false;

            for (unsigned j = 0; j < batches.Size(); ++j)
            {
                const SourceBatch& srcBatch = batches[j];

                // Check here if the material refers to a rendertarget texture with camera(s) attached
                // Only check this for backbuffer views (null rendertarget)
                if (srcBatch.material_ && srcBatch.material_->GetAuxViewFrameNumber() != frame_.frameNumber_ && !renderer_)
                    CheckMaterialForAuxView(srcBatch.material_);

                Technique* tech = GetTechnique(drawable, srcBatch.material_);
                if (!srcBatch.geometry_ || !srcBatch.numWorldTransforms_ || !tech)
                    continue;

                // Check each of the scene passes
                for (auto& info : scenePasses_)
                {
                    // Skip forward base pass if the corresponding litbase pass already exists
                    if (info.passIndex_ == basePassIndex_ && j < 32 && drawable->HasBasePass(j))
                        continue;

                    Pass* pass = tech->GetPass(info.passIndex_);
                    if (!pass)
                        continue;

                    Batch destBatch(srcBatch);
                    destBatch.pass_ = pass;
                    destBatch.zone_ = GetZone(drawable);
                    destBatch.isBase_ = true;
                    destBatch.lightMask_ = (unsigned char)GetLightMask(drawable);

                    if (info.vertexLights_)
                    {
                        const PODVector<Light*>& drawableVertexLights = drawable->GetVertexLights();
                        if (drawableVertexLights.Size() && !vertexLightsProcessed)
                        {
                            // Limit vertex lights. If this is a deferred opaque batch, remove converted per-pixel lights,
                            // as they will be rendered as light volumes in any case, and drawing them also as vertex lights
                            // would result in double lighting
                            drawable->LimitVertexLights(deferred_ && destBatch.pass_->GetBlendMode() == BLEND_REPLACE);
                            vertexLightsProcessed = true;
                        }

                        if (drawableVertexLights.Size())
                        {
                            // Find a vertex light queue. If not found, create new
                            unsigned long long hash = GetVertexLightQueueHash(drawableVertexLights);
                            HashMap<unsigned long long, LightBatchQueue>::Iterator i = vertexLightQueues_.Find(hash);
                            if (i == vertexLightQueues_.End())
                            {
                                i = vertexLightQueues_.Insert(MakePair(hash, LightBatchQueue()));
                                i->second_.light_ = nullptr;
                                i->second_.shadowMap_ = nullptr;
                                i->second_.vertexLights_ = drawableVertexLights;
                            }

                            destBatch.lightQueue_ = &(i->second_);
                        }
                    }
                    else
                        destBatch.lightQueue_ = nullptr;

                    bool allowInstancing = info.allowInstancing_;
                    if (allowInstancing && info.markToStencil_ && destBatch.lightMask_ != (destBatch.zone_->GetLightMask() & 0xffu))
                        allowInstancing = false;

                    AddBatchToQueue(*info.batchQueue_, destBatch, tech, allowInstancing);
                }
            }
        }
    }

    void View::UpdateGeometries()
    {
        // Update geometries in the source view if necessary (prepare order may differ from render order)
        if (sourceView_ && !sourceView_->geometriesUpdated_)
        {
            sourceView_->UpdateGeometries();
            return;
        }

        auto* queue = GetSubsystem<WorkQueue>();

        // Sort batches
        {
            for (unsigned i = 0; i < renderPath_->commands_.Size(); ++i)
            {
                const RenderPathCommand &command = renderPath_->commands_[i];
                if (!IsNecessary(command))
                    continue;

                if (command.type_ == CMD_SCENEPASS)
                {
                    SharedPtr<WorkItem> item = queue->GetFreeItem();
                    item->priority_ = M_MAX_UNSIGNED;
                    item->workFunction_ =
                            command.sortMode_ == SORT_FRONTTOBACK ? SortBatchQueueFrontToBackWork : SortBatchQueueBackToFrontWork;
                    item->start_ = &batchQueues_[command.passIndex_];
                    queue->AddWorkItem(item);
                }
            }

            for (Vector<LightBatchQueue>::Iterator i = lightQueues_.Begin(); i != lightQueues_.End(); ++i)
            {
                SharedPtr<WorkItem> lightItem = queue->GetFreeItem();
                lightItem->priority_ = M_MAX_UNSIGNED;
                lightItem->workFunction_ = SortLightQueueWork;
                lightItem->start_ = &(*i);
                queue->AddWorkItem(lightItem);

                if (i->shadowSplits_.Size())
                {
                    SharedPtr<WorkItem> shadowItem = queue->GetFreeItem();
                    shadowItem->priority_ = M_MAX_UNSIGNED;
                    shadowItem->workFunction_ = SortShadowQueueWork;
                    shadowItem->start_ = &(*i);
                    queue->AddWorkItem(shadowItem);
                }
            }
        }

        // Update geometries. Split into threaded and non-threaded updates.
        {
            if (threadedGeometries_.Size())
            {
                // In special cases (context loss, multi-view) a drawable may theoretically first have reported a threaded update, but will actually
                // require a main thread update. Check these cases first and move as applicable. The threaded work routine will tolerate the null
                // pointer holes that we leave to the threaded update queue.
                for (PODVector<Drawable*>::Iterator i = threadedGeometries_.Begin(); i != threadedGeometries_.End(); ++i)
                {
                    if ((*i)->GetUpdateGeometryType() == UPDATE_MAIN_THREAD)
                    {
                        nonThreadedGeometries_.Push(*i);
                        *i = nullptr;
                    }
                }

                int numWorkItems = queue->GetNumThreads() + 1; // Worker threads + main thread
                int drawablesPerItem = threadedGeometries_.Size() / numWorkItems;

                PODVector<Drawable*>::Iterator start = threadedGeometries_.Begin();
                for (int i = 0; i < numWorkItems; ++i)
                {
                    PODVector<Drawable*>::Iterator end = threadedGeometries_.End();
                    if (i < numWorkItems - 1 && end - start > drawablesPerItem)
                        end = start + drawablesPerItem;

                    SharedPtr<WorkItem> item = queue->GetFreeItem();
                    item->priority_ = M_MAX_UNSIGNED;
                    item->workFunction_ = UpdateDrawableGeometriesWork;
                    item->aux_ = const_cast<FrameInfo*>(&frame_);
                    item->start_ = &(*start);
                    item->end_ = &(*end);
                    queue->AddWorkItem(item);

                    start = end;
                }
            }

            // While the work queue is processed, update non-threaded geometries
            for (PODVector<Drawable*>::ConstIterator i = nonThreadedGeometries_.Begin(); i != nonThreadedGeometries_.End(); ++i)
                (*i)->UpdateGeometry(frame_);
        }

        // Finally ensure all threaded work has completed
        queue->Complete(M_MAX_UNSIGNED);
        geometriesUpdated_ = true;
    }

    void View::GetLitBatches(Drawable* drawable, LightBatchQueue& lightQueue, BatchQueue* alphaQueue)
    {
        Light* light = lightQueue.light_;
        Zone* zone = GetZone(drawable);
        const Vector<SourceBatch>& batches = drawable->GetBatches();

        bool allowLitBase =
                useLitBase_ && !lightQueue.negative_ && light == drawable->GetFirstLight() && drawable->GetVertexLights().Empty() &&
                !zone->GetAmbientGradient();

        for (unsigned i = 0; i < batches.Size(); ++i)
        {
            const SourceBatch& srcBatch = batches[i];

            Technique* tech = GetTechnique(drawable, srcBatch.material_);
            if (!srcBatch.geometry_ || !srcBatch.numWorldTransforms_ || !tech)
                continue;

            // Do not create pixel lit forward passes for materials that render into the G-buffer
            if (gBufferPassIndex_ != M_MAX_UNSIGNED && tech->HasPass(gBufferPassIndex_))
                continue;

            Batch destBatch(srcBatch);
            bool isLitAlpha = false;

            // Check for lit base pass. Because it uses the replace blend mode, it must be ensured to be the first light
            // Also vertex lighting or ambient gradient require the non-lit base pass, so skip in those cases
            if (i < 32 && allowLitBase)
            {
                destBatch.pass_ = tech->GetPass(litBasePassIndex_);
                if (destBatch.pass_)
                {
                    destBatch.isBase_ = true;
                    drawable->SetBasePass(i);
                }
                else
                    destBatch.pass_ = tech->GetPass(lightPassIndex_);
            }
            else
                destBatch.pass_ = tech->GetPass(lightPassIndex_);

            // If no lit pass, check for lit alpha
            if (!destBatch.pass_)
            {
                destBatch.pass_ = tech->GetPass(litAlphaPassIndex_);
                isLitAlpha = true;
            }

            // Skip if material does not receive light at all
            if (!destBatch.pass_)
                continue;

            destBatch.lightQueue_ = &lightQueue;
            destBatch.zone_ = zone;

            if (!isLitAlpha)
            {
                if (destBatch.isBase_)
                    AddBatchToQueue(lightQueue.litBaseBatches_, destBatch, tech);
                else
                    AddBatchToQueue(lightQueue.litBatches_, destBatch, tech);
            }
            else if (alphaQueue)
            {
                // Transparent batches can not be instanced, and shadows on transparencies can only be rendered if shadow maps are
                // not reused
                AddBatchToQueue(*alphaQueue, destBatch, tech, false, !renderer_->GetReuseShadowMaps());
            }
        }
    }

    void View::ExecuteRenderPathCommands()
    {
        View* actualView = sourceView_ ? sourceView_ : this;

        // If not reusing shadowmaps, render all of them first
        if (!renderer_->GetReuseShadowMaps() && renderer_->GetDrawShadows() && !actualView->lightQueues_.Empty())
        {
            for (Vector<LightBatchQueue>::Iterator i = actualView->lightQueues_.Begin(); i != actualView->lightQueues_.End(); ++i)
            {
                if (NeedRenderShadowMap(*i))
                    RenderShadowMap(*i);
            }
        }

        {
            // Set for safety in case of empty renderpath
            currentRenderTarget_ = substituteRenderTarget_ ? substituteRenderTarget_ : renderTarget_;
            currentViewportTexture_ = nullptr;
            passCommand_ = nullptr;

            bool viewportModified = false;
            bool isPingponging = false;
            usedResolve_ = false;

            unsigned lastCommandIndex = 0;
            for (unsigned i = 0; i < renderPath_->commands_.Size(); ++i)
            {
                RenderPathCommand& command = renderPath_->commands_[i];
                if (actualView->IsNecessary(command))
                    lastCommandIndex = i;
            }

            for (unsigned i = 0; i < renderPath_->commands_.Size(); ++i)
            {
                RenderPathCommand& command = renderPath_->commands_[i];
                if (!actualView->IsNecessary(command))
                    continue;

                bool viewportRead = actualView->CheckViewportRead(command);
                bool viewportWrite = actualView->CheckViewportWrite(command);
                bool beginPingpong = actualView->CheckPingpong(i);

                // Has the viewport been modified and will be read as a texture by the current command?
                if (viewportRead && viewportModified)
                {
                    // Start pingponging without a blit if already rendering to the substitute render target
                    if (currentRenderTarget_ && currentRenderTarget_ == substituteRenderTarget_ && beginPingpong)
                        isPingponging = true;

                    // If not using pingponging, simply resolve/copy to the first viewport texture
                    if (!isPingponging)
                    {
                        if (!currentRenderTarget_)
                        {
                            graphics_->ResolveToTexture(dynamic_cast<Texture2D*>(viewportTextures_[0]), viewRect_);
                            currentViewportTexture_ = viewportTextures_[0];
                            viewportModified = false;
                            usedResolve_ = true;
                        }
                        else
                        {
                            if (viewportWrite)
                            {
                                BlitFramebuffer(currentRenderTarget_->GetParentTexture(),
                                                GetRenderSurfaceFromTexture(viewportTextures_[0]), false);
                                currentViewportTexture_ = viewportTextures_[0];
                                viewportModified = false;
                            }
                            else
                            {
                                // If the current render target is already a texture, and we are not writing to it, can read that
                                // texture directly instead of blitting. However keep the viewport dirty flag in case a later command
                                // will do both read and write, and then we need to blit / resolve
                                currentViewportTexture_ = currentRenderTarget_->GetParentTexture();
                            }
                        }
                    }
                    else
                    {
                        // Swap the pingpong double buffer sides. Texture 0 will be read next
                        viewportTextures_[1] = viewportTextures_[0];
                        viewportTextures_[0] = currentRenderTarget_->GetParentTexture();
                        currentViewportTexture_ = viewportTextures_[0];
                        viewportModified = false;
                    }
                }

                if (beginPingpong)
                    isPingponging = true;

                // Determine viewport write target
                if (viewportWrite)
                {
                    if (isPingponging)
                    {
                        currentRenderTarget_ = GetRenderSurfaceFromTexture(viewportTextures_[1]);
                        // If the render path ends into a quad, it can be redirected to the final render target
                        // However, on OpenGL we can not reliably do this in case the final target is the backbuffer, and we want to
                        // render depth buffer sensitive debug geometry afterward (backbuffer and textures can not share depth)
#ifndef MY3D_OPENGL
                        if (i == lastCommandIndex && command.type_ == CMD_QUAD)
#else
                            if (i == lastCommandIndex && command.type_ == CMD_QUAD && renderTarget_)
#endif
                            currentRenderTarget_ = renderTarget_;
                    }
                    else
                        currentRenderTarget_ = substituteRenderTarget_ ? substituteRenderTarget_ : renderTarget_;
                }

                switch (command.type_)
                {
                    case CMD_CLEAR:
                    {
                        Color clearColor = command.clearColor_;
                        if (command.useFogColor_)
                            clearColor = actualView->farClipZone_->GetFogColor();

                        SetRenderTargets(command);
                        graphics_->Clear(command.clearFlags_, clearColor, command.clearDepth_, command.clearStencil_);
                    }
                    break;

                    case CMD_SCENEPASS:
                    {
                        BatchQueue& queue = actualView->batchQueues_[command.passIndex_];
                        if (!queue.IsEmpty())
                        {
                            SetRenderTargets(command);
                            bool allowDepthWrite = SetTextures(command);
                            graphics_->SetClipPlane(camera_->GetUseClipping(), camera_->GetClipPlane(), camera_->GetView(),
                                camera_->GetGPUProjection());

                            if (command.shaderParameters_.Size())
                            {
                                // If pass defines shader parameters, reset parameter sources now to ensure they all will be set
                                // (will be set after camera shader parameters)
                                graphics_->ClearParameterSources();
                                passCommand_ = &command;
                            }

                            queue.Draw(this, camera_, command.markToStencil_, false, allowDepthWrite);

                            passCommand_ = nullptr;
                        }
                    }
                    break;

                    case CMD_QUAD:
                    {
                        SetRenderTargets(command);
                        SetTextures(command);
                        RenderQuad(command);
                    }
                    break;

                    case CMD_FORWARDLIGHTS:
                    // Render shadow maps + opaque objects' additive lighting
                    if (!actualView->lightQueues_.Empty())
                    {
                        SetRenderTargets(command);

                        for (auto& lightQueue : actualView->lightQueues_)
                        {
                            // If reusing shadowmaps, render each of them before the lit batches
                            if (renderer_->GetReuseShadowMaps() && NeedRenderShadowMap(lightQueue))
                            {
                                RenderShadowMap(lightQueue);
                                SetRenderTargets(command);
                            }

                            bool allowDepthWrite = SetTextures(command);
                            graphics_->SetClipPlane(camera_->GetUseClipping(), camera_->GetClipPlane(), camera_->GetView(),
                                                    camera_->GetGPUProjection());

                            if (command.shaderParameters_.Size())
                            {
                                graphics_->ClearParameterSources();
                                passCommand_ = &command;
                            }

                            // Draw base (replace blend) batches first
                            lightQueue.litBaseBatches_.Draw(this, camera_, false, false, allowDepthWrite);

                            // Then, if there are additive passes, optimize the light and draw them
                            if (!lightQueue.litBatches_.IsEmpty())
                            {
                                renderer_->OptimizeLightByScissor(lightQueue.light_, camera_);
                                if (!noStencil_)
                                    renderer_->OptimizeLightByStencil(lightQueue.light_, camera_);
                                lightQueue.litBatches_.Draw(this, camera_, false, true, allowDepthWrite);
                            }

                            passCommand_ = nullptr;
                        }

                        graphics_->SetScissorTest(false);
                        graphics_->SetStencilTest(false);
                    }
                    break;
                }
            }
        }
    }

    void View::SetRenderTargets(RenderPathCommand &command)
    {
        unsigned index = 0;
        bool useColorWrite = true;
        bool useCustomDepth = false;
        bool useViewportOutput = false;

        while (index < command.outputs_.Size())
        {
            if (!command.outputs_[index].first_.Compare("viewport", false))
            {
                graphics_->SetRenderTarget(index, currentRenderTarget_);
                useViewportOutput = true;
            }
            else
            {
                Texture* texture = FindNamedTexture(command.outputs_[index].first_, true, false);

                // Check for depth only rendering (by specifying a depth texture as the sole output)
                if (!index && command.outputs_.Size() == 1 && (texture->GetFormat() == Graphics::GetReadableDepthFormat() ||
                                                               texture->GetFormat() == Graphics::GetDepthStencilFormat()))
                {
                    useColorWrite = false;
                    useCustomDepth = true;

                    graphics_->SetRenderTarget(0, GetRenderSurfaceFromTexture(depthOnlyDummyTexture_));
                    graphics_->SetDepthStencil(GetRenderSurfaceFromTexture(texture));
                }
                else
                    graphics_->SetRenderTarget(index, GetRenderSurfaceFromTexture(texture, command.outputs_[index].second_));
            }

            ++index;
        }

        while (index < MAX_RENDERTARGETS)
        {
            graphics_->SetRenderTarget(index, (RenderSurface*) nullptr);
            ++index;
        }

        if (command.depthStencilName_.Length())
        {
            Texture* depthTexture = FindNamedTexture(command.depthStencilName_, true, false);
            if (depthTexture)
            {
                useCustomDepth = true;
                lastCustomDepthSurface_ = GetRenderSurfaceFromTexture(depthTexture);
                graphics_->SetDepthStencil(lastCustomDepthSurface_);
            }
        }

        // When rendering to the final destination rendertarget, use the actual viewport.
        // Otherwise texture rendertargets should use their full size as the viewport
        IntVector2 rtSizeNow = graphics_->GetRenderTargetDimensions();
        IntRect viewport = (useViewportOutput && currentRenderTarget_ == renderTarget_) ?
                viewRect_ : IntRect(0, 0, rtSizeNow.x_, rtSizeNow.y_);

        if (!useCustomDepth)
            graphics_->SetDepthStencil(GetDepthStencil(graphics_->GetRenderTarget(0)));
        graphics_->SetViewport(viewport);
        graphics_->SetColorWrite(useColorWrite);
    }

    bool View::SetTextures(RenderPathCommand &command)
    {
        bool allowDepthWrite = true;

        for (unsigned i = 0; i < MAX_TEXTURE_UNITS; ++i)
        {
            if (command.textureNames_[i].Empty())
                continue;

            // Bind the rendered output
            if (!command.textureNames_[i].Compare("viewport", false))
            {
                graphics_->SetTexture(i, currentViewportTexture_);
                continue;
            }

            Texture* texture = FindNamedTexture(command.textureNames_[i], false, i == TU_VOLUMEMAP);

            if (texture)
            {
                graphics_->SetTexture(i, texture);
                // Check if the current depth stencil is being sampled
                if (graphics_->GetDepthStencil() && texture == graphics_->GetDepthStencil()->GetParentTexture())
                    allowDepthWrite = false;
            }
            else
            {
                // If requesting a texture fails, clear the texture name to prevent redundant attempts
                command.textureNames_[i] = String::EMPTY;
            }
        }

        return allowDepthWrite;
    }

    void View::RenderQuad(RenderPathCommand &command)
    {
        if (command.vertexShaderName_.Empty() || command.pixelShaderName_.Empty())
            return;

        // If shader can not be found, clear it from the command to prevent redundant attempts
        ShaderVariation* vs = graphics_->GetShader(VS, command.vertexShaderName_, command.vertexShaderDefines_);
        if (!vs)
            command.vertexShaderName_ = String::EMPTY;
        ShaderVariation* ps = graphics_->GetShader(PS, command.pixelShaderName_, command.pixelShaderDefines_);
        if (!ps)
            command.pixelShaderName_ = String::EMPTY;

        // Set shaders & shader parameters and textures
        graphics_->SetShaders(vs, ps);

        SetGlobalShaderParameters();
        SetCameraShaderParameters(camera_);

        // During renderpath commands the G-Buffer or viewport texture is assumed to always be viewport-sized
        IntRect viewport = graphics_->GetViewport();
        IntVector2 viewSize = IntVector2(viewport.Width(), viewport.Height());
        SetGBufferShaderParameters(viewSize, IntRect(0, 0, viewSize.x_, viewSize.y_));

        // Set per-rendertarget inverse size / offset shader parameters as necessary
        for (unsigned i = 0; i < renderPath_->renderTargets_.Size(); ++i)
        {
            const RenderTargetInfo& rtInfo = renderPath_->renderTargets_[i];
            if (!rtInfo.enabled_)
                continue;

            StringHash nameHash(rtInfo.name_);
            if (!renderTargets_.Contains(nameHash))
                continue;

            String invSizeName = rtInfo.name_ + "InvSize";
            String offsetsName = rtInfo.name_ + "Offsets";
            auto width = (float)renderTargets_[nameHash]->GetWidth();
            auto height = (float)renderTargets_[nameHash]->GetHeight();

            const Vector2& pixelUVOffset = Graphics::GetPixelUVOffset();
            graphics_->SetShaderParameter(invSizeName, Vector2(1.0f / width, 1.0f / height));
            graphics_->SetShaderParameter(offsetsName, Vector2(pixelUVOffset.x_ / width, pixelUVOffset.y_ / height));
        }

        // Set command's shader parameters last to allow them to override any of the above
        SetCommandShaderParameters(command);

        graphics_->SetBlendMode(command.blendMode_);
        graphics_->SetDepthTest(CMP_ALWAYS);
        graphics_->SetDepthWrite(false);
        graphics_->SetFillMode(FILL_SOLID);
        graphics_->SetLineAntiAlias(false);
        graphics_->SetClipPlane(false);
        graphics_->SetScissorTest(false);
        graphics_->SetStencilTest(false);

        DrawFullscreenQuad(false);
    }

    bool View::IsNecessary(const RenderPathCommand& command)
    {
        return command.enabled_ && command.outputs_.Size() &&
               (command.type_ != CMD_SCENEPASS || !batchQueues_[command.passIndex_].IsEmpty());
    }

    bool View::CheckViewportRead(const RenderPathCommand& command)
    {
        for (const auto& textureName : command.textureNames_)
        {
            if (!textureName.Empty() && !textureName.Compare("viewport", false))
                return true;
        }

        return false;
    }

    bool View::CheckViewportWrite(const RenderPathCommand& command)
    {
        for (unsigned i = 0; i < command.outputs_.Size(); ++i)
        {
            if (!command.outputs_[i].first_.Compare("viewport", false))
                return true;
        }

        return false;
    }

    bool View::CheckPingpong(unsigned int index)
    {
        // Current command must be a viewport-reading & writing quad to begin the pingpong chain
        RenderPathCommand& current = renderPath_->commands_[index];
        if (current.type_ != CMD_QUAD || !CheckViewportRead(current) || !CheckViewportWrite(current))
            return false;

        // If there are commands other than quads that target the viewport, we must keep rendering to the final target and resolving
        // to a viewport texture when necessary instead of pingponging, as a scene pass is not guaranteed to fill the entire viewport
        for (unsigned i = index + 1; i < renderPath_->commands_.Size(); ++i)
        {
            RenderPathCommand& command = renderPath_->commands_[i];
            if (!IsNecessary(command))
                continue;
            if (CheckViewportWrite(command))
            {
                if (command.type_ != CMD_QUAD)
                    return false;
            }
        }

        return true;
    }

    void View::ProcessLight(LightQueryResult &query, unsigned int threadIndex)
    {
        Light* light = query.light_;
        LightType type = light->GetLightType();
        unsigned lightMask = light->GetLightMask();
        const Frustum& frustum = cullCamera_->GetFrustum();

        // Check if light should be shadowed
        bool isShadowed = drawShadows_ && light->GetCastShadows() && !light->GetPerVertex() && light->GetShadowIntensity() < 1.0f;
        // If shadow distance non-zero, check it
        if (isShadowed && light->GetShadowDistance() > 0.0f && light->GetDistance() > light->GetShadowDistance())
            isShadowed = false;

        // Get lit geometries. They must match the light mask and be inside the main camera frustum to be considered
        PODVector<Drawable*>& tempDrawables = tempDrawables_[threadIndex];
        query.litGeometries_.Clear();

        switch (type)
        {
        case LIGHT_DIRECTIONAL:
            for (unsigned  i = 0; i < geometries_.Size(); ++i)
            {
                if (GetLightMask(geometries_[i]) & lightMask)
                    query.litGeometries_.Push(geometries_[i]);
            }
            break;

        case LIGHT_SPOT:
            {
                FrustumOctreeQuery octreeQuery(tempDrawables, light->GetFrustum(), DRAWABLE_GEOMETRY,
                    cullCamera_->GetViewMask());
                octree_->GetDrawables(octreeQuery);

                for (unsigned i = 0; i < tempDrawables.Size(); ++i)
                {
                    if (tempDrawables[i]->IsInView(frame_) && (GetLightMask(tempDrawables[i]) & lightMask))
                        query.litGeometries_.Push(tempDrawables[i]);
                }
            }
            break;

        case LIGHT_POINT:
            {
                SphereOctreeQuery octreeQuery(tempDrawables, Sphere(light->GetNode()->GetWorldPosition(), light->GetRange()),
                    DRAWABLE_GEOMETRY, cullCamera_->GetViewMask());
                octree_->GetDrawables(octreeQuery);
                for (unsigned i = 0; i < tempDrawables.Size(); ++i)
                {
                    if (tempDrawables[i]->IsInView(frame_) && (GetLightMask(tempDrawables[i]) & lightMask))
                        query.litGeometries_.Push(tempDrawables[i]);
                }
            }
            break;
        }

        // If no lit geometries or not shadowed, no need to process shadow cameras
        if (query.litGeometries_.Empty() || !isShadowed)
        {
            query.numSplits_ = 0;
            return;
        }

        // Determine number of shadow cameras and setup their initial positions
        SetupShadowCameras(query);

        // Process each split for shadow casters
        query.shadowCasters_.Clear();
        for (unsigned  i = 0; i < query.numSplits_; ++i)
        {
            Camera* shadowCamera = query.shadowCameras_[i];
            const Frustum& shadowCameraFrustum = shadowCamera->GetFrustum();
            query.shadowCasterBegin_[i] = query.shadowCasterEnd_[i] = query.shadowCasters_.Size();

            // For point light check that the face is visible: if not, can skip the split
            if (type == LIGHT_POINT && frustum.IsInsideFast(BoundingBox(shadowCameraFrustum)) == OUTSIDE)
                continue;

            // For directional light check that the split is inside the visible scene: if not, can skip the split
            if (type == LIGHT_DIRECTIONAL)
            {
                if (minZ_ > query.shadowFarSplits_[i])
                    continue;
                if (maxZ_ < query.shadowNearSplits_[i])
                    continue;

                // Reuse lit geometry query for all except directional lights
                ShadowCasterOctreeQuery query(tempDrawables, shadowCameraFrustum, DRAWABLE_GEOMETRY, cullCamera_->GetViewMask());
                octree_->GetDrawables(query);
            }

            // Check which shadow casters actually contribute to the shadowing
            ProcessShadowCasters(query, tempDrawables, i);
        }

        // If no shadow casters, the light can be rendered unshadowed. At this point we have not allocated a shadow map yet, so the
        // only cost has been the shadow camera setup & queries
        if (query.shadowCasters_.Empty())
            query.numSplits_ = 0;
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

    void View::FindZone(Drawable* drawable)
    {
        Vector3 center = drawable->GetWorldBoundingBox().Center();
        int bestPriority = M_MIN_INT;
        Zone* newZone = nullptr;

        // If bounding box center is in view, the zone assignment is conclusive also for next frames. Otherwise it is temporary
        // (possibly incorrect) and must be re-evaluated on the next frame
        bool temporary = !cullCamera_->GetFrustum().IsInside(center);

        // First check if the current zone remains a conclusive result
        Zone* lastZone = drawable->GetZone();

        if (lastZone && (lastZone->GetViewMask() & cullCamera_->GetViewMask()) && lastZone->GetPriority() >= highestZonePriority_ &&
            (drawable->GetZoneMask() & lastZone->GetZoneMask()) && lastZone->IsInside(center))
            newZone = lastZone;
        else
        {
            for (PODVector<Zone*>::Iterator i = zones_.Begin(); i != zones_.End(); ++i)
            {
                Zone* zone = *i;
                int priority = zone->GetPriority();
                if (priority > bestPriority && (drawable->GetZoneMask() & zone->GetZoneMask()) && zone->IsInside(center))
                {
                    newZone = zone;
                    bestPriority = priority;
                }
            }
        }

        drawable->SetZone(newZone, temporary);
    }

    Technique* View::GetTechnique(Drawable* drawable, Material* material)
    {
        if (!material)
            return renderer_->GetDefaultMaterial()->GetTechniques()[0].technique_;

        const Vector<TechniqueEntry>& techniques = material->GetTechniques();
        // If only one technique, no choice
        if (techniques.Size() == 1)
            return techniques[0].technique_;
        else
        {
            float lodDistance = drawable->GetLodDistance();

            // Check for suitable technique. Techniques should be ordered like this:
            // Most distant & highest quality
            // Most distant & lowest quality
            // Second most distant & highest quality
            // ...
            for (unsigned i = 0; i < techniques.Size(); ++i)
            {
                const TechniqueEntry& entry = techniques[i];
                Technique* tech = entry.technique_;

                if (!tech || materialQuality_ < entry.qualityLevel_)
                    continue;

                if (lodDistance >= entry.lodDistance_)
                    return tech;
            }

            // If no suitable technique found, fallback to the last
            return techniques.Size() ? techniques.Back().technique_ : nullptr;
        }
    }
}