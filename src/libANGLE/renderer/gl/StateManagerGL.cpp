//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManagerGL.h: Defines a class for caching applied OpenGL state

#include "libANGLE/renderer/gl/StateManagerGL.h"

#include "common/BitSetIterator.h"
#include "libANGLE/Data.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/ProgramGL.h"
#include "libANGLE/renderer/gl/TextureGL.h"
#include "libANGLE/renderer/gl/VertexArrayGL.h"

namespace rx
{
StateManagerGL::StateManagerGL(const FunctionsGL *functions, const gl::Caps &rendererCaps)
    : mFunctions(functions),
      mProgram(0),
      mVAO(0),
      mVertexAttribCurrentValues(rendererCaps.maxVertexAttributes),
      mBuffers(),
      mTextureUnitIndex(0),
      mTextures(),
      mUnpackAlignment(4),
      mUnpackRowLength(0),
      mUnpackSkipRows(0),
      mUnpackSkipPixels(0),
      mUnpackImageHeight(0),
      mUnpackSkipImages(0),
      mPackAlignment(4),
      mPackRowLength(0),
      mPackSkipRows(0),
      mPackSkipPixels(0),
      mFramebuffers(angle::FramebufferBindingSingletonMax, 0),
      mRenderbuffer(0),
      mScissorTestEnabled(false),
      mScissor(0, 0, 0, 0),
      mViewport(0, 0, 0, 0),
      mNear(0.0f),
      mFar(1.0f),
      mBlendEnabled(false),
      mBlendColor(0, 0, 0, 0),
      mSourceBlendRGB(GL_ONE),
      mDestBlendRGB(GL_ZERO),
      mSourceBlendAlpha(GL_ONE),
      mDestBlendAlpha(GL_ZERO),
      mBlendEquationRGB(GL_FUNC_ADD),
      mBlendEquationAlpha(GL_FUNC_ADD),
      mColorMaskRed(true),
      mColorMaskGreen(true),
      mColorMaskBlue(true),
      mColorMaskAlpha(true),
      mSampleAlphaToCoverageEnabled(false),
      mSampleCoverageEnabled(false),
      mSampleCoverageValue(1.0f),
      mSampleCoverageInvert(false),
      mDepthTestEnabled(false),
      mDepthFunc(GL_LESS),
      mDepthMask(true),
      mStencilTestEnabled(false),
      mStencilFrontFunc(GL_ALWAYS),
      mStencilFrontValueMask(static_cast<GLuint>(-1)),
      mStencilFrontStencilFailOp(GL_KEEP),
      mStencilFrontStencilPassDepthFailOp(GL_KEEP),
      mStencilFrontStencilPassDepthPassOp(GL_KEEP),
      mStencilFrontWritemask(static_cast<GLuint>(-1)),
      mStencilBackFunc(GL_ALWAYS),
      mStencilBackValueMask(static_cast<GLuint>(-1)),
      mStencilBackStencilFailOp(GL_KEEP),
      mStencilBackStencilPassDepthFailOp(GL_KEEP),
      mStencilBackStencilPassDepthPassOp(GL_KEEP),
      mStencilBackWritemask(static_cast<GLuint>(-1)),
      mCullFaceEnabled(false),
      mCullFace(GL_BACK),
      mFrontFace(GL_CCW),
      mPolygonOffsetFillEnabled(false),
      mPolygonOffsetFactor(0.0f),
      mPolygonOffsetUnits(0.0f),
      mMultisampleEnabled(true),
      mRasterizerDiscardEnabled(false),
      mLineWidth(1.0f),
      mPrimitiveRestartEnabled(false),
      mClearColor(0.0f, 0.0f, 0.0f, 0.0f),
      mClearDepth(1.0f),
      mClearStencil(0),
      mLocalDirtyBits()
{
    ASSERT(mFunctions);

    mTextures[GL_TEXTURE_2D].resize(rendererCaps.maxCombinedTextureImageUnits);
    mTextures[GL_TEXTURE_CUBE_MAP].resize(rendererCaps.maxCombinedTextureImageUnits);
    mTextures[GL_TEXTURE_2D_ARRAY].resize(rendererCaps.maxCombinedTextureImageUnits);
    mTextures[GL_TEXTURE_3D].resize(rendererCaps.maxCombinedTextureImageUnits);

    // Initialize point sprite state for desktop GL
    if (mFunctions->standard == STANDARD_GL_DESKTOP)
    {
        mFunctions->enable(GL_PROGRAM_POINT_SIZE);

        // GL_POINT_SPRITE was deprecated in the core profile. Point rasterization is always
        // performed
        // as though POINT_SPRITE were enabled.
        if ((mFunctions->profile & GL_CONTEXT_CORE_PROFILE_BIT) == 0)
        {
            mFunctions->enable(GL_POINT_SPRITE);
        }
    }
}

void StateManagerGL::deleteProgram(GLuint program)
{
    if (program != 0)
    {
        if (mProgram == program)
        {
            useProgram(0);
        }

        mFunctions->deleteProgram(program);
    }
}

void StateManagerGL::deleteVertexArray(GLuint vao)
{
    if (vao != 0)
    {
        if (mVAO == vao)
        {
            bindVertexArray(0, 0);
        }

        mFunctions->deleteVertexArrays(1, &vao);
    }
}

void StateManagerGL::deleteTexture(GLuint texture)
{
    if (texture != 0)
    {
        for (const auto &textureTypeIter : mTextures)
        {
            const std::vector<GLuint> &textureVector = textureTypeIter.second;
            for (size_t textureUnitIndex = 0; textureUnitIndex < textureVector.size(); textureUnitIndex++)
            {
                if (textureVector[textureUnitIndex] == texture)
                {
                    activeTexture(textureUnitIndex);
                    bindTexture(textureTypeIter.first, 0);
                }
            }
        }

        mFunctions->deleteTextures(1, &texture);
    }
}
void StateManagerGL::deleteBuffer(GLuint buffer)
{
    if (buffer != 0)
    {
        for (const auto &bufferTypeIter : mBuffers)
        {
            if (bufferTypeIter.second == buffer)
            {
                bindBuffer(bufferTypeIter.first, 0);
            }
        }

        mFunctions->deleteBuffers(1, &buffer);
    }
}

void StateManagerGL::deleteFramebuffer(GLuint fbo)
{
    if (fbo != 0)
    {
        for (size_t binding = 0; binding < mFramebuffers.size(); ++binding)
        {
            if (mFramebuffers[binding] == fbo)
            {
                GLenum enumValue = angle::FramebufferBindingToEnum(
                    static_cast<angle::FramebufferBinding>(binding));
                bindFramebuffer(enumValue, 0);
            }
            mFunctions->deleteFramebuffers(1, &fbo);
        }
    }
}

void StateManagerGL::deleteRenderbuffer(GLuint rbo)
{
    if (rbo != 0)
    {
        if (mRenderbuffer == rbo)
        {
            bindRenderbuffer(GL_RENDERBUFFER, 0);
        }

        mFunctions->deleteRenderbuffers(1, &rbo);
    }
}

void StateManagerGL::useProgram(GLuint program)
{
    if (mProgram != program)
    {
        mProgram = program;
        mFunctions->useProgram(mProgram);
    }
}

void StateManagerGL::bindVertexArray(GLuint vao, GLuint elementArrayBuffer)
{
    if (mVAO != vao)
    {
        mVAO = vao;
        mBuffers[GL_ELEMENT_ARRAY_BUFFER] = elementArrayBuffer;
        mFunctions->bindVertexArray(vao);
    }
}

void StateManagerGL::bindBuffer(GLenum type, GLuint buffer)
{
    if (mBuffers[type] != buffer)
    {
        mBuffers[type] = buffer;
        mFunctions->bindBuffer(type, buffer);
    }
}

void StateManagerGL::activeTexture(size_t unit)
{
    if (mTextureUnitIndex != unit)
    {
        mTextureUnitIndex = unit;
        mFunctions->activeTexture(GL_TEXTURE0 + static_cast<GLenum>(mTextureUnitIndex));
    }
}

void StateManagerGL::bindTexture(GLenum type, GLuint texture)
{
    if (mTextures[type][mTextureUnitIndex] != texture)
    {
        mTextures[type][mTextureUnitIndex] = texture;
        mFunctions->bindTexture(type, texture);
    }
}

void StateManagerGL::setPixelUnpackState(const gl::PixelUnpackState &unpack, bool setDirty)
{
    const gl::Buffer *unpackBuffer = unpack.pixelBuffer.get();
    if (unpackBuffer != nullptr)
    {
        UNIMPLEMENTED();
    }
    setPixelUnpackState(unpack.alignment, unpack.rowLength, unpack.skipRows, unpack.skipPixels,
                        unpack.imageHeight, unpack.skipImages, setDirty);
}

void StateManagerGL::setPixelUnpackState(GLint alignment,
                                         GLint rowLength,
                                         GLint skipRows,
                                         GLint skipPixels,
                                         GLint imageHeight,
                                         GLint skipImages,
                                         bool setDirty)
{
    if (mUnpackAlignment != alignment)
    {
        mUnpackAlignment = alignment;
        mFunctions->pixelStorei(GL_UNPACK_ALIGNMENT, mUnpackAlignment);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_UNPACK_ALIGNMENT);
        }
    }

    if (mUnpackRowLength != rowLength)
    {
        mUnpackRowLength = rowLength;
        mFunctions->pixelStorei(GL_UNPACK_ROW_LENGTH, mUnpackRowLength);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_UNPACK_ROW_LENGTH);
        }
    }

    if (mUnpackSkipRows != skipRows)
    {
        mUnpackSkipRows = rowLength;
        mFunctions->pixelStorei(GL_UNPACK_SKIP_ROWS, mUnpackSkipRows);

        // TODO: set dirty bit once one exists
    }

    if (mUnpackSkipPixels != skipPixels)
    {
        mUnpackSkipPixels = skipPixels;
        mFunctions->pixelStorei(GL_UNPACK_SKIP_PIXELS, mUnpackSkipPixels);

        // TODO: set dirty bit once one exists
    }

    if (mUnpackImageHeight != imageHeight)
    {
        mUnpackImageHeight = imageHeight;
        mFunctions->pixelStorei(GL_UNPACK_IMAGE_HEIGHT, mUnpackImageHeight);

        // TODO: set dirty bit once one exists
    }

    if (mUnpackSkipImages != skipImages)
    {
        mUnpackSkipImages = skipImages;
        mFunctions->pixelStorei(GL_UNPACK_SKIP_IMAGES, mUnpackSkipImages);

        // TODO: set dirty bit once one exists
    }
}

