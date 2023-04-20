//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageTestMetal:
//   Tests the correctness of eglImage with native Metal texture extensions.
//

#include "test_utils/ANGLETest.h"

#include "common/mathutil.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"

#include <CoreFoundation/CoreFoundation.h>
#include <Metal/Metal.h>

namespace angle
{
namespace
{
constexpr int kTileSize                        = 32;
constexpr int kWindowWidth                     = 8 * kTileSize;
constexpr int kWindowHeight                    = kWindowWidth;
constexpr int kZones                           = 7;
constexpr float kQuality[kZones]               = {0.24f, 0.24f, 0.24f, 0.24f, 0.49f, 0.49f, 0.49f};
constexpr int kViewportWidth                   = 3 * kTileSize;
constexpr int kViewportHeight                  = 3 * kTileSize;
constexpr char kVariableRasterizationRateExt[] = "GL_ANGLE_rasterization_rate_map_metal";
constexpr char kVS[]                           = R"(precision highp float;
attribute vec4 position;
varying vec2 texcoord;

void main()
{
    gl_Position = position;
    texcoord = (position.xy * 0.5) + 0.5;
})";
constexpr char kCheckerBoardFS[]               = R"(
precision highp float;
varying vec2 texcoord;
uniform vec2 u_resolution;
uniform float u_checkCount;

void main()
{
    vec2 uv = texcoord * u_checkCount;

    float u = 1.0 - floor(mod(uv.x, 2.0));
    float v = 1.0 - floor(mod(uv.y, 2.0));
    vec3 col = vec3(1.0);

    if(u == 1.0 && v < 1.0 || v == 1.0 && u < 1.0) {
        col = vec3(0.);
    }
    gl_FragColor = vec4(col,1.0);
})";
}  // anonymous namespace

class ScopeMetalRasterizationRateMapRef : angle::NonCopyable
{
  public:
    explicit ScopeMetalRasterizationRateMapRef(id<MTLRasterizationRateMap> &&map) : mMap(map) {}

    ~ScopeMetalRasterizationRateMapRef()
    {
        if (mMap)
        {
            release();
            mMap = nil;
        }
    }

    operator GLMTLRasterizationRateMapANGLE() const
    {
        return reinterpret_cast<GLMTLRasterizationRateMapANGLE>(mMap);
    }

    NSUInteger retainCount() const { return mMap.retainCount; }

    NSUInteger layerCount() const { return mMap.layerCount; }
    MTLSize screenSize() const { return mMap.screenSize; }
    MTLSize physicalGranularity() const { return mMap.physicalGranularity; }
    MTLSize viewportSize() const { return [mMap physicalSizeForLayer:0]; }
    MTLCoordinate2D mapScreenToView(MTLCoordinate2D screenCoord)
    {
        return [mMap mapScreenToPhysicalCoordinates:screenCoord forLayer:0];
    }
    MTLCoordinate2D mapViewToScreen(MTLCoordinate2D viewCoord)
    {
        return [mMap mapPhysicalToScreenCoordinates:viewCoord forLayer:0];
    }

  private:
    void release()
    {
#if !__has_feature(objc_arc)
        [mMap release];
#endif
    }

    id<MTLRasterizationRateMap> mMap = nil;
};

ScopeMetalRasterizationRateMapRef CreateMetalRasterizationRateMap(id<MTLDevice> deviceMtl,
                                                                  int width,
                                                                  int height,
                                                                  int horizontalCount,
                                                                  const float *horizontal,
                                                                  int verticalCount,
                                                                  const float *vertical)
{
    @autoreleasepool
    {
        MTLRasterizationRateLayerDescriptor *rrLayer = [[MTLRasterizationRateLayerDescriptor alloc]
            initWithSampleCount:MTLSizeMake(horizontalCount, verticalCount, 0)
                     horizontal:horizontal
                       vertical:vertical];
        MTLRasterizationRateMapDescriptor *rrDesc    = [MTLRasterizationRateMapDescriptor
            rasterizationRateMapDescriptorWithScreenSize:MTLSizeMake(width, height, 0)
                                                   layer:rrLayer];
        rrDesc.label                                 = @"Test RasterizationRate";

        return ScopeMetalRasterizationRateMapRef(
            [deviceMtl newRasterizationRateMapWithDescriptor:rrDesc]);
    }
}

