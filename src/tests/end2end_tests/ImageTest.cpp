#include "ANGLETest.h"

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_TYPED_TEST_CASE(ImageTest, ES2_D3D9, ES2_D3D11, ES2_OPENGL);

template<typename T>
class ImageTest : public ANGLETest
{
protected:
    ImageTest() : ANGLETest(T::GetGlesMajorVersion(), T::GetPlatform())
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();
    }

    void TearDown() override
    {
        ANGLETest::TearDown();
    }
};

TYPED_TEST(ImageTest, NULL)
{
}
