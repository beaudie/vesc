//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManagerGL.h: Defines a class for caching applied OpenGL state

#ifndef LIBANGLE_RENDERER_GL_STATEMANAGERGL_H_
#define LIBANGLE_RENDERER_GL_STATEMANAGERGL_H_

#include "common/debug.h"
#include "libANGLE/Error.h"
#include "libANGLE/State.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/gl/functionsgl_typedefs.h"

#include <map>

namespace gl
{
struct Caps;
struct Data;
class State;
}

namespace rx
{

class FunctionsGL;

class StateManagerGL final : angle::NonCopyable
{
  public:
    StateManagerGL(const FunctionsGL *functions, const gl::Caps &rendererCaps);

    void deleteProgram(GLuint program);
    void deleteVertexArray(GLuint vao);
    void deleteTexture(GLuint texture);
    void deleteBuffer(GLuint buffer);
    void deleteFramebuffer(GLuint fbo);
    void deleteRenderbuffer(GLuint rbo);

    void useProgram(GLuint program);
    void bindVertexArray(GLuint vao, GLuint elementArrayBuffer);
    void bindBuffer(GLenum type, GLuint buffer);
    void activeTexture(size_t unit);
    void bindTexture(GLenum type, GLuint texture);
    void bindFramebuffer(GLenum type, GLuint framebuffer);
    void bindRenderbuffer(GLenum type, GLuint renderbuffer);

    gl::Error setDrawArraysState(const gl::Data &data, GLint first, GLsizei count);
    gl::Error setDrawElementsState(const gl::Data &data, GLsizei count, GLenum type, const GLvoid *indices,
                                   const GLvoid **outIndices);

    void syncState(const gl::State &state, const gl::State::DirtyBits &dirtyBits);

  private:
    gl::Error setGenericDrawState(const gl::Data &data);

    void setAttributeCurrentData(size_t index, const gl::VertexAttribCurrentValueData &data, bool setDirty);

    void setScissorTestEnabled(bool enabled, bool setDirty);
    void setScissor(const gl::Rectangle &scissor, bool setDirty);

    void setViewport(const gl::Rectangle &viewport, bool setDirty);
    void setDepthRange(float near, float far, bool setDirty);

    void setBlendEnabled(bool enabled, bool setDirty);
    void setBlendColor(const gl::ColorF &blendColor, bool setDirty);
    void setBlendFuncs(GLenum sourceBlendRGB, GLenum destBlendRGB, GLenum sourceBlendAlpha, GLenum destBlendAlpha, bool setDirty);
    void setBlendEquations(GLenum blendEquationRGB, GLenum blendEquationAlpha, bool setDirty);
    void setColorMask(bool red, bool green, bool blue, bool alpha, bool setDirty);
    void setSampleAlphaToCoverageEnabled(bool enabled, bool setDirty);
    void setSampleCoverageEnabled(bool enabled, bool setDirty);
    void setSampleCoverage(float value, bool invert, bool setDirty);

    void setDepthTestEnabled(bool enabled, bool setDirty);
    void setDepthFunc(GLenum depthFunc, bool setDirty);
    void setDepthMask(bool mask, bool setDirty);
    void setStencilTestEnabled(bool enabled, bool setDirty);
    void setStencilFrontWritemask(GLuint mask, bool setDirty);
    void setStencilBackWritemask(GLuint mask, bool setDirty);
    void setStencilFrontFuncs(GLenum func, GLint ref, GLuint mask, bool setDirty);
    void setStencilBackFuncs(GLenum func, GLint ref, GLuint mask, bool setDirty);
    void setStencilFrontOps(GLenum sfail, GLenum dpfail, GLenum dppass, bool setDirty);
    void setStencilBackOps(GLenum sfail, GLenum dpfail, GLenum dppass, bool setDirty);

    void setCullFaceEnabled(bool enabled, bool setDirty);
    void setCullFace(GLenum cullFace, bool setDirty);
    void setFrontFace(GLenum frontFace, bool setDirty);
    void setPolygonOffsetFillEnabled(bool enabled, bool setDirty);
    void setPolygonOffset(float factor, float units, bool setDirty);
    void setMultisampleEnabled(bool enabled, bool setDirty);
    void setRasterizerDiscardEnabled(bool enabled, bool setDirty);
    void setLineWidth(float width, bool setDirty);

