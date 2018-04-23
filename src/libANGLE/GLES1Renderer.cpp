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
#include "libANGLE/Program.h"
#include "libANGLE/ResourceManager.h"
#include "libANGLE/Shader.h"
#include "libANGLE/State.h"
#include "libANGLE/renderer/ContextImpl.h"

namespace
{

#include "libANGLE/GLES1Shaders.inc"

}  // namespace

namespace gl
{

ShaderProgramManager *GLES1Renderer::ProgramResources = nullptr;

GLES1Renderer::GLES1Renderer() : mRendererProgramInitialized(false)
{
}

void GLES1Renderer::onDestroy(Context *gl)
{
    if (mRendererProgramInitialized)
    {
        ProgramResources->deleteProgram(gl, mProgramState.program);
    }
}

GLES1Renderer::~GLES1Renderer() = default;

Error GLES1Renderer::prepareForDraw(Context *gl, State *glState)
{
    ANGLE_TRY(initializeRendererProgram(gl, glState));

    const GLES1State &gles1State = glState->gles1();

    const GLES1ProgramState &programState = mProgramState;
    Program *programObject                = getProgram(mProgramState.program);

    GLES1UniformBuffers &uniformBuffers = mUniformBuffers;

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
        if (programState.projMatrixLoc != -1)
        {
            programObject->setUniformMatrix4fv(programState.projMatrixLoc, 1, GL_FALSE,
                                               proj.data());
        }

        angle::Mat4 modelview = gles1State.mModelviewMatrices.back();
        if (programState.modelviewMatrixLoc != -1)
        {
            programObject->setUniformMatrix4fv(programState.modelviewMatrixLoc, 1, GL_FALSE,
                                               modelview.data());
        }

        angle::Mat4 modelviewInvTr = modelview.transpose().inverse();
        if (programState.modelviewInvTrLoc != -1)
        {
            programObject->setUniformMatrix4fv(programState.modelviewInvTrLoc, 1, GL_FALSE,
                                               modelviewInvTr.data());
        }

        Mat4Uniform *textureMatrixBuffer = uniformBuffers.textureMatrices.data();

        for (int i = 0; i < kTexUnitCount; i++)
        {
            angle::Mat4 textureMatrix = gles1State.mTextureMatrices[i].back();
            memcpy(textureMatrixBuffer + i, textureMatrix.data(), sizeof(Mat4Uniform));
        }

        if (programState.textureMatrixLoc != -1)
        {
            programObject->setUniformMatrix4fv(programState.textureMatrixLoc, 4, GL_FALSE,
                                               (float *)uniformBuffers.textureMatrices.data());
        }
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

        if (programState.enableTexture2DLoc != -1)
        {
            programObject->setUniform1iv(programState.enableTexture2DLoc, kTexUnitCount,
                                         tex2DEnables.data());
        }
        if (programState.enableTextureCubeMapLoc != -1)
        {
            programObject->setUniform1iv(programState.enableTextureCubeMapLoc, kTexUnitCount,
                                         texCubeEnables.data());
        }

        GLint flatShading = gles1State.mShadeModel == ShadingModel::Flat;

        if (programState.shadeModelFlatLoc != -1)
        {
            programObject->setUniform1iv(programState.shadeModelFlatLoc, 1, &flatShading);
        }
    }

    // None of those are changes in sampler, so there is no need to set the GL_PROGRAM dirty.
    // Otherwise, put the dirtying here.

    return NoError();
}

Shader *GLES1Renderer::getShader(GLuint handle) const
{
    return ProgramResources->getShader(handle);
}

Program *GLES1Renderer::getProgram(GLuint handle) const
{
    return ProgramResources->getProgram(handle);
}

Error GLES1Renderer::compileShader(Context *gl,
                                   ShaderType shaderType,
                                   const char *src,
                                   GLuint *shaderOut)
{
    rx::ContextImpl *implementation = gl->getImplementation();
    Limitations limitations         = implementation->getNativeLimitations();

    GLuint shader = ProgramResources->createShader(implementation, limitations, shaderType);

    Shader *shaderObject = getShader(shader);

    if (!shaderObject)
        return InvalidValue();

    shaderObject->setSource(1, &src, nullptr);
    shaderObject->compile(gl);

    *shaderOut = shader;

    if (!shaderObject->isCompiled(gl))
    {
        GLint infoLogLength = shaderObject->getInfoLogLength(gl);
        std::vector<char> infoLog(infoLogLength, 0);
        shaderObject->getInfoLog(gl, infoLogLength - 1, nullptr, infoLog.data());
        fprintf(stderr, "GLES1Renderer::%s: Info log: %s\n", __func__, infoLog.data());
        ERR() << "GLES1Renderer shader compile failed. Source: " << src
              << " Info log: " << infoLog.data();
        return InvalidValue();
    }

    return NoError();
}

