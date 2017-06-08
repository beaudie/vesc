//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WebGLReadOutsideFramebufferTest.cpp : Test functions which read the framebuffer (readPixels,
// copyTexSubImage2D, copyTexImage2D) on areas outside the framebuffer.

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

namespace
{

class PixelRect
{
  public:
    PixelRect(int width, int height) : mWidth(width), mHeight(height), mData(width * height) {}

    // Set each pixel to a different color.
    void fill(unsigned tag)
    {
        for (int i = 0; i < mWidth; ++i)
        {
            for (int j = 0; j < mHeight; ++j)
            {
                mData[i + j * mWidth] = angle::GLColor(i + (j << 8) + (tag << 16));
            }
        }
    }

    void setPixel(GLubyte x, GLubyte y, GLubyte z, GLubyte w)
    {
        mData[x + y * mWidth] = angle::GLColor(x, y, z, w);
    }

    void toTexture2D(GLuint texid) const
    {
        glBindTexture(GL_TEXTURE_2D, texid);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     mData.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    void readFB(int x, int y)
    {
        glReadPixels(x, y, mWidth, mHeight, GL_RGBA, GL_UNSIGNED_BYTE, mData.data());
    }

    // Read pixels from 'other' into 'this' from position (x,y).
    // Pixels outside 'other' are untouched or zeroed according to 'zeroOutside.'
    void readOther(const PixelRect &other, int x, int y, bool zeroOutside)
    {
        for (int i = 0; i < mWidth; ++i)
        {
            for (int j = 0; j < mHeight; ++j)
            {
                angle::GLColor *dest = &mData[i + j * mWidth];
                if (!other.getPixel(x + i, y + j, dest) && zeroOutside)
                {
                    *dest = angle::GLColor(0);
                }
            }
        }
    }

    bool getPixel(int x, int y, angle::GLColor *colorOut) const
    {
        if (0 <= x && x < mWidth && 0 <= y && y < mHeight)
        {
            *colorOut = mData[x + y * mWidth];
            return true;
        }
        return false;
    }

    void compare(const PixelRect &expected) const
    {
        for (int i = 0; i < mWidth * mHeight; ++i)
            ASSERT_EQ(expected.mData[i], mData[i]);
    }

    void compare(const PixelRect &expected, int ii, int jj,int test,int plat) const
    {
                const int ox        = ii * 64 - 2;
                const int oy        = jj * 64 - 2;
        for (int i = 0; i < mWidth * mHeight; ++i)
        {
            // XXX EXPECT_EQ helps with debugging but otherwise produces too much output - change to ASSERT_EQ later
            int x=i%mWidth, y=i/mWidth;
            int fx=x+ox,fy=y+oy;
            char b[999];
            const char *tests[] = {"???","read","texsub","teximg"};
            const char *plats[] = {"ES2_D3D9", "ES2_D3D11", "ES3_D3D11", "ES2_D3D11_FL9_3", "ES2_OPENGL", "ES3_OPENGL", "ES2_OPENGLES", "ES3_OPENGLES", "???"};
            sprintf(b,"ZZZ test %s/%s - box %d,%d - pixel %d,%d - %sside", tests[test],plats[plat],ii,jj,x,y,0<=fx&&fx<128&&0<=fy&&fy<128 ?"in":"out");
            //angle::GLColor pos(x,y,w,w);
            //angle::GLColor zero(0);
            if(expected.mData[i]== mData[i]) {} else
            {
                GTEST_NONFATAL_FAILURE_(b);
                std::stringstream ss;
                ss << "YYY expected " << expected.mData[i] << "   got " << mData[i];
                GTEST_NONFATAL_FAILURE_(ss.str().c_str());
                //EXPECT_EQ(zero,pos);
                //EXPECT_EQ(expected.mData[i], mData[i]);
            }
        }
    }

  private:
    int mWidth, mHeight;
    std::vector<angle::GLColor> mData;
};

}  // namespace

namespace angle
{

class WebGLReadOutsideFramebufferTest : public ANGLETest
{
  public:
    void UseReadPixels(int x, int y, PixelRect *pixelsOut) { pixelsOut->readFB(x, y); }

    void UseCopyTexSubImage2D(int x, int y, PixelRect *pixelsOut)
    {
        GLTexture destTexture;
        pixelsOut->toTexture2D(destTexture.get());

        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, y, readWidth, readHeight);

        readTexture(readWidth, readHeight, pixelsOut);
    }

    void UseCopyTexImage2D(int x, int y, PixelRect *pixelsOut)
    {
        GLTexture destTexture;
        pixelsOut->toTexture2D(destTexture.get());
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, readWidth, readHeight, 0);

        readTexture(readWidth, readHeight, pixelsOut);
    }

  protected:
    static constexpr int fbWidth    = 128;
    static constexpr int fbHeight   = 128;
    static constexpr int readWidth  = 4;
    static constexpr int readHeight = 4;

    // these values ensure no overlap between framebuffer pixel values and read buffer pixel values
    static constexpr GLuint fbStartColor   = 0x01010101;
    static constexpr GLuint readStartColor = 0xf1f1f1f1;

