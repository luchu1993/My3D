//
// Created by luchu on 2022/2/3.
//

#include "Core/CoreEvents.h"
#include "Graphics/Camera.h"
#include "Graphics/Geometry.h"
#include "Graphics/Graphics.h"
#include "Graphics/GraphicsEvents.h"
#include "Graphics/GraphicsImpl.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/Material.h"
#include "Graphics/Octree.h"
#include "Graphics/Renderer.h"
#include "Graphics/RenderPath.h"
#include "Graphics/ShaderVariation.h"
#include "Graphics/Technique.h"
#include "Graphics/Texture2D.h"
#include "Graphics/TextureCube.h"
#include "Graphics/Zone.h"
#include "Graphics/Light.h"
#include "Graphics/View.h"
#include "Resource/ResourceCache.h"
#include "Resource/XMLFile.h"
#include "Scene/Scene.h"
#include "IO/Log.h"


namespace My3D
{
    static const float dirLightVertexData[] =
    {
        -1, 1, 0,
        1, 1, 0,
        1, -1, 0,
        -1, -1, 0,
    };

    static const unsigned short dirLightIndexData[] =
    {
        0, 1, 2,
        2, 3, 0,
    };

    static const float pointLightVertexData[] =
    {
        -0.423169f, -1.000000f, 0.423169f,
        -0.423169f, -1.000000f, -0.423169f,
        0.423169f, -1.000000f, -0.423169f,
        0.423169f, -1.000000f, 0.423169f,
        0.423169f, 1.000000f, -0.423169f,
        -0.423169f, 1.000000f, -0.423169f,
        -0.423169f, 1.000000f, 0.423169f,
        0.423169f, 1.000000f, 0.423169f,
        -1.000000f, 0.423169f, -0.423169f,
        -1.000000f, -0.423169f, -0.423169f,
        -1.000000f, -0.423169f, 0.423169f,
        -1.000000f, 0.423169f, 0.423169f,
        0.423169f, 0.423169f, -1.000000f,
        0.423169f, -0.423169f, -1.000000f,
        -0.423169f, -0.423169f, -1.000000f,
        -0.423169f, 0.423169f, -1.000000f,
        1.000000f, 0.423169f, 0.423169f,
        1.000000f, -0.423169f, 0.423169f,
        1.000000f, -0.423169f, -0.423169f,
        1.000000f, 0.423169f, -0.423169f,
        0.423169f, -0.423169f, 1.000000f,
        0.423169f, 0.423169f, 1.000000f,
        -0.423169f, 0.423169f, 1.000000f,
        -0.423169f, -0.423169f, 1.000000f
    };

    static const unsigned short pointLightIndexData[] =
    {
        0, 1, 2,
        0, 2, 3,
        4, 5, 6,
        4, 6, 7,
        8, 9, 10,
        8, 10, 11,
        12, 13, 14,
        12, 14, 15,
        16, 17, 18,
        16, 18, 19,
        20, 21, 22,
        20, 22, 23,
        0, 10, 9,
        0, 9, 1,
        13, 2, 1,
        13, 1, 14,
        23, 0, 3,
        23, 3, 20,
        17, 3, 2,
        17, 2, 18,
        21, 7, 6,
        21, 6, 22,
        7, 16, 19,
        7, 19, 4,
        5, 8, 11,
        5, 11, 6,
        4, 12, 15,
        4, 15, 5,
        22, 11, 10,
        22, 10, 23,
        8, 15, 14,
        8, 14, 9,
        12, 19, 18,
        12, 18, 13,
        16, 21, 20,
        16, 20, 17,
        0, 23, 10,
        1, 9, 14,
        2, 13, 18,
        3, 17, 20,
        6, 11, 22,
        5, 15, 8,
        4, 19, 12,
        7, 21, 16
    };

    static const float spotLightVertexData[] =
    {
        0.00001f, 0.00001f, 0.00001f,
        0.00001f, -0.00001f, 0.00001f,
        -0.00001f, -0.00001f, 0.00001f,
        -0.00001f, 0.00001f, 0.00001f,
        1.00000f, 1.00000f, 0.99999f,
        1.00000f, -1.00000f, 0.99999f,
        -1.00000f, -1.00000f, 0.99999f,
        -1.00000f, 1.00000f, 0.99999f,
    };

    static const unsigned short spotLightIndexData[] =
    {
        3, 0, 1,
        3, 1, 2,
        0, 4, 5,
        0, 5, 1,
        3, 7, 4,
        3, 4, 0,
        7, 3, 2,
        7, 2, 6,
        6, 2, 1,
        6, 1, 5,
        7, 5, 4,
        7, 6, 5
    };

    static const char* geometryVSVariations[] =
    {
        "",
        "SKINNED ",
        "INSTANCED ",
        "BILLBOARD ",
        "DIRBILLBOARD ",
        "TRAILFACECAM ",
        "TRAILBONE "
    };

