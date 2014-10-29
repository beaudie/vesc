//
// Copyright (c) 2012-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Renderer.h: Defines a back-end specific class that hides the details of the
// implementation-specific renderer.

#ifndef LIBGLESV2_RENDERER_RENDERER_H_
#define LIBGLESV2_RENDERER_RENDERER_H_

#include "libGLESv2/Caps.h"
#include "libGLESv2/Error.h"
#include "libGLESv2/Uniform.h"
#include "libGLESv2/angletypes.h"
#include "libGLESv2/renderer/Workarounds.h"
#include "common/NativeWindow.h"
#include "common/mathutil.h"

#include <cstdint>

#include <EGL/egl.h>

#if !defined(ANGLE_COMPILE_OPTIMIZATION_LEVEL)
// WARNING: D3DCOMPILE_OPTIMIZATION_LEVEL3 may lead to a DX9 shader compiler hang.
// It should only be used selectively to work around specific bugs.
#define ANGLE_COMPILE_OPTIMIZATION_LEVEL D3DCOMPILE_OPTIMIZATION_LEVEL1
#endif

namespace egl
{
class Display;
}

namespace rx
{
class QueryImpl;
class FenceNVImpl;
class FenceSyncImpl;
class BufferImpl;
class VertexArrayImpl;
class BufferStorage;
class ShaderImpl;
class ProgramImpl;
class TextureImpl;
class TransformFeedbackImpl;
class RendererImpl;
struct TranslatedIndexData;
struct Workarounds;

struct ConfigDesc
{
    GLenum  renderTargetFormat;
    GLenum  depthStencilFormat;
    GLint   multiSample;
    bool    fastConfig;
    bool    es3Capable;
};

}

namespace gl
{
class InfoLog;
class ProgramBinary;
struct LinkedVarying;
struct VertexAttribute;
class Buffer;
class Texture;
class Framebuffer;
struct VertexAttribCurrentValueData;
struct Data;

class Renderer
{
  public:
    Renderer(rx::RendererImpl *implementation);
    ~Renderer();

    EGLint initialize();

    // TODO(jmadill): fix virtual for egl::Display
    virtual bool resetDevice();

    // TODO(jmadill): figure out ConfigDesc namespace & virtual for egl::Display
    virtual int generateConfigs(rx::ConfigDesc **configDescList);
    virtual void deleteConfigs(rx::ConfigDesc *configDescList);

    gl::Error sync(bool block);

    // TODO(jmadill): pass state and essetial params only
    gl::Error drawArrays(const gl::Data &data, GLenum mode,
                         GLint first, GLsizei count, GLsizei instances);
    gl::Error drawElements(const gl::Data &data, GLenum mode, GLsizei count, GLenum type,
                           const GLvoid *indices, GLsizei instances,
                           const rx::RangeUI &indexRange);

    gl::Error clear(const gl::Data &data, GLbitfield mask);
    gl::Error clearBufferfv(const gl::Data &data, GLenum buffer, GLint drawbuffer, const GLfloat *values);
    gl::Error clearBufferuiv(const gl::Data &data, GLenum buffer, GLint drawbuffer, const GLuint *values);
    gl::Error clearBufferiv(const gl::Data &data, GLenum buffer, GLint drawbuffer, const GLint *values);
    gl::Error clearBufferfi(const gl::Data &data, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);

    gl::Error readPixels(gl::Framebuffer *framebuffer, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format,
                         GLenum type, GLuint outputPitch, const gl::PixelPackState &pack, uint8_t *pixels);

    gl::Error blitFramebuffer(const gl::Data &data,
                              GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                              GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                              GLbitfield mask, GLenum filter);

    // TODO(jmadill): caps? and virtual for egl::Display
    virtual bool getShareHandleSupport() const;
    virtual bool getPostSubBufferSupport() const;

    // Shader creation
    rx::ShaderImpl *createShader(GLenum type);
    rx::ProgramImpl *createProgram();

