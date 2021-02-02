//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "ANGLEPerfTest.h"

#include "common/matrix_utils.h"
#include "common/vector_utils.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_platform.h"
#include "util/gles_loader_autogen.h"
#include "util/shader_utils.h"

namespace
{
constexpr unsigned int kIterationsPerStep = 1;
constexpr unsigned int kTextureWidth      = 1280;
constexpr unsigned int kTextureHeight     = 720;

using angle::Vector3;
using angle::Mat4;

const std::array<Vector3, 6> kQuadVertices = {{
    Vector3(-1.0f, 1.0f, 0.5f),
    Vector3(-1.0f, -1.0f, 0.5f),
    Vector3(1.0f, -1.0f, 0.5f),
    Vector3(-1.0f, 1.0f, 0.5f),
    Vector3(1.0f, -1.0f, 0.5f),
    Vector3(1.0f, 1.0f, 0.5f),
}};

constexpr float Kr = 0.2126f;
constexpr float Kb = 0.0722f;
constexpr float Kg = 1.0f - Kr - Kb;
constexpr float u_m = 0.5f / (1.0f - Kb);
constexpr float v_m = 0.5f / (1.0f - Kr);

constexpr float scale_y = 255.0f / 219.0f;
constexpr float scale_uv = 255.0f / 224.0f;
constexpr float translate_y = -16.0f / 219.0f;
constexpr float translate_uv = -16.0f / 224.0f;

const Mat4 kRec709RGBToYUVMatrix(
                 Kr,        Kg,                Kb, 0.0f,  // Y
          u_m * -Kr, u_m * -Kg, u_m * (1.0f - Kb), 0.5f,  // U
  v_m * (1.0f - Kr), v_m * -Kg,         v_m * -Kb, 0.5f,  // V
               0.0f,      0.0f,              0.0f, 1.0f
);

const Mat4 kRangeAdjustMatrix(
    scale_y, 0.0f,     0.0f,     translate_y,
    0.0f,    scale_uv, 0.0f,     translate_uv,
    0.0f,    0.0f,     scale_uv, translate_uv,
    0.0f,    0.0f,     0.0f,     1.0f
);

constexpr char kVertexShader[] =
    R"(precision highp float;
        attribute vec4 position;
        varying vec2 texcoord;

        void main()
        {
            gl_Position = position;
            texcoord = (position.xy * 0.5) + 0.5;
            texcoord.y = 1.0 - texcoord.y;
        })";

constexpr char kRGBFragmentShader[] =
    R"(precision mediump float;
       uniform sampler2D tex;
       varying vec2 texcoord;

       void main()
       {
            gl_FragColor = texture2D(tex, texcoord);
       })";

constexpr char kYUVFragmentShader[] =
    R"(#extension GL_OES_EGL_image_external : require
        precision mediump float;
        uniform samplerExternalOES texY;
        uniform samplerExternalOES texUV;
        uniform mat4 rec709YUVToRGB;
        varying vec2 texcoord;

        void main()
        {
            float y = texture2D(texY, texcoord).r;
            vec2 uv = texture2D(texUV, texcoord).rg;
            vec4 yuv = vec4(y, uv.r, uv.g, 1.0);
            gl_FragColor = yuv * rec709YUVToRGB;
        })";

struct TestParams final : public RenderTestParams
{
    TestParams(int passes)
    {
        eglParameters                    = angle::egl_platform::D3D11();
        // eglParameters.debugLayersEnabled = true;
        iterationsPerStep                = kIterationsPerStep;
        windowWidth                      = 256;
        windowHeight                     = 256;
        surfaceType                      = SurfaceType::Offscreen;
        numPasses                        = passes;
    }

    std::string story() const override
    {
        std::stringstream storyStr;
        storyStr << RenderTestParams::story();
        if (numPasses > 1)
        {
            storyStr << "_" << numPasses << "_passes";
        }
        return storyStr.str();
    }

    unsigned int numPasses = 1;
};

