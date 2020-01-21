//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BootAnimationTest.cpp: Tests that make the same gl calls as Android's boot animations

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "common/debug.h"
#include "util/test_utils.h"

using namespace angle;
class BootAnimationTest : public ANGLETest
{
  protected:
    BootAnimationTest()
    {
        setWindowWidth(32);
        setWindowHeight(16);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    struct Texture
    {
        GLint w;
        GLint h;
        GLuint name;
    };
    void initTextureWithData(Texture *texture,
                             const void *data,
                             GLint width,
                             GLint height,
                             unsigned int channels)
    {
        texture->w = width;
        texture->h = height;

        GLint crop[4] = {0, texture->h, texture->w, -texture->h};

        glGenTextures(1, &texture->name);
        glBindTexture(GL_TEXTURE_2D, texture->name);

        switch (channels)
        {
            case 3:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->w, texture->h, 0, GL_RGB,
                             GL_UNSIGNED_SHORT_5_6_5, data);
                break;
            case 4:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RGBA,
                             GL_UNSIGNED_BYTE, data);
                break;
            default:
                UNREACHABLE();
        }

        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, crop);
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    void testSetUp() override
    {
        setWindowVisible(true);
        EGLWindow *window = getEGLWindow();
        mDisplay          = window->getDisplay();
        mSurface          = window->getSurface();
    }

    EGLDisplay mDisplay;
    EGLSurface mSurface;
};

// Makes the same GLES 1 calls as Android's default boot animation
// The original animation uses 2 different images -
// One image acts as a mask and one that moves(a gradient that acts as a shining light)
// We do the same here except with different images of much smaller resolution
// The results of each frame of the animation are compared against expected values
class BootAnimationTestDefault : public BootAnimationTest
{
  protected:
    void checkMaskColor(unsigned int iterationCount, unsigned int maskSlot)
    {
        // kOffset eliminates having to deal with negatives
        constexpr unsigned int kOffset = 15;

        // this solves for the color at any given position in our shine equivalent
        unsigned int color = ((iterationCount - maskSlot + kOffset) / 4) % 4;
        switch (color)
        {
            case 0:  // white
                EXPECT_PIXEL_EQ(12 + maskSlot, 7, 0xFF, 0xFF, 0xFF, 0xFF);
                break;
            case 1:  // blue
                EXPECT_PIXEL_EQ(12 + maskSlot, 7, 0x00, 0x00, 0xFF, 0xFF);
                break;
            case 2:  // green
                EXPECT_PIXEL_EQ(12 + maskSlot, 7, 0x00, 0xFF, 0x00, 0xFF);
                break;
            case 3:  // red
                EXPECT_PIXEL_EQ(12 + maskSlot, 7, 0xFF, 0x00, 0x00, 0xFF);
                break;
        }
    }

    void checkClearColor()
    {
        // Areas outside of the 8x1 mask area should be the clear color due to our glScissor call
        EXPECT_PIXEL_RECT_EQ(0, 0, 32, 7, GLColor::cyan);
        EXPECT_PIXEL_RECT_EQ(0, 8, 32, 8, GLColor::cyan);
        EXPECT_PIXEL_RECT_EQ(0, 7, 12, 1, GLColor::cyan);
        EXPECT_PIXEL_RECT_EQ(20, 7, 12, 1, GLColor::cyan);
    }

    void validateColors(unsigned int iterationCount)
    {
        // validate all slots in our mask
        for (int maskSlot = 0; maskSlot < 8; ++maskSlot)
        {
            // parts that are blocked in our mask are black
            switch (maskSlot)
            {
                case 0:
                case 3:
                case 4:
                case 7:
                    // slots with non zero alpha are black
                    EXPECT_PIXEL_EQ(12 + maskSlot, 7, 0x00, 0x00, 0x00, 0xFF);
                    continue;
                default:
                    checkMaskColor(iterationCount, maskSlot);
            }
        }
        // validate surrounding pixels are equal to clear color
        checkClearColor();
    }

