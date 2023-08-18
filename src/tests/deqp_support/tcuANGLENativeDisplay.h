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

#ifndef SRC_TESTS_DEQP_SUPPORT_TCUANGLENATIVEDISPLAY_H
#define SRC_TESTS_DEQP_SUPPORT_TCUANGLENATIVEDISPLAY_H

#include "egluNativeDisplay.hpp"
#include "eglwDefs.hpp"
#include "eglwLibrary.hpp"

#include <EGL/eglplatform.h>

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

class ANGLENativeDisplay : public eglu::NativeDisplay
{
  public:
    explicit ANGLENativeDisplay(EGLNativeDisplayType display, std::vector<eglw::EGLAttrib> attribs);
    ~ANGLENativeDisplay() override = default;

    virtual void *getPlatformNative() override;

    virtual const eglw::EGLAttrib *getPlatformAttributes() const override
    {
        return &mPlatformAttributes[0];
    }
    virtual const eglw::Library &getLibrary() const override { return mLibrary; }

    EGLNativeDisplayType getDeviceContext() const { return mDeviceContext; }

  private:
    EGLNativeDisplayType mDeviceContext;
    eglw::DefaultLibrary mLibrary;
    std::vector<eglw::EGLAttrib> mPlatformAttributes;
};
}  // namespace tcu
#endif  // SRC_TESTS_DEQP_SUPPORT_TCUANGLENATIVEDISPLAY_H
