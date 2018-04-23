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
      mPositionAttribIndex(-1),
      mNormalAttribIndex(-1),
      mColorAttribIndex(-1),
      mPointsizeAttribIndex(-1),
      mTexcoordAttribIndexBase(-1)
{

    Context *gl = mContext;

    std::vector<const char *> srcs(1);

    GLint stat;
    std::vector<char> buf(4096, 0);

    GLuint drawVShader = gl->createShader(ShaderType::Vertex);
    srcs[0]            = kGLES1DrawVShader;

    gl->shaderSource(drawVShader, 1, (const GLchar *const *)(srcs.data()), nullptr);
    gl->compileShader(drawVShader);
    gl->getShaderiv(drawVShader, GL_COMPILE_STATUS, &stat);

    if (!stat)
    {
        fprintf(stderr, "%s: vertex shader compile failed.\n", __func__);
        gl->getShaderInfoLog(drawVShader, 4095, nullptr, buf.data());
        fprintf(stderr, "%s: info log: %s\n", __func__, buf.data());
    }

    GLuint drawFShader     = gl->createShader(ShaderType::Fragment);
    std::string fshadersrc = "";
    fshadersrc += kGLES1DrawFShaderHeader;
    fshadersrc += kGLES1DrawFShaderUniformDefs;
    fshadersrc += kGLES1DrawFShaderFunctions;
    fshadersrc += kGLES1DrawFShaderMain;
    srcs[0] = fshadersrc.c_str();

    gl->shaderSource(drawFShader, 1, (const GLchar *const *)(srcs.data()), nullptr);
    gl->compileShader(drawFShader);
    gl->getShaderiv(drawFShader, GL_COMPILE_STATUS, &stat);

    if (!stat)
    {
        fprintf(stderr, "%s: fragment shader compile failed.\n", __func__);
        gl->getShaderInfoLog(drawFShader, 4095, nullptr, buf.data());
        fprintf(stderr, "%s: info log: %s\n", __func__, buf.data());
    }

    auto &programState = mDrawState.programState;

    programState.program = gl->createProgram();
    gl->attachShader(programState.program, drawVShader);
    gl->attachShader(programState.program, drawFShader);

    mPositionAttribIndex     = mContext->vertexArrayIndex(ClientVertexArrayType::Vertex);
    mNormalAttribIndex       = mContext->vertexArrayIndex(ClientVertexArrayType::Normal);
    mColorAttribIndex        = mContext->vertexArrayIndex(ClientVertexArrayType::Color);
    mPointsizeAttribIndex    = mContext->vertexArrayIndex(ClientVertexArrayType::PointSize);
    mTexcoordAttribIndexBase = mContext->vertexArrayIndex(ClientVertexArrayType::TextureCoord);

    gl->bindAttribLocation(programState.program, mPositionAttribIndex, "pos");
    gl->bindAttribLocation(programState.program, mNormalAttribIndex, "normal");
    gl->bindAttribLocation(programState.program, mColorAttribIndex, "color");
    gl->bindAttribLocation(programState.program, mPointsizeAttribIndex, "pointsize");

    gl->bindAttribLocation(programState.program, mTexcoordAttribIndexBase, "texcoord0");
    gl->bindAttribLocation(programState.program, mTexcoordAttribIndexBase + 1, "texcoord1");
    gl->bindAttribLocation(programState.program, mTexcoordAttribIndexBase + 2, "texcoord2");
    gl->bindAttribLocation(programState.program, mTexcoordAttribIndexBase + 3, "texcoord3");

    gl->linkProgram(programState.program);

    gl->getProgramiv(programState.program, GL_LINK_STATUS, &stat);

    if (!stat)
    {
        fprintf(stderr, "%s: fragment shader link failed.\n", __func__);
        gl->getProgramInfoLog(programState.program, 4095, nullptr, buf.data());
        fprintf(stderr, "%s: info log: %s\n", __func__, buf.data());
        gl->deleteProgram(programState.program);
        programState.program = 0;
    }

    gl->deleteShader(drawFShader);
    gl->deleteShader(drawVShader);

    {
        GLuint prog = programState.program;

        programState.projMatrixLoc          = gl->getUniformLocation(prog, "projection");
        programState.modelviewMatrixLoc     = gl->getUniformLocation(prog, "modelview");
        programState.textureMatrixLoc       = gl->getUniformLocation(prog, "texture_matrix");
        programState.modelviewInvTrLoc      = gl->getUniformLocation(prog, "modelview_invtr");
        programState.textureSampler0Loc     = gl->getUniformLocation(prog, "tex_sampler0");
        programState.textureCubeSampler0Loc = gl->getUniformLocation(prog, "tex_cube_sampler0");
        programState.textureSampler1Loc     = gl->getUniformLocation(prog, "tex_sampler1");
        programState.textureCubeSampler1Loc = gl->getUniformLocation(prog, "tex_cube_sampler1");
        programState.textureSampler2Loc     = gl->getUniformLocation(prog, "tex_sampler2");
        programState.textureCubeSampler2Loc = gl->getUniformLocation(prog, "tex_cube_sampler2");
        programState.textureSampler3Loc     = gl->getUniformLocation(prog, "tex_sampler3");
        programState.textureCubeSampler3Loc = gl->getUniformLocation(prog, "tex_cube_sampler3");

        programState.shadeModelFlatLoc = gl->getUniformLocation(prog, "shade_model_flat");

        programState.enableTexture2DLoc = gl->getUniformLocation(prog, "enable_texture_2d");

        programState.enableTextureCubeMapLoc =
            gl->getUniformLocation(prog, "enable_texture_cube_map");

        gl->useProgram(prog);
        gl->uniform1i(programState.textureSampler0Loc, 0);
        gl->uniform1i(programState.textureSampler1Loc, 1);
        gl->uniform1i(programState.textureSampler2Loc, 2);
        gl->uniform1i(programState.textureSampler3Loc, 3);
        gl->uniform1i(programState.textureCubeSampler0Loc, 4);
        gl->uniform1i(programState.textureCubeSampler1Loc, 5);
        gl->uniform1i(programState.textureCubeSampler2Loc, 6);
        gl->uniform1i(programState.textureCubeSampler2Loc, 7);
    }
}

GLES1Renderer::~GLES1Renderer()
{
    if (mDrawState.programState.program)
    {
        mContext->deleteProgram(mDrawState.programState.program);
    }
}

void GLES1Renderer::prepareForDraw()
{

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
        gl->uniform1i(programState.textureSampler0Loc,
                      (tex2DEnables[0] || !texCubeEnables[0]) ? 0 : 4);
        gl->uniform1i(programState.textureSampler1Loc,
                      (tex2DEnables[1] || !texCubeEnables[1]) ? 1 : 5);
        gl->uniform1i(programState.textureSampler2Loc,
                      (tex2DEnables[2] || !texCubeEnables[2]) ? 2 : 6);
        gl->uniform1i(programState.textureSampler3Loc,
                      (tex2DEnables[3] || !texCubeEnables[3]) ? 3 : 7);
        gl->uniform1i(programState.textureCubeSampler0Loc, texCubeEnables[0] ? 0 : 4);
        gl->uniform1i(programState.textureCubeSampler1Loc, texCubeEnables[1] ? 1 : 5);
        gl->uniform1i(programState.textureCubeSampler2Loc, texCubeEnables[2] ? 2 : 6);
        gl->uniform1i(programState.textureCubeSampler3Loc, texCubeEnables[3] ? 3 : 7);

        gl->uniform1i(programState.shadeModelFlatLoc, gles1State.mShadeModel == ShadingModel::Flat);
    }
}

}  // namespace gl