std::ostream &operator<<(std::ostream &os, const TestParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

class D3D11YUVTexturePerf : public ANGLERenderTest, public ::testing::WithParamInterface<TestParams>
{
  public:
    D3D11YUVTexturePerf(const std::string &testName) : ANGLERenderTest(testName, GetParam()) {}

    void initializeBenchmark() override;
    void destroyBenchmark() override;

  protected:
    void initializeYUVProgram();
    void initializeRGBProgram();

    unsigned int numPasses = 1;

    EGLDisplay mDisplay = nullptr;

    Microsoft::WRL::ComPtr<ID3D11Device> mD3D11Device;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> mD3D11Texture;

    GLuint mYUVProgram = 0;
    GLuint mRGBProgram = 0;

    GLuint mFramebuffer  = 0;
    GLuint mRenderbuffer = 0;

    EGLImage mImageY  = nullptr;
    EGLImage mImageUV = nullptr;

    GLuint mTextureY  = 0;
    GLuint mTextureUV = 0;
};

void D3D11YUVTexturePerf::initializeBenchmark()
{
    numPasses = GetParam().numPasses;

    mDisplay = static_cast<EGLWindow *>(getGLWindow())->getDisplay();

    EGLAttrib result = 0;
    ASSERT_TRUE(IsEGLClientExtensionEnabled("EGL_EXT_device_query"));
    ASSERT_EGL_TRUE(eglQueryDisplayAttribEXT(mDisplay, EGL_DEVICE_EXT, &result));

    EGLDeviceEXT device = reinterpret_cast<EGLDeviceEXT>(result);

    ASSERT_TRUE(IsEGLDeviceExtensionEnabled(device, "EGL_ANGLE_device_d3d"));
    ASSERT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_D3D11_DEVICE_ANGLE, &result));

    mD3D11Device = reinterpret_cast<ID3D11Device *>(result);

    ASSERT_TRUE(mD3D11Device != nullptr);
    CD3D11_TEXTURE2D_DESC desc(DXGI_FORMAT_NV12, kTextureWidth, kTextureHeight, 1, 1,
                               D3D11_BIND_DECODER | D3D11_BIND_SHADER_RESOURCE);

    const unsigned char kYUVFillValue = 160;
    std::vector<unsigned char> imageData(kTextureWidth * kTextureHeight * 3 / 2, kYUVFillValue);

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem                = static_cast<const void *>(imageData.data());
    data.SysMemPitch            = kTextureWidth;

    ASSERT_TRUE(SUCCEEDED(mD3D11Device->CreateTexture2D(&desc, &data, &mD3D11Texture)));

    glGenRenderbuffers(1, &mRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, kTextureWidth, kTextureHeight);
    ASSERT_GL_NO_ERROR();

    glGenFramebuffers(1, &mFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mRenderbuffer);
    ASSERT_GLENUM_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER), GL_FRAMEBUFFER_COMPLETE);
    ASSERT_GL_NO_ERROR();

    glViewport(0, 0, kTextureWidth, kTextureHeight);
}

void D3D11YUVTexturePerf::initializeRGBProgram()
{
    mRGBProgram = CompileProgram(kVertexShader, kRGBFragmentShader);
    ASSERT_NE(0u, mRGBProgram) << "shader compilation failed.";

    glUseProgram(mRGBProgram);

    GLint positionLocation = glGetAttribLocation(mRGBProgram, "position");
    ASSERT_NE(-1, positionLocation);

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, kQuadVertices.data());
    glEnableVertexAttribArray(positionLocation);

    GLint textureLocation = glGetUniformLocation(mRGBProgram, "tex");
    ASSERT_NE(-1, textureLocation);

    glUniform1i(textureLocation, 0);
}