    static const char* lightVSVariations[] =
    {
        "PERPIXEL DIRLIGHT ",
        "PERPIXEL SPOTLIGHT ",
        "PERPIXEL POINTLIGHT ",
        "PERPIXEL DIRLIGHT SHADOW ",
        "PERPIXEL SPOTLIGHT SHADOW ",
        "PERPIXEL POINTLIGHT SHADOW ",
        "PERPIXEL DIRLIGHT SHADOW NORMALOFFSET ",
        "PERPIXEL SPOTLIGHT SHADOW NORMALOFFSET ",
        "PERPIXEL POINTLIGHT SHADOW NORMALOFFSET "
    };

    static const char* vertexLightVSVariations[] =
    {
        "",
        "NUMVERTEXLIGHTS=1 ",
        "NUMVERTEXLIGHTS=2 ",
        "NUMVERTEXLIGHTS=3 ",
        "NUMVERTEXLIGHTS=4 ",
    };

    static const char* deferredLightVSVariations[] =
    {
        "",
        "DIRLIGHT ",
        "ORTHO ",
        "DIRLIGHT ORTHO "
    };

    static const char* lightPSVariations[] =
    {
        "PERPIXEL DIRLIGHT ",
        "PERPIXEL SPOTLIGHT ",
        "PERPIXEL POINTLIGHT ",
        "PERPIXEL POINTLIGHT CUBEMASK ",
        "PERPIXEL DIRLIGHT SPECULAR ",
        "PERPIXEL SPOTLIGHT SPECULAR ",
        "PERPIXEL POINTLIGHT SPECULAR ",
        "PERPIXEL POINTLIGHT CUBEMASK SPECULAR ",
        "PERPIXEL DIRLIGHT SHADOW ",
        "PERPIXEL SPOTLIGHT SHADOW ",
        "PERPIXEL POINTLIGHT SHADOW ",
        "PERPIXEL POINTLIGHT CUBEMASK SHADOW ",
        "PERPIXEL DIRLIGHT SPECULAR SHADOW ",
        "PERPIXEL SPOTLIGHT SPECULAR SHADOW ",
        "PERPIXEL POINTLIGHT SPECULAR SHADOW ",
        "PERPIXEL POINTLIGHT CUBEMASK SPECULAR SHADOW ",
        "PERPIXEL DIRLIGHT SHADOW NORMALOFFSET ",
        "PERPIXEL SPOTLIGHT SHADOW NORMALOFFSET ",
        "PERPIXEL POINTLIGHT SHADOW NORMALOFFSET ",
        "PERPIXEL POINTLIGHT CUBEMASK SHADOW NORMALOFFSET ",
        "PERPIXEL DIRLIGHT SPECULAR SHADOW NORMALOFFSET ",
        "PERPIXEL SPOTLIGHT SPECULAR SHADOW NORMALOFFSET ",
        "PERPIXEL POINTLIGHT SPECULAR SHADOW NORMALOFFSET ",
        "PERPIXEL POINTLIGHT CUBEMASK SPECULAR SHADOW NORMALOFFSET "
    };

    static const char* heightFogVariations[] =
    {
        "",
        "HEIGHTFOG "
    };

    static const unsigned MAX_BUFFER_AGE = 1000;

    static const int MAX_EXTRA_INSTANCING_BUFFER_ELEMENTS = 4;

    inline PODVector<VertexElement> CreateInstancingBufferElements(unsigned numExtraElements)
    {
        static const unsigned NUM_INSTANCEMATRIX_ELEMENTS = 3;
        static const unsigned FIRST_UNUSED_TEXCOORD = 4;

        PODVector<VertexElement> elements;
        for (unsigned i = 0; i < NUM_INSTANCEMATRIX_ELEMENTS + numExtraElements; ++i)
            elements.Push(VertexElement(TYPE_VECTOR4, SEM_TEXCOORD, FIRST_UNUSED_TEXCOORD + i, true));
        return elements;
    }

    Renderer::Renderer(Context* context)
        : Object(context)
    {
        SubscribeToEvent(E_SCREENMODE, MY3D_HANDLER(Renderer, HandleScreenMode));

        // Try to initialize right now, but skip if screen mode is not yet set
        Initialize();
    }

    Renderer::~Renderer() = default;

    void Renderer::SetShadowQuality(ShadowQuality quality)
    {
        if (!graphics_)
            return;

        // If no hardware PCF, do not allow to select one-sample quality
        if (!graphics_->GetHardwareShadowSupport())
        {
            if (quality == SHADOWQUALITY_SIMPLE_16BIT)
                quality = SHADOWQUALITY_PCF_16BIT;

            if (quality == SHADOWQUALITY_SIMPLE_24BIT)
                quality = SHADOWQUALITY_PCF_24BIT;
        }
        // if high resolution is not allowed
        if (!graphics_->GetHiresShadowMapFormat())
        {
            if (quality == SHADOWQUALITY_SIMPLE_24BIT)
                quality = SHADOWQUALITY_SIMPLE_16BIT;

            if (quality == SHADOWQUALITY_PCF_24BIT)
                quality = SHADOWQUALITY_PCF_16BIT;
        }
        if (quality != shadowQuality_)
        {
            shadowQuality_ = quality;
            shadersDirty_ = true;
            if (quality == SHADOWQUALITY_BLUR_VSM)
                SetShadowMapFilter(this, static_cast<ShadowMapFilter>(&Renderer::BlurShadowMap));
            else
                SetShadowMapFilter(nullptr, nullptr);

            ResetShadowMaps();
        }
    }

