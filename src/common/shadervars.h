//
// Copyright (c) 2013-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// shadervars.h:
//  Types to represent GL variables (varyings, uniforms, etc)
//

#ifndef COMMON_SHADERVARIABLE_H_
#define COMMON_SHADERVARIABLE_H_

#include <string>
#include <vector>
#include <algorithm>

#include <GLES3/gl3.h>
#include <GLES2/gl2.h>

namespace gl
{

// Varying interpolation qualifier, see section 4.3.9 of the ESSL 3.00.4 spec
enum InterpolationType
{
    INTERPOLATION_SMOOTH,
    INTERPOLATION_CENTROID,
    INTERPOLATION_FLAT
};

// Uniform block layout qualifier, see section 4.3.8.3 of the ESSL 3.00.4 spec
enum BlockLayoutType
{
    BLOCKLAYOUT_STANDARD,
    BLOCKLAYOUT_PACKED,
    BLOCKLAYOUT_SHARED
};

// Base class for all variables defined in shaders, including Varyings, Uniforms, etc
struct ShaderVariable
{
    GLenum type;
    GLenum precision;
    std::string name;
    unsigned int arraySize;

    ShaderVariable(GLenum typeIn, GLenum precisionIn, const char *nameIn, unsigned int arraySizeIn);
    ShaderVariable(const ShaderVariable& other);
    void operator=(const ShaderVariable& other);

    bool isArray() const { return arraySize > 0; }
    unsigned int elementCount() const { return std::max(1u, arraySize); }

private:
    // To prevent large automatic inline code generation.
    bool operator==(const ShaderVariable& other) const;
};

// Uniform registers (and element indices) are assigned when outputting shader code
struct Uniform : public ShaderVariable
{
    std::vector<Uniform> fields;

    // HLSL-specific members
    unsigned int registerIndex;
    unsigned int elementIndex; // Offset within a register, for struct members

    Uniform(GLenum typeIn, GLenum precisionIn, const char *nameIn, unsigned int arraySizeIn,
            unsigned int registerIndexIn, unsigned int elementIndexIn);
    Uniform(const Uniform& other);
    void operator=(const Uniform& other);

    bool isStruct() const { return !fields.empty(); }
private:
    // To prevent large automatic inline code generation.
    bool operator==(const Uniform& other) const;
};

struct Attribute : public ShaderVariable
{
    int location;

    Attribute();
    Attribute(GLenum typeIn, GLenum precisionIn, const char *nameIn, unsigned int arraySizeIn, int locationIn);
    Attribute(const Attribute& other);
    void operator=(const Attribute& other);
private:
    // To prevent large automatic inline code generation.
    bool operator==(const Attribute& other) const;
};

struct InterfaceBlockField : public ShaderVariable
{
    bool isRowMajorMatrix;
    std::vector<InterfaceBlockField> fields;

    InterfaceBlockField(GLenum typeIn, GLenum precisionIn, const char *nameIn, unsigned int arraySizeIn, bool isRowMajorMatrix);
    InterfaceBlockField(const InterfaceBlockField& other);
    void operator=(const InterfaceBlockField& other);

    bool isStruct() const { return !fields.empty(); }
private:
    // To prevent large automatic inline code generation.
    bool operator==(const InterfaceBlockField& other) const;
};

struct Varying : public ShaderVariable
{
    InterpolationType interpolation;
    std::vector<Varying> fields;
    std::string structName;

    Varying(GLenum typeIn, GLenum precisionIn, const char *nameIn, unsigned int arraySizeIn, InterpolationType interpolationIn);
    Varying(const Varying& other);
    void operator=(const Varying& other);

    bool isStruct() const { return !fields.empty(); }
private:
    // To prevent large automatic inline code generation.
    bool operator==(const Varying& other) const;
};

struct BlockMemberInfo
{
    int offset;
    int arrayStride;
    int matrixStride;
    bool isRowMajorMatrix;

    static BlockMemberInfo getDefaultBlockInfo()
    {
        return BlockMemberInfo(-1, -1, -1, false);
    }

    // Borderline too large for inline. If someone adds another field, or makes one of the
    // existing fields more complicated, outline this and add explicit copy and assignment
    // constructors/operators.
    BlockMemberInfo(int offset, int arrayStride, int matrixStride, bool isRowMajorMatrix)
      : offset(offset),
        arrayStride(arrayStride),
        matrixStride(matrixStride),
        isRowMajorMatrix(isRowMajorMatrix)
    {}
};

typedef std::vector<BlockMemberInfo> BlockMemberInfoArray;

struct InterfaceBlock
{
    std::string name;
    unsigned int arraySize;
    size_t dataSize;
    BlockLayoutType layout;
    bool isRowMajorLayout;
    std::vector<InterfaceBlockField> fields;
    std::vector<BlockMemberInfo> blockInfo;

    // HLSL-specific members
    unsigned int registerIndex;

    InterfaceBlock(const char *name, unsigned int arraySize, unsigned int registerIndex);
    InterfaceBlock(const InterfaceBlock& other);
    void operator=(const InterfaceBlock& other);

private:
    // To prevent large automatic inline code generation.
    bool operator==(const InterfaceBlock& other) const;

};

}

#endif // COMMON_SHADERVARIABLE_H_
