//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// IOSurfaceSurfaceVkMac.mm:
//    Implements methods from IOSurfaceSurfaceVkMac.
//

#include "libANGLE/renderer/vulkan/mac/IOSurfaceSurfaceVkMac.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/FramebufferVk.h"

#include <IOSurface/IOSurface.h>

namespace rx
{

namespace
{

struct IOSurfaceFormatInfo
{
    GLenum internalFormat;
    GLenum type;

    size_t componentBytes;

    GLenum nativeInternalFormat;
    GLenum nativeFormat;
    GLenum nativeType;
};

// clang-format off
static const IOSurfaceFormatInfo kIOSurfaceFormats[] = {
    {GL_RED,      GL_UNSIGNED_BYTE,  1, GL_RED,  GL_RED,  GL_UNSIGNED_BYTE           },
    {GL_R16UI,    GL_UNSIGNED_SHORT, 2, GL_RED,  GL_RED,  GL_UNSIGNED_SHORT          },
    {GL_RG,       GL_UNSIGNED_BYTE,  2, GL_RG,   GL_RG,   GL_UNSIGNED_BYTE           },
    // TODO(jonahr) figure out how to implement these formats
    // {GL_RGB,      GL_UNSIGNED_BYTE,  4, GL_RGBA, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV},
    // {GL_BGRA_EXT, GL_UNSIGNED_BYTE,  4, GL_RGBA, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV},
    {GL_RGBA,     GL_HALF_FLOAT,     8, GL_RGBA, GL_RGBA, GL_HALF_FLOAT              },
};
// clang-format on

int FindIOSurfaceFormatIndex(GLenum internalFormat, GLenum type)
{
    for (int i = 0; i < static_cast<int>(ArraySize(kIOSurfaceFormats)); ++i)
    {
        const auto &formatInfo = kIOSurfaceFormats[i];
        if (formatInfo.internalFormat == internalFormat && formatInfo.type == type)
        {
            return i;
        }
    }
    return -1;
}

}  // anonymous namespace

IOSurfaceSurfaceVkMac::IOSurfaceSurfaceVkMac(const egl::SurfaceState &state,
                                             EGLClientBuffer buffer,
                                             const egl::AttributeMap &attribs)
    : SurfaceVk(state), mIOSurface(nullptr), mWidth(0), mHeight(0), mPlane(0), mFormatIndex(-1)
{
    // Keep reference to the IOSurface so it doesn't get deleted while the pbuffer exists.
    mIOSurface = reinterpret_cast<IOSurfaceRef>(buffer);
    CFRetain(mIOSurface);

    // Extract attribs useful for the call to CGLTexImageIOSurface2D
    mWidth  = static_cast<int>(attribs.get(EGL_WIDTH));
    mHeight = static_cast<int>(attribs.get(EGL_HEIGHT));
    mPlane  = static_cast<int>(attribs.get(EGL_IOSURFACE_PLANE_ANGLE));

    EGLAttrib internalFormat = attribs.get(EGL_TEXTURE_INTERNAL_FORMAT_ANGLE);
    EGLAttrib type           = attribs.get(EGL_TEXTURE_TYPE_ANGLE);
    mFormatIndex =
        FindIOSurfaceFormatIndex(static_cast<GLenum>(internalFormat), static_cast<GLenum>(type));
    ASSERT(mFormatIndex >= 0);
}

IOSurfaceSurfaceVkMac::~IOSurfaceSurfaceVkMac()
{
    if (mIOSurface != nullptr)
    {
        CFRelease(mIOSurface);
        mIOSurface = nullptr;
    }
}

egl::Error IOSurfaceSurfaceVkMac::initialize(const egl::Display *display)
{
    DisplayVk *displayVk = vk::GetImpl(display);
    angle::Result result = initializeImpl(displayVk);
    return angle::ToEGL(result, displayVk, EGL_BAD_SURFACE);
}

angle::Result IOSurfaceSurfaceVkMac::initializeImpl(vk::Context *context)
{
    // TODO(jonahr): Get VkImage from sws with IOSurface instead of this block
    {
        VkExtent3D extents;
        extents.width  = mWidth;
        extents.height = mHeight;
        extents.depth  = 1;
        const vk::Format &format =
            context->getRenderer()->getFormat(kIOSurfaceFormats[mFormatIndex].internalFormat);
        GLint samples           = mState.config->samples;
        VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        ANGLE_TRY(
            image.init(context, gl::TextureType::_2D, extents, format, samples, usage, 0, 0, 1, 1));
        ANGLE_TRY(image.initMemory(context, context->getRenderer()->getMemoryProperties(),
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
    }

    mColorRenderTarget.init(&image, &imageViews, 0, 0);

    return angle::Result::Continue;
}

void IOSurfaceSurfaceVkMac::destroy(const egl::Display *display)
{
    DisplayVk *displayVk = vk::GetImpl(display);
    VkDevice device      = displayVk->getDevice();

    // It should be safe to immediately destroy the backing images of a surface on surface
    // destruction. If this assumption is incorrect, we could use the last submit serial
    // to determine when to destroy the surface.
    image.destroy(device);
    imageViews.destroy(device);
}

egl::Error IOSurfaceSurfaceVkMac::makeCurrent(const gl::Context *context)
{
    return egl::NoError();
}

egl::Error IOSurfaceSurfaceVkMac::unMakeCurrent(const gl::Context *context)
{
    ASSERT(context != nullptr);
    ContextVk *contextVk = vk::GetImpl(context);
    DisplayVk *displayVk = vk::GetImpl(context->getDisplay());
    angle::Result result = contextVk->flushImpl(nullptr);
    return angle::ToEGL(result, displayVk, EGL_BAD_SURFACE);
}

egl::Error IOSurfaceSurfaceVkMac::swap(const gl::Context *context)
{
    return egl::NoError();
}

egl::Error IOSurfaceSurfaceVkMac::postSubBuffer(const gl::Context *context,
                                                EGLint x,
                                                EGLint y,
                                                EGLint width,
                                                EGLint height)
{
    UNREACHABLE();
    return egl::NoError();
}

egl::Error IOSurfaceSurfaceVkMac::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNREACHABLE();
    return egl::NoError();
}

egl::Error IOSurfaceSurfaceVkMac::bindTexImage(const gl::Context *context,
                                               gl::Texture *texture,
                                               EGLint buffer)
{

    // TODO(jonahr)

    return egl::NoError();
}

egl::Error IOSurfaceSurfaceVkMac::releaseTexImage(const gl::Context *context, EGLint buffer)
{
    // TODO(jonahr)

    return egl::NoError();
}

egl::Error IOSurfaceSurfaceVkMac::getSyncValues(EGLuint64KHR *ust,
                                                EGLuint64KHR *msc,
                                                EGLuint64KHR *sbc)
{
    UNIMPLEMENTED();
    return egl::EglBadAccess();
}

void IOSurfaceSurfaceVkMac::setSwapInterval(EGLint interval)
{
    UNREACHABLE();
}

EGLint IOSurfaceSurfaceVkMac::getWidth() const
{
    return mWidth;
}

EGLint IOSurfaceSurfaceVkMac::getHeight() const
{
    return mHeight;
}

EGLint IOSurfaceSurfaceVkMac::isPostSubBufferSupported() const
{
    UNREACHABLE();
    return EGL_FALSE;
}

EGLint IOSurfaceSurfaceVkMac::getSwapBehavior() const
{
    // N/A because you can't MakeCurrent an IOSurface, return any valid value.
    return EGL_BUFFER_PRESERVED;
}

// static
bool IOSurfaceSurfaceVkMac::ValidateAttributes(EGLClientBuffer buffer,
                                               const egl::AttributeMap &attribs)
{
    IOSurfaceRef ioSurface = reinterpret_cast<IOSurfaceRef>(buffer);

    // The plane must exist for this IOSurface. IOSurfaceGetPlaneCount can return 0 for non-planar
    // ioSurfaces but we will treat non-planar like it is a single plane.
    size_t surfacePlaneCount = std::max(size_t(1), IOSurfaceGetPlaneCount(ioSurface));
    EGLAttrib plane          = attribs.get(EGL_IOSURFACE_PLANE_ANGLE);
    if (plane < 0 || static_cast<size_t>(plane) >= surfacePlaneCount)
    {
        return false;
    }

    // The width height specified must be at least (1, 1) and at most the plane size
    EGLAttrib width  = attribs.get(EGL_WIDTH);
    EGLAttrib height = attribs.get(EGL_HEIGHT);
    if (width <= 0 || static_cast<size_t>(width) > IOSurfaceGetWidthOfPlane(ioSurface, plane) ||
        height <= 0 || static_cast<size_t>(height) > IOSurfaceGetHeightOfPlane(ioSurface, plane))
    {
        return false;
    }

    // Find this IOSurface format
    EGLAttrib internalFormat = attribs.get(EGL_TEXTURE_INTERNAL_FORMAT_ANGLE);
    EGLAttrib type           = attribs.get(EGL_TEXTURE_TYPE_ANGLE);

    int formatIndex =
        FindIOSurfaceFormatIndex(static_cast<GLenum>(internalFormat), static_cast<GLenum>(type));

    if (formatIndex < 0)
    {
        return false;
    }

    // Check that the format matches this IOSurface plane
    if (IOSurfaceGetBytesPerElementOfPlane(ioSurface, plane) !=
        kIOSurfaceFormats[formatIndex].componentBytes)
    {
        return false;
    }

    return true;
}

FramebufferImpl *IOSurfaceSurfaceVkMac::createDefaultFramebuffer(const gl::Context *context,
                                                                 const gl::FramebufferState &state)
{
    RendererVk *renderer = vk::GetImpl(context)->getRenderer();

    // Use a user FBO for a pbuffer surface
    return FramebufferVk::CreateUserFBO(renderer, state);
}

}  // namespace rx