    void setPrimitiveRestartEnabled(bool enabled, bool setDirty);

    void setClearColor(const gl::ColorF &clearColor, bool setDirty);
    void setClearDepth(float clearDepth, bool setDirty);
    void setClearStencil(GLint clearStencil, bool setDirty);

    void setPixelUnpackState(const gl::PixelUnpackState &unpack, bool setDirty);
    void setPixelUnpackState(GLint alignment,
                             GLint rowLength,
                             GLint skipRows,
                             GLint skipPixels,
                             GLint imageHeight,
                             GLint skipImages,
                             bool setDirty);
    void setPixelPackState(const gl::PixelPackState &pack, bool setDirty);
    void setPixelPackState(GLint alignment, GLint rowLength, GLint skipRows, GLint skipPixels, bool setDirty);

    const FunctionsGL *mFunctions;

    GLuint mProgram;

    GLuint mVAO;
    std::vector<gl::VertexAttribCurrentValueData> mVertexAttribCurrentValues;

    std::map<GLenum, GLuint> mBuffers;

    size_t mTextureUnitIndex;
    std::map<GLenum, std::vector<GLuint>> mTextures;

    GLint mUnpackAlignment;
    GLint mUnpackRowLength;
    GLint mUnpackSkipRows;
    GLint mUnpackSkipPixels;
    GLint mUnpackImageHeight;
    GLint mUnpackSkipImages;

    GLint mPackAlignment;
    GLint mPackRowLength;
    GLint mPackSkipRows;
    GLint mPackSkipPixels;

    // TODO(jmadill): Convert to std::array when available
    std::vector<GLenum> mFramebuffers;
    GLuint mRenderbuffer;

    bool mScissorTestEnabled;
    gl::Rectangle mScissor;

    gl::Rectangle mViewport;
    float mNear;
    float mFar;

    bool mBlendEnabled;
    gl::ColorF mBlendColor;
    GLenum mSourceBlendRGB;
    GLenum mDestBlendRGB;
    GLenum mSourceBlendAlpha;
    GLenum mDestBlendAlpha;
    GLenum mBlendEquationRGB;
    GLenum mBlendEquationAlpha;
    bool mColorMaskRed;
    bool mColorMaskGreen;
    bool mColorMaskBlue;
    bool mColorMaskAlpha;
    bool mSampleAlphaToCoverageEnabled;
    bool mSampleCoverageEnabled;
    float mSampleCoverageValue;
    bool mSampleCoverageInvert;

    bool mDepthTestEnabled;
    GLenum mDepthFunc;
    bool mDepthMask;
    bool mStencilTestEnabled;
    GLenum mStencilFrontFunc;
    GLint mStencilFrontRef;
    GLuint mStencilFrontValueMask;
    GLenum mStencilFrontStencilFailOp;
    GLenum mStencilFrontStencilPassDepthFailOp;
    GLenum mStencilFrontStencilPassDepthPassOp;
    GLuint mStencilFrontWritemask;
    GLenum mStencilBackFunc;
    GLint mStencilBackRef;
    GLuint mStencilBackValueMask;
    GLenum mStencilBackStencilFailOp;
    GLenum mStencilBackStencilPassDepthFailOp;
    GLenum mStencilBackStencilPassDepthPassOp;
    GLuint mStencilBackWritemask;

    bool mCullFaceEnabled;
    GLenum mCullFace;
    GLenum mFrontFace;
    bool mPolygonOffsetFillEnabled;
    GLfloat mPolygonOffsetFactor;
    GLfloat mPolygonOffsetUnits;
    bool mMultisampleEnabled;
    bool mRasterizerDiscardEnabled;
    float mLineWidth;

    bool mPrimitiveRestartEnabled;

    gl::ColorF mClearColor;
    float mClearDepth;
    GLint mClearStencil;

    gl::State::DirtyBits mLocalDirtyBits;
};

}

#endif // LIBANGLE_RENDERER_GL_STATEMANAGERGL_H_
