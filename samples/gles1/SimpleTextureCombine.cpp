//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//            Based on Simple_Texture2D.c from
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com

#include "SampleApplication.h"
#include "shader_utils.h"
#include "texture_utils.h"

#include <GLES/gl.h>
#include <GLES/glext.h>

#include <array>
#include <vector>

class SimpleTextureCombineSample : public SampleApplication
{
  public:
    SimpleTextureCombineSample()
        : SampleApplication("SimpleTextureEnvironment", 1280, 720,
                            1 /* gles1 */)
    {
    }

    struct CombineParameters {
        GLenum combineRgb;
        GLenum combineAlpha;
        GLenum src0Rgb;
        GLenum src0Alpha;
        GLenum src1Rgb;
        GLenum src1Alpha;
        GLenum src2Rgb;
        GLenum src2Alpha;
        GLenum op0Rgb;
        GLenum op0Alpha;
        GLenum op1Rgb;
        GLenum op1Alpha;
        GLenum op2Rgb;
        GLenum op2Alpha;
        float rgbScale;
        float alphaScale;
    };

    virtual bool initialize()
    {
        // Load the textures
        mTextures.push_back(CreateSimpleTexture2D());

        // Populate texture environment parameters
        mColors.push_back({ 1.0f, 1.0f, 1.0f, 1.0f });
        mColors.push_back({ 0.0f, 0.0f, 0.0f, 1.0f });
        mColors.push_back({ 1.0f, 1.0f, 1.0f, 0.5f });
        mColors.push_back({ 0.0f, 0.0f, 0.0f, 0.5f });
        mColors.push_back({ 1.0f, 0.0f, 0.0f, 1.0f });
        mColors.push_back({ 0.0f, 1.0f, 0.0f, 1.0f });
        mColors.push_back({ 0.0f, 0.0f, 1.0f, 1.0f });
        mColors.push_back({ 1.0f, 0.0f, 0.0f, 0.5f });
        mColors.push_back({ 0.0f, 1.0f, 0.0f, 0.5f });
        mColors.push_back({ 0.0f, 0.0f, 1.0f, 0.5f });

        // Generate some combinations
        std::vector<GLenum> combineRgbs;
        std::vector<GLenum> combineAlphas;

        combineRgbs.push_back(GL_REPLACE);
        combineRgbs.push_back(GL_MODULATE);
        combineRgbs.push_back(GL_ADD);
        combineRgbs.push_back(GL_ADD_SIGNED);
        combineRgbs.push_back(GL_INTERPOLATE);
        combineRgbs.push_back(GL_SUBTRACT);
        combineRgbs.push_back(GL_DOT3_RGB);
        combineRgbs.push_back(GL_DOT3_RGBA);

        combineAlphas.push_back(GL_REPLACE);
        combineAlphas.push_back(GL_MODULATE);
        combineAlphas.push_back(GL_ADD);
        combineAlphas.push_back(GL_ADD_SIGNED);
        combineAlphas.push_back(GL_INTERPOLATE);
        combineAlphas.push_back(GL_SUBTRACT);

        for (const auto combineRgb : combineRgbs) {
            for (const auto combineAlpha : combineAlphas) {
                mCombineParameters.push_back({
                    combineRgb,
                    combineAlpha,
                    GL_TEXTURE, // src0rgb
                    GL_TEXTURE, // src0alpha
                    GL_PRIMARY_COLOR, // src1rgb
                    GL_PRIMARY_COLOR, // src1alpha
                    GL_TEXTURE, // src2rgb
                    GL_TEXTURE, // src2alpha
                    GL_SRC_COLOR, // op0rgb
                    GL_SRC_ALPHA, // op0alpha
                    GL_SRC_COLOR, // op1rgb
                    GL_SRC_ALPHA, // op1alpha
                    GL_SRC_COLOR, // op2rgb
                    GL_SRC_ALPHA, // op2alpha
                    1.0f, // rgbScale
                    1.0f, // alphaScale
                });
            }
        }
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        return true;
    }

    virtual void destroy()
    {
        glDeleteTextures((GLsizei)mTextures.size(), mTextures.data());
    }

    virtual void draw()
    {
        GLfloat vertices[] =
        {
            -0.5f,  0.5f, 0.0f,  // Position 0
             0.0f,  0.0f,        // TexCoord 0
            -0.5f, -0.5f, 0.0f,  // Position 1
             0.0f,  1.0f,        // TexCoord 1
             0.5f, -0.5f, 0.0f,  // Position 2
             1.0f,  1.0f,        // TexCoord 2
             0.5f,  0.5f, 0.0f,  // Position 3
             1.0f,  0.0f         // TexCoord 3
        };
        GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        // Load the vertex position
        glVertexPointer(3, GL_FLOAT, 5 * sizeof(GLfloat), vertices);
        // Load the texture coordinate
        glTexCoordPointer(2, GL_FLOAT, 5 * sizeof(GLfloat), vertices + 3);

        glActiveTexture(GL_TEXTURE0);

        glMatrixMode(GL_MODELVIEW);

        float currX = -0.8f;
        float currY = -0.8f;

        for (size_t i = 0; i < mColors.size(); i++) {
            currX = -0.8f;
            for (size_t j = 0; j < mCombineParameters.size(); j++) {
                const auto& currColor = mColors[i];
                const auto& combineParams = mCombineParameters[j];

                glBindTexture(GL_TEXTURE_2D, mTextures[0]);
                glColor4f(currColor[0], currColor[1], currColor[2], currColor[3]);

                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
                glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, combineParams.combineRgb);
                glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, combineParams.combineAlpha);
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, combineParams.src0Rgb);
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, combineParams.src0Alpha);
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, combineParams.src1Rgb);
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, combineParams.src1Alpha);
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, combineParams.src2Rgb);
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_ALPHA, combineParams.src2Alpha);
                glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, combineParams.op0Rgb);
                glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, combineParams.op0Alpha);
                glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, combineParams.op1Rgb);
                glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, combineParams.op1Alpha);
                glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, combineParams.op2Rgb);
                glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, combineParams.op2Alpha);
                glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, combineParams.rgbScale);
                glTexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, combineParams.alphaScale);

                glLoadIdentity();
                glTranslatef(currX, currY, 0.0f);
                glPushMatrix();
                float scaleFactorX = (0.8f + 0.8f) / (1.4 * mCombineParameters.size());
                float scaleFactorY = (0.8f + 0.8f) / (1.4 * mColors.size());
                glScalef(scaleFactorX, scaleFactorY, 1.0f);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
                glPopMatrix();

                float translateX = (0.8f + 0.8f) / (mCombineParameters.size());
                currX += translateX;
            }
            float translateY = (0.8f + 0.8f) / (mColors.size());
            currY += translateY;
        }

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

  private:
    // Texture handles, colors
    std::vector<GLuint> mTextures;
    std::vector<std::array<GLfloat, 4> > mColors;

    // Combine args
    std::vector<CombineParameters> mCombineParameters;
};

int main(int argc, char **argv)
{
    SimpleTextureCombineSample app;
    return app.run();
}