void StateManagerGL::setPixelPackState(const gl::PixelPackState &pack, bool setDirty)
{
    const gl::Buffer *packBuffer = pack.pixelBuffer.get();
    if (packBuffer != nullptr)
    {
        UNIMPLEMENTED();
    }
    setPixelPackState(pack.alignment, pack.rowLength, pack.skipRows, pack.skipPixels, setDirty);
}

void StateManagerGL::setPixelPackState(GLint alignment,
                                       GLint rowLength,
                                       GLint skipRows,
                                       GLint skipPixels,
                                       bool setDirty)
{
    if (mPackAlignment != alignment)
    {
        mPackAlignment = alignment;
        mFunctions->pixelStorei(GL_PACK_ALIGNMENT, mPackAlignment);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_PACK_ALIGNMENT);
        }
    }

    if (mPackRowLength != rowLength)
    {
        mPackRowLength = rowLength;
        mFunctions->pixelStorei(GL_PACK_ROW_LENGTH, mPackRowLength);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_UNPACK_ROW_LENGTH);
        }
    }

    if (mPackSkipRows != skipRows)
    {
        mPackSkipRows = rowLength;
        mFunctions->pixelStorei(GL_PACK_SKIP_ROWS, mPackSkipRows);

        // TODO: set dirty bit once one exists
    }

    if (mPackSkipPixels != skipPixels)
    {
        mPackSkipPixels = skipPixels;
        mFunctions->pixelStorei(GL_PACK_SKIP_PIXELS, mPackSkipPixels);

        // TODO: set dirty bit once one exists
    }
}