    WebGLReadOutsideFramebufferTest() : mFBData(fbWidth, fbHeight)
    {
        setWindowWidth(fbWidth);
        setWindowHeight(fbHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setWebGLCompatibilityEnabled(true);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();
        const angle::PlatformParameters &param = GetParam();
        if      (param==ES2_D3D9())        mPlatform=0;
        else if (param==ES2_D3D11())       mPlatform=1;
        else if (param==ES3_D3D11())       mPlatform=2;
        else if (param==ES2_D3D11_FL9_3()) mPlatform=3;
        else if (param==ES2_OPENGL())      mPlatform=4;
        else if (param==ES3_OPENGL())      mPlatform=5;
        else if (param==ES2_OPENGLES())    mPlatform=6;
        else if (param==ES3_OPENGLES())    mPlatform=7;
        else                               mPlatform=9;

        const std::string vertexShader =
            "attribute vec3 a_position;\n"
            "varying vec2 v_texCoord;\n"
            "void main() {\n"
            "    v_texCoord = a_position.xy * 0.5 + 0.5;\n"
            "    gl_Position = vec4(a_position, 1);\n"
            "}\n";
        const std::string fragmentShader =
            "precision mediump float;\n"
            "varying vec2 v_texCoord;\n"
            "uniform sampler2D u_texture;\n"
            "void main() {\n"
            "    gl_FragColor = texture2D(u_texture, v_texCoord);\n"
            "}\n";

        mProgram = CompileProgram(vertexShader, fragmentShader);
        glUseProgram(mProgram);
        GLint uniformLoc = glGetUniformLocation(mProgram, "u_texture");
        ASSERT_NE(-1, uniformLoc);
        glUniform1i(uniformLoc, 0);

        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        // fill framebuffer with unique pixels
        mFBData.fill(0xffff);
        GLTexture fbTexture;
        mFBData.toTexture2D(fbTexture.get());
        drawQuad(mProgram, "a_position", 0.0f, 1.0f, true);
    }

    void TearDown() override
    {
        glDeleteProgram(mProgram);
        ANGLETest::TearDown();
    }

    void Test(void (WebGLReadOutsideFramebufferTest::*readFunc)(int x, int y, PixelRect *dest), int test)
    {
        PixelRect actual(readWidth, readHeight);
        PixelRect expected(readWidth, readHeight);

        // Read a readWidth*readHeight rectangle of pixels from places that include:
        // - completely outside framebuffer, on all sides of it (i,j < 0 or > 2)
        // - completely inside framebuffer (i,j == 1)
        // - straddling framebuffer boundary, at each corner and side
        for (int i = -1; i < 4; ++i)
        {
            for (int j = -1; j < 4; ++j)
            {
                const int x        = i * fbWidth / 2 - readWidth / 2;
                const int y        = j * fbHeight / 2 - readHeight / 2;
                //XXX  w=1 in, 2 out   t=1 readpix, 2 copytexsub, 3 copytex, p=1..9 platform

                // Fill framebuffer  a, b, 2ij, tp
                for (int a = 0; a < fbWidth; ++a)
                for (int b = 0; b < fbHeight; ++b)
                {
                    mFBData.setPixel((GLubyte)a, (GLubyte)b, (GLubyte)(200 + (i + 1)*10 + (j + 1)), (GLubyte)(test*10 + mPlatform));
                }
                GLTexture fbTexture;
                mFBData.toTexture2D(fbTexture.get());
                glViewport(0, 0, fbWidth, fbHeight);
                drawQuad(mProgram, "a_position", 0.0f, 1.0f, true);

                //XXX sanity
                //PixelRect sanity(fbWidth, fbHeight);
                //sanity.readFB(0,0);
                //mFBData.compare(sanity);

                // Put unique pixel values into the read destinations.
                //actual.fill(tag);
                // Fill read buffers  a, b, 1ij, wtp
                for (int a = 0; a < readWidth; ++a)
                for (int b = 0; b < readHeight; ++b)
                {
                    angle::GLColor junk;
                    int w=mFBData.getPixel(x+a, y+b, &junk) ? 1 : 2;
                    actual.setPixel((GLubyte)a, (GLubyte)b, (GLubyte)(100 + (i + 1)*10 + (j + 1)), (GLubyte)(w*100 + test*10 + mPlatform));
                }
                expected.readOther(actual, 0, 0, false);

                // Read from framebuffer into 'actual.'
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                (this->*readFunc)(x, y, &actual);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                // Simulate framebuffer read, into 'expected.'
                expected.readOther(mFBData, x, y, test==3);

                // See if they are the same.
                actual.compare(expected,i,j,test,mPlatform);
            }
        }
    }

    // Get contents of current texture by drawing it into a framebuffer then reading with
    // glReadPixels().
    void readTexture(GLsizei width, GLsizei height, PixelRect *out)
    {
        GLRenderbuffer colorBuffer;
        glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  colorBuffer.get());

        glViewport(0, 0, width, height);
        drawQuad(mProgram, "a_position", 0.0f, 1.0f, true);
        out->readFB(0, 0);
    }

    int mPlatform;
    PixelRect mFBData;
    GLuint mProgram;
};

// Check that copyTexSubImage2D does not set a destination pixel when
// the corresponding source pixel is outside the framebuffer.
TEST_P(WebGLReadOutsideFramebufferTest, CopyTexSubImage2D)
{
    Test(&WebGLReadOutsideFramebufferTest::UseCopyTexSubImage2D, 2);
}

ANGLE_INSTANTIATE_TEST(WebGLReadOutsideFramebufferTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_D3D11_FL9_3(),
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES2_OPENGLES(),
                       ES3_OPENGLES());

}  // namespace
