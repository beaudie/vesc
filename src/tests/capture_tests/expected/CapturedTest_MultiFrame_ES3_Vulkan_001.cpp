#include "CapturedTest_MultiFrame_ES3_Vulkan.h"
#include "angle_trace_gl.h"

const char *const glShaderSource_string_0[] = { 
"precision highp float;\n"
"attribute vec3 attr1;\n"
"void main(void) {\n"
"   gl_Position = vec4(attr1, 1.0);\n"
"}",
};
const char *const glShaderSource_string_1[] = { 
"precision highp float;\n"
"void main(void) {\n"
"   gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
"}",
};

// Private Functions

void SetupReplayContext1(void)
{
    eglMakeCurrent(gEGLDisplay, gSurfaceMap2[0], gSurfaceMap2[0], gContextMap2[1]);
    glUseProgram(gShaderProgramMap[0]);
    UpdateCurrentProgram(0);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, gTransformFeedbackMap[0]);
    glViewport(0, 0, 128, 128);
    glScissor(0, 0, 128, 128);
}

void ReplayFrame1(void)
{
    eglGetError();
    glClearColor(0.25, 0.5, 0.5, 0.5);
    glClear(GL_COLOR_BUFFER_BIT);
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void *)gReadBuffer);
    glGetError();
}

void ReplayFrame2(void)
{
    eglGetError();
    glGenBuffers(1, (GLuint *)gReadBuffer);
    UpdateBufferID(1, 0);
    glBindBuffer(GL_ARRAY_BUFFER, gBufferMap[1]);
    CreateProgram(6);
    CreateShader(GL_VERTEX_SHADER, 7);
    glShaderSource(gShaderProgramMap[7], 1, glShaderSource_string_0, (const GLint *)&gBinaryData[0]);
    glCompileShader(gShaderProgramMap[7]);
    glGetShaderiv(gShaderProgramMap[7], GL_COMPILE_STATUS, (GLint *)gReadBuffer);
    CreateShader(GL_FRAGMENT_SHADER, 8);
    glShaderSource(gShaderProgramMap[8], 1, glShaderSource_string_1, (const GLint *)&gBinaryData[16]);
    glCompileShader(gShaderProgramMap[8]);
    glGetShaderiv(gShaderProgramMap[8], GL_COMPILE_STATUS, (GLint *)gReadBuffer);
    glAttachShader(gShaderProgramMap[6], gShaderProgramMap[7]);
    glDeleteShader(gShaderProgramMap[7]);
    glAttachShader(gShaderProgramMap[6], gShaderProgramMap[8]);
    glDeleteShader(gShaderProgramMap[8]);
    glLinkProgram(gShaderProgramMap[6]);
    glGetError();
    glGetProgramiv(gShaderProgramMap[6], GL_LINK_STATUS, (GLint *)gReadBuffer);
    glBindAttribLocation(gShaderProgramMap[6], 0, "attr1");
    glLinkProgram(gShaderProgramMap[6]);
    glGetError();
    glGetProgramiv(gShaderProgramMap[6], GL_LINK_STATUS, (GLint *)gReadBuffer);
    glUseProgram(gShaderProgramMap[6]);
    UpdateCurrentProgram(6);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_FALSE, 1, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glGetError();
    glTexImage2D(GL_TEXTURE_2D, 0, 6407, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, (const GLubyte *)&gBinaryData[32]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 9728);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 9728);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(gShaderProgramMap[3]);
    UpdateCurrentProgram(3);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 20, 0);
    glUniform1i(gUniformLocations[gCurrentProgram][0], 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const GLubyte *)&gBinaryData[48]);
    glDeleteProgram(gShaderProgramMap[6]);
    DeleteUniformLocations(gShaderProgramMap[6]);
    UpdateResourceIDBuffer(0, gBufferMap[1]);
glDeleteBuffers(1, gResourceIDBuffer);
}

void ReplayFrame3(void)
{
    eglGetError();
}

void ResetReplayContextShared(void)
{
    glBindTexture(GL_TEXTURE_2D, gTextureMap[2]);
    glUseProgram(gShaderProgramMap[3]);
    UpdateCurrentProgram(3);
    glUniform1iv(gUniformLocations[gCurrentProgram][0], 1, (const GLint *)&gBinaryData[64]);
    glUniform1iv(gUniformLocations[gCurrentProgram][0], 1, (const GLint *)&gBinaryData[80]);
}

void ResetReplayContext1(void)
{
}

void ReplayFrame4(void)
{
    eglGetError();
}

// Public Functions

void SetupReplay(void)
{
    InitReplay();
    SetupReplayContextShared();
    if (gReplayResourceMode == angle::ReplayResourceMode::All)
    {
        SetupReplayContextSharedInactive();
    }
    SetCurrentContextID(1);
    SetupReplayContext1();

}

void ResetReplay(void)
{
    ResetReplayContextShared();
    ResetReplayContext1();

    // Reset main context state
    glUseProgram(gShaderProgramMap[0]);
    UpdateCurrentProgram(0);
}

