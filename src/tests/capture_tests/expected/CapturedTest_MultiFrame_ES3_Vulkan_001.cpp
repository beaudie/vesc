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

void ReplayFrame1(void)
{
    eglMakeCurrent(gEGLDisplay, gSurfaceMap2[1], gSurfaceMap2[1], gContextMap2[1]);
    eglGetError();
    glViewport(0, 0, 128, 128);
    glGenFramebuffers(2, (GLuint *)gReadBuffer);
    UpdateFramebufferID(1, 0);
    UpdateFramebufferID(2, 4);
    glGetError();
}

void ReplayFrame2(void)
{
    eglGetError();
    glClearColor(0.25, 0.5, 0.5, 0.5);
    glClear(GL_COLOR_BUFFER_BIT);
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void *)gReadBuffer);
    glGetError();
}

void ReplayFrame3(void)
{
    eglGetError();
    glGenBuffers(1, (GLuint *)gReadBuffer);
    UpdateBufferID(1, 0);
    glBindBuffer(GL_ARRAY_BUFFER, gBufferMap[1]);
    CreateProgram(1);
    CreateShader(GL_VERTEX_SHADER, 2);
    glShaderSource(gShaderProgramMap[2], 1, glShaderSource_string_0, (const GLint *)&gBinaryData[0]);
    glCompileShader(gShaderProgramMap[2]);
    glGetShaderiv(gShaderProgramMap[2], GL_COMPILE_STATUS, (GLint *)gReadBuffer);
    CreateShader(GL_FRAGMENT_SHADER, 3);
    glShaderSource(gShaderProgramMap[3], 1, glShaderSource_string_1, (const GLint *)&gBinaryData[16]);
    glCompileShader(gShaderProgramMap[3]);
    glGetShaderiv(gShaderProgramMap[3], GL_COMPILE_STATUS, (GLint *)gReadBuffer);
    glAttachShader(gShaderProgramMap[1], gShaderProgramMap[2]);
    glDeleteShader(gShaderProgramMap[2]);
    glAttachShader(gShaderProgramMap[1], gShaderProgramMap[3]);
    glDeleteShader(gShaderProgramMap[3]);
    glLinkProgram(gShaderProgramMap[1]);
    glGetError();
    glGetProgramiv(gShaderProgramMap[1], GL_LINK_STATUS, (GLint *)gReadBuffer);
    glBindAttribLocation(gShaderProgramMap[1], 0, "attr1");
    glLinkProgram(gShaderProgramMap[1]);
    glGetError();
    glGetProgramiv(gShaderProgramMap[1], GL_LINK_STATUS, (GLint *)gReadBuffer);
    glUseProgram(gShaderProgramMap[1]);
    UpdateCurrentProgram(1);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_FALSE, 1, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glGetError();
    glDeleteProgram(gShaderProgramMap[1]);
    DeleteUniformLocations(gShaderProgramMap[1]);
    UpdateResourceIDBuffer(0, gBufferMap[1]);
glDeleteBuffers(1, gResourceIDBuffer);
}

void ReplayFrame4(void)
{
    eglGetError();
    UpdateResourceIDBuffer(0, gFramebufferMap[1]);
UpdateResourceIDBuffer(1, gFramebufferMap[2]);
glDeleteFramebuffers(2, gResourceIDBuffer);
}

// Public Functions

void SetupReplay(void)
{
    InitReplay();
}

