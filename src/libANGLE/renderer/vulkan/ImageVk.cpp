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

ImageVk::ImageVk(const egl::ImageState &state, const gl::Context *context)
    : ImageImpl(state), mImageLevel(0), mContext(context)
{}

ImageVk::~ImageVk() {}

void ImageVk::onDestroy(const egl::Display *display)
{
    DisplayVk *displayVk = vk::GetImpl(display);
    RendererVk *renderer = displayVk->getRenderer();

    std::vector<vk::GarbageObjectBase> garbage;

    if (mImage.valid())
    {
        mImage.reset(displayVk, &garbage);
    }

    if (egl::IsExternalImageTarget(mState.target))
    {
        ExternalImageSiblingVk *externalImageSibling = getExternalSource();
        externalImageSibling->release(displayVk, &garbage);
    }

    if (!garbage.empty())
    {
        renderer->addGarbage(std::move(mImageLastUseFences), std::move(garbage));
    }
    else
    {
        for (vk::Shared<vk::Fence> &fence : mImageLastUseFences)
        {
            fence.reset(displayVk->getDevice());
        }
    }
}

egl::Error ImageVk::initialize(const egl::Display *display)
{
    if (egl::IsTextureTarget(mState.target))
    {
        TextureVk *textureVk = GetImplAs<TextureVk>(GetAs<gl::Texture>(mState.source));

        // Make sure the texture has created its backing storage
        ASSERT(mContext != nullptr);
        ContextVk *contextVk = vk::GetImpl(mContext);
        ANGLE_TRY(ResultToEGL(textureVk->ensureImageInitialized(contextVk)));

        mImage.copy(textureVk->getSharedImage(), contextVk);

        // The staging buffer for a texture source should already be initialized

        mImageTextureType = mState.imageIndex.getType();
        mImageLevel       = mState.imageIndex.getLevelIndex();
        mImageLayer       = mState.imageIndex.hasLayer() ? mState.imageIndex.getLayerIndex() : 0;
    }
    else
    {
        RendererVk *renderer = nullptr;
        if (egl::IsRenderbufferTarget(mState.target))
        {
            ContextVk *contextVk = vk::GetImpl(mContext);

            RenderbufferVk *renderbufferVk =
                GetImplAs<RenderbufferVk>(GetAs<gl::Renderbuffer>(mState.source));
            mImage.copy(renderbufferVk->getSharedImage(), contextVk);

            ASSERT(mContext != nullptr);
            renderer = contextVk->getRenderer();
        }
        else if (egl::IsExternalImageTarget(mState.target))
        {
            DisplayVk *displayVk = vk::GetImpl(display);

            const ExternalImageSiblingVk *externalImageSibling = getExternalSource();
            mImage.copy(*externalImageSibling->getImage(), displayVk->getDevice());

            ASSERT(mContext == nullptr);
            renderer = displayVk->getRenderer();
        }
        else
        {
            UNREACHABLE();
            return egl::EglBadAccess();
        }

        // Make sure a staging buffer is ready to use to upload data
        mImage.get().initStagingBuffer(renderer, mImage.get().getFormat(), vk::kStagingBufferFlags,
                                       vk::kStagingBufferSize);

        mImageTextureType = gl::TextureType::_2D;
        mImageLevel       = 0;
        mImageLayer       = 0;
    }

    // mContext is no longer needed, make sure it's not used by accident.
    mContext = nullptr;

    return egl::NoError();
}

angle::Result ImageVk::orphan(const gl::Context *context, egl::ImageSibling *sibling)
{
    // Grab a fence from the releasing context to know when the image is no longer used
    ASSERT(context != nullptr);
    ContextVk *contextVk = vk::GetImpl(context);

    if (sibling == mState.source)
    {
        if (egl::IsTextureTarget(mState.target))
        {
            TextureVk *textureVk = GetImplAs<TextureVk>(GetAs<gl::Texture>(mState.source));
            ASSERT(&mImage.get() == &textureVk->getImage());
            textureVk->releaseImage(contextVk);
        }
        else if (egl::IsRenderbufferTarget(mState.target))
        {
            RenderbufferVk *renderbufferVk =
                GetImplAs<RenderbufferVk>(GetAs<gl::Renderbuffer>(mState.source));
            ASSERT(&mImage.get() == &renderbufferVk->getImage());
            renderbufferVk->releaseImage(contextVk);
        }
        else
        {
            ANGLE_VK_UNREACHABLE(vk::GetImpl(context));
            return angle::Result::Stop;
        }
    }

    // Flush the context to make sure the fence has been submitted.
    ANGLE_TRY(contextVk->flushImpl(nullptr));

    vk::Shared<vk::Fence> fence = contextVk->getLastSubmittedFence();
    if (fence.isReferenced())
    {
        mImageLastUseFences.push_back(std::move(fence));
    }

    return angle::Result::Continue;
}

ExternalImageSiblingVk *ImageVk::getExternalSource() const
{
    ASSERT(egl::IsExternalImageTarget(mState.target));
    return GetImplAs<ExternalImageSiblingVk>(GetAs<egl::ExternalImageSibling>(mState.source));
}
}  // namespace rx