void StateManagerGL::bindFramebuffer(GLenum type, GLuint framebuffer)
{
    if (type == GL_FRAMEBUFFER)
    {
        if (mFramebuffers[angle::FramebufferBindingRead] != framebuffer ||
            mFramebuffers[angle::FramebufferBindingDraw] != framebuffer)
        {
            mFramebuffers[angle::FramebufferBindingRead] = framebuffer;
            mFramebuffers[angle::FramebufferBindingDraw] = framebuffer;
            mFunctions->bindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        }
    }
    else
    {
        angle::FramebufferBinding binding = angle::EnumToFramebufferBinding(type);

        if (mFramebuffers[binding] != framebuffer)
        {
            mFramebuffers[binding] = framebuffer;
            mFunctions->bindFramebuffer(type, framebuffer);
        }
    }
}

void StateManagerGL::bindRenderbuffer(GLenum type, GLuint renderbuffer)
{
    ASSERT(type == GL_RENDERBUFFER);
    if (mRenderbuffer != renderbuffer)
    {
        mRenderbuffer = renderbuffer;
        mFunctions->bindRenderbuffer(type, mRenderbuffer);
    }
}

gl::Error StateManagerGL::setDrawArraysState(const gl::Data &data, GLint first, GLsizei count)
{
    const gl::State &state = *data.state;

    const gl::Program *program = state.getProgram();
    const ProgramGL *programGL = GetImplAs<ProgramGL>(program);

    const gl::VertexArray *vao = state.getVertexArray();
    const VertexArrayGL *vaoGL = GetImplAs<VertexArrayGL>(vao);

    gl::Error error =
        vaoGL->syncDrawArraysState(programGL->getActiveAttributesMask(), first, count);
    if (error.isError())
    {
        return error;
    }

    bindVertexArray(vaoGL->getVertexArrayID(), vaoGL->getAppliedElementArrayBufferID());

    return setGenericDrawState(data);
}

gl::Error StateManagerGL::setDrawElementsState(const gl::Data &data, GLsizei count, GLenum type, const GLvoid *indices,
                                               const GLvoid **outIndices)
{
    const gl::State &state = *data.state;

    const gl::Program *program = state.getProgram();
    const ProgramGL *programGL = GetImplAs<ProgramGL>(program);

    const gl::VertexArray *vao = state.getVertexArray();
    const VertexArrayGL *vaoGL = GetImplAs<VertexArrayGL>(vao);

    gl::Error error = vaoGL->syncDrawElementsState(programGL->getActiveAttributesMask(), count,
                                                   type, indices, outIndices);
    if (error.isError())
    {
        return error;
    }

    bindVertexArray(vaoGL->getVertexArrayID(), vaoGL->getAppliedElementArrayBufferID());

    return setGenericDrawState(data);
}

