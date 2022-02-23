//
// Created by luchu on 2022/2/21.
//

#pragma once

#include "Core/Variant.h"
#include "Container/Ptr.h"
#include "Container/RefCounted.h"
#include "Graphics/GraphicsDefs.h"
#include "Math/Color.h"
#include "Math/Vector4.h"


namespace My3D
{
    class XMLElement;
    class XMLFile;

    /// Rendering path command types.
    enum RenderCommandType
    {
        CMD_NONE = 0,
        CMD_CLEAR,
        CMD_SCENEPASS,
        CMD_QUAD,
        CMD_FORWARDLIGHTS,
        CMD_LIGHTVOLUMES,
        CMD_RENDERUI,
        CMD_SENDEVENT
    };

    /// Rendering path sorting modes.
    enum RenderCommandSortMode
    {
        SORT_FRONTTOBACK = 0,
        SORT_BACKTOFRONT
    };

    /// Rendertarget size mode.
    enum RenderTargetSizeMode
    {
        SIZE_ABSOLUTE = 0,
        SIZE_VIEWPORTDIVISOR,
        SIZE_VIEWPORTMULTIPLIER
    };

    /// Rendertarget definition.
    struct MY3D_API RenderTargetInfo
    {
        /// Read from an XML element.
        void Load(const XMLElement& element);

        /// Name.
        String name_;
        /// Tag name.
        String tag_;
        /// Texture format.
        unsigned format_{};
        /// Absolute size or multiplier.
        Vector2 size_;
        /// Size mode.
        RenderTargetSizeMode sizeMode_{SIZE_ABSOLUTE};
        /// Multisampling level (1 = no multisampling).
        int multiSample_{1};
        /// Multisampling autoresolve flag.
        bool autoResolve_{true};
        /// Enabled flag.
        bool enabled_{true};
        /// Cube map flag.
        bool cubemap_{};
        /// Filtering flag.
        bool filtered_{};
        /// sRGB sampling/writing mode flag.
        bool sRGB_{};
        /// Should be persistent and not shared/reused between other buffers of same size.
        bool persistent_{};
    };

    /// Rendering path command.
    struct MY3D_API RenderPathCommand
    {
        /// Read from an XML element.
        void Load(const XMLElement& element);
        /// Set a texture resource name. Can also refer to a rendertarget defined in the rendering path.
        void SetTextureName(TextureUnit unit, const String& name);
        /// Set a shader parameter.
        void SetShaderParameter(const String& name, const Variant& value);
        /// Remove a shader parameter.
        void RemoveShaderParameter(const String& name);
        /// Set number of output rendertargets.
        void SetNumOutputs(unsigned num);
        /// Set output rendertarget name and face index for cube maps.
        void SetOutput(unsigned index, const String& name, CubeMapFace face = FACE_POSITIVE_X);
        /// Set output rendertarget name.
        void SetOutputName(unsigned index, const String& name);
        /// Set output rendertarget face index for cube maps.
        void SetOutputFace(unsigned index, CubeMapFace face);
        /// Set depth-stencil output name. When empty, will assign a depth-stencil buffer automatically.
        void SetDepthStencilName(const String& name);
        /// Return texture resource name.
        const String& GetTextureName(TextureUnit unit) const;
        /// Return shader parameter.
        const Variant& GetShaderParameter(const String& name) const;
        /// Return number of output rendertargets.
        unsigned GetNumOutputs() const { return outputs_.Size(); }
        /// Return output rendertarget name.
        const String& GetOutputName(unsigned index) const;
        /// Return output rendertarget face index.
        CubeMapFace GetOutputFace(unsigned index) const;
        /// Return depth-stencil output name.
        const String& GetDepthStencilName() const { return depthStencilName_; }

        /// Tag name.
        String tag_;
        /// Command type.
        RenderCommandType type_{};
        /// Sorting mode.
        RenderCommandSortMode sortMode_{};
        /// Scene pass name.
        String pass_;
        /// Scene pass index. Filled by View.
        unsigned passIndex_{};
        /// Command/pass metadata.
        String metadata_;
        /// Vertex shader name.
        String vertexShaderName_;
        /// Pixel shader name.
        String pixelShaderName_;
        /// Vertex shader defines.
        String vertexShaderDefines_;
        /// Pixel shader defines.
        String pixelShaderDefines_;
        /// Textures.
        String textureNames_[MAX_TEXTURE_UNITS];
        /// %Shader parameters.
        HashMap<StringHash, Variant> shaderParameters_;
        /// Output rendertarget names and faces.
        Vector<Pair<String, CubeMapFace> > outputs_;
        /// Depth-stencil output name.
        String depthStencilName_;
        /// Clear flags. Affects clear command only.
        ClearTargetFlags clearFlags_{};
        /// Clear color. Affects clear command only.
        Color clearColor_;
        /// Clear depth. Affects clear command only.
        float clearDepth_{};
        /// Clear stencil value. Affects clear command only.
        unsigned clearStencil_{};
        /// Blend mode. Affects quad command only.
        BlendMode blendMode_{BLEND_REPLACE};
        /// Enabled flag.
        bool enabled_{true};
        /// Use fog color for clearing.
        bool useFogColor_{};
        /// Mark to stencil flag.
        bool markToStencil_{};
        /// Use lit base pass optimization for forward per-pixel lights.
        bool useLitBase_{true};
        /// Vertex lights flag.
        bool vertexLights_{};
        /// Event name.
        String eventName_;
    };

    /// Rendering path definition. A sequence of commands (e.g. clear screen, draw objects with specific pass) that yields the scene rendering result.
    class MY3D_API RenderPath : public RefCounted
    {
    public:
        /// Construct.
        RenderPath();
        /// Destruct.
        ~RenderPath() override;

        /// Clone the rendering path.
        SharedPtr<RenderPath> Clone();
        /// Clear existing data and load from an XML file. Return true if successful.
        bool Load(XMLFile* file);
        /// Append data from an XML file. Return true if successful.
        bool Append(XMLFile* file);

        /// Rendertargets.
        Vector<RenderTargetInfo> renderTargets_;
        /// Rendering commands.
        Vector<RenderPathCommand> commands_;
    };
}