//
// Created by luchu on 2022/2/23.
//

#include "Graphics/Camera.h"
#include "Graphics/Geometry.h"
#include "Graphics/Graphics.h"
#include "Graphics/GraphicsImpl.h"
#include "Graphics/Material.h"
#include "Graphics/Renderer.h"
#include "Graphics/ShaderVariation.h"
#include "Graphics/Technique.h"
#include "Graphics/Texture2D.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/Batch.h"
#include "Scene/Scene.h"


namespace My3D
{
    void Batch::Prepare(View* view, Camera* camera, bool setModelTransform, bool allowDepthWrite) const
    {
        if (!vertexShader_ || !pixelShader_)
            return;

        Graphics* graphics = view->GetGraphics();
        Renderer* renderer = view->GetRenderer();
        Node* cameraNode = camera ? camera->GetNode() : nullptr;
        Light* light = lightQueue_ ? lightQueue_->light_ : nullptr;
        Texture2D* shadowMap = lightQueue_ ? lightQueue_->shadowMap_ : nullptr;

    }
}
