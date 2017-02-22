//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLContextCompatibilityTest.cpp:
//   This test will try to use all combinations of context configs and
//   surface configs. If the configs are compatible, it checks that simple
//   rendering works, otherwise it checks an error is generated one MakeCurrent.
#include <gtest/gtest.h>

#include <vector>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "../../libANGLE/Config.h"

#include "OSWindow.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_configs.h"

using namespace angle;

struct Config
{
    Config();

    GLenum renderTargetFormat;      // TODO(geofflang): remove this
    GLenum depthStencilFormat;      // TODO(geofflang): remove this

    EGLint bufferSize;              // Depth of the color buffer
    EGLint redSize;                 // Bits of Red in the color buffer
    EGLint greenSize;               // Bits of Green in the color buffer
    EGLint blueSize;                // Bits of Blue in the color buffer
    EGLint luminanceSize;           // Bits of Luminance in the color buffer
    EGLint alphaSize;               // Bits of Alpha in the color buffer
    EGLint alphaMaskSize;           // Bits of Alpha Mask in the mask buffer
    EGLBoolean bindToTextureRGB;    // True if bindable to RGB textures.
    EGLBoolean bindToTextureRGBA;   // True if bindable to RGBA textures.
    EGLenum colorBufferType;        // Color buffer type
    EGLenum configCaveat;           // Any caveats for the configuration
    EGLint configID;                // Unique EGLConfig identifier
    EGLint conformant;              // Whether contexts created with this config are conformant
    EGLint depthSize;               // Bits of Z in the depth buffer
    EGLint level;                   // Frame buffer level
    EGLBoolean matchNativePixmap;   // Match the native pixmap format
    EGLint maxPBufferWidth;         // Maximum width of pbuffer
    EGLint maxPBufferHeight;        // Maximum height of pbuffer
    EGLint maxPBufferPixels;        // Maximum size of pbuffer
    EGLint maxSwapInterval;         // Maximum swap interval
    EGLint minSwapInterval;         // Minimum swap interval
    EGLBoolean nativeRenderable;    // EGL_TRUE if native rendering APIs can render to surface
    EGLint nativeVisualID;          // Handle of corresponding native visual
    EGLint nativeVisualType;        // Native visual type of the associated visual
    EGLint renderableType;          // Which client rendering APIs are supported.
    EGLint sampleBuffers;           // Number of multisample buffers
    EGLint samples;                 // Number of samples per pixel
    EGLint stencilSize;             // Bits of Stencil in the stencil buffer
    EGLint surfaceType;             // Which types of EGL surfaces are supported.
    EGLenum transparentType;        // Type of transparency supported
    EGLint transparentRedValue;     // Transparent red value
    EGLint transparentGreenValue;   // Transparent green value
    EGLint transparentBlueValue;    // Transparent blue value
    EGLint optimalOrientation;      // Optimal window surface orientation
    EGLenum colorComponentType;     // Color component type
};

namespace
{

const EGLint contextAttribs[] =
{
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
};

}

class EGLContextCompatibilityTest : public ANGLETest
{
  public:
    EGLContextCompatibilityTest()
      : mDisplay(0)
    {
    }

    void SetUp() override
    {
        PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
        ASSERT_TRUE(eglGetPlatformDisplayEXT != nullptr);

        EGLint dispattrs[] =
        {
            EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(),
            EGL_NONE
        };
        mDisplay = eglGetPlatformDisplayEXT(
            EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);

        ASSERT_TRUE(eglInitialize(mDisplay, nullptr, nullptr) == EGL_TRUE);

        int nConfigs = 0;
        ASSERT_TRUE(eglGetConfigs(mDisplay, nullptr, 0, &nConfigs) == EGL_TRUE);
        ASSERT_TRUE(nConfigs != 0);

        int nReturnedConfigs = 0;
        mConfigs.resize(nConfigs);
        ASSERT_TRUE(eglGetConfigs(mDisplay, mConfigs.data(), nConfigs, &nReturnedConfigs) == EGL_TRUE);
        ASSERT_TRUE(nConfigs == nReturnedConfigs);
    }

