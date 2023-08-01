//
// Copyright 2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_UNIFORM_H_
#define LIBANGLE_UNIFORM_H_

#include <string>
#include <vector>

#include "angle_gl.h"
#include "common/MemoryBuffer.h"
#include "common/debug.h"
#include "common/utilities.h"
#include "compiler/translator/blocklayout.h"
#include "libANGLE/angletypes.h"

namespace gl
{
struct UniformTypeInfo;
struct UsedUniform;

struct ActiveVariable
{
    ActiveVariable();
    ActiveVariable(const ActiveVariable &rhs);
    virtual ~ActiveVariable();

    ActiveVariable &operator=(const ActiveVariable &rhs);

    ShaderType getFirstActiveShaderType() const
    {
        return static_cast<ShaderType>(ScanForward(mActiveUseBits.bits()));
    }
    void setActive(ShaderType shaderType, bool used, uint32_t id);
    void unionReferencesWith(const ActiveVariable &other);
    bool isActive(ShaderType shaderType) const
    {
        ASSERT(shaderType != ShaderType::InvalidEnum);
        return mActiveUseBits[shaderType];
    }
    const ShaderMap<uint32_t> &getIds() const { return mIds; }
    uint32_t getId(ShaderType shaderType) const { return mIds[shaderType]; }
    ShaderBitSet activeShaders() const { return mActiveUseBits; }
    GLuint activeShaderCount() const { return static_cast<GLuint>(mActiveUseBits.count()); }

  private:
    ShaderBitSet mActiveUseBits;
    // The id of a linked variable in each shader stage.  This id originates from
    // sh::ShaderVariable::id or sh::InterfaceBlock::id
    ShaderMap<uint32_t> mIds;
};

// Helper struct representing a single shader uniform
struct LinkedUniform
{
    LinkedUniform();
    LinkedUniform(GLenum typeIn,
                  GLenum precisionIn,
                  const std::string &nameIn,
                  const std::vector<unsigned int> &arraySizesIn,
                  const int bindingIn,
                  const int offsetIn,
                  const int locationIn,
                  const int bufferIndexIn,
                  const sh::BlockMemberInfo &blockInfoIn);
    LinkedUniform(const LinkedUniform &other);
    LinkedUniform(const UsedUniform &usedUniform);
    LinkedUniform &operator=(const LinkedUniform &other);
    ~LinkedUniform();

    bool isSampler() const { return typeInfo->isSampler; }
    bool isImage() const { return typeInfo->isImageType; }
    bool isAtomicCounter() const { return IsAtomicCounterType(type); }
    bool isInDefaultBlock() const { return bufferIndex == -1; }
    size_t getElementSize() const { return typeInfo->externalSize; }
    size_t getElementComponents() const { return typeInfo->componentCount; }

    bool isArrayOfArrays() const { return arraySizes.size() >= 2u; }
    bool isArray() const { return !arraySizes.empty(); }
    unsigned int getArraySizeProduct() const { return gl::ArraySizeProduct(arraySizes); }
    // Array size 0 means not an array when passed to or returned from these functions.
    // Note that setArraySize() is deprecated and should not be used inside ANGLE.
    unsigned int getOutermostArraySize() const { return isArray() ? arraySizes.back() : 0; }
    // This function should only be used with variables that are of a basic type or an array of a
    // basic type. Shader interface variables that are enumerated according to rules in GLES 3.1
    // spec section 7.3.1.1 page 77 are fine. For those variables the return value should match the
    // ARRAY_SIZE value that can be queried through the API.
    unsigned int getBasicTypeElementCount() const
    {
        // GLES 3.1 Nov 2016 section 7.3.1.1 page 77 specifies that a separate entry should be
        // generated for each array element when dealing with an array of arrays or an array of
        // structs.
        ASSERT(!isArrayOfArrays());
        ASSERT(!isStruct || !isArray());

        // GLES 3.1 Nov 2016 page 82.
        if (isArray())
        {
            return getOutermostArraySize();
        }
        return 1u;
    }

    unsigned int getExternalSize() const;

    // All of the shader's variables are described using nested data
    // structures. This is needed in order to disambiguate similar looking
    // types, such as two structs containing the same fields, but in
    // different orders. "findInfoByMappedName" provides an easy query for
    // users to dive into the data structure and fetch the unique variable
    // instance corresponding to a dereferencing chain of the top-level
    // variable.
    // Given a mapped name like 'a[0].b.c[0]', return the ShaderVariable
    // that defines 'c' in |leafVar|, and the original name 'A[0].B.C[0]'
    // in |originalName|, based on the assumption that |this| defines 'a'.
    // If no match is found, return false.
    bool findInfoByMappedName(const std::string &mappedFullName,
                              const sh::ShaderVariable **leafVar,
                              std::string *originalFullName) const;
    bool isBuiltIn() const { return gl::IsBuiltInName(name); }

    // Offset of this variable in parent arrays. In case the parent is an array of arrays, the
    // offset is outerArrayElement * innerArraySize + innerArrayElement.
    // For example, if there's a variable declared as size 3 array of size 4 array of int:
    //   int a[3][4];
    // then the flattenedOffsetInParentArrays of a[2] would be 2.
    // and flattenedOffsetInParentArrays of a[2][1] would be 2*4 + 1 = 9.
    int parentArrayIndex() const
    {
        return hasParentArrayIndex() ? flattenedOffsetInParentArrays : 0;
    }
    int getFlattenedOffsetInParentArrays() const { return flattenedOffsetInParentArrays; }
    void setParentArrayIndex(int indexIn) { flattenedOffsetInParentArrays = indexIn; }

