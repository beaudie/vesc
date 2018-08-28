//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageVk.cpp:
//    Implements the class methods for ImageVk.
//

#include "libANGLE/renderer/vulkan/ImageVk.h"

#include "common/debug.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/Image.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/Texture.h"
#include "libANGLE/renderer/vulkan/RenderbufferVk.h"
#include "libANGLE/renderer/vulkan/TextureVk.h"

namespace rx
{

ImageVk::ImageVk(const egl::ImageState &state) : ImageImpl(state), mRenderTarget(nullptr)
{
}

ImageVk::~ImageVk()
{
}

egl::Error ImageVk::initialize(const egl::Display *display)
{
    return egl::NoError();
}

gl::Error ImageVk::orphan(const gl::Context *context, egl::ImageSibling *sibling)
{
    UNIMPLEMENTED();
    return gl::NoError();
}

vk::ImageHelper &ImageVk::getImage()
{
    if (mState.source != nullptr)
    {
        if (egl::IsTextureTarget(mState.target))
        {
            TextureVk *textureVk = GetImplAs<TextureVk>(GetAs<gl::Texture>(mState.source));
            return textureVk->getImage();
        }
        else if (egl::IsRenderbufferTarget(mState.target))
        {
            RenderbufferVk *renderbufferVk =
                GetImplAs<RenderbufferVk>(GetAs<gl::Renderbuffer>(mState.source));
            return renderbufferVk->getImage();
        }
        else
        {
            UNREACHABLE();
            return *mImage;
        }
    }

    // Orphaned, image is local
    ASSERT(mImage != nullptr);
    return *mImage;
}

vk::ImageView &ImageVk::getImageView()
{
    if (mState.source != nullptr)
    {
        if (egl::IsTextureTarget(mState.target))
        {
            TextureVk *textureVk = GetImplAs<TextureVk>(GetAs<gl::Texture>(mState.source));
            return textureVk->getImageView();
        }
        else if (egl::IsRenderbufferTarget(mState.target))
        {
            RenderbufferVk *renderbufferVk =
                GetImplAs<RenderbufferVk>(GetAs<gl::Renderbuffer>(mState.source));
            return renderbufferVk->getImageView();
        }
        else
        {
            UNREACHABLE();
            return *mView;
        }
    }

    // Orphaned, image is local
    ASSERT(mView != nullptr);
    return *mView;
}

angle::Result ImageVk::getRenderTarget(const gl::Context *context,
                                       FramebufferAttachmentRenderTarget **outRT)
{
    if (mState.source != nullptr)
    {
        ASSERT(!mRenderTarget);
        ANGLE_TRY_HANDLE(context, mState.source->getAttachmentRenderTarget(
                                      context, GL_NONE, mState.imageIndex, outRT));
        return angle::Result::Continue();
    }

    ASSERT(mRenderTarget);
    *outRT = mRenderTarget;
    return angle::Result::Continue();
}

angle::Result ImageVk::ensureImageInitialized(ContextVk *contextVk)
{
    if (mState.source != nullptr)
    {
        if (egl::IsTextureTarget(mState.target))
        {
            TextureVk *textureVk = GetImplAs<TextureVk>(GetAs<gl::Texture>(mState.source));
            ANGLE_TRY(textureVk->ensureImageInitialized(contextVk));
        }
    }

    return angle::Result::Continue();
}

}  // namespace rx
