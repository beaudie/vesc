/*-------------------------------------------------------------------------
 * drawElements Quality Program Tester Core
 * ----------------------------------------
 *
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "egluNativeDisplay.hpp"

#include "tcuANGLENativeDisplayFactory.h"
#include "tcuANGLENativeWindowFactory.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "deClock.h"
#include "deMemory.h"
#include "egluDefs.hpp"
#include "eglwLibrary.hpp"
#include "tcuTexture.hpp"
#include "util/OSPixmap.h"
#include "util/OSWindow.h"
#include "util/autogen/angle_features_autogen.h"

#if defined(ANGLE_USE_X11)
#    include <X11/Xlib.h>
#endif

#if defined(ANGLE_USE_WAYLAND)
#    include <wayland-client.h>
#    include <wayland-egl-backend.h>
#endif

namespace tcu
{
namespace
{

constexpr eglu::NativeDisplay::Capability kDisplayCapabilities =
    static_cast<eglu::NativeDisplay::Capability>(
        eglu::NativeDisplay::CAPABILITY_GET_DISPLAY_PLATFORM |
        eglu::NativeDisplay::CAPABILITY_GET_DISPLAY_PLATFORM_EXT);
constexpr eglu::NativePixmap::Capability kBitmapCapabilities =
    eglu::NativePixmap::CAPABILITY_CREATE_SURFACE_LEGACY;

class NativePixmapFactory : public eglu::NativePixmapFactory
{
  public:
    NativePixmapFactory();
    ~NativePixmapFactory() override = default;

    eglu::NativePixmap *createPixmap(eglu::NativeDisplay *nativeDisplay,
                                     int width,
                                     int height) const override;
    eglu::NativePixmap *createPixmap(eglu::NativeDisplay *nativeDisplay,
                                     eglw::EGLDisplay display,
                                     eglw::EGLConfig config,
                                     const eglw::EGLAttrib *attribList,
                                     int width,
                                     int height) const override;
};

class NativePixmap : public eglu::NativePixmap
{
  public:
    NativePixmap(EGLNativeDisplayType display, int width, int height, int bitDepth);
    virtual ~NativePixmap();

    eglw::EGLNativePixmapType getLegacyNative() override;

  private:
    OSPixmap *mPixmap;
};
// NativePixmap

NativePixmap::NativePixmap(EGLNativeDisplayType display, int width, int height, int bitDepth)
    : eglu::NativePixmap(kBitmapCapabilities), mPixmap(CreateOSPixmap())
{
#if (DE_OS != DE_OS_UNIX)
    throw tcu::NotSupportedError("Pixmap not supported");
#else
    if (!mPixmap)
    {
        throw ResourceError("Failed to create pixmap", DE_NULL, __FILE__, __LINE__);
    }

    if (!mPixmap->initialize(display, width, height, bitDepth))
    {
        throw ResourceError("Failed to initialize pixmap", DE_NULL, __FILE__, __LINE__);
    }
#endif
}

NativePixmap::~NativePixmap()
{
    delete mPixmap;
}

eglw::EGLNativePixmapType NativePixmap::getLegacyNative()
{
    return reinterpret_cast<eglw::EGLNativePixmapType>(mPixmap->getNativePixmap());
}

// NativePixmapFactory

NativePixmapFactory::NativePixmapFactory()
    : eglu::NativePixmapFactory("bitmap", "ANGLE Bitmap", kBitmapCapabilities)
{}

eglu::NativePixmap *NativePixmapFactory::createPixmap(eglu::NativeDisplay *nativeDisplay,
                                                      eglw::EGLDisplay display,
                                                      eglw::EGLConfig config,
                                                      const eglw::EGLAttrib *attribList,
                                                      int width,
                                                      int height) const
{
    const eglw::Library &egl = nativeDisplay->getLibrary();
    int nativeVisual         = 0;

    DE_ASSERT(display != EGL_NO_DISPLAY);

    egl.getConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &nativeVisual);
    EGLU_CHECK_MSG(egl, "eglGetConfigAttrib()");

    return new NativePixmap(dynamic_cast<ANGLENativeDisplay *>(nativeDisplay)->getDeviceContext(),
                            width, height, nativeVisual);
}

eglu::NativePixmap *NativePixmapFactory::createPixmap(eglu::NativeDisplay *nativeDisplay,
                                                      int width,
                                                      int height) const
{
    const int defaultDepth = 32;
    return new NativePixmap(dynamic_cast<ANGLENativeDisplay *>(nativeDisplay)->getDeviceContext(),
                            width, height, defaultDepth);
}
}  // namespace

ANGLENativeDisplayFactory::ANGLENativeDisplayFactory(
    const std::string &name,
    const std::string &description,
    std::vector<eglw::EGLAttrib> platformAttributes,
    EventState *eventState)
    : eglu::NativeDisplayFactory(name,
                                 description,
                                 kDisplayCapabilities,
                                 EGL_PLATFORM_ANGLE_ANGLE,
                                 "EGL_EXT_platform_base"),
      mNativeDisplay(bitCast<eglw::EGLNativeDisplayType>(EGL_DEFAULT_DISPLAY)),
      mPlatformAttributes(std::move(platformAttributes))
{
#if (DE_OS == DE_OS_UNIX)
#    if defined(ANGLE_USE_X11)
    // Make sure to only open the X display once so that it can be used by the EGL display as well
    // as pixmaps
    mNativeDisplay = bitCast<eglw::EGLNativeDisplayType>(XOpenDisplay(nullptr));
#    endif  // ANGLE_USE_X11

#    if defined(ANGLE_USE_WAYLAND)
    if (mNativeDisplay == 0)
    {
        mNativeDisplay = bitCast<eglw::EGLNativeDisplayType>(wl_display_connect(nullptr));
    }
#    endif  // ANGLE_USE_WAYLAND
#endif      // (DE_OS == DE_OS_UNIX)

    // If pre-rotating, let NativeWindowFactory know.
    uint32_t preRotation = 0;
    for (size_t attrIndex = 0;
         attrIndex < mPlatformAttributes.size() && mPlatformAttributes[attrIndex] != EGL_NONE;
         attrIndex += 2)
    {
        if (mPlatformAttributes[attrIndex] != EGL_FEATURE_OVERRIDES_ENABLED_ANGLE)
        {
            continue;
        }

        const char **enabledFeatures =
            reinterpret_cast<const char **>(mPlatformAttributes[attrIndex + 1]);
        DE_ASSERT(enabledFeatures != nullptr && *enabledFeatures != nullptr);

        for (; *enabledFeatures; ++enabledFeatures)
        {
            if (strcmp(enabledFeatures[0],
                       angle::GetFeatureName(angle::Feature::EmulatedPrerotation90)) == 0)
            {
                preRotation = 90;
            }
            else if (strcmp(enabledFeatures[0],
                            angle::GetFeatureName(angle::Feature::EmulatedPrerotation180)) == 0)
            {
                preRotation = 180;
            }
            else if (strcmp(enabledFeatures[0],
                            angle::GetFeatureName(angle::Feature::EmulatedPrerotation270)) == 0)
            {
                preRotation = 270;
            }
        }
        break;
    }

    // m_nativeWindowRegistry.registerFactory(new NativeWindowFactory(eventState, preRotation));
    m_nativeWindowRegistry.registerFactory(NativeWindowFactory::New(eventState, preRotation));
    m_nativePixmapRegistry.registerFactory(new NativePixmapFactory());
}

ANGLENativeDisplayFactory::~ANGLENativeDisplayFactory() = default;

eglu::NativeDisplay *ANGLENativeDisplayFactory::createDisplay(
    const eglw::EGLAttrib *attribList) const
{
    DE_UNREF(attribList);
    return new ANGLENativeDisplay(bitCast<EGLNativeDisplayType>(mNativeDisplay),
                                  mPlatformAttributes);
}

}  // namespace tcu