void D3D11YUVTexturePerf::initializeYUVProgram()
{
    const EGLint yAttribs[] = {EGL_D3D11_TEXTURE_PLANE_ANGLE, 0, EGL_TEXTURE_INTERNAL_FORMAT_ANGLE,
                               GL_R8_EXT, EGL_NONE};
    mImageY                 = eglCreateImageKHR(mDisplay, EGL_NO_CONTEXT, EGL_D3D11_TEXTURE_ANGLE,
                                static_cast<EGLClientBuffer>(mD3D11Texture.Get()), yAttribs);
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(mImageY, EGL_NO_IMAGE_KHR);

    // Create and bind Y plane texture to image.
    glGenTextures(1, &mTextureY);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTextureY);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    ASSERT_GL_NO_ERROR();

    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, mImageY);
    ASSERT_GL_NO_ERROR();

    const EGLint uvAttribs[] = {EGL_D3D11_TEXTURE_PLANE_ANGLE, 1, EGL_TEXTURE_INTERNAL_FORMAT_ANGLE,
                                GL_RG8_EXT, EGL_NONE};
    mImageUV                 = eglCreateImageKHR(mDisplay, EGL_NO_CONTEXT, EGL_D3D11_TEXTURE_ANGLE,
                                 static_cast<EGLClientBuffer>(mD3D11Texture.Get()), uvAttribs);
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(mImageUV, EGL_NO_IMAGE_KHR);

    // Create and bind UV plane texture to image.
    glGenTextures(1, &mTextureUV);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTextureUV);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    ASSERT_GL_NO_ERROR();

    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, mImageUV);
    ASSERT_GL_NO_ERROR();

    mYUVProgram = CompileProgram(kVertexShader, kYUVFragmentShader);
    ASSERT_NE(0u, mYUVProgram) << "shader compilation failed.";

    glUseProgram(mYUVProgram);

    GLint positionLocation = glGetAttribLocation(mYUVProgram, "position");
    ASSERT_NE(-1, positionLocation);

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, kQuadVertices.data());
    glEnableVertexAttribArray(positionLocation);

    GLint textureLocationY = glGetUniformLocation(mYUVProgram, "texY");
    ASSERT_NE(-1, textureLocationY);

    GLint textureLocationUV = glGetUniformLocation(mYUVProgram, "texUV");
    ASSERT_NE(-1, textureLocationUV);

    glUniform1i(textureLocationY, 0);
    glUniform1i(textureLocationUV, 1);
    ASSERT_GL_NO_ERROR();

    GLint rec709YUVToRGBLocation = glGetUniformLocation(mYUVProgram, "rec709YUVToRGB");
    ASSERT_NE(-1, rec709YUVToRGBLocation);

    const Mat4 kRec709YUVToRGBMatrix = kRangeAdjustMatrix.product(kRec709RGBToYUVMatrix.inverse());
    glUniformMatrix4fv(rec709YUVToRGBLocation, 1, GL_FALSE, kRec709YUVToRGBMatrix.constData());
}

void D3D11YUVTexturePerf::destroyBenchmark()
{
    glDeleteFramebuffers(1, &mFramebuffer);
    glDeleteRenderbuffers(1, &mRenderbuffer);

    if (mYUVProgram)
    {
        glDeleteProgram(mYUVProgram);
        glDeleteTextures(1, &mTextureY);
        glDeleteTextures(1, &mTextureUV);

        eglDestroyImageKHR(mDisplay, mImageY);
        eglDestroyImageKHR(mDisplay, mImageUV);
    }

    if (mRGBProgram)
        glDeleteProgram(mRGBProgram);

    mD3D11Texture.Reset();
    mD3D11Device.Reset();
}

class D3D11YUVTextureSamplerPerf : public D3D11YUVTexturePerf

{
  public:
    D3D11YUVTextureSamplerPerf() : D3D11YUVTexturePerf("D3D11YUVTextureSamplerPerf") {}

    void initializeBenchmark() override;
    void drawBenchmark() override;
};

void D3D11YUVTextureSamplerPerf::initializeBenchmark()
{
    D3D11YUVTexturePerf::initializeBenchmark();
    D3D11YUVTexturePerf::initializeYUVProgram();

    drawBenchmark();

    EXPECT_PIXEL_EQ(static_cast<GLint>(kTextureWidth) / 2, static_cast<GLint>(kTextureHeight) / 2, 225,
                    144, 235, 255);
    ASSERT_GL_NO_ERROR();
}

void D3D11YUVTextureSamplerPerf::drawBenchmark()
{
    glUseProgram(mYUVProgram);
    for (unsigned int i = 0; i < numPasses; i++)
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glFlush();
}