    void Renderer::SetShadowMapFilter(Object* instance, ShadowMapFilter functionPtr)
    {
        shadowMapFilterInstance_ = instance;
        shadowMapFilter_ = functionPtr;
    }

    void Renderer::SetReuseShadowMaps(bool enable)
    {
        reuseShadowMaps_ = enable;
    }

    void Renderer::SetMaxShadowMaps(int shadowMaps)
    {
        if (shadowMaps < 1)
            return;

        maxShadowMaps_ = shadowMaps;
        for (HashMap<int, Vector<SharedPtr<Texture2D> > >::Iterator i = shadowMaps_.Begin(); i != shadowMaps_.End(); ++i)
        {
            if ((int)i->second_.Size() > maxShadowMaps_)
                i->second_.Resize((unsigned)maxShadowMaps_);
        }
    }

    void Renderer::SetDynamicInstancing(bool enable)
    {
        if (!instancingBuffer_)
            enable = false;

        dynamicInstancing_ = enable;
    }

    void Renderer::SetNumExtraInstancingBufferElements(int elements)
    {
        if (numExtraInstancingBufferElements_ != elements)
        {
            numExtraInstancingBufferElements_ = Clamp(elements, 0, MAX_EXTRA_INSTANCING_BUFFER_ELEMENTS);
            CreateInstancingBuffer();
        }
    }

    void Renderer::SetMinInstances(int instances)
    {
        minInstances_ = Max(instances, 1);
    }

    void Renderer::ReloadShaders()
    {
        shadersDirty_ = true;
    }

    Viewport* Renderer::GetViewport(unsigned index) const
    {
        return index < viewports_.Size() ? viewports_[index] : nullptr;
    }

    void Renderer::CreateInstancingBuffer()
    {
        // Do not create buffer if instancing not supported
        if (!graphics_->GetInstancingSupport())
        {
            instancingBuffer_.Reset();
            dynamicInstancing_ = false;
            return;
        }

        instancingBuffer_ = new VertexBuffer(context_);
        const PODVector<VertexElement> instancingBufferElements = CreateInstancingBufferElements(numExtraInstancingBufferElements_);
        if (!instancingBuffer_->SetSize(INSTANCING_BUFFER_DEFAULT_SIZE, instancingBufferElements, true))
        {
            instancingBuffer_.Reset();
            dynamicInstancing_ = false;
        }
    }

    void Renderer::ResetShadowMaps()
    {
        shadowMaps_.Clear();
        shadowMapAllocations_.Clear();
        colorShadowMaps_.Clear();
    }

    void Renderer::ResetBuffers()
    {
        // occlusionBuffers_.Clear();
        screenBuffers_.Clear();
        screenBufferAllocations_.Clear();
    }

    String Renderer::GetShadowVariations() const
    {
        switch (shadowQuality_)
        {
            case SHADOWQUALITY_SIMPLE_16BIT:
            #ifdef MY3D_OPENGL
                return "SIMPLE_SHADOW ";
            #else
                if (graphics_->GetHardwareShadowSupport())
                    return "SIMPLE_SHADOW ";
                else
                    return "SIMPLE_SHADOW SHADOWCMP ";
            #endif
            case SHADOWQUALITY_SIMPLE_24BIT:
                return "SIMPLE_SHADOW ";
            case SHADOWQUALITY_PCF_16BIT:
            #ifdef MY3D_OPENGL
                return "PCF_SHADOW ";
            #else
                if (graphics_->GetHardwareShadowSupport())
                    return "PCF_SHADOW ";
                else
                    return "PCF_SHADOW SHADOWCMP ";
            #endif
            case SHADOWQUALITY_PCF_24BIT:
                return "PCF_SHADOW ";
            case SHADOWQUALITY_VSM:
                return "VSM_SHADOW ";
            case SHADOWQUALITY_BLUR_VSM:
                return "VSM_SHADOW ";
        }
        return "";
    }

    const Rect& Renderer::GetLightScissor(Light* light, Camera* camera)
    {
        Pair<Light*, Camera*> combination(light, camera);

        HashMap<Pair<Light*, Camera*>, Rect>::Iterator i = lightScissorCache_.Find(combination);
        if (i != lightScissorCache_.End())
            return i->second_;

        const Matrix3x4& view = camera->GetView();
        const Matrix4& projection = camera->GetProjection();

        assert(light->GetLightType() != LIGHT_DIRECTIONAL);
        if (light->GetLightType() == LIGHT_SPOT)
        {
            Frustum viewFrustum(light->GetViewSpaceFrustum(view));
            return lightScissorCache_[combination] = viewFrustum.Projected(projection);
        }
        else // LIGHT_POINT
        {
            BoundingBox viewBox(light->GetWorldBoundingBox().Transformed(view));
            return lightScissorCache_[combination] = viewBox.Projected(projection);
        }
    }

