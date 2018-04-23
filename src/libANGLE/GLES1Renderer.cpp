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
    : mContext(context),
      mGLState(state),
      mRendererProgramInitialized(false),
      mPositionAttribIndex(-1),
      mNormalAttribIndex(-1),
      mColorAttribIndex(-1),
      mPointsizeAttribIndex(-1),
      mTexcoordAttribIndexBase(-1)
{
}

GLES1Renderer::~GLES1Renderer()
{
    if (mRendererProgramInitialized)
    {
        mContext->deleteProgram(mDrawState.programState.program);
    }
}

void GLES1Renderer::prepareForDraw()
{
    if (!initializeRendererProgram())
    {
        fprintf(stderr, "%s: could not initialize renderer program. abort\n", __func__);
    }

    Context *gl = mContext;
    State *glS  = mGLState;

    const auto &gles1State = glS->gles1();

    const auto &programState = mDrawState.programState;
    auto &uniformBuffers     = mDrawState.uniformBuffers;

    if (!gles1State.isClientStateEnabled(ClientVertexArrayType::Normal))
    {
        const auto normal = gles1State.getCurrentNormal();
        gl->vertexAttrib3f(mNormalAttribIndex, normal.x(), normal.y(), normal.z());
    }

    if (!gles1State.isClientStateEnabled(ClientVertexArrayType::Color))
    {
        const auto color = gles1State.getCurrentColor();
        gl->vertexAttrib4f(mColorAttribIndex, color.red, color.green, color.blue, color.alpha);
    }

    if (!gles1State.isClientStateEnabled(ClientVertexArrayType::PointSize))
    {
        GLfloat pointSize = gles1State.mPointParameters.pointSize;
        gl->vertexAttrib1f(mPointsizeAttribIndex, pointSize);
    }

    for (int i = 0; i < kTexUnitCount; i++)
    {
        if (!gles1State.mTexCoordArrayEnabled[i])
        {
            const auto texcoord = gles1State.getCurrentTextureCoords(i);
            gl->vertexAttrib4f(mTexcoordAttribIndexBase + i, texcoord.s, texcoord.t, texcoord.r,
                               texcoord.q);
        }
    }

    {
        auto proj           = gles1State.mProjectionMatrices.back();
        auto modelview      = gles1State.mModelviewMatrices.back();
        auto modelviewInvTr = modelview.transpose().inverse();

        gl->uniformMatrix4fv(programState.projMatrixLoc, 1, GL_FALSE, proj.data());
        gl->uniformMatrix4fv(programState.modelviewMatrixLoc, 1, GL_FALSE, modelview.data());
        gl->uniformMatrix4fv(programState.modelviewInvTrLoc, 1, GL_FALSE, modelviewInvTr.data());

        for (int i = 0; i < kTexUnitCount; i++)
        {
            auto textureMatrix = gles1State.mTextureMatrices[i].back();
            memcpy(uniformBuffers.textureMatrices.data() + i * 16, textureMatrix.data(),
                   16 * sizeof(float));
        }

        gl->uniformMatrix4fv(programState.textureMatrixLoc, 4, GL_FALSE,
                             uniformBuffers.textureMatrices.data());
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

        // Fixed sampler locations---just make sure no two samplers use the same unit
        // if (programState.textureSampler0Loc != -1) {
        //     gl->uniform1i(programState.textureSampler0Loc,
        //             (tex2DEnables[0] || !texCubeEnables[0]) ? 0 : 4);
        // }
        // if (programState.textureSampler1Loc != -1) {
        //     gl->uniform1i(programState.textureSampler1Loc, (tex2DEnables[1] ||
        //     !texCubeEnables[1]) ? 1 : 5);
        // }
        // if (programState.textureSampler2Loc != -1) {
        //     gl->uniform1i(programState.textureSampler2Loc, (tex2DEnables[2] ||
        //     !texCubeEnables[2]) ? 2 : 6);
        // }
        // if (programState.textureSampler3Loc != -1) {
        //     gl->uniform1i(programState.textureSampler3Loc, (tex2DEnables[3] ||
        //     !texCubeEnables[3]) ? 3 : 7);
        // }
        // if (programState.textureCubeSampler0Loc != -1) {
        //     gl->uniform1i(programState.textureCubeSampler0Loc, texCubeEnables[0] ? 0 : 4);
        // }
        // if (programState.textureCubeSampler1Loc != -1) {
        //     gl->uniform1i(programState.textureCubeSampler1Loc, texCubeEnables[1] ? 1 : 5);
        // }
        // if (programState.textureCubeSampler2Loc != -1) {
        //     gl->uniform1i(programState.textureCubeSampler2Loc, texCubeEnables[2] ? 2 : 6);
        // }
        // if (programState.textureCubeSampler3Loc != -1) {
        //     gl->uniform1i(programState.textureCubeSampler3Loc, texCubeEnables[3] ? 3 : 7);
        // }

        gl->uniform1i(programState.shadeModelFlatLoc, gles1State.mShadeModel == ShadingModel::Flat);
    }
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
            return 0;
        }
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
            fprintf(stderr, "GLES1Renderer::%s: program link failed.\n", __func__);
            gl->getProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
            std::vector<char> infoLog(infoLogLength, 0);
            gl->getProgramInfoLog(program, infoLogLength - 1, nullptr, infoLog.data());
            fprintf(stderr, "GLES1Renderer::%s: Info log: %s\n", __func__, infoLog.data());
            return 0;
        }
    }
    return program;
}

