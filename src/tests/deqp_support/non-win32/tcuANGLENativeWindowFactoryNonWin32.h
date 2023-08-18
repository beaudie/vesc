#ifndef _TCUANGLENATIVEDISPLAYFACTORYANDROID_H
#define _TCUANGLENATIVEDISPLAYFACTORYANDROID_H

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
#endif