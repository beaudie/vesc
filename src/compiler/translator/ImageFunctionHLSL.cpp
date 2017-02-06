//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TextureFunctionHLSL: Class for writing implementations of ESSL texture functions into HLSL
// output. Some of the implementations are straightforward and just call the HLSL equivalent of the
// ESSL texture function, others do more work to emulate ESSL texture sampling or size query
// behavior.
//

#include "compiler/translator/ImageFunctionHLSL.h"

#include "compiler/translator/UtilsHLSL.h"

namespace sh
{

namespace
{
void OutputImageFunctionArgumentList(TInfoSinkBase &out,
                                     const ImageFunctionHLSL::ImageFunction &imageFunction)
{
    out << "const uint imageIndex";

    if (imageFunction.method == ImageFunctionHLSL::ImageFunction::LOAD)
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
    }
    else if (imageFunction.method == ImageFunctionHLSL::ImageFunction::STORE)
    {
        switch (imageFunction.image)
        {
            case EbtImage2D:
                out << ", int2 p, float4 data";
                break;
            case EbtIImage2D:
                out << ", int2 p, int4 data";
                break;
            case EbtUImage2D:
                out << ", int2 p, uint4 data";
                break;
            case EbtImage3D:
                out << ", int3 p, float4 data";
                break;
            case EbtIImage3D:
                out << ", int3 p, int4 data";
                break;
            case EbtUImage3D:
                out << ", int3 p, uint data";
                break;
            case EbtImageCube:
                out << ", int3 p, float4 data";
                break;
            case EbtIImageCube:
                out << ", int3 p, int4 data";
                break;
            case EbtUImageCube:
                out << ", int3 p, uint4 data";
                break;
            case EbtImage2DArray:
                out << ", int3 p, float4 data";
                break;
            case EbtIImage2DArray:
                out << ", int3 p, int4 data";
                break;
            case EbtUImage2DArray:
                out << ", int3 p, uint4 data";
                break;
            default:
                UNREACHABLE();
        }
    }
}

void GetImageReference(TInfoSinkBase &out,
                       const ImageFunctionHLSL::ImageFunction &imageFunction,
                       TString *imageReference)
{
    if (imageFunction.readOnly)
    {
        HLSLSRVTextureGroup srvTextureGroup =
            SRVTextureGroup(imageFunction.image, imageFunction.imageInternalFormat);
        TString suffix = SRVTextureGroupSuffix(srvTextureGroup);
        if (srvTextureGroup == HLSL_SRV_TEXTURE_2D_FLOAT4)
        {
            *imageReference = TString("images") + suffix + "[imageIndex]";
        }
        else
        {
            out << "    const uint srvIndex = imageIndex - imageIndexOffset" << suffix << ";\n";
            *imageReference = TString("images") + suffix + "[srvIndex]";
        }
    }
    else
    {
        HLSLUAVTextureGroup uavTextureGroup =
            UAVTextureGroup(imageFunction.image, imageFunction.imageInternalFormat);
        TString suffix = UAVTextureGroupSuffix(uavTextureGroup);
        if (uavTextureGroup == HLSL_UAV_TEXTURE_2D_FLOAT4)
        {
            *imageReference = TString("images") + suffix + "[imageIndex]";
        }
        else
        {
            out << "    const uint uavIndex = imageIndex - imageIndexOffset" << suffix << ";\n";
            *imageReference = TString("images") + suffix + "[uavIndex]";
        }
    }
}

void OutputImageSizeFunctionBody(TInfoSinkBase &out,
                                 const ImageFunctionHLSL::ImageFunction &imageFunction,
                                 const TString &imageReference)
{
    if (IsImage3D(imageFunction.image) || IsImage2DArray(imageFunction.image))
    {
        // "depth" stores either the number of layers in an array texture or 3D depth
        out << "    uint width; uint height; uint depth;\n"
            << "    " << imageReference << ".GetDimensions(width, height, depth);\n";
    }
    else if (IsImage2D(imageFunction.image) || IsImageCube(imageFunction.image))
    {
        out << "    uint width; uint height;\n"
            << "    " << imageReference << ".GetDimensions(width, height);\n";
    }
    else
        UNREACHABLE();

    if (strcmp(imageFunction.getReturnType(), "int3") == 0)
    {
        out << "    return int3(width, height, depth);\n";
    }
    else
    {
        out << "    return int2(width, height);\n";
    }
}

void OutputImageLoadFunctionBody(TInfoSinkBase &out,
                                 const ImageFunctionHLSL::ImageFunction &imageFunction,
                                 const TString &imageReference)
{
    if (IsImage3D(imageFunction.image) || IsImage2DArray(imageFunction.image) ||
        IsImageCube(imageFunction.image))
    {
        // "depth" stores either the number of layers in an array texture or 3D depth
        out << "    uint width; uint height; uint depth;\n"
            << "    " << imageReference << ".GetDimensions(width, height, depth);\n"
            << "    return " << imageReference << ".Load(p.x * width + p.y * height + p.z);\n";
    }
    else if (IsImage2D(imageFunction.image))
    {
        out << "    uint width; uint height;\n"
            << "    " << imageReference << ".GetDimensions(width, height);\n"
            << "    return " << imageReference << ".Load(p.x * width + p.y);\n";
    }
    else
        UNREACHABLE();
}

void OutputImageStoreFunctionBody(TInfoSinkBase &out,
                                  const ImageFunctionHLSL::ImageFunction &imageFunction,
                                  const TString &imageReference)
{
    if (IsImage3D(imageFunction.image) || IsImage2DArray(imageFunction.image) ||
        IsImageCube(imageFunction.image) || IsImage2D(imageFunction.image))
    {
        out << "    " << imageReference << "[p] = data;\n";
    }
    else
        UNREACHABLE();
}

}  // Anonymous namespace

