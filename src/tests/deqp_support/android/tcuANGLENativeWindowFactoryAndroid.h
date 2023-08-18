#ifndef _TCUANGLENATIVEDISPLAYFACTORYANDROID_H
#define _TCUANGLENATIVEDISPLAYFACTORYANDROID_H

#include "../tcuANGLENativeDisplay.h"
#include "egluDefs.hpp"
#include "eglwDefs.hpp"
#include "tcuDefs.hpp"
#include "util/OSWindow.h"

namespace tcu
{
class NativeWindowFactoryAndroid : public eglu::NativeWindowFactory
{
  public:
    explicit NativeWindowFactoryAndroid(EventState *eventState, uint32_t preRotation);
    ~NativeWindowFactoryAndroid() override = default;

    eglu::NativeWindow *createWindow(eglu::NativeDisplay *nativeDisplay,
                                     const eglu::WindowParams &params) const override;
    eglu::NativeWindow *createWindow(eglu::NativeDisplay *nativeDisplay,
                                     eglw::EGLDisplay display,
                                     eglw::EGLConfig config,
                                     const eglw::EGLAttrib *attribList,
                                     const eglu::WindowParams &params) const override;

  private:
    EventState *mEvents;
    uint32_t mPreRotation;
};

class NativeWindowAndroid : public eglu::NativeWindow
{
  public:
    NativeWindowAndroid(ANGLENativeDisplay *NativeDisplay,
                        const eglu::WindowParams &params,
                        EventState *eventState,
                        uint32_t preRotation);
    ~NativeWindowAndroid() override;
    eglw::EGLNativeWindowType getLegacyNative() override;
    void *getPlatformExtension() override;
    IVec2 getSurfaceSize() const override;
    IVec2 getScreenSize() const override;
    void processEvents() override;
    void setSurfaceSize(IVec2 size) override;
    void setVisibility(eglu::WindowParams::Visibility visibility) override;
    void readScreenPixels(tcu::TextureLevel *dst) const override;

  private:
    OSWindow *mWindow;
    EventState *mEvents;
    uint32_t mPreRotation;
};
}  // namespace tcu
#endif