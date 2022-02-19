//
// Created by luchu on 2022/2/19.
//

#pragma once

#include "Graphics/GraphicsDefs.h"
#include "Resource/Resource.h"


namespace My3D
{
    class ShaderVariation;

    /// Lighting mode of a pass.
    enum PassLightingMode
    {
        LIGHTING_UNLIT = 0,
        LIGHTING_PERVERTEX,
        LIGHTING_PERPIXEL
    };

    /// Material rendering pass, which defines shaders and render state.
    class MY3D_API Pass : public RefCounted
    {
    public:
        /// Construct.
        explicit Pass(const String& name);
        /// Destruct.
        ~Pass() override;

    private:
        /// Pass index.
        unsigned index_;
        /// Blend mode.
        BlendMode blendMode_;
        /// Culling mode.
        CullMode cullMode_;
        /// Depth compare mode.
        CompareMode depthTestMode_;
    };

    /// Material technique. Consists of several passes.
    class MY3D_API Technique : public Resource
    {
        MY3D_OBJECT(Technique, Resource);
        friend class Renderer;

    public:
        /// Construct.
        explicit Technique(Context* context);
        /// Destruct.
        ~Technique() override;
        /// Register object factory.
        static void RegisterObject(Context* context);

        /// Return a pass type index by name. Allocate new if not used yet.
        static unsigned GetPassIndex(const String& passName);

        /// Index for base pass. Initialized once GetPassIndex() has been called for the first time.
        static unsigned basePassIndex;
        /// Index for alpha pass. Initialized once GetPassIndex() has been called for the first time.
        static unsigned alphaPassIndex;
        /// Index for prepass material pass. Initialized once GetPassIndex() has been called for the first time.
        static unsigned materialPassIndex;
        /// Index for deferred G-buffer pass. Initialized once GetPassIndex() has been called for the first time.
        static unsigned deferredPassIndex;
        /// Index for per-pixel light pass. Initialized once GetPassIndex() has been called for the first time.
        static unsigned lightPassIndex;
        /// Index for lit base pass. Initialized once GetPassIndex() has been called for the first time.
        static unsigned litBasePassIndex;
        /// Index for lit alpha pass. Initialized once GetPassIndex() has been called for the first time.
        static unsigned litAlphaPassIndex;
        /// Index for shadow pass. Initialized once GetPassIndex() has been called for the first time.
        static unsigned shadowPassIndex;

    private:
        /// Require desktop GPU flag.
        bool isDesktop_;
        /// Cached desktop GPU support flag.
        bool desktopSupport_;
        /// Passes.
        Vector<SharedPtr<Pass> > passes_;
        /// Cached clones with added shader compilation defines.
        HashMap<Pair<StringHash, StringHash>, SharedPtr<Technique> > cloneTechniques_;
        /// Pass index assignments.
        static HashMap<String, unsigned> passIndices;
    };
}