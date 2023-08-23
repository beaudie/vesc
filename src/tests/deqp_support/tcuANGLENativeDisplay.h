#ifndef _TCU_ANGLE_NATIVE_DISPLAY_H_
#define _TCU_ANGLE_NATIVE_DISPLAY_H_

#include "EGL/eglplatform.h"
#include "egluNativeDisplay.hpp"
#include "eglwLibrary.hpp"

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
}  // namespace tcu
#endif