TEST_P(D3D11YUVTextureSamplerPerf, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(D3D11YUVTextureSamplerPerf, TestParams(1), TestParams(2), TestParams(3));

class D3D11YUVTextureShaderCopyPerf : public D3D11YUVTexturePerf
{
  public:
    D3D11YUVTextureShaderCopyPerf() : D3D11YUVTexturePerf("D3D11YUVTextureShaderCopyPerf") {}

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  private:
    GLuint mCopyTexture = 0u;
    GLuint mCopyFramebuffer = 0u;
};

void D3D11YUVTextureShaderCopyPerf::initializeBenchmark()
{
    D3D11YUVTexturePerf::initializeBenchmark();
    D3D11YUVTexturePerf::initializeYUVProgram();
    D3D11YUVTexturePerf::initializeRGBProgram();

    glGenTextures(1, &mCopyTexture);
    glBindTexture(GL_TEXTURE_2D, mCopyTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8_OES, kTextureWidth, kTextureHeight);

    glGenFramebuffers(1, &mCopyFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mCopyFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mCopyTexture, 0);
    ASSERT_GLENUM_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER), GL_FRAMEBUFFER_COMPLETE);

    drawBenchmark();

    EXPECT_PIXEL_EQ(static_cast<GLint>(kTextureWidth) / 2, static_cast<GLint>(kTextureHeight) / 2, 225,
                    144, 235, 255);
    ASSERT_GL_NO_ERROR();
}

void D3D11YUVTextureShaderCopyPerf::destroyBenchmark()
{
    glDeleteFramebuffers(1, &mCopyFramebuffer);
    glDeleteTextures(1, &mCopyTexture);
    D3D11YUVTexturePerf::destroyBenchmark();
}

void D3D11YUVTextureShaderCopyPerf::drawBenchmark()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mCopyFramebuffer);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTextureY);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTextureUV);

    glUseProgram(mYUVProgram);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mCopyTexture);

    glUseProgram(mRGBProgram);
    for (unsigned int i = 0; i < numPasses; i++)
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glFlush();
}

