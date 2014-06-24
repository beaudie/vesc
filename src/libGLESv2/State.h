//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// State.h: Defines the State class, encapsulating raw GL state

#ifndef LIBGLESV2_STATE_H_
#define LIBGLESV2_STATE_H_

#include "common/angleutils.h"
#include "common/RefCountObject.h"
#include "libGLESv2/angletypes.h"
#include "libGLESv2/VertexAttribute.h"
#include "libGLESv2/Renderbuffer.h"
#include "libGLESv2/Texture.h"
#include "libGLESv2/TransformFeedback.h"
#include "libGLESv2/Program.h"
#include "libGLESv2/Sampler.h"

namespace gl
{
class Query;
class VertexArray;
class Context;

class State
{
  public:
    State();
    ~State();

    void setContext(Context *context) { mContext = context; }

    // State chunk getters
    const RasterizerState &getRasterizerState() const;
    const BlendState &getBlendState() const;
    const DepthStencilState &getDepthStencilState() const;

    // Clear behavior setters & state parameter block generation function
    void setClearColor(float red, float green, float blue, float alpha);
    void setClearDepth(float depth);
    void setClearStencil(int stencil);
    ClearParameters getClearParameters(GLbitfield mask) const;

    // Write mask manipulation
    void setColorMask(bool red, bool green, bool blue, bool alpha);
    void setDepthMask(bool mask);

    // Discard toggle & query
    void setRasterizerDiscard(bool enabled);
    bool isRasterizerDiscardEnabled() const;

    // Face culling state manipulation
    bool isCullFaceEnabled() const;
    void setCullMode(GLenum mode);
    void setCullFace(bool enabled);
    void setFrontFace(GLenum front);

    // Depth test state manipulation
    bool isDepthTestEnabled() const;
    void setDepthTest(bool enabled);
    void setDepthFunc(GLenum depthFunc);
    void setDepthRange(float zNear, float zFar);
    void getDepthRange(float *zNear, float *zFar) const;
    
    // Blend state manipulation
    bool isBlendEnabled() const;
    void setBlend(bool enabled);
    void setBlendFactors(GLenum sourceRGB, GLenum destRGB, GLenum sourceAlpha, GLenum destAlpha);
    void setBlendColor(float red, float green, float blue, float alpha);
    const ColorF &getBlendColor() const { return mBlendColor; }
    void setBlendEquation(GLenum rgbEquation, GLenum alphaEquation);

    // Stencil state maniupulation
    bool isStencilTestEnabled() const;
    void setStencilTest(bool enabled);
    void setStencilParams(GLenum stencilFunc, GLint stencilRef, GLuint stencilMask);
    void setStencilBackParams(GLenum stencilBackFunc, GLint stencilBackRef, GLuint stencilBackMask);
    void setStencilWritemask(GLuint stencilWritemask);
    void setStencilBackWritemask(GLuint stencilBackWritemask);
    void setStencilOperations(GLenum stencilFail, GLenum stencilPassDepthFail, GLenum stencilPassDepthPass);
    void setStencilBackOperations(GLenum stencilBackFail, GLenum stencilBackPassDepthFail, GLenum stencilBackPassDepthPass);
    GLint getStencilRef() const;
    GLint getStencilBackRef() const;

    // Depth bias/polygon offset state manipulation
    void setPolygonOffsetFill(bool enabled);
    bool isPolygonOffsetFillEnabled() const;
    void setPolygonOffsetParams(GLfloat factor, GLfloat units);

    // Multisample coverage state manipulation
    void setSampleAlphaToCoverage(bool enabled);
    bool isSampleAlphaToCoverageEnabled() const;
    void setSampleCoverage(bool enabled);
    bool isSampleCoverageEnabled() const;
    void setSampleCoverageParams(GLclampf value, bool invert);
    void getSampleCoverageParams(GLclampf *value, bool *invert);

    // Scissor test state toggle & query
    void setScissorTest(bool enabled);
    bool isScissorTestEnabled() const;
    void setScissorParams(GLint x, GLint y, GLsizei width, GLsizei height);
    const Rectangle &getScissor() const;

    // Dither state toggle & query
    void setDither(bool enabled);
    bool isDitherEnabled() const;

    // Generic state toggle & query
    void setCap(GLenum cap, bool enabled);
    bool getCap(GLenum cap);

    // Line with state setter
    void setLineWidth(GLfloat width);

    // Hint setters
    void setGenerateMipmapHint(GLenum hint);
    void setFragmentShaderDerivativeHint(GLenum hint);

