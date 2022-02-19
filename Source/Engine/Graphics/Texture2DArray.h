//
// Created by luchu on 2022/2/19.
//

#pragma once

#include "Container/Ptr.h"
#include "Graphics/RenderSurface.h"
#include "Graphics/Texture.h"


namespace My3D
{
    class Deserializer;
    class Image;

    /// 2D texture array resource.
    class MY3D_API Texture2DArray : public Texture
    {
        MY3D_OBJECT(Texture2DArray, Texture);

        public:
        /// Construct.
        explicit Texture2DArray(Context* context);
        /// Destruct.
        ~Texture2DArray() override;
        /// Register object factory.
        /// @nobind
        static void RegisterObject(Context* context);

        /// Load resource from stream. May be called from a worker thread. Return true if successful.
        bool BeginLoad(Deserializer& source) override;
        /// Finish resource loading. Always called from the main thread. Return true if successful.
        bool EndLoad() override;
        /// Mark the GPU resource destroyed on context destruction.
        void OnDeviceLost() override;
        /// Recreate the GPU resource and restore data if applicable.
        void OnDeviceReset() override;
        /// Release the texture.
        void Release() override;

        /// Set the number of layers in the texture. To be used before SetData.
        /// @property
        void SetLayers(unsigned layers);
        /// Set layers, size, format and usage. Set layers to zero to leave them unchanged. Return true if successful.
        bool SetSize(unsigned layers, int width, int height, unsigned format, TextureUsage usage = TEXTURE_STATIC);
        /// Set data either partially or fully on a layer's mip level. Return true if successful.
        bool SetData(unsigned layer, unsigned level, int x, int y, int width, int height, const void* data);
        /// Set data of one layer from a stream. Return true if successful.
        bool SetData(unsigned layer, Deserializer& source);
        /// Set data of one layer from an image. Return true if successful. Optionally make a single channel image alpha-only.
        bool SetData(unsigned layer, Image* image, bool useAlpha = false);

        /// Return number of layers in the texture.
        /// @property
        unsigned GetLayers() const { return layers_; }
        /// Get data from a mip level. The destination buffer must be big enough. Return true if successful.
        bool GetData(unsigned layer, unsigned level, void* dest) const;
        /// Return render surface.
        /// @property
        RenderSurface* GetRenderSurface() const { return renderSurface_; }

protected:
        /// Create the GPU texture.
        bool Create() override;

private:
        /// Handle render surface update event.
        void HandleRenderSurfaceUpdate(StringHash eventType, VariantMap& eventData);

        /// Texture array layers number.
        unsigned layers_{};
        /// Render surface.
        SharedPtr<RenderSurface> renderSurface_;
        /// Memory use per layer.
        PODVector<unsigned> layerMemoryUse_;
        /// Layer image files acquired during BeginLoad.
        Vector<SharedPtr<Image> > loadImages_;
        /// Parameter file acquired during BeginLoad.
        SharedPtr<XMLFile> loadParameters_;
    };
}
