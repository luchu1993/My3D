//
// Created by luchu on 2022/2/19.
//

#include "Core/Context.h"
#include "Graphics/Graphics.h"
#include "Graphics/GraphicsEvents.h"
#include "Graphics/Renderer.h"
#include "Graphics/Texture2D.h"
#include "IO/FileSystem.h"
#include "IO/Log.h"
#include "Resource/ResourceCache.h"
#include "Resource/XMLFile.h"


namespace My3D
{

    Texture2D::Texture2D(Context *context)
        : Texture(context)
    {
#ifdef MY3D_OPENGL
        target_ = GL_TEXTURE_2D;
#endif
    }

    Texture2D::~Texture2D()
    {
        Release();
    }

    void Texture2D::RegisterObject(Context* context)
    {
        context->RegisterFactory<Texture2D>();
    }

    bool Texture2D::BeginLoad(Deserializer& source)
    {
        // In headless mode, do not actually load the texture, just return success
        if (!graphics_)
            return true;

        // If device is lost, retry later
        if (graphics_->IsDeviceLost())
        {
            MY3D_LOGWARNING("Texture load while device is lost");
            dataPending_ = true;
            return true;
        }

        // Load the image data for EndLoad()
        loadImage_ = new Image(context_);
        if (!loadImage_->Load(source))
        {
            loadImage_.Reset();
            return false;
        }

        // Precalculate mip levels if async loading
        if (GetAsyncLoadState() == ASYNC_LOADING)
            loadImage_->PrecalculateLevels();

        // Load the optional parameters file
        auto* cache = GetSubsystem<ResourceCache>();
        String xmlName = ReplaceExtension(GetName(), ".xml");
        loadParameters_ = cache->GetTempResource<XMLFile>(xmlName, false);

        return true;
    }

    bool Texture2D::EndLoad()
    {
        // In headless mode, do not actually load the texture, just return success
        if (!graphics_ || graphics_->IsDeviceLost())
            return true;

        // If over the texture budget, see if materials can be freed to allow textures to be freed
        CheckTextureBudget(GetTypeStatic());

        SetParameters(loadParameters_);
        bool success = SetData(loadImage_);

        loadImage_.Reset();
        loadParameters_.Reset();

        return success;
    }

    bool Texture2D::SetSize(int width, int height, unsigned format, TextureUsage usage, int multiSample, bool autoResolve)
    {
        if (width <= 0 || height <= 0)
        {
            MY3D_LOGERROR("Zero or negative texture dimensions");
            return false;
        }

        multiSample = Clamp(multiSample, 1, 16);
        if (multiSample == 1)
            autoResolve = false;
        else if (multiSample > 1 && usage < TEXTURE_RENDERTARGET)
        {
            MY3D_LOGERROR("Multisampling is only supported for rendertarget or depth-stencil textures");
            return false;
        }

        // Disable mipmaps if multisample & custom resolve
        if (multiSample > 1 && autoResolve == false)
            requestedLevels_ = 1;

        // Delete the old rendersurface if any
        renderSurface_.Reset();

        usage_ = usage;

        if (usage >= TEXTURE_RENDERTARGET)
        {
            renderSurface_ = new RenderSurface(this);

            // Clamp mode addressing by default and nearest filtering
            addressModes_[COORD_U] = ADDRESS_CLAMP;
            addressModes_[COORD_V] = ADDRESS_CLAMP;
            filterMode_ = FILTER_NEAREST;
        }

        if (usage == TEXTURE_RENDERTARGET)
            SubscribeToEvent(E_RENDERSURFACEUPDATE, MY3D_HANDLER(Texture2D, HandleRenderSurfaceUpdate));
        else
            UnsubscribeFromEvent(E_RENDERSURFACEUPDATE);

        width_ = width;
        height_ = height;
        format_ = format;
        depth_ = 1;
        multiSample_ = multiSample;
        autoResolve_ = autoResolve;

        return Create();
    }

    bool Texture2D::GetImage(Image& image) const
    {
        if (format_ != Graphics::GetRGBAFormat() && format_ != Graphics::GetRGBFormat())
        {
            MY3D_LOGERROR("Unsupported texture format, can not convert to Image");
            return false;
        }

        image.SetSize(width_, height_, GetComponents());
        GetData(0, image.GetData());
        return true;
    }

    SharedPtr<Image> Texture2D::GetImage() const
    {
        auto rawImage = MakeShared<Image>(context_);
        if (!GetImage(*rawImage))
            return nullptr;
        return rawImage;
    }

    void Texture2D::HandleRenderSurfaceUpdate(StringHash eventType, VariantMap& eventData)
    {
        if (renderSurface_ && (renderSurface_->GetUpdateMode() == SURFACE_UPDATEALWAYS || renderSurface_->IsUpdateQueued()))
        {
            auto* renderer = GetSubsystem<Renderer>();
            if (renderer)
                renderer->QueueRenderSurface(renderSurface_);
            renderSurface_->ResetUpdateQueued();
        }
    }
}
