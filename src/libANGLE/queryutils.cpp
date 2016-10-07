//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// queryutils.cpp: Utilities for querying values from GL objects

#include "libANGLE/queryutils.h"

#include "libANGLE/Buffer.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/Program.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/Shader.h"
#include "libANGLE/Texture.h"

namespace gl
{

namespace
{
template <typename QueryType>
void QueryTexParameterBase(const Texture *texture, GLenum pname, QueryType *params)
{
    ASSERT(texture != nullptr);

    switch (pname)
    {
        case GL_TEXTURE_MAG_FILTER:
            *params = static_cast<QueryType>(texture->getMagFilter());
            break;
        case GL_TEXTURE_MIN_FILTER:
            *params = static_cast<QueryType>(texture->getMinFilter());
            break;
        case GL_TEXTURE_WRAP_S:
            *params = static_cast<QueryType>(texture->getWrapS());
            break;
        case GL_TEXTURE_WRAP_T:
            *params = static_cast<QueryType>(texture->getWrapT());
            break;
        case GL_TEXTURE_WRAP_R:
            *params = static_cast<QueryType>(texture->getWrapR());
            break;
        case GL_TEXTURE_IMMUTABLE_FORMAT:
            *params = static_cast<QueryType>(texture->getImmutableFormat() ? GL_TRUE : GL_FALSE);
            break;
        case GL_TEXTURE_IMMUTABLE_LEVELS:
            *params = static_cast<QueryType>(texture->getImmutableLevels());
            break;
        case GL_TEXTURE_USAGE_ANGLE:
            *params = static_cast<QueryType>(texture->getUsage());
            break;
        case GL_TEXTURE_MAX_ANISOTROPY_EXT:
            *params = static_cast<QueryType>(texture->getMaxAnisotropy());
            break;
        case GL_TEXTURE_SWIZZLE_R:
            *params = static_cast<QueryType>(texture->getSwizzleRed());
            break;
        case GL_TEXTURE_SWIZZLE_G:
            *params = static_cast<QueryType>(texture->getSwizzleGreen());
            break;
        case GL_TEXTURE_SWIZZLE_B:
            *params = static_cast<QueryType>(texture->getSwizzleBlue());
            break;
        case GL_TEXTURE_SWIZZLE_A:
            *params = static_cast<QueryType>(texture->getSwizzleAlpha());
            break;
        case GL_TEXTURE_BASE_LEVEL:
            *params = static_cast<QueryType>(texture->getBaseLevel());
            break;
        case GL_TEXTURE_MAX_LEVEL:
            *params = static_cast<QueryType>(texture->getMaxLevel());
            break;
        case GL_TEXTURE_MIN_LOD:
            *params = static_cast<QueryType>(texture->getSamplerState().minLod);
            break;
        case GL_TEXTURE_MAX_LOD:
            *params = static_cast<QueryType>(texture->getSamplerState().maxLod);
            break;
        case GL_TEXTURE_COMPARE_MODE:
            *params = static_cast<QueryType>(texture->getCompareMode());
            break;
        case GL_TEXTURE_COMPARE_FUNC:
            *params = static_cast<QueryType>(texture->getCompareFunc());
            break;
        default:
            UNREACHABLE();
            break;
    }
}

}  // anonymous namespace

void QueryFramebufferAttachmentParameteriv(const Framebuffer *framebuffer,
                                           GLenum attachment,
                                           GLenum pname,
                                           GLint *params)
{
    ASSERT(framebuffer);

    const FramebufferAttachment *attachmentObject = framebuffer->getAttachment(attachment);
    if (attachmentObject == nullptr)
    {
        // ES 2.0.25 spec pg 127 states that if the value of FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE
        // is NONE, then querying any other pname will generate INVALID_ENUM.

        // ES 3.0.2 spec pg 235 states that if the attachment type is none,
        // GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME will return zero and be an
        // INVALID_OPERATION for all other pnames

        switch (pname)
        {
            case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
                *params = GL_NONE;
                break;

            case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
                *params = 0;
                break;

            default:
                UNREACHABLE();
                break;
        }

        return;
    }

    switch (pname)
    {
        case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
            *params = attachmentObject->type();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
            *params = attachmentObject->id();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
            *params = attachmentObject->mipLevel();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
            *params = attachmentObject->cubeMapFace();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE:
            *params = attachmentObject->getRedSize();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE:
            *params = attachmentObject->getGreenSize();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE:
            *params = attachmentObject->getBlueSize();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE:
            *params = attachmentObject->getAlphaSize();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE:
            *params = attachmentObject->getDepthSize();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE:
            *params = attachmentObject->getStencilSize();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE:
            *params = attachmentObject->getComponentType();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING:
            *params = attachmentObject->getColorEncoding();
            break;

        case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER:
            *params = attachmentObject->layer();
            break;

        default:
            UNREACHABLE();
            break;
    }
}

void QueryBufferParameteriv(const Buffer *buffer, GLenum pname, GLint *params)
{
    ASSERT(buffer != nullptr);

    switch (pname)
    {
        case GL_BUFFER_USAGE:
            *params = static_cast<GLint>(buffer->getUsage());
            break;
        case GL_BUFFER_SIZE:
            *params = clampCast<GLint>(buffer->getSize());
            break;
        case GL_BUFFER_ACCESS_FLAGS:
            *params = buffer->getAccessFlags();
            break;
        case GL_BUFFER_ACCESS_OES:
            *params = buffer->getAccess();
            break;
        case GL_BUFFER_MAPPED:
            *params = static_cast<GLint>(buffer->isMapped());
            break;
        case GL_BUFFER_MAP_OFFSET:
            *params = clampCast<GLint>(buffer->getMapOffset());
            break;
        case GL_BUFFER_MAP_LENGTH:
            *params = clampCast<GLint>(buffer->getMapLength());
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void QueryProgramiv(const Program *program, GLenum pname, GLint *params)
{
    ASSERT(program != nullptr);

    switch (pname)
    {
        case GL_DELETE_STATUS:
            *params = program->isFlaggedForDeletion();
            return;
        case GL_LINK_STATUS:
            *params = program->isLinked();
            return;
        case GL_VALIDATE_STATUS:
            *params = program->isValidated();
            return;
        case GL_INFO_LOG_LENGTH:
            *params = program->getInfoLogLength();
            return;
        case GL_ATTACHED_SHADERS:
            *params = program->getAttachedShadersCount();
            return;
        case GL_ACTIVE_ATTRIBUTES:
            *params = program->getActiveAttributeCount();
            return;
        case GL_ACTIVE_ATTRIBUTE_MAX_LENGTH:
            *params = program->getActiveAttributeMaxLength();
            return;
        case GL_ACTIVE_UNIFORMS:
            *params = program->getActiveUniformCount();
            return;
        case GL_ACTIVE_UNIFORM_MAX_LENGTH:
            *params = program->getActiveUniformMaxLength();
            return;
        case GL_PROGRAM_BINARY_LENGTH_OES:
            *params = program->getBinaryLength();
            return;
        case GL_ACTIVE_UNIFORM_BLOCKS:
            *params = program->getActiveUniformBlockCount();
            return;
        case GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH:
            *params = program->getActiveUniformBlockMaxLength();
            break;
        case GL_TRANSFORM_FEEDBACK_BUFFER_MODE:
            *params = program->getTransformFeedbackBufferMode();
            break;
        case GL_TRANSFORM_FEEDBACK_VARYINGS:
            *params = program->getTransformFeedbackVaryingCount();
            break;
        case GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH:
            *params = program->getTransformFeedbackVaryingMaxLength();
            break;
        case GL_PROGRAM_BINARY_RETRIEVABLE_HINT:
            *params = program->getBinaryRetrievableHint();
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void QueryRenderbufferiv(const Renderbuffer *renderbuffer, GLenum pname, GLint *params)
{
    ASSERT(renderbuffer != nullptr);

    switch (pname)
    {
        case GL_RENDERBUFFER_WIDTH:
            *params = renderbuffer->getWidth();
            break;
        case GL_RENDERBUFFER_HEIGHT:
            *params = renderbuffer->getHeight();
            break;
        case GL_RENDERBUFFER_INTERNAL_FORMAT:
            *params = renderbuffer->getFormat().info->internalFormat;
            break;
        case GL_RENDERBUFFER_RED_SIZE:
            *params = renderbuffer->getRedSize();
            break;
        case GL_RENDERBUFFER_GREEN_SIZE:
            *params = renderbuffer->getGreenSize();
            break;
        case GL_RENDERBUFFER_BLUE_SIZE:
            *params = renderbuffer->getBlueSize();
            break;
        case GL_RENDERBUFFER_ALPHA_SIZE:
            *params = renderbuffer->getAlphaSize();
            break;
        case GL_RENDERBUFFER_DEPTH_SIZE:
            *params = renderbuffer->getDepthSize();
            break;
        case GL_RENDERBUFFER_STENCIL_SIZE:
            *params = renderbuffer->getStencilSize();
            break;
        case GL_RENDERBUFFER_SAMPLES_ANGLE:
            *params = renderbuffer->getSamples();
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void QueryShaderiv(const Shader *shader, GLenum pname, GLint *params)
{
    ASSERT(shader != nullptr);

    switch (pname)
    {
        case GL_SHADER_TYPE:
            *params = shader->getType();
            return;
        case GL_DELETE_STATUS:
            *params = shader->isFlaggedForDeletion();
            return;
        case GL_COMPILE_STATUS:
            *params = shader->isCompiled() ? GL_TRUE : GL_FALSE;
            return;
        case GL_INFO_LOG_LENGTH:
            *params = shader->getInfoLogLength();
            return;
        case GL_SHADER_SOURCE_LENGTH:
            *params = shader->getSourceLength();
            return;
        case GL_TRANSLATED_SHADER_SOURCE_LENGTH_ANGLE:
            *params = shader->getTranslatedSourceWithDebugInfoLength();
            return;
        default:
            UNREACHABLE();
            break;
    }
}

void QueryTexParameterfv(const Texture *texture, GLenum pname, GLfloat *params)
{
    QueryTexParameterBase(texture, pname, params);
}

void QueryTexParameteriv(const Texture *texture, GLenum pname, GLint *params)
{
    QueryTexParameterBase(texture, pname, params);
}
}
