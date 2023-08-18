/*-------------------------------------------------------------------------
 * drawElements Quality Program Tester Core
 * ----------------------------------------
 *
 * Copyright 2023 The Android Open Source Project
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

#ifndef SRC_TESTS_DEQP_SUPPORT_TCUANGLENATIVEWINDOWFACTORY_H_
#define SRC_TESTS_DEQP_SUPPORT_TCUANGLENATIVEWINDOWFACTORY_H_

#include "egluNativeWindow.hpp"
#include "eglwDefs.hpp"
#include "tcuANGLENativeDisplay.h"
#include "util/OSWindow.h"

namespace tcu
{
class NativeWindowFactory : public eglu::NativeWindowFactory
{
  public:
    static NativeWindowFactory *New(EventState *eventState, uint32_t preRotation);

    eglu::NativeWindow *createWindow(eglu::NativeDisplay *nativeDisplay,
                                     const eglu::WindowParams &params) const override;

    eglu::NativeWindow *createWindow(eglu::NativeDisplay *nativeDisplay,
                                     eglw::EGLDisplay display,
                                     eglw::EGLConfig config,
                                     const eglw::EGLAttrib *attribList,
                                     const eglu::WindowParams &params) const override;

    ~NativeWindowFactory() override = default;

  protected:
    NativeWindowFactory(EventState *eventState,
                        uint32_t preRotation,
                        const eglu::NativeWindow::Capability windowCapabilities);

  private:
    EventState *mEvents;
    uint32_t mPreRotation;
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
    IVec2 getScreenSize() const override;
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
}  // namespace tcu

#endif  // SRC_TESTS_DEQP_SUPPORT_TCUANGLENATIVEWINDOWFACTORY_H_
