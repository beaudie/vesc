#ifndef _TCUANGLENATIVEDISPLAY_H
#define _TCUANGLENATIVEDISPLAY_H

#include "EGL/eglplatform.h"
#include "egluNativeDisplay.hpp"
#include "eglwDefs.hpp"
#include "eglwLibrary.hpp"

namespace tcu
{
enum
{
    DEFAULT_SURFACE_WIDTH  = 400,
    DEFAULT_SURFACE_HEIGHT = 300,
};
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
#endif