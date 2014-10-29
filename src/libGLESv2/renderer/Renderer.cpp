//
// Copyright (c) 2012-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Renderer.cpp: Implements EGL dependencies for creating and destroying Renderer instances.

#include "common/utilities.h"
#include "libGLESv2/main.h"
#include "libGLESv2/Shader.h"
#include "libGLESv2/Program.h"
#include "libGLESv2/renderer/Renderer.h"
#include "libGLESv2/renderer/RendererImpl.h"

#include "third_party/trace_event/trace_event.h"

#include <EGL/eglext.h>

#if defined (ANGLE_ENABLE_D3D9)
#include "libGLESv2/renderer/d3d/d3d9/Renderer9.h"
#endif // ANGLE_ENABLE_D3D9

#if defined (ANGLE_ENABLE_D3D11)
#include "libGLESv2/renderer/d3d/d3d11/Renderer11.h"
#endif // ANGLE_ENABLE_D3D11

#if defined (ANGLE_TEST_CONFIG)
#define ANGLE_DEFAULT_D3D11 1
#endif

#if !defined(ANGLE_DEFAULT_D3D11)
// Enables use of the Direct3D 11 API for a default display, when available
#define ANGLE_DEFAULT_D3D11 0
#endif

namespace gl
{

Renderer::Renderer(rx::RendererImpl *implementation)
    : mImplementation(implementation)
{
    ASSERT(implementation);
}

Renderer::~Renderer()
{
    SafeDelete(mImplementation);
}

EGLint Renderer::initialize()
{
    return mImplementation->initialize();
}

bool Renderer::resetDevice()
{
    return mImplementation->resetDevice();
}

int Renderer::generateConfigs(rx::ConfigDesc **configDescList)
{
    return mImplementation->generateConfigs(configDescList);
}

void Renderer::deleteConfigs(rx::ConfigDesc *configDescList)
{
    mImplementation->deleteConfigs(configDescList);
}

gl::Error Renderer::sync(bool block)
{
    return mImplementation->sync(block);
}

gl::Error Renderer::drawArrays(const gl::Data &data, GLenum mode,
                              GLint first, GLsizei count, GLsizei instances)
{
    return mImplementation->drawArrays(data, mode, first, count, instances);
}

gl::Error Renderer::drawElements(const gl::Data &data, GLenum mode, GLsizei count, GLenum type,
                                 const GLvoid *indices, GLsizei instances,
                                 const rx::RangeUI &indexRange)
{
    return mImplementation->drawElements(data, mode, count, type, indices, instances, indexRange);
}

gl::Error Renderer::clear(const gl::ClearParameters &clearParams, gl::Framebuffer *frameBuffer)
{
    return mImplementation->clear(clearParams, frameBuffer);
}

gl::Error Renderer::readPixels(gl::Framebuffer *framebuffer, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format,
                               GLenum type, GLuint outputPitch, const gl::PixelPackState &pack, uint8_t *pixels)
{
    return mImplementation->readPixels(framebuffer, x, y, width, height, format, type, outputPitch, pack, pixels);
}

bool Renderer::getShareHandleSupport() const
{
    return mImplementation->getShareHandleSupport();
}

bool Renderer::getPostSubBufferSupport() const
{
    return mImplementation->getPostSubBufferSupport();
}

rx::ShaderImpl *Renderer::createShader(GLenum type)
{
    return mImplementation->createShader(type);
}

void Renderer::releaseShaderCompiler()
{
    mImplementation->releaseShaderCompiler();
}

rx::ProgramImpl *Renderer::createProgram()
{
    return mImplementation->createProgram();
}

rx::TextureImpl *Renderer::createTexture(GLenum target)
{
    return mImplementation->createTexture(target);
}

rx::BufferImpl *Renderer::createBuffer()
{
    return mImplementation->createBuffer();
}

rx::VertexArrayImpl *Renderer::createVertexArray()
{
    return mImplementation->createVertexArray();
}

rx::QueryImpl *Renderer::createQuery(GLenum type)
{
    return mImplementation->createQuery(type);
}

rx::FenceNVImpl *Renderer::createFenceNV()
{
    return mImplementation->createFenceNV();
}

rx::FenceSyncImpl *Renderer::createFenceSync()
{
    return mImplementation->createFenceSync();
}

rx::TransformFeedbackImpl* Renderer::createTransformFeedback()
{
    return mImplementation->createTransformFeedback();
}

void Renderer::notifyDeviceLost()
{
    return mImplementation->notifyDeviceLost();
}

bool Renderer::isDeviceLost()
{
    return mImplementation->isDeviceLost();
}

bool Renderer::testDeviceLost(bool notify)
{
    return mImplementation->testDeviceLost(notify);
}

bool Renderer::testDeviceResettable()
{
    return mImplementation->testDeviceResettable();
}

DWORD Renderer::getAdapterVendor() const
{
    return mImplementation->getAdapterVendor();
}

std::string Renderer::getRendererDescription() const
{
    return mImplementation->getRendererDescription();
}

GUID Renderer::getAdapterIdentifier() const
{
    return mImplementation->getAdapterIdentifier();
}

const Caps &Renderer::getRendererCaps() const
{
    return mImplementation->getRendererCaps();
}

const TextureCapsMap &Renderer::getRendererTextureCaps() const
{
    return mImplementation->getRendererTextureCaps();
}

const Extensions &Renderer::getRendererExtensions() const
{
    return mImplementation->getRendererExtensions();
}

const rx::Workarounds &Renderer::getWorkarounds() const
{
    return mImplementation->getWorkarounds();
}

// TODO(jmadill): needed by egl::Display, probably should be removed
int Renderer::getMajorShaderModel() const
{
    return mImplementation->getMajorShaderModel();
}

int Renderer::getMinSwapInterval() const
{
    return mImplementation->getMinSwapInterval();
}

int Renderer::getMaxSwapInterval() const
{
    return mImplementation->getMaxSwapInterval();
}

bool Renderer::getLUID(LUID *adapterLuid) const
{
    return mImplementation->getLUID(adapterLuid);
}

typedef gl::Renderer *(*CreateRendererFunction)(egl::Display*, EGLNativeDisplayType, EGLint);

template <typename RendererType>
gl::Renderer *CreateRenderer(egl::Display *display, EGLNativeDisplayType nativeDisplay, EGLint requestedDisplayType)
{
    return new gl::Renderer(new RendererType(display, nativeDisplay, requestedDisplayType));
}

}

