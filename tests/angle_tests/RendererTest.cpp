#include "ANGLETest.h"

// These tests are designed to ensure that the various configurations of the test fixtures work as expected.
// If one of these tests fails, then it is likely that some of the other tests are being configured incorrectly.
// For example, they might be using the D3D11 renderer when the test is meant to be using the D3D9 renderer.

// We also run run this test against some additional test fixture types, which request specific feature levels of the D3D11 renderer.

struct D3D11_FL11_0
{
    static const EGLint *GetDisplayAttributes()
    {
        static const EGLint displayAttributes[] =
        {
            EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
            EGL_PLATFORM_ANGLE_VERSION_MAJOR, 11,
            EGL_PLATFORM_ANGLE_VERSION_MINOR, 0,
            EGL_NONE,
        };
        return &(displayAttributes[0]);
    };
};

struct D3D11_FL10
{
    static const EGLint *GetDisplayAttributes()
    {
        static const EGLint displayAttributes[] =
        {
            EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
            EGL_PLATFORM_ANGLE_VERSION_MAJOR, 10,
            EGL_NONE,
        };
        return &(displayAttributes[0]);
    };
};

struct D3D11_FL10_1
{
    static const EGLint *GetDisplayAttributes()
    {
        static const EGLint displayAttributes[] =
        {
            EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
            EGL_PLATFORM_ANGLE_VERSION_MAJOR, 10,
            EGL_PLATFORM_ANGLE_VERSION_MINOR, 1,
            EGL_NONE,
        };
        return &(displayAttributes[0]);
    };
};

struct D3D11_FL10_0
{
    static const EGLint *GetDisplayAttributes()
    {
        static const EGLint displayAttributes[] =
        {
            EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
            EGL_PLATFORM_ANGLE_VERSION_MAJOR, 10,
            EGL_PLATFORM_ANGLE_VERSION_MINOR, 0,
            EGL_NONE,
        };
        return &(displayAttributes[0]);
    };
};

struct D3D11_FL10_0_WARP
{
    static const EGLint *GetDisplayAttributes()
    {
        static const EGLint displayAttributes[] =
        {
            EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
            EGL_PLATFORM_ANGLE_VERSION_MAJOR, 10,
            EGL_PLATFORM_ANGLE_VERSION_MINOR, 0,
            EGL_PLATFORM_ANGLE_D3D11_USE_WARP, EGL_TRUE,
            EGL_NONE,
        };
        return &(displayAttributes[0]);
    };
};

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
typedef ::testing::Types<TFT<Gles::Three, Rend::D3D11>,       TFT<Gles::Two, Rend::D3D11>,
                         TFT<Gles::Three, D3D11_FL11_0>,      TFT<Gles::Two, D3D11_FL11_0>,
                         TFT<Gles::Three, D3D11_FL10>,        TFT<Gles::Two, D3D11_FL10>,
                         TFT<Gles::Three, D3D11_FL10_1>,      TFT<Gles::Two, D3D11_FL10_1>,
                         TFT<Gles::Three, D3D11_FL10_0>,      TFT<Gles::Two, D3D11_FL10_0>,
                         TFT<Gles::Three, D3D11_FL10_0_WARP>, TFT<Gles::Two, D3D11_FL10_0_WARP>,
                         TFT<Gles::Three, Rend::WARP>,        TFT<Gles::Two, Rend::WARP>,
                                                              TFT<Gles::Two, Rend::D3D9>  > TestFixtureTypes;
TYPED_TEST_CASE(RendererTest, TestFixtureTypes);

