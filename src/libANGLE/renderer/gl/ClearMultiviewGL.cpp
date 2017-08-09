//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ClearMultiviewGL:
//   A helper for clearing multiview side-by-side and layered framebuffers.
//

#include "libANGLE/renderer/gl/ClearMultiviewGL.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/TextureGL.h"

namespace rx
{

ClearMultiviewGL::ClearMultiviewGL(const FunctionsGL *functions, StateManagerGL *stateManager)
    : mFunctions(functions), mStateManager(stateManager), mFramebuffer(0u)
{
}

ClearMultiviewGL::~ClearMultiviewGL()
{
    if (mFramebuffer != 0u)
    {
        mFunctions->deleteFramebuffers(1, &mFramebuffer);
    }
}

gl::Error ClearMultiviewGL::clearLayeredFBO(const gl::FramebufferState &state,
                                            ClearCommandType clearCommandType,
                                            GLbitfield mask,
                                            GLenum buffer,
                                            GLint drawbuffer,
                                            const uint8_t *values,
                                            GLfloat depth,
                                            GLint stencil)
{
    ANGLE_TRY(initializeResources());

    mStateManager->bindFramebuffer(GL_DRAW_FRAMEBUFFER, mFramebuffer);

    const auto &firstAttachment = state.getFirstNonNullAttachment();
    ASSERT(firstAttachment->getMultiviewLayout() == GL_FRAMEBUFFER_MULTIVIEW_LAYERED_ANGLE);
    int numViews      = firstAttachment->getNumViews();
    int baseViewIndex = firstAttachment->getBaseViewIndex();

    // Clear the dirty attachments.
    for (auto dirtyAttachment : mDirtyAttachments)
    {
        switch (dirtyAttachment)
        {
            case DIRTY_BIT_DEPTH_ATTACHMENT:
                for (int i = 0; i < numViews; ++i)
                {
                    mFunctions->framebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0,
                                                        0, baseViewIndex + i);
                }
                break;
            case DIRTY_BIT_STENCIL_ATTACHMENT:
                for (int i = 0; i < numViews; ++i)
                {
                    mFunctions->framebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                                        0, 0, baseViewIndex + i);
                }
                break;
            case DIRTY_BIT_DEPTH_STENCIL_ATTACHMENT:
                for (int i = 0; i < numViews; ++i)
                {
                    mFunctions->framebufferTextureLayer(
                        GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 0, 0, baseViewIndex + i);
                }
                break;
            default:
                ASSERT(DIRTY_BIT_COLOR_ATTACHMENT_0 == 0 &&
                       dirtyAttachment < DIRTY_BIT_COLOR_ATTACHMENT_MAX);
                for (int i = 0; i < numViews; ++i)
                {
                    mFunctions->framebufferTextureLayer(
                        GL_DRAW_FRAMEBUFFER,
                        static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + dirtyAttachment), 0, 0,
                        baseViewIndex + i);
                }
        }
    }
    mDirtyAttachments.reset();

    const auto &drawBuffers = state.getDrawBufferStates();
    mFunctions->drawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());

    // Attach the new attachments and clear.
    for (int i = 0; i < numViews; ++i)
    {
        attachTextures(state, baseViewIndex + i);
        genericClear(clearCommandType, mask, buffer, drawbuffer, values, depth, stencil);
    }

    return gl::NoError();
}

gl::Error ClearMultiviewGL::clearSideBySideFBO(const gl::FramebufferState &state,
                                               const gl::Rectangle &scissorBase,
                                               ClearCommandType clearCommandType,
                                               GLbitfield mask,
                                               GLenum buffer,
                                               GLint drawbuffer,
                                               const uint8_t *values,
                                               GLfloat depth,
                                               GLint stencil)
{
    const auto &firstAttachment = state.getFirstNonNullAttachment();
    ASSERT(firstAttachment->getMultiviewLayout() == GL_FRAMEBUFFER_MULTIVIEW_SIDE_BY_SIDE_ANGLE);

    const auto &viewportOffsets = firstAttachment->getMultiviewViewportOffsets();
    for (size_t i = 0u; i < viewportOffsets.size(); ++i)
    {
        gl::Rectangle scissor(scissorBase.x + viewportOffsets[i].x,
                              scissorBase.y + viewportOffsets[i].y, scissorBase.width,
                              scissorBase.height);
        mStateManager->setScissorIndexed(0u, scissor);
        genericClear(clearCommandType, mask, buffer, drawbuffer, values, depth, stencil);
    }
    return gl::NoError();
}

