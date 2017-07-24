//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Multiview draw tests:
// Test issuing multiview Draw* commands.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

namespace
{

GLuint CreateTexture2D(GLenum internalFormat,
                       GLenum format,
                       GLenum type,
                       GLsizei width,
                       GLsizei height)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);
    return tex;
}

}  // namespace

class MultiviewDrawTest : public ANGLETest
{
  protected:
    MultiviewDrawTest() : mProgram(0), mMultiviewFramebuffer(0)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setWebGLCompatibilityEnabled(true);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        glGenFramebuffers(1, &mMultiviewFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mMultiviewFramebuffer);

        glRequestExtensionANGLE = reinterpret_cast<PFNGLREQUESTEXTENSIONANGLEPROC>(
            eglGetProcAddress("glRequestExtensionANGLE"));
    }

    void TearDown() override
    {

        if (mProgram != 0)
        {
            glDeleteProgram(mProgram);
            mProgram = 0;
        }

        if (mMultiviewFramebuffer != 0)
        {
            glDeleteFramebuffers(1, &mMultiviewFramebuffer);
            mMultiviewFramebuffer = 0;
        }

        ANGLETest::TearDown();
    }

    // Requests the ANGLE_multiview extension and returns true if the operation succeeds.
    bool requestMultiviewExtension()
    {
        if (extensionRequestable("GL_ANGLE_multiview"))
        {
            glRequestExtensionANGLE("GL_ANGLE_multiview");
        }

        if (!extensionEnabled("GL_ANGLE_multiview"))
        {
            std::cout << "Test skipped due to missing GL_ANGLE_multiview." << std::endl;
            return false;
        }
        return true;
    }

    void createProgram(const std::string &vs, const std::string &fs)
    {
        mProgram = CompileProgram(vs, fs);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }
        ASSERT_GL_NO_ERROR();
    }

    GLuint mProgram;
    GLuint mMultiviewFramebuffer;
    PFNGLREQUESTEXTENSIONANGLEPROC glRequestExtensionANGLE = nullptr;
};

// The test verifies that glDraw*Indirect:
// 1) generates an INVALID_OPERATION error if the number of views in the draw framebuffer is greater
// than 1.
// 2) does not generate any error if the draw framebuffer has exactly 1 view.
TEST_P(MultiviewDrawTest, IndirectDraw)
{
    if (!requestMultiviewExtension())
    {
        return;
    }

    GLuint tex2d                   = CreateTexture2D(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 1, 1);
    const GLint viewportOffsets[4] = {0, 0, 2, 0};
    ASSERT_GL_NO_ERROR();

    const std::string vsSource =
        "#version 300 es\n"
        "#extension GL_OVR_multiview : require\n"
        "layout(num_views = 2) in;\n"
        "void main()\n"
        "{}\n";

    const std::string fsSource =
        "#version 300 es\n"
        "#extension GL_OVR_multiview : require\n"
        "precision mediump float;\n"
        "void main()\n"
        "{}\n";

    createProgram(vsSource, fsSource);
    glUseProgram(mProgram);

    GLuint vao = 0u;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    const float kVertexData[3]     = {0.0f};
    const unsigned int kIndices[3] = {0u, 1u, 2u};

    GLuint vbo = 0u;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3u, &kVertexData[0], GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    GLuint ibo = 0u;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 3, &kIndices[0], GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    GLuint commandBuffer = 0u;
    glGenBuffers(1, &commandBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer);
    const GLuint commandData[] = {1u, 1u, 0u, 0u, 0u};
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(GLuint) * 5u, &commandData[0], GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    // Check for an GL_INVALID_OPERATION error with the framebuffer having 2 views.
    {
        glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex2d, 0,
                                                     2, &viewportOffsets[0]);

        glDrawArraysIndirect(GL_TRIANGLES, nullptr);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }

    // Check that no errors are generated if the number of views is 1.
    {
        glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex2d, 0,
                                                     1, &viewportOffsets[0]);

        glDrawArraysIndirect(GL_TRIANGLES, nullptr);
        EXPECT_GL_NO_ERROR();

        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);
        EXPECT_GL_NO_ERROR();
    }
}

ANGLE_INSTANTIATE_TEST(MultiviewDrawTest, ES31_OPENGL());