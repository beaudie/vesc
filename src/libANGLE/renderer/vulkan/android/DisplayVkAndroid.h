//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkAndroid.h:
//    Defines the class interface for DisplayVkAndroid, implementing DisplayVk for Android.
//

#ifndef LIBANGLE_RENDERER_VULKAN_ANDROID_DISPLAYVKANDROID_H_
#define LIBANGLE_RENDERER_VULKAN_ANDROID_DISPLAYVKANDROID_H_

#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/android/AHBFunctions.h"

namespace rx
{
class DisplayVkAndroid : public DisplayVk
{
  public:
    DisplayVkAndroid(const egl::DisplayState &state);

    egl::Error initialize(egl::Display *display) override;

    bool isValidNativeWindow(EGLNativeWindowType window) const override;

    SurfaceImpl *createWindowSurfaceVk(const egl::SurfaceState &state,
                                       EGLNativeWindowType window) override;

    egl::ConfigSet generateConfigs() override;
    void checkConfigSupport(egl::Config *config) override;

    egl::Error validateImageClientBuffer(const gl::Context *context,
                                         EGLenum target,
                                         EGLClientBuffer clientBuffer,
                                         const egl::AttributeMap &attribs) const override;

    ExternalImageSiblingImpl *createExternalImageSibling(const gl::Context *context,
                                                         EGLenum target,
                                                         EGLClientBuffer buffer,
                                                         const egl::AttributeMap &attribs) override;

    const char *getWSIExtension() const override;

    const AHBFunctions &getAHBFunctions() const { return mAHBFunctions; }

    // Adds an entry to mSurfaceMap (if not already) and returns the surface.  If the result is
    // VK_NULL_HANDLE, a new surface must be created to fill in the map entry.
    VkSurfaceKHR *onSurfaceCreate(EGLNativeWindowType nativeWindow);
    // Removes a ref from and possibly removes an entry in mSurfaceMap.  If true is returned, the
    // surface must be destroyed.
    bool onSurfaceDestroy(EGLNativeWindowType nativeWindow);

  private:
    void enableRecordableIfSupported(egl::Config *config);

    AHBFunctions mAHBFunctions;

    // Support for handing VkSurfaceKHR from one WindowSurfaceVkAndroid to another.  The Android WSI
    // extension does not allow multiple VkSurfaceKHR handles created from the same ANativeWindow.
    // In the following scenario, there are temporarily two EGL surfaces referencing the same
    // ANativeWindow however:
    //
    // - eglCreateWindowSurface + eglMakeCurrent
    // - eglDestroyWindowSurface
    //   * The Surface stays alive because it is current, VkSurfaceKHR is not destroyed
    // - eglCreateWindowSurface
    //   * Another Surface references the same ANativeWindow, fails to create VkSurfaceKHR
    // - eglMakeCurrent
    //   * Deletes the old surface and makes the new one current
    //
    // To support the above scenario, the following map is used such that the second
    // eglCreateWindowSurface succeeds by retrieving the VkSurfaceKHR created by the first surface
    // (and adds a ref).  When the first surface is destroyed, it removes a ref.
    //
    // Note that the above is also the reason why swapchain creation is deferred to eglMakeCurrent
    // in WindowSurfaceVk.cpp; each surface can only have one swapchain so the first surface's
    // swapchain must be destroyed before the second surface's swapchain can be created.
    std::mutex mSurfaceMapMutex;
    angle::HashMap<EGLNativeWindowType, vk::RefCounted<VkSurfaceKHR>> mSurfaceMap;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_ANDROID_DISPLAYVKANDROID_H_