template<typename T>
class RendererTest : public ANGLETest
{
  protected:
    RendererTest() : ANGLETest(T::GetGlesMajorVersion(), T::GetDisplayAttributes())
    {
        setWindowWidth(128);
        setWindowHeight(128);

        mGlesMajorVersion = T::GetGlesMajorVersion();
        const EGLint *displayAttributes = T::GetDisplayAttributes();
        
        mRenderer = EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE;
        mHasMajorVersion = false;
        mHasMinorVersion = false;
        mMajorVersion = 0;
        mMinorVersion = 0;
        mWARP = false;

        if (displayAttributes)
        {
            for (const EGLint *curAttrib = displayAttributes; curAttrib[0] != EGL_NONE; curAttrib += 2)
            {
                switch (curAttrib[0])
                {
                  case EGL_PLATFORM_ANGLE_TYPE_ANGLE:
                    mRenderer = curAttrib[1];
                    break;
    
                  case EGL_PLATFORM_ANGLE_VERSION_MAJOR:
                    mHasMajorVersion = true;
                    mMajorVersion = curAttrib[1];
                    break;

                  case EGL_PLATFORM_ANGLE_VERSION_MINOR:
                    mHasMinorVersion = true;
                    mMinorVersion = curAttrib[1];
                    break;

                  case EGL_PLATFORM_ANGLE_D3D11_USE_WARP:
                    mWARP = true;
                    break;

                  default:
                    break;
                }
            }
        }
    }
    
    EGLint mGlesMajorVersion;
    EGLint mRenderer;
    bool mHasMajorVersion;
    bool mHasMinorVersion;
    EGLint mMajorVersion;
    EGLint mMinorVersion;
    bool mWARP;
};

TYPED_TEST(RendererTest, RequestedRendererCreated)
{
    std::string rendererString = std::string(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    std::transform(rendererString.begin(), rendererString.end(), rendererString.begin(), ::tolower);

    std::string versionString = std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    std::transform(versionString.begin(), versionString.end(), versionString.begin(), ::tolower);

    // Ensure that the renderer string contains D3D11, if we requested a D3D11 renderer.
    if (mRenderer == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        ASSERT_NE(rendererString.find(std::string("direct3d11")), std::string::npos);
    }

    // Ensure that the renderer string contains D3D9, if we requested a D3D9 renderer.
    if (mRenderer == EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE)
    {
        ASSERT_NE(rendererString.find(std::string("direct3d9")), std::string::npos);
    }

    // Ensure that the renderer uses WARP, if we requested it.
    if (mWARP)
    {
        auto basicRenderPos = rendererString.find(std::string("microsoft basic render"));
        auto softwareAdapterPos = rendererString.find(std::string("software adapter"));
        ASSERT_TRUE(basicRenderPos != std::string::npos || softwareAdapterPos != std::string::npos);
    }

    // Ensure that the major and minor versions trigger expected behavior in D3D11
    if (mRenderer == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        // When no specific major/minor version is requested, then ANGLE should return the highest possible feature level by default.
        // The current hardware driver might not support Feature Level 11_0, but WARP always does.
        // Therefore if WARP is specified but no major/minor version is specified, then we test to check that ANGLE returns FL11_0.
        if (mHasMajorVersion || (!mHasMajorVersion && mWARP))
        {
            if (mMajorVersion == 11 || (!mHasMajorVersion && mWARP))
            {
                // Feature Level 11 corresponds to shader model 5_0
                ASSERT_NE(rendererString.find(std::string("ps_5_0")), std::string::npos);
            }
            else if (mMajorVersion == 10)
            {
                if (!mHasMinorVersion || mMinorVersion == 1)
                {
                    // Feature Level 10_1 corresponds to shader model 4_1
                    ASSERT_NE(rendererString.find(std::string("ps_4_1")), std::string::npos);
                }
                else if (mMinorVersion == 0)
                {
                    // Feature Level 10_0 corresponds to shader model 4_0
                    ASSERT_NE(rendererString.find(std::string("ps_4_0")), std::string::npos);
                }
                else
                {
                    GTEST_FATAL_FAILURE_("Unexpected requested major and minor versions for egl display");
                }
            }
        }
    }

    // Ensure that the renderer string contains GL ES 3.0, if we requested a GL ES 3.0
    if (mGlesMajorVersion == 3)
    {
        ASSERT_NE(versionString.find(std::string("es 3.0")), std::string::npos);
    }

    // Ensure that the version string contains GL ES 2.0, if we requested GL ES 2.0
    if (mGlesMajorVersion == 2)
    {
        ASSERT_NE(versionString.find(std::string("es 2.0")), std::string::npos);
    }
}
