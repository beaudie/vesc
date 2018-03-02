//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageFunctionHLSL: Class for writing implementations of ESSL image functions into HLSL output.
//

#include "compiler/translator/ImageFunctionHLSL.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/UtilsHLSL.h"

namespace sh
{

// static
void ImageFunctionHLSL::OutputImageFunctionArgumentList(
    TInfoSinkBase &out,
    const ImageFunctionHLSL::ImageFunction &imageFunction)
{
    out << "uint imageIndex";

    if (imageFunction.method == ImageFunctionHLSL::ImageFunction::Method::LOAD ||
        imageFunction.method == ImageFunctionHLSL::ImageFunction::Method::STORE)
    {
        switch (imageFunction.image)
        {
            case EbtImage2D:
            case EbtIImage2D:
            case EbtUImage2D:
                out << ", int2 p";
                break;
            case EbtImage3D:
            case EbtIImage3D:
            case EbtUImage3D:
            case EbtImageCube:
            case EbtIImageCube:
            case EbtUImageCube:
            case EbtImage2DArray:
            case EbtIImage2DArray:
            case EbtUImage2DArray:
                out << ", int3 p";
                break;
            default:
                UNREACHABLE();
        }

        if (imageFunction.method == ImageFunctionHLSL::ImageFunction::Method::STORE)
        {
            switch (imageFunction.image)
            {
                case EbtImage2D:
                case EbtImage3D:
                case EbtImageCube:
                case EbtImage2DArray:
                    out << ", float4 data";
                    break;
                case EbtIImage2D:
                case EbtIImage3D:
                case EbtIImageCube:
                case EbtIImage2DArray:
                    out << ", int4 data";
                    break;
                case EbtUImage2D:
                case EbtUImage3D:
                case EbtUImageCube:
                case EbtUImage2DArray:
                    out << ", uint4 data";
                    break;
                default:
                    UNREACHABLE();
            }
        }
    }
}

void GetTextureReference(const ImageFunctionHLSL::ImageFunction &imageFunction,
                         ImmutableString *imageReference)
{
    if (imageFunction.readonly)
    {
        const ImmutableString kImageStr("readonlyImages");
        ImmutableString suffix(
            TextureGroupSuffix(imageFunction.image, imageFunction.imageInternalFormat));
        ImmutableStringBuilder imageRefBuilder(kImageStr.length() + suffix.length());
        imageRefBuilder << kImageStr << suffix;
        *imageReference = imageRefBuilder;
    }
    else
    {
        const ImmutableString kImageStr("images");
        ImmutableString suffix(
            RWTextureGroupSuffix(imageFunction.image, imageFunction.imageInternalFormat));
        ImmutableStringBuilder imageRefBuilder(kImageStr.length() + suffix.length());
        imageRefBuilder << kImageStr << suffix;
        *imageReference = imageRefBuilder;
    }
}

TString GetImageIndexOffset(const ImageFunctionHLSL::ImageFunction &imageFunction, std::string str)
{
    TString name = "";
    if (imageFunction.readonly)
    {
        name += "readonlyImageIndexOffset";
        name += TextureGroupSuffix(imageFunction.image, imageFunction.imageInternalFormat);
    }
    else
    {
        name += "imageIndexOffset";
        name += RWTextureGroupSuffix(imageFunction.image, imageFunction.imageInternalFormat);
    }

    name += str.c_str();
    return name;
}

void GenerateMacroBegin(TInfoSinkBase &out,
                        const ImageFunctionHLSL::ImageFunction &imageFunction,
                        std::string str)
{
    TString macro = "gl_image";
    if (imageFunction.readonly)
    {
        macro += TextureTypeSuffix(imageFunction.image, imageFunction.imageInternalFormat);
    }
    else
    {
        macro += RWTextureTypeSuffix(imageFunction.image, imageFunction.imageInternalFormat);
    }
    macro += str.c_str();

    out << "#ifdef " << macro << "\n";
}

void GenerateMacroEnd(TInfoSinkBase &out)
{
    out << "#endif\n";
}

// static
void ImageFunctionHLSL::OutputImageSizeFunctionBody(
    TInfoSinkBase &out,
    const ImageFunctionHLSL::ImageFunction &imageFunction,
    const ImmutableString &imageReference)
{
    if (IsImage3D(imageFunction.image) || IsImage2DArray(imageFunction.image) ||
        IsImageCube(imageFunction.image))
    {
        out << "    const uint index = imageIndex - " << GetImageIndexOffset(imageFunction, "")
            << ";\n";
        // "depth" stores either the number of layers in an array texture or 3D depth
        out << "    uint width; uint height; uint depth;\n";
        out << "    " << imageReference << "[index].GetDimensions(width, height, depth);\n";
    }
    else if (IsImage2D(imageFunction.image))
    {
        GenerateMacroBegin(out, imageFunction, "");
        out << "    const uint index = imageIndex - " << GetImageIndexOffset(imageFunction, "")
            << ";\n";
        out << "    uint width; uint height;\n";
        out << "    " << imageReference << "[index].GetDimensions(width, height);\n";
        out << "    return int2(width, height);\n";
        GenerateMacroEnd(out);

        GenerateMacroBegin(out, imageFunction, "2D");
        out << "    if (imageIndex >= " << GetImageIndexOffset(imageFunction, "2D")
            << " && imageIndex < " << GetImageIndexOffset(imageFunction, "3D") << ")\n";
        out << "    {\n";
        out << "        const uint index = imageIndex - "
            << GetImageIndexOffset(imageFunction, "2D") << ";\n";
        out << "        uint width; uint height;\n";
        out << "        " << imageReference << "2D[index].GetDimensions(width, height);\n";
        out << "        return int2(width, height);\n";
        out << "    }\n";
        GenerateMacroEnd(out);

        GenerateMacroBegin(out, imageFunction, "3D");
        out << "    if (imageIndex >= " << GetImageIndexOffset(imageFunction, "3D")
            << " && imageIndex < " << GetImageIndexOffset(imageFunction, "2DArray") << ")\n";
        out << "    {\n";
        out << "        const uint index = imageIndex - "
            << GetImageIndexOffset(imageFunction, "3D") << ";\n";
        out << "        uint width; uint height; uint depth;\n";
        out << "        " << imageReference << "3D[index].GetDimensions(width, height, depth);\n";
        out << "        return int2(width, height);\n";
        out << "    }\n";
        GenerateMacroEnd(out);

        GenerateMacroBegin(out, imageFunction, "2DArray");
        out << "    if (imageIndex >= " << GetImageIndexOffset(imageFunction, "2DArray") << ")\n";
        out << "    {\n";
        out << "        const uint index = imageIndex - "
            << GetImageIndexOffset(imageFunction, "2DArray") << ";\n";
        out << "        uint width; uint height; uint depth;\n";
        out << "        " << imageReference
            << "2DArray[index].GetDimensions(width, height, depth);\n";
        out << "        return int2(width, height);\n";
        out << "    }\n";
        GenerateMacroEnd(out);
    }
    else
        UNREACHABLE();
}

// static
void ImageFunctionHLSL::OutputImageLoadFunctionBody(
    TInfoSinkBase &out,
    const ImageFunctionHLSL::ImageFunction &imageFunction,
    const ImmutableString &imageReference)
{
    if (IsImage3D(imageFunction.image) || IsImage2DArray(imageFunction.image) ||
        IsImageCube(imageFunction.image))
    {
        out << "    const uint index = imageIndex - " << GetImageIndexOffset(imageFunction, "")
            << ";\n";
        out << "    return " << imageReference << "[index][uint3(p.x, p.y, p.z)];\n";
    }
    else if (IsImage2D(imageFunction.image))
    {
        GenerateMacroBegin(out, imageFunction, "");
        out << "    const uint index = imageIndex - " << GetImageIndexOffset(imageFunction, "")
            << ";\n";
        out << "    return " << imageReference << "[index][uint2(p.x, p.y)];\n";
        GenerateMacroEnd(out);

        GenerateMacroBegin(out, imageFunction, "2D");
        out << "    if (imageIndex >= " << GetImageIndexOffset(imageFunction, "2D")
            << " && imageIndex < " << GetImageIndexOffset(imageFunction, "3D") << ")\n";
        out << "    {\n";
        out << "        const uint index = imageIndex - "
            << GetImageIndexOffset(imageFunction, "2D") << ";\n";
        out << "        return " << imageReference << "2D[index][uint2(p.x, p.y)];\n";
        out << "    }\n";
        GenerateMacroEnd(out);

        GenerateMacroBegin(out, imageFunction, "3D");
        out << "    if (imageIndex >= " << GetImageIndexOffset(imageFunction, "3D")
            << " && imageIndex < " << GetImageIndexOffset(imageFunction, "2DArray") << ")\n";
        out << "    {\n";
        out << "        const uint index = imageIndex - "
            << GetImageIndexOffset(imageFunction, "3D") << ";\n";
        out << "        return " << imageReference
            << "3D[index][uint3(p.x, p.y, layer[imageIndex])];\n";
        out << "    }\n";
        GenerateMacroEnd(out);

        GenerateMacroBegin(out, imageFunction, "2DArray");
        out << "    if (imageIndex >= " << GetImageIndexOffset(imageFunction, "2DArray") << ")\n";
        out << "    {\n";
        out << "        const uint index = imageIndex - "
            << GetImageIndexOffset(imageFunction, "2DArray") << ";\n";
        out << "        return " << imageReference
            << "2DArray[index][uint3(p.x, p.y, layer[imageIndex])];\n";
        out << "    }\n";
        GenerateMacroEnd(out);
    }
    else
        UNREACHABLE();
}

// static
void ImageFunctionHLSL::OutputImageStoreFunctionBody(
    TInfoSinkBase &out,
    const ImageFunctionHLSL::ImageFunction &imageFunction,
    const ImmutableString &imageReference)
{
    if (IsImage3D(imageFunction.image) || IsImage2DArray(imageFunction.image) ||
        IsImageCube(imageFunction.image))
    {
        out << "    const uint index = imageIndex - " << GetImageIndexOffset(imageFunction, "")
            << ";\n";
        out << "    " << imageReference << "[index][p] = data;\n";
    }
    else if (IsImage2D(imageFunction.image))
    {
        GenerateMacroBegin(out, imageFunction, "");
        out << "    const uint index = imageIndex - " << GetImageIndexOffset(imageFunction, "")
            << ";\n";
        out << "    " << imageReference << "[index][p] = data;\n";
        GenerateMacroEnd(out);

        GenerateMacroBegin(out, imageFunction, "2D");
        out << "    if (imageIndex >= " << GetImageIndexOffset(imageFunction, "2D")
            << " && imageIndex < " << GetImageIndexOffset(imageFunction, "3D") << ")\n";
        out << "    {\n";
        out << "        const uint index = imageIndex - "
            << GetImageIndexOffset(imageFunction, "2D") << ";\n";
        out << "        " << imageReference << "2D[index][p] = data;\n";
        out << "    }\n";
        GenerateMacroEnd(out);

        GenerateMacroBegin(out, imageFunction, "3D");
        out << "    if (imageIndex >= " << GetImageIndexOffset(imageFunction, "3D")
            << " && imageIndex < " << GetImageIndexOffset(imageFunction, "2DArray") << ")\n";
        out << "    {\n";
        out << "        const uint index = imageIndex - "
            << GetImageIndexOffset(imageFunction, "3D") << ";\n";
        out << "        " << imageReference << "3D[index][uint3(p, layer[imageIndex])] = data;\n";
        out << "    }\n";
        GenerateMacroEnd(out);

        GenerateMacroBegin(out, imageFunction, "2DArray");
        out << "    if (imageIndex >= " << GetImageIndexOffset(imageFunction, "2DArray") << ")\n";
        out << "    {\n";
        out << "        const uint index = imageIndex - "
            << GetImageIndexOffset(imageFunction, "2DArray") << ";\n";
        out << "        " << imageReference
            << "2DArray[index][uint3(p, layer[imageIndex])] = data;\n";
        out << "    }\n";
        GenerateMacroEnd(out);
    }
    else
        UNREACHABLE();
}

TString ImageFunctionHLSL::ImageFunction::name() const
{
    TString name = "gl_image";
    if (readonly)
    {
        name += TextureTypeSuffix(image, imageInternalFormat);
    }
    else
    {
        name += RWTextureTypeSuffix(image, imageInternalFormat);
    }

    switch (method)
    {
        case Method::SIZE:
            name += "Size";
            break;
        case Method::LOAD:
            name += "Load";
            break;
        case Method::STORE:
            name += "Store";
            break;
        default:
            UNREACHABLE();
    }

    return name;
}

const char *ImageFunctionHLSL::ImageFunction::getReturnType() const
{
    if (method == ImageFunction::Method::SIZE)
    {
        switch (image)
        {
            case EbtImage2D:
            case EbtIImage2D:
            case EbtUImage2D:
            case EbtImageCube:
            case EbtIImageCube:
            case EbtUImageCube:
                return "int2";
            case EbtImage3D:
            case EbtIImage3D:
            case EbtUImage3D:
            case EbtImage2DArray:
            case EbtIImage2DArray:
            case EbtUImage2DArray:
                return "int3";
            default:
                UNREACHABLE();
        }
    }
    else if (method == ImageFunction::Method::LOAD)
    {
        switch (image)
        {
            case EbtImage2D:
            case EbtImage3D:
            case EbtImageCube:
            case EbtImage2DArray:
                return "float4";
            case EbtIImage2D:
            case EbtIImage3D:
            case EbtIImageCube:
            case EbtIImage2DArray:
                return "int4";
            case EbtUImage2D:
            case EbtUImage3D:
            case EbtUImageCube:
            case EbtUImage2DArray:
                return "uint4";
            default:
                UNREACHABLE();
        }
    }
    else if (method == ImageFunction::Method::STORE)
    {
        return "void";
    }
    else
    {
        UNREACHABLE();
    }
    return "";
}

ImageFunctionHLSL::ImageFunction::DataType ImageFunctionHLSL::ImageFunction::getDataType(
    TLayoutImageInternalFormat format) const
{
    switch (format)
    {
        case EiifRGBA32F:
        case EiifRGBA16F:
        case EiifR32F:
            return ImageFunction::DataType::FLOAT4;
        case EiifRGBA32UI:
        case EiifRGBA16UI:
        case EiifRGBA8UI:
        case EiifR32UI:
            return ImageFunction::DataType::UINT4;
        case EiifRGBA32I:
        case EiifRGBA16I:
        case EiifRGBA8I:
        case EiifR32I:
            return ImageFunction::DataType::INT4;
        case EiifRGBA8:
            return ImageFunction::DataType::UNORM_FLOAT4;
        case EiifRGBA8_SNORM:
            return ImageFunction::DataType::SNORM_FLOAT4;
        default:
            UNREACHABLE();
    }

    return ImageFunction::DataType::NONE;
}

bool ImageFunctionHLSL::ImageFunction::operator<(const ImageFunction &rhs) const
{
    return std::tie(image, type, method, readonly) <
           std::tie(rhs.image, rhs.type, rhs.method, rhs.readonly);
}

TString ImageFunctionHLSL::useImageFunction(const ImmutableString &name,
                                            const TBasicType &type,
                                            TLayoutImageInternalFormat imageInternalFormat,
                                            bool readonly)
{
    ASSERT(IsImage(type));
    ImageFunction imageFunction;
    imageFunction.image               = type;
    imageFunction.imageInternalFormat = imageInternalFormat;
    imageFunction.readonly            = readonly;
    imageFunction.type                = imageFunction.getDataType(imageInternalFormat);

    if (name == "imageSize")
    {
        imageFunction.method = ImageFunction::Method::SIZE;
    }
    else if (name == "imageLoad")
    {
        imageFunction.method = ImageFunction::Method::LOAD;
    }
    else if (name == "imageStore")
    {
        imageFunction.method = ImageFunction::Method::STORE;
    }
    else
        UNREACHABLE();

    mUsesImage.insert(imageFunction);
    return imageFunction.name();
}

void ImageFunctionHLSL::imageFunctionHeader(TInfoSinkBase &out)
{
    for (const ImageFunction &imageFunction : mUsesImage)
    {
        // Function header
        out << imageFunction.getReturnType() << " " << imageFunction.name() << "(";

        OutputImageFunctionArgumentList(out, imageFunction);

        out << ")\n"
               "{\n";

        ImmutableString imageReference("");
        GetTextureReference(imageFunction, &imageReference);

        if (imageFunction.method == ImageFunction::Method::SIZE)
        {
            OutputImageSizeFunctionBody(out, imageFunction, imageReference);
        }
        else if (imageFunction.method == ImageFunction::Method::LOAD)
        {
            OutputImageLoadFunctionBody(out, imageFunction, imageReference);
        }
        else
        {
            OutputImageStoreFunctionBody(out, imageFunction, imageReference);
        }

        out << "}\n"
               "\n";
    }
}

}  // namespace sh
