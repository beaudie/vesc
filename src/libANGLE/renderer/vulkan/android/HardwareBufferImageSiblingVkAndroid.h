//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// HardwareBufferImageSiblingVkAndroid.h: Defines the HardwareBufferImageSiblingVkAndroid to wrap
// EGL images created from AHardwareBuffer objects

#ifndef LIBANGLE_RENDERER_VULKAN_ANDROID_HARDWAREBUFFERIMAGESIBLINGVKANDROID_H_
#define LIBANGLE_RENDERER_VULKAN_ANDROID_HARDWAREBUFFERIMAGESIBLINGVKANDROID_H_

#include "libANGLE/renderer/vulkan/ImageVk.h"

namespace rx
{

class HardwareBufferImageSiblingVkAndroid : public ExternalImageSiblingVk
{
  public:
    HardwareBufferImageSiblingVkAndroid(EGLClientBuffer buffer);
    ~HardwareBufferImageSiblingVkAndroid() override;

    egl::Error initialize(const egl::Display *display) override;
    void onDestroy(const egl::Display *display) override;

    // ExternalImageSiblingImpl interface
    gl::Format getFormat() const override;
    bool isRenderable(const gl::Context *context) const override;
    bool isTexturable(const gl::Context *context) const override;
    gl::Extents getSize() const override;
    size_t getSamples() const override;

    // ExternalImageSiblingVk interface
    vk::ImageHelper *getImage() const override;

  private:
    EGLClientBuffer mBuffer;
    gl::Extents mSize;
    gl::Format mFormat;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_ANDROID_HARDWAREBUFFERIMAGESIBLINGVKANDROID_H_