    void testSetUp() override
    {
        BootAnimationTest::testSetUp();
        /**
         * The mask is a 8 by 1 texture colored:
         * --- --- --- --- --- --- --- ---
         * |B| |A| |A| |B| |B| |A| |A| |B|
         * --- --- --- --- --- --- --- ---
         * B is black, A is black with alpha of 0xFF
         */
        constexpr GLubyte mask[] = {
            0x0, 0x0, 0x0, 0xff,  // black
            0x0, 0x0, 0x0, 0x0,   // transparent black
            0x0, 0x0, 0x0, 0x0,   // transparent black
            0x0, 0x0, 0x0, 0xff,  // black
            0x0, 0x0, 0x0, 0xff,  // black
            0x0, 0x0, 0x0, 0x0,   // transparent black
            0x0, 0x0, 0x0, 0x0,   // transparent black
            0x0, 0x0, 0x0, 0xff   // black
        };
        /**
         * The shine is a 16 by 1 texture colored:
         * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
         * |R| |R| |R| |R| |G| |G| |G| |G| |B| |B| |B| |B| |W| |W| |W| |W|
         * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
         * R is red, G is green, B is blue, W is white
         */
        constexpr GLushort shine[] = {0xF800,  // 4 red pixels
                                      0xF800, 0xF800, 0xF800,
                                      0x07E0,  // 4 green pixels
                                      0x07E0, 0x07E0, 0x07E0,
                                      0x001F,  // 4 blue pixels
                                      0x001F, 0x001F, 0x001F,
                                      0xFFFF,  // 4 white pixels
                                      0xFFFF, 0xFFFF, 0xFFFF};

        constexpr unsigned int maskColorChannels  = 4;
        constexpr unsigned int shineColorChannels = 3;
        constexpr unsigned int maskWidth          = 8;
        constexpr unsigned int maskHeight         = 1;
        constexpr unsigned int shineWidth         = 16;
        constexpr unsigned int shineHeight        = 1;

        initTextureWithData(&mTextures[0], mask, maskWidth, maskHeight, maskColorChannels);
        initTextureWithData(&mTextures[1], shine, shineWidth, shineHeight, shineColorChannels);

        // clear screen
        glShadeModel(GL_FLAT);
        glDisable(GL_DITHER);
        glDisable(GL_SCISSOR_TEST);
        glClearColor(0, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        eglSwapBuffers(mDisplay, mSurface);
        glEnable(GL_TEXTURE_2D);
        glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        glScissor(xc, yc, mTextures[0].w, mTextures[0].h);

        // Blend state
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    }

    void testTearDown() override
    {
        glDeleteTextures(1, &mTextures[0].name);
        glDeleteTextures(1, &mTextures[1].name);
    }

    Texture mTextures[2];
    static constexpr GLint xc = 12;  // left boundary of mask
    static constexpr GLint yc = 7;   // right boundary of mask
};

TEST_P(BootAnimationTestDefault, DefaultBootAnimation)
{
    constexpr uint64_t MAX_ITERATION_COUNT = 16;
    constexpr int startingShinePosition    = -4;
    constexpr int endingShinePosition      = 12;
    GLint x                                = startingShinePosition;
    uint64_t iterationCount                = 0;
    do
    {
        glDisable(GL_SCISSOR_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_SCISSOR_TEST);
        glDisable(GL_BLEND);
        glBindTexture(GL_TEXTURE_2D, mTextures[1].name);
        glDrawTexiOES(x, yc, 0, mTextures[1].w, mTextures[1].h);
        glDrawTexiOES(x + mTextures[1].w, yc, 0, mTextures[1].w, mTextures[1].h);
        glEnable(GL_BLEND);
        glBindTexture(GL_TEXTURE_2D, mTextures[0].name);
        glDrawTexiOES(xc, yc, 0, mTextures[0].w, mTextures[0].h);
        validateColors(iterationCount);
        EGLBoolean res = eglSwapBuffers(mDisplay, mSurface);
        if (res == EGL_FALSE)
        {
            break;
        }

        if (x == endingShinePosition)
        {
            x = startingShinePosition;
        }
        ++x;
        ++iterationCount;
    } while (iterationCount < MAX_ITERATION_COUNT);
}

ANGLE_INSTANTIATE_TEST_ES1(BootAnimationTestDefault);