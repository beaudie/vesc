//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// queryutils.cpp: GL and EGL query and conversion utilities.

#include "libANGLE/queryutils.h"

#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/State.h"

namespace
{

// Helper class for converting a GL type to a GLenum:
// We can't use CastStateValueEnum generally, because of GLboolean + GLubyte overlap.
// We restrict our use to CastStateValue, where it eliminates duplicate parameters.

template <typename GLType>
struct CastStateValueEnum { static GLenum mEnumForType; };

template <> GLenum CastStateValueEnum<GLint>::mEnumForType      = GL_INT;
template <> GLenum CastStateValueEnum<GLuint>::mEnumForType     = GL_UNSIGNED_INT;
template <> GLenum CastStateValueEnum<GLboolean>::mEnumForType  = GL_BOOL;
template <> GLenum CastStateValueEnum<GLint64>::mEnumForType    = GL_INT_64_ANGLEX;
template <> GLenum CastStateValueEnum<GLfloat>::mEnumForType    = GL_FLOAT;

template <typename NativeT, typename QueryT>
QueryT CastStateValueToInt(GLenum pname, NativeT value)
{
    GLenum queryType = CastStateValueEnum<QueryT>::mEnumForType;
    GLenum nativeType = CastStateValueEnum<NativeT>::mEnumForType;

    if (nativeType == GL_FLOAT)
    {
        // RGBA color values and DepthRangeF values are converted to integer using Equation 2.4 from Table 4.5
        if (pname == GL_DEPTH_RANGE || pname == GL_COLOR_CLEAR_VALUE || pname == GL_DEPTH_CLEAR_VALUE || pname == GL_BLEND_COLOR)
        {
            return static_cast<QueryT>((static_cast<GLfloat>(0xFFFFFFFF) * value - 1.0f) / 2.0f);
        }
        else
        {
            return gl::iround<QueryT>(value);
        }
    }

    // Clamp 64-bit int values when casting to int
    if (nativeType == GL_INT_64_ANGLEX && queryType == GL_INT)
    {
        GLint64 minIntValue = static_cast<GLint64>(std::numeric_limits<GLint>::min());
        GLint64 maxIntValue = static_cast<GLint64>(std::numeric_limits<GLint>::max());
        GLint64 clampedValue = std::max(std::min(static_cast<GLint64>(value), maxIntValue), minIntValue);
        return static_cast<QueryT>(clampedValue);
    }

    return static_cast<QueryT>(value);
}

template <typename NativeT, typename QueryT>
QueryT CastStateValue(GLenum pname, NativeT value)
{
    GLenum queryType = CastStateValueEnum<QueryT>::mEnumForType;

    switch (queryType)
    {
      case GL_INT:              return CastStateValueToInt<QueryT, NativeT>(pname, value);
      case GL_INT_64_ANGLEX:    return CastStateValueToInt<QueryT, NativeT>(pname, value);
      case GL_FLOAT:            return static_cast<QueryT>(value);
      case GL_BOOL:             return (value == static_cast<NativeT>(0) ? GL_FALSE : GL_TRUE);
      default: UNREACHABLE();   return 0;
    }
}

template <typename NativeT, typename QueryT>
void InsertResult(std::vector<QueryT> &result, GLenum pname, NativeT value)
{
    result.push_back(CastStateValue<NativeT, QueryT>(pname, value));
}

}

