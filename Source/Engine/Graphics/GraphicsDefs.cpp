//
// Created by luchu on 2022/1/21.
//

#include "Graphics/GraphicsDefs.h"
#include "Math/Vector3.h"

namespace My3D
{
    extern MY3D_API const StringHash VSP_AMBIENTSTARTCOLOR("AmbientStartColor");
    extern MY3D_API const StringHash VSP_AMBIENTENDCOLOR("AmbientEndColor");
    extern MY3D_API const StringHash VSP_BILLBOARDROT("BillboardRot");
    extern MY3D_API const StringHash VSP_CAMERAPOS("CameraPos");
    extern MY3D_API const StringHash VSP_CLIPPLANE("ClipPlane");
    extern MY3D_API const StringHash VSP_NEARCLIP("NearClip");
    extern MY3D_API const StringHash VSP_FARCLIP("FarClip");
    extern MY3D_API const StringHash VSP_DEPTHMODE("DepthMode");
    extern MY3D_API const StringHash VSP_DELTATIME("DeltaTime");
    extern MY3D_API const StringHash VSP_ELAPSEDTIME("ElapsedTime");
    extern MY3D_API const StringHash VSP_FRUSTUMSIZE("FrustumSize");
    extern MY3D_API const StringHash VSP_GBUFFEROFFSETS("GBufferOffsets");
    extern MY3D_API const StringHash VSP_LIGHTDIR("LightDir");
    extern MY3D_API const StringHash VSP_LIGHTPOS("LightPos");
    extern MY3D_API const StringHash VSP_NORMALOFFSETSCALE("NormalOffsetScale");
    extern MY3D_API const StringHash VSP_MODEL("Model");
    extern MY3D_API const StringHash VSP_VIEW("View");
    extern MY3D_API const StringHash VSP_VIEWINV("ViewInv");
    extern MY3D_API const StringHash VSP_VIEWPROJ("ViewProj");
    extern MY3D_API const StringHash VSP_UOFFSET("UOffset");
    extern MY3D_API const StringHash VSP_VOFFSET("VOffset");
    extern MY3D_API const StringHash VSP_ZONE("Zone");
    extern MY3D_API const StringHash VSP_LIGHTMATRICES("LightMatrices");
    extern MY3D_API const StringHash VSP_SKINMATRICES("SkinMatrices");
    extern MY3D_API const StringHash VSP_VERTEXLIGHTS("VertexLights");
    extern MY3D_API const StringHash PSP_AMBIENTCOLOR("AmbientColor");
    extern MY3D_API const StringHash PSP_CAMERAPOS("CameraPosPS");
    extern MY3D_API const StringHash PSP_DELTATIME("DeltaTimePS");
    extern MY3D_API const StringHash PSP_DEPTHRECONSTRUCT("DepthReconstruct");
    extern MY3D_API const StringHash PSP_ELAPSEDTIME("ElapsedTimePS");
    extern MY3D_API const StringHash PSP_FOGCOLOR("FogColor");
    extern MY3D_API const StringHash PSP_FOGPARAMS("FogParams");
    extern MY3D_API const StringHash PSP_GBUFFERINVSIZE("GBufferInvSize");
    extern MY3D_API const StringHash PSP_LIGHTCOLOR("LightColor");
    extern MY3D_API const StringHash PSP_LIGHTDIR("LightDirPS");
    extern MY3D_API const StringHash PSP_LIGHTPOS("LightPosPS");
    extern MY3D_API const StringHash PSP_NORMALOFFSETSCALE("NormalOffsetScalePS");
    extern MY3D_API const StringHash PSP_MATDIFFCOLOR("MatDiffColor");
    extern MY3D_API const StringHash PSP_MATEMISSIVECOLOR("MatEmissiveColor");
    extern MY3D_API const StringHash PSP_MATENVMAPCOLOR("MatEnvMapColor");
    extern MY3D_API const StringHash PSP_MATSPECCOLOR("MatSpecColor");
    extern MY3D_API const StringHash PSP_NEARCLIP("NearClipPS");
    extern MY3D_API const StringHash PSP_FARCLIP("FarClipPS");
    extern MY3D_API const StringHash PSP_SHADOWCUBEADJUST("ShadowCubeAdjust");
    extern MY3D_API const StringHash PSP_SHADOWDEPTHFADE("ShadowDepthFade");
    extern MY3D_API const StringHash PSP_SHADOWINTENSITY("ShadowIntensity");
    extern MY3D_API const StringHash PSP_SHADOWMAPINVSIZE("ShadowMapInvSize");
    extern MY3D_API const StringHash PSP_SHADOWSPLITS("ShadowSplits");
    extern MY3D_API const StringHash PSP_LIGHTMATRICES("LightMatricesPS");
    extern MY3D_API const StringHash PSP_VSMSHADOWPARAMS("VSMShadowParams");
    extern MY3D_API const StringHash PSP_ROUGHNESS("Roughness");
    extern MY3D_API const StringHash PSP_METALLIC("Metallic");
    extern MY3D_API const StringHash PSP_LIGHTRAD("LightRad");
    extern MY3D_API const StringHash PSP_LIGHTLENGTH("LightLength");
    extern MY3D_API const StringHash PSP_ZONEMIN("ZoneMin");
    extern MY3D_API const StringHash PSP_ZONEMAX("ZoneMax");

    extern MY3D_API const Vector3 DOT_SCALE(1 / 3.0f, 1 / 3.0f, 1 / 3.0f);

    extern MY3D_API const unsigned ELEMENT_TYPESIZES[] =
    {
        sizeof(int),
        sizeof(float),
        2 * sizeof(float),
        3 * sizeof(float),
        4 * sizeof(float),
        sizeof(unsigned),
        sizeof(unsigned)
    };
}