TString ImageFunctionHLSL::ImageFunction::name() const
{
    TString name = "gl_image";

    if (readOnly)
    {
        name += SRVTextureGroupSuffix(SRVTextureGroup(image, imageInternalFormat));
    }
    else
    {
        name += UAVTextureGroupSuffix(UAVTextureGroup(image, imageInternalFormat));
    }

    switch (method)
    {
        case SIZE:
            name += "Size";
            break;
        case LOAD:
            name += "Load";
            break;
        case STORE:
            name += "Store";
            break;
        default:
            UNREACHABLE();
    }

    return name;
}

const char *ImageFunctionHLSL::ImageFunction::getReturnType() const
{
    if (method == ImageFunction::SIZE)
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
    else if (method == ImageFunction::LOAD)
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
    else if (method == ImageFunction::STORE)
    {
        switch (image)
        {
            case EbtImage2D:
            case EbtImage3D:
            case EbtImageCube:
            case EbtImage2DArray:
            case EbtIImage2D:
            case EbtIImage3D:
            case EbtIImageCube:
            case EbtIImage2DArray:
            case EbtUImage2D:
            case EbtUImage3D:
            case EbtUImageCube:
            case EbtUImage2DArray:
                return "void";
            default:
                UNREACHABLE();
        }
    }
    else
        UNREACHABLE();
    return "";
}

bool ImageFunctionHLSL::ImageFunction::operator<(const ImageFunction &rhs) const
{
    return std::tie(image, imageInternalFormat, readOnly, method) <
           std::tie(rhs.image, rhs.imageInternalFormat, rhs.readOnly, rhs.method);
}

TString ImageFunctionHLSL::useImageFunction(const TString &name,
                                            const TBasicType &type,
                                            TLayoutImageInternalFormat imageInternalFormat,
                                            bool readOnly)
{
    ImageFunction imageFunction;
    imageFunction.image               = type;
    imageFunction.imageInternalFormat = imageInternalFormat;
    imageFunction.readOnly            = readOnly;
    imageFunction.method              = ImageFunction::IMPLICIT;

    if (name == "imageSize")
    {
        imageFunction.method = ImageFunction::SIZE;
    }
    else if (name == "imageLoad")
    {
        imageFunction.method = ImageFunction::LOAD;
    }
    else if (name == "imageStore")
    {
        imageFunction.method = ImageFunction::STORE;
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

        TString imageReference;
        GetImageReference(out, imageFunction, &imageReference);

        if (imageFunction.method == ImageFunction::SIZE)
        {
            OutputImageSizeFunctionBody(out, imageFunction, imageReference);
        }
        else if (imageFunction.method == ImageFunction::LOAD)
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
