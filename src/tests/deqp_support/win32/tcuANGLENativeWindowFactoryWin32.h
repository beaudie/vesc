//
// Created by yuxinhu on 8/21/23.
//

#ifndef SRC_TESTS_DEQP_SUPPORT_TCUANGLENATIVEWINDOWFACTORYWIN32_H_
#define SRC_TESTS_DEQP_SUPPORT_TCUANGLENATIVEWINDOWFACTORYWIN32_H_

#include "../tcuANGLENativeDisplay.h"
#include "../tcuANGLENativeWindowFactory.h"
#include "egluDefs.hpp"
#include "eglwDefs.hpp"
#include "tcuDefs.hpp"
#include "util/OSWindow.h"

namespace tcu
{
class NativeWindowFactoryWin32 : public NativeWindowFactory
{
  public:
    explicit NativeWindowFactoryWin32(EventState *eventState,
                                      uint32_t preRotation,
                                      const eglu::NativeWindow::Capability windowCapability);
    ~NativeWindowFactoryWin32() override = default;
};

class NativeWindowWin32 : public NativeWindow
{
  public:
    NativeWindowWin32(ANGLENativeDisplay *NativeDisplay,
                      const eglu::WindowParams &params,
                      EventState *eventState,
                      uint32_t preRotation,
                      const eglu::NativeWindow::Capability windowCapability);
    ~NativeWindowWin32() override = default;
};
}  // namespace tcu

#endif  // SRC_TESTS_DEQP_SUPPORT_TCUANGLENATIVEWINDOWFACTORYWIN32_H_
