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
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/RenderbufferVk.h"
#include "libANGLE/renderer/vulkan/TextureVk.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

namespace rx
{

ImageVk::ImageVk(const egl::ImageState &state)
    : ImageImpl(state), mBaseMipLevel(0), mOwnsImage(false), mImage(nullptr)
{}

ImageVk::~ImageVk() {}

void ImageVk::onDestroy(const egl::Display *display)
{
    DisplayVk *displayVk = vk::GetImpl(display);
    RendererVk *renderer = displayVk->getRenderer();

    if (mImage != nullptr && mOwnsImage)
    {
        mImage->release(renderer, true);
        delete mImage;
    }
    mImage = nullptr;
}

egl::Error ImageVk::initialize(const egl::Display *display)
{
    DisplayVk *displayVk = vk::GetImpl(display);
    RendererVk *renderer = displayVk->getRenderer();

    if (egl::IsTextureTarget(mState.target))
    {
        TextureVk *textureVk = GetImplAs<TextureVk>(GetAs<gl::Texture>(mState.source));

        // Make sure the texture has created its backing storage
        ANGLE_TRY(ResultToEGL(textureVk->ensureImageInitialized(displayVk)));

        mImage = &textureVk->getImage();

        // The staging buffer for a texture source should already be initialized

        mOwnsImage = false;

        mBaseMipLevel = mState.imageIndex.getLevelIndex();
    }
    else if (egl::IsRenderbufferTarget(mState.target))
    {
        RenderbufferVk *renderbufferVk =
            GetImplAs<RenderbufferVk>(GetAs<gl::Renderbuffer>(mState.source));
        mImage = renderbufferVk->getImage();

        // Make sure a staging buffer is ready to use to upload data
        mImage->initStagingBuffer(renderer);

        mOwnsImage = false;

        mBaseMipLevel = 0;
    }
    else
    {
        UNREACHABLE();
        return egl::EglBadAccess();
    }

    return egl::NoError();
}

angle::Result ImageVk::orphan(const gl::Context *context, egl::ImageSibling *sibling)
{
    if (sibling == mState.source)
    {
        if (egl::IsTextureTarget(mState.target))
        {
            TextureVk *textureVk = GetImplAs<TextureVk>(GetAs<gl::Texture>(mState.source));
            ASSERT(mImage == &textureVk->getImage());
            textureVk->releaseOwnershipOfImage();
            mOwnsImage = true;
        }
        else if (egl::IsRenderbufferTarget(mState.target))
        {
            RenderbufferVk *renderbufferVk =
                GetImplAs<RenderbufferVk>(GetAs<gl::Renderbuffer>(mState.source));
            ASSERT(mImage == renderbufferVk->getImage());
            renderbufferVk->releaseOwnershipOfImage();
            mOwnsImage = true;
        }
        else
        {
            ANGLE_VK_UNREACHABLE(vk::GetImpl(context));
            return angle::Result::Stop;
        }
    }

    return angle::Result::Continue;
}

uint32_t ImageVk::getImageBaseMipLevel() const
{
    return mBaseMipLevel;
}

vk::ImageHelper *ImageVk::getImage() const
{
    return mImage;
}

}  // namespace rx