gl::Error StateManagerGL::setGenericDrawState(const gl::Data &data)
{
    const gl::State &state = *data.state;

    const gl::Program *program = state.getProgram();
    const ProgramGL *programGL = GetImplAs<ProgramGL>(program);
    useProgram(programGL->getProgramID());

    const std::vector<SamplerBindingGL> &appliedSamplerUniforms = programGL->getAppliedSamplerUniforms();
    for (const SamplerBindingGL &samplerUniform : appliedSamplerUniforms)
    {
        GLenum textureType = samplerUniform.textureType;
        for (GLuint textureUnitIndex : samplerUniform.boundTextureUnits)
        {
            const gl::Texture *texture = state.getSamplerTexture(textureUnitIndex, textureType);
            if (texture != nullptr)
            {
                const TextureGL *textureGL = GetImplAs<TextureGL>(texture);

                if (mTextures[textureType][textureUnitIndex] != textureGL->getTextureID())
                {
                    activeTexture(textureUnitIndex);
                    bindTexture(textureType, textureGL->getTextureID());
                }

                textureGL->syncSamplerState(texture->getSamplerState());

                // TODO: apply sampler object if one is bound
            }
            else
            {
                if (mTextures[textureType][textureUnitIndex] != 0)
                {
                    activeTexture(textureUnitIndex);
                    bindTexture(textureType, 0);
                }
            }
        }
    }

    const gl::Framebuffer *framebuffer = state.getDrawFramebuffer();
    const FramebufferGL *framebufferGL = GetImplAs<FramebufferGL>(framebuffer);
    bindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferGL->getFramebufferID());

    return gl::Error(GL_NO_ERROR);
}

void StateManagerGL::setAttributeCurrentData(size_t index,
                                             const gl::VertexAttribCurrentValueData &data,
                                             bool setDirty)
{
    if (mVertexAttribCurrentValues[index] != data)
    {
        mVertexAttribCurrentValues[index] = data;
        switch (mVertexAttribCurrentValues[index].Type)
        {
            case GL_FLOAT:
                mFunctions->vertexAttrib4fv(static_cast<GLuint>(index),
                                            mVertexAttribCurrentValues[index].FloatValues);
                break;
            case GL_INT:
                mFunctions->vertexAttrib4iv(static_cast<GLuint>(index),
                                            mVertexAttribCurrentValues[index].IntValues);
                break;
            case GL_UNSIGNED_INT:
                mFunctions->vertexAttrib4uiv(static_cast<GLuint>(index),
                                             mVertexAttribCurrentValues[index].UnsignedIntValues);
                break;
          default: UNREACHABLE();
        }

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_CURRENT_VALUE_0 + index);
        }
    }
}

void StateManagerGL::setScissorTestEnabled(bool enabled, bool setDirty)
{
    if (mScissorTestEnabled != enabled)
    {
        mScissorTestEnabled = enabled;
        if (mScissorTestEnabled)
        {
            mFunctions->enable(GL_SCISSOR_TEST);
        }
        else
        {
            mFunctions->disable(GL_SCISSOR_TEST);
        }

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_SCISSOR_TEST_ENABLED);
        }
    }
}

void StateManagerGL::setScissor(const gl::Rectangle &scissor, bool setDirty)
{
    if (scissor != mScissor)
    {
        mScissor = scissor;
        mFunctions->scissor(mScissor.x, mScissor.y, mScissor.width, mScissor.height);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_SCISSOR);
        }
    }
}

void StateManagerGL::setViewport(const gl::Rectangle &viewport, bool setDirty)
{
    if (viewport != mViewport)
    {
        mViewport = viewport;
        mFunctions->viewport(mViewport.x, mViewport.y, mViewport.width, mViewport.height);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_VIEWPORT);
        }
    }
}

void StateManagerGL::setDepthRange(float near, float far, bool setDirty)
{
    if (mNear != near || mFar != far)
    {
        mNear = near;
        mFar = far;
        mFunctions->depthRange(mNear, mFar);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_DEPTH_RANGE);
        }
    }
}

void StateManagerGL::setBlendEnabled(bool enabled, bool setDirty)
{
    if (mBlendEnabled != enabled)
    {
        mBlendEnabled = enabled;
        if (mBlendEnabled)
        {
            mFunctions->enable(GL_BLEND);
        }
        else
        {
            mFunctions->disable(GL_BLEND);
        }

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_BLEND_ENABLED);
        }
    }
}

void StateManagerGL::setBlendColor(const gl::ColorF &blendColor, bool setDirty)
{
    if (mBlendColor != blendColor)
    {
        mBlendColor = blendColor;
        mFunctions->blendColor(mBlendColor.red, mBlendColor.green, mBlendColor.blue, mBlendColor.alpha);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_BLEND_COLOR);
        }
    }
}

void StateManagerGL::setBlendFuncs(GLenum sourceBlendRGB,
                                   GLenum destBlendRGB,
                                   GLenum sourceBlendAlpha,
                                   GLenum destBlendAlpha,
                                   bool setDirty)
{
    if (mSourceBlendRGB != sourceBlendRGB || mDestBlendRGB != destBlendRGB ||
        mSourceBlendAlpha != sourceBlendAlpha || mDestBlendAlpha != destBlendAlpha)
    {
        mSourceBlendRGB = sourceBlendRGB;
        mDestBlendRGB = destBlendRGB;
        mSourceBlendAlpha = sourceBlendAlpha;
        mDestBlendAlpha = destBlendAlpha;

        mFunctions->blendFuncSeparate(mSourceBlendRGB, mDestBlendRGB, mSourceBlendAlpha, mDestBlendAlpha);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_BLEND_FUNCS);
        }
    }
}