class RasterizationRateMapTestMetal : public ANGLETest<>
{
  protected:
    RasterizationRateMapTestMetal()
    {
        setWindowWidth(kWindowWidth);
        setWindowHeight(kWindowHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void testSetUp() override
    {
        mProgram = CompileProgram(kVS, kCheckerBoardFS);
        if (!mProgram)
        {
            FAIL() << "shader compilation failed.";
        }
        mUniformResolutionLocation = glGetUniformLocation(mProgram, "u_resolution");
        mUniformCheckCountLocation = glGetUniformLocation(mProgram, "u_checkCount");

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override {}

    id<MTLDevice> getMtlDevice()
    {
        EGLAttrib angleDevice = 0;
        EGLAttrib device      = 0;
        EXPECT_EGL_TRUE(
            eglQueryDisplayAttribEXT(getEGLWindow()->getDisplay(), EGL_DEVICE_EXT, &angleDevice));

        EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(angleDevice),
                                                EGL_METAL_DEVICE_ANGLE, &device));

        return (__bridge id<MTLDevice>)reinterpret_cast<void *>(device);
    }

    template <size_t W, size_t H>
    ScopeMetalRasterizationRateMapRef createMetalRasterizationRateMap(int width,
                                                                      int height,
                                                                      const float (&horizontal)[W],
                                                                      const float (&vertical)[H])
    {
        id<MTLDevice> device = getMtlDevice();

        return CreateMetalRasterizationRateMap(device, width, height, W, horizontal, H, vertical);
    }

    GLuint mProgram;
    GLint mUniformResolutionLocation;
    GLint mUniformCheckCountLocation;
};

TEST_P(RasterizationRateMapTestMetal, Binding)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kVariableRasterizationRateExt));

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(glGetFramebufferMTLRasterizationRateMapANGLE(GL_FRAMEBUFFER), nullptr);

    auto rateMapMtl =
        createMetalRasterizationRateMap(kWindowWidth, kWindowHeight, kQuality, kQuality);
    EXPECT_EQ(rateMapMtl.retainCount(), 1ul);

    glFramebufferMTLRasterizationRateMapANGLE(GL_FRAMEBUFFER, rateMapMtl);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(rateMapMtl.retainCount(), 2ul);

    EXPECT_EQ(glGetFramebufferMTLRasterizationRateMapANGLE(GL_FRAMEBUFFER), rateMapMtl);
}

