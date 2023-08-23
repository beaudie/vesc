//
// Created by yuxinhu on 8/21/23.
//
#ifndef SRC_TESTS_DEQP_SUPPORT_TCUANGLENATIVEWINDOWFACTORY_H_
#define SRC_TESTS_DEQP_SUPPORT_TCUANGLENATIVEWINDOWFACTORY_H_
#include "egluNativeDisplay.hpp"
// #include "eglwDefs.hpp"
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