void StateManagerGL::setBlendEquations(GLenum blendEquationRGB,
                                       GLenum blendEquationAlpha,
                                       bool setDirty)
{
    if (mBlendEquationRGB != blendEquationRGB || mBlendEquationAlpha != blendEquationAlpha)
    {
        mBlendEquationRGB = blendEquationRGB;
        mBlendEquationAlpha = blendEquationAlpha;

        mFunctions->blendEquationSeparate(mBlendEquationRGB, mBlendEquationAlpha);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_BLEND_EQUATIONS);
        }
    }
}

void StateManagerGL::setColorMask(bool red, bool green, bool blue, bool alpha, bool setDirty)
{
    if (mColorMaskRed != red || mColorMaskGreen != green || mColorMaskBlue != blue || mColorMaskAlpha != alpha)
    {
        mColorMaskRed = red;
        mColorMaskGreen = green;
        mColorMaskBlue = blue;
        mColorMaskAlpha = alpha;
        mFunctions->colorMask(mColorMaskRed, mColorMaskGreen, mColorMaskBlue, mColorMaskAlpha);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_COLOR_MASK);
        }
    }
}

void StateManagerGL::setSampleAlphaToCoverageEnabled(bool enabled, bool setDirty)
{
    if (mSampleAlphaToCoverageEnabled != enabled)
    {
        mSampleAlphaToCoverageEnabled = enabled;
        if (mSampleAlphaToCoverageEnabled)
        {
            mFunctions->enable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        }
        else
        {
            mFunctions->disable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        }

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE_ENABLED);
        }
    }
}

void StateManagerGL::setSampleCoverageEnabled(bool enabled, bool setDirty)
{
    if (mSampleCoverageEnabled != enabled)
    {
        mSampleCoverageEnabled = enabled;
        if (mSampleCoverageEnabled)
        {
            mFunctions->enable(GL_SAMPLE_COVERAGE);
        }
        else
        {
            mFunctions->disable(GL_SAMPLE_COVERAGE);
        }

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_SAMPLE_COVERAGE_ENABLED);
        }
    }
}

void StateManagerGL::setSampleCoverage(float value, bool invert, bool setDirty)
{
    if (mSampleCoverageValue != value || mSampleCoverageInvert != invert)
    {
        mSampleCoverageValue = value;
        mSampleCoverageInvert = invert;
        mFunctions->sampleCoverage(mSampleCoverageValue, mSampleCoverageInvert);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_SAMPLE_COVERAGE);
        }
    }
}

void StateManagerGL::setDepthTestEnabled(bool enabled, bool setDirty)
{
    if (mDepthTestEnabled != enabled)
    {
        mDepthTestEnabled = enabled;
        if (mDepthTestEnabled)
        {
            mFunctions->enable(GL_DEPTH_TEST);
        }
        else
        {
            mFunctions->disable(GL_DEPTH_TEST);
        }

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_DEPTH_TEST_ENABLED);
        }
    }
}

void StateManagerGL::setDepthFunc(GLenum depthFunc, bool setDirty)
{
    if (mDepthFunc != depthFunc)
    {
        mDepthFunc = depthFunc;
        mFunctions->depthFunc(mDepthFunc);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_DEPTH_FUNC);
        }
    }
}

void StateManagerGL::setDepthMask(bool mask, bool setDirty)
{
    if (mDepthMask != mask)
    {
        mDepthMask = mask;
        mFunctions->depthMask(mDepthMask);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_DEPTH_MASK);
        }
    }
}

void StateManagerGL::setStencilTestEnabled(bool enabled, bool setDirty)
{
    if (mStencilTestEnabled != enabled)
    {
        mStencilTestEnabled = enabled;
        if (mStencilTestEnabled)
        {
            mFunctions->enable(GL_STENCIL_TEST);
        }
        else
        {
            mFunctions->disable(GL_STENCIL_TEST);
        }

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_STENCIL_TEST_ENABLED);
        }
    }
}

void StateManagerGL::setStencilFrontWritemask(GLuint mask, bool setDirty)
{
    if (mStencilFrontWritemask != mask)
    {
        mStencilFrontWritemask = mask;
        mFunctions->stencilMaskSeparate(GL_FRONT, mStencilFrontWritemask);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_STENCIL_WRITEMASK_FRONT);
        }
    }
}

void StateManagerGL::setStencilBackWritemask(GLuint mask, bool setDirty)
{
    if (mStencilBackWritemask != mask)
    {
        mStencilBackWritemask = mask;
        mFunctions->stencilMaskSeparate(GL_BACK, mStencilBackWritemask);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_STENCIL_WRITEMASK_BACK);
        }
    }
}

void StateManagerGL::setStencilFrontFuncs(GLenum func, GLint ref, GLuint mask, bool setDirty)
{
    if (mStencilFrontFunc != func || mStencilFrontRef != ref || mStencilFrontValueMask != mask)
    {
        mStencilFrontFunc = func;
        mStencilFrontRef = ref;
        mStencilFrontValueMask = mask;
        mFunctions->stencilFuncSeparate(GL_FRONT, mStencilFrontFunc, mStencilFrontRef, mStencilFrontValueMask);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_STENCIL_FUNCS_FRONT);
        }
    }
}

