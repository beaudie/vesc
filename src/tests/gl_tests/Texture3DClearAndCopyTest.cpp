
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

namespace angle
{

class Texture3DClearAndCopyTest : public ANGLETest
{};

TEST_P(Texture3DClearAndCopyTest, Test)
{
    uint32_t width  = 128;
    uint32_t height = 128;
    uint32_t depth  = 7;

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, width, height, depth);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);

    GLuint buffer = 0;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 4 * width * height * depth, nullptr, GL_STATIC_DRAW);

    // Doesn't crash if this is just 1.
    // Also doesn't crash if |depth| is 2 instead of 7.
    uint32_t slicesToTest = 2;

    {
        GLuint framebuffer = 0;
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);

        std::array<GLfloat, 4> fClearColorData;
        fClearColorData.fill(1.f);

        for (uint32_t z = 0; z < slicesToTest; ++z)
        {
            glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, z);
            glClearBufferfv(GL_COLOR, 0, fClearColorData.data());
        }

        glDeleteFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
    {
        uint8_t *offset    = nullptr;
        GLuint framebuffer = 0;
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer);
        glPixelStorei(GL_PACK_ROW_LENGTH, 4 * width);

        for (uint32_t z = 0; z < slicesToTest; ++z)
        {
            glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, z);
            glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, offset);
            offset += 4 * width * height;

            // The second time, this crashes:
            // vk_helpers.cpp:6576 (readPixels):       ! Assert failed in readPixels
            // (../../src/libANGLE/renderer/vulkan/vk_helpers.cpp:6576):
            // !hasStagedUpdatesForSubresource(levelGL, layer, 1)
        }

        glDeleteFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }
}

ANGLE_INSTANTIATE_TEST_ES3(Texture3DClearAndCopyTest);

}  // namespace angle