    void Renderer::QueueRenderSurface(RenderSurface* renderTarget)
    {
        if (renderTarget)
        {
            unsigned numViewports = renderTarget->GetNumViewports();

            for (unsigned i = 0; i < numViewports; ++i)
                QueueViewport(renderTarget, renderTarget->GetViewport(i));
        }
    }

    void Renderer::QueueViewport(RenderSurface* renderTarget, Viewport* viewport)
    {
        if (viewport)
        {
            Pair<WeakPtr<RenderSurface>, WeakPtr<Viewport> > newView =
                    MakePair(WeakPtr<RenderSurface>(renderTarget), WeakPtr<Viewport>(viewport));

            // Prevent double add of the same rendertarget/viewport combination
            if (!queuedViewports_.Contains(newView))
                queuedViewports_.Push(newView);
        }
    }

    Camera* Renderer::GetShadowCamera()
    {
        MutexLock lock(rendererMutex_);

        assert(numShadowCameras_ <= shadowCameraNodes_.Size());
        if (numShadowCameras_ == shadowCameraNodes_.Size())
        {
            SharedPtr<Node> newNode(new Node(context_));
            newNode->CreateComponent<Camera>();
            shadowCameraNodes_.Push(newNode);
        }

        auto* camera = shadowCameraNodes_[numShadowCameras_++]->GetComponent<Camera>();
        camera->SetOrthographic(false);
        camera->SetZoom(1.0f);

        return camera;
    }

    void Renderer::StorePreparedView(View* view, Camera* camera)
    {
        if (view && camera)
            preparedViews_[camera] = view;
    }

    View* Renderer::GetPreparedView(Camera* camera)
    {
        HashMap<Camera*, WeakPtr<View> >::Iterator i = preparedViews_.Find(camera);
        return i != preparedViews_.End() ? i->second_ : nullptr;
    }

    void Renderer::SetLightVolumeBatchShaders(Batch& batch, Camera* camera, const String& vsName, const String& psName, const String& vsDefines, const String& psDefines)
    {
        assert(deferredLightPSVariations_.Size());

        unsigned vsi = DLVS_NONE;
        unsigned psi = DLPS_NONE;
        Light* light = batch.lightQueue_->light_;

        switch (light->GetLightType())
        {
            case LIGHT_DIRECTIONAL:
                vsi += DLVS_DIR;
                break;

            case LIGHT_SPOT:
                psi += DLPS_SPOT;
                break;

            case LIGHT_POINT:
                if (light->GetShapeTexture())
                    psi += DLPS_POINTMASK;
                else
                    psi += DLPS_POINT;
                break;
        }

        if (batch.lightQueue_->shadowMap_)
        {
            if (light->GetShadowBias().normalOffset_ > 0.0)
                psi += DLPS_SHADOWNORMALOFFSET;
            else
                psi += DLPS_SHADOW;
        }

        if (specularLighting_ && light->GetSpecularIntensity() > 0.0f)
            psi += DLPS_SPEC;

        if (camera->IsOrthographic())
        {
            vsi += DLVS_ORTHO;
            psi += DLPS_ORTHO;
        }

        if (vsDefines.Length())
            batch.vertexShader_ = graphics_->GetShader(VS, vsName, deferredLightVSVariations[vsi] + vsDefines);
        else
            batch.vertexShader_ = graphics_->GetShader(VS, vsName, deferredLightVSVariations[vsi]);

        if (psDefines.Length())
            batch.pixelShader_ = graphics_->GetShader(PS, psName, deferredLightPSVariations_[psi] + psDefines);
        else
            batch.pixelShader_ = graphics_->GetShader(PS, psName, deferredLightPSVariations_[psi]);
    }

    void Renderer::SetCullMode(CullMode mode, Camera* camera)
    {
        // If a camera is specified, check whether it reverses culling due to vertical flipping or reflection
        if (camera && camera->GetReverseCulling())
        {
            if (mode == CULL_CW)
                mode = CULL_CCW;
            else if (mode == CULL_CCW)
                mode = CULL_CW;
        }

        graphics_->SetCullMode(mode);
    }

    void Renderer::OptimizeLightByScissor(Light* light, Camera* camera)
    {
        if (light && light->GetLightType() != LIGHT_DIRECTIONAL)
            graphics_->SetScissorTest(true, GetLightScissor(light, camera));
        else
            graphics_->SetScissorTest(false);
    }

