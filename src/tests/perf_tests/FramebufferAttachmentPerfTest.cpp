//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FramebufferAttachPerfTest:
//   Performance test for attaching and detaching resources to a Framebuffer.
//

#include "ANGLEPerfTest.h"
#include "test_utils/gl_raii.h"

#include <iostream>
#include <random>
#include <sstream>

namespace angle
{
constexpr unsigned int kIterationsPerStep = 256;
constexpr unsigned int kTextureSize       = 256;
constexpr std::size_t kTextureCount       = 4;
constexpr std::size_t kFboCount           = kTextureCount;

struct FramebufferAttachmentParams final : public RenderTestParams
{
    FramebufferAttachmentParams()
    {
        iterationsPerStep = kIterationsPerStep;

        // Common default params
        majorVersion = 3;
        minorVersion = 0;
        windowWidth  = kTextureSize;
        windowHeight = kTextureSize;

        mAttachDetachCount = 1;
    }

    std::string story() const override;
    size_t mAttachDetachCount;
};

std::ostream &operator<<(std::ostream &os, const FramebufferAttachmentParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

std::string FramebufferAttachmentParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story();
    strstr << "_" << mAttachDetachCount << "_attach_detach_count";

    return strstr.str();
}

class FramebufferAttachmentBenchmark
    : public ANGLERenderTest,
      public ::testing::WithParamInterface<FramebufferAttachmentParams>
{
  public:
    FramebufferAttachmentBenchmark() : ANGLERenderTest("Framebuffers", GetParam()) {}

    void initializeBenchmark() override;
    void drawBenchmark() override;

  private:
    void initTextures();

    std::array<GLTexture, kTextureCount> mTextures;
    std::array<GLFramebuffer, kFboCount> mFbo;
};

void FramebufferAttachmentBenchmark::initializeBenchmark()
{
    initTextures();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    ASSERT_GL_NO_ERROR();
}

void FramebufferAttachmentBenchmark::initTextures()
{
    std::vector<GLubyte> textureData(kTextureSize * kTextureSize * 4);
    for (auto &byte : textureData)
    {
        byte = rand() % 255u;
    }

    for (GLTexture &texture : mTextures)
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureSize, kTextureSize, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, textureData.data());
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void FramebufferAttachmentBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    GLint maxAttachmentCount;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttachmentCount);
    if (mTextures.size() > static_cast<size_t>(maxAttachmentCount))
    {
        FAIL() << "Texture count (" << mTextures.size() << ")"
               << " exceeds maximum attachment unit count: " << maxAttachmentCount << std::endl;
    }

    for (size_t it = 0; it < params.mAttachDetachCount; ++it)
    {
        // Attach
        for (size_t fboIndex = 0; fboIndex < mFbo.size(); fboIndex++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, mFbo[fboIndex]);
            for (size_t textureIndex = 0; textureIndex < mTextures.size(); textureIndex++)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + textureIndex,
                                       GL_TEXTURE_2D, mTextures[textureIndex], 0);
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // Detach
        for (size_t fboIndex = 0; fboIndex < mFbo.size(); fboIndex++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, mFbo[fboIndex]);
            for (size_t index = 0; index < mTextures.size(); index++)
            {
                size_t textureIndex = mTextures.size() - (index + 1);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + textureIndex,
                                       GL_TEXTURE_2D, 0, 0);
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    ASSERT_GL_NO_ERROR();
}

FramebufferAttachmentParams VulkanParams()
{
    FramebufferAttachmentParams params;
    params.eglParameters      = egl_platform::VULKAN_NULL();
    params.mAttachDetachCount = 8;

    return params;
}

TEST_P(FramebufferAttachmentBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(FramebufferAttachmentBenchmark, VulkanParams());
}  // namespace angle
