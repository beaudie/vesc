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

namespace rx
{

HardwareBufferImageSiblingVkAndroid::HardwareBufferImageSiblingVkAndroid(EGLClientBuffer buffer)
    : mBuffer(buffer), mFormat(GL_NONE), mImage(nullptr)
{}

HardwareBufferImageSiblingVkAndroid::~HardwareBufferImageSiblingVkAndroid() {}

// Static
egl::Error HardwareBufferImageSiblingVkAndroid::ValidateHardwareBuffer(RendererVk *renderer,
                                                                       EGLClientBuffer buffer)
{
    struct ANativeWindowBuffer *windowBuffer = ClientBufferToANativeWindowBuffer(buffer);
    struct AHardwareBuffer *hardwareBuffer =
        angle::android::ANativeWindowBufferToAHardwareBuffer(windowBuffer);

    VkAndroidHardwareBufferFormatPropertiesANDROID bufferFormatProperties;
    bufferFormatProperties.sType =
        VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    bufferFormatProperties.pNext = nullptr;

    VkAndroidHardwareBufferPropertiesANDROID bufferProperties = {};
    bufferProperties.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    bufferProperties.pNext = &bufferFormatProperties;

    VkDevice device = renderer->getDevice();
    VkResult result =
        vkGetAndroidHardwareBufferPropertiesANDROID(device, hardwareBuffer, &bufferProperties);
    if (result != VK_SUCCESS)
    {
        return egl::EglBadParameter() << "Failed to query AHardwareBuffer properties";
    }

    if (!HasFullTextureFormatSupport(renderer, bufferFormatProperties.format))
    {
        return egl::EglBadParameter()
               << "AHardwareBuffer format does not support enough features to use as a texture.";
    }

    return egl::NoError();
}

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

    struct ANativeWindowBuffer *windowBuffer = ClientBufferToANativeWindowBuffer(mBuffer);

    int pixelFormat = 0;
    angle::android::GetANativeWindowBufferProperties(windowBuffer, &mSize.width, &mSize.height,
                                                     &mSize.depth, &pixelFormat);
    GLenum internalFormat = angle::android::NativePixelFormatToGLInternalFormat(pixelFormat);
    mFormat               = gl::Format(internalFormat);

    struct AHardwareBuffer *hardwareBuffer =
        angle::android::ANativeWindowBufferToAHardwareBuffer(windowBuffer);

    VkAndroidHardwareBufferFormatPropertiesANDROID bufferFormatProperties;
    bufferFormatProperties.sType =
        VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    bufferFormatProperties.pNext = nullptr;

    VkAndroidHardwareBufferPropertiesANDROID bufferProperties = {};
    bufferProperties.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    bufferProperties.pNext = &bufferFormatProperties;

    VkDevice device = renderer->getDevice();
    ANGLE_VK_TRY(displayVk, vkGetAndroidHardwareBufferPropertiesANDROID(device, hardwareBuffer,
                                                                        &bufferProperties));

    const vk::Format &vkFormat         = renderer->getFormat(internalFormat);
    const angle::Format &textureFormat = vkFormat.textureFormat();
    bool isDepthOrStencilFormat = textureFormat.depthBits > 0 || textureFormat.stencilBits > 0;
    const VkImageUsageFlags usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT |
        (textureFormat.redBits > 0 ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0) |
        (isDepthOrStencilFormat ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : 0);

    VkExternalFormatANDROID externalFormat = {};
    externalFormat.sType                   = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    externalFormat.externalFormat          = 0;

    if (bufferFormatProperties.format == VK_FORMAT_UNDEFINED)
    {
        externalFormat.externalFormat = bufferFormatProperties.externalFormat;
    }

    VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {};
    externalMemoryImageCreateInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    externalMemoryImageCreateInfo.pNext = &externalFormat;
    externalMemoryImageCreateInfo.handleTypes =
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    mImage = new vk::ImageHelper();
    ANGLE_TRY(mImage->initExternal(displayVk, gl::TextureType::_2D, mSize, vkFormat, 1, usage,
                                   vk::ImageLayout::PreInitialized, &externalMemoryImageCreateInfo,
                                   1, 1));

    VkImportAndroidHardwareBufferInfoANDROID importHardwareBufferInfo = {};
    importHardwareBufferInfo.sType  = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
    importHardwareBufferInfo.buffer = hardwareBuffer;

    VkMemoryDedicatedAllocateInfo dedicatedAllocInfo = {};
    dedicatedAllocInfo.sType  = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    dedicatedAllocInfo.pNext  = &importHardwareBufferInfo;
    dedicatedAllocInfo.image  = mImage->getImage().getHandle();
    dedicatedAllocInfo.buffer = VK_NULL_HANDLE;

    VkMemoryRequirements externalMemoryRequirements = {};
    externalMemoryRequirements.size                 = bufferProperties.allocationSize;
    externalMemoryRequirements.alignment            = 0;
    externalMemoryRequirements.memoryTypeBits       = bufferProperties.memoryTypeBits;

    VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    ANGLE_TRY(mImage->initExternalMemory(displayVk, renderer->getMemoryProperties(),
                                         externalMemoryRequirements, &dedicatedAllocInfo,
                                         VK_QUEUE_FAMILY_FOREIGN_EXT, flags));

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
