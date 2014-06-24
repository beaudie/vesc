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

    void setCap(GLenum cap, bool enabled);
    bool getCap(GLenum cap);

    const RasterizerState &getRasterizerState() const;
    const BlendState &getBlendState() const;
    const DepthStencilState &getDepthStencilState() const;

    void setClearColor(float red, float green, float blue, float alpha);

    void setClearDepth(float depth);

    void setClearStencil(int stencil);

    void setRasterizerDiscard(bool enabled);
    bool isRasterizerDiscardEnabled() const;

    void setCullFace(bool enabled);
    bool isCullFaceEnabled() const;

    void setCullMode(GLenum mode);

    void setFrontFace(GLenum front);

    void setDepthTest(bool enabled);
    bool isDepthTestEnabled() const;

    void setDepthFunc(GLenum depthFunc);

    void setDepthRange(float zNear, float zFar);
    void getDepthRange(float *zNear, float *zFar) const;
    
    void setBlend(bool enabled);
    bool isBlendEnabled() const;

    void setBlendFactors(GLenum sourceRGB, GLenum destRGB, GLenum sourceAlpha, GLenum destAlpha);
    void setBlendColor(float red, float green, float blue, float alpha);
    void setBlendEquation(GLenum rgbEquation, GLenum alphaEquation);
    const ColorF &getBlendColor() const { return mBlendColor; }

    void setStencilTest(bool enabled);
    bool isStencilTestEnabled() const;

    void setStencilParams(GLenum stencilFunc, GLint stencilRef, GLuint stencilMask);
    void setStencilBackParams(GLenum stencilBackFunc, GLint stencilBackRef, GLuint stencilBackMask);
    void setStencilWritemask(GLuint stencilWritemask);
    void setStencilBackWritemask(GLuint stencilBackWritemask);
    void setStencilOperations(GLenum stencilFail, GLenum stencilPassDepthFail, GLenum stencilPassDepthPass);
    void setStencilBackOperations(GLenum stencilBackFail, GLenum stencilBackPassDepthFail, GLenum stencilBackPassDepthPass);

    GLint getStencilRef() const;
    GLint getStencilBackRef() const;

    void setPolygonOffsetFill(bool enabled);
    bool isPolygonOffsetFillEnabled() const;

    void setPolygonOffsetParams(GLfloat factor, GLfloat units);

    void setSampleAlphaToCoverage(bool enabled);
    bool isSampleAlphaToCoverageEnabled() const;

    void setSampleCoverage(bool enabled);
    bool isSampleCoverageEnabled() const;

    void setSampleCoverageParams(GLclampf value, bool invert);
    void getSampleCoverageParams(GLclampf *value, bool *invert);

    void setScissorTest(bool enabled);
    bool isScissorTestEnabled() const;

    void setDither(bool enabled);
    bool isDitherEnabled() const;

    void setLineWidth(GLfloat width);

    void setGenerateMipmapHint(GLenum hint);
    void setFragmentShaderDerivativeHint(GLenum hint);

    void setViewportParams(GLint x, GLint y, GLsizei width, GLsizei height);
    const Rectangle &getViewport() const;

    void setScissorParams(GLint x, GLint y, GLsizei width, GLsizei height);
    const Rectangle &getScissor() const;

    void setColorMask(bool red, bool green, bool blue, bool alpha);
    void setDepthMask(bool mask);

    void setActiveSampler(unsigned int active);
    void setSamplerTexture(TextureType type, Texture *texture);
    Texture *getSamplerTexture(unsigned int sampler, TextureType type) const;
    GLuint getSamplerTextureId(unsigned int sampler, TextureType type) const;

    ClearParameters getClearParameters(GLbitfield mask) const;

    GLuint getRenderbufferHandle() const;
    void setRenderbufferBinding(Renderbuffer *renderbuffer);
    Renderbuffer *getCurrentRenderbuffer();

    GLuint getVertexArrayHandle() const;
    void setVertexArrayBinding(GLuint handle, VertexArray *vertexArray);

    GLuint getSamplerHandle(GLuint textureUnit) const;
    void setSamplerBinding(GLuint textureUnit, Sampler *sampler);
    Sampler *getSampler(GLuint textureUnit) const;
    unsigned int getActiveSampler() const;

    GLuint getArrayBufferHandle() const;
    void setArrayBufferBinding(Buffer *buffer);

    GLuint getReadFramebufferHandle() const;
    GLuint getDrawFramebufferHandle() const;
    void setReadFramebufferBinding(GLuint handle, Framebuffer *framebuffer);
    void setDrawFramebufferBinding(GLuint handle, Framebuffer *framebuffer);

    void setGenericUniformBufferBinding(Buffer *buffer);
    void setGenericTransformFeedbackBufferBinding(Buffer *buffer);
    void setIndexedUniformBufferBinding(GLuint index, Buffer *buffer, GLintptr offset, GLsizeiptr size);
    void setIndexedTransformFeedbackBufferBinding(GLuint index, Buffer *buffer, GLintptr offset, GLsizeiptr size);

    void setTransformFeedbackBinding(TransformFeedback *transformFeedback);

    GLuint getUniformBufferHandle(GLuint index) const;
    Buffer *getUniformBuffer(GLuint index) const;

    GLuint getTransformFeedbackBufferHandle(GLuint index) const;
    Buffer *getTransformFeedbackBuffer(GLuint index) const;
    GLuint getTransformFeedbackBufferOffset(GLuint index) const;

    void setCopyReadBufferBinding(Buffer *buffer);
    void setCopyWriteBufferBinding(Buffer *buffer);

    void setPixelPackBufferBinding(Buffer *buffer);
    void setPixelUnpackBufferBinding(Buffer *buffer);

    Buffer *getTargetBuffer(GLenum target) const;

    bool isQueryActive() const;
    Query *getActiveQuery(GLenum target) const;
    GLuint getActiveQueryId(GLenum target) const;
    void setActiveQuery(GLenum target, Query *query);

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

    void setUnpackAlignment(GLint alignment);
    GLint getUnpackAlignment() const;
    const PixelUnpackState &getUnpackState() const;

    void setPackAlignment(GLint alignment);
    GLint getPackAlignment() const;
    const PixelPackState &getPackState() const;

    void setPackReverseRowOrder(bool reverseRowOrder);
    bool getPackReverseRowOrder() const;

    VertexArray *getCurrentVertexArray() const;

    GLuint getCurrentProgramId() const;
    ProgramBinary *getCurrentProgramBinary() const;
    void setCurrentProgram(GLuint programId, Program *newProgram);
    void setCurrentProgramBinary(ProgramBinary *binary);

    Framebuffer *getTargetFramebuffer(GLenum target) const;
    GLuint getTargetFramebufferHandle(GLenum target) const;
    Framebuffer *getReadFramebuffer();
    Framebuffer *getDrawFramebuffer();
    const Framebuffer *getDrawFramebuffer() const;
    const Framebuffer *getReadFramebuffer() const;
    TransformFeedback *getCurrentTransformFeedback() const;

    void getBooleanv(GLenum pname, GLboolean *params);
    void getFloatv(GLenum pname, GLfloat *params);
    void getIntegerv(GLenum pname, GLint *params);

    bool getIndexedIntegerv(GLenum target, GLuint index, GLint *data);
    bool getIndexedInteger64v(GLenum target, GLuint index, GLint64 *data);

    void detachTexture(GLuint texture);
    void detachSampler(GLuint sampler);

    bool removeBufferBinding(GLuint buffer);
    bool removeReadFramebufferBinding(GLuint framebuffer);
    bool removeDrawFramebufferBinding(GLuint framebuffer);
    bool removeRenderbufferBinding(GLuint renderbuffer);
    bool removeVertexArrayBinding(GLuint vertexArray);
    bool removeTransformFeedbackBinding(GLuint transformFeedback);

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
    GLuint mReadFramebufferId;
    Framebuffer *mBoundReadFramebuffer;
    GLuint mDrawFramebufferId;
    Framebuffer *mBoundDrawFramebuffer;
    BindingPointer<Renderbuffer> mRenderbuffer;
    GLuint mCurrentProgramId;
    BindingPointer<ProgramBinary> mCurrentProgramBinary;

    VertexAttribCurrentValueData mVertexAttribCurrentValues[MAX_VERTEX_ATTRIBS]; // From glVertexAttrib
    unsigned int mVertexArrayId;
    VertexArray *mBoundVertexArray;

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

