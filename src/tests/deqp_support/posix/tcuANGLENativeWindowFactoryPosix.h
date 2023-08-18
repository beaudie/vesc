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

#ifndef SRC_TESTS_DEQP_SUPPORT_TCUANGLENATIVEDISPLAYFACTORYPOSIX_H
#define SRC_TESTS_DEQP_SUPPORT_TCUANGLENATIVEDISPLAYFACTORYPOSIX_H

#include "../tcuANGLENativeDisplay.h"
#include "../tcuANGLENativeWindowFactory.h"
#include "egluDefs.hpp"
#include "eglwDefs.hpp"
#include "tcuDefs.hpp"
#include "util/OSWindow.h"

namespace tcu
{
class NativeWindowFactoryNonWin32 : public NativeWindowFactory
{
  public:
    explicit NativeWindowFactoryNonWin32(EventState *eventState,
                                         uint32_t preRotation,
                                         const eglu::NativeWindow::Capability windowCapability);
    ~NativeWindowFactoryNonWin32() override = default;
};

class NativeWindowNonWin32 : public NativeWindow
{
  public:
    NativeWindowNonWin32(ANGLENativeDisplay *NativeDisplay,
                         const eglu::WindowParams &params,
                         EventState *eventState,
                         uint32_t preRotation,
                         const eglu::NativeWindow::Capability windowCapability);
    ~NativeWindowNonWin32() override = default;
};
}  // namespace tcu
#endif  // SRC_TESTS_DEQP_SUPPORT_TCUANGLENATIVEDISPLAYFACTORYPOSIX_H
