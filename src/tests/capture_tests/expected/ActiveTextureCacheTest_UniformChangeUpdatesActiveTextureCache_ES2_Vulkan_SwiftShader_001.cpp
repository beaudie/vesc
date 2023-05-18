#include "ActiveTextureCacheTest_UniformChangeUpdatesActiveTextureCache_ES2_Vulkan_SwiftShader.h"
#include "angle_trace_gl.h"

const char *const glShaderSource_string_0[] = { 
"precision highp float;\n"
"\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(0.0, 0.0, 0.0, 0.0);\n"
"}\n"
"",
};
const char *const glShaderSource_string_1[] = { 
"precision highp float;\n"
"uniform sampler2D tex2D;\n"
"uniform samplerCube texCube;\n"
"\n"
"void main()\n"
"{\n"
"   gl_FragColor = texture2D(tex2D, vec2(0.0, 0.0)) + textureCube(texCube, vec3(0.0, 0.0, 0.0));\n"
"}\n"
"",
};

// Private Functions

void ReplayFrame1(void)
{
    eglMakeCurrent(gEGLDisplay, gSurfaceMap2[1], gSurfaceMap2[1], gContextMap2[1]);
    eglGetError();
    glViewport(0, 0, 128, 128);
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
    UpdateUniformLocation(1, "tex2D", 0, 1);
    UpdateUniformLocation(1, "texCube", 1, 1);
    glGetError();
    glGetProgramiv(gShaderProgramMap[1], GL_LINK_STATUS, (GLint *)gReadBuffer);
    glGetUniformLocation(gShaderProgramMap[1], "tex2D");
    glGetUniformLocation(gShaderProgramMap[1], "texCube");
    glUseProgram(gShaderProgramMap[1]);
    UpdateCurrentProgram(1);
    glGenTextures(1, (GLuint *)gReadBuffer);
    UpdateTextureID(1, 0);
    glBindTexture(GL_TEXTURE_2D, gTextureMap[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, 6408, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glGenTextures(1, (GLuint *)gReadBuffer);
    UpdateTextureID(2, 0);
    glBindTexture(GL_TEXTURE_2D, gTextureMap[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, 6408, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, gTextureMap[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gTextureMap[1]);
    glUniform1i(gUniformLocations[gCurrentProgram][0], 1);
    glUniform1i(gUniformLocations[gCurrentProgram][1], 1);
    glBindTexture(GL_TEXTURE_2D, gTextureMap[2]);
    UpdateResourceIDBuffer(0, gTextureMap[1]);
glDeleteTextures(1, gResourceIDBuffer);
}

void ReplayFrame2(void)
{
    eglGetError();
    glDeleteProgram(gShaderProgramMap[1]);
    DeleteUniformLocations(gShaderProgramMap[1]);
}

// Public Functions

void SetupReplay(void)
{
    InitReplay();
    gEGLDisplay = eglGetCurrentDisplay();
}