// Test that the rasterization rate map affects the rendering to fbo bound
// attachments.
TEST_P(RasterizationRateMapTestMetal, Rendering)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kVariableRasterizationRateExt));
    EXPECT_GL_NO_ERROR();

    auto rateMapMtl =
        createMetalRasterizationRateMap(kWindowWidth, kWindowHeight, kQuality, kQuality);
    auto screenSize = rateMapMtl.screenSize();
    EXPECT_EQ(screenSize.width, static_cast<NSUInteger>(kWindowWidth));
    EXPECT_EQ(screenSize.height, static_cast<NSUInteger>(kWindowHeight));
    auto viewportSize = rateMapMtl.viewportSize();
    EXPECT_EQ(viewportSize.width, static_cast<NSUInteger>(kViewportWidth));
    EXPECT_EQ(viewportSize.height, static_cast<NSUInteger>(kViewportHeight));

    GLTexture rbColor;
    glBindTexture(GL_TEXTURE_2D, rbColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewportSize.width, viewportSize.height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rbColor, 0);
    EXPECT_GL_NO_ERROR();

    glFramebufferMTLRasterizationRateMapANGLE(GL_FRAMEBUFFER, rateMapMtl);
    EXPECT_GL_NO_ERROR();

    glScissor(0, 0, screenSize.width, screenSize.height);
    glViewport(0, 0, screenSize.width, screenSize.height);

    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glUseProgram(mProgram);
    if (mUniformResolutionLocation != -1)
        glUniform2f(mUniformResolutionLocation, viewportSize.width, viewportSize.height);
    if (mUniformCheckCountLocation != -1)
        glUniform1f(mUniformCheckCountLocation, 9.0f);
    drawQuad(mProgram, "position", 0.5f);

    // Sample a selection of pixels to confirm the expected warped checkerboard rendering
    for (int o : std::array{6, 38, 67})
    {
        EXPECT_PIXEL_COLOR_NEAR(o + 0, o + 0, GLColor::white, 1.0);
        EXPECT_PIXEL_COLOR_NEAR(o + 1, o + 0, GLColor::black, 1.0);
        EXPECT_PIXEL_COLOR_NEAR(o + 0, o + 1, GLColor::black, 1.0);
        EXPECT_PIXEL_COLOR_NEAR(o + 1, o + 1, GLColor::white, 1.0);
    }
}

// Test that rasterization rate map only affects the rendering to attachments
// that it's bound to.
TEST_P(RasterizationRateMapTestMetal, MapAffectsBoundFBOOnly)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kVariableRasterizationRateExt));
    EXPECT_GL_NO_ERROR();

    auto rateMapMtl =
        createMetalRasterizationRateMap(kWindowWidth, kWindowHeight, kQuality, kQuality);
    auto screenSize = rateMapMtl.screenSize();
    EXPECT_EQ(screenSize.width, static_cast<NSUInteger>(kWindowWidth));
    EXPECT_EQ(screenSize.height, static_cast<NSUInteger>(kWindowHeight));
    auto viewportSize = rateMapMtl.viewportSize();
    EXPECT_EQ(viewportSize.width, static_cast<NSUInteger>(kViewportWidth));
    EXPECT_EQ(viewportSize.height, static_cast<NSUInteger>(kViewportHeight));

    GLTexture rbColor;
    glBindTexture(GL_TEXTURE_2D, rbColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewportSize.width, viewportSize.height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fboWithMap;
    glBindFramebuffer(GL_FRAMEBUFFER, fboWithMap);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rbColor, 0);
    glFramebufferMTLRasterizationRateMapANGLE(GL_FRAMEBUFFER, rateMapMtl);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fboWithoutMap;
    glBindFramebuffer(GL_FRAMEBUFFER, fboWithoutMap);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rbColor, 0);
    EXPECT_GL_NO_ERROR();

    glScissor(0, 0, viewportSize.width, viewportSize.height);
    glViewport(0, 0, viewportSize.width, viewportSize.height);

    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glUseProgram(mProgram);
    if (mUniformResolutionLocation != -1)
        glUniform2f(mUniformResolutionLocation, viewportSize.width, viewportSize.height);
    if (mUniformCheckCountLocation != -1)
        glUniform1f(mUniformCheckCountLocation, 9.0f);
    drawQuad(mProgram, "position", 0.5f);

    // Sample a selection of pixels to confirm the expected unwarped checkerboard rendering
    for (int o : std::array{20, 52, 74})
    {
        EXPECT_PIXEL_COLOR_NEAR(o + 0, o + 0, GLColor::white, 1.0);
        EXPECT_PIXEL_COLOR_NEAR(o + 1, o + 0, GLColor::black, 1.0);
        EXPECT_PIXEL_COLOR_NEAR(o + 0, o + 1, GLColor::black, 1.0);
        EXPECT_PIXEL_COLOR_NEAR(o + 1, o + 1, GLColor::white, 1.0);
    }
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(RasterizationRateMapTestMetal, ES2_METAL(), ES3_METAL());

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(RasterizationRateMapTestMetal);
}  // namespace angle
