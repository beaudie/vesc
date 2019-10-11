//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLPrintConfigTest.cpp:
//   This test prints out the configs and their attributes
//

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"

using namespace angle;

class EGLPrintEGLinfoTest : public ANGLETest
{
  public:
    EGLPrintEGLinfoTest() : mDisplay(EGL_NO_DISPLAY) {}

    void testSetUp() override
    {
        EGLint dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(), EGL_NONE};
        mDisplay           = eglGetPlatformDisplayEXT(
            EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        EXPECT_TRUE(mDisplay != EGL_NO_DISPLAY);
        EXPECT_EGL_TRUE(eglInitialize(mDisplay, nullptr, nullptr));
    }

    void testTearDown() override
    {
        if (mDisplay != EGL_NO_DISPLAY)
        {
            eglTerminate(mDisplay);
            eglReleaseThread();
        }
        ASSERT_EGL_SUCCESS() << "Error during test TearDown";
    }

    EGLDisplay mDisplay = EGL_NO_DISPLAY;
};

TEST_P(EGLPrintEGLinfoTest, PrintEGLInfo)
{
    std::cout << "    EGL Information:" << std::endl;

    const char *s_vendor = eglQueryString(mDisplay, EGL_VENDOR);
    if (s_vendor)
        std::cout << "\tVendor: " << s_vendor << std::endl;

    const char *s_version = eglQueryString(mDisplay, EGL_VERSION);
    if (s_version)
        std::cout << "\tVersion: " << s_version << std::endl;

    const char *s_clients = eglQueryString(mDisplay, EGL_CLIENT_APIS);
    if (s_clients)
        std::cout << "\tClient APIs: " << s_clients << std::endl;

    const char *s_clientExtensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (s_clientExtensions)
    {
        std::cout << "\tEGL Client Extensions:" << std::endl;
        std::istringstream ss(s_clientExtensions);
        do
        {
            std::string extension;
            ss >> extension;
            std::cout << "\t\t" << extension << std::endl;
        } while (ss);
    }

    const char *s_displayExtensions = eglQueryString(mDisplay, EGL_EXTENSIONS);
    if (s_displayExtensions)
    {
        std::cout << "\tEGL Display Extensions:" << std::endl;
        std::istringstream ss(s_displayExtensions);
        do
        {
            std::string extension;
            ss >> extension;
            std::cout << "\t\t" << extension << std::endl;
        } while (ss);
    }

    std::cout << std::endl;
}

TEST_P(EGLPrintEGLinfoTest, PrintGLInfo)
{
    // clang-format off
    EGLint count;
    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_EGL_TRUE(eglGetConfigs(mDisplay, &config, 1, &count));
    EXPECT_TRUE(count > 0);

    int clientVersion = GetParam().majorVersion;
    EGLint ctxattrs[] = {EGL_CONTEXT_CLIENT_VERSION, clientVersion, EGL_NONE};
    EGLContext context = eglCreateContext(mDisplay, config, nullptr, ctxattrs);
    EXPECT_TRUE(context != EGL_NO_CONTEXT);

    EGLint surfattrs[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE};
    EGLSurface surface = eglCreatePbufferSurface(mDisplay, config, surfattrs);
    EXPECT_TRUE(surface != EGL_NO_SURFACE);

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, surface, surface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed\n";

    std::cout << "    GLES Information:" << std::endl;

    const GLubyte *s_vendor = glGetString(GL_VENDOR);
    if (s_vendor)
        std::cout << "\tVendor: " << s_vendor << std::endl;

    const GLubyte *s_version = glGetString(GL_VERSION);
    if (s_version)
        std::cout << "\tVersion: " << s_version << std::endl;

    const GLubyte *s_renderer = glGetString(GL_RENDERER);
    if (s_renderer)
        std::cout << "\tRenderer: " << s_renderer << std::endl;

    const GLubyte *s_shader = glGetString(GL_SHADING_LANGUAGE_VERSION);
    if (s_shader)
        std::cout << "\tShader: " << s_shader << std::endl;

    const GLubyte *s_extensions = glGetString(GL_EXTENSIONS);
    if (s_extensions)
    {
        std::cout << "\tExtensions:" << std::endl;
        std::istringstream ss((const char*)s_extensions);
        do
        {
            std::string extension;
            ss >> extension;
            std::cout << "\t\t" << extension << std::endl;
        } while (ss);
    }

    // clang-format on
    std::cout << std::endl;
}

