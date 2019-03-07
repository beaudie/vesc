//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RendererTest:
//   These tests are designed to ensure that the various configurations of the test fixtures work as
//   expected. If one of these tests fails, then it is likely that some of the other tests are being
//   configured incorrectly. For example, they might be using the D3D11 renderer when the test is
//   meant to be using the D3D9 renderer.

#include "test_utils/ANGLETest.h"

#include "common/string_utils.h"

using namespace angle;

using ANGLETestParams = std::initializer_list<angle::PlatformParameters>;

namespace
{

// Helper macro for defining tests.
#define ANGLE_GTEST_TEST_(Param, TestSuiteName, TestName, parent_id)                         \
    class ANGLE_TEST_NAME(Param, TestSuiteName, TestName)                                    \
        : public ANGLE_TEST_CLASS_NAME(TestSuiteName, TestName)                              \
    {                                                                                        \
      public:                                                                                \
        ANGLE_TEST_NAME(Param, TestSuiteName, TestName)                                      \
        () : ANGLE_TEST_CLASS_NAME(TestSuiteName, TestName)(Param())                         \
        {}                                                                                   \
                                                                                             \
      private:                                                                               \
        static ::testing::TestInfo *const test_info_ GTEST_ATTRIBUTE_UNUSED_;                \
        GTEST_DISALLOW_COPY_AND_ASSIGN_(ANGLE_TEST_NAME(Param, TestSuiteName, TestName));    \
    };                                                                                       \
                                                                                             \
    ::testing::TestInfo *const ANGLE_TEST_NAME(Param, TestSuiteName, TestName)::test_info_ = \
        ::testing::internal::MakeAndRegisterTestInfo(                                        \
            #Param, #TestSuiteName "_" #TestName, nullptr, nullptr,                          \
            ::testing::internal::CodeLocation(__FILE__, __LINE__), (parent_id),              \
            ::testing::internal::SuiteApiResolver<TestSuiteName>::GetSetUpCaseOrSuite(),     \
            ::testing::internal::SuiteApiResolver<TestSuiteName>::GetTearDownCaseOrSuite(),  \
            new ::testing::internal::TestFactoryImpl<ANGLE_TEST_NAME(Param, TestSuiteName,   \
                                                                     TestName)>);

#define ANGLE_TEST_NAME(Param, TestSuiteName, TestName) \
    GTEST_TEST_CLASS_NAME_(Param, TestSuiteName##TestName)
#define ANGLE_TEST_CLASS_NAME(TestSuiteName, TestName) _##TestSuiteName##_##TestName##BaseClass

#define ANGLE_INSTANTIATE_TESTS(Param, TestSuiteName, TestName) \
    ANGLE_GTEST_TEST_(Param, TestSuiteName, TestName,           \
                      ::testing::internal::GetTypeId<TestSuiteName>())

#define ANGLE_TEST_P(TestSuiteName, TestName)                                   \
    class ANGLE_TEST_CLASS_NAME(TestSuiteName, TestName) : public TestSuiteName \
    {                                                                           \
      public:                                                                   \
        ANGLE_TEST_CLASS_NAME(TestSuiteName, TestName)                          \
        (const PlatformParameters &params) : TestSuiteName(params)              \
        {}                                                                      \
        virtual void TestBody();                                                \
    };                                                                          \
    ANGLE_ALL_TEST_CONFIGS_X(ANGLE_INSTANTIATE_TESTS, TestSuiteName, TestName)  \
    void ANGLE_TEST_CLASS_NAME(TestSuiteName, TestName)::TestBody()

class RendererTest : public ANGLETest
{
  protected:
    RendererTest(const PlatformParameters &params) : ANGLETest(params)
    {
        setWindowWidth(128);
        setWindowHeight(128);
    }
};

// Print vendor, renderer, version and extension strings. Useful for debugging.
ANGLE_TEST_P(RendererTest, Strings)
{
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Extensions: " << glGetString(GL_EXTENSIONS) << std::endl;
    EXPECT_GL_NO_ERROR();
}

ANGLE_TEST_P(RendererTest, RequestedRendererCreated)
{
    std::string rendererString =
        std::string(reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    angle::ToLower(&rendererString);

    std::string versionString =
        std::string(reinterpret_cast<const char *>(glGetString(GL_VERSION)));
    angle::ToLower(&versionString);

    if (GetParam().driver != angle::GLESDriverType::AngleEGL)
        return;

    const EGLPlatformParameters &platform = GetParam().eglParameters;

    // Ensure that the renderer string contains D3D11, if we requested a D3D11 renderer.
    if (platform.renderer == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        ASSERT_NE(rendererString.find(std::string("direct3d11")), std::string::npos);
    }

    // Ensure that the renderer string contains D3D9, if we requested a D3D9 renderer.
    if (platform.renderer == EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE)
    {
        ASSERT_NE(rendererString.find(std::string("direct3d9")), std::string::npos);
    }

    // Ensure that the major and minor versions trigger expected behavior in D3D11
    if (platform.renderer == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        // Ensure that the renderer uses WARP, if we requested it.
        if (platform.deviceType == EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_WARP_ANGLE)
        {
            auto basicRenderPos     = rendererString.find(std::string("microsoft basic render"));
            auto softwareAdapterPos = rendererString.find(std::string("software adapter"));
            ASSERT_TRUE(basicRenderPos != std::string::npos ||
                        softwareAdapterPos != std::string::npos);
        }

        std::vector<std::string> acceptableShaderModels;

        // When no specific major/minor version is requested, then ANGLE should return the highest
        // possible feature level by default. The current hardware driver might not support Feature
        // Level 11_0, but WARP always does. Therefore if WARP is specified but no major/minor
        // version is specified, then we test to check that ANGLE returns FL11_0.
        if (platform.majorVersion >= 11 || platform.majorVersion == EGL_DONT_CARE)
        {
            // Feature Level 10_0 corresponds to shader model 5_0
            acceptableShaderModels.push_back("ps_5_0");
        }

        if (platform.majorVersion >= 10 || platform.majorVersion == EGL_DONT_CARE)
        {
            if (platform.minorVersion >= 1 || platform.minorVersion == EGL_DONT_CARE)
            {
                // Feature Level 10_1 corresponds to shader model 4_1
                acceptableShaderModels.push_back("ps_4_1");
            }

            if (platform.minorVersion >= 0 || platform.minorVersion == EGL_DONT_CARE)
            {
                // Feature Level 10_0 corresponds to shader model 4_0
                acceptableShaderModels.push_back("ps_4_0");
            }
        }

        if (platform.majorVersion == 9 && platform.minorVersion == 3)
        {
            acceptableShaderModels.push_back("ps_4_0_level_9_3");
        }

        bool found = false;
        for (size_t i = 0; i < acceptableShaderModels.size(); i++)
        {
            if (rendererString.find(acceptableShaderModels[i]) != std::string::npos)
            {
                found = true;
            }
        }

        ASSERT_TRUE(found);
    }

    if (platform.renderer == EGL_PLATFORM_ANGLE_TYPE_NULL_ANGLE)
    {
        ASSERT_TRUE(IsNULL());
    }

    if (platform.renderer == EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE)
    {
        ASSERT_TRUE(IsVulkan());
    }

    EGLint glesMajorVersion = GetParam().majorVersion;
    EGLint glesMinorVersion = GetParam().minorVersion;

    // Ensure that the renderer string contains the requested version number
    if (glesMajorVersion == 3 && glesMinorVersion == 1)
    {
        ASSERT_NE(versionString.find(std::string("es 3.1")), std::string::npos);
    }
    else if (glesMajorVersion == 3 && glesMinorVersion == 0)
    {
        ASSERT_NE(versionString.find(std::string("es 3.0")), std::string::npos);
    }
    else if (glesMajorVersion == 2 && glesMinorVersion == 0)
    {
        ASSERT_NE(versionString.find(std::string("es 2.0")), std::string::npos);
    }
    else if (glesMajorVersion == 1 && glesMinorVersion == 0)
    {
        ASSERT_NE(versionString.find(std::string("es 1.0")), std::string::npos);
    }
    else
    {
        FAIL() << "Unhandled GL ES client version.";
    }

    ASSERT_GL_NO_ERROR();
    ASSERT_EGL_SUCCESS();
}

// Perform a simple operation (clear and read pixels) to verify the device is working
ANGLE_TEST_P(RendererTest, SimpleOperation)
{
    if (IsNULL() ||
        GetParam().eglParameters.deviceType == EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE)
    {
        std::cout << "ANGLE NULL backend clears are not functional" << std::endl;
        return;
    }

    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_EQ(0, 0, 0, 255, 0, 255);

    ASSERT_GL_NO_ERROR();
}
}  // anonymous namespace