extern "C"
{

gl::Renderer *glCreateRenderer(egl::Display *display, EGLNativeDisplayType nativeDisplay, EGLint requestedDisplayType)
{
    std::vector<gl::CreateRendererFunction> rendererCreationFunctions;

#   if defined(ANGLE_ENABLE_D3D11)
        if (nativeDisplay == EGL_D3D11_ELSE_D3D9_DISPLAY_ANGLE ||
            nativeDisplay == EGL_D3D11_ONLY_DISPLAY_ANGLE ||
            requestedDisplayType == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE ||
            requestedDisplayType == EGL_PLATFORM_ANGLE_TYPE_D3D11_WARP_ANGLE)
        {
            rendererCreationFunctions.push_back(gl::CreateRenderer<rx::Renderer11>);
        }
#   endif

#   if defined(ANGLE_ENABLE_D3D9)
        if (nativeDisplay == EGL_D3D11_ELSE_D3D9_DISPLAY_ANGLE ||
            requestedDisplayType == EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE)
        {
            rendererCreationFunctions.push_back(gl::CreateRenderer<rx::Renderer9>);
        }
#   endif

    if (nativeDisplay != EGL_D3D11_ELSE_D3D9_DISPLAY_ANGLE &&
        nativeDisplay != EGL_D3D11_ONLY_DISPLAY_ANGLE &&
        requestedDisplayType == EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE)
    {
        // The default display is requested, try the D3D9 and D3D11 renderers, order them using
        // the definition of ANGLE_DEFAULT_D3D11
#       if ANGLE_DEFAULT_D3D11
#           if defined(ANGLE_ENABLE_D3D11)
                rendererCreationFunctions.push_back(gl::CreateRenderer<rx::Renderer11>);
#           endif
#           if defined(ANGLE_ENABLE_D3D9)
                rendererCreationFunctions.push_back(gl::CreateRenderer<rx::Renderer9>);
#           endif
#       else
#           if defined(ANGLE_ENABLE_D3D9)
                rendererCreationFunctions.push_back(gl::CreateRenderer<rx::Renderer9>);
#           endif
#           if defined(ANGLE_ENABLE_D3D11)
                rendererCreationFunctions.push_back(gl::CreateRenderer<rx::Renderer11>);
#           endif
#       endif
    }

    for (size_t i = 0; i < rendererCreationFunctions.size(); i++)
    {
        gl::Renderer *renderer = rendererCreationFunctions[i](display, nativeDisplay, requestedDisplayType);
        if (renderer->initialize() == EGL_SUCCESS)
        {
            return renderer;
        }
        else
        {
            // Failed to create the renderer, try the next
            SafeDelete(renderer);
        }
    }

    return NULL;
}

void glDestroyRenderer(gl::Renderer *renderer)
{
    ASSERT(renderer);
    SafeDelete(renderer);
}

}