    // Shader operations
    virtual void releaseShaderCompiler() = 0;
    virtual gl::Error loadExecutable(const void *function, size_t length, rx::ShaderType type,
                                     const std::vector<gl::LinkedVarying> &transformFeedbackVaryings,
                                     bool separatedOutputBuffers, ShaderExecutable **outExecutable) = 0;
    virtual gl::Error compileToExecutable(gl::InfoLog &infoLog, const std::string &shaderHLSL, rx::ShaderType type,
                                          const std::vector<gl::LinkedVarying> &transformFeedbackVaryings,
                                          bool separatedOutputBuffers, D3DWorkaroundType workaround,
                                          ShaderExecutable **outExectuable) = 0;
    virtual UniformStorage *createUniformStorage(size_t storageSize) = 0;

    // Image operations
    virtual Image *createImage() = 0;
    virtual gl::Error generateMipmap(Image *dest, Image *source) = 0;
    virtual TextureStorage *createTextureStorage2D(SwapChain *swapChain) = 0;
    virtual TextureStorage *createTextureStorage2D(GLenum internalformat, bool renderTarget, GLsizei width, GLsizei height, int levels) = 0;
    virtual TextureStorage *createTextureStorageCube(GLenum internalformat, bool renderTarget, int size, int levels) = 0;
    virtual TextureStorage *createTextureStorage3D(GLenum internalformat, bool renderTarget, GLsizei width, GLsizei height, GLsizei depth, int levels) = 0;
    virtual TextureStorage *createTextureStorage2DArray(GLenum internalformat, bool renderTarget, GLsizei width, GLsizei height, GLsizei depth, int levels) = 0;

    // Texture creation
    rx::TextureImpl *createTexture(GLenum target);

    // Buffer creation
    rx::BufferImpl *createBuffer();

    // Vertex Array creation
    rx::VertexArrayImpl *createVertexArray();

    // Query and Fence creation
    rx::QueryImpl *createQuery(GLenum type);
    rx::FenceNVImpl *createFenceNV();
    rx::FenceSyncImpl *createFenceSync();

    // Transform Feedback creation
    rx::TransformFeedbackImpl* createTransformFeedback();

    rx::RendererImpl *getImplementation() { return mImplementation; }
    const rx::RendererImpl *getImplementation() const { return mImplementation; }

    // lost device
    //TODO(jmadill): investigate if this stuff is necessary in GL, and fix virtual
    virtual void notifyDeviceLost();
    virtual bool isDeviceLost();
    virtual bool testDeviceLost(bool notify);
    virtual bool testDeviceResettable();

    DWORD getAdapterVendor() const;
    std::string getRendererDescription() const;
    GUID getAdapterIdentifier() const;

    // Renderer capabilities (virtual because it is used by egl::Display)
    virtual const Caps &getRendererCaps() const;
    virtual const TextureCapsMap &getRendererTextureCaps() const;
    virtual const Extensions &getRendererExtensions() const;
    virtual const rx::Workarounds &getWorkarounds() const;

    // TODO(jmadill): needed by egl::Display, probably should be removed
    virtual int getMajorShaderModel() const;
    virtual int getMinSwapInterval() const;
    virtual int getMaxSwapInterval() const;
    virtual bool getLUID(LUID *adapterLuid) const;

  private:
    rx::RendererImpl *mImplementation;
};

}

namespace rx
{
class TextureStorage;
class VertexBuffer;
class IndexBuffer;
class ShaderExecutable;
class SwapChain;
class RenderTarget;
class Image;
class TextureStorage;
class UniformStorage;

struct dx_VertexConstants
{
    float depthRange[4];
    float viewAdjust[4];
};

struct dx_PixelConstants
{
    float depthRange[4];
    float viewCoords[4];
    float depthFront[4];
};

enum ShaderType
{
    SHADER_VERTEX,
    SHADER_PIXEL,
    SHADER_GEOMETRY
};

}
#endif // LIBGLESV2_RENDERER_RENDERER_H_
