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

#ifndef TCU_ANGLE_WIN32_NATIVE_DISPLAY_FACTORY_H_
#define TCU_ANGLE_WIN32_NATIVE_DISPLAY_FACTORY_H_

#include <EGL/egl.h>
#include "egluNativeDisplay.hpp"
#include "eglwDefs.hpp"
#include "eglwLibrary.hpp"
#include "tcuDefs.hpp"
#include "util/OSWindow.h"

namespace tcu
{

template <typename destType, typename sourceType>
destType bitCast(sourceType source)
{
    constexpr size_t copySize =
        sizeof(destType) < sizeof(sourceType) ? sizeof(destType) : sizeof(sourceType);
    destType output(0);
    memcpy(&output, &source, copySize);
    return output;
}

class EventState
{
  public:
    EventState() : mQuit(false) {}
    bool quitSignaled() const { return mQuit; };
    void signalQuitEvent() { mQuit = true; };

  private:
    bool mQuit;
};

class ANGLENativeDisplayFactory : public eglu::NativeDisplayFactory
{
  public:
    ANGLENativeDisplayFactory(const std::string &name,
                              const std::string &description,
                              std::vector<eglw::EGLAttrib> platformAttributes,
                              EventState *eventState);
    ~ANGLENativeDisplayFactory() override;

    eglu::NativeDisplay *createDisplay(const eglw::EGLAttrib *attribList) const override;

  private:
    eglw::EGLNativeDisplayType mNativeDisplay;
    std::vector<eglw::EGLAttrib> mPlatformAttributes;
};

class ANGLENativeDisplay : public eglu::NativeDisplay
{
  public:
    explicit ANGLENativeDisplay(EGLNativeDisplayType display, std::vector<eglw::EGLAttrib> attribs);
    ~ANGLENativeDisplay() override = default;

    void *getPlatformNative() override
    {
        // On OSX 64bits mDeviceContext is a 32 bit integer, so we can't simply
        // use reinterpret_cast<void*>.
        return bitCast<void *>(mDeviceContext);
    }
    const eglw::EGLAttrib *getPlatformAttributes() const override
    {
        return &mPlatformAttributes[0];
    }
    const eglw::Library &getLibrary() const override { return mLibrary; }

    EGLNativeDisplayType getDeviceContext() const { return mDeviceContext; }

  private:
    EGLNativeDisplayType mDeviceContext;
    eglw::DefaultLibrary mLibrary;
    std::vector<eglw::EGLAttrib> mPlatformAttributes;
};

class NativeWindow : public eglu::NativeWindow
{
  public:
    static NativeWindow *New(ANGLENativeDisplay *nativeDisplay,
                             const eglu::WindowParams &params,
                             EventState *eventState,
                             uint32_t preRotation);
    ~NativeWindow() override;

    eglw::EGLNativeWindowType getLegacyNative() override;
    void *getPlatformExtension() override;
    IVec2 getSurfaceSize() const override;
    IVec2 getScreenSize() const override { return getSurfaceSize(); }
    void processEvents() override;
    void setSurfaceSize(IVec2 size) override;
    void setVisibility(eglu::WindowParams::Visibility visibility) override;
    void readScreenPixels(tcu::TextureLevel *dst) const override;

  protected:
    NativeWindow(ANGLENativeDisplay *NativeDisplay,
                 const eglu::WindowParams &params,
                 EventState *eventState,
                 uint32_t preRotation,
                 const eglu::NativeWindow::Capability windowCapability);

  private:
    OSWindow *mWindow;
    EventState *mEvents;
    uint32_t mPreRotation;
};

class NativeWindowFactory : public eglu::NativeWindowFactory
{
  public:
    static NativeWindowFactory *New(EventState *eventState, uint32_t preRotation);

    ~NativeWindowFactory() override = default;

    eglu::NativeWindow *createWindow(eglu::NativeDisplay *nativeDisplay,
                                     const eglu::WindowParams &params) const override;
    eglu::NativeWindow *createWindow(eglu::NativeDisplay *nativeDisplay,
                                     eglw::EGLDisplay display,
                                     eglw::EGLConfig config,
                                     const eglw::EGLAttrib *attribList,
                                     const eglu::WindowParams &params) const override;

  protected:
    explicit NativeWindowFactory(EventState *eventState,
                                 uint32_t preRotation,
                                 const eglu::NativeWindow::Capability windowCapabilities);

  private:
    EventState *mEvents;
    uint32_t mPreRotation;
};

}  // namespace tcu

#endif  // TCU_ANGLE_WIN32_NATIVE_DISPLAY_FACTORY_H_
