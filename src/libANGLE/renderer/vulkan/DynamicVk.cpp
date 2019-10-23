//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DynamicVK.cpp: Implementation for link and run-time GL generation
//

#include "libANGLE/renderer/vulkan/DynamicVk.h"

#include <unordered_map>

#include "libANGLE/VaryingPacking.h"
#include "libANGLE/formatutils.h"

using namespace gl;

namespace rx
{

DynamicVk::DynamicVk() : mGeometryShader("") {}

DynamicVk::~DynamicVk()
{
    mGeometryShader.clear();
}

constexpr const char *componentString[][4] = {{"uint", "uvec2", "uvec3", "uvec4"},
                                              {"int", "ivec2", "ivec3", "ivec4"},
                                              {"float", "vec2", "vec3", "vec4"}};

constexpr const char *kStructDeclarePrefix = "st_";
constexpr const char *kFieldVariblePrefix  = "f";

constexpr const char *kInputVarying  = "in";
constexpr const char *kOutputVarying = "out";

constexpr const char *kFlatInterploation     = "flat";
constexpr const char *kCentroidInterploation = "centroid";
constexpr const char *kEmptyString           = "";

struct StructField
{
    std::string parentStructName;
    std::string componentName;
    std::string name;
    sh::InterpolationType interpolation;
    int location;
    size_t registerIndex;
    int index;
    int byteSize;
};

void GLSLComponentTypeString(std::ostringstream &glslStream,
                             GLenum componentType,
                             int componentCount)
{
    std::unordered_map<GLenum, int> typeIndexMap = {
        {GL_UNSIGNED_INT, 0},      {GL_INT, 1},   {GL_UNSIGNED_NORMALIZED, 2},
        {GL_SIGNED_NORMALIZED, 2}, {GL_FLOAT, 2},
    };

    ASSERT(componentCount > 0 && componentCount < 5);
    ASSERT(typeIndexMap.find(componentType) != typeIndexMap.end());
    glslStream << componentString[typeIndexMap[componentType]][componentCount - 1];
}

void generatePrimitiveLayout(std::ostringstream &glslStream,
                             const std::string inType,
                             const std::string outType,
                             int maxVertices)
{
    glslStream << "layout (" << inType << ") in; \n"
               << "layout (" << outType << ", max_vertices = " << maxVertices << ") out;\n";
}

void generateEmitPrimitive(std::ostringstream &glslStream,
                           int vertexCount,
                           const std::string &lastVertex)
{
    glslStream << "void emitPrimitive(){\n";
    glslStream << "  int last_vertex = " << lastVertex << ";\n";
    for (int i = 0; i < vertexCount; i++)
    {
        glslStream << "  copyVertex(" << i << ", last_vertex); \n"
                   << "  EmitVertex();\n";
    }
    glslStream << "  EndPrimitive();\n}\n";
}

void generateVariableDeclarationVK(std::ostringstream &glslStream,
                                   int location,
                                   const std::string interpolation,
                                   const std::string inout,
                                   const std::string type,
                                   const std::string variable)
{
    ASSERT(location >= 0);
    glslStream << inout << "  layout (location = " << location << ") " << interpolation << " ";
    glslStream << type << " " << inout << variable;
    if (inout == kInputVarying)
    {
        glslStream << "[]";
    }
    glslStream << ";\n";
}

void DynamicVk::generateStructreDeclarationVk(std::ostringstream &glslStream,
                                              const StructType structType,
                                              const std::string &postString)
{
    glslStream << "struct " << kStructDeclarePrefix << structType.name << " {\n";
    for (auto field : structType.fields)
    {
        glslStream << field.second.first << " " << field.second.second << ";\n";
    }
    glslStream << "} " << postString;
}

void DynamicVk::generateStructListVk(const gl::VaryingPacking &varyingPacking)
{
    const auto &registerInfos = varyingPacking.getRegisterList();
    std::vector<StructField> structFieldList;
    for (size_t registerIndex = 0u; registerIndex < registerInfos.size(); ++registerIndex)
    {
        const gl::PackedVaryingRegister &registerInfo = registerInfos[registerIndex];
        const auto &packedVarying                     = *registerInfo.packedVarying;
        const auto &varying                           = *registerInfo.packedVarying->varying;

        if (packedVarying.vertexOnly || !varying.active)
        {
            continue;
        }

        if (packedVarying.isStructField())
        {
            StructField newField;
            newField.parentStructName = packedVarying.parentStructName;
            newField.location         = registerInfo.registerRow;
            newField.registerIndex    = registerIndex;
            newField.index            = packedVarying.fieldIndex;
            newField.interpolation    = registerInfo.packedVarying->interpolation;
            newField.name = kFieldVariblePrefix + std::to_string(packedVarying.fieldIndex);

            GLenum transposedType = gl::TransposeMatrixType(varying.type);
            GLenum componentType  = gl::VariableComponentType(transposedType);
            int columnCount       = gl::VariableColumnCount(transposedType);
            std::ostringstream typeStream;
            GLSLComponentTypeString(typeStream, componentType, columnCount);
            newField.componentName = typeStream.str();
            newField.byteSize      = columnCount;
            structFieldList.push_back(newField);
        }
    }

    mVaryingStructList.clear();
    int matRows = 0;
    for (auto field : structFieldList)
    {
        auto &varying = mVaryingStructList[field.parentStructName];
        if (varying.name.compare(field.parentStructName))
        {
            varying.name = field.parentStructName;
        }

        const std::string fieldNames = field.name;
        // when a matrix is a member of a struct, all rows have the same field name.
        if (!varying.fields[field.index].second.compare(field.name))
        {
            matRows++;
        }
        else
        {
            matRows = 0;
        }

        std::string componentName;
        if (matRows)
        {
            componentName = "mat" + std::to_string(matRows + 1);
            if (field.byteSize != matRows + 1)
            {
                componentName += "x" + std::to_string(field.byteSize);
            }
        }
        else
        {
            componentName = field.componentName;
        }

        varying.fields[field.index] = std::make_pair(componentName, fieldNames);
        if (!matRows && varying.location < field.location)
        {
            varying.location = field.location;
        }

        if (field.interpolation != sh::INTERPOLATION_SMOOTH)
        {
            varying.interpolation = field.interpolation;
        }
    }
}

void DynamicVk::generateVaryingDeclarationStructVk(std::ostringstream &glslStream, bool isOut) const
{
    for (auto &structVarying : mVaryingStructList)
    {
        std::string interpolation = kEmptyString;
        if (isOut)
        {
            if (structVarying.second.interpolation == sh::INTERPOLATION_FLAT)
            {
                interpolation = kFlatInterploation;
            }
            else if (structVarying.second.interpolation == sh::INTERPOLATION_CENTROID)
            {
                interpolation = kCentroidInterploation;
            }
        }
        const std::string name               = structVarying.second.name;
        const std::string structDeclareName  = kStructDeclarePrefix + name;
        const std::string structVariableName = name;
        generateVariableDeclarationVK(glslStream, structVarying.second.location, interpolation,
                                      isOut ? kOutputVarying : kInputVarying, structDeclareName,
                                      structVariableName);
    }
}

void DynamicVk::generateVaryingDeclarationVaribleVk(std::ostringstream &glslStream,
                                                    const gl::VaryingPacking &varyingPacking,
                                                    bool isOut) const
{
    const auto &registerInfos = varyingPacking.getRegisterList();
    std::string interpolation;
    int skipRows = 0;
    for (size_t registerIndex = registerInfos.size() - 1; registerIndex != std::size_t(-1);
         registerIndex--)
    {
        const gl::PackedVaryingRegister &registerInfo = registerInfos[registerIndex];
        const auto &packedVarying                     = *registerInfo.packedVarying;
        const auto &varying                           = *registerInfo.packedVarying->varying;
        ASSERT(!varying.isStruct());

        if (packedVarying.vertexOnly || !varying.active)
        {
            continue;
        }

        if (!packedVarying.isStructField())
        {
            if (skipRows > 0)
            {
                skipRows--;
                continue;
            }

            interpolation = kEmptyString;
            if (isOut)
            {
                if (registerInfo.packedVarying->interpolation == sh::INTERPOLATION_FLAT)
                {
                    interpolation = kFlatInterploation;
                }
                else if (registerInfo.packedVarying->interpolation == sh::INTERPOLATION_CENTROID)
                {
                    interpolation = kCentroidInterploation;
                }
            }

            int matRows;
            if (registerInfo.varyingRowIndex > 0)
            {
                // it is the last row of a matrix, other rows will be skipped.
                skipRows = registerInfo.varyingRowIndex;
                matRows  = registerInfo.varyingRowIndex + 1;
            }
            else
            {
                matRows = 0;
            }

            GLenum transposedType = gl::TransposeMatrixType(varying.type);
            GLenum componentType  = gl::VariableComponentType(transposedType);
            int columnCount       = gl::VariableColumnCount(transposedType);
            std::ostringstream componentTypeStream;
            int location;
            if (matRows)
            {
                componentTypeStream << "mat" << matRows;
                if (matRows != columnCount)
                {
                    componentTypeStream << "x" << columnCount;
                }
                location = registerInfo.registerRow - registerInfo.varyingRowIndex;
            }
            else
            {
                GLSLComponentTypeString(componentTypeStream, componentType, columnCount);
                location = registerInfo.registerRow;
            }

            generateVariableDeclarationVK(
                glslStream, location, interpolation, isOut ? kOutputVarying : kInputVarying,
                componentTypeStream.str(), kFieldVariblePrefix + std::to_string(registerIndex));
        }
    }
}

const std::string DynamicVk::generateGeometryShader(const gl::PrimitiveMode &drawMode,
                                                    bool enableLineRasterEmulation,
                                                    size_t inputSize,
                                                    gl::ProvokingVertexConvention provokingVertex)
{
    std::ostringstream shaderStream;
    std::string lastVertex;
    bool isLastVertexConvention =
        (provokingVertex == gl::ProvokingVertexConvention::LastVertexConvention) ? true : false;

    int maxVertices = 0;
    shaderStream << "#version 450 core\n";
    switch (drawMode)
    {
        case gl::PrimitiveMode::Points:
            maxVertices = 1;
            generatePrimitiveLayout(shaderStream, "points", "points", maxVertices);
            break;
        case gl::PrimitiveMode::LineLoop:
        case gl::PrimitiveMode::Lines:
        case gl::PrimitiveMode::LineStrip:
            maxVertices = 2;
            generatePrimitiveLayout(shaderStream, "lines", "line_strip", maxVertices);
            break;
        case gl::PrimitiveMode::Triangles:
        case gl::PrimitiveMode::TriangleStrip:
        case gl::PrimitiveMode::TriangleFan:
            maxVertices = 3;
            generatePrimitiveLayout(shaderStream, "triangles", "triangle_strip", maxVertices);
            break;
        default:
            UNREACHABLE();
            return kEmptyString;
    }
    shaderStream << mGeometryShader;
    switch (drawMode)
    {
        case gl::PrimitiveMode::Points:
            lastVertex = "0";
            break;
        case gl::PrimitiveMode::LineLoop:
        case gl::PrimitiveMode::Lines:
        case gl::PrimitiveMode::LineStrip:
            lastVertex = (isLastVertexConvention) ? "1" : "0";
            break;
        case gl::PrimitiveMode::Triangles:
            lastVertex = (isLastVertexConvention) ? "2" : "0";
            break;
            // Primitive generation of TriangleStrip and TriangleFan is different between VK and
            // GL
        case gl::PrimitiveMode::TriangleStrip:
            lastVertex = (isLastVertexConvention) ? "(gl_PrimitiveIDIn % 2 == 1) ? 1 : 2" : "0";
            break;
        case gl::PrimitiveMode::TriangleFan:
            lastVertex = (isLastVertexConvention) ? "1" : "2";
            break;
        default:
            UNREACHABLE();
            return kEmptyString;
    }
    generateEmitPrimitive(shaderStream, maxVertices, lastVertex);

    if (enableLineRasterEmulation)
    {
        shaderStream << "#define ANGLE_ENABLE_LINE_SEGMENT_RASTERIZATION\n"
                        "#ifdef ANGLE_ENABLE_LINE_SEGMENT_RASTERIZATION\n"
                        "layout (location = ";
        shaderStream << inputSize;
        shaderStream << ") out vec4 ANGLEPosition;\n"
                        "#endif\n";
    }
    shaderStream << "void main() {\n  emitPrimitive();\n";
    if (enableLineRasterEmulation)
    {
        shaderStream << "#ifdef ANGLE_ENABLE_LINE_SEGMENT_RASTERIZATION\n"
                        "  ANGLEPosition = gl_Position;\n"
                        "#endif\n";
    }
    shaderStream << "}\n";
    return shaderStream.str();
}

bool DynamicVk::findPointSize(const gl::VaryingPacking &varyingPacking)
{
    for (const PackedVarying &varyings : varyingPacking.getPackedVaryings())
    {
        if (!varyings.varying->name.compare("gl_PointSize"))
        {
            return true;
        }
    }
    return false;
}

void DynamicVk::generateGeometryShaderPreamble(const gl::VaryingPacking &varyingPacking,
                                               bool shaderTessellationAndGeometryPointSize)
{
    std::ostringstream preambleStream;

    generateStructListVk(varyingPacking);
    if (mVaryingStructList.size())
    {
        for (auto structType : mVaryingStructList)
        {
            generateStructreDeclarationVk(preambleStream, structType.second, ";\n");
        }
        generateVaryingDeclarationStructVk(preambleStream, false);
        generateVaryingDeclarationStructVk(preambleStream, true);
    }
    generateVaryingDeclarationVaribleVk(preambleStream, varyingPacking, false);
    generateVaryingDeclarationVaribleVk(preambleStream, varyingPacking, true);

    preambleStream << "void copyVertex(int index, int last_vertex) {\n"
                   << "  gl_Position = gl_in[index].gl_Position;\n";
    if (shaderTessellationAndGeometryPointSize)
    {
        if (findPointSize(varyingPacking))
        {
            preambleStream << "  gl_PointSize = gl_in[index].gl_PointSize;\n";
        }
    }
    const auto &registerInfos = varyingPacking.getRegisterList();

    int skipRows = 0;
    for (size_t registerIndex = registerInfos.size() - 1; registerIndex != std::size_t(-1);
         registerIndex--)
    {
        const gl::PackedVaryingRegister &varyingRegister = registerInfos[registerIndex];
        const gl::PackedVaryingRegister &registerInfo    = registerInfos[registerIndex];
        std::string variableName;
        if (varyingRegister.packedVarying->isStructField())
        {
            continue;
        }

        if (skipRows > 0)
        {
            skipRows--;
            continue;
        }

        if (registerInfo.varyingRowIndex > 0)
        {
            skipRows = registerInfo.varyingRowIndex;
        }

        const std::string fieldName      = kFieldVariblePrefix + std::to_string(registerIndex);
        const std::string inputVariable  = kInputVarying + variableName;
        const std::string outputVariable = kOutputVarying + variableName;
        preambleStream << "  " << kOutputVarying << variableName << fieldName << " = "
                       << inputVariable << fieldName;
        if (varyingRegister.packedVarying->interpolation == sh::INTERPOLATION_FLAT)
        {
            preambleStream << "[last_vertex]";
        }
        else
        {
            preambleStream << "[index]";
        }
        preambleStream << "; \n";
    }

    for (auto varyingStruct : mVaryingStructList)
    {
        const std::string inputVariable  = kInputVarying + varyingStruct.second.name;
        const std::string outputVariable = kOutputVarying + varyingStruct.second.name;
        for (auto field : varyingStruct.second.fields)
        {
            preambleStream << "  " << outputVariable << "." << field.second.second << " = "
                           << inputVariable;
            if (varyingStruct.second.interpolation == sh::INTERPOLATION_FLAT)
            {
                preambleStream << "[last_vertex].";
            }
            else
            {
                preambleStream << "[index].";
            }
            preambleStream << field.second.second << "; \n";
        }
    }

    preambleStream << "}\n";
    mGeometryShader = preambleStream.str();
    return;
}

}  // namespace rx