    Geometry* Renderer::GetLightGeometry(Light* light)
    {
        switch (light->GetLightType())
        {
            case LIGHT_DIRECTIONAL:
                return dirLightGeometry_;
            case LIGHT_SPOT:
                return spotLightGeometry_;
            case LIGHT_POINT:
                return pointLightGeometry_;
        }

        return nullptr;
    }

    Geometry* Renderer::GetQuadGeometry()
    {
        return dirLightGeometry_;
    }

    RenderSurface* Renderer::GetDepthStencil(int width, int height, int multiSample, bool autoResolve)
    {
        // Return the default depth-stencil surface if applicable
        // (when using OpenGL Graphics will allocate right size surfaces on demand to emulate Direct3D9)
        if (width == graphics_->GetWidth() && height == graphics_->GetHeight() && multiSample == 1 &&
            graphics_->GetMultiSample() == multiSample)
            return nullptr;
        else
        {
            return static_cast<Texture2D*>(GetScreenBuffer(width, height, Graphics::GetDepthStencilFormat(), multiSample, autoResolve, false, false, false))->GetRenderSurface();
        }
    }

    void Renderer::Update(float timeStep)
    {

    }

    Texture* Renderer::GetScreenBuffer(int width, int height, unsigned format, int multiSample, bool autoResolve, bool cubemap, bool filtered, bool srgb,
                                       unsigned persistentKey)
    {
        bool depthStencil = (format == Graphics::GetDepthStencilFormat()) || (format == Graphics::GetReadableDepthFormat());
        if (depthStencil)
        {
            filtered = false;
            srgb = false;
        }

        if (cubemap)
            height = width;

        multiSample = Clamp(multiSample, 1, 16);
        if (multiSample == 1)
            autoResolve = false;

        auto searchKey = (unsigned long long)format << 32u | multiSample << 24u | width << 12u | height;
        if (filtered)
            searchKey |= 0x8000000000000000ULL;
        if (srgb)
            searchKey |= 0x4000000000000000ULL;
        if (cubemap)
            searchKey |= 0x2000000000000000ULL;
        if (autoResolve)
            searchKey |= 0x1000000000000000ULL;

        // Add persistent key if defined
        if (persistentKey)
            searchKey += (unsigned long long)persistentKey << 32u;

        // If new size or format, initialize the allocation stats
        if (screenBuffers_.Find(searchKey) == screenBuffers_.End())
            screenBufferAllocations_[searchKey] = 0;

        // Reuse depth-stencil buffers whenever the size matches, instead of allocating new
        // Unless persistency specified
        unsigned allocations = screenBufferAllocations_[searchKey];
        if (!depthStencil || persistentKey)
            ++screenBufferAllocations_[searchKey];

        if (allocations >= screenBuffers_[searchKey].Size())
        {
            SharedPtr<Texture> newBuffer;

            if (!cubemap)
            {
                SharedPtr<Texture2D> newTex2D(new Texture2D(context_));
                /// \todo Mipmaps disabled for now. Allow to request mipmapped buffer?
                newTex2D->SetNumLevels(1);
                newTex2D->SetSize(width, height, format, depthStencil ? TEXTURE_DEPTHSTENCIL : TEXTURE_RENDERTARGET, multiSample, autoResolve);

            #ifdef URHO3D_OPENGL
                // OpenGL hack: clear persistent floating point screen buffers to ensure the initial contents aren't illegal (NaN)?
                // Otherwise eg. the AutoExposure post process will not work correctly
                if (persistentKey && Texture::GetDataType(format) == GL_FLOAT)
                {
                    // Note: this loses current rendertarget assignment
                    graphics_->ResetRenderTargets();
                    graphics_->SetRenderTarget(0, newTex2D);
                    graphics_->SetDepthStencil((RenderSurface*)nullptr);
                    graphics_->SetViewport(IntRect(0, 0, width, height));
                    graphics_->Clear(CLEAR_COLOR);
                }
            #endif

                newBuffer = newTex2D;
            }
            else
            {
                SharedPtr<TextureCube> newTexCube(new TextureCube(context_));
                newTexCube->SetNumLevels(1);
                newTexCube->SetSize(width, format, TEXTURE_RENDERTARGET, multiSample);

                newBuffer = newTexCube;
            }

            newBuffer->SetSRGB(srgb);
            newBuffer->SetFilterMode(filtered ? FILTER_BILINEAR : FILTER_NEAREST);
            newBuffer->ResetUseTimer();
            screenBuffers_[searchKey].Push(newBuffer);

            MY3D_LOGDEBUG("Allocated new screen buffer size " + String(width) + "x" + String(height) + " format " + String(format));
            return newBuffer;
        }
        else
        {
            Texture* buffer = screenBuffers_[searchKey][allocations];
            buffer->ResetUseTimer();
            return buffer;
        }
    }

