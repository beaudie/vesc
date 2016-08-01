//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageGLES31Test:
//   GLES 3.10 Image tests.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include <vector>

using namespace angle;

using namespace angle;

namespace
{

class ImageGLES31Test : public ANGLETest
{
  protected:
    ImageGLES31Test() {}

  protected:
    GLuint mProgram;

    GLuint create2DTexture(GLenum internalFormat)
    {
        GLuint tex = 0;

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, 1, 1);

        glBindTexture(GL_TEXTURE_2D, 0);

        return tex;
    }
    GLuint create3DTexture(GLenum internalFormat)
    {
        GLuint tex = 0;

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_3D, tex);

        glTexStorage3D(GL_TEXTURE_3D, 1, internalFormat, 1, 1, 1);

        glBindTexture(GL_TEXTURE_3D, 0);

        return tex;
    }
    GLuint create2DArrayTexture(GLenum internalFormat, int numLayers)
    {
        GLuint tex = 0;

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D_ARRAY, tex);

        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, internalFormat, 1, 1, numLayers);

        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

        return tex;
    }

    GLuint createCubeMapTexture(GLenum internalFormat)
    {
        GLuint tex = 0;

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

        for (int i = 0; i < 6; ++i)
        {
            glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, 1, 1);
        }

        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

        return tex;
    }
};

// The test checks whether the location of the images can be retrieved.
TEST_P(ImageGLES31Test, GetUniformLocation)
{
    const std::string csSource =
        "#version 310 es\n"
        "layout(local_size_x=1) in;\n"
        "precision highp image2D;\n"
        "layout(rgba32f) uniform image2D myImage;\n"
        "layout(rgba32f) uniform image2D myImage2;\n"
        "void main()\n"
        "{\n"
        "   imageLoad(myImage, ivec2(0));"
        "   imageLoad(myImage2, ivec2(0));"
        "}\n";

    ANGLE_GL_COMPUTE_PROGRAM(program, csSource);

    GLint myImageLoc = glGetUniformLocation(program.get(), "myImage");
    EXPECT_NE(-1, myImageLoc);

    GLint myImage2Loc = glGetUniformLocation(program.get(), "myImage2");
    EXPECT_NE(-1, myImage2Loc);
}

// The test checks whether the binding locations can be retrieved through GetUniformiv.
// Each image should have a separate image binding.
TEST_P(ImageGLES31Test, ImplicitBindingPoints)
{
    const std::string csSource =
        "#version 310 es\n"
        "layout(local_size_x=1) in;\n"
        "precision highp image2D;\n"
        "layout(rgba32f) uniform image2D myImage;\n"
        "layout(rgba32f) uniform image2D myImage2;\n"
        "void main()\n"
        "{\n"
        "   imageLoad(myImage, ivec2(0));"
        "   imageLoad(myImage2, ivec2(0));"
        "}\n";

    ANGLE_GL_COMPUTE_PROGRAM(program, csSource);

    GLint myImageLoc  = glGetUniformLocation(program.get(), "myImage");
    GLint myImage2Loc = glGetUniformLocation(program.get(), "myImage2");

    GLint myImageUnit  = -1;
    GLint myImage2Unit = -1;

    glGetUniformiv(program.get(), myImageLoc, &myImageUnit);
    EXPECT_EQ(0, myImageUnit);

    glGetUniformiv(program.get(), myImage2Loc, &myImage2Unit);
    EXPECT_EQ(0, myImage2Unit);
}

// The test checks whether the binding locations can be retrieved through GetUniformiv.
// Each image should have a separate image binding.
TEST_P(ImageGLES31Test, ExplicitBindingPoints)
{
    const std::string csSource =
        "#version 310 es\n"
        "layout(local_size_x=1) in;\n"
        "precision highp image2D;\n"
        "layout(rgba32f, binding = 1) uniform image2D myImage;\n"
        "layout(rgba32f, binding = 4) uniform image2D myImage2;\n"
        "void main()\n"
        "{\n"
        "   imageLoad(myImage, ivec2(0));"
        "   imageLoad(myImage2, ivec2(0));"
        "}\n";

    ANGLE_GL_COMPUTE_PROGRAM(program, csSource);

    GLint myImageLoc  = glGetUniformLocation(program.get(), "myImage");
    GLint myImage2Loc = glGetUniformLocation(program.get(), "myImage2");

    GLint myImageUnit  = -1;
    GLint myImage2Unit = -1;

    glGetUniformiv(program.get(), myImageLoc, &myImageUnit);
    EXPECT_EQ(1, myImageUnit);

    glGetUniformiv(program.get(), myImage2Loc, &myImage2Unit);
    EXPECT_EQ(4, myImage2Unit);
}

ANGLE_INSTANTIATE_TEST(ImageGLES31Test, ES31_OPENGL(), ES31_OPENGLES());

}  // namespace
