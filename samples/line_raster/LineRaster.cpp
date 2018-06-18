//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// LineRaster:
//   Sample displaying line rasterization properties. Used to debug Vulkan
//   line rasterization.

#include <array>

#include "SampleApplication.h"
#include "shader_utils.h"

constexpr GLint kSize = 16;

constexpr char quadVS[] = R"(attribute vec2 position;
varying vec2 texCoord;
void main()
{
    gl_Position = vec4(position, 0, 1);
    texCoord = position * 0.5 + vec2(0.5);
})";

constexpr char quadFS[] = R"(precision mediump float;
uniform sampler2D tex;
varying vec2 texCoord;
void main()
{
    gl_FragColor = texture2D(tex, texCoord);
})";

class LineRasterSample : public SampleApplication
{
  public:
    LineRasterSample(EGLint displayType)
        : SampleApplication("LineRaster", 512, 512, 2, 0, displayType)
    {
    }

    virtual bool initialize()
    {
        const std::string vs =
            R"(attribute vec4 vPosition;
            uniform mat2 rotation;
            varying vec2 pos;
            void main()
            {
                pos = vPosition.xy * rotation;
                gl_Position = vec4(pos, 0, 1);
            })";

        const std::string lineFS =
            R"(precision mediump float;
            varying vec2 pos;
            uniform vec4 color;
            bool ok(float f) { return f >= 0.0 && f <= 1.0; }
            float cross2D(vec2 v1, vec2 v2) {
              return v1.y * v2.x - v1.x * v2.y;
            }
            bool li(vec2 a, vec2 b, vec2 c, vec2 dc) {
              float rup = cross2D(a - c, dc);
              float rlo = cross2D(a - b, dc);
              return ok(rup / rlo); 
            }
            void main()
            {
                vec2 w = ((pos * 0.5) + 0.5) * 16.0;
                vec2 wt = vec2(gl_FragCoord.y - w.y, w.x - gl_FragCoord.x);
                vec2 a = vec2(gl_FragCoord.x + 0.5, gl_FragCoord.y); 
                vec2 b = vec2(gl_FragCoord.x, gl_FragCoord.y + 0.5); 
                vec2 c = vec2(gl_FragCoord.x - 0.5, gl_FragCoord.y); 
                vec2 d = vec2(gl_FragCoord.x, gl_FragCoord.y - 0.5);
                if (li(a, b, w, wt) || li(b, c, w, wt) || li(c, d, w, wt) || li(d, a, w, wt))
                {
                    gl_FragColor = color;
                }
                else
                {
                    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
                }
            })";

        mLineProgram = CompileProgram(vs, lineFS);
        if (!mLineProgram)
        {
            return false;
        }

        mBlitProgram = CompileProgram(quadVS, quadFS);
        if (!mBlitProgram)
        {
            return false;
        }

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glEnableVertexAttribArray(0);

        GLfloat lineVerts[] =
        {
             -1.0f, -1.0f, 1.0f, 1.0f,
        };

        glGenBuffers(1, &mLineBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mLineBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(lineVerts), lineVerts, GL_STATIC_DRAW);

        GLfloat quadVerts[] =
        {
            -1.0f, 1.0f,
            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, -1.0f,
            1.0f, 1.0f,
        };

        glGenBuffers(1, &mQuadBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mQuadBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

        glGenFramebuffers(1, &mFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

        glGenTextures(1, &mTexture);
        glBindTexture(GL_TEXTURE_2D, mTexture);
        glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);

        mMatLoc = glGetUniformLocation(mLineProgram, "rotation");
        mColorLoc = glGetUniformLocation(mLineProgram, "color");

        mTimer.reset(CreateTimer());
        mTimer->start();
        return true;
    }

    void destroy() override
    {
        glDeleteTextures(1, &mTexture);
        glDeleteBuffers(1, &mLineBuffer);
        glDeleteBuffers(1, &mQuadBuffer);
        glDeleteFramebuffers(1, &mFramebuffer);
        glDeleteProgram(mLineProgram);
        glDeleteProgram(mBlitProgram);
    }

    void draw() override
    {
        std::array<GLfloat, 4> mat = {{1, 0, 0, 1}};

        //GLfloat time = static_cast<GLfloat>(1) * 0.1f;
        GLfloat time = static_cast<GLfloat>(mTimer->getElapsedTime()) * 0.1f;
        mat[0] = cosf(time);
        mat[1] = -sinf(time);
        mat[2] = -mat[1];
        mat[3] = mat[0];

        glViewport(0, 0, kSize, kSize);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
        glUseProgram(mLineProgram);
        glUniform4f(mColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        glUniformMatrix2fv(mMatLoc, 1, GL_FALSE, mat.data());
        glBindBuffer(GL_ARRAY_BUFFER, mLineBuffer);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_LINES, 0, 2);

        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(mBlitProgram);
        glBindBuffer(GL_ARRAY_BUFFER, mQuadBuffer);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUseProgram(mLineProgram);
        glUniform4f(mColorLoc, 1.0f, 0.0f, 0.0f, 1.0f);
        glBindBuffer(GL_ARRAY_BUFFER, mLineBuffer);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glDrawArrays(GL_LINES, 0, 2);
    }

  private:
    GLuint mTexture;
    GLuint mFramebuffer;
    GLuint mLineProgram;
    GLuint mBlitProgram;
    GLuint mLineBuffer;
    GLuint mQuadBuffer;
    GLint mMatLoc;
    GLint mColorLoc;
    std::unique_ptr<Timer> mTimer;
};

int main(int argc, char **argv)
{
    EGLint displayType = EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE;

    if (argc > 1)
    {
        displayType = GetDisplayTypeFromArg(argv[1]);
    }

    LineRasterSample app(displayType);
    return app.run();
}