    void Renderer::Initialize()
    {
        auto* graphics = GetSubsystem<Graphics>();
        auto* cache = GetSubsystem<ResourceCache>();

        if (!graphics || !graphics->IsInitialized() || !cache)
            return;

        graphics_ = graphics;

        if (!graphics_->GetShadowMapFormat())
            drawShadows_ = false;

        // Validate the shadow quality level
        SetShadowQuality(shadowQuality_);

        defaultLightRamp_ = cache->GetResource<Texture2D>("Textures/Ramp.png");
        defaultLightSpot_ = cache->GetResource<Texture2D>("Textures/Spot.png");
        defaultMaterial_ = new Material(context_);

        defaultRenderPath_ = new RenderPath();
        defaultRenderPath_->Load(cache->GetResource<XMLFile>("RenderPaths/Forward.xml"));

        CreateGeometries();
        CreateInstancingBuffer();

        viewports_.Resize(1);
        ResetShadowMaps();
        ResetBuffers();

        initialized_ = true;

        SubscribeToEvent(E_RENDERUPDATE, MY3D_HANDLER(Renderer, HandleRenderUpdate));

        MY3D_LOGINFO("Initialized renderer");
    }

    void Renderer::LoadShaders()
    {
        MY3D_LOGDEBUG("Reloading shaders");

        // Release old material shaders, mark them for reload
        ReleaseMaterialShaders();
        shadersChangedFrameNumber_ = GetSubsystem<Time>()->GetFrameNumber();

        // Construct new names for deferred light volume pixel shaders based on rendering options
        deferredLightPSVariations_.Resize(MAX_DEFERRED_LIGHT_PS_VARIATIONS);

        for (unsigned i = 0; i < MAX_DEFERRED_LIGHT_PS_VARIATIONS; ++i)
        {
            deferredLightPSVariations_[i] = lightPSVariations[i % DLPS_ORTHO];
            if ((i % DLPS_ORTHO) >= DLPS_SHADOW)
                deferredLightPSVariations_[i] += GetShadowVariations();
            if (i >= DLPS_ORTHO)
                deferredLightPSVariations_[i] += "ORTHO ";
        }

        shadersDirty_ = false;
    }

    void Renderer::LoadPassShaders(Pass *pass, Vector<SharedPtr<ShaderVariation>> &vertexShaders, Vector<SharedPtr<ShaderVariation>> &pixelShaders, const BatchQueue &queue)
    {
        // Forget all the old shaders
        vertexShaders.Clear();
        pixelShaders.Clear();

        String vsDefines = pass->GetEffectiveVertexShaderDefines();
        String psDefines = pass->GetEffectivePixelShaderDefines();

        // Make sure to end defines with space to allow appending engine's defines
        if (vsDefines.Length() && !vsDefines.EndsWith(" "))
            vsDefines += ' ';
        if (psDefines.Length() && !psDefines.EndsWith(" "))
            psDefines += ' ';

        // Append defines from batch queue (renderpath command) if needed
        if (queue.vsExtraDefines_.Length())
        {
            vsDefines += queue.vsExtraDefines_;
            vsDefines += ' ';
        }
        if (queue.psExtraDefines_.Length())
        {
            psDefines += queue.psExtraDefines_;
            psDefines += ' ';
        }

        // Add defines for VSM in the shadow pass if necessary
        if (pass->GetName() == "shadow"
            && (shadowQuality_ == SHADOWQUALITY_VSM || shadowQuality_ == SHADOWQUALITY_BLUR_VSM))
        {
            vsDefines += "VSM_SHADOW ";
            psDefines += "VSM_SHADOW ";
        }

        if (pass->GetLightingMode() == LIGHTING_PERPIXEL)
        {
            // Load forward pixel lit variations
            vertexShaders.Resize(MAX_GEOMETRYTYPES * MAX_LIGHT_VS_VARIATIONS);
            pixelShaders.Resize(MAX_LIGHT_PS_VARIATIONS * 2);

            for (unsigned j = 0; j < MAX_GEOMETRYTYPES * MAX_LIGHT_VS_VARIATIONS; ++j)
            {
                unsigned g = j / MAX_LIGHT_VS_VARIATIONS;
                unsigned l = j % MAX_LIGHT_VS_VARIATIONS;

                vertexShaders[j] = graphics_->GetShader(VS, pass->GetVertexShader(),
                    vsDefines + lightVSVariations[l] + geometryVSVariations[g]);
            }
            for (unsigned j = 0; j < MAX_LIGHT_PS_VARIATIONS * 2; ++j)
            {
                unsigned l = j % MAX_LIGHT_PS_VARIATIONS;
                unsigned h = j / MAX_LIGHT_PS_VARIATIONS;

                if (l & LPS_SHADOW)
                {
                    pixelShaders[j] = graphics_->GetShader(PS, pass->GetPixelShader(),
                        psDefines + lightPSVariations[l] + GetShadowVariations() + heightFogVariations[h]);
                }
                else
                    pixelShaders[j] = graphics_->GetShader(PS, pass->GetPixelShader(),
                        psDefines + lightPSVariations[l] + heightFogVariations[h]);
            }
        }
        else
        {
            // Load vertex light variations
            if (pass->GetLightingMode() == LIGHTING_PERVERTEX)
            {
                vertexShaders.Resize(MAX_GEOMETRYTYPES * MAX_VERTEXLIGHT_VS_VARIATIONS);
                for (unsigned j = 0; j < MAX_GEOMETRYTYPES * MAX_VERTEXLIGHT_VS_VARIATIONS; ++j)
                {
                    unsigned g = j / MAX_VERTEXLIGHT_VS_VARIATIONS;
                    unsigned l = j % MAX_VERTEXLIGHT_VS_VARIATIONS;
                    vertexShaders[j] = graphics_->GetShader(VS, pass->GetVertexShader(),
                        vsDefines + vertexLightVSVariations[l] + geometryVSVariations[g]);
                }
            }
            else
            {
                vertexShaders.Resize(MAX_GEOMETRYTYPES);
                for (unsigned j = 0; j < MAX_GEOMETRYTYPES; ++j)
                {
                    vertexShaders[j] = graphics_->GetShader(VS, pass->GetVertexShader(), vsDefines + geometryVSVariations[j]);
                }
            }

            pixelShaders.Resize(2);
            for (unsigned j = 0; j < 2; ++j)
            {
                pixelShaders[j] = graphics_->GetShader(PS, pass->GetPixelShader(), psDefines + heightFogVariations[j]);
            }
        }

        pass->MarkShadersLoaded(shadersChangedFrameNumber_);
    }

