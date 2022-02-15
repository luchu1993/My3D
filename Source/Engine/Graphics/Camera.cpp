//
// Created by luchu on 2022/2/15.
//

#include "Graphics/Camera.h"
#include "Core/Context.h"
#include "Graphics/Drawable.h"
#include "Scene/Node.h"


namespace My3D
{
    extern const char* SCENE_CATEGORY;

    static const char* fillModeNames[] =
    {
        "Solid",
        "Wireframe",
        "Point",
        nullptr
    };

    static const Matrix4 flipMatrix(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    Camera::Camera(Context *context)
        : Component(context)
        , viewDirty_(true)
        , projectionDirty_(true)
        , frustumDirty_(true)
        , orthographic_(false)
        , nearClip_(DEFAULT_NEARCLIP)
        , farClip_(DEFAULT_FARCLIP)
        , fov_(DEFAULT_CAMERA_FOV)
        , orthoSize_(DEFAULT_ORTHOSIZE)
        , aspectRatio_(1.0f)
        , zoom_(1.0f)
        , lodBias_(1.0f)
        , viewMask_(DEFAULT_VIEWMASK)
        , viewOverrideFlags_(VO_NONE)
        , fillMode_(FILL_SOLID)
        , projectionOffset_(Vector2::ZERO)
        , reflectionPlane_(Plane::UP)
        , clipPlane_(Plane::UP)
        , autoAspectRatio_(true)
        , flipVertical_(false)
        , useReflection_(false)
        , useClipping_(false)
        , customProjection_(false)
    {
        reflectionMatrix_ = reflectionPlane_.ReflectionMatrix();

        MY3D_ACCESSOR_ATTRIBUTE("Is Enabled", IsEnabled, SetEnabled, bool, true, AM_DEFAULT);
        MY3D_ACCESSOR_ATTRIBUTE("Near Clip", GetNearClip, SetNearClip, float, DEFAULT_NEARCLIP, AM_DEFAULT);
        MY3D_ACCESSOR_ATTRIBUTE("Far Clip", GetFarClip, SetFarClip, float, DEFAULT_FARCLIP, AM_DEFAULT);
        MY3D_ACCESSOR_ATTRIBUTE("FOV", GetFov, SetFov, float, DEFAULT_CAMERA_FOV, AM_DEFAULT);
        MY3D_ACCESSOR_ATTRIBUTE("Aspect Ratio", GetAspectRatio, SetAspectRatioInternal, float, 1.0f, AM_DEFAULT);
        MY3D_ENUM_ATTRIBUTE("Fill Mode", fillMode_, fillModeNames, FILL_SOLID, AM_DEFAULT);
        MY3D_ATTRIBUTE("Auto Aspect Ratio", bool, autoAspectRatio_, true, AM_DEFAULT);
    }

    Camera::~Camera() = default;

    void Camera::RegisterObject(Context *context)
    {
        context->RegisterFactory<Camera>(SCENE_CATEGORY);
    }

    void Camera::DrawDebugGeometry(DebugRenderer *debug, bool depthTest)
    {

    }

    void Camera::SetNearClip(float nearClip)
    {
        nearClip_ = Max(nearClip, M_MIN_NEARCLIP);
        frustumDirty_ = true;
        projectionDirty_ = true;
        MarkNetworkUpdate();
    }

    void Camera::SetFarClip(float farClip)
    {
        farClip_ = Max(farClip, M_MIN_NEARCLIP);
        frustumDirty_ = true;
        projectionDirty_ = true;
        MarkNetworkUpdate();
    }

    void Camera::SetFov(float fov)
    {
        fov_ = Clamp(fov, 0.0f, M_MAX_FOV);
        frustumDirty_ = true;
        projectionDirty_ = true;
        MarkNetworkUpdate();
    }

    void Camera::SetOrthoSize(float orthoSize)
    {
        orthoSize_ = orthoSize;
        aspectRatio_ = 1.0f;
        frustumDirty_ = true;
        projectionDirty_ = true;
        MarkNetworkUpdate();
    }

    void Camera::SetOrthoSize(const Vector2& orthoSize)
    {
        autoAspectRatio_ = false;
        orthoSize_ = orthoSize.y_;
        aspectRatio_ = orthoSize.x_ / orthoSize.y_;
        frustumDirty_ = true;
        projectionDirty_ = true;
        MarkNetworkUpdate();
    }

    void Camera::SetAspectRatio(float aspectRatio)
    {
        autoAspectRatio_ = false;
        SetAspectRatioInternal(aspectRatio);
    }

    void Camera::SetZoom(float zoom)
    {
        zoom_ = Max(zoom, M_EPSILON);
        frustumDirty_ = true;
        projectionDirty_ = true;
        MarkNetworkUpdate();
    }

    void Camera::SetLodBias(float bias)
    {
        lodBias_ = Max(bias, M_EPSILON);
        MarkNetworkUpdate();
    }


    void Camera::SetViewMask(unsigned mask)
    {
        viewMask_ = mask;
        MarkNetworkUpdate();
    }

    void Camera::SetViewOverrideFlags(ViewOverrideFlags flags)
    {
        viewOverrideFlags_ = flags;
        MarkNetworkUpdate();
    }

    void Camera::SetFillMode(FillMode mode)
    {
        fillMode_ = mode;
        MarkNetworkUpdate();
    }

    void Camera::SetOrthographic(bool enable)
    {
        orthographic_ = enable;
        frustumDirty_ = true;
        projectionDirty_ = true;
        MarkNetworkUpdate();
    }

    void Camera::SetAutoAspectRatio(bool enable)
    {
        autoAspectRatio_ = enable;
        MarkNetworkUpdate();
    }

    void Camera::SetProjectionOffset(const Vector2& offset)
    {
        projectionOffset_ = offset;
        projectionDirty_ = true;
        MarkNetworkUpdate();
    }

    void Camera::SetUseReflection(bool enable)
    {
        useReflection_ = enable;
        viewDirty_ = true;
        frustumDirty_ = true;
        MarkNetworkUpdate();
    }

    void Camera::SetReflectionPlane(const Plane& plane)
    {
        reflectionPlane_ = plane;
        reflectionMatrix_ = reflectionPlane_.ReflectionMatrix();
        viewDirty_ = true;
        frustumDirty_ = true;
        MarkNetworkUpdate();
    }

    void Camera::SetUseClipping(bool enable)
    {
        useClipping_ = enable;
        projectionDirty_ = true;
        MarkNetworkUpdate();
    }

    void Camera::SetClipPlane(const Plane& plane)
    {
        clipPlane_ = plane;
        MarkNetworkUpdate();
    }

    void Camera::SetFlipVertical(bool enable)
    {
        flipVertical_ = enable;
        MarkNetworkUpdate();
    }

    void Camera::SetProjection(const Matrix4& projection)
    {
        projection_ = projection;
        Matrix4 projInverse = projection_.Inverse();

        // Calculate the actual near & far clip from the custom matrix
        projNearClip_ = (projInverse * Vector3(0.0f, 0.0f, 0.0f)).z_;
        projFarClip_ = (projInverse * Vector3(0.0f, 0.0f, 1.0f)).z_;
        projectionDirty_ = false;
        autoAspectRatio_ = false;
        frustumDirty_ = true;
        customProjection_ = true;
        // Called due to autoAspectRatio changing state, the projection itself is not serialized
        MarkNetworkUpdate();
    }

    float Camera::GetNearClip() const
    {
        if (projectionDirty_)
            UpdateProjection();

        return projNearClip_;
    }

    float Camera::GetFarClip() const
    {
        if (projectionDirty_)
            UpdateProjection();

        return projFarClip_;
    }

    const Frustum& Camera::GetFrustum() const
    {
        // Use projection_ instead of GetProjection() so that Y-flip has no effect. Update first if necessary
        if (projectionDirty_)
            UpdateProjection();

        if (frustumDirty_)
        {
            if (customProjection_)
                frustum_.Define(projection_ * GetView());
            else
            {
                // If not using a custom projection, prefer calculating frustum from projection parameters instead of matrix
                // for better accuracy
                if (!orthographic_)
                    frustum_.Define(fov_, aspectRatio_, zoom_, GetNearClip(), GetFarClip(), GetEffectiveWorldTransform());
                else
                    frustum_.DefineOrtho(orthoSize_, aspectRatio_, zoom_, GetNearClip(), GetFarClip(), GetEffectiveWorldTransform());
            }

            frustumDirty_ = false;
        }

        return frustum_;
    }

    Matrix4 Camera::GetProjection() const
    {
        if (projectionDirty_)
            UpdateProjection();

        return flipVertical_ ? flipMatrix * projection_ : projection_;
    }

    float Camera::GetDistance(const Vector3& worldPos) const
    {
        if (!orthographic_)
        {
            const Vector3& cameraPos = node_ ? node_->GetWorldPosition() : Vector3::ZERO;
            return (worldPos - cameraPos).Length();
        }
        else
            return Abs((GetView() * worldPos).z_);
    }

    float Camera::GetHalfViewSize() const
    {
        if (!orthographic_)
            return tanf(fov_ * M_DEGTORAD * 0.5f) / zoom_;
        else
            return orthoSize_ * 0.5f / zoom_;
    }

    float Camera::GetDistanceSquared(const Vector3& worldPos) const
    {
        if (!orthographic_)
        {
            const Vector3& cameraPos = node_ ? node_->GetWorldPosition() : Vector3::ZERO;
            return (worldPos - cameraPos).LengthSquared();
        }
        else
        {
            float distance = (GetView() * worldPos).z_;
            return distance * distance;
        }
    }

    Matrix3x4 Camera::GetEffectiveWorldTransform() const
    {
        Matrix3x4 worldTransform = node_ ? Matrix3x4(node_->GetWorldPosition(), node_->GetWorldRotation(), 1.0f) : Matrix3x4::IDENTITY;
        return useReflection_ ? reflectionMatrix_ * worldTransform : worldTransform;
    }

    bool Camera::IsProjectionValid() const
    {
        return GetFarClip() > GetNearClip();
    }

    const Matrix3x4& Camera::GetView() const
    {
        if (viewDirty_)
        {
            // Note: view matrix is unaffected by node or parent scale
            view_ = GetEffectiveWorldTransform().Inverse();
            viewDirty_ = false;
        }

        return view_;
    }

    void Camera::SetAspectRatioInternal(float aspectRatio)
    {
        if (aspectRatio != aspectRatio_)
        {
            aspectRatio_ = aspectRatio;
            frustumDirty_ = true;
            projectionDirty_ = true;
        }
        MarkNetworkUpdate();
    }

    void Camera::SetOrthoSizeAttr(float orthoSize)
    {
        orthoSize_ = orthoSize;
        frustumDirty_ = true;
        projectionDirty_ = true;
        MarkNetworkUpdate();
    }

    void Camera::SetReflectionPlaneAttr(const Vector4& value)
    {
        SetReflectionPlane(Plane(value));
    }

    void Camera::SetClipPlaneAttr(const Vector4& value)
    {
        SetClipPlane(Plane(value));
    }

    Vector4 Camera::GetReflectionPlaneAttr() const
    {
        return reflectionPlane_.ToVector4();
    }

    Vector4 Camera::GetClipPlaneAttr() const
    {
        return clipPlane_.ToVector4();
    }

    void Camera::OnNodeSet(Node* node)
    {
        if (node)
            node->AddListener(this);
    }

    void Camera::OnMarkedDirty(Node* node)
    {
        frustumDirty_ = true;
        viewDirty_ = true;
    }

    void Camera::UpdateProjection() const
    {

    }
}
