//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
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
#include "libANGLE/GLES1Shaders.inc"
#include "libANGLE/State.h"

namespace gl
{

GLES1Renderer::GLES1Renderer() : mRendererProgramInitialized(false)
{
}

void GLES1Renderer::deinitialize(Context *gl)
{
    if (mRendererProgramInitialized)
    {
        gl->deleteProgram(mProgramState.program);
    }
}

GLES1Renderer::~GLES1Renderer() = default;

Error GLES1Renderer::prepareForDraw(Context *gl, const State *glS)
{
    ANGLE_TRY(initializeRendererProgram(gl));

    const GLES1State &gles1State = glS->gles1();

    const GLES1ProgramState &programState = mProgramState;
    GLES1UniformBuffers &uniformBuffers   = mUniformBuffers;

    if (!gles1State.isClientStateEnabled(ClientVertexArrayType::Normal))
    {
        const angle::Vector3 normal = gles1State.getCurrentNormal();
        gl->vertexAttrib3f(programState.normalAttribIndex, normal.x(), normal.y(), normal.z());
    }

    if (!gles1State.isClientStateEnabled(ClientVertexArrayType::Color))
    {
        const ColorF color = gles1State.getCurrentColor();
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
            const TextureCoordF texcoord = gles1State.getCurrentTextureCoords(i);
            gl->vertexAttrib4f(programState.texcoordAttribIndexBase + i, texcoord.s, texcoord.t,
                               texcoord.r, texcoord.q);
        }
    }

    {
        angle::Mat4 proj = gles1State.mProjectionMatrices.back();
        gl->uniformMatrix4fv(programState.projMatrixLoc, 1, GL_FALSE, proj.data());

        angle::Mat4 modelview = gles1State.mModelviewMatrices.back();
        gl->uniformMatrix4fv(programState.modelviewMatrixLoc, 1, GL_FALSE, modelview.data());

        angle::Mat4 modelviewInvTr = modelview.transpose().inverse();
        gl->uniformMatrix4fv(programState.modelviewInvTrLoc, 1, GL_FALSE, modelviewInvTr.data());

        Mat4Uniform *textureMatrixBuffer = uniformBuffers.textureMatrices.data();

        for (int i = 0; i < kTexUnitCount; i++)
        {
            angle::Mat4 textureMatrix = gles1State.mTextureMatrices[i].back();
            memcpy(textureMatrixBuffer + i, textureMatrix.data(), sizeof(Mat4Uniform));
        }

        gl->uniformMatrix4fv(programState.textureMatrixLoc, 4, GL_FALSE,
                             (float *)uniformBuffers.textureMatrices.data());
    }

    {
        std::array<GLint, kTexUnitCount> &tex2DEnables   = uniformBuffers.tex2DEnables;
        std::array<GLint, kTexUnitCount> &texCubeEnables = uniformBuffers.texCubeEnables;

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

Error GLES1Renderer::compileShader(Context *gl,
                                   ShaderType shaderType,
                                   const char *src,
                                   GLuint *shaderOut)
{
    GLuint shader = gl->createShader(shaderType);
    gl->shaderSource(shader, 1, &src, nullptr);
    gl->compileShader(shader);

    *shaderOut = shader;
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
            return InvalidValue();
        }
        ASSERT(status);
    }

    return NoError();
}

Error GLES1Renderer::linkProgram(Context *gl,
                                 GLuint vertexShader,
                                 GLuint fragmentShader,
                                 const std::unordered_map<GLint, std::string> &attribLocs,
                                 GLuint *programOut)
{
    GLuint program = gl->createProgram();
    *programOut    = program;

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
            return InvalidValue();
        }
        ASSERT(status);
    }

    gl->detachShader(program, vertexShader);
    gl->detachShader(program, fragmentShader);

    return NoError();
}

Error GLES1Renderer::initializeRendererProgram(Context *gl)
{
    if (mRendererProgramInitialized)
    {
        return NoError();
    }

    GLuint vertexShader;
    GLuint fragmentShader;

    ANGLE_TRY(compileShader(gl, ShaderType::Vertex, kGLES1DrawVShader, &vertexShader));

    std::stringstream fragmentStream;
    fragmentStream << kGLES1DrawFShaderHeader;
    fragmentStream << kGLES1DrawFShaderUniformDefs;
    fragmentStream << kGLES1DrawFShaderFunctions;
    fragmentStream << kGLES1DrawFShaderMain;

    ANGLE_TRY(
        compileShader(gl, ShaderType::Fragment, fragmentStream.str().c_str(), &fragmentShader));

    std::unordered_map<GLint, std::string> attribLocs;

    GLES1ProgramState &programState = mProgramState;

    programState.positionAttribIndex  = gl->vertexArrayIndex(ClientVertexArrayType::Vertex);
    programState.normalAttribIndex    = gl->vertexArrayIndex(ClientVertexArrayType::Normal);
    programState.colorAttribIndex     = gl->vertexArrayIndex(ClientVertexArrayType::Color);
    programState.pointsizeAttribIndex = gl->vertexArrayIndex(ClientVertexArrayType::PointSize);
    programState.texcoordAttribIndexBase =
        gl->vertexArrayIndex(ClientVertexArrayType::TextureCoord);

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

    ANGLE_TRY(linkProgram(gl, vertexShader, fragmentShader, attribLocs, &programState.program));

    gl->deleteShader(vertexShader);
    gl->deleteShader(fragmentShader);

    programState.projMatrixLoc      = gl->getUniformLocation(programState.program, "projection");
    programState.modelviewMatrixLoc = gl->getUniformLocation(programState.program, "modelview");
    programState.textureMatrixLoc = gl->getUniformLocation(programState.program, "texture_matrix");
    programState.modelviewInvTrLoc =
        gl->getUniformLocation(programState.program, "modelview_invtr");

    for (int i = 0; i < kTexUnitCount; i++)
    {
        std::stringstream ss2d;
        std::stringstream sscube;

        ss2d << "tex_sampler" << i;
        sscube << "tex_cube_sampler" << i;

        programState.tex2DSamplerLocs[i] =
            gl->getUniformLocation(programState.program, ss2d.str().c_str());
        programState.texCubeSamplerLocs[i] =
            gl->getUniformLocation(programState.program, sscube.str().c_str());
    }

    programState.shadeModelFlatLoc =
        gl->getUniformLocation(programState.program, "shade_model_flat");

    programState.enableTexture2DLoc =
        gl->getUniformLocation(programState.program, "enable_texture_2d");
    programState.enableTextureCubeMapLoc =
        gl->getUniformLocation(programState.program, "enable_texture_cube_map");

    gl->useProgram(programState.program);

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
    return NoError();
}

}  // namespace gl