    void Renderer::SetBatchShaders(Batch& batch, Technique* tech, bool allowShadows, const BatchQueue& queue)
    {
        Pass* pass = batch.pass_;

        // Check if need to release/reload all shaders
        if (pass->GetShadersLoadedFrameNumber() != shadersChangedFrameNumber_)
            pass->ReleaseShaders();

        Vector<SharedPtr<ShaderVariation> >& vertexShaders = queue.hasExtraDefines_ ? pass->GetVertexShaders(queue.vsExtraDefinesHash_) : pass->GetVertexShaders();
        Vector<SharedPtr<ShaderVariation> >& pixelShaders = queue.hasExtraDefines_ ? pass->GetPixelShaders(queue.psExtraDefinesHash_) : pass->GetPixelShaders();

        // Load shaders now if necessary
        if (!vertexShaders.Size() || !pixelShaders.Size())
            LoadPassShaders(pass, vertexShaders, pixelShaders, queue);

        // Make sure shaders are loaded now
        if (vertexShaders.Size() && pixelShaders.Size())
        {
            bool heightFog = batch.zone_ && batch.zone_->GetHeightFog();

            // If instancing is not supported, but was requested, choose static geometry vertex shader instead
            if (batch.geometryType_ == GEOM_INSTANCED && !GetDynamicInstancing())
                batch.geometryType_ = GEOM_STATIC;

            if (batch.geometryType_ == GEOM_STATIC_NOINSTANCING)
                batch.geometryType_ = GEOM_STATIC;

            //  Check whether is a pixel lit forward pass. If not, there is only one pixel shader
            if (pass->GetLightingMode() == LIGHTING_PERPIXEL)
            {
                LightBatchQueue* lightQueue = batch.lightQueue_;
                if (!lightQueue)
                {
                    // Do not log error, as it would result in a lot of spam
                    batch.vertexShader_ = nullptr;
                    batch.pixelShader_ = nullptr;
                    return;
                }

                Light* light = lightQueue->light_;
                unsigned vsi = 0;
                unsigned psi = 0;
                vsi = batch.geometryType_ * MAX_LIGHT_VS_VARIATIONS;

                bool materialHasSpecular = batch.material_ ? batch.material_->GetSpecular() : true;
                if (specularLighting_ && light->GetSpecularIntensity() > 0.0f && materialHasSpecular)
                    psi += LPS_SPEC;
                if (allowShadows && lightQueue->shadowMap_)
                {
                    if (light->GetShadowBias().normalOffset_ > 0.0f)
                        vsi += LVS_SHADOWNORMALOFFSET;
                    else
                        vsi += LVS_SHADOW;
                    psi += LPS_SHADOW;
                }

                switch (light->GetLightType())
                {
                    case LIGHT_DIRECTIONAL:
                        vsi += LVS_DIR;
                        break;

                    case LIGHT_SPOT:
                        psi += LPS_SPOT;
                        vsi += LVS_SPOT;
                        break;

                    case LIGHT_POINT:
                        if (light->GetShapeTexture())
                            psi += LPS_POINTMASK;
                        else
                            psi += LPS_POINT;
                        vsi += LVS_POINT;
                        break;
                }

                if (heightFog)
                    psi += MAX_LIGHT_PS_VARIATIONS;

                batch.vertexShader_ = vertexShaders[vsi];
                batch.pixelShader_ = pixelShaders[psi];
            }
            else
            {
                // Check if pass has vertex lighting support
                if (pass->GetLightingMode() == LIGHTING_PERVERTEX)
                {
                    unsigned numVertexLights = 0;
                    if (batch.lightQueue_)
                        numVertexLights = batch.lightQueue_->vertexLights_.Size();

                    unsigned vsi = batch.geometryType_ * MAX_VERTEXLIGHT_VS_VARIATIONS + numVertexLights;
                    batch.vertexShader_ = vertexShaders[vsi];
                }
                else
                {
                    unsigned vsi = batch.geometryType_;
                    batch.vertexShader_ = vertexShaders[vsi];
                }

                batch.pixelShader_ = pixelShaders[heightFog ? 1 : 0];
            }
        }

        // Log error if shaders could not be assigned, but only once per technique
        if (!batch.vertexShader_ || !batch.pixelShader_)
        {
            if (!shaderErrorDisplayed_.Contains(tech))
            {
                shaderErrorDisplayed_.Insert(tech);
                MY3D_LOGERROR("Technique " + tech->GetName() + " has missing shaders");
            }
        }
    }