    void TearDown() override
    {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglTerminate(mDisplay);
    };

  protected:
    bool areConfigsCompatible(EGLConfig c1, EGLConfig c2, EGLint surfaceBit)
    {
        EGLint colorBufferType1, colorBufferType2;
        EGLint red1, red2, green1, green2, blue1, blue2, alpha1, alpha2;
        EGLint depth1, depth2, stencil1, stencil2;
        EGLint surfaceType1, surfaceType2;

        eglGetConfigAttrib(mDisplay, c1, EGL_COLOR_BUFFER_TYPE, &colorBufferType1);
        eglGetConfigAttrib(mDisplay, c2, EGL_COLOR_BUFFER_TYPE, &colorBufferType2);

        eglGetConfigAttrib(mDisplay, c1, EGL_RED_SIZE, &red1);
        eglGetConfigAttrib(mDisplay, c2, EGL_RED_SIZE, &red2);
        eglGetConfigAttrib(mDisplay, c1, EGL_GREEN_SIZE, &green1);
        eglGetConfigAttrib(mDisplay, c2, EGL_GREEN_SIZE, &green2);
        eglGetConfigAttrib(mDisplay, c1, EGL_BLUE_SIZE, &blue1);
        eglGetConfigAttrib(mDisplay, c2, EGL_BLUE_SIZE, &blue2);
        eglGetConfigAttrib(mDisplay, c1, EGL_ALPHA_SIZE, &alpha1);
        eglGetConfigAttrib(mDisplay, c2, EGL_ALPHA_SIZE, &alpha2);

        eglGetConfigAttrib(mDisplay, c1, EGL_DEPTH_SIZE, &depth1);
        eglGetConfigAttrib(mDisplay, c2, EGL_DEPTH_SIZE, &depth2);
        eglGetConfigAttrib(mDisplay, c1, EGL_STENCIL_SIZE, &stencil1);
        eglGetConfigAttrib(mDisplay, c2, EGL_STENCIL_SIZE, &stencil2);

        eglGetConfigAttrib(mDisplay, c1, EGL_SURFACE_TYPE, &surfaceType1);
        eglGetConfigAttrib(mDisplay, c2, EGL_SURFACE_TYPE, &surfaceType2);

        EGLint colorComponentType1 = EGL_COLOR_COMPONENT_TYPE_FIXED_EXT;
        EGLint colorComponentType2 = EGL_COLOR_COMPONENT_TYPE_FIXED_EXT;
        if (eglDisplayExtensionEnabled(mDisplay, "EGL_EXT_pixel_format_float"))
        {
            eglGetConfigAttrib(mDisplay, c1, EGL_COLOR_COMPONENT_TYPE_EXT, &colorComponentType1);
            eglGetConfigAttrib(mDisplay, c2, EGL_COLOR_COMPONENT_TYPE_EXT, &colorComponentType2);
        }

        EXPECT_EGL_SUCCESS();

        return colorBufferType1 == colorBufferType2 && red1 == red2 && green1 == green2 &&
               blue1 == blue2 && alpha1 == alpha2 && colorComponentType1 == colorComponentType2 &&
               depth1 == depth2 && stencil1 == stencil2 && (surfaceType1 & surfaceBit) != 0 &&
               (surfaceType2 & surfaceBit) != 0;
    }

    void testWindowCompatibility(EGLConfig windowConfig, EGLConfig contextConfig, bool compatible) const
    {
        OSWindow *osWindow = CreateOSWindow();
        ASSERT_TRUE(osWindow != nullptr);
        osWindow->initialize("EGLContextCompatibilityTest", 500, 500);
        osWindow->setVisible(true);

        EGLContext context = eglCreateContext(mDisplay, contextConfig, EGL_NO_CONTEXT, contextAttribs);
        ASSERT_TRUE(context != EGL_NO_CONTEXT);

        EGLSurface window = eglCreateWindowSurface(mDisplay, windowConfig, osWindow->getNativeWindow(), nullptr);
        ASSERT_EGL_SUCCESS();

        if (compatible)
        {
            testClearSurface(window, windowConfig, context);
        }
        else
        {
            testMakeCurrentFails(window, context);
        }

        eglDestroySurface(mDisplay, window);
        ASSERT_EGL_SUCCESS();

        eglDestroyContext(mDisplay, context);
        ASSERT_EGL_SUCCESS();

        SafeDelete(osWindow);
    }

