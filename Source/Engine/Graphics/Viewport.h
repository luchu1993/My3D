//
// Created by luchu on 2022/1/22.
//

#pragma once

#include "Core/Object.h"
#include "Container/Ptr.h"
#include "Math/Ray.h"
#include "Math/Rect.h"
#include "Math/Vector2.h"


namespace My3D
{
    class Camera;
    class RenderPath;
    class Scene;
    class XMLFile;
    class View;

    /// Viewport definition either for a render surface or the backbuffer.
    class MY3D_API Viewport : public Object
    {
        MY3D_OBJECT(Viewport, Object);
    public:
        /// Construct
        explicit Viewport(Context* context);
        /// Construct with a full rectangle
        Viewport(Context* context, Scene* scene, Camera* camera, RenderPath* renderPath = nullptr);
        /// Destruct
        ~Viewport() override;

    private:
        /// Scene pointer
        WeakPtr<Scene> scene_;
        /// Camera pointer
        WeakPtr<Camera> camera_;
        /// Culling camera pointer
        WeakPtr<Camera> cullCamera_;
        /// Viewport rectangle
        IntRect rect_;
        /// Internal rendering structure.
        SharedPtr<View> view_;
        /// Debug draw flag.
        bool drawDebug_;
    };
}
