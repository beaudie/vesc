#include "gtest/gtest.h"

#include <thread>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "system_utils.h"

typedef EGLAPI EGLDisplay(EGLAPIENTRY *EGLGetDisplay)(EGLNativeDisplayType display_id);
typedef EGLAPI EGLBoolean(EGLAPIENTRY *EGLInitialize)(EGLDisplay dpy, EGLint *major, EGLint *minor);
typedef EGLAPI EGLContext(EGLAPIENTRY *EGLGetCurrentContext)(void);
typedef EGLAPI EGLSurface(EGLAPIENTRY *EGLGetCurrentSurface)(EGLint readdraw);
typedef EGLAPI EGLBoolean(EGLAPIENTRY *EGLTerminate)(EGLDisplay dpy);

class EGLThreadTest : public testing::Test
{
  public:
    void ThreadingTest();

  protected:
    EGLGetDisplay mGetDisplay               = nullptr;
    EGLInitialize mInitialize               = nullptr;
    EGLGetCurrentContext mGetCurrentContext = nullptr;
    EGLGetCurrentSurface mGetCurrentSurface = nullptr;

    EGLDisplay mDisplay = EGL_NO_DISPLAY;

    std::unique_ptr<angle::Library> mEGL;
};

void EGLThreadTest::ThreadingTest()
{
    mEGL.reset(angle::loadLibrary("libEGL"));

    EXPECT_TRUE(mEGL);

    mGetDisplay = reinterpret_cast<EGLGetDisplay>(mEGL->getSymbol("eglGetDisplay"));
    mInitialize = reinterpret_cast<EGLInitialize>(mEGL->getSymbol("eglInitialize"));
    mGetCurrentContext =
        reinterpret_cast<EGLGetCurrentContext>(mEGL->getSymbol("eglGetCurrentContext"));
    mGetCurrentSurface =
        reinterpret_cast<EGLGetCurrentSurface>(mEGL->getSymbol("eglGetCurrentSurface"));

    EXPECT_TRUE(mGetDisplay != NULL);
    EXPECT_TRUE(mInitialize != NULL);
    EXPECT_TRUE(mGetCurrentContext != NULL);
    EXPECT_TRUE(mGetCurrentSurface != NULL);

    mDisplay = mGetDisplay(EGL_DEFAULT_DISPLAY);

    EXPECT_TRUE(mDisplay != EGL_NO_DISPLAY);

    mInitialize(mDisplay, NULL, NULL);
    mGetCurrentContext();
}

// Test a bug in our EGL TLS implementation.
TEST_F(EGLThreadTest, thread_init_crash)
{
    std::thread runner(&EGLThreadTest::ThreadingTest, this);

    // wait for signal from thread
    runner.join();

    // crash, because the TLS value is NULL on main thread
    mGetCurrentSurface(EGL_DRAW);
    mGetCurrentContext();

    auto terminate = reinterpret_cast<EGLTerminate>(mEGL->getSymbol("eglTerminate"));
    terminate(mDisplay);
}