void StateManagerGL::setStencilBackFuncs(GLenum func, GLint ref, GLuint mask, bool setDirty)
{
    if (mStencilBackFunc != func || mStencilBackRef != ref || mStencilBackValueMask != mask)
    {
        mStencilBackFunc = func;
        mStencilBackRef = ref;
        mStencilBackValueMask = mask;
        mFunctions->stencilFuncSeparate(GL_BACK, mStencilBackFunc, mStencilBackRef, mStencilBackValueMask);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_STENCIL_FUNCS_BACK);
        }
    }
}

void StateManagerGL::setStencilFrontOps(GLenum sfail, GLenum dpfail, GLenum dppass, bool setDirty)
{
    if (mStencilFrontStencilFailOp != sfail || mStencilFrontStencilPassDepthFailOp != dpfail || mStencilFrontStencilPassDepthPassOp != dppass)
    {
        mStencilFrontStencilFailOp = sfail;
        mStencilFrontStencilPassDepthFailOp = dpfail;
        mStencilFrontStencilPassDepthPassOp = dppass;
        mFunctions->stencilOpSeparate(GL_FRONT, mStencilFrontStencilFailOp, mStencilFrontStencilPassDepthFailOp, mStencilFrontStencilPassDepthPassOp);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_STENCIL_OPS_FRONT);
        }
    }
}

void StateManagerGL::setStencilBackOps(GLenum sfail, GLenum dpfail, GLenum dppass, bool setDirty)
{
    if (mStencilBackStencilFailOp != sfail || mStencilBackStencilPassDepthFailOp != dpfail || mStencilBackStencilPassDepthPassOp != dppass)
    {
        mStencilBackStencilFailOp = sfail;
        mStencilBackStencilPassDepthFailOp = dpfail;
        mStencilBackStencilPassDepthPassOp = dppass;
        mFunctions->stencilOpSeparate(GL_BACK, mStencilBackStencilFailOp, mStencilBackStencilPassDepthFailOp, mStencilBackStencilPassDepthPassOp);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_STENCIL_OPS_BACK);
        }
    }
}

void StateManagerGL::setCullFaceEnabled(bool enabled, bool setDirty)
{
    if (mCullFaceEnabled != enabled)
    {
        mCullFaceEnabled = enabled;
        if (mCullFaceEnabled)
        {
            mFunctions->enable(GL_CULL_FACE);
        }
        else
        {
            mFunctions->disable(GL_CULL_FACE);
        }

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_CULL_FACE_ENABLED);
        }
    }
}

void StateManagerGL::setCullFace(GLenum cullFace, bool setDirty)
{
    if (mCullFace != cullFace)
    {
        mCullFace = cullFace;
        mFunctions->cullFace(mCullFace);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_CULL_FACE);
        }
    }
}

void StateManagerGL::setFrontFace(GLenum frontFace, bool setDirty)
{
    if (mFrontFace != frontFace)
    {
        mFrontFace = frontFace;
        mFunctions->frontFace(mFrontFace);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_FRONT_FACE);
        }
    }
}

void StateManagerGL::setPolygonOffsetFillEnabled(bool enabled, bool setDirty)
{
    if (mPolygonOffsetFillEnabled != enabled)
    {
        mPolygonOffsetFillEnabled = enabled;
        if (mPolygonOffsetFillEnabled)
        {
            mFunctions->enable(GL_POLYGON_OFFSET_FILL);
        }
        else
        {
            mFunctions->disable(GL_POLYGON_OFFSET_FILL);
        }

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_POLYGON_OFFSET_FILL_ENABLED);
        }
    }
}

void StateManagerGL::setPolygonOffset(float factor, float units, bool setDirty)
{
    if (mPolygonOffsetFactor != factor || mPolygonOffsetUnits != units)
    {
        mPolygonOffsetFactor = factor;
        mPolygonOffsetUnits = units;
        mFunctions->polygonOffset(mPolygonOffsetFactor, mPolygonOffsetUnits);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_POLYGON_OFFSET);
        }
    }
}

void StateManagerGL::setMultisampleEnabled(bool enabled, bool setDirty)
{
    if (mMultisampleEnabled != enabled)
    {
        mMultisampleEnabled = enabled;
        if (mMultisampleEnabled)
        {
            mFunctions->enable(GL_MULTISAMPLE);
        }
        else
        {
            mFunctions->disable(GL_MULTISAMPLE);
        }

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_MULTISAMPLE_ENABLED);
        }
    }
}

void StateManagerGL::setRasterizerDiscardEnabled(bool enabled, bool setDirty)
{
    if (mRasterizerDiscardEnabled != enabled)
    {
        mRasterizerDiscardEnabled = enabled;
        if (mRasterizerDiscardEnabled)
        {
            mFunctions->enable(GL_RASTERIZER_DISCARD);
        }
        else
        {
            mFunctions->disable(GL_RASTERIZER_DISCARD);
        }

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_RASTERIZER_DISCARD_ENABLED);
        }
    }
}

void StateManagerGL::setLineWidth(float width, bool setDirty)
{
    if (mLineWidth != width)
    {
        mLineWidth = width;
        mFunctions->lineWidth(mLineWidth);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_LINE_WIDTH);
        }
    }
}

