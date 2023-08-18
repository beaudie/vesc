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
class NativeWindowFactoryAndroid : public NativeWindowFactory
{
  public:
    explicit NativeWindowFactoryAndroid(EventState *eventState,
                                        uint32_t preRotation,
                                        const eglu::NativeWindow::Capability windowCapability);
    ~NativeWindowFactoryAndroid() override = default;
};

class NativeWindowAndroid : public NativeWindow
{
  public:
    NativeWindowAndroid(ANGLENativeDisplay *NativeDisplay,
                        const eglu::WindowParams &params,
                        EventState *eventState,
                        uint32_t preRotation,
                        const eglu::NativeWindow::Capability windowCapability);
    ~NativeWindowAndroid() override = default;
};
}  // namespace tcu
#endif