bool GLES1Renderer::initializeRendererProgram()
{

    if (mRendererProgramInitialized)
    {
        return true;
    }

    GLuint vertexShader = compileShader(ShaderType::Vertex, kGLES1DrawVShader);

    std::string fshadersrc = "";
    fshadersrc += kGLES1DrawFShaderHeader;
    fshadersrc += kGLES1DrawFShaderUniformDefs;
    fshadersrc += kGLES1DrawFShaderFunctions;
    fshadersrc += kGLES1DrawFShaderMain;

    GLuint fragmentShader = compileShader(ShaderType::Fragment, fshadersrc.c_str());

    if (!vertexShader || !fragmentShader)
    {
        return false;
    }

    std::unordered_map<GLint, std::string> attribLocs;

    mPositionAttribIndex     = mContext->vertexArrayIndex(ClientVertexArrayType::Vertex);
    mNormalAttribIndex       = mContext->vertexArrayIndex(ClientVertexArrayType::Normal);
    mColorAttribIndex        = mContext->vertexArrayIndex(ClientVertexArrayType::Color);
    mPointsizeAttribIndex    = mContext->vertexArrayIndex(ClientVertexArrayType::PointSize);
    mTexcoordAttribIndexBase = mContext->vertexArrayIndex(ClientVertexArrayType::TextureCoord);

    attribLocs[mPositionAttribIndex]  = "pos";
    attribLocs[mNormalAttribIndex]    = "normal";
    attribLocs[mColorAttribIndex]     = "color";
    attribLocs[mPointsizeAttribIndex] = "pointsize";

    for (int i = 0; i < kTexUnitCount; i++)
    {
        std::stringstream ss;
        ss << "texcoord" << i;
        attribLocs[mTexcoordAttribIndexBase + i] = ss.str();
    }

    GLuint program = linkProgram(vertexShader, fragmentShader, attribLocs);

    if (!program)
    {
        return false;
    }

    Context *gl = mContext;

    gl->deleteShader(vertexShader);
    gl->deleteShader(fragmentShader);

    auto &programState = mDrawState.programState;

    programState.program = program;

    programState.projMatrixLoc          = gl->getUniformLocation(program, "projection");
    programState.modelviewMatrixLoc     = gl->getUniformLocation(program, "modelview");
    programState.textureMatrixLoc       = gl->getUniformLocation(program, "texture_matrix");
    programState.modelviewInvTrLoc      = gl->getUniformLocation(program, "modelview_invtr");
    programState.textureSampler0Loc     = gl->getUniformLocation(program, "tex_sampler0");
    programState.textureCubeSampler0Loc = gl->getUniformLocation(program, "tex_cube_sampler0");
    programState.textureSampler1Loc     = gl->getUniformLocation(program, "tex_sampler1");
    programState.textureCubeSampler1Loc = gl->getUniformLocation(program, "tex_cube_sampler1");
    programState.textureSampler2Loc     = gl->getUniformLocation(program, "tex_sampler2");
    programState.textureCubeSampler2Loc = gl->getUniformLocation(program, "tex_cube_sampler2");
    programState.textureSampler3Loc     = gl->getUniformLocation(program, "tex_sampler3");
    programState.textureCubeSampler3Loc = gl->getUniformLocation(program, "tex_cube_sampler3");

    programState.shadeModelFlatLoc = gl->getUniformLocation(program, "shade_model_flat");

    programState.enableTexture2DLoc = gl->getUniformLocation(program, "enable_texture_2d");

    programState.enableTextureCubeMapLoc =
        gl->getUniformLocation(program, "enable_texture_cube_map");

    gl->useProgram(program);

    if (programState.textureSampler0Loc != -1)
    {
        gl->uniform1i(programState.textureSampler0Loc, 0);
    }
    if (programState.textureSampler1Loc != -1)
    {
        gl->uniform1i(programState.textureSampler1Loc, 1);
    }
    if (programState.textureSampler2Loc != -1)
    {
        gl->uniform1i(programState.textureSampler2Loc, 2);
    }
    if (programState.textureSampler3Loc != -1)
    {
        gl->uniform1i(programState.textureSampler3Loc, 3);
    }
    if (programState.textureCubeSampler0Loc != -1)
    {
        gl->uniform1i(programState.textureCubeSampler0Loc, 4);
    }
    if (programState.textureCubeSampler1Loc != -1)
    {
        gl->uniform1i(programState.textureCubeSampler1Loc, 5);
    }
    if (programState.textureCubeSampler2Loc != -1)
    {
        gl->uniform1i(programState.textureCubeSampler2Loc, 6);
    }
    if (programState.textureCubeSampler3Loc != -1)
    {
        gl->uniform1i(programState.textureCubeSampler3Loc, 7);
    }

    fprintf(stderr, "%s: succesful initalzied renderer program\n", __func__);

    mRendererProgramInitialized = true;
    return true;
}

}  // namespace gl
