#include "libANGLE/renderer/gl/ClearMultiviewGL.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/TextureGL.h"

namespace rx
{

ClearMultiviewGL::ClearMultiviewGL(const FunctionsGL *functions,
                                   const WorkaroundsGL &workarounds,
                                   StateManagerGL *stateManager)
    : mFunctions(functions),
      mWorkarounds(workarounds),
      mStateManager(stateManager),
      mFramebuffer(0u)
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
    int numViews = firstAttachment->getNumViews();
    int offset   = firstAttachment->getBaseViewIndex();

    for (int i = 0; i < numViews; ++i)
    {
        attachTextures(state, i + offset);
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
    const auto &drawBufferMask = state.getEnabledDrawBuffers();
    for (auto drawBufferId : drawBufferMask)
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
    }

    const auto &dirtyDrawBuffers = (mPrevDrawBufferMask & ~drawBufferMask);
    for (auto drawBufferId : dirtyDrawBuffers)
    {
        const auto &attachment = state.getColorAttachment(drawBufferId);
        if (attachment == nullptr)
        {
            continue;
        }
        GLenum colorAttachment =
            static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + static_cast<int>(drawBufferId));
        mFunctions->framebufferTextureLayer(GL_DRAW_FRAMEBUFFER, colorAttachment, 0, 0, layer);
    }

    const auto &depthAttachment = state.getDepthAttachment();
    if (depthAttachment != nullptr)
    {
        const auto &imageIndex = depthAttachment->getTextureImageIndex();
        ASSERT(imageIndex.type == GL_TEXTURE_2D_ARRAY);

        const TextureGL *textureGL = GetImplAs<TextureGL>(depthAttachment->getTexture());
        mFunctions->framebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                            textureGL->getTextureID(), imageIndex.mipIndex, layer);
    }
    else
    {
        mFunctions->framebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0, layer);
    }

    const auto &stencilAttachment = state.getStencilAttachment();
    if (stencilAttachment != nullptr)
    {
        const auto &imageIndex = stencilAttachment->getTextureImageIndex();
        ASSERT(imageIndex.type == GL_TEXTURE_2D_ARRAY);

        const TextureGL *textureGL = GetImplAs<TextureGL>(stencilAttachment->getTexture());
        mFunctions->framebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                            textureGL->getTextureID(), imageIndex.mipIndex, layer);
    }
    else
    {
        mFunctions->framebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 0, 0,
                                            layer);
    }

    const auto &depthStencilAttachment = state.getDepthStencilAttachment();
    if (depthStencilAttachment != nullptr)
    {
        const auto &imageIndex = depthStencilAttachment->getTextureImageIndex();
        ASSERT(imageIndex.type == GL_TEXTURE_2D_ARRAY);

        const TextureGL *textureGL = GetImplAs<TextureGL>(depthStencilAttachment->getTexture());
        mFunctions->framebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                            textureGL->getTextureID(), imageIndex.mipIndex, layer);
    }
    else
    {
        mFunctions->framebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 0, 0,
                                            layer);
    }

    mPrevDrawBufferMask = drawBufferMask;
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