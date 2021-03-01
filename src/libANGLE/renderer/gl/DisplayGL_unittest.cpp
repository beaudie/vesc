//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "libANGLE/Display.h"
#include "libANGLE/renderer/gl/DisplayGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/RendererGL.h"
#include "tests/angle_unittests_utils.h"

using namespace rx;
using namespace testing;

namespace
{

class MockFunctionsGL : public rx::FunctionsGL
{
  public:
    MockFunctionsGL() : FunctionsGL() {}
    virtual ~MockFunctionsGL() { destructor(); }

    MOCK_METHOD1(getString, const GLubyte *(GLenum));
    MOCK_METHOD0(destructor, void());

    // FunctionsGL pure virtual functions
    void *loadProcAddress(const std::string &function) const override { return nullptr; }
};

std::unique_ptr<FunctionsGL> g_functions(new MockFunctionsGL());

class MockRendererGL : public rx::RendererGL
{
  public:
    MockRendererGL(DisplayGL *display)
        : RendererGL(std::move(g_functions), egl::AttributeMap(), display)
    {}
    virtual ~MockRendererGL() { destructor(); }

    MOCK_METHOD0(destructor, void());

    // RendererGL pure virtual functions
    WorkerContext *createWorkerContext(std::string *infoLog) override { return nullptr; }
};

class MockDisplayGL : public rx::DisplayGL
{
  public:
    MockDisplayGL() : DisplayGL(mockState), mockState(0), mRenderer(new MockRendererGL(this)) {}
    virtual ~MockDisplayGL() { destructor(); }

    MOCK_METHOD0(destructor, void());

    // Replace with mock renderer
    RendererGL *getRenderer() const override { return mRenderer.get(); }

    // EGLImplFactory pure virtual functions
    SurfaceImpl *createWindowSurface(const egl::SurfaceState &state,
                                     EGLNativeWindowType window,
                                     const egl::AttributeMap &attribs) override
    {
        return nullptr;
    }
    SurfaceImpl *createPbufferSurface(const egl::SurfaceState &state,
                                      const egl::AttributeMap &attribs) override
    {
        return nullptr;
    }
    SurfaceImpl *createPbufferFromClientBuffer(const egl::SurfaceState &state,
                                               EGLenum buftype,
                                               EGLClientBuffer clientBuffer,
                                               const egl::AttributeMap &attribs) override
    {
        return nullptr;
    }
    SurfaceImpl *createPixmapSurface(const egl::SurfaceState &state,
                                     NativePixmapType nativePixmap,
                                     const egl::AttributeMap &attribs) override
    {
        return nullptr;
    }
    ImageImpl *createImage(const egl::ImageState &state,
                           const gl::Context *context,
                           EGLenum target,
                           const egl::AttributeMap &attribs) override
    {
        return nullptr;
    }
    ContextImpl *createContext(const gl::State &state,
                               gl::ErrorSet *errorSet,
                               const egl::Config *configuration,
                               const gl::Context *shareContext,
                               const egl::AttributeMap &attribs) override
    {
        return nullptr;
    }
    StreamProducerImpl *createStreamProducerD3DTexture(egl::Stream::ConsumerType consumerType,
                                                       const egl::AttributeMap &attribs) override
    {
        return nullptr;
    }
    ShareGroupImpl *createShareGroup() override { return nullptr; }

    // DisplayImpl pure virtual functions
    egl::ConfigSet generateConfigs() override { return egl::ConfigSet(); }
    bool testDeviceLost() override { return false; }
    egl::Error restoreLostDevice(const egl::Display *display) override { return egl::NoError(); }
    bool isValidNativeWindow(EGLNativeWindowType window) const override { return true; }
    egl::Error waitClient(const gl::Context *context) override { return egl::NoError(); }
    egl::Error waitNative(const gl::Context *context, EGLint engine) override
    {
        return egl::NoError();
    }
    gl::Version getMaxSupportedESVersion() const override { return gl::Version(); }
    gl::Version getMaxConformantESVersion() const override { return gl::Version(); }
    void populateFeatureList(angle::FeatureList *features) override {}
    void generateCaps(egl::Caps *outCaps) const override {}

    // Members
    egl::DisplayState mockState;
    std::shared_ptr<MockRendererGL> mRenderer;
};

TEST(DisplayGLTest, Test)
{
    MockDisplayGL *display   = new MockDisplayGL();
    MockRendererGL *renderer = reinterpret_cast<MockRendererGL *>(display->getRenderer());
    MockFunctionsGL *functions =
        reinterpret_cast<MockFunctionsGL *>(const_cast<FunctionsGL *>(renderer->getFunctions()));

    const unsigned char test[100] = "test";
    EXPECT_CALL(*functions, getString(_)).WillOnce(Return(test));

    EXPECT_EQ(display->getRendererDescription(), "test");

    // Only needed because the mock is leaked if bugs are present,
    // which logs an error, but does not cause the test to fail.
    // Ordinarily mocks are verified when destroyed.
    // Mock::VerifyAndClear(functions);
    Mock::VerifyAndClear(renderer);
    Mock::VerifyAndClear(display);
}

}  // namespace