    void testPbufferCompatibility(EGLConfig pbufferConfig, EGLConfig contextConfig, bool compatible) const
    {
        Config *p = static_cast<Config*>(pbufferConfig);
        Config *c = static_cast<Config*>(contextConfig);
        std::cout << "Start testPbufferCompatibility\n";
        std::cout << "pbuffer-S:" << p->samples << " RTF:" << p->renderTargetFormat << " DSF:" << p->depthStencilFormat << "\n";
        std::cout << "context-S:" << c->samples << " RTF:" << c->renderTargetFormat << " DSF:" << c->depthStencilFormat << "\n";

        std::cout << "Start eglCreateContext\n";
        EGLContext context = eglCreateContext(mDisplay, contextConfig, EGL_NO_CONTEXT, contextAttribs);
        std::cout << "success eglCreateContext\n";
        ASSERT_TRUE(context != EGL_NO_CONTEXT);

        const EGLint pBufferAttribs[] =
        {
            EGL_WIDTH, 500,
            EGL_HEIGHT, 500,
            EGL_NONE,
        };
        std::cout << "Start eglCreatePbufferSurface\n";
        EGLSurface pbuffer = eglCreatePbufferSurface(mDisplay, pbufferConfig, pBufferAttribs);
        std::cout << "Success eglCreatePbufferSurface\n";
        ASSERT_TRUE(pbuffer != EGL_NO_SURFACE);

        if (compatible)
        {
            std::cout << "Start Compatible\n";
            testClearSurface(pbuffer, pbufferConfig, context);
            std::cout << "Success Compatible\n";
        }
        else
        {
            std::cout << "Start Safe Fail\n";
            testMakeCurrentFails(pbuffer, context);
            std::cout << "Success Safe Fail\n";
        }
        std::cout << "Start eglDestroySurface\n";
        eglDestroySurface(mDisplay, pbuffer);
        std::cout << "Success eglDestroySurface\n";
        ASSERT_EGL_SUCCESS();

        std::cout << "Start eglDestroyContext\n";
        eglDestroyContext(mDisplay, context);
        std::cout << "Success eglDestroyContext\n";
        ASSERT_EGL_SUCCESS();
        std::cout << "Success testPbufferCompatibility\n";
    }

    std::vector<EGLConfig> mConfigs;
    EGLDisplay mDisplay;

  private:
    void testClearSurface(EGLSurface surface, EGLConfig surfaceConfig, EGLContext context) const
    {
        std::cout << "Start eglMakeCurrent\n";
        eglMakeCurrent(mDisplay, surface, surface, context);
        ASSERT_EGL_SUCCESS();
        std::cout << "Success eglMakeCurrent\n";

        std::cout << "Start gl calls\n";
        glViewport(0, 0, 500, 500);
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR();
        std::cout << "Success gl calls\n";

        EGLint surfaceCompontentType = EGL_COLOR_COMPONENT_TYPE_FIXED_EXT;
        if (eglDisplayExtensionEnabled(mDisplay, "EGL_EXT_pixel_format_float"))
        {
            eglGetConfigAttrib(mDisplay, surfaceConfig, EGL_COLOR_COMPONENT_TYPE_EXT,
                               &surfaceCompontentType);
        }

        if (surfaceCompontentType == EGL_COLOR_COMPONENT_TYPE_FIXED_EXT)
        {
            EXPECT_PIXEL_EQ(250, 250, 0, 0, 255, 255);
        }
        else
        {
            EXPECT_PIXEL_32F_EQ(250, 250, 0, 0, 1.0f, 1.0f);
        }

        std::cout << "Start eglMakeCurrent\n";
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        ASSERT_EGL_SUCCESS();
        std::cout << "Success eglMakeCurrent\n";

    }

