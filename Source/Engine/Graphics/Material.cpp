//
// Created by luchu on 2022/2/19.
//

#include "Core/Context.h"
#include "Core/CoreEvents.h"
#include "Graphics/Graphics.h"
#include "Graphics/Material.h"
#include "Graphics/Texture2D.h"
#include "Graphics/TextureCube.h"
#include "Graphics/Texture2DArray.h"
#include "Graphics/Texture2D.h"
#include "Graphics/Texture3D.h"
#include "Graphics/Technique.h"
#include "IO/FileSystem.h"
#include "IO/Log.h"
#include "IO/VectorBuffer.h"
#include "Resource/ResourceCache.h"
#include "Resource/XMLFile.h"
#include "Scene/Scene.h"
#include "Scene/SceneEvents.h"
#include "Scene/ValueAnimation.h"


namespace My3D
{
    extern const char* wrapModeNames[];
    static const char* textureUnitNames[] =
    {
        "diffuse",
        "normal",
        "specular",
        "emissive",
        "environment",
        "volume",
        "custom1",
        "custom2",
        "lightramp",
        "lightshape",
        "shadowmap",
        "faceselect",
        "indirection",
        "depth",
        "light",
        "zone",
        nullptr
    };

    const char* cullModeNames[] =
    {
        "none",
        "ccw",
        "cw",
        nullptr
    };

    static const char* fillModeNames[] =
    {
        "solid",
        "wireframe",
        "point",
        nullptr
    };

    TextureUnit ParseTextureUnitName(String name)
    {
        name = name.ToLower().Trimmed();

        auto unit = (TextureUnit)GetStringListIndex(name.CString(), textureUnitNames, MAX_TEXTURE_UNITS);
        if (unit == MAX_TEXTURE_UNITS)
        {
            // Check also for shorthand names
            if (name == "diff")
                unit = TU_DIFFUSE;
            else if (name == "albedo")
                unit = TU_DIFFUSE;
            else if (name == "norm")
                unit = TU_NORMAL;
            else if (name == "spec")
                unit = TU_SPECULAR;
            else if (name == "env")
                unit = TU_ENVIRONMENT;
                // Finally check for specifying the texture unit directly as a number
            else if (name.Length() < 3)
                unit = (TextureUnit)Clamp(ToInt(name), 0, MAX_TEXTURE_UNITS - 1);
        }

        if (unit == MAX_TEXTURE_UNITS)
            MY3D_LOGERROR("Unknown texture unit name " + name);

        return unit;
    }

    StringHash ParseTextureTypeName(const String& name)
    {
        String lowerCaseName = name.ToLower().Trimmed();

        if (lowerCaseName == "texture")
            return Texture2D::GetTypeStatic();
        else if (lowerCaseName == "cubemap")
            return TextureCube::GetTypeStatic();
        else if (lowerCaseName == "texture3d")
            return Texture3D::GetTypeStatic();
        else if (lowerCaseName == "texturearray")
            return Texture2DArray::GetTypeStatic();

        return nullptr;
    }

    StringHash ParseTextureTypeXml(ResourceCache* cache, const String& filename)
    {
        StringHash type = nullptr;
        if (!cache)
            return type;

        SharedPtr<File> texXmlFile = cache->GetFile(filename, false);
        if (texXmlFile.NotNull())
        {
            SharedPtr<XMLFile> texXml(new XMLFile(cache->GetContext()));
            if (texXml->Load(*texXmlFile))
                type = ParseTextureTypeName(texXml->GetRoot().GetName());
        }
        return type;
    }

    static TechniqueEntry noEntry;

    bool CompareTechniqueEntries(const TechniqueEntry& lhs, const TechniqueEntry& rhs)
    {
        if (lhs.lodDistance_ != rhs.lodDistance_)
            return lhs.lodDistance_ > rhs.lodDistance_;
        else
            return lhs.qualityLevel_ > rhs.qualityLevel_;
    }

    TechniqueEntry::TechniqueEntry() noexcept
        : qualityLevel_(QUALITY_LOW)
        , lodDistance_(0.0f)
    {
    }

    TechniqueEntry::TechniqueEntry(Technique* tech, MaterialQuality qualityLevel, float lodDistance) noexcept
        : technique_(tech)
        , original_(tech)
        , qualityLevel_(qualityLevel)
        , lodDistance_(lodDistance)
    {
    }

    ShaderParameterAnimationInfo::ShaderParameterAnimationInfo(const ShaderParameterAnimationInfo& other) = default;

    ShaderParameterAnimationInfo::~ShaderParameterAnimationInfo() = default;

    void ShaderParameterAnimationInfo::ApplyValue(const Variant& newValue)
    {
        static_cast<Material*>(target_.Get())->SetShaderParameter(name_, newValue);
    }

    Material::Material(Context* context)
        : Resource(context)
    {
    }

    Material::~Material() = default;

    void Material::RefreshShaderParameterHash()
    {
        VectorBuffer temp;
        for (HashMap<StringHash, MaterialShaderParameter>::ConstIterator i = shaderParameters_.Begin();
             i != shaderParameters_.End(); ++i)
        {
            temp.WriteStringHash(i->first_);
            temp.WriteVariant(i->second_.value_);
        }

        shaderParameterHash_ = 0;
        const unsigned char* data = temp.GetData();
        unsigned dataSize = temp.GetSize();
        for (unsigned i = 0; i < dataSize; ++i)
            shaderParameterHash_ = SDBMHash(shaderParameterHash_, data[i]);
    }

    void Material::RefreshMemoryUse()
    {
        unsigned memoryUse = sizeof(Material);

        memoryUse += techniques_.Size() * sizeof(TechniqueEntry);
        memoryUse += MAX_TEXTURE_UNITS * sizeof(SharedPtr<Texture>);
        memoryUse += shaderParameters_.Size() * sizeof(MaterialShaderParameter);

        SetMemoryUse(memoryUse);
    }

    void Material::SetShaderParameter(const String& name, const Variant& value)
    {
        MaterialShaderParameter newParam;
        newParam.name_ = name;
        newParam.value_ = value;

        StringHash nameHash(name);
        shaderParameters_[nameHash] = newParam;

        if (nameHash == PSP_MATSPECCOLOR)
        {
            VariantType type = value.GetType();
            if (type == VAR_VECTOR3)
            {
                const Vector3& vec = value.GetVector3();
                specular_ = vec.x_ > 0.0f || vec.y_ > 0.0f || vec.z_ > 0.0f;
            }
            else if (type == VAR_VECTOR4)
            {
                const Vector4& vec = value.GetVector4();
                specular_ = vec.x_ > 0.0f || vec.y_ > 0.0f || vec.z_ > 0.0f;
            }
        }

        if (!batchedParameterUpdate_)
        {
            RefreshShaderParameterHash();
            RefreshMemoryUse();
        }
    }

    String Material::GetTextureUnitName(TextureUnit unit)
    {
        return textureUnitNames[unit];
    }

    Variant Material::ParseShaderParameterValue(const String& value)
    {
        String valueTrimmed = value.Trimmed();
        if (valueTrimmed.Length() && IsAlpha((unsigned)valueTrimmed[0]))
            return Variant(ToBool(valueTrimmed));
        else
            return ToVectorVariant(valueTrimmed);
    }
}