TEST_P(EGLPrintEGLinfoTest, PrintConfigInfo)
{
    // Get all the configs
    EGLint count;
    EXPECT_EGL_TRUE(eglGetConfigs(mDisplay, nullptr, 0, &count));
    EXPECT_TRUE(count > 0);
    std::vector<EGLConfig> configs(count);
    EXPECT_EGL_TRUE(eglGetConfigs(mDisplay, configs.data(), count, &count));
    configs.resize(count);
    // sort configs by increaing ID
    struct SortById
    {
        SortById(EGLDisplay inDisplay) : display(inDisplay) {}
        bool operator()(EGLConfig a, EGLConfig b) const
        {
            EGLint aId;
            EGLint bId;
            EXPECT_EGL_TRUE(eglGetConfigAttrib(display, a, EGL_CONFIG_ID, &aId));
            EXPECT_EGL_TRUE(eglGetConfigAttrib(display, b, EGL_CONFIG_ID, &bId));
            return aId < bId;
        }
        EGLDisplay display;
    };
    std::sort(configs.begin(), configs.end(), SortById(mDisplay));
    std::cout << "Configs - Count: " << count << std::endl;

    // For each config, print its attributes
    for (auto config : configs)
    {
        // clang-format off

        // Config ID
        EGLint configId = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_CONFIG_ID, &configId));
        std::cout << "    Config: " << configId << std::endl;

        // Color
        EGLint bufferSize = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_BUFFER_SIZE, &bufferSize));
        EGLint redSize = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_RED_SIZE, &redSize));
        EGLint greenSize = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_GREEN_SIZE, &greenSize));
        EGLint blueSize = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_BLUE_SIZE, &blueSize));
        EGLint lumSize = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_LUMINANCE_SIZE, &lumSize));
        EGLint alphaSize = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_ALPHA_SIZE, &alphaSize));
        EGLint alphaMaskSize = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_ALPHA_MASK_SIZE, &alphaMaskSize));
        EGLint colorComponentType;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_COLOR_COMPONENT_TYPE_EXT, &colorComponentType));
        const char * s_compType = (colorComponentType == EGL_COLOR_COMPONENT_TYPE_FLOAT_EXT) ? "Float" : "Fixed";
        EGLint colorBufType = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_COLOR_BUFFER_TYPE, &colorBufType));
        const char * s_buffType = (colorBufType == EGL_LUMINANCE_BUFFER) ? "LUMINANCE" : "RGB";
        std::cout << "\tColor:" << bufferSize << "bit " << s_compType << " " << s_buffType << " Red:" << redSize << " Green:" << greenSize \
            << " Blue:" << blueSize << " Alpha:" << alphaSize << " Lum:" << lumSize << " AlphaMask:" << alphaMaskSize << std::endl;

        // Texture Binding
        EGLint bindRGB = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_BIND_TO_TEXTURE_RGB, &bindRGB));
        EGLint bindRGBA = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_BIND_TO_TEXTURE_RGBA, &bindRGBA));
        EGLint maxPbuffWidth = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_MAX_PBUFFER_WIDTH, &maxPbuffWidth));
        EGLint maxPbuffHeight = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_MAX_PBUFFER_HEIGHT, &maxPbuffHeight));
        EGLint maxPbuffPixels = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_MAX_PBUFFER_PIXELS, &maxPbuffPixels));
        std::cout << "\tBinding RGB:" << (bool)bindRGB << " RGBA:" << (bool)bindRGBA << std::endl;

        // Conformant
        EGLint caveat = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_CONFIG_CAVEAT, &caveat));
        const char * s_caveat = nullptr;
        switch (caveat)
        {
            case EGL_NONE: s_caveat = "None."; break;
            case EGL_SLOW_CONFIG: s_caveat = "Slow."; break;
            case EGL_NON_CONFORMANT_CONFIG: s_caveat = "Non-Conformant."; break;
            default: s_caveat = ".";
        }
        EGLint conformant = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_CONFORMANT, &conformant));
        std::cout << "\tCaveate: " << s_caveat << " Conformant: ";
        if (conformant & EGL_OPENGL_BIT) std::cout << "OpenGL ";
        if (conformant & EGL_OPENGL_ES_BIT) std::cout << "ES1 ";
        if (conformant & EGL_OPENGL_ES2_BIT) std::cout << "ES2 ";
        if (conformant & EGL_OPENGL_ES3_BIT) std::cout << "ES3 ";
        std::cout << std::endl;

        // Ancilary buffers
        EGLint depthSize = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_DEPTH_SIZE, &depthSize));
        EGLint sampleBuffs = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_SAMPLE_BUFFERS, &sampleBuffs));
        EGLint samples = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_SAMPLES, &samples));
        EGLint stencilSize = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_STENCIL_SIZE, &stencilSize));
        std::cout << "\tAncilary " << "Depth:" << depthSize << " Samples:"  << samples \
            << " SampleBuffs:" << sampleBuffs << " Stencil:" << stencilSize << std::endl;

        // Swap interval
        EGLint maxSwapInt = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_MAX_SWAP_INTERVAL, &maxSwapInt));
        EGLint minSwapInt = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_MIN_SWAP_INTERVAL, &minSwapInt));
        std::cout << "\tSwap Interval Min:" << minSwapInt << " Max:" << maxSwapInt << std::endl;

        // Native
        EGLint nativeRend = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_NATIVE_RENDERABLE, &nativeRend));
        EGLint nativeVisualId = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_NATIVE_VISUAL_ID, &nativeVisualId));
        EGLint nativeVisualType = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_NATIVE_VISUAL_TYPE, &nativeVisualType));
        std::cout << "\tNative Renderable: " << nativeRend << ", VisualID: " << nativeVisualId << ", VisualType: " << nativeVisualType << std::endl;

        // Surface type
        EGLint surfaceType = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_SURFACE_TYPE, &surfaceType));
        std::cout << "\tSurface Type: ";
        if (surfaceType & EGL_WINDOW_BIT) std::cout << "WINDOW ";
        if (surfaceType & EGL_WINDOW_BIT) std::cout << "PIXMAP ";
        if (surfaceType & EGL_WINDOW_BIT) std::cout << "PBUFFER ";
        if (surfaceType & EGL_WINDOW_BIT) std::cout << "MULTISAMPLE ";
        if (surfaceType & EGL_WINDOW_BIT) std::cout << "SWAP_PRESERVE ";
        std::cout << std::endl;

        // Renderable
        EGLint rendType = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_RENDERABLE_TYPE, &rendType));
        std::cout << "\tRender: ";
        if (rendType & EGL_OPENGL_BIT) std::cout << "OpenGL ";
        if (rendType & EGL_OPENGL_ES_BIT) std::cout << "ES1 ";
        if (rendType & EGL_OPENGL_ES2_BIT) std::cout << "ES2 ";
        if (rendType & EGL_OPENGL_ES3_BIT) std::cout << "ES3 ";
        std::cout << std::endl;

        // Extensions
        EGLint recordable;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_RECORDABLE_ANDROID, &recordable));
        std::cout << "\tAndroid Recordable: " << recordable << std::endl;

        // Separator between configs
        std::cout << std::endl;

        // clang-format on
    }
}

ANGLE_INSTANTIATE_TEST(EGLPrintEGLinfoTest,
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES3_VULKAN()));
