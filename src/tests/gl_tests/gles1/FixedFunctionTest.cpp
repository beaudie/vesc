//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Tests fixed function computing the correct values

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "common/matrix_utils.h"
#include "common/vector_utils.h"
#include "util/random_utils.h"

#include <stdint.h>

#include <vector>

using namespace angle;

class FixedFunctionTest : public ANGLETest<>
{
  protected:
    FixedFunctionTest()
    {
        setWindowWidth(32);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void drawTestQuad();
};

// Check a case that approximates the one caught in the wild
TEST_P(FixedFunctionTest, Bug6201)
{
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    std::vector<GLColor> colors;
    for (uint32_t x = 0; x < 1024; x++)
    {
        for (uint32_t y = 0; y < 1024; y++)
        {
            float x_ratio = (float)x / 1024.0f;
            GLubyte v     = (GLubyte)(255u * x_ratio);

            GLColor color = {v, v, v, 255u};
            colors.push_back(color);
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 colors.data());

    glMatrixMode(GL_PROJECTION);

    const GLfloat projectionMatrix[16] = {
        0.615385, 0, 0, 0, 0, 1.333333, 0, 0, 0, 0, 1, 1, 0, 0, -2, 0,
    };
    glLoadMatrixf(projectionMatrix);

    glEnable(GL_LIGHT0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glClearColor(1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0.0f, 0.0f, 32.0f, 32.0f);

    const GLfloat ambient[4]  = {2.0f, 2.0f, 2.0f, 1.0f};
    const GLfloat diffuse[4]  = {1.0f, 1.0f, 1.0f, 1.0f};
    const GLfloat position[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    glMatrixMode(GL_MODELVIEW);

    const GLfloat modelMatrix[16] = {
        0.976656, 0.000000, -0.214807, 0.000000, 0.000000,   1.000000, 0.000000,   0.000000,
        0.214807, 0.000000, 0.976656,  0.000000, -96.007507, 0.000000, 200.000000, 1.000000,
    };
    glLoadMatrixf(modelMatrix);

    glBindTexture(GL_TEXTURE_2D, texture);

    std::vector<float> positions = {
        -64.0f, -89.0f, 1.0f, -64.0f, 89.0f, 1.0f, 64.0f, -89.0f, 1.0f, 64.0f, 89.0f, 1.0f,
    };

    std::vector<float> uvs = {
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    };

    std::vector<float> normals = {
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    };

    glVertexPointer(3, GL_FLOAT, 0, positions.data());
    glTexCoordPointer(2, GL_FLOAT, 0, uvs.data());
    glNormalPointer(GL_FLOAT, 0, normals.data());

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_NEAR(11, 11, GLColor(29, 29, 29, 255), 1);
}

void FixedFunctionTest::drawTestQuad()
{
    struct Vertex
    {
        GLfloat position[3];
        GLfloat normal[3];
    };

    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustumf(-1, 1, -1, 1, 5.0, 60.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -8.0f);
    glRotatef(150, 0, 1, 0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    {
        GLfloat ambientAndDiffuse[4] = {1.0f, 0.0f, 0.0f, 1.0f};
        GLfloat specular[4]          = {0.0f, 0.0f, 10.0f, 1.0f};
        GLfloat shininess            = 2.0f;

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, ambientAndDiffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    }

    std::vector<Vertex> vertices = {
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    };
    glVertexPointer(3, GL_FLOAT, sizeof vertices[0], &vertices[0].position);
    glNormalPointer(GL_FLOAT, sizeof vertices[0], &vertices[0].normal);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size());

    EXPECT_GL_NO_ERROR();
}

// Check smooth lighting
TEST_P(FixedFunctionTest, SmoothLitMesh)
{
    {
        GLfloat position[4] = {0.0f, 0.0f, -20.0f, 1.0f};
        GLfloat diffuse[4]  = {0.7f, 0.7f, 0.7f, 1.0f};
        GLfloat specular[4] = {0.1f, 0.1f, 1.0f, 1.0f};

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, position);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    }

    drawTestQuad();
    EXPECT_PIXEL_COLOR_NEAR(16, 16, GLColor(205, 0, 92, 255), 1);
}

// Check flat lighting
TEST_P(FixedFunctionTest, FlatLitMesh)
{
    {
        GLfloat position[4] = {0.0f, 0.0f, -20.0f, 1.0f};
        GLfloat diffuse[4]  = {0.7f, 0.7f, 0.7f, 1.0f};
        GLfloat specular[4] = {0.1f, 0.1f, 1.0f, 1.0f};

        glEnable(GL_LIGHTING);
        glShadeModel(GL_FLAT);
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, position);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    }

    drawTestQuad();
    EXPECT_PIXEL_COLOR_NEAR(16, 16, GLColor(211, 0, 196, 255), 1);
}

// Check fog
TEST_P(FixedFunctionTest, ExponentiallyFoggyMesh)
{
    angle::RNG rng;

    glEnable(GL_FOG);

    for (int i = 0; i < 10; i++)
    {
        {
            GLfloat fogColor[4] = {0.0f, 1.0f, 0.0f, 1.0f};

            glFogf(GL_FOG_DENSITY, 0.1f);
            glFogf(GL_FOG_START, rng.randomFloat());
            glFogf(GL_FOG_END, rng.randomFloat());
            glFogf(GL_FOG_MODE, GL_EXP);
            glFogfv(GL_FOG_COLOR, fogColor);
        }

        drawTestQuad();
        EXPECT_PIXEL_COLOR_NEAR(16, 16, GLColor(116, 255, 116, 255), 1);
    }
}

// Check exp2 fog
TEST_P(FixedFunctionTest, Exponentially2FoggyMesh)
{
    angle::RNG rng;

    glEnable(GL_FOG);

    for (int i = 0; i < 10; i++)
    {
        {
            GLfloat fogColor[4] = {0.0f, 1.0f, 0.0f, 1.0f};

            glFogf(GL_FOG_DENSITY, 0.1f);
            glFogf(GL_FOG_START, rng.randomFloat());
            glFogf(GL_FOG_END, rng.randomFloat());
            glFogf(GL_FOG_MODE, GL_EXP2);
            glFogfv(GL_FOG_COLOR, fogColor);
        }

        drawTestQuad();
        EXPECT_PIXEL_COLOR_NEAR(16, 16, GLColor(136, 255, 136, 255), 1);
    }
}

// Check linear fog
TEST_P(FixedFunctionTest, LinearlyFoggyMesh)
{
    angle::RNG rng;

    glEnable(GL_FOG);

    for (int i = 0; i < 10; i++)
    {
        {
            GLfloat fogColor[4] = {0.0f, 1.0f, 0.0f, 1.0f};

            glFogf(GL_FOG_DENSITY, rng.randomFloat());
            glFogf(GL_FOG_START, 7.9f);
            glFogf(GL_FOG_END, 8.5f);
            glFogf(GL_FOG_MODE, GL_LINEAR);
            glFogfv(GL_FOG_COLOR, fogColor);
        }

        drawTestQuad();
        EXPECT_PIXEL_COLOR_NEAR(16, 16, GLColor(225, 255, 225, 255), 1);
    }
}

ANGLE_INSTANTIATE_TEST_ES1(FixedFunctionTest);
