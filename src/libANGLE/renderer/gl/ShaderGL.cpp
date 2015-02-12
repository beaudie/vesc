//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderGL.cpp: Implements the class methods for ShaderGL.

#include "libANGLE/renderer/gl/ShaderGL.h"

#include "common/debug.h"
#include "libANGLE/Compiler.h"
#include "libANGLE/renderer/gl/CompilerGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"

namespace rx
{

ShaderGL::ShaderGL(GLenum type, const FunctionsGL *functions)
    : ShaderImpl(),
      mFunctions(functions),
      mType(type),
      mShaderID(0)
{
    ASSERT(mFunctions);
}

ShaderGL::~ShaderGL()
{
    if (mShaderID != 0)
    {
        mFunctions->deleteShader(mShaderID);
    }
}

bool ShaderGL::compile(gl::Compiler *compiler, const std::string &source)
{
    CompilerGL *compilerGL = GetImplAs<CompilerGL>(compiler);
    ShHandle compilerHandle = compilerGL->getCompilerHandle(mType);

    int compileOptions = (SH_OBJECT_CODE | SH_VARIABLES);
    const char* sourceCString = source.c_str();
    if (!ShCompile(compilerHandle, &sourceCString, 1, compileOptions))
    {
        mInfoLog = ShGetInfoLog(compilerHandle);
        TRACE("\n%s", mInfoLog.c_str());
        return false;
    }

    mTranslatedSource = ShGetObjectCode(compilerHandle);
    const char* translatedSourceCString = mTranslatedSource.c_str();

    // Generate a shader object and set the source
    mShaderID = mFunctions->createShader(mType);
    mFunctions->shaderSource(mShaderID, 1, &translatedSourceCString, nullptr);
    mFunctions->compileShader(mShaderID);

    // Check for compile errors from the native driver
    GLint compileStatus = GL_FALSE;
    mFunctions->getShaderiv(mShaderID, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE)
    {
        // Compilation failed, put the error into the info log
        GLint infoLogLength = 0;
        mFunctions->getShaderiv(mShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<char> buf(infoLogLength);
        mFunctions->getShaderInfoLog(mShaderID, infoLogLength, nullptr, &buf[0]);

        mFunctions->deleteShader(mShaderID);
        mShaderID = 0;

        mInfoLog = &buf[0];
        TRACE("\n%s", mInfoLog.c_str());
        return false;
    }

    return true;
}

std::string ShaderGL::getDebugInfo() const
{
    UNIMPLEMENTED();
    return std::string();
}

}
