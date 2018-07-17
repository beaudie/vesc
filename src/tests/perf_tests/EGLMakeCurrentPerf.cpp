/*
  Tests for performance of EGLMakeCurrent
 */

#include "ANGLEPerfTest.h"
#include "test_utils/angle_test_configs.h"
#include "test_utils/angle_test_instantiate.h"
#include "platform/Platform.h"

using namespace testing;

namespace
{
class EGLMakeCurrentPerfTest : public ANGLEPerfTest,
                               public WithParamInterface<angle::PlatformParameters>
{
  public:
  EGLMakeCurrentPerfTest();

    void step() override;
    void SetUp() override;
    void TearDown() override;

  private:
    OSWindow *mOSWindow;
    EGLDisplay mDisplay;
    EGLSurface mSurface;
    EGLConfig mConfig;
    int currContext = 0;
    EGLContext mContext[2];
};

EGLMakeCurrentPerfTest::EGLMakeCurrentPerfTest()
    : ANGLEPerfTest("EGLMakeCurrent", "_run"),
      mOSWindow(nullptr),
      mDisplay(EGL_NO_DISPLAY)
{
  auto platform = GetParam().eglParameters;

  std::vector<EGLint> displayAttributes;
  displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
  displayAttributes.push_back(platform.renderer);
  displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE);
  displayAttributes.push_back(platform.majorVersion);
  displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE);
  displayAttributes.push_back(platform.minorVersion);

  if (platform.renderer == EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE ||
      platform.renderer == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
  {
    displayAttributes.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE);
    displayAttributes.push_back(platform.deviceType);
  }
  displayAttributes.push_back(EGL_NONE);

  mOSWindow = CreateOSWindow();
  mOSWindow->initialize("EGLMakeCurrent Test", 64, 64);

  auto eglGetPlatformDisplayEXT =
      reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
  if (eglGetPlatformDisplayEXT == nullptr)
  {
    std::cerr << "Error getting platform display!" << std::endl;
    return;
  }

  mDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE,
                                      reinterpret_cast<void *>(mOSWindow->getNativeDisplay()),
                                      &displayAttributes[0]);
}

void EGLMakeCurrentPerfTest::SetUp(){
  EGLint majorVersion, minorVersion;
  eglInitialize(mDisplay, &majorVersion, &minorVersion);

  EGLint numConfigs;
  EGLint configAttrs[] = {
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_RENDERABLE_TYPE, GetParam().majorVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT,
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_NONE
  };

  eglChooseConfig(mDisplay, configAttrs, &mConfig, 1, &numConfigs);

  mContext[0] = eglCreateContext(mDisplay, mConfig, EGL_NO_CONTEXT, NULL);
  mContext[1] = eglCreateContext(mDisplay, mConfig, EGL_NO_CONTEXT, NULL);

  mSurface = eglCreateWindowSurface(mDisplay, mConfig, mOSWindow->getNativeWindow(), NULL);
  eglMakeCurrent(mDisplay, mSurface, mSurface, mContext[0]);
}

void EGLMakeCurrentPerfTest::TearDown(){
  ANGLEPerfTest::TearDown();
  eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  eglDestroySurface(mDisplay, mSurface);
  eglDestroyContext(mDisplay, mContext[0]);
  eglDestroyContext(mDisplay, mContext[1]);
  ANGLEResetDisplayPlatform(mDisplay);
}

void EGLMakeCurrentPerfTest::step(){
  eglMakeCurrent(mDisplay, mSurface, mSurface, mContext[!currContext]);
}

TEST_P(EGLMakeCurrentPerfTest, Run){
  run();
}

ANGLE_INSTANTIATE_TEST(EGLMakeCurrentPerfTest, angle::ES2_OPENGL(),
                                               angle::ES2_OPENGLES(),
                                               angle::ES2_VULKAN());

} //namespace