void StateManagerGL::setPrimitiveRestartEnabled(bool enabled, bool setDirty)
{
    if (mPrimitiveRestartEnabled != enabled)
    {
        mPrimitiveRestartEnabled = enabled;

        if (mPrimitiveRestartEnabled)
        {
            mFunctions->enable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
        }
        else
        {
            mFunctions->disable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
        }

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_PRIMITIVE_RESTART_ENABLED);
        }
    }
}

void StateManagerGL::setClearDepth(float clearDepth, bool setDirty)
{
    if (mClearDepth != clearDepth)
    {
        mClearDepth = clearDepth;
        mFunctions->clearDepth(mClearDepth);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_CLEAR_DEPTH);
        }
    }
}

void StateManagerGL::setClearColor(const gl::ColorF &clearColor, bool setDirty)
{
    if (mClearColor != clearColor)
    {
        mClearColor = clearColor;
        mFunctions->clearColor(mClearColor.red, mClearColor.green, mClearColor.blue, mClearColor.alpha);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_CLEAR_COLOR);
        }
    }
}

void StateManagerGL::setClearStencil(GLint clearStencil, bool setDirty)
{
    if (mClearStencil != clearStencil)
    {
        mClearStencil = clearStencil;
        mFunctions->clearStencil(mClearStencil);

        if (setDirty)
        {
            mLocalDirtyBits.set(gl::State::DIRTY_BIT_CLEAR_STENCIL);
        }
    }
}

