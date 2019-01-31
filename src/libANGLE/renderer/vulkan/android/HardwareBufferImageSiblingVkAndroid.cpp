//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// HardwareBufferImageSiblingVkAndroid.cpp: Implements HardwareBufferImageSiblingVkAndroid.

#include "libANGLE/renderer/vulkan/android/HardwareBufferImageSiblingVkAndroid.h"

#include "common/android_util.h"

#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"

#include <android/hardware_buffer.h>
#include <android/log.h>

namespace rx
{
HardwareBufferImageSiblingVkAndroid::HardwareBufferImageSiblingVkAndroid(EGLClientBuffer buffer)
    : mBuffer(buffer), mFormat(GL_NONE), mImage(nullptr)
{}

HardwareBufferImageSiblingVkAndroid::~HardwareBufferImageSiblingVkAndroid() {}

egl::Error HardwareBufferImageSiblingVkAndroid::initialize(const egl::Display *display)
{
    DisplayVk *displayVk = vk::GetImpl(display);
    return angle::ToEGL(initImpl(displayVk), displayVk, EGL_BAD_PARAMETER);
}

angle::Result HardwareBufferImageSiblingVkAndroid::initImpl(DisplayVk *displayVk)
{
    __android_log_print(ANDROID_LOG_ERROR, "ANGLE",
                        "HardwareBufferImageSiblingVkAndroid::initialize");

    RendererVk *renderer = displayVk->getRenderer();

    struct ANativeWindowBuffer *windowBuffer =
        reinterpret_cast<struct ANativeWindowBuffer *>(mBuffer);

    int pixelFormat = 0;
    angle::android::GetANativeWindowBufferProperties(windowBuffer, &mSize.width, &mSize.height,
                                                     &mSize.depth, &pixelFormat);
    GLenum internalFormat = angle::android::NativePixelFormatToGLInternalFormat(pixelFormat);
    mFormat               = gl::Format(internalFormat);

    __android_log_print(ANDROID_LOG_ERROR, "ANGLE",
                        "HardwareBufferImageSiblingVkAndroid: %u x %u, format: 0x%X", mSize.width,
                        mSize.height, internalFormat);

    // The EGL_ANDROID_image_native_buffer spec says that the buffer is a ANativeWindowBuffer. This
    // is the same type as AHardwareBuffer in android currently but may change in the future. If
    // this happens, we will need to request new APIs to convert between them.
    struct AHardwareBuffer *hardwareBuffer =
        angle::android::ANativeWindowBufferToAHardwareBuffer(windowBuffer);

    VkAndroidHardwareBufferPropertiesANDROID bufferProperties = {};
    bufferProperties.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;

    VkDevice device = renderer->getDevice();
    ANGLE_VK_TRY(displayVk, vkGetAndroidHardwareBufferPropertiesANDROID(device, hardwareBuffer,
                                                                        &bufferProperties));

    __android_log_print(ANDROID_LOG_ERROR, "ANGLE",
                        "HardwareBufferImageSiblingVkAndroid: size: %lu",
                        bufferProperties.allocationSize);

    const vk::Format &vkFormat = renderer->getFormat(internalFormat);

    const angle::Format &textureFormat = vkFormat.textureFormat();

    __android_log_print(
        ANDROID_LOG_ERROR, "ANGLE",
        "HardwareBufferImageSiblingVkAndroid: vk texture format: GLenum: 0x%X vkFormat: 0x%X",
        textureFormat.glInternalFormat, vkFormat.vkTextureFormat);

    bool isDepthOrStencilFormat = textureFormat.depthBits > 0 || textureFormat.stencilBits > 0;
    const VkImageUsageFlags usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT |
        (textureFormat.redBits > 0 ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0) |
        (isDepthOrStencilFormat ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : 0);

    mImage = new vk::ImageHelper();
    ANGLE_TRY(mImage->init(displayVk, gl::TextureType::_2D, mSize, vkFormat, 1, usage,
                           vk::ImageLayout::PreInitialized, 1, 1));

    __android_log_print(ANDROID_LOG_ERROR, "ANGLE",
                        "HardwareBufferImageSiblingVkAndroid: initialized image.");

    VkImportAndroidHardwareBufferInfoANDROID importHardwareBufferInfo = {};
    importHardwareBufferInfo.sType  = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
    importHardwareBufferInfo.buffer = hardwareBuffer;

    VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    ANGLE_TRY(mImage->initExternalMemory(displayVk, renderer->getMemoryProperties(),
                                         &importHardwareBufferInfo, VK_QUEUE_FAMILY_FOREIGN_EXT,
                                         flags));

    __android_log_print(ANDROID_LOG_ERROR, "ANGLE",
                        "HardwareBufferImageSiblingVkAndroid: initialized image memory.");

    // Make sure a staging buffer is ready to use to upload data
    mImage->initStagingBuffer(renderer);

    return angle::Result::Continue;
}

void HardwareBufferImageSiblingVkAndroid::onDestroy(const egl::Display *display)
{
    DisplayVk *displayVk = vk::GetImpl(display);
    RendererVk *renderer = displayVk->getRenderer();

    if (mImage != nullptr)
    {
        mImage->releaseImage(renderer);
        mImage->releaseStagingBuffer(renderer);
        SafeDelete(mImage);
    }
}

gl::Format HardwareBufferImageSiblingVkAndroid::getFormat() const
{
    return mFormat;
}

bool HardwareBufferImageSiblingVkAndroid::isRenderable(const gl::Context *context) const
{
    return true;
}

bool HardwareBufferImageSiblingVkAndroid::isTexturable(const gl::Context *context) const
{
    return true;
}

gl::Extents HardwareBufferImageSiblingVkAndroid::getSize() const
{
    return mSize;
}

size_t HardwareBufferImageSiblingVkAndroid::getSamples() const
{
    return 0;
}

// ExternalImageSiblingVk interface
vk::ImageHelper *HardwareBufferImageSiblingVkAndroid::getImage() const
{
    return mImage;
}
}  // namespace rx
