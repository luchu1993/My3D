//
// Created by luchu on 2022/2/20.
//

#pragma once

#include "Core/StringHash.h"
#include "Container/String.h"
#include "Container/HashMap.h"
#include "Container/RefCounted.h"
#include "Container/ArrayPtr.h"
#include "Graphics/GPUObject.h"
#include "Graphics/GraphicsDefs.h"


namespace My3D
{
    class ConstantBuffer;
    class Shader;

    /// Shader parameter definition
    struct ShaderParameter
    {
        /// Construct with defaults.
        ShaderParameter() = default;
        /// Construct with name, glType and location, leaving the remaining attributes zero-initialized (used only in OpenGL).
        ShaderParameter(const String& name, unsigned glType, int location);
        /// Construct with type, name, offset, size, and buffer, leaving the remaining attributes zero-initialized (used only in Direct3D11).
        ShaderParameter(ShaderType type, const String& name, unsigned offset, unsigned size, unsigned buffer);
        /// Construct with type, name, register, and register count, leaving the remaining attributes zero-initialized (used only in Direct3D9).
        ShaderParameter(ShaderType type, const String& name, unsigned reg, unsigned regCount);

        /// Shader type.
        ShaderType type_{};
        /// Name of the parameter.
        String name_{};

        union
        {
            /// Offset in constant buffer.
            unsigned offset_;
            /// OpenGL uniform location.
            int location_;
        };

        union
        {
            /// Parameter size. Used only on Direct3D11 to calculate constant buffer size.
            unsigned size_;
            /// Parameter OpenGL type.
            unsigned glType_;
        };

        /// Constant buffer index. Only used on Direct3D11.
        unsigned buffer_{};
        /// Constant buffer pointer. Defined only in shader programs.
        ConstantBuffer* bufferPtr_{};
    };

    /// Vertex or pixel shader on the GPU.
    class MY3D_API ShaderVariation : public RefCounted, public GPUObject
    {
    public:
        /// Construct.
        ShaderVariation(Shader* owner, ShaderType type);
        /// Destruct.
        ~ShaderVariation() override;

        /// Mark the GPU resource destroyed on graphics context destruction.
        void OnDeviceLost() override;
        /// Release the shader.
        void Release() override;

        /// Compile the shader. Return true if successful.
        bool Create();
        /// Set name.
        void SetName(const String& name);
        /// Set defines.
        void SetDefines(const String& defines);

        /// Return the owner resource.
        Shader* GetOwner() const;

        /// Return shader type.
        ShaderType GetShaderType() const { return type_; }

        /// Return shader name.
        const String& GetName() const { return name_; }

        /// Return full shader name.
        String GetFullName() const { return name_ + "(" + defines_ + ")"; }

        /// Return whether uses a parameter. Not applicable on OpenGL, where this information is contained in ShaderProgram instead.
        bool HasParameter(StringHash param) const { return parameters_.Contains(param); }

        /// Return whether uses a texture unit (only for pixel shaders). Not applicable on OpenGL, where this information is contained in ShaderProgram instead.
        bool HasTextureUnit(TextureUnit unit) const { return useTextureUnits_[unit]; }

        /// Return all parameter definitions. Not applicable on OpenGL, where this information is contained in ShaderProgram instead.
        const HashMap<StringHash, ShaderParameter>& GetParameters() const { return parameters_; }

        /// Return vertex element hash.
        unsigned long long GetElementHash() const { return elementHash_; }

        /// Return shader bytecode. Stored persistently on Direct3D11 only.
        const PODVector<unsigned char>& GetByteCode() const { return byteCode_; }

        /// Return defines.
        const String& GetDefines() const { return defines_; }

        /// Return compile error/warning string.
        const String& GetCompilerOutput() const { return compilerOutput_; }

        /// Return constant buffer data sizes.
        const unsigned* GetConstantBufferSizes() const { return &constantBufferSizes_[0]; }

        /// Return defines with the CLIPPLANE define appended. Used internally on Direct3D11 only, will be empty on other APIs.
        const String& GetDefinesClipPlane() { return definesClipPlane_; }

        /// D3D11 vertex semantic names. Used internally.
        static const char* elementSemanticNames[];

    private:
        /// Load bytecode from a file. Return true if successful.
        bool LoadByteCode(const String& binaryShaderName);
        /// Compile from source. Return true if successful.
        bool Compile();
        /// Inspect the constant parameters and input layout (if applicable) from the shader bytecode.
        void ParseParameters(unsigned char* bufData, unsigned bufSize);
        /// Save bytecode to a file.
        void SaveByteCode(const String& binaryShaderName);
        /// Calculate constant buffer sizes from parameters.
        void CalculateConstantBufferSizes();

        /// Shader this variation belongs to.
        WeakPtr<Shader> owner_;
        /// Shader type.
        ShaderType type_;
        /// Vertex element hash for vertex shaders. Zero for pixel shaders. Note that hashing is different than vertex buffers.
        unsigned long long elementHash_{};
        /// Shader parameters.
        HashMap<StringHash, ShaderParameter> parameters_;
        /// Texture unit use flags.
        bool useTextureUnits_[MAX_TEXTURE_UNITS]{};
        /// Constant buffer sizes. 0 if a constant buffer slot is not in use.
        unsigned constantBufferSizes_[MAX_SHADER_PARAMETER_GROUPS]{};
        /// Shader bytecode. Needed for inspecting the input signature and parameters. Not used on OpenGL.
        PODVector<unsigned char> byteCode_;
        /// Shader name.
        String name_;
        /// Defines to use in compiling.
        String defines_;
        /// Defines to use in compiling + CLIPPLANE define appended. Used only on Direct3D11.
        String definesClipPlane_;
        /// Shader compile error string.
        String compilerOutput_;
    };
}