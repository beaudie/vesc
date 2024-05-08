//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SurfaceWgpu.cpp:
//    Implements the class methods for SurfaceWgpu.
//

#include "libANGLE/renderer/wgpu/SurfaceWgpu.h"

#include "common/debug.h"

#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/wgpu/DisplayWgpu.h"
#include "libANGLE/renderer/wgpu/FramebufferWgpu.h"

namespace rx
{

constexpr wgpu::TextureUsage kSurfaceTextureUsage =
    wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment |
    wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;

SurfaceWgpu::SurfaceWgpu(const egl::SurfaceState &surfaceState) : SurfaceImpl(surfaceState) {}

SurfaceWgpu::~SurfaceWgpu() {}

OffscreenSurfaceWgpu::OffscreenSurfaceWgpu(const egl::SurfaceState &surfaceState)
    : SurfaceWgpu(surfaceState),
      mWidth(surfaceState.attributes.getAsInt(EGL_WIDTH, 0)),
      mHeight(surfaceState.attributes.getAsInt(EGL_HEIGHT, 0))
{}

OffscreenSurfaceWgpu::~OffscreenSurfaceWgpu() {}

egl::Error OffscreenSurfaceWgpu::initialize(const egl::Display *display)
{
    return angle::ResultToEGL(initializeImpl(display));
}

egl::Error OffscreenSurfaceWgpu::swap(const gl::Context *context)
{
    UNREACHABLE();
    return egl::NoError();
}

egl::Error OffscreenSurfaceWgpu::bindTexImage(const gl::Context *context,
                                              gl::Texture *texture,
                                              EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

egl::Error OffscreenSurfaceWgpu::releaseTexImage(const gl::Context *context, EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

void OffscreenSurfaceWgpu::setSwapInterval(EGLint interval) {}

EGLint OffscreenSurfaceWgpu::getWidth() const
{
    return mWidth;
}

EGLint OffscreenSurfaceWgpu::getHeight() const
{
    return mHeight;
}

EGLint OffscreenSurfaceWgpu::getSwapBehavior() const
{
    return EGL_BUFFER_DESTROYED;
}

angle::Result OffscreenSurfaceWgpu::initializeContents(const gl::Context *context,
                                                       GLenum binding,
                                                       const gl::ImageIndex &imageIndex)
{
    UNIMPLEMENTED();
    return angle::Result::Continue;
}

egl::Error OffscreenSurfaceWgpu::attachToFramebuffer(const gl::Context *context,
                                                     gl::Framebuffer *framebuffer)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

egl::Error OffscreenSurfaceWgpu::detachFromFramebuffer(const gl::Context *context,
                                                       gl::Framebuffer *framebuffer)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

angle::Result OffscreenSurfaceWgpu::getAttachmentRenderTarget(
    const gl::Context *context,
    GLenum binding,
    const gl::ImageIndex &imageIndex,
    GLsizei samples,
    FramebufferAttachmentRenderTarget **rtOut)
{
    if (binding == GL_BACK)
    {
        *rtOut = &mColorAttachment.renderTarget;
    }
    else
    {
        ASSERT(binding == GL_DEPTH || binding == GL_STENCIL || binding == GL_DEPTH_STENCIL);
        *rtOut = &mDepthStencilAttachment.renderTarget;
    }

    return angle::Result::Continue;
}

angle::Result OffscreenSurfaceWgpu::initializeImpl(const egl::Display *display)
{
    DisplayWgpu *displayWgpu = webgpu::GetImpl(display);
    wgpu::Device &device     = displayWgpu->getDevice();

    const egl::Config *config = mState.config;

    if (config->renderTargetFormat != GL_NONE)
    {
        ASSERT(config->renderTargetFormat == GL_RGBA8);
        wgpu::TextureDescriptor desc = mColorAttachment.texture.createTextureDescriptor(
            kSurfaceTextureUsage, wgpu::TextureDimension::e2D,
            {static_cast<uint32_t>(mWidth), static_cast<uint32_t>(mHeight), 1},
            wgpu::TextureFormat::RGBA8Unorm, 1, 1);
        ANGLE_TRY(mColorAttachment.texture.initImage(device, gl::LevelIndex(0), desc));

        wgpu::TextureViewDescriptor textureViewDesc;
        textureViewDesc.format          = desc.format;
        textureViewDesc.dimension       = wgpu::TextureViewDimension::e2D;
        textureViewDesc.baseMipLevel    = 0;
        textureViewDesc.mipLevelCount   = 1;
        textureViewDesc.baseArrayLayer  = 0;
        textureViewDesc.arrayLayerCount = 1;
        textureViewDesc.aspect          = wgpu::TextureAspect::All;

        wgpu::TextureView view = mColorAttachment.texture.getTexture().CreateView(&textureViewDesc);
        mColorAttachment.renderTarget.set(&mColorAttachment.texture, view);
    }

    if (config->depthStencilFormat != GL_NONE)
    {
        UNIMPLEMENTED();
    }

    return angle::Result::Continue;
}

WindowSurfaceWgpu::WindowSurfaceWgpu(const egl::SurfaceState &surfaceState,
                                     EGLNativeWindowType window)
    : SurfaceWgpu(surfaceState), mNativeWindow(window)
{}

WindowSurfaceWgpu::~WindowSurfaceWgpu() {}

egl::Error WindowSurfaceWgpu::initialize(const egl::Display *display)
{
    return angle::ResultToEGL(initializeImpl(display));
}

void WindowSurfaceWgpu::destroy(const egl::Display *display)
{
    mSurface = nullptr;
    mColorAttachment.renderTarget.reset();
    mColorAttachment.texture.resetImage();

    // DisplayWgpu *displayWgpu = webgpu::GetImpl(display);
    // wgpu::Device &device     = displayWgpu->getDevice();
    // device.Tick();
}

egl::Error WindowSurfaceWgpu::swap(const gl::Context *context)
{
    return angle::ResultToEGL(swapImpl(context->getDisplay()));
}

egl::Error WindowSurfaceWgpu::bindTexImage(const gl::Context *context,
                                           gl::Texture *texture,
                                           EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

egl::Error WindowSurfaceWgpu::releaseTexImage(const gl::Context *context, EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

void WindowSurfaceWgpu::setSwapInterval(EGLint interval)
{
    UNIMPLEMENTED();
}

EGLint WindowSurfaceWgpu::getWidth() const
{
    return mCurrentSurfaceSize.width;
}

EGLint WindowSurfaceWgpu::getHeight() const
{
    return mCurrentSurfaceSize.height;
}

EGLint WindowSurfaceWgpu::getSwapBehavior() const
{
    UNIMPLEMENTED();
    return 0;
}

angle::Result WindowSurfaceWgpu::initializeContents(const gl::Context *context,
                                                    GLenum binding,
                                                    const gl::ImageIndex &imageIndex)
{
    UNIMPLEMENTED();
    return angle::Result::Continue;
}

egl::Error WindowSurfaceWgpu::attachToFramebuffer(const gl::Context *context,
                                                  gl::Framebuffer *framebuffer)
{
    return egl::NoError();
}

egl::Error WindowSurfaceWgpu::detachFromFramebuffer(const gl::Context *context,
                                                    gl::Framebuffer *framebuffer)
{
    return egl::NoError();
}

angle::Result WindowSurfaceWgpu::getAttachmentRenderTarget(
    const gl::Context *context,
    GLenum binding,
    const gl::ImageIndex &imageIndex,
    GLsizei samples,
    FramebufferAttachmentRenderTarget **rtOut)
{
    if (binding == GL_BACK)
    {
        *rtOut = &mColorAttachment.renderTarget;
    }
    else
    {
        ASSERT(binding == GL_DEPTH || binding == GL_STENCIL || binding == GL_DEPTH_STENCIL);
        *rtOut = &mDepthStencilAttachment.renderTarget;
    }

    return angle::Result::Continue;
}

angle::Result WindowSurfaceWgpu::initializeImpl(const egl::Display *display)
{
    ANGLE_TRY(createWgpuSurface(display, &mSurface));

    const egl::Config *config = mState.config;
    ASSERT(config->renderTargetFormat != GL_NONE);
    if (config->depthStencilFormat != GL_NONE)
    {
        UNIMPLEMENTED();
    }

    gl::Extents size;
    ANGLE_TRY(getCurrentWindowSize(display, &size));
    ANGLE_TRY(reconfigureSurface(display, size));
    ANGLE_TRY(updateCurrentTexture(display));

    return angle::Result::Continue;
}

angle::Result WindowSurfaceWgpu::reconfigureSurface(const egl::Display *display,
                                                    const gl::Extents &size)
{
    DisplayWgpu *displayWgpu = webgpu::GetImpl(display);
    wgpu::Device &device     = displayWgpu->getDevice();
    wgpu::Adapter &adapter   = displayWgpu->getAdapter();

    wgpu::TextureFormat format = mSurface.GetPreferredFormat(adapter);

    wgpu::SurfaceConfiguration surfaceConfig;
    surfaceConfig.device      = device;
    surfaceConfig.format      = format;
    surfaceConfig.usage       = kSurfaceTextureUsage;
    surfaceConfig.width       = size.width;
    surfaceConfig.height      = size.height;
    surfaceConfig.presentMode = wgpu::PresentMode::Mailbox;

    mSurface.Configure(&surfaceConfig);
    mCurrentSurfaceSize = size;

    return angle::Result::Continue;
}

angle::Result WindowSurfaceWgpu::updateCurrentTexture(const egl::Display *display)
{
    wgpu::SurfaceTexture surfaceTexture;
    mSurface.GetCurrentTexture(&surfaceTexture);
    ASSERT(surfaceTexture.status == wgpu::SurfaceGetCurrentTextureStatus::Success);

    ANGLE_TRY(mColorAttachment.texture.initExternal(surfaceTexture.texture));

    wgpu::TextureViewDescriptor textureViewDesc;
    textureViewDesc.format          = mColorAttachment.texture.getTexture().GetFormat();
    textureViewDesc.dimension       = wgpu::TextureViewDimension::e2D;
    textureViewDesc.baseMipLevel    = 0;
    textureViewDesc.mipLevelCount   = 1;
    textureViewDesc.baseArrayLayer  = 0;
    textureViewDesc.arrayLayerCount = 1;
    textureViewDesc.aspect          = wgpu::TextureAspect::All;

    wgpu::TextureView view = mColorAttachment.texture.getTexture().CreateView(&textureViewDesc);
    mColorAttachment.renderTarget.set(&mColorAttachment.texture, view);

    return angle::Result::Continue;
}

angle::Result WindowSurfaceWgpu::swapImpl(const egl::Display *display)
{
    mSurface.Present();

    gl::Extents currentSize;
    ANGLE_TRY(getCurrentWindowSize(display, &currentSize));

    if (currentSize != mCurrentSurfaceSize)
    {
        ANGLE_TRY(reconfigureSurface(display, currentSize));
    }

    ANGLE_TRY(updateCurrentTexture(display));

    return angle::Result::Continue;
}

}  // namespace rx