    // Viewport state setter/getter
    void setViewportParams(GLint x, GLint y, GLsizei width, GLsizei height);
    const Rectangle &getViewport() const;

    // Texture binding & active texture unit manipulation
    void setActiveSampler(unsigned int active);
    unsigned int getActiveSampler() const;
    void setSamplerTexture(TextureType type, Texture *texture);
    Texture *getSamplerTexture(unsigned int sampler, TextureType type) const;
    GLuint getSamplerTextureId(unsigned int sampler, TextureType type) const;
    void detachTexture(GLuint texture);

    // Sampler object binding manipulation
    GLuint getSamplerHandle(GLuint textureUnit) const;
    void setSamplerBinding(GLuint textureUnit, Sampler *sampler);
    void detachSampler(GLuint sampler);
    Sampler *getSampler(GLuint textureUnit) const;

    // Renderbuffer binding manipulation
    GLuint getRenderbufferHandle() const;
    void setRenderbufferBinding(Renderbuffer *renderbuffer);
    Renderbuffer *getCurrentRenderbuffer();
    GLuint getReadFramebufferHandle() const;
    GLuint getDrawFramebufferHandle() const;
    void setReadFramebufferBinding(GLuint handle, Framebuffer *framebuffer);
    void setDrawFramebufferBinding(GLuint handle, Framebuffer *framebuffer);

    // Vertex array object binding manipulation
    GLuint getVertexArrayHandle() const;
    VertexArray *getCurrentVertexArray() const;
    void setVertexArrayBinding(GLuint handle, VertexArray *vertexArray);

    //// Typed buffer binding point manipulation ////
    // GL_ARRAY_BUFFER
    GLuint getArrayBufferHandle() const;
    void setArrayBufferBinding(Buffer *buffer);

    // GL_UNIFORM_BUFFER - Both indexed and generic targets
    GLuint getIndexedUniformBufferHandle(GLuint index) const; // GL_UNIFORM_BUFFER
    Buffer *getIndexedUniformBuffer(GLuint index) const;
    void setIndexedUniformBufferBinding(GLuint index, Buffer *buffer, GLintptr offset, GLsizeiptr size);
    void setGenericUniformBufferBinding(Buffer *buffer);

    // GL_TRANSFORM_FEEDBACK_BUFFER - Both indexed and generic targets
    GLuint getIndexedTransformFeedbackBufferHandle(GLuint index) const;
    Buffer *getIndexedTransformFeedbackBuffer(GLuint index) const;
    void setIndexedTransformFeedbackBufferBinding(GLuint index, Buffer *buffer, GLintptr offset, GLsizeiptr size);
    GLuint getIndexedTransformFeedbackBufferOffset(GLuint index) const;
    void setGenericTransformFeedbackBufferBinding(Buffer *buffer);

    // GL_COPY_[READ/WRITE]_BUFFER
    void setCopyReadBufferBinding(Buffer *buffer);
    void setCopyWriteBufferBinding(Buffer *buffer);

    // GL_PIXEL[PACK/UNPACK]_BUFFER
    void setPixelPackBufferBinding(Buffer *buffer);
    void setPixelUnpackBufferBinding(Buffer *buffer);

    // Retrieve typed buffer by target (non-indexed)
    Buffer *getTargetBuffer(GLenum target) const;

    // Transform feedback object (not buffer) binding manipulation
    void setTransformFeedbackBinding(TransformFeedback *transformFeedback);
    void detachTransformFeedback(GLuint transformFeedback);

    // Query binding manipulation
    bool isQueryActive() const;
    Query *getActiveQuery(GLenum target) const;
    GLuint getActiveQueryId(GLenum target) const;
    void setActiveQuery(GLenum target, Query *query);

    // Vertex attrib manipulation
    void setEnableVertexAttribArray(unsigned int attribNum, bool enabled);
    const VertexAttribute &getVertexAttribState(unsigned int attribNum) const;
    const VertexAttribCurrentValueData &getVertexAttribCurrentValue(unsigned int attribNum) const;
    const VertexAttribCurrentValueData *getVertexAttribCurrentValues() const;
    void setVertexAttribf(GLuint index, const GLfloat values[4]);
    void setVertexAttribu(GLuint index, const GLuint values[4]);
    void setVertexAttribi(GLuint index, const GLint values[4]);
    void setVertexAttribState(unsigned int attribNum, Buffer *boundBuffer, GLint size, GLenum type,
                              bool normalized, bool pureInteger, GLsizei stride, const void *pointer);
    const void *getVertexAttribPointer(unsigned int attribNum) const;

