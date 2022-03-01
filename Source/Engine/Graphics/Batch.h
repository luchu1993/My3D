//
// Created by luchu on 2022/2/23.
//

#pragma once

#include "Container/Ptr.h"
#include "Graphics/Drawable.h"
#include "Graphics/Material.h"
#include "Math/MathDefs.h"
#include "Math/Matrix3x4.h"
#include "Math/Rect.h"


namespace My3D
{
    class Camera;
    class Drawable;
    class Geometry;
    class Light;
    class Material;
    class Matrix3x4;
    class Pass;
    class ShaderVariation;
    class Texture2D;
    class VertexBuffer;
    class View;
    class Zone;
    struct LightBatchQueue;

    /// Queued 3D geometry draw call.
    struct Batch
    {
        /// Construct with defaults.
        Batch() = default;

        /// Construct from a drawable's source batch.
        explicit Batch(const SourceBatch& rhs)
            : distance_(rhs.distance_)
            , renderOrder_(rhs.material_ ? rhs.material_->GetRenderOrder() : DEFAULT_RENDER_ORDER)
            , isBase_(false)
            , geometry_(rhs.geometry_)
            , material_(rhs.material_)
            , worldTransform_(rhs.worldTransform_)
            , numWorldTransforms_(rhs.numWorldTransforms_)
            , instancingData_(rhs.instancingData_)
            , lightQueue_(nullptr)
            , geometryType_(rhs.geometryType_)
        {
        }

        /// Calculate state sorting key, which consists of base pass flag, light, pass and geometry.
        void CalculateSortKey();
        /// Prepare for rendering.
        void Prepare(View* view, Camera* camera, bool setModelTransform, bool allowDepthWrite) const;
        /// Prepare and draw.
        void Draw(View* view, Camera* camera, bool allowDepthWrite) const;

        /// State sorting key.
        unsigned long long sortKey_{};
        /// Distance from camera.
        float distance_{};
        /// 8-bit render order modifier from material.
        unsigned char renderOrder_{};
        /// 8-bit light mask for stencil marking in deferred rendering.
        unsigned char lightMask_{};
        /// Base batch flag. This tells to draw the object fully without light optimizations.
        bool isBase_{};
        /// Geometry.
        Geometry* geometry_{};
        /// Material.
        Material* material_{};
        /// World transform(s). For a skinned model, these are the bone transforms.
        const Matrix3x4* worldTransform_{};
        /// Number of world transforms.
        unsigned numWorldTransforms_{};
        /// Per-instance data. If not null, must contain enough data to fill instancing buffer.
        void* instancingData_{};
        /// Zone.
        Zone* zone_{};
        /// Light properties.
        LightBatchQueue* lightQueue_{};
        /// Material pass.
        Pass* pass_{};
        /// Vertex shader.
        ShaderVariation* vertexShader_{};
        /// Pixel shader.
        ShaderVariation* pixelShader_{};
        /// %Geometry type.
        GeometryType geometryType_{};
    };

    /// Data for one geometry instance.
    struct InstanceData
    {
        /// Construct undefined.
        InstanceData() = default;

        /// Construct with transform, instancing data and distance.
        InstanceData(const Matrix3x4* worldTransform, const void* instancingData, float distance)
            : worldTransform_(worldTransform)
            , instancingData_(instancingData)
            , distance_(distance)
        {
        }
        /// World transform.
        const Matrix3x4* worldTransform_{};
        /// Instancing data buffer.
        const void* instancingData_{};
        /// Distance from camera.
        float distance_{};
    };
}
