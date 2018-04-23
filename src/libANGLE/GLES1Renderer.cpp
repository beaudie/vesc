//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// GLES1Renderer.cpp: Implements the GLES1Renderer renderer.

#include "libANGLE/GLES1Renderer.h"

#include <string.h>
#include <iterator>
#include <sstream>
#include <vector>

#include "libANGLE/Context.h"
#include "libANGLE/GLES1Shaders.h"
#include "libANGLE/State.h"

namespace gl
{

GLES1Renderer::GLES1Renderer(Context *context, State *state)
    : mContext(context), mGLState(state), mRendererProgramInitialized(false)
{
}

GLES1Renderer::~GLES1Renderer()
{
    if (mRendererProgramInitialized)
    {
        mContext->deleteProgram(mProgramState.program);
    }
}

Error GLES1Renderer::prepareForDraw()
{
    if (!initializeRendererProgram())
    {
        return InvalidOperation();
    }

    Context *gl = mContext;
    State *glS  = mGLState;

    const auto &gles1State = glS->gles1();

    const auto &programState = mProgramState;
    auto &uniformBuffers     = mUniformBuffers;

    if (!gles1State.isClientStateEnabled(ClientVertexArrayType::Normal))
    {
        const auto normal = gles1State.getCurrentNormal();
        gl->vertexAttrib3f(programState.normalAttribIndex, normal.x(), normal.y(), normal.z());
    }

    if (!gles1State.isClientStateEnabled(ClientVertexArrayType::Color))
    {
        const auto color = gles1State.getCurrentColor();
        gl->vertexAttrib4f(programState.colorAttribIndex, color.red, color.green, color.blue,
                           color.alpha);
    }

    if (!gles1State.isClientStateEnabled(ClientVertexArrayType::PointSize))
    {
        GLfloat pointSize = gles1State.mPointParameters.pointSize;
        gl->vertexAttrib1f(programState.pointsizeAttribIndex, pointSize);
    }

    for (int i = 0; i < kTexUnitCount; i++)
    {
        if (!gles1State.mTexCoordArrayEnabled[i])
        {
            const auto texcoord = gles1State.getCurrentTextureCoords(i);
            gl->vertexAttrib4f(programState.texcoordAttribIndexBase + i, texcoord.s, texcoord.t,
                               texcoord.r, texcoord.q);
        }
    }

    {
        auto proj = gles1State.mProjectionMatrices.back();
        gl->uniformMatrix4fv(programState.projMatrixLoc, 1, GL_FALSE, proj.data());

        auto modelview = gles1State.mModelviewMatrices.back();
        gl->uniformMatrix4fv(programState.modelviewMatrixLoc, 1, GL_FALSE, modelview.data());

        auto modelviewInvTr = modelview.transpose().inverse();
        gl->uniformMatrix4fv(programState.modelviewInvTrLoc, 1, GL_FALSE, modelviewInvTr.data());

        Mat4Uniform *textureMatrixBuffer = uniformBuffers.textureMatrices.data();

        for (int i = 0; i < kTexUnitCount; i++)
        {
            auto textureMatrix = gles1State.mTextureMatrices[i].back();
            memcpy(textureMatrixBuffer + i, textureMatrix.data(), sizeof(Mat4Uniform));
        }

        gl->uniformMatrix4fv(programState.textureMatrixLoc, 4, GL_FALSE,
                             (float *)uniformBuffers.textureMatrices.data());
    }

    {
        auto &tex2DEnables   = uniformBuffers.tex2DEnables;
        auto &texCubeEnables = uniformBuffers.texCubeEnables;

        for (int i = 0; i < kTexUnitCount; i++)
        {
            // GL_OES_cube_map allows only one of TEXTURE_2D / TEXTURE_CUBE_MAP
            // to be enabled per unit, thankfully. From the extension text:
            //
            //  --  Section 3.8.10 "Texture Application"
            //
            //      Replace the beginning sentences of the first paragraph (page 138)
            //      with:
            //
            //      "Texturing is enabled or disabled using the generic Enable
            //      and Disable commands, respectively, with the symbolic constants
            //      TEXTURE_2D or TEXTURE_CUBE_MAP_OES to enable the two-dimensional or cube
            //      map texturing respectively.  If the cube map texture and the two-
            //      dimensional texture are enabled, then cube map texturing is used.  If
            //      texturing is disabled, a rasterized fragment is passed on unaltered to the
            //      next stage of the GL (although its texture coordinates may be discarded).
            //      Otherwise, a texture value is found according to the parameter values of
            //      the currently bound texture image of the appropriate dimensionality.

            texCubeEnables[i] = gles1State.isTextureTargetEnabled(i, TextureType::CubeMap);
            tex2DEnables[i] =
                !texCubeEnables[i] && (gles1State.isTextureTargetEnabled(i, TextureType::_2D));
        }

        gl->uniform1iv(programState.enableTexture2DLoc, kTexUnitCount, tex2DEnables.data());
        gl->uniform1iv(programState.enableTextureCubeMapLoc, kTexUnitCount, texCubeEnables.data());

        gl->uniform1i(programState.shadeModelFlatLoc, gles1State.mShadeModel == ShadingModel::Flat);
    }

    return NoError();
}

GLuint GLES1Renderer::compileShader(ShaderType shaderType, const char *src)
{
    Context *gl = mContext;

    GLuint shader = gl->createShader(shaderType);
    gl->shaderSource(shader, 1, &src, nullptr);
    gl->compileShader(shader);

    {
        GLint status;
        gl->getShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (!status)
        {
            GLint infoLogLength;
            fprintf(stderr, "GLES1Renderer::%s: shader compile failed.\n", __func__);
            gl->getShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
            std::vector<char> infoLog(infoLogLength, 0);
            gl->getShaderInfoLog(shader, infoLogLength - 1, nullptr, infoLog.data());
            fprintf(stderr, "GLES1Renderer::%s: Info log: %s\n", __func__, infoLog.data());
            ERR() << "GLES1Renderer shader compile failed. Source: " << src
                  << " Info log: " << infoLog.data();
            return 0;
        }
        ASSERT(status);
    }

    return shader;
}

GLuint GLES1Renderer::linkProgram(GLuint vertexShader,
                                  GLuint fragmentShader,
                                  const std::unordered_map<GLint, std::string> &attribLocs)
{
    Context *gl = mContext;

    GLuint program = gl->createProgram();
    gl->attachShader(program, vertexShader);
    gl->attachShader(program, fragmentShader);

    for (auto it : attribLocs)
    {
        GLint index             = it.first;
        const std::string &name = it.second;
        gl->bindAttribLocation(program, index, name.c_str());
    }

    gl->linkProgram(program);

    {
        GLint status;
        gl->getProgramiv(program, GL_LINK_STATUS, &status);
        if (!status)
        {
            GLint infoLogLength;
            gl->getProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
            std::vector<char> infoLog(infoLogLength, 0);
            gl->getProgramInfoLog(program, infoLogLength - 1, nullptr, infoLog.data());
            ERR() << "GLES1Renderer program link failed. Info log: " << infoLog.data();
            return 0;
        }
        ASSERT(status);
    }

    gl->detachShader(program, vertexShader);
    gl->detachShader(program, fragmentShader);

    return program;
}

bool GLES1Renderer::initializeRendererProgram()
{

    if (mRendererProgramInitialized)
    {
        return true;
    }

    GLuint vertexShader = compileShader(ShaderType::Vertex, kGLES1DrawVShader);

    std::stringstream fragmentStream;
    fragmentStream << kGLES1DrawFShaderHeader;
    fragmentStream << kGLES1DrawFShaderUniformDefs;
    fragmentStream << kGLES1DrawFShaderFunctions;
    fragmentStream << kGLES1DrawFShaderMain;

    GLuint fragmentShader = compileShader(ShaderType::Fragment, fragmentStream.str().c_str());

    if (!vertexShader || !fragmentShader)
    {
        return false;
    }

    std::unordered_map<GLint, std::string> attribLocs;

    auto &programState = mProgramState;

    programState.positionAttribIndex = mContext->vertexArrayIndex(ClientVertexArrayType::Vertex);
    programState.normalAttribIndex   = mContext->vertexArrayIndex(ClientVertexArrayType::Normal);
    programState.colorAttribIndex    = mContext->vertexArrayIndex(ClientVertexArrayType::Color);
    programState.pointsizeAttribIndex =
        mContext->vertexArrayIndex(ClientVertexArrayType::PointSize);
    programState.texcoordAttribIndexBase =
        mContext->vertexArrayIndex(ClientVertexArrayType::TextureCoord);

    attribLocs[programState.positionAttribIndex]  = "pos";
    attribLocs[programState.normalAttribIndex]    = "normal";
    attribLocs[programState.colorAttribIndex]     = "color";
    attribLocs[programState.pointsizeAttribIndex] = "pointsize";

    for (int i = 0; i < kTexUnitCount; i++)
    {
        std::stringstream ss;
        ss << "texcoord" << i;
        attribLocs[programState.texcoordAttribIndexBase + i] = ss.str();
    }

    GLuint program = linkProgram(vertexShader, fragmentShader, attribLocs);

    if (!program)
    {
        return false;
    }

    Context *gl = mContext;

    gl->deleteShader(vertexShader);
    gl->deleteShader(fragmentShader);

    programState.program = program;

    programState.projMatrixLoc      = gl->getUniformLocation(program, "projection");
    programState.modelviewMatrixLoc = gl->getUniformLocation(program, "modelview");
    programState.textureMatrixLoc   = gl->getUniformLocation(program, "texture_matrix");
    programState.modelviewInvTrLoc  = gl->getUniformLocation(program, "modelview_invtr");

    for (int i = 0; i < kTexUnitCount; i++)
    {
        std::stringstream ss2d;
        std::stringstream sscube;

        ss2d << "tex_sampler" << i;
        sscube << "tex_cube_sampler" << i;

        programState.tex2DSamplerLocs[i]   = gl->getUniformLocation(program, ss2d.str().c_str());
        programState.texCubeSamplerLocs[i] = gl->getUniformLocation(program, sscube.str().c_str());
    }

    programState.shadeModelFlatLoc = gl->getUniformLocation(program, "shade_model_flat");

    programState.enableTexture2DLoc = gl->getUniformLocation(program, "enable_texture_2d");
    programState.enableTextureCubeMapLoc =
        gl->getUniformLocation(program, "enable_texture_cube_map");

    gl->useProgram(program);

    for (int i = 0; i < kTexUnitCount; i++)
    {
        if (programState.tex2DSamplerLocs[i] != -1)
        {
            gl->uniform1i(programState.tex2DSamplerLocs[i], i);
        }
        if (programState.texCubeSamplerLocs[i] != -1)
        {
            gl->uniform1i(programState.texCubeSamplerLocs[i], i + kTexUnitCount);
        }
    }

    mRendererProgramInitialized = true;
    return true;
}

}  // namespace gl