    bool hasParentArrayIndex() const { return flattenedOffsetInParentArrays != -1; }
    // InterfaceBlockField
    // Decide whether two InterfaceBlock fields are the same at shader
    // link time, assuming they are from consecutive shader stages.
    // See GLSL ES Spec 3.00.3, sec 4.3.7.
    bool isSameInterfaceBlockFieldAtLinkTime(const sh::ShaderVariable &other) const;

    bool isSameVariableAtLinkTime(const sh::ShaderVariable &other,
                                  bool matchPrecision,
                                  bool matchName) const;

    ShaderType getFirstActiveShaderType() const
    {
        return activeVariable.getFirstActiveShaderType();
    }
    void setActive(ShaderType shaderType, bool used, uint32_t _id)
    {
        activeVariable.setActive(shaderType, used, _id);
    }
    bool isActive(ShaderType shaderType) const { return activeVariable.isActive(shaderType); }
    const ShaderMap<uint32_t> &getIds() const { return activeVariable.getIds(); }
    uint32_t getId(ShaderType shaderType) const { return activeVariable.getId(shaderType); }
    ShaderBitSet activeShaders() const { return activeVariable.activeShaders(); }
    GLuint activeShaderCount() const { return activeVariable.activeShaderCount(); }

    GLenum type;
    GLenum precision;
    std::string name;
    // Only used by GL backend
    std::string mappedName;

    // Used to make an array type. Outermost array size is stored at the end of the vector.
    std::vector<unsigned int> arraySizes;

    union
    {
        struct
        {
            // Static use means that the variable is accessed somewhere in the shader source.
            uint32_t staticUse;
            // A variable is active unless the compiler determined that it is not accessed by the
            // shader. All active variables are statically used, but not all statically used
            // variables are necessarily active. GLES 3.0.5 section 2.12.6. GLES 3.1 section 7.3.1.
            uint32_t active;

            uint32_t isStruct;
            uint32_t rasterOrdered;
            uint32_t readonly;
            uint32_t writeonly;

            // From EXT_shader_framebuffer_fetch / KHR_blend_equation_advanced
            uint32_t isFragmentInOut;

            // If the variable is a sampler that has ever been statically used with texelFetch
            uint32_t texelFetchStaticUse;
        };
        uint32_t flagBitsAsUint;
    };

    // VariableWithLocation
    int location;

    // Uniform
    int binding;
    GLenum imageUnitFormat;
    int offset;

    // Id of the variable in the shader.  Currently used by the SPIR-V output to communicate the
    // SPIR-V id of the variable.  This value is only set for variables that the SPIR-V transformer
    // needs to know about, i.e. active variables, excluding non-zero array elements etc.
    uint32_t id;

    int flattenedOffsetInParentArrays;

    ActiveVariable activeVariable;

    const UniformTypeInfo *typeInfo;

    // Identifies the containing buffer backed resource -- interface block or atomic counter buffer.
    int bufferIndex;
    sh::BlockMemberInfo blockInfo;
    unsigned int outerArraySizeProduct;
    unsigned int outerArrayOffset;
};

struct BufferVariable : public sh::ShaderVariable, public ActiveVariable
{
    BufferVariable();
    BufferVariable(GLenum type,
                   GLenum precision,
                   const std::string &name,
                   const std::vector<unsigned int> &arraySizes,
                   const int bufferIndex,
                   const sh::BlockMemberInfo &blockInfo);
    ~BufferVariable() override;

    int bufferIndex;
    sh::BlockMemberInfo blockInfo;

    int topLevelArraySize;
};

// Parent struct for atomic counter, uniform block, and shader storage block buffer, which all
// contain a group of shader variables, and have a GL buffer backed.
struct ShaderVariableBuffer : public ActiveVariable
{
    ShaderVariableBuffer();
    ShaderVariableBuffer(const ShaderVariableBuffer &other);
    ~ShaderVariableBuffer() override;
    int numActiveVariables() const;

    int binding;
    unsigned int dataSize;
    std::vector<unsigned int> memberIndexes;
};

using AtomicCounterBuffer = ShaderVariableBuffer;

// Helper struct representing a single shader interface block
struct InterfaceBlock : public ShaderVariableBuffer
{
    InterfaceBlock();
    InterfaceBlock(const std::string &nameIn,
                   const std::string &mappedNameIn,
                   bool isArrayIn,
                   bool isReadOnlyIn,
                   unsigned int arrayElementIn,
                   unsigned int firstFieldArraySizeIn,
                   int bindingIn);
    InterfaceBlock(const InterfaceBlock &other);

    std::string nameWithArrayIndex() const;
    std::string mappedNameWithArrayIndex() const;

    std::string name;
    std::string mappedName;
    bool isArray;
    // Only valid for SSBOs, specifies whether it has the readonly qualifier.
    bool isReadOnly;
    unsigned int arrayElement;
    unsigned int firstFieldArraySize;
};

}  // namespace gl

#endif  // LIBANGLE_UNIFORM_H_
