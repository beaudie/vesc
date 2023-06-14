//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageMtl.cpp:
//    Implements the class methods for ImageMtl.
//

#include "libANGLE/renderer/metal/ImageMtl.h"

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"
#include "libANGLE/renderer/metal/RenderBufferMtl.h"
#include "libANGLE/renderer/metal/TextureMtl.h"

namespace rx
{

namespace
{

struct IOSurfaceFormatInfo
{
    GLenum internalFormat;
    GLenum type;
    size_t componentBytes;

    angle::FormatID angleFormatId;
    MTLPixelFormat metalPixelFormat;
};

// clang-format off
constexpr std::array<IOSurfaceFormatInfo, 8> kIOSurfaceFormats = {{
    {GL_RED,      GL_UNSIGNED_BYTE,               1, angle::FormatID::R8_UNORM,           MTLPixelFormatR8Unorm     },
    {GL_RED,      GL_UNSIGNED_SHORT,              2, angle::FormatID::R16_UNORM,          MTLPixelFormatR16Unorm    },
    {GL_RG,       GL_UNSIGNED_BYTE,               2, angle::FormatID::R8G8_UNORM,         MTLPixelFormatRG8Unorm    },
    {GL_RG,       GL_UNSIGNED_SHORT,              4, angle::FormatID::R16G16_UNORM,       MTLPixelFormatRG16Unorm   },
    {GL_RGB,      GL_UNSIGNED_BYTE,               4, angle::FormatID::B8G8R8A8_UNORM,     MTLPixelFormatBGRA8Unorm  },
    {GL_BGRA_EXT, GL_UNSIGNED_BYTE,               4, angle::FormatID::B8G8R8A8_UNORM,     MTLPixelFormatBGRA8Unorm  },
    {GL_RGBA,     GL_HALF_FLOAT,                  8, angle::FormatID::R16G16B16A16_FLOAT, MTLPixelFormatRGBA16Float },
    {GL_RGB10_A2, GL_UNSIGNED_INT_2_10_10_10_REV, 4, angle::FormatID::B10G10R10A2_UNORM,  MTLPixelFormatBGR10A2Unorm},
}};
// clang-format on

int FindIOSurfaceFormatIndex(GLenum internalFormat, GLenum type)
{
    for (int i = 0; i < static_cast<int>(kIOSurfaceFormats.size()); ++i)
    {
        const auto &formatInfo = kIOSurfaceFormats[i];
        if (formatInfo.internalFormat == internalFormat && formatInfo.type == type)
        {
            return i;
        }
    }
    return -1;
}

}  // namespace

// TextureImageSiblingMtl implementation
TextureImageSiblingMtl::TextureImageSiblingMtl(GLenum buftype,
                                               EGLClientBuffer buffer,
                                               const egl::AttributeMap &attribs)
    : mBuftype(buftype), mBuffer(buffer)
{
    if (mBuftype == EGL_IOSURFACE_ANGLE)
    {
        IOSurfaceRef surface = (__bridge IOSurfaceRef)(mBuffer);

        auto plane       = attribs.getAsInt(EGL_IOSURFACE_PLANE_ANGLE);
        mIOSurfaceWidth  = attribs.get(EGL_WIDTH, IOSurfaceGetWidthOfPlane(surface, plane));
        mIOSurfaceHeight = attribs.get(EGL_HEIGHT, IOSurfaceGetHeightOfPlane(surface, plane));
        ASSERT(mIOSurfaceWidth > 0);
        ASSERT(mIOSurfaceHeight > 0);
        mIOSurfacePlane = plane;

        // Find this IOSurface format
        GLenum internalFormat = attribs.getAsGLenum(EGL_TEXTURE_INTERNAL_FORMAT_ANGLE);
        GLenum type           = attribs.getAsGLenum(EGL_TEXTURE_TYPE_ANGLE);

        mIOSurfaceFormatIdx = FindIOSurfaceFormatIndex(static_cast<GLenum>(internalFormat),
                                                       static_cast<GLenum>(type));
        ASSERT(mIOSurfaceFormatIdx >= 0);
    }
}

TextureImageSiblingMtl::~TextureImageSiblingMtl() {}

static bool ValidateIOSurfaceClientBuffer(const DisplayMtl *display,
                                          EGLClientBuffer buffer,
                                          const egl::AttributeMap &attribs)
{
    IOSurfaceRef surface = (__bridge IOSurfaceRef)(buffer);

    // The plane must exist for this IOSurface. IOSurfaceGetPlaneCount can return
    // 0 for non-planar IOSurfaces but we will treat non-planar like it is a single
    // plane.
    size_t surfacePlaneCount = std::max(1ul, IOSurfaceGetPlaneCount(surface));
    EGLAttrib plane          = attribs.get(EGL_IOSURFACE_PLANE_ANGLE);
    if (plane < 0 || static_cast<size_t>(plane) > surfacePlaneCount)
    {
        return false;
    }

    // The width and height specified must be at least (1, 1) and at most the
    // plane size.
    auto surfaceWidth  = IOSurfaceGetWidthOfPlane(surface, plane);
    auto surfaceHeight = IOSurfaceGetHeightOfPlane(surface, plane);

    EGLAttrib width  = attribs.get(EGL_WIDTH, surfaceWidth);
    EGLAttrib height = attribs.get(EGL_HEIGHT, surfaceHeight);

    if (width <= 0 || static_cast<size_t>(width) > surfaceWidth || height <= 0 ||
        static_cast<size_t>(height) > surfaceHeight)
    {
        return false;
    }

    // Find this IOSurface format
    GLenum internalFormat = attribs.getAsGLenum(EGL_TEXTURE_INTERNAL_FORMAT_ANGLE);
    GLenum type           = attribs.getAsGLenum(EGL_TEXTURE_TYPE_ANGLE);

    int formatIndex = FindIOSurfaceFormatIndex(internalFormat, type);
    if (formatIndex < 0)
    {
        return false;
    }

    // FIXME: Check that the format matches this IOSurface plane for pixel formats that we know of.
    // We could map IOSurfaceGetPixelFormat to expected type plane and format type.
    // However, the caller might supply us non-public pixel format, which makes exhaustive checks
    // problematic.
    if (IOSurfaceGetBytesPerElementOfPlane(surface, plane) !=
        kIOSurfaceFormats[formatIndex].componentBytes)
    {
        WARN() << "IOSurface bytes per elements does not match the pbuffer internal format.";
    }

    return true;
}

static bool ValidateMtlTextureClientBuffer(const DisplayMtl *display, EGLClientBuffer buffer)
{
    id<MTLTexture> texture = (__bridge id<MTLTexture>)(buffer);
    if (!texture || texture.device != display->getMetalDevice())
    {
        return false;
    }

    if (texture.textureType != MTLTextureType2D && texture.textureType != MTLTextureTypeCube)
    {
        return false;
    }

    angle::FormatID angleFormatId = mtl::Format::MetalToAngleFormatID(texture.pixelFormat);
    const mtl::Format &format     = display->getPixelFormat(angleFormatId);
    if (!format.valid())
    {
        ERR() << "Unrecognized format";
        // Not supported
        return false;
    }

    return true;
}

// Static
bool TextureImageSiblingMtl::ValidateClientBuffer(const DisplayMtl *display,
                                                  GLenum buftype,
                                                  EGLClientBuffer buffer,
                                                  const egl::AttributeMap &attribs)
{
    switch (buftype)
    {
        case EGL_IOSURFACE_ANGLE:
            return ValidateIOSurfaceClientBuffer(display, buffer, attribs);
        case EGL_METAL_TEXTURE_ANGLE:
            return ValidateMtlTextureClientBuffer(display, buffer);
        default:
            return false;
    }
}

egl::Error TextureImageSiblingMtl::initialize(const egl::Display *display)
{
    DisplayMtl *displayMtl = mtl::GetImpl(display);
    if (initImpl(displayMtl) != angle::Result::Continue)
    {
        return egl::EglBadParameter();
    }

    return egl::NoError();
}

angle::Result TextureImageSiblingMtl::initImpl(DisplayMtl *displayMtl)
{
    angle::FormatID angleFormatId = angle::FormatID::NONE;

    switch (mBuftype)
    {
        case EGL_IOSURFACE_ANGLE:
            ANGLE_MTL_OBJC_SCOPE
            {
                angleFormatId         = kIOSurfaceFormats[mIOSurfaceFormatIdx].angleFormatId;
                auto metalPixelFormat = kIOSurfaceFormats[mIOSurfaceFormatIdx].metalPixelFormat;
                auto texDesc =
                    [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:metalPixelFormat
                                                                       width:mIOSurfaceWidth
                                                                      height:mIOSurfaceHeight
                                                                   mipmapped:NO];

                texDesc.usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;

                mNativeTexture = mtl::Texture::MakeFromMetal([displayMtl->getMetalDevice()
                    newTextureWithDescriptor:texDesc
                                   iosurface:(__bridge IOSurfaceRef)(mBuffer)plane
                                            :mIOSurfacePlane]);
            }
            break;

        case EGL_METAL_TEXTURE_ANGLE:
            mNativeTexture = mtl::Texture::MakeFromMetal((__bridge id<MTLTexture>)(mBuffer));
            angleFormatId  = mtl::Format::MetalToAngleFormatID(mNativeTexture->pixelFormat());
            break;

        default:
            UNIMPLEMENTED();
            return angle::Result::Stop;
    }

    mFormat = displayMtl->getPixelFormat(angleFormatId);

    if (mNativeTexture)
    {
        size_t resourceSize = EstimateTextureSizeInBytes(
            mFormat, mNativeTexture->widthAt0(), mNativeTexture->heightAt0(),
            mNativeTexture->depthAt0(), mNativeTexture->samples(), mNativeTexture->mipmapLevels());
        mNativeTexture->setEstimatedByteSize(resourceSize);
    }

    mGLFormat = gl::Format(mFormat.intendedAngleFormat().glInternalFormat);

    mRenderable = mFormat.getCaps().depthRenderable || mFormat.getCaps().colorRenderable;

    mTextureable = mFormat.getCaps().filterable || mFormat.hasDepthOrStencilBits();

    return angle::Result::Continue;
}

void TextureImageSiblingMtl::onDestroy(const egl::Display *display)
{
    mNativeTexture = nullptr;
}

gl::Format TextureImageSiblingMtl::getFormat() const
{
    return mGLFormat;
}

bool TextureImageSiblingMtl::isRenderable(const gl::Context *context) const
{
    return mRenderable;
}

bool TextureImageSiblingMtl::isTexturable(const gl::Context *context) const
{
    return mTextureable;
}

gl::Extents TextureImageSiblingMtl::getSize() const
{
    return mNativeTexture ? mNativeTexture->sizeAt0() : gl::Extents(0, 0, 0);
}

size_t TextureImageSiblingMtl::getSamples() const
{
    uint32_t samples = mNativeTexture ? mNativeTexture->samples() : 0;
    return samples > 1 ? samples : 0;
}

bool TextureImageSiblingMtl::isYUV() const
{
    // NOTE(hqle): not supporting YUV image yet.
    return false;
}

bool TextureImageSiblingMtl::hasProtectedContent() const
{
    return false;
}

// ImageMtl implementation
ImageMtl::ImageMtl(const egl::ImageState &state, const gl::Context *context) : ImageImpl(state) {}

ImageMtl::~ImageMtl() {}

void ImageMtl::onDestroy(const egl::Display *display)
{
    mNativeTexture = nullptr;
}

egl::Error ImageMtl::initialize(const egl::Display *display)
{
    if (mState.target == EGL_IOSURFACE_ANGLE || mState.target == EGL_METAL_TEXTURE_ANGLE)
    {
        const TextureImageSiblingMtl *externalImageSibling =
            GetImplAs<TextureImageSiblingMtl>(GetAs<egl::ExternalImageSibling>(mState.source));

        mNativeTexture = externalImageSibling->getTexture();

        switch (mNativeTexture->textureType())
        {
            case MTLTextureType2D:
                mImageTextureType = gl::TextureType::_2D;
                break;
            case MTLTextureTypeCube:
                mImageTextureType = gl::TextureType::CubeMap;
                break;
            default:
                UNREACHABLE();
        }

        mImageLevel = 0;
        mImageLayer = 0;
    }
    else
    {
        UNREACHABLE();
        return egl::EglBadAccess();
    }

    return egl::NoError();
}

angle::Result ImageMtl::orphan(const gl::Context *context, egl::ImageSibling *sibling)
{
    if (sibling == mState.source)
    {
        mNativeTexture = nullptr;
    }

    return angle::Result::Continue;
}

}  // namespace rx