namespace gl
{

template <typename QueryT>
std::vector<QueryT> QueryState(const Context *context, GLenum pname)
{
    //const State &state = context->getState();
    const Caps &caps = context->getCaps();
    const Extensions &extensions = context->getExtensions();

    std::vector<QueryT> result;

    switch (pname)
    {
      // Integer query types
      case GL_MAX_VERTEX_ATTRIBS:                       InsertResult<GLint>(result, pname, caps.maxVertexAttributes));                            break;
      case GL_MAX_VERTEX_UNIFORM_VECTORS:               InsertResult<GLint>(result, pname, caps.maxVertexUniformVectors));                        break;
      case GL_MAX_VERTEX_UNIFORM_COMPONENTS:            InsertResult<GLint>(result, pname, caps.maxVertexUniformComponents));                     break;
      case GL_MAX_VARYING_VECTORS:                      InsertResult<GLint>(result, pname, caps.maxVaryingVectors));                              break;
      case GL_MAX_VARYING_COMPONENTS:                   InsertResult<GLint>(result, pname, caps.maxVertexOutputComponents));                      break;
      case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:         InsertResult<GLint>(result, pname, caps.maxCombinedTextureImageUnits));                   break;
      case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:           InsertResult<GLint>(result, pname, caps.maxVertexTextureImageUnits));                     break;
      case GL_MAX_TEXTURE_IMAGE_UNITS:                  InsertResult<GLint>(result, pname, caps.maxTextureImageUnits));                           break;
      case GL_MAX_FRAGMENT_UNIFORM_VECTORS:             InsertResult<GLint>(result, pname, caps.maxFragmentUniformVectors));                      break;
      case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:          InsertResult<GLint>(result, pname, caps.maxFragmentInputComponents));                     break;
      case GL_MAX_RENDERBUFFER_SIZE:                    InsertResult<GLint>(result, pname, caps.maxRenderbufferSize));                            break;
      case GL_MAX_COLOR_ATTACHMENTS_EXT:                InsertResult<GLint>(result, pname, caps.maxColorAttachments));                            break;
      case GL_MAX_DRAW_BUFFERS_EXT:                     InsertResult<GLint>(result, pname, caps.maxDrawBuffers));                                 break;
      //case GL_FRAMEBUFFER_BINDING:                    // now equivalent to GL_DRAW_FRAMEBUFFER_BINDING_ANGLE
      case GL_SUBPIXEL_BITS:                            InsertResult<GLint>(result, pname, 4));                                                   break;
      case GL_MAX_TEXTURE_SIZE:                         InsertResult<GLint>(result, pname, caps.max2DTextureSize));                               break;
      case GL_MAX_CUBE_MAP_TEXTURE_SIZE:                InsertResult<GLint>(result, pname, caps.maxCubeMapTextureSize));                          break;
      case GL_MAX_3D_TEXTURE_SIZE:                      InsertResult<GLint>(result, pname, caps.max3DTextureSize));                               break;
      case GL_MAX_ARRAY_TEXTURE_LAYERS:                 InsertResult<GLint>(result, pname, caps.maxArrayTextureLayers));                          break;
      case GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT:          InsertResult<GLint>(result, pname, caps.uniformBufferOffsetAlignment));                   break;
      case GL_MAX_UNIFORM_BUFFER_BINDINGS:              InsertResult<GLint>(result, pname, caps.maxUniformBufferBindings));                       break;
      case GL_MAX_VERTEX_UNIFORM_BLOCKS:                InsertResult<GLint>(result, pname, caps.maxVertexUniformBlocks));                         break;
      case GL_MAX_FRAGMENT_UNIFORM_BLOCKS:              InsertResult<GLint>(result, pname, caps.maxFragmentUniformBlocks));                       break;
      case GL_MAX_COMBINED_UNIFORM_BLOCKS:              InsertResult<GLint>(result, pname, caps.maxCombinedTextureImageUnits));                   break;
      case GL_MAJOR_VERSION:                            InsertResult<GLint>(result, pname, context->getClientVersion()));                         break;
      case GL_MINOR_VERSION:                            InsertResult<GLint>(result, pname, 0));                                                   break;
      case GL_MAX_ELEMENTS_INDICES:                     InsertResult<GLint>(result, pname, caps.maxElementsIndices));                             break;
      case GL_MAX_ELEMENTS_VERTICES:                    InsertResult<GLint>(result, pname, caps.maxElementsVertices));                            break;
      case GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS: InsertResult<GLint>(result, pname, caps.maxTransformFeedbackInterleavedComponents)); break;
      case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS:       InsertResult<GLint>(result, pname, caps.maxTransformFeedbackSeparateAttributes));    break;
      case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS:    InsertResult<GLint>(result, pname, caps.maxTransformFeedbackSeparateComponents));    break;
      case GL_NUM_COMPRESSED_TEXTURE_FORMATS:           InsertResult<GLint>(result, pname, caps.compressedTextureFormats.size()));                break;
      case GL_MAX_SAMPLES_ANGLE:                        InsertResult<GLint>(result, pname, extensions.maxSamples));                               break;
      case GL_MAX_VIEWPORT_DIMS:
        InsertResult<GLint>(result, pname, caps.maxViewportWidth));
        InsertResult<GLint>(result, pname, caps.maxViewportHeight));
        break;
      case GL_COMPRESSED_TEXTURE_FORMATS:
        for (size_t i = 0; i < caps.compressedTextureFormats.size(); i++)
        {
            InsertResult<GLint>(result, pname, caps.compressedTextureFormats[i]));
        }
        break;
      case GL_RESET_NOTIFICATION_STRATEGY_EXT:
        InsertResult<GLint>(result, pname, context->getResetNotificationStrategy()));
        break;
      case GL_NUM_SHADER_BINARY_FORMATS:
        InsertResult<GLint>(result, pname, caps.shaderBinaryFormats.size()));
        break;
      case GL_SHADER_BINARY_FORMATS:
        for (size_t i = 0; i < caps.shaderBinaryFormats.size(); i++)
        {
            InsertResult<GLint>(result, pname, caps.shaderBinaryFormats[i]));
        }
        break;
      case GL_NUM_PROGRAM_BINARY_FORMATS:
        InsertResult<GLint>(result, pname, caps.programBinaryFormats.size()));
        break;
      case GL_PROGRAM_BINARY_FORMATS:
        for (size_t i = 0; i < caps.programBinaryFormats.size(); i++)
        {
            InsertResult<GLint>(result, pname, caps.programBinaryFormats[i]));
        }
        break;
      case GL_NUM_EXTENSIONS:
        InsertResult<GLint>(result, pname, context->getExtensionStringCount()));
        break;

      // State integer queries
      case GL_ARRAY_BUFFER_BINDING:                     InsertResult<GLint>(result, pname, state.getArrayBufferId()));                              break;
      case GL_ELEMENT_ARRAY_BUFFER_BINDING:             InsertResult<GLint>(result, pname, state.getVertexArray()->getElementArrayBufferId()));     break;
      //case GL_FRAMEBUFFER_BINDING:                    // now equivalent to GL_DRAW_FRAMEBUFFER_BINDING_ANGLE
      case GL_DRAW_FRAMEBUFFER_BINDING_ANGLE:           *params = mDrawFramebuffer->id();                         break;
      case GL_READ_FRAMEBUFFER_BINDING_ANGLE:           *params = mReadFramebuffer->id();                         break;
      case GL_RENDERBUFFER_BINDING:                     *params = mRenderbuffer.id();                             break;
      case GL_VERTEX_ARRAY_BINDING:                     *params = mVertexArray->id();                             break;
      case GL_CURRENT_PROGRAM:                          InsertResult<GLint>(result, pname, state.getProgram() ? state.getProgram()->id() : 0));     break;
      case GL_PACK_ALIGNMENT:                           InsertResult<GLint>(result, pname, state.getPackAlignment()));                              break;
      case GL_PACK_REVERSE_ROW_ORDER_ANGLE:             InsertResult<GLint>(result, pname, state.getPackReverseRowOrder()));                        break;
      case GL_UNPACK_ALIGNMENT:                         InsertResult<GLint>(result, pname, state.getUnpackAlignment()));                            break;
      case GL_GENERATE_MIPMAP_HINT:                     *params = mGenerateMipmapHint;                            break;
      case GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES:      *params = mFragmentShaderDerivativeHint;                  break;
      case GL_ACTIVE_TEXTURE:                           InsertResult<GLint>(result, pname, state.getActiveSampler() + GL_TEXTURE0));      break;
      case GL_STENCIL_FUNC:                             InsertResult<GLint>(result, pname, state.getDepthStencilState().stencilFunc));                      break;
      case GL_STENCIL_REF:                              InsertResult<GLint>(result, pname, state.getStencilRef()));                                    break;
      case GL_STENCIL_VALUE_MASK:                       InsertResult<GLint>(result, pname, clampToInt(state.getDepthStencilState().stencilMask)));          break;
      case GL_STENCIL_BACK_FUNC:                        InsertResult<GLint>(result, pname, state.getDepthStencilState().stencilBackFunc));                  break;
      case GL_STENCIL_BACK_REF:                         InsertResult<GLint>(result, pname, state.getStencilBackRef()));                                break;
      case GL_STENCIL_BACK_VALUE_MASK:                  InsertResult<GLint>(result, pname, state.getDepthStencilState().stencilBackMask));       break;
      case GL_STENCIL_FAIL:                             InsertResult<GLint>(result, pname, state.getDepthStencilState().stencilFail));                      break;
      case GL_STENCIL_PASS_DEPTH_FAIL:                  InsertResult<GLint>(result, pname, state.getDepthStencilState().stencilPassDepthFail));             break;
      case GL_STENCIL_PASS_DEPTH_PASS:                  InsertResult<GLint>(result, pname, state.getDepthStencilState().stencilPassDepthPass));             break;
      case GL_STENCIL_BACK_FAIL:                        InsertResult<GLint>(result, pname, state.getDepthStencilState().stencilBackFail));                   break;
      case GL_STENCIL_BACK_PASS_DEPTH_FAIL:             InsertResult<GLint>(result, pname, state.getDepthStencilState().stencilBackPassDepthFail));         break;
      case GL_STENCIL_BACK_PASS_DEPTH_PASS:             InsertResult<GLint>(result, pname, state.getDepthStencilState().stencilBackPassDepthPass));         break;
      case GL_DEPTH_FUNC:                               InsertResult<GLint>(result, pname, state.getDepthStencilState().depthFunc));                        break;
      case GL_BLEND_SRC_RGB:                            InsertResult<GLint>(result, pname, state.getBlendState().sourceBlendRGB));                          break;
      case GL_BLEND_SRC_ALPHA:                          InsertResult<GLint>(result, pname, state.getBlendState().sourceBlendAlpha));                        break;
      case GL_BLEND_DST_RGB:                            InsertResult<GLint>(result, pname, state.getBlendState().destBlendRGB));                            break;
      case GL_BLEND_DST_ALPHA:                          InsertResult<GLint>(result, pname, state.getBlendState().destBlendAlpha));                          break;
      case GL_BLEND_EQUATION_RGB:                       InsertResult<GLint>(result, pname, state.getBlendState().blendEquationRGB));                        break;
      case GL_BLEND_EQUATION_ALPHA:                     InsertResult<GLint>(result, pname, state.getBlendState().blendEquationAlpha));                      break;
      case GL_STENCIL_WRITEMASK:                        InsertResult<GLint>(result, pname, clampToInt(state.getDepthStencilState().stencilWritemask)));     break;
      case GL_STENCIL_BACK_WRITEMASK:                   InsertResult<GLint>(result, pname, clampToInt(state.getDepthStencilState().stencilBackWritemask))); break;
      case GL_STENCIL_CLEAR_VALUE:                      *params = mStencilClearValue;                             break;
      case GL_IMPLEMENTATION_COLOR_READ_TYPE:           InsertResult<GLint>(result, pname, state.getReadFramebuffer()->getImplementationColorReadType()));  break;
      case GL_IMPLEMENTATION_COLOR_READ_FORMAT:         InsertResult<GLint>(result, pname, state.getReadFramebuffer()->getImplementationColorReadFormat())); break;
      case GL_SAMPLE_BUFFERS:
      case GL_SAMPLES:
        {
            gl::Framebuffer *framebuffer = mDrawFramebuffer;
            if (framebuffer->checkStatus(data) == GL_FRAMEBUFFER_COMPLETE)
            {
                switch (pname)
                {
                  case GL_SAMPLE_BUFFERS:
                    if (framebuffer->getSamples(data) != 0)
                    {
                        *params = 1;
                    }
                    else
                    {
                        *params = 0;
                    }
                    break;
                  case GL_SAMPLES:
                    *params = framebuffer->getSamples(data);
                    break;
                }
            }
            else
            {
                *params = 0;
            }
        }
        break;
      case GL_VIEWPORT:
        {
            const Rectangle &viewport = state.getViewport();
            InsertResult<GLint>(result, pname, viewport.x));
            InsertResult<GLint>(result, pname, viewport.y));
            InsertResult<GLint>(result, pname, viewport.width));
            InsertResult<GLint>(result, pname, viewport.height));
        }
        break;
      case GL_SCISSOR_BOX:
        {
            const Rectangle &scissor = state.getScissor();
            InsertResult<GLint>(result, pname, scissor.x));
            InsertResult<GLint>(result, pname, scissor.y));
            InsertResult<GLint>(result, pname, scissor.width));
            InsertResult<GLint>(result, pname, scissor.height));
        }
        break;
      case GL_CULL_FACE_MODE:                   InsertResult<GLint>(result, pname, clampToInt(state.getRasterizerState().cullMode)));   break;
      case GL_FRONT_FACE:                       InsertResult<GLint>(result, pname, clampToInt(state.getRasterizerState().frontFace)));  break;
      case GL_RED_BITS:
      case GL_GREEN_BITS:
      case GL_BLUE_BITS:
      case GL_ALPHA_BITS:
        {
            gl::Framebuffer *framebuffer = state.getDrawFramebuffer();
            gl::FramebufferAttachment *colorbuffer = framebuffer->getFirstColorbuffer();

            if (colorbuffer)
            {
                switch (pname)
                {
                  case GL_RED_BITS:   InsertResult<GLint>(result, pname, colorbuffer->getRedSize()));      break;
                  case GL_GREEN_BITS: InsertResult<GLint>(result, pname, colorbuffer->getGreenSize()));    break;
                  case GL_BLUE_BITS:  InsertResult<GLint>(result, pname, colorbuffer->getBlueSize()));     break;
                  case GL_ALPHA_BITS: InsertResult<GLint>(result, pname, colorbuffer->getAlphaSize()));    break;
                }
            }
            else
            {
                InsertResult<GLint>(result, pname, 0));
            }
        }
        break;
      case GL_DEPTH_BITS:
        {
            gl::Framebuffer *framebuffer = state.getDrawFramebuffer();
            gl::FramebufferAttachment *depthbuffer = framebuffer->getDepthbuffer();

            if (depthbuffer)
            {
                InsertResult<GLint>(result, pname, depthbuffer->getDepthSize()));
            }
            else
            {
                InsertResult<GLint>(result, pname, 0));
            }
        }
        break;
      case GL_STENCIL_BITS:
        {
            gl::Framebuffer *framebuffer = state.getDrawFramebuffer();
            gl::FramebufferAttachment *stencilbuffer = framebuffer->getStencilbuffer();

            if (stencilbuffer)
            {
                InsertResult<GLint>(result, pname, stencilbuffer->getStencilSize()));
            }
            else
            {
                InsertResult<GLint>(result, pname, 0));
            }
        }
        break;
      case GL_TEXTURE_BINDING_2D:
        InsertResult<GLint>(result, pname, state.getSamplerTextureId(state.getActiveSampler(), GL_TEXTURE_2D)));
        break;
      case GL_TEXTURE_BINDING_CUBE_MAP:
        InsertResult<GLint>(result, pname, state.getSamplerTextureId(state.getActiveSampler(), GL_TEXTURE_CUBE_MAP)));
        break;
      case GL_TEXTURE_BINDING_3D:
        InsertResult<GLint>(result, pname, state.getSamplerTextureId(state.getActiveSampler(), GL_TEXTURE_3D)));
        break;
      case GL_TEXTURE_BINDING_2D_ARRAY:
        InsertResult<GLint>(result, pname, state.getSamplerTextureId(state.getActiveSampler(), GL_TEXTURE_2D_ARRAY)));
        break;
      case GL_UNIFORM_BUFFER_BINDING:
        *params = mGenericUniformBuffer.id();
        break;
      case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
        *params = mGenericTransformFeedbackBuffer.id();
        break;
      case GL_COPY_READ_BUFFER_BINDING:
        *params = mCopyReadBuffer.id();
        break;
      case GL_COPY_WRITE_BUFFER_BINDING:
        *params = mCopyWriteBuffer.id();
        break;
      case GL_PIXEL_PACK_BUFFER_BINDING:
        *params = mPack.pixelBuffer.id();
        break;
      case GL_PIXEL_UNPACK_BUFFER_BINDING:
        *params = mUnpack.pixelBuffer.id();
        break;

    }

    return result;
}

template <typename QueryT>
std::vector<QueryT> QueryStateIndexed(const Context *context, GLenum pname, size_t index)
{
    std::vector<QueryT> result;

    return result;
}

// Explicit template instantiation (how we export template functions in different files)
// The calls below will make templated query functions successfully link with the GL state query types
// The GL state query API types are: bool, int, uint, float, int64

template std::vector<GLint> QueryState<GLint>(const Context *, GLenum);


}
