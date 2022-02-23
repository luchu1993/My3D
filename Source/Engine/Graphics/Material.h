//
// Created by luchu on 2022/2/19.
//

#pragma once

#include "Resource/Resource.h"
#include "Scene/ValueAnimation.h"
#include "Graphics/GraphicsDefs.h"
#include "Scene/ValueAnimationInfo.h"
#include "Resource/XMLFile.h"


namespace My3D
{
    class Material;
    class Pass;
    class Scene;
    class Technique;
    class Texture;
    class Texture2D;
    class TextureCube;
    class ValueAnimationInfo;

    static const unsigned char DEFAULT_RENDER_ORDER = 128;

    /// Material's shader parameter definition.
    struct MaterialShaderParameter
    {
        /// Name.
        String name_;
        /// Value.
        Variant value_;
    };

    /// Material's technique list entry.
    struct TechniqueEntry
    {
        /// Construct with defaults.
        TechniqueEntry() noexcept;
        /// Construct with parameters.
        TechniqueEntry(Technique* tech, MaterialQuality qualityLevel, float lodDistance) noexcept;
        /// Destruct.
        ~TechniqueEntry() noexcept = default;

        /// Technique.
        SharedPtr<Technique> technique_;
        /// Original technique, in case the material adds shader compilation defines. The modified clones are requested from it.
        SharedPtr<Technique> original_;
        /// Quality level.
        MaterialQuality qualityLevel_;
        /// LOD distance.
        float lodDistance_;
    };

    /// Material's shader parameter animation instance.
    class ShaderParameterAnimationInfo : public ValueAnimationInfo
    {
    public:
        /// Construct.
        ShaderParameterAnimationInfo(Material* material, const String& name, ValueAnimation* attributeAnimation, WrapMode wrapMode, float speed);
        /// Copy construct.
        ShaderParameterAnimationInfo(const ShaderParameterAnimationInfo& other);
        /// Destruct.
        ~ShaderParameterAnimationInfo() override;

        /// Return shader parameter name.
        const String& GetName() const { return name_; }

    protected:
        /// Apply new animation value to the target object. Called by Update().
        void ApplyValue(const Variant& newValue) override;

    private:
        /// Shader parameter name.
        String name_;
    };

    /// TextureUnit hash function.
    template <> inline unsigned MakeHash(const TextureUnit& value)
    {
        return (unsigned)value;
    }

    /// Describes how to render 3D geometries.
    class MY3D_API Material : public Resource
    {
        MY3D_OBJECT(Material, Resource)
    public:
        /// Construct.
        explicit Material(Context* context);
        /// Destruct.
        ~Material() override;
        /// Register object factory.
        static void RegisterObject(Context* context);

        /// Load from an XML element. Return true if successful.
        bool Load(const XMLElement& source);
        /// Save to an XML element. Return true if successful.
        bool Save(XMLElement& dest) const;
        /// Set number of techniques.
        void SetNumTechniques(unsigned num);
        /// Set technique.
        void SetTechnique(unsigned index, Technique* tech, MaterialQuality qualityLevel = QUALITY_LOW, float lodDistance = 0.0f);
        /// Set additional vertex shader defines. Separate multiple defines with spaces. Setting defines at the material level causes technique(s) to be cloned as necessary.
        void SetVertexShaderDefines(const String& defines);
        /// Set additional pixel shader defines. Separate multiple defines with spaces. Setting defines at the material level causes technique(s) to be cloned as necessary.
        void SetPixelShaderDefines(const String& defines);
        /// Set shader parameter.
        void SetShaderParameter(const String& name, const Variant& value);

        /// Return name for texture unit.
        static String GetTextureUnitName(TextureUnit unit);
        /// Parse a shader parameter value from a string. Retunrs either a bool, a float, or a 2 to 4-component vector.
        static Variant ParseShaderParameterValue(const String& value);

    private:
        /// Helper function for loading XML files.
        bool BeginLoadXML(Deserializer& source);
        /// Reset to defaults.
        void ResetToDefaults();
        /// Recalculate shader parameter hash.
        void RefreshShaderParameterHash();
        /// Recalculate the memory used by the material.
        void RefreshMemoryUse();
        /// Reapply shader defines to technique index. By default reapply all.
        void ApplyShaderDefines(unsigned index = M_MAX_UNSIGNED);
        /// Return shader parameter animation info.
        ShaderParameterAnimationInfo* GetShaderParameterAnimationInfo(const String& name) const;
        /// Update whether should be subscribed to scene or global update events for shader parameter animation.
        void UpdateEventSubscription();
        /// Update shader parameter animations.
        void HandleAttributeAnimationUpdate(StringHash eventType, VariantMap& eventData);

        /// Techniques.
        Vector<TechniqueEntry> techniques_;
        /// Textures.
        HashMap<TextureUnit, SharedPtr<Texture> > textures_;
        /// Shader parameters.
        HashMap<StringHash, MaterialShaderParameter> shaderParameters_;
        /// Shader parameters animation infos.
        HashMap<StringHash, SharedPtr<ShaderParameterAnimationInfo> > shaderParameterAnimationInfos_;
        /// Vertex shader defines.
        String vertexShaderDefines_;
        /// Pixel shader defines.
        String pixelShaderDefines_;
        /// Normal culling mode.
        CullMode cullMode_{};
        /// Culling mode for shadow rendering.
        CullMode shadowCullMode_{};
        /// Polygon fill mode.
        FillMode fillMode_{};
        /// Render order value.
        unsigned char renderOrder_{};
        /// Last auxiliary view rendered frame number.
        unsigned auxViewFrameNumber_{};
        /// Shader parameter hash value.
        unsigned shaderParameterHash_{};
        /// Alpha-to-coverage flag.
        bool alphaToCoverage_{};
        /// Line antialiasing flag.
        bool lineAntiAlias_{};
        /// Render occlusion flag.
        bool occlusion_{true};
        /// Specular lighting flag.
        bool specular_{};
        /// Flag for whether is subscribed to animation updates.
        bool subscribed_{};
        /// Flag to suppress parameter hash and memory use recalculation when setting multiple shader parameters (loading or resetting the material).
        bool batchedParameterUpdate_{};
        /// XML file used while loading.
        SharedPtr<XMLFile> loadXMLFile_;
        /// Associated scene for shader parameter animation updates.
        WeakPtr<Scene> scene_;
    };
}
