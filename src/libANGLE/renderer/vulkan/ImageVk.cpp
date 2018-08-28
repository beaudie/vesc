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
    return gl::InternalError();
}

const vk::ImageHelper &ImageVk::getImage() const
{
    if (mState.source != nullptr)
    {
        if (egl::IsTextureTarget(mState.target))
        {
            const TextureVk *textureVk = GetImplAs<TextureVk>(GetAs<gl::Texture>(mState.source));
            return textureVk->getImage();
        }
        else if (egl::IsRenderbufferTarget(mState.target))
        {
            const RenderbufferVk *renderbufferVk =
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

angle::Result ImageVk::getRenderTarget(const gl::Context *context, RenderTargetVk **outRT)
{
    if (mState.source != nullptr)
    {
        ASSERT(!mRenderTarget);
        FramebufferAttachmentRenderTarget *rt = nullptr;
        ANGLE_TRY_HANDLE(context, mState.source->getAttachmentRenderTarget(context, GL_NONE,
                                                                           mState.imageIndex, &rt));
        *outRT = GetAs<RenderTargetVk>(rt);
        return angle::Result::Continue();
    }

    ASSERT(mRenderTarget);
    *outRT = mRenderTarget;
    return angle::Result::Continue();
}

}  // namespace rx