void ClearMultiviewGL::genericClear(ClearCommandType clearCommandType,
                                    GLbitfield mask,
                                    GLenum buffer,
                                    GLint drawbuffer,
                                    const uint8_t *values,
                                    GLfloat depth,
                                    GLint stencil)
{
    switch (clearCommandType)
    {
        case ClearCommandType::Clear:
            mFunctions->clear(mask);
            break;
        case ClearCommandType::ClearBufferfv:
            mFunctions->clearBufferfv(buffer, drawbuffer,
                                      reinterpret_cast<const GLfloat *>(values));
            break;
        case ClearCommandType::ClearBufferuiv:
            mFunctions->clearBufferuiv(buffer, drawbuffer,
                                       reinterpret_cast<const GLuint *>(values));
            break;
        case ClearCommandType::ClearBufferiv:
            mFunctions->clearBufferiv(buffer, drawbuffer, reinterpret_cast<const GLint *>(values));
            break;
        case ClearCommandType::ClearBufferfi:
            mFunctions->clearBufferfi(buffer, drawbuffer, depth, stencil);
            break;
        default:
            UNREACHABLE();
    }
}

void ClearMultiviewGL::attachTextures(const gl::FramebufferState &state, int layer)
{
    for (auto drawBufferId : state.getEnabledDrawBuffers())
    {
        const auto &attachment = state.getColorAttachment(drawBufferId);
        if (attachment == nullptr)
        {
            continue;
        }

        const auto &imageIndex = attachment->getTextureImageIndex();
        ASSERT(imageIndex.type == GL_TEXTURE_2D_ARRAY);

        GLenum colorAttachment =
            static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + static_cast<int>(drawBufferId));
        const TextureGL *textureGL = GetImplAs<TextureGL>(attachment->getTexture());
        mFunctions->framebufferTextureLayer(GL_DRAW_FRAMEBUFFER, colorAttachment,
                                            textureGL->getTextureID(), imageIndex.mipIndex, layer);

        mDirtyAttachments.set(DIRTY_BIT_COLOR_ATTACHMENT_0 + drawBufferId);
    }

    const auto &depthStencilAttachment = state.getDepthStencilAttachment();
    const auto &depthAttachment        = state.getDepthAttachment();
    const auto &stencilAttachment      = state.getStencilAttachment();
    if (depthStencilAttachment != nullptr)
    {
        const auto &imageIndex = depthStencilAttachment->getTextureImageIndex();
        ASSERT(imageIndex.type == GL_TEXTURE_2D_ARRAY);

        const TextureGL *textureGL = GetImplAs<TextureGL>(depthStencilAttachment->getTexture());
        mFunctions->framebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                            textureGL->getTextureID(), imageIndex.mipIndex, layer);
        mDirtyAttachments.set(DIRTY_BIT_DEPTH_STENCIL_ATTACHMENT);
    }
    else if (depthAttachment != nullptr)
    {
        const auto &imageIndex = depthAttachment->getTextureImageIndex();
        ASSERT(imageIndex.type == GL_TEXTURE_2D_ARRAY);

        const TextureGL *textureGL = GetImplAs<TextureGL>(depthAttachment->getTexture());
        mFunctions->framebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                            textureGL->getTextureID(), imageIndex.mipIndex, layer);
        mDirtyAttachments.set(DIRTY_BIT_DEPTH_ATTACHMENT);
    }
    else if (stencilAttachment != nullptr)
    {
        const auto &imageIndex = stencilAttachment->getTextureImageIndex();
        ASSERT(imageIndex.type == GL_TEXTURE_2D_ARRAY);

        const TextureGL *textureGL = GetImplAs<TextureGL>(stencilAttachment->getTexture());
        mFunctions->framebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                            textureGL->getTextureID(), imageIndex.mipIndex, layer);
        mDirtyAttachments.set(DIRTY_BIT_STENCIL_ATTACHMENT);
    }
}

gl::Error ClearMultiviewGL::initializeResources()
{
    if (mFramebuffer == 0u)
    {
        mFunctions->genFramebuffers(1, &mFramebuffer);
    }
    return gl::NoError();
}

}  // namespace rx