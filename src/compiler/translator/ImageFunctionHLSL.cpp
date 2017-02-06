//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageFunctionHLSL: Class for writing implementations of ESSL image functions into HLSL output.
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
    if (imageFunction.readonly)
    {
        out << TextureString(imageFunction.image, imageFunction.imageInternalFormat) << " tex";
    }
    else
    {
        out << RWTextureString(imageFunction.image, imageFunction.imageInternalFormat) << " tex";
    }

    if (imageFunction.method == ImageFunctionHLSL::ImageFunction::Method::LOAD)
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
    else if (imageFunction.method == ImageFunctionHLSL::ImageFunction::Method::STORE)
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
                out << ", int3 p, uint4 data";
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
    *imageReference = TString("tex");
}

void OutputImageSizeFunctionBody(TInfoSinkBase &out,
                                 const ImageFunctionHLSL::ImageFunction &imageFunction,
                                 const TString &imageReference)
{
    if (IsImage3D(imageFunction.image) || IsImage2DArray(imageFunction.image) ||
        IsImageCube(imageFunction.image))
    {
        // "depth" stores either the number of layers in an array texture or 3D depth
        out << "    uint width; uint height; uint depth;\n"
            << "    " << imageReference << ".GetDimensions(width, height, depth);\n";
    }
    else if (IsImage2D(imageFunction.image))
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
        out << "    return " << imageReference << "[uint3(p.x, p.y, p.z)];\n";
    }
    else if (IsImage2D(imageFunction.image))
    {
        out << "    return " << imageReference << "[uint2(p.x, p.y)];\n";
    }
    else
        UNREACHABLE();
}

void OutputImageStoreFunctionBody(TInfoSinkBase &out,
                                  const ImageFunctionHLSL::ImageFunction &imageFunction,
                                  const TString &imageReference)
{
    if (IsImage3D(imageFunction.image) || IsImage2DArray(imageFunction.image) ||
        IsImage2D(imageFunction.image) || IsImageCube(imageFunction.image))
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
    return std::tie(image, imageInternalFormat, readonly, method) <
           std::tie(rhs.image, rhs.imageInternalFormat, rhs.readonly, rhs.method);
}

TString ImageFunctionHLSL::useImageFunction(const TString &name,
                                            const TBasicType &type,
                                            TLayoutImageInternalFormat imageInternalFormat,
                                            bool readonly)
{
    ImageFunction imageFunction;
    imageFunction.image               = type;
    imageFunction.imageInternalFormat = imageInternalFormat;
    imageFunction.readonly            = readonly;

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

        TString imageReference;
        GetImageReference(out, imageFunction, &imageReference);

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