    RenderPath* Renderer::GetDefaultRenderPath() const
    {
        return defaultRenderPath_;
    }

    Technique* Renderer::GetDefaultTechnique() const
    {
        // Assign default when first asked if not assigned yet
        if (!defaultTechnique_)
            const_cast<SharedPtr<Technique>& >(defaultTechnique_) = GetSubsystem<ResourceCache>()->GetResource<Technique>("Techniques/NoTexture.xml");

        return defaultTechnique_;
    }

    void Renderer::ReleaseMaterialShaders()
    {
        auto* cache = GetSubsystem<ResourceCache>();
        PODVector<Material*> materials;

        cache->GetResources<Material>(materials);

        for (unsigned i = 0; i < materials.Size(); ++i)
            materials[i]->ReleaseShaders();
    }

    void Renderer::ReloadTextures()
    {
        auto* cache = GetSubsystem<ResourceCache>();
        PODVector<Resource*> textures;

        cache->GetResources(textures, Texture2D::GetTypeStatic());
        for (unsigned i = 0; i < textures.Size(); ++i)
            cache->ReloadResource(textures[i]);

        cache->GetResources(textures, TextureCube::GetTypeStatic());
        for (unsigned i = 0; i < textures.Size(); ++i)
            cache->ReloadResource(textures[i]);
    }

    void Renderer::HandleScreenMode(StringHash eventType, VariantMap& eventData)
    {
        if (!initialized_)
            Initialize();
        else
            resetViews_ = true;
    }

    void Renderer::HandleRenderUpdate(StringHash eventType, VariantMap& eventData)
    {
        using namespace RenderUpdate;

        Update(eventData[P_TIMESTEP].GetFloat());
    }

    void Renderer::BlurShadowMap(View* view, Texture2D* shadowMap, float blurScale)
    {
        graphics_->SetBlendMode(BLEND_REPLACE);
        graphics_->SetDepthTest(CMP_ALWAYS);
        graphics_->SetClipPlane(false);
        graphics_->SetScissorTest(false);

        // Get a temporary render buffer
        auto* tmpBuffer = static_cast<Texture2D*>(GetScreenBuffer(shadowMap->GetWidth(), shadowMap->GetHeight(),
                                                                  shadowMap->GetFormat(), 1, false, false, false, false));
        graphics_->SetRenderTarget(0, tmpBuffer->GetRenderSurface());
        graphics_->SetDepthStencil(GetDepthStencil(shadowMap->GetWidth(), shadowMap->GetHeight(), shadowMap->GetMultiSample(), shadowMap->GetAutoResolve()));
        graphics_->SetViewport(IntRect(0, 0, shadowMap->GetWidth(), shadowMap->GetHeight()));

        // Get shaders
        static const char* shaderName = "ShadowBlur";
        ShaderVariation* vs = graphics_->GetShader(VS, shaderName);
        ShaderVariation* ps = graphics_->GetShader(PS, shaderName);
        graphics_->SetShaders(vs, ps);

        view->SetGBufferShaderParameters(IntVector2(shadowMap->GetWidth(), shadowMap->GetHeight()), IntRect(0, 0, shadowMap->GetWidth(), shadowMap->GetHeight()));

        // Horizontal blur of the shadow map
        static const StringHash blurOffsetParam("BlurOffsets");

        graphics_->SetShaderParameter(blurOffsetParam, Vector2(shadowSoftness_ * blurScale / shadowMap->GetWidth(), 0.0f));
        graphics_->SetTexture(TU_DIFFUSE, shadowMap);
        view->DrawFullscreenQuad(true);

        // Vertical blur
        graphics_->SetRenderTarget(0, shadowMap);
        graphics_->SetViewport(IntRect(0, 0, shadowMap->GetWidth(), shadowMap->GetHeight()));
        graphics_->SetShaderParameter(blurOffsetParam, Vector2(0.0f, shadowSoftness_ * blurScale / shadowMap->GetHeight()));

        graphics_->SetTexture(TU_DIFFUSE, tmpBuffer);
        view->DrawFullscreenQuad(true);
    }
}
