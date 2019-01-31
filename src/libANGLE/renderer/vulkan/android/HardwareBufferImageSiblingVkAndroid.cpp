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

#include <android/log.h>

namespace rx
{
HardwareBufferImageSiblingVkAndroid::HardwareBufferImageSiblingVkAndroid(EGLClientBuffer buffer)
    : mBuffer(buffer), mFormat(GL_NONE)
{}

HardwareBufferImageSiblingVkAndroid::~HardwareBufferImageSiblingVkAndroid() {}

egl::Error HardwareBufferImageSiblingVkAndroid::initialize(const egl::Display *display)
{
    __android_log_print(ANDROID_LOG_ERROR, "ANGLE",
                        "HardwareBufferImageSiblingVkAndroid::initialize");

    DisplayVk *displayVk = vk::GetImpl(display);
    RendererVk *renderer = displayVk->getRenderer();

    // The EGL_ANDROID_image_native_buffer spec says that the buffer is a ANativeWindowBuffer. This
    // is the same type as AHardwareBuffer in android currently but may change in the future. If
    // this happens, we will need to request new APIs to convert between them.
    struct AHardwareBuffer *hardwareBuffer = static_cast<struct AHardwareBuffer *>(mBuffer);

    VkAndroidHardwareBufferPropertiesANDROID bufferProperties = {};
    bufferProperties.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;

    VkResult result = vkGetAndroidHardwareBufferPropertiesANDROID(
        renderer->getDevice(), hardwareBuffer, &bufferProperties);
    if (result != VK_SUCCESS)
    {
        return egl::EglBadParameter() << "Failed to query hardware buffer properties.";
    }

    int pixelFormat = 0;
    angle::android::GetANativeWindowBufferProperties(mBuffer, &mSize.width, &mSize.height,
                                                     &mSize.depth, &pixelFormat);
    mFormat = gl::Format(angle::android::NativePixelFormatToGLInternalFormat(pixelFormat));

    __android_log_print(ANDROID_LOG_ERROR, "ANGLE",
                        "HardwareBufferImageSiblingVkAndroid: %u x %u, size: %lu", mSize.width,
                        mSize.height, bufferProperties.allocationSize);

    return egl::NoError();
}

void HardwareBufferImageSiblingVkAndroid::onDestroy(const egl::Display *display) {}

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
    return nullptr;
}
}  // namespace rx