    void testMakeCurrentFails(EGLSurface surface, EGLContext context) const
    {
        eglMakeCurrent(mDisplay, surface, surface, context);
        EXPECT_EGL_ERROR(EGL_BAD_MATCH);
    }
};

// The test is split in several subtest so that simple cases
// are tested separately. Also each surface types are not tested
// together.

// Basic test checking contexts and windows created with the
// same config can render.
TEST_P(EGLContextCompatibilityTest, WindowSameConfig)
{
    for (size_t i = 0; i < mConfigs.size(); i++)
    {
        EGLConfig config = mConfigs[i];
       
        EGLint surfaceType;
        eglGetConfigAttrib(mDisplay, config, EGL_SURFACE_TYPE, &surfaceType);
        ASSERT_EGL_SUCCESS();

        if ((surfaceType & EGL_WINDOW_BIT) != 0)
        {
            testWindowCompatibility(config, config, true);
        }
    }
}

// Basic test checking contexts and pbuffers created with the
// same config can render.
TEST_P(EGLContextCompatibilityTest, PbufferSameConfig)
{
    std::cout << "=======================================================\n";
    std::cout << "PbufferSameConfig Test Start\n";
    for (size_t i = 0; i < mConfigs.size(); i++)
    {
        EGLConfig config = mConfigs[i];

        Config *configuration = static_cast<Config*>(config);
        std::cout << "\n";
        std::cout << "Loop Start-S:" << configuration->samples << " RTF:" << configuration->renderTargetFormat << " DSF:" << configuration->depthStencilFormat << "\n";
        EGLint surfaceType;
     
        std::cout << "Start GetConfigAttrib\n";
        eglGetConfigAttrib(mDisplay, config, EGL_SURFACE_TYPE, &surfaceType);
        std::cout << "Success GetConfigAttrib\n";
        ASSERT_EGL_SUCCESS();

        if ((surfaceType & EGL_PBUFFER_BIT) != 0)
        {
            testPbufferCompatibility(config, config, true);
        }
    }
}

// Check that a context rendering to a window with a different
// config works or errors according to the EGL compatibility rules
TEST_P(EGLContextCompatibilityTest, WindowDifferentConfig)
{
    for (size_t i = 0; i < mConfigs.size(); i++)
    {
        EGLConfig config1 = mConfigs[i];
        EGLint surfaceType;
        eglGetConfigAttrib(mDisplay, config1, EGL_SURFACE_TYPE, &surfaceType);
        ASSERT_EGL_SUCCESS();

        if ((surfaceType & EGL_WINDOW_BIT) == 0)
        {
            continue;
        }

        for (size_t j = 0; j < mConfigs.size(); j++)
        {
            EGLConfig config2 = mConfigs[j];
            testWindowCompatibility(config1, config2,
                                    areConfigsCompatible(config1, config2, EGL_WINDOW_BIT));
        }
    }
}

// Check that a context rendering to a pbuffer with a different
// config works or errors according to the EGL compatibility rules
TEST_P(EGLContextCompatibilityTest, PbufferDifferentConfig)
{
    for (size_t i = 0; i < mConfigs.size(); i++)
    {
        EGLConfig config1 = mConfigs[i];
        EGLint surfaceType;
        eglGetConfigAttrib(mDisplay, config1, EGL_SURFACE_TYPE, &surfaceType);
        ASSERT_EGL_SUCCESS();

        if ((surfaceType & EGL_PBUFFER_BIT) == 0)
        {
            continue;
        }

        for (size_t j = 0; j < mConfigs.size(); j++)
        {
            EGLConfig config2 = mConfigs[j];
            testPbufferCompatibility(config1, config2, areConfigsCompatible(config1, config2, EGL_PBUFFER_BIT));
        }
    }
}

ANGLE_INSTANTIATE_TEST(EGLContextCompatibilityTest, ES2_D3D9(), ES2_D3D11(), ES2_OPENGL());