    // Pixel unpack state manipulation
    void setUnpackAlignment(GLint alignment);
    GLint getUnpackAlignment() const;
    const PixelUnpackState &getUnpackState() const;

    // Pixel pack state manipulation
    void setPackAlignment(GLint alignment);
    GLint getPackAlignment() const;
    const PixelPackState &getPackState() const;
    void setPackReverseRowOrder(bool reverseRowOrder);
    bool getPackReverseRowOrder() const;

    // Program binding manipulation
    GLuint getCurrentProgramId() const;
    ProgramBinary *getCurrentProgramBinary() const;
    void setCurrentProgram(GLuint programId, Program *newProgram);
    void setCurrentProgramBinary(ProgramBinary *binary);

    // Framebuffer binding manipulation
    Framebuffer *getTargetFramebuffer(GLenum target) const;
    GLuint getTargetFramebufferHandle(GLenum target) const;
    Framebuffer *getReadFramebuffer();
    Framebuffer *getDrawFramebuffer();
    const Framebuffer *getDrawFramebuffer() const;
    const Framebuffer *getReadFramebuffer() const;
    TransformFeedback *getCurrentTransformFeedback() const;

    // State query functions
    void getBooleanv(GLenum pname, GLboolean *params);
    void getFloatv(GLenum pname, GLfloat *params);
    void getIntegerv(GLenum pname, GLint *params);
    bool getIndexedIntegerv(GLenum target, GLuint index, GLint *data);
    bool getIndexedInteger64v(GLenum target, GLuint index, GLint64 *data);

    bool removeBufferBinding(GLuint buffer);
    bool removeReadFramebufferBinding(GLuint framebuffer);
    bool removeDrawFramebufferBinding(GLuint framebuffer);
    void detachRenderbuffer(GLuint renderbuffer);
    bool removeVertexArrayBinding(GLuint vertexArray);

  private:
    DISALLOW_COPY_AND_ASSIGN(State);

    Context *mContext;

    ColorF mColorClearValue;
    GLclampf mDepthClearValue;
    int mStencilClearValue;

    RasterizerState mRasterizer;
    bool mScissorTest;
    Rectangle mScissor;

    BlendState mBlend;
    ColorF mBlendColor;
    bool mSampleCoverage;
    GLclampf mSampleCoverageValue;
    bool mSampleCoverageInvert;

    DepthStencilState mDepthStencil;
    GLint mStencilRef;
    GLint mStencilBackRef;

    GLfloat mLineWidth;

    GLenum mGenerateMipmapHint;
    GLenum mFragmentShaderDerivativeHint;

    Rectangle mViewport;
    float mNearZ;
    float mFarZ;

    unsigned int mActiveSampler;   // Active texture unit selector - GL_TEXTURE0
    BindingPointer<Buffer> mArrayBuffer;
    Framebuffer *mReadFramebuffer;
    Framebuffer *mDrawFramebuffer;
    BindingPointer<Renderbuffer> mRenderbuffer;
    GLuint mCurrentProgramId;
    BindingPointer<ProgramBinary> mCurrentProgramBinary;

    VertexAttribCurrentValueData mVertexAttribCurrentValues[MAX_VERTEX_ATTRIBS]; // From glVertexAttrib
    VertexArray *mVertexArray;

    BindingPointer<Texture> mSamplerTexture[TEXTURE_TYPE_COUNT][IMPLEMENTATION_MAX_COMBINED_TEXTURE_IMAGE_UNITS];
    BindingPointer<Sampler> mSamplers[IMPLEMENTATION_MAX_COMBINED_TEXTURE_IMAGE_UNITS];

    typedef std::map< GLenum, BindingPointer<Query> > ActiveQueryMap;
    ActiveQueryMap mActiveQueries;

    BindingPointer<Buffer> mGenericUniformBuffer;
    OffsetBindingPointer<Buffer> mUniformBuffers[IMPLEMENTATION_MAX_COMBINED_SHADER_UNIFORM_BUFFERS];

    BindingPointer<TransformFeedback> mTransformFeedback;
    BindingPointer<Buffer> mGenericTransformFeedbackBuffer;
    OffsetBindingPointer<Buffer> mTransformFeedbackBuffers[IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS];

    BindingPointer<Buffer> mCopyReadBuffer;
    BindingPointer<Buffer> mCopyWriteBuffer;

    PixelUnpackState mUnpack;
    PixelPackState mPack;
};

}

#endif // LIBGLESV2_STATE_H_