TEST_P(D3D11YUVTextureShaderCopyPerf, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(D3D11YUVTextureShaderCopyPerf, TestParams(1), TestParams(2), TestParams(3));

class D3D11YUVTextureVideoProcessorBltPerf : public D3D11YUVTexturePerf
{
  public:
    D3D11YUVTextureVideoProcessorBltPerf()
        : D3D11YUVTexturePerf("D3D11YUVTextureVideoProcessorBltPerf")
    {}

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  private:
    Microsoft::WRL::ComPtr<ID3D11Texture2D> mBlitTexture;
    Microsoft::WRL::ComPtr<ID3D11VideoContext> mVideoContext;
    Microsoft::WRL::ComPtr<ID3D11VideoProcessor> mVideoProcessor;
    Microsoft::WRL::ComPtr<ID3D11VideoProcessorInputView> mInputView;
    Microsoft::WRL::ComPtr<ID3D11VideoProcessorOutputView> mOutputView;

    EGLImage mImage = nullptr;
    GLuint mTexture = 0;
};

void D3D11YUVTextureVideoProcessorBltPerf::initializeBenchmark()
{
    D3D11YUVTexturePerf::initializeBenchmark();

    CD3D11_TEXTURE2D_DESC desc(DXGI_FORMAT_B8G8R8A8_UNORM, kTextureWidth, kTextureHeight, 1, 1,
                               D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);
    ASSERT_TRUE(SUCCEEDED(mD3D11Device->CreateTexture2D(&desc, nullptr, &mBlitTexture)));

    Microsoft::WRL::ComPtr<ID3D11VideoDevice> videoDevice;
    ASSERT_TRUE(SUCCEEDED(mD3D11Device.As(&videoDevice)));

    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    mD3D11Device->GetImmediateContext(&context);
    ASSERT_TRUE(SUCCEEDED(context.As(&mVideoContext)));

    D3D11_VIDEO_PROCESSOR_CONTENT_DESC contentDesc = {};
    contentDesc.InputFrameFormat                   = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
    contentDesc.InputFrameRate.Numerator           = 60;
    contentDesc.InputFrameRate.Denominator         = 1;
    contentDesc.InputWidth                         = kTextureWidth;
    contentDesc.InputHeight                        = kTextureHeight;
    contentDesc.OutputFrameRate.Numerator          = 60;
    contentDesc.OutputFrameRate.Denominator        = 1;
    contentDesc.OutputWidth                        = kTextureWidth;
    contentDesc.OutputHeight                       = kTextureHeight;
    contentDesc.Usage                              = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

    Microsoft::WRL::ComPtr<ID3D11VideoProcessorEnumerator> videoProcessorEnumerator;
    ASSERT_TRUE(SUCCEEDED(
        videoDevice->CreateVideoProcessorEnumerator(&contentDesc, &videoProcessorEnumerator)));

    ASSERT_TRUE(SUCCEEDED(
        videoDevice->CreateVideoProcessor(videoProcessorEnumerator.Get(), 0, &mVideoProcessor)));

    Microsoft::WRL::ComPtr<ID3D11VideoContext1> context1;
    ASSERT_TRUE(SUCCEEDED(mVideoContext.As(&context1)));
    context1->VideoProcessorSetStreamColorSpace1(mVideoProcessor.Get(), 0,
                                                 DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709);
    context1->VideoProcessorSetOutputColorSpace1(mVideoProcessor.Get(),
                                                 DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709);

    const RECT destRect{0, 0, kTextureWidth, kTextureHeight};
    mVideoContext->VideoProcessorSetOutputTargetRect(mVideoProcessor.Get(), TRUE, &destRect);
    mVideoContext->VideoProcessorSetStreamDestRect(mVideoProcessor.Get(), 0, TRUE, &destRect);
    const RECT sourceRect{0, 0, kTextureWidth, kTextureHeight};
    mVideoContext->VideoProcessorSetStreamSourceRect(mVideoProcessor.Get(), 0, TRUE, &sourceRect);

    D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC inputDesc = {};
    inputDesc.ViewDimension                         = D3D11_VPIV_DIMENSION_TEXTURE2D;
    inputDesc.Texture2D.ArraySlice                  = 0;
    ASSERT_TRUE(SUCCEEDED(videoDevice->CreateVideoProcessorInputView(
        mD3D11Texture.Get(), videoProcessorEnumerator.Get(), &inputDesc, &mInputView)));

    D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC outputDesc = {};
    outputDesc.ViewDimension                          = D3D11_VPOV_DIMENSION_TEXTURE2D;
    outputDesc.Texture2D.MipSlice                     = 0;
    ASSERT_TRUE(SUCCEEDED(videoDevice->CreateVideoProcessorOutputView(
        mBlitTexture.Get(), videoProcessorEnumerator.Get(), &outputDesc, &mOutputView)));

    glGenTextures(1, &mTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    ASSERT_GL_NO_ERROR();

    const EGLint attribs[] = {EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_RGBA, EGL_NONE};
    mImage                 = eglCreateImageKHR(mDisplay, EGL_NO_CONTEXT, EGL_D3D11_TEXTURE_ANGLE,
                               static_cast<EGLClientBuffer>(mBlitTexture.Get()), attribs);
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(mImage, EGL_NO_IMAGE_KHR);

    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, mImage);
    ASSERT_GL_NO_ERROR();

    D3D11YUVTexturePerf::initializeRGBProgram();

    drawBenchmark();
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_EQ(static_cast<GLint>(kTextureWidth) / 2, static_cast<GLint>(kTextureHeight) / 2,
                    225, 144, 235, 255);
    ASSERT_GL_NO_ERROR();
}

void D3D11YUVTextureVideoProcessorBltPerf::destroyBenchmark()
{
    glDeleteTextures(1, &mTexture);

    eglDestroyImageKHR(mDisplay, mImage);

    mOutputView.Reset();
    mInputView.Reset();
    mBlitTexture.Reset();
    mVideoProcessor.Reset();
    mVideoContext.Reset();

    D3D11YUVTexturePerf::destroyBenchmark();
}

void D3D11YUVTextureVideoProcessorBltPerf::drawBenchmark()
{
    D3D11_VIDEO_PROCESSOR_STREAM stream = {};
    stream.Enable                       = true;
    stream.OutputIndex                  = 0;
    stream.InputFrameOrField            = 0;
    stream.PastFrames                   = 0;
    stream.FutureFrames                 = 0;
    stream.pInputSurface                = mInputView.Get();

    HRESULT hr =
        mVideoContext->VideoProcessorBlt(mVideoProcessor.Get(), mOutputView.Get(), 0, 1, &stream);
    ASSERT_TRUE(SUCCEEDED(hr));

    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexture);

    glUseProgram(mRGBProgram);
    for (unsigned int i = 0; i < numPasses; i++)
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glFlush();
}

TEST_P(D3D11YUVTextureVideoProcessorBltPerf, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(D3D11YUVTextureVideoProcessorBltPerf,
                       TestParams(1),
                       TestParams(2),
                       TestParams(3));

}  // namespace
