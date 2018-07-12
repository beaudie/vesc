//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DynamicImage2DHLSL.cpp: Implementation for link and run-time HLSL generation
//

#include "libANGLE/renderer/d3d/DynamicImage2DHLSL.h"

#include "common/string_utils.h"
#include "libANGLE/renderer/d3d/ProgramD3D.h"
#include "libANGLE/renderer/d3d/ShaderD3D.h"

using namespace gl;

namespace rx
{

namespace
{

enum Image2DHLSLGroup
{
    IMAGE2D_R_FLOAT4,
    IMAGE2D_MIN = IMAGE2D_R_FLOAT4,
    IMAGE2D_R_UNORM,
    IMAGE2D_R_SNORM,
    IMAGE2D_R_UINT4,
    IMAGE2D_R_INT4,
    IMAGE2D_W_FLOAT4,
    IMAGE2D_W_UNORM,
    IMAGE2D_W_SNORM,
    IMAGE2D_W_UINT4,
    IMAGE2D_W_INT4,
    IMAGE2D_UNKNOWN,
    IMAGE2D_MAX = IMAGE2D_UNKNOWN
};

enum Image2DMethod
{
    IMAGE2DSIZE,
    IMAGE2DLOAD,
    IMAGE2DSTORE
};

Image2DHLSLGroup image2DHLSLGroup(const sh::Uniform &uniform)
{
    GLenum format = uniform.imageUnitFormat;
    bool readonly = uniform.readonly;
    switch (uniform.type)
    {
        case GL_IMAGE_2D:
        {
            switch (format)
            {
                case GL_RGBA32F:
                case GL_RGBA16F:
                case GL_R32F:
                    return readonly ? IMAGE2D_R_FLOAT4 : IMAGE2D_W_FLOAT4;
                case GL_RGBA8:
                    return readonly ? IMAGE2D_R_UNORM : IMAGE2D_W_UNORM;
                case GL_RGBA8_SNORM:
                    return readonly ? IMAGE2D_R_SNORM : IMAGE2D_W_SNORM;
                default:
                    UNREACHABLE();
                    return IMAGE2D_UNKNOWN;
            }
        }
        case GL_INT_IMAGE_2D:
        {
            switch (format)
            {
                case GL_RGBA32I:
                case GL_RGBA16I:
                case GL_RGBA8I:
                case GL_R32I:
                    return readonly ? IMAGE2D_R_INT4 : IMAGE2D_W_INT4;
                default:
                    UNREACHABLE();
                    return IMAGE2D_UNKNOWN;
            }
        }
        case GL_UNSIGNED_INT_IMAGE_2D:
        {
            switch (format)
            {
                case GL_RGBA32UI:
                case GL_RGBA16UI:
                case GL_RGBA8UI:
                case GL_R32UI:
                    return readonly ? IMAGE2D_R_UINT4 : IMAGE2D_W_UINT4;
                default:
                    UNREACHABLE();
                    return IMAGE2D_UNKNOWN;
            }
        }
        default:
            UNREACHABLE();
            return IMAGE2D_UNKNOWN;
    }
}

std::string Image2DHLSLGroupSuffix(Image2DHLSLGroup group)
{
    switch (group)
    {
        case IMAGE2D_R_FLOAT4:
            return "2D";
        case IMAGE2D_R_UNORM:
            return "2D_unorm_float4_";
        case IMAGE2D_R_SNORM:
            return "2D_snorm_float4_";
        case IMAGE2D_R_UINT4:
            return "2D_uint4_";
        case IMAGE2D_R_INT4:
            return "2D_int4_";
        case IMAGE2D_W_FLOAT4:
            return "RW2D_float4_";
        case IMAGE2D_W_UNORM:
            return "RW2D_unorm_float4_";
        case IMAGE2D_W_SNORM:
            return "RW2D_snorm_float4_";
        case IMAGE2D_W_UINT4:
            return "RW2D_uint4_";
        case IMAGE2D_W_INT4:
            return "RW2D_int4_";
        default:
            UNREACHABLE();
    }

    return "<unknown group type>";
}

std::string Image2DHLSLTextureString(Image2DHLSLGroup group, gl::TextureType type)
{
    std::string textureString;
    switch (group)
    {
        case IMAGE2D_R_FLOAT4:
        case IMAGE2D_R_UNORM:
        case IMAGE2D_R_SNORM:
        case IMAGE2D_R_UINT4:
        case IMAGE2D_R_INT4:
            break;
        case IMAGE2D_W_FLOAT4:
        case IMAGE2D_W_UNORM:
        case IMAGE2D_W_SNORM:
        case IMAGE2D_W_UINT4:
        case IMAGE2D_W_INT4:
            textureString += "RW";
            break;
        default:
            UNREACHABLE();
    }

    textureString += "Texture";

    switch (type)
    {
        case gl::TextureType::_2D:
            textureString += "2D";
            break;
        case gl::TextureType::_3D:
            textureString += "3D";
            break;
        case gl::TextureType::_2DArray:
            textureString += "2DArray";
            break;
        default:
            UNREACHABLE();
    }

    switch (group)
    {
        case IMAGE2D_R_FLOAT4:
        case IMAGE2D_W_FLOAT4:
            textureString += "<float4>";
            break;
        case IMAGE2D_R_UNORM:
        case IMAGE2D_W_UNORM:
            textureString += "<unorm float4>";
            break;
        case IMAGE2D_R_SNORM:
        case IMAGE2D_W_SNORM:
            textureString += "<snorm float4>";
            break;
        case IMAGE2D_R_UINT4:
        case IMAGE2D_W_UINT4:
            textureString += "<uint4>";
            break;
        case IMAGE2D_R_INT4:
        case IMAGE2D_W_INT4:
            textureString += "<int4>";
            break;
        default:
            UNREACHABLE();
    }

    return textureString;
}

std::string Image2DHLSLGroupOffsetPrefix(Image2DHLSLGroup group)
{
    switch (group)
    {
        case IMAGE2D_R_FLOAT4:
        case IMAGE2D_R_UNORM:
        case IMAGE2D_R_SNORM:
        case IMAGE2D_R_UINT4:
        case IMAGE2D_R_INT4:
            return "readonlyImageIndexOffset";
        case IMAGE2D_W_FLOAT4:
        case IMAGE2D_W_UNORM:
        case IMAGE2D_W_SNORM:
        case IMAGE2D_W_UINT4:
        case IMAGE2D_W_INT4:
            return "imageIndexOffset";
        default:
            UNREACHABLE();
    }

    return "<unknown group type>";
}

std::string Image2DHLSLGroupDeclarationPrefix(Image2DHLSLGroup group)
{
    switch (group)
    {
        case IMAGE2D_R_FLOAT4:
        case IMAGE2D_R_UNORM:
        case IMAGE2D_R_SNORM:
        case IMAGE2D_R_UINT4:
        case IMAGE2D_R_INT4:
            return "readonlyImages";
        case IMAGE2D_W_FLOAT4:
        case IMAGE2D_W_UNORM:
        case IMAGE2D_W_SNORM:
        case IMAGE2D_W_UINT4:
        case IMAGE2D_W_INT4:
            return "images";
        default:
            UNREACHABLE();
    }

    return "<unknown group type>";
}

std::string Image2DHLSLGroupRegisterSuffix(Image2DHLSLGroup group)
{
    switch (group)
    {
        case IMAGE2D_R_FLOAT4:
        case IMAGE2D_R_UNORM:
        case IMAGE2D_R_SNORM:
        case IMAGE2D_R_UINT4:
        case IMAGE2D_R_INT4:
            return "t";
        case IMAGE2D_W_FLOAT4:
        case IMAGE2D_W_UNORM:
        case IMAGE2D_W_SNORM:
        case IMAGE2D_W_UINT4:
        case IMAGE2D_W_INT4:
            return "u";
        default:
            UNREACHABLE();
    }

    return "<unknown group type>";
}

std::string Image2DHLSLGroupFunctionName(Image2DHLSLGroup group, Image2DMethod method)
{
    std::string name = "gl_image";
    name += Image2DHLSLGroupSuffix(group);
    switch (method)
    {
        case IMAGE2DSIZE:
            name += "Size";
            break;
        case IMAGE2DLOAD:
            name += "Load";
            break;
        case IMAGE2DSTORE:
            name += "Store";
            break;
        default:
            UNREACHABLE();
    }

    return name;
}

std::string getImage2DGroupReturnType(Image2DHLSLGroup group, Image2DMethod method)
{
    switch (method)
    {
        case IMAGE2DSIZE:
            return "int2";
        case IMAGE2DLOAD:
            switch (group)
            {
                case IMAGE2D_R_FLOAT4:
                case IMAGE2D_R_UNORM:
                case IMAGE2D_R_SNORM:
                case IMAGE2D_W_FLOAT4:
                case IMAGE2D_W_UNORM:
                case IMAGE2D_W_SNORM:
                    return "float4";
                case IMAGE2D_R_UINT4:
                case IMAGE2D_W_UINT4:
                    return "uint4";
                case IMAGE2D_R_INT4:
                case IMAGE2D_W_INT4:
                    return "int4";
                default:
                    UNREACHABLE();
                    return "unknown group type";
            }
        case IMAGE2DSTORE:
            return "void";
        default:
            UNREACHABLE();
            return "unknown image method";
    }
}

void OutputImage2DFunctionArgumentList(std::ostringstream &out,
                                       Image2DHLSLGroup group,
                                       Image2DMethod method)
{
    out << "uint imageIndex";

    if (method == IMAGE2DLOAD || method == IMAGE2DSTORE)
    {
        out << ", int2 p";
        if (method == IMAGE2DSTORE)
        {
            switch (group)
            {
                case IMAGE2D_R_FLOAT4:
                case IMAGE2D_R_UNORM:
                case IMAGE2D_R_SNORM:
                case IMAGE2D_W_FLOAT4:
                case IMAGE2D_W_UNORM:
                case IMAGE2D_W_SNORM:
                    out << ", float4 data";
                    break;
                case IMAGE2D_R_UINT4:
                case IMAGE2D_W_UINT4:
                    out << ", uint4 data";
                    break;
                case IMAGE2D_R_INT4:
                case IMAGE2D_W_INT4:
                    out << ", int4 data";
                    break;
                default:
                    UNREACHABLE();
            }
        }
    }
}

void OutputImage2DSizeFunction(std::ostringstream &out,
                               Image2DHLSLGroup textureGroup,
                               unsigned int totalCount,
                               unsigned int texture2DCount,
                               unsigned int texture3DCount,
                               unsigned int texture2DArrayCount,
                               const std::string &offsetStr,
                               const std::string &declarationStr)
{
    out << getImage2DGroupReturnType(textureGroup, IMAGE2DSIZE) << " "
        << Image2DHLSLGroupFunctionName(textureGroup, IMAGE2DSIZE) << "(";
    OutputImage2DFunctionArgumentList(out, textureGroup, IMAGE2DSIZE);
    out << ")\n"
           "{\n";
    out << "    uint width, height;\n";

    if (texture2DCount > 0)
    {
        if (texture2DCount == totalCount)
        {
            out << "    const uint index = imageIndex -  " << offsetStr << "2D;\n";
            out << "    " << declarationStr << "2D[index].GetDimensions(width, height);\n";
        }
        else
        {
            out << "    if (imageIndex >= " << offsetStr << "2D && imageIndex < " << offsetStr
                << "2D + " << texture2DCount << ")\n";
            out << "    {\n";
            out << "        const uint index = imageIndex -  " << offsetStr << "2D;\n";
            out << "        " << declarationStr << "2D[index].GetDimensions(width, height);\n";
            out << "    }\n";
        }
    }

    if (texture3DCount > 0)
    {
        if (texture3DCount == totalCount)
        {
            out << "    const uint index = imageIndex -  " << offsetStr << "3D;\n";
            out << "    uint depth;\n";
            out << "    " << declarationStr << "3D[index].GetDimensions(width, height, depth);\n";
        }
        else
        {
            out << "    if (imageIndex >= " << offsetStr << "3D && imageIndex < " << offsetStr
                << "3D + " << texture3DCount << ")\n";
            out << "    {\n";
            out << "        const uint index = imageIndex -  " << offsetStr << "3D;\n";
            out << "        uint depth;\n";
            out << "        " << declarationStr
                << "3D[index].GetDimensions(width, height, depth);\n";
            out << "    }\n";
        }
    }

    if (texture2DArrayCount > 0)
    {
        if (texture2DArrayCount == totalCount)
        {
            out << "    const uint index = imageIndex -  " << offsetStr << "2DArray;\n";
            out << "    uint depth;\n";
            out << "    " << declarationStr
                << "2DArray[index].GetDimensions(width, height, depth);\n";
        }
        else
        {
            out << "    if (imageIndex >= " << offsetStr << "2DArray && imageIndex < " << offsetStr
                << "2DArray + " << texture2DArrayCount << ")\n";
            out << "    {\n";
            out << "        const uint index = imageIndex -  " << offsetStr << "2DArray;\n";
            out << "        uint depth;\n";
            out << "        " << declarationStr
                << "2DArray[index].GetDimensions(width, height, depth);\n";
            out << "    }\n";
        }
    }
    out << "    return int2(width, height);\n";

    out << "}\n";
}

void OutputImage2DLoadFunction(std::ostringstream &out,
                               Image2DHLSLGroup textureGroup,
                               unsigned int totalCount,
                               unsigned int texture2DCount,
                               unsigned int texture3DCount,
                               unsigned int texture2DArrayCount,
                               const std::string &offsetStr,
                               const std::string &declarationStr)
{
    out << getImage2DGroupReturnType(textureGroup, IMAGE2DLOAD) << " "
        << Image2DHLSLGroupFunctionName(textureGroup, IMAGE2DLOAD) << "(";
    OutputImage2DFunctionArgumentList(out, textureGroup, IMAGE2DLOAD);
    out << ")\n"
           "{\n";

    out << "    " << getImage2DGroupReturnType(textureGroup, IMAGE2DLOAD) << " result;\n";

    if (texture2DCount > 0)
    {
        if (texture2DCount == totalCount)
        {
            out << "    const uint index = imageIndex -  " << offsetStr << "2D;\n";
            out << "    result = " << declarationStr << "2D[index][uint2(p.x, p.y)];\n";
        }
        else
        {
            out << "    if (imageIndex >= " << offsetStr << "2D && imageIndex < " << offsetStr
                << "2D + " << texture2DCount << ")\n";
            out << "    {\n";
            out << "        const uint index = imageIndex -  " << offsetStr << "2D;\n";
            out << "        result = " << declarationStr << "2D[index][uint2(p.x, p.y)];\n";
            out << "    }\n";
        }
    }

    if (texture3DCount > 0)
    {
        if (texture3DCount == totalCount)
        {
            out << "    const uint index = imageIndex -  " << offsetStr << "3D;\n";
            out << "    result = " << declarationStr
                << "3D[index][uint3(p.x, p.y, imageUnitLayersMap[imageIndex"
                << "])];\n";
        }
        else
        {
            out << "    if (imageIndex >= " << offsetStr << "3D && imageIndex < " << offsetStr
                << "3D + " << texture3DCount << ")\n";
            out << "    {\n";
            out << "        const uint index = imageIndex -  " << offsetStr << "3D;\n";
            out << "        result = " << declarationStr
                << "3D[index][uint3(p.x, p.y, imageUnitLayersMap[imageIndex"
                << "])];\n";
            out << "    }\n";
        }
    }

    if (texture2DArrayCount > 0)
    {
        if (texture2DArrayCount == totalCount)
        {
            out << "    const uint index = imageIndex -  " << offsetStr << "2DArray;\n";
            out << "    result = " << declarationStr
                << "2DArray[index][uint3(p.x, p.y, imageUnitLayersMap[imageIndex"
                << "])];\n";
        }
        else
        {
            out << "    if (imageIndex >= " << offsetStr << "2DArray && imageIndex < " << offsetStr
                << "2DArray + " << texture2DArrayCount << ")\n";
            out << "    {\n";
            out << "        const uint index = imageIndex -  " << offsetStr << "2DArray;\n";
            out << "        result = " << declarationStr
                << "2DArray[index][uint3(p.x, p.y, imageUnitLayersMap[imageIndex"
                << "])];\n";
            out << "    }\n";
        }
    }

    out << "    return result;\n";
    out << "}\n";
}

void OutputImage2DStoreFunction(std::ostringstream &out,
                                Image2DHLSLGroup textureGroup,
                                unsigned int totalCount,
                                unsigned int texture2DCount,
                                unsigned int texture3DCount,
                                unsigned int texture2DArrayCount,
                                const std::string &offsetStr,
                                const std::string &declarationStr)
{
    out << getImage2DGroupReturnType(textureGroup, IMAGE2DSTORE) << " "
        << Image2DHLSLGroupFunctionName(textureGroup, IMAGE2DSTORE) << "(";
    OutputImage2DFunctionArgumentList(out, textureGroup, IMAGE2DSTORE);
    out << ")\n"
           "{\n";

    if (texture2DCount > 0)
    {
        if (texture2DCount == totalCount)
        {
            out << "    const uint index = imageIndex -  " << offsetStr << "2D;\n";
            out << "    " << declarationStr << "2D[index][uint2(p.x, p.y)] = data;\n";
        }
        else
        {
            out << "    if (imageIndex >= " << offsetStr << "2D && imageIndex < " << offsetStr
                << "2D + " << texture2DCount << ")\n";
            out << "    {\n";
            out << "        const uint index = imageIndex -  " << offsetStr << "2D;\n";
            out << "        " << declarationStr << "2D[index][uint2(p.x, p.y)] = data;\n";
            out << "    }\n";
        }
    }

    if (texture3DCount > 0)
    {
        if (texture3DCount == totalCount)
        {
            out << "    const uint index = imageIndex -  " << offsetStr << "3D;\n";
            out << "    " << declarationStr
                << "3D[index][uint3(p.x, p.y, imageUnitLayersMap[imageIndex "
                << "])] = data;\n";
        }
        else
        {
            out << "    if (imageIndex >= " << offsetStr << "3D && imageIndex < " << offsetStr
                << "3D + " << texture3DCount << ")\n";
            out << "    {\n";
            out << "        const uint index = imageIndex -  " << offsetStr << "3D;\n";
            out << "        " << declarationStr
                << "3D[index][uint3(p.x, p.y, imageUnitLayersMap[imageIndex "
                << "])] = data;\n";
            out << "    }\n";
        }
    }

    if (texture2DArrayCount > 0)
    {
        if (texture2DArrayCount == totalCount)
        {
            out << "    const uint index = imageIndex -  " << offsetStr << "2DArray;\n";
            out << "    " << declarationStr
                << "2DArray[index][uint3(p.x, p.y, imageUnitLayersMap[imageIndex "
                << "])] = data;\n";
        }
        else
        {
            out << "    if (imageIndex >= " << offsetStr << "2DArray && imageIndex < " << offsetStr
                << "2DArray + " << texture2DArrayCount << ")\n";
            out << "    {\n";
            out << "        const uint index = imageIndex -  " << offsetStr << "2DArray;\n";
            out << "        " << declarationStr
                << "2DArray[index][uint3(p.x, p.y, imageUnitLayersMap[imageIndex "
                << "])] = data;\n";
            out << "    }\n";
        }
    }

    out << "}\n";
}

unsigned int *GetImage2DRegisterIndex(Image2DHLSLGroup textureGroup,
                                      unsigned int *groupTextureRegisterIndex,
                                      unsigned int *groupRWTextureRegisterIndex)
{
    switch (textureGroup)
    {
        case IMAGE2D_R_FLOAT4:
        case IMAGE2D_R_UNORM:
        case IMAGE2D_R_SNORM:
        case IMAGE2D_R_UINT4:
        case IMAGE2D_R_INT4:
            return groupTextureRegisterIndex;
        case IMAGE2D_W_FLOAT4:
        case IMAGE2D_W_UNORM:
        case IMAGE2D_W_SNORM:
        case IMAGE2D_W_UINT4:
        case IMAGE2D_W_INT4:
            return groupRWTextureRegisterIndex;
        default:
            UNREACHABLE();
            return nullptr;
    }
}

void outputHLSLImage2DUniformGroup(
    ProgramD3D &programD3D,
    const gl::ProgramState &programData,
    std::ostringstream &out,
    const Image2DHLSLGroup textureGroup,
    const std::vector<sh::Uniform> &group,
    const std::map<unsigned int, gl::TextureType> &image2DBoundLayout,
    unsigned int *groupTextureRegisterIndex,
    unsigned int *groupRWTextureRegisterIndex,
    unsigned int *image2DTexture3D,
    unsigned int *image2DTexture2DArray,
    unsigned int *image2DTexture2D,
    std::map<unsigned int, unsigned int> &readonlyImage2DImageIndexCS,
    std::map<unsigned int, unsigned int> &image2DImageIndexCS)
{
    if (group.empty())
    {
        return;
    }

    unsigned int texture2DCount = 0, texture3DCount = 0, texture2DArrayCount = 0;
    for (const auto &uniform : group)
    {
        if (!programD3D.getD3DUniformByName(uniform.name))
        {
            continue;
        }
        for (unsigned int index = 0; index < uniform.getArraySizeProduct(); index++)
        {
            switch (image2DBoundLayout.at(uniform.binding + index))
            {
                case gl::TextureType::_2D:
                    texture2DCount++;
                    break;
                case gl::TextureType::_3D:
                    texture3DCount++;
                    break;
                case gl::TextureType::_2DArray:
                case gl::TextureType::CubeMap:
                    texture2DArrayCount++;
                    break;
                default:
                    UNREACHABLE();
            }
        }
    }

    unsigned int totalCount        = texture2DCount + texture3DCount + texture2DArrayCount;
    unsigned int texture2DRegister = 0, texture3DRegister = 0, texture2DArrayRegister = 0;
    unsigned int *image2DRegisterIndex = GetImage2DRegisterIndex(
        textureGroup, groupTextureRegisterIndex, groupRWTextureRegisterIndex);
    unsigned int texture2DRegisterStart      = *image2DRegisterIndex;
    unsigned int texture3DRegisterStart      = texture2DRegisterStart + texture2DCount;
    unsigned int texture2DArrayRegisterStart = texture3DRegisterStart + texture3DCount;

    std::string offsetStr =
        Image2DHLSLGroupOffsetPrefix(textureGroup) + Image2DHLSLGroupSuffix(textureGroup);
    std::string declarationStr =
        Image2DHLSLGroupDeclarationPrefix(textureGroup) + Image2DHLSLGroupSuffix(textureGroup);
    std::string registerStr = Image2DHLSLGroupRegisterSuffix(textureGroup);
    if (texture2DCount > 0)
    {
        out << "static const uint " << offsetStr << "2D = " << *image2DTexture2D << ";\n";
        out << "uniform " << Image2DHLSLTextureString(textureGroup, gl::TextureType::_2D) << " "
            << declarationStr << "2D[" << texture2DCount << "]"
            << " : register(" << registerStr << *image2DRegisterIndex << ");\n";
        *image2DRegisterIndex += texture2DCount;
    }
    if (texture3DCount > 0)
    {
        out << "static const uint " << offsetStr << "3D = " << *image2DTexture3D << ";\n";
        out << "uniform " << Image2DHLSLTextureString(textureGroup, gl::TextureType::_3D) << " "
            << declarationStr << "3D[" << texture3DCount << "]"
            << " : register(" << registerStr << *image2DRegisterIndex << ");\n";
        *image2DRegisterIndex += texture3DCount;
    }
    if (texture2DArrayCount > 0)
    {
        out << "static const uint " << offsetStr << "2DArray = " << *image2DTexture2DArray << ";\n";
        out << "uniform " << Image2DHLSLTextureString(textureGroup, gl::TextureType::_2DArray)
            << " " << declarationStr << "2DArray[" << texture2DArrayCount << "]"
            << " : register(" << registerStr << *image2DRegisterIndex << ");\n";
        *image2DRegisterIndex += texture2DArrayCount;
    }
    for (const auto &uniform : group)
    {
        if (!programD3D.getD3DUniformByName(uniform.name))
        {
            continue;
        }

        out << "static const uint " << DecorateVariable(uniform.name)
            << ArrayIndexString(uniform.arraySizes) << " = {";
        for (unsigned int index = 0; index < uniform.getArraySizeProduct(); index++)
        {
            if (index > 0)
            {
                out << ", ";
            }
            switch (image2DBoundLayout.at(uniform.binding + index))
            {
                case gl::TextureType::_2D:
                {
                    out << (*image2DTexture2D)++;
                    programD3D.assignImage2DRegisters(texture2DRegisterStart + texture2DRegister,
                                                      uniform.binding + index, uniform.readonly);
                    texture2DRegister++;
                    break;
                }
                case gl::TextureType::_3D:
                {
                    unsigned int imageIndex3D = (*image2DTexture3D)++;
                    out << imageIndex3D;
                    programD3D.assignImage2DRegisters(texture3DRegisterStart + texture3DRegister,
                                                      uniform.binding + index, uniform.readonly);
                    if (uniform.readonly)
                    {
                        readonlyImage2DImageIndexCS[texture3DRegisterStart + texture3DRegister] =
                            imageIndex3D;
                    }
                    else
                    {
                        image2DImageIndexCS[texture3DRegisterStart + texture3DRegister] =
                            imageIndex3D;
                    }
                    texture3DRegister++;
                    break;
                }
                case gl::TextureType::_2DArray:
                case gl::TextureType::CubeMap:
                {
                    unsigned int imageIndex2DArray = (*image2DTexture2DArray)++;
                    out << imageIndex2DArray;
                    programD3D.assignImage2DRegisters(
                        texture2DArrayRegisterStart + texture2DArrayRegister,
                        uniform.binding + index, uniform.readonly);
                    if (uniform.readonly)
                    {
                        readonlyImage2DImageIndexCS[texture2DArrayRegisterStart +
                                                    texture2DArrayRegister] = imageIndex2DArray;
                    }
                    else
                    {
                        image2DImageIndexCS[texture2DArrayRegisterStart + texture2DArrayRegister] =
                            imageIndex2DArray;
                    }
                    texture2DArrayRegister++;
                    break;
                }
                default:
                    UNREACHABLE();
            }
        }
        out << "};\n";
    }

    gl::Shader *computeShaderGL    = programData.getAttachedShader(ShaderType::Compute);
    const ShaderD3D *computeShader = GetImplAs<ShaderD3D>(computeShaderGL);

    if (computeShader->useImage2DFunction(Image2DHLSLGroupFunctionName(textureGroup, IMAGE2DSIZE)))
    {
        OutputImage2DSizeFunction(out, textureGroup, totalCount, texture2DCount, texture3DCount,
                                  texture2DArrayCount, offsetStr, declarationStr);
    }
    if (computeShader->useImage2DFunction(Image2DHLSLGroupFunctionName(textureGroup, IMAGE2DLOAD)))
    {
        OutputImage2DLoadFunction(out, textureGroup, totalCount, texture2DCount, texture3DCount,
                                  texture2DArrayCount, offsetStr, declarationStr);
    }
    if (computeShader->useImage2DFunction(Image2DHLSLGroupFunctionName(textureGroup, IMAGE2DSTORE)))
    {
        OutputImage2DStoreFunction(out, textureGroup, totalCount, texture2DCount, texture3DCount,
                                   texture2DArrayCount, offsetStr, declarationStr);
    }
}

// IMAGE2D_DECLARATION_FUNCTION_STRING and IMAGE2D_LAYER_OF_TEXTURE_STRING must be the same as
// outputHLSL.
constexpr const char *IMAGE2D_DECLARATION_FUNCTION_STRING =
    "// @@ IMAGE2D DECLARATION FUNCTION STRING @@";
constexpr const char *IMAGE2D_LAYER_OF_TEXTURE_STRING = "// @@ IMAGE2D LAYER OF TEXTURE STRING @@";
}  // anonymous namespace

std::string DynamicImage2DHLSL::generateComputeShaderForImage2DBoundSignature(
    const d3d::Context *context,
    ProgramD3D &programD3D,
    const gl::ProgramState &programData,
    std::vector<sh::Uniform> &image2DUniforms,
    const std::map<unsigned int, gl::TextureType> &image2DBoundLayout,
    std::map<unsigned int, unsigned int> &readonlyImage2DImageIndexCS,
    std::map<unsigned int, unsigned int> &image2DImageIndexCS)
{
    std::vector<std::vector<sh::Uniform>> groupedImage2DUniforms(IMAGE2D_MAX + 1);
    unsigned int image2DTexture2DCount = 0, image2DTexture3DCount = 0,
                 image2DTexture2DArrayCount = 0;
    for (auto &image2D : image2DUniforms)
    {
        for (unsigned int index = 0; index < image2D.getArraySizeProduct(); index++)
        {
            // Any image variable declared without a binding qualifier is initially bound to unit
            // zero.
            if (image2D.binding == -1)
            {
                image2D.binding = 0;
            }
            switch (image2DBoundLayout.at(image2D.binding + index))
            {
                case gl::TextureType::_2D:
                    image2DTexture2DCount++;
                    break;
                case gl::TextureType::_3D:
                    image2DTexture3DCount++;
                    break;
                case gl::TextureType::_2DArray:
                case gl::TextureType::CubeMap:
                    image2DTexture2DArrayCount++;
                    break;
                default:
                    UNREACHABLE();
            }
        }
        Image2DHLSLGroup group = image2DHLSLGroup(image2D);
        groupedImage2DUniforms[group].push_back(image2D);
    }

    gl::Shader *computeShaderGL              = programData.getAttachedShader(ShaderType::Compute);
    const ShaderD3D *computeShader           = GetImplAs<ShaderD3D>(computeShaderGL);
    unsigned int groupTextureRegisterIndex   = computeShader->getReadonlyImage2DRegisterIndex();
    unsigned int groupRWTextureRegisterIndex = computeShader->getImage2DRegisterIndex();
    unsigned int image2DTexture3DIndex       = 0;
    unsigned int image2DTexture2DArrayIndex  = image2DTexture3DCount;
    unsigned int image2DTexture2DIndex       = image2DTexture3DCount + image2DTexture2DArrayCount;
    std::ostringstream out;

    for (int groupId = IMAGE2D_MIN; groupId < IMAGE2D_MAX; ++groupId)
    {
        outputHLSLImage2DUniformGroup(
            programD3D, programData, out, Image2DHLSLGroup(groupId),
            groupedImage2DUniforms[groupId], image2DBoundLayout, &groupTextureRegisterIndex,
            &groupRWTextureRegisterIndex, &image2DTexture3DIndex, &image2DTexture2DArrayIndex,
            &image2DTexture2DIndex, readonlyImage2DImageIndexCS, image2DImageIndexCS);
    }

    std::string computeHLSL(
        programData.getAttachedShader(ShaderType::Compute)->getTranslatedSource());
    bool success =
        angle::ReplaceSubstring(&computeHLSL, IMAGE2D_DECLARATION_FUNCTION_STRING, out.str());
    ASSERT(success);

    if (image2DTexture3DCount + image2DTexture2DArrayCount > 0)
    {
        std::ostringstream layerArrayDeclaration;
        layerArrayDeclaration << "uint imageUnitLayersMap["
                              << image2DTexture3DCount + image2DTexture2DArrayCount << "] "
                              << ": packoffset(c" << computeShader->getSamplerUniformsCount() + 1
                              << ");";
        success = angle::ReplaceSubstring(&computeHLSL, IMAGE2D_LAYER_OF_TEXTURE_STRING,
                                          layerArrayDeclaration.str());
        ASSERT(success);
    }

    return computeHLSL;
}

}  // namespace rx