void StateManagerGL::syncState(const gl::State &state, const gl::State::DirtyBits &dirtyBits)
{
    // TODO(jmadill): Investigate only syncing vertex state for active attributes
    for (unsigned int dirtyBit : angle::IterateBitSet(dirtyBits | mLocalDirtyBits))
    {
        switch (dirtyBit)
        {
            case gl::State::DIRTY_BIT_SCISSOR_TEST_ENABLED:
                setScissorTestEnabled(state.isScissorTestEnabled(), false);
                break;
            case gl::State::DIRTY_BIT_SCISSOR:
                setScissor(state.getScissor(), false);
                break;
            case gl::State::DIRTY_BIT_VIEWPORT:
                setViewport(state.getViewport(), false);
                break;
            case gl::State::DIRTY_BIT_DEPTH_RANGE:
                setDepthRange(state.getNearPlane(), state.getFarPlane(), false);
                break;
            case gl::State::DIRTY_BIT_BLEND_ENABLED:
                setBlendEnabled(state.isBlendEnabled(), false);
                break;
            case gl::State::DIRTY_BIT_BLEND_COLOR:
                setBlendColor(state.getBlendColor(), false);
                break;
            case gl::State::DIRTY_BIT_BLEND_FUNCS:
            {
                const auto &blendState = state.getBlendState();
                setBlendFuncs(blendState.sourceBlendRGB, blendState.destBlendRGB,
                              blendState.sourceBlendAlpha, blendState.destBlendAlpha, false);
                break;
            }
            case gl::State::DIRTY_BIT_BLEND_EQUATIONS:
            {
                const auto &blendState = state.getBlendState();
                setBlendEquations(blendState.blendEquationRGB, blendState.blendEquationAlpha,
                                  false);
                break;
            }
            case gl::State::DIRTY_BIT_COLOR_MASK:
            {
                const auto &blendState = state.getBlendState();
                setColorMask(blendState.colorMaskRed, blendState.colorMaskGreen,
                             blendState.colorMaskBlue, blendState.colorMaskAlpha, false);
                break;
            }
            case gl::State::DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE_ENABLED:
                setSampleAlphaToCoverageEnabled(state.isSampleAlphaToCoverageEnabled(), false);
                break;
            case gl::State::DIRTY_BIT_SAMPLE_COVERAGE_ENABLED:
                setSampleCoverageEnabled(state.isSampleCoverageEnabled(), false);
                break;
            case gl::State::DIRTY_BIT_SAMPLE_COVERAGE:
                setSampleCoverage(state.getSampleCoverageValue(), state.getSampleCoverageInvert(),
                                  false);
                break;
            case gl::State::DIRTY_BIT_DEPTH_TEST_ENABLED:
                setDepthTestEnabled(state.isDepthTestEnabled(), false);
                break;
            case gl::State::DIRTY_BIT_DEPTH_FUNC:
                setDepthFunc(state.getDepthStencilState().depthFunc, false);
                break;
            case gl::State::DIRTY_BIT_DEPTH_MASK:
                setDepthMask(state.getDepthStencilState().depthMask, false);
                break;
            case gl::State::DIRTY_BIT_STENCIL_TEST_ENABLED:
                setStencilTestEnabled(state.isStencilTestEnabled(), false);
                break;
            case gl::State::DIRTY_BIT_STENCIL_FUNCS_FRONT:
            {
                const auto &depthStencilState = state.getDepthStencilState();
                setStencilFrontFuncs(depthStencilState.stencilFunc, state.getStencilRef(),
                                     depthStencilState.stencilMask, false);
                break;
            }
            case gl::State::DIRTY_BIT_STENCIL_FUNCS_BACK:
            {
                const auto &depthStencilState = state.getDepthStencilState();
                setStencilBackFuncs(depthStencilState.stencilBackFunc, state.getStencilBackRef(),
                                    depthStencilState.stencilBackMask, false);
                break;
            }
            case gl::State::DIRTY_BIT_STENCIL_OPS_FRONT:
            {
                const auto &depthStencilState = state.getDepthStencilState();
                setStencilFrontOps(depthStencilState.stencilFail,
                                   depthStencilState.stencilPassDepthFail,
                                   depthStencilState.stencilPassDepthPass, false);
                break;
            }
            case gl::State::DIRTY_BIT_STENCIL_OPS_BACK:
            {
                const auto &depthStencilState = state.getDepthStencilState();
                setStencilBackOps(depthStencilState.stencilBackFail,
                                  depthStencilState.stencilBackPassDepthFail,
                                  depthStencilState.stencilBackPassDepthPass, false);
                break;
            }
            case gl::State::DIRTY_BIT_STENCIL_WRITEMASK_FRONT:
                setStencilFrontWritemask(state.getDepthStencilState().stencilWritemask, false);
                break;
            case gl::State::DIRTY_BIT_STENCIL_WRITEMASK_BACK:
                setStencilBackWritemask(state.getDepthStencilState().stencilBackWritemask, false);
                break;
            case gl::State::DIRTY_BIT_CULL_FACE_ENABLED:
                setCullFaceEnabled(state.isCullFaceEnabled(), false);
                break;
            case gl::State::DIRTY_BIT_CULL_FACE:
                setCullFace(state.getRasterizerState().cullMode, false);
                break;
            case gl::State::DIRTY_BIT_FRONT_FACE:
                setFrontFace(state.getRasterizerState().frontFace, false);
                break;
            case gl::State::DIRTY_BIT_POLYGON_OFFSET_FILL_ENABLED:
                setPolygonOffsetFillEnabled(state.isPolygonOffsetFillEnabled(), false);
                break;
            case gl::State::DIRTY_BIT_POLYGON_OFFSET:
            {
                const auto &rasterizerState = state.getRasterizerState();
                setPolygonOffset(rasterizerState.polygonOffsetFactor,
                                 rasterizerState.polygonOffsetUnits, false);
                break;
            }
            case gl::State::DIRTY_BIT_MULTISAMPLE_ENABLED:
                setMultisampleEnabled(state.getRasterizerState().multiSample, false);
                break;
            case gl::State::DIRTY_BIT_RASTERIZER_DISCARD_ENABLED:
                setRasterizerDiscardEnabled(state.isRasterizerDiscardEnabled(), false);
                break;
            case gl::State::DIRTY_BIT_LINE_WIDTH:
                setLineWidth(state.getLineWidth(), false);
                break;
            case gl::State::DIRTY_BIT_PRIMITIVE_RESTART_ENABLED:
                setPrimitiveRestartEnabled(state.isPrimitiveRestartEnabled(), false);
                break;
            case gl::State::DIRTY_BIT_CLEAR_COLOR:
                setClearColor(state.getColorClearValue(), false);
                break;
            case gl::State::DIRTY_BIT_CLEAR_DEPTH:
                setClearDepth(state.getDepthClearValue(), false);
                break;
            case gl::State::DIRTY_BIT_CLEAR_STENCIL:
                setClearStencil(state.getStencilClearValue(), false);
                break;
            case gl::State::DIRTY_BIT_UNPACK_ALIGNMENT:
                // TODO(jmadill): split this
                setPixelUnpackState(state.getUnpackState(), false);
                break;
            case gl::State::DIRTY_BIT_UNPACK_ROW_LENGTH:
                // TODO(jmadill): split this
                setPixelUnpackState(state.getUnpackState(), false);
                break;
            case gl::State::DIRTY_BIT_PACK_ALIGNMENT:
                // TODO(jmadill): split this
                setPixelPackState(state.getPackState(), false);
                break;
            case gl::State::DIRTY_BIT_PACK_REVERSE_ROW_ORDER:
                // TODO(jmadill): split this
                setPixelPackState(state.getPackState(), false);
                break;
            case gl::State::DIRTY_BIT_DITHER_ENABLED:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_GENERATE_MIPMAP_HINT:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_SHADER_DERIVATIVE_HINT:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_READ_FRAMEBUFFER_BINDING:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_READ_FRAMEBUFFER_OBJECT:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_DRAW_FRAMEBUFFER_BINDING:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_DRAW_FRAMEBUFFER_OBJECT:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_RENDERBUFFER_BINDING:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_VERTEX_ARRAY_BINDING:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_VERTEX_ARRAY_OBJECT:
                state.getVertexArray()->syncImplState();
                break;
            case gl::State::DIRTY_BIT_PROGRAM_BINDING:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_PROGRAM_OBJECT:
                // TODO(jmadill): implement this
                break;
            default:
            {
                ASSERT(dirtyBit >= gl::State::DIRTY_BIT_CURRENT_VALUE_0 &&
                       dirtyBit < gl::State::DIRTY_BIT_CURRENT_VALUE_MAX);
                size_t attribIndex =
                    static_cast<size_t>(dirtyBit) - gl::State::DIRTY_BIT_CURRENT_VALUE_0;
                setAttributeCurrentData(attribIndex, state.getVertexAttribCurrentValue(
                                                         static_cast<unsigned int>(attribIndex)),
                                        false);
                break;
            }
        }

        mLocalDirtyBits.reset();
    }
}
}
