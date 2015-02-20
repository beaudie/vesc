#include "ANGLETest.h"
#include <d3d11.h>
#include <d3d9.h>
#include "com_utils.h"

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_TYPED_TEST_CASE(QueryDisplayAttributeTest, ES2_D3D9, ES2_D3D11);

template<typename T>
class QueryDisplayAttributeTest : public ANGLETest
{
  protected:
      QueryDisplayAttributeTest() : ANGLETest(T::GetGlesMajorVersion(), T::GetPlatform())
    {
        mQueryDisplayAttribANGLE = NULL;
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        char *extensionString = (char*)eglQueryString(getEGLWindow()->getDisplay(), EGL_EXTENSIONS);
        if (strstr(extensionString, "EGL_ANGLE_query_display_attrib"))
        {
            mQueryDisplayAttribANGLE = (PFNEGLQUERYDISPLAYATTRIBANGLEPROC)eglGetProcAddress("eglQueryDisplayAttribANGLE");
        }

        if (!mQueryDisplayAttribANGLE)
        {
            FAIL() << "ANGLE extension EGL_ANGLE_query_display_attrib was not found";
        }
    }

    virtual void TearDown()
    {
        ANGLETest::TearDown();
    }

    PFNEGLQUERYDISPLAYATTRIBANGLEPROC mQueryDisplayAttribANGLE;
};

TYPED_TEST(QueryDisplayAttributeTest, QueryDevice)
{
    void *device = nullptr;
    if (getPlatformRenderer() == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        EXPECT_EQ(EGL_TRUE, mQueryDisplayAttribANGLE(getEGLWindow()->getDisplay(), EGL_D3D11_DEVICE_ANGLE, &device));
        ID3D11Device* d3d11Device = reinterpret_cast<ID3D11Device*>(device);
        IDXGIDevice*  dxgiDevice = DynamicCastComObject<IDXGIDevice>(d3d11Device);
        EXPECT_TRUE(dxgiDevice != nullptr);
        SafeRelease(dxgiDevice);
    }

    if (getPlatformRenderer() == EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE)
    {
        EXPECT_EQ(EGL_TRUE, mQueryDisplayAttribANGLE(getEGLWindow()->getDisplay(), EGL_D3D9_DEVICE_ANGLE, &device));
        IDirect3DDevice9* d3d9Device = reinterpret_cast<IDirect3DDevice9*>(device);
        IDirect3D9* d3d9 = nullptr;
        EXPECT_EQ(S_OK, d3d9Device->GetDirect3D(&d3d9));
        EXPECT_TRUE(d3d9 != nullptr);
        SafeRelease(d3d9);
    }
}