Error GLES1Renderer::linkProgram(Context *gl,
                                 State *glState,
                                 GLuint vertexShader,
                                 GLuint fragmentShader,
                                 const std::unordered_map<GLint, std::string> &attribLocs,
                                 GLuint *programOut)
{
    GLuint program = ProgramResources->createProgram(gl->getImplementation());

    Program *programObject = getProgram(program);

    if (!programObject)
    {
        return InvalidValue();
    }

    *programOut = program;

    programObject->attachShader(getShader(vertexShader));
    programObject->attachShader(getShader(fragmentShader));

    for (auto it : attribLocs)
    {
        GLint index             = it.first;
        const std::string &name = it.second;
        programObject->bindAttributeLocation(index, name.c_str());
    }

    ANGLE_TRY(programObject->link(gl));

    glState->onProgramExecutableChange(programObject);

    if (!programObject->isLinked())
    {
        GLint infoLogLength = programObject->getInfoLogLength();
        std::vector<char> infoLog(infoLogLength, 0);
        programObject->getInfoLog(infoLogLength - 1, nullptr, infoLog.data());
        ERR() << "GLES1Renderer program link failed. Info log: " << infoLog.data();
        return InvalidValue();
    }

    programObject->detachShader(gl, getShader(vertexShader));
    programObject->detachShader(gl, getShader(fragmentShader));

    return NoError();
}

Error GLES1Renderer::initializeRendererProgram(Context *gl, State *glState)
{
    if (mRendererProgramInitialized)
    {
        return NoError();
    }

    // Initialize the resource manager if it doesn't exist already
    if (!ProgramResources)
    {
        ProgramResources = new ShaderProgramManager();
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

    ANGLE_TRY(
        linkProgram(gl, glState, vertexShader, fragmentShader, attribLocs, &programState.program));

    ProgramResources->deleteShader(gl, vertexShader);
    ProgramResources->deleteShader(gl, fragmentShader);

    Program *programObject = getProgram(programState.program);

    programState.projMatrixLoc      = programObject->getUniformLocation("projection");
    programState.modelviewMatrixLoc = programObject->getUniformLocation("modelview");
    programState.textureMatrixLoc   = programObject->getUniformLocation("texture_matrix");
    programState.modelviewInvTrLoc  = programObject->getUniformLocation("modelview_invtr");

    for (int i = 0; i < kTexUnitCount; i++)
    {
        std::stringstream ss2d;
        std::stringstream sscube;

        ss2d << "tex_sampler" << i;
        sscube << "tex_cube_sampler" << i;

        programState.tex2DSamplerLocs[i] = programObject->getUniformLocation(ss2d.str().c_str());
        programState.texCubeSamplerLocs[i] =
            programObject->getUniformLocation(sscube.str().c_str());
    }

    programState.shadeModelFlatLoc = programObject->getUniformLocation("shade_model_flat");

    programState.enableTexture2DLoc = programObject->getUniformLocation("enable_texture_2d");
    programState.enableTextureCubeMapLoc =
        programObject->getUniformLocation("enable_texture_cube_map");

    glState->setProgram(gl, programObject);

    for (int i = 0; i < kTexUnitCount; i++)
    {

        if (programState.tex2DSamplerLocs[i] != -1)
        {
            GLint val = i;
            programObject->setUniform1iv(programState.tex2DSamplerLocs[i], 1, &val);
        }

        if (programState.texCubeSamplerLocs[i] != -1)
        {
            GLint val = i + kTexUnitCount;
            programObject->setUniform1iv(programState.texCubeSamplerLocs[i], 1, &val);
        }
    }

    glState->setObjectDirty(GL_PROGRAM);

    mRendererProgramInitialized = true;
    return NoError();
}

}  // namespace gl
