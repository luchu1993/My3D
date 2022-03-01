//
// Created by luchu on 2022/2/25.
//

#pragma once

#include "Math/Color.h"
#include "Graphics/Drawable.h"
#include "Math/Frustum.h"
#include "Graphics/Texture.h"


namespace My3D
{
    class Camera;
    class LightBatchQueue;

    // Light types
    enum LightType
    {
        LIGHT_DIRECTIONAL = 0,
        LIGHT_SPOT,
        LIGHT_POINT
    };

    static const float SHADOW_MIN_QUANTIZE = 0.1f;
    static const float SHADOW_MIN_VIEW = 1.0f;
    static const int MAX_LIGHT_SPLITS = 6;

    static const unsigned MAX_CASCADE_SPLITS = 4;

    /// Depth bias parameters. Used both by lights (for shadow mapping) and materials.
    struct MY3D_API BiasParameters
    {
        /// Construct undefined.
        BiasParameters() = default;

        /// Construct with initial values.
        BiasParameters(float constantBias, float slopeScaledBias, float normalOffset = 0.0f)
            : constantBias_(constantBias)
            , slopeScaledBias_(slopeScaledBias)
            , normalOffset_(normalOffset)
        {
        }

        /// Validate parameters.
        void Validate();

        /// Constant bias.
        float constantBias_;
        /// Slope scaled bias.
        float slopeScaledBias_;
        /// Normal offset multiplier.
        float normalOffset_;
    };

    /// Cascaded shadow map parameters.
    struct MY3D_API CascadeParameters
    {
        /// Construct undefined.
        CascadeParameters() = default;

        CascadeParameters(float split1, float split2, float split3, float split4, float fadeStart, float biasAutoAdjust = 1.0f)
            : fadeStart_(fadeStart)
            , biasAutoAdjust_(biasAutoAdjust)
        {
            splits_[0] = split1;
            splits_[1] = split2;
            splits_[2] = split3;
            splits_[3] = split4;
        }

        /// Validate parameters.
        void Validate();

        /// Return shadow maximum range.
        float GetShadowRange() const
        {
            float ret = 0.0f;
            for (unsigned i = 0; i < MAX_CASCADE_SPLITS; ++i)
                ret = Max(ret, splits_[i]);

            return ret;
        }
        /// Far clip values of the splits.
        Vector4 splits_;
        /// The point relative to the total shadow range where shadow fade begins (0.0 - 1.0).
        float fadeStart_{};
        /// Automatic depth bias adjustment strength.
        float biasAutoAdjust_{};
    };

    /// Shadow map focusing parameters.
    struct MY3D_API FocusParameters
    {
        /// Construct undefined.
        FocusParameters() = default;

        /// Construct with initial values.
        FocusParameters(bool focus, bool nonUniform, bool autoSize, float quantize, float minView)
            : focus_(focus)
            , nonUniform_(nonUniform)
            , autoSize_(autoSize)
            , quantize_(quantize)
            , minView_(minView)
        {
        }

        /// Validate parameters.
        void Validate();

        /// Focus flag.
        bool focus_;
        /// Non-uniform focusing flag.
        bool nonUniform_;
        /// Auto-size (reduce resolution when far away) flag.
        bool autoSize_;
        /// Focus quantization.
        float quantize_;
        /// Minimum view size.
        float minView_;
    };

    /// Light Component
    class MY3D_API Light : public Component
    {
        MY3D_OBJECT(Light, Drawable)

    public:
        /// Construct.
        explicit Light(Context* context);
        /// Destruct.
        ~Light() override;
        /// Register object factory. Drawable must be registered first.
        static void RegisterObject(Context* context);

    private:
        /// Validate shadow focus.
        void ValidateShadowFocus() { shadowFocus_.Validate(); }
        /// Validate shadow cascade.
        void ValidateShadowCascade() { shadowCascade_.Validate(); }
        /// Validate shadow bias.
        void ValidateShadowBias() { shadowBias_.Validate(); }
        /// Light type.
        LightType lightType_;
        /// Color.
        Color color_;
        /// Light temperature.
        float temperature_;
        /// Radius of the light source. If above 0 it will turn the light into an area light.  Works only with PBR shaders.
        float lightRad_;
        /// Length of the light source. If above 0 and radius is above 0 it will create a tube light. Works only with PBR shaders.
        float lightLength_;
        /// Shadow depth bias parameters.
        BiasParameters shadowBias_;
        /// Directional light cascaded shadow parameters.
        CascadeParameters shadowCascade_;
        /// Shadow map focus parameters.
        FocusParameters shadowFocus_;
        /// Custom world transform for the light volume.
        Matrix3x4 volumeTransform_;
        /// Range attenuation texture.
        SharedPtr<Texture> rampTexture_;
        /// Spotlight attenuation texture.
        SharedPtr<Texture> shapeTexture_;
        /// Light queue.
        LightBatchQueue* lightQueue_;
        /// Specular intensity.
        float specularIntensity_;
        /// Brightness multiplier.
        float brightness_;
        /// Range.
        float range_;
        /// Spotlight field of view.
        float fov_;
        /// Spotlight aspect ratio.
        float aspectRatio_;
        /// Fade start distance.
        float fadeDistance_;
        /// Shadow fade start distance.
        float shadowFadeDistance_;
        /// Light intensity in shadow.
        float shadowIntensity_;
        /// Shadow resolution.
        float shadowResolution_;
        /// Shadow camera near/far clip distance ratio.
        float shadowNearFarRatio_;
        /// Directional shadow max. extrusion distance.
        float shadowMaxExtrusion_;
        /// Per-vertex lighting flag.
        bool perVertex_;
        /// Use physical light values flag.
        bool usePhysicalValues_;
    };
}