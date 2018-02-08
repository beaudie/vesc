//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// GLES1on3.cpp: Implements the GLES1on3 renderer.

#include "libANGLE/gles1/GLES1on3.h"

#include <string.h>
#include <iterator>
#include <sstream>
#include <vector>

#include "common/matrix_utils.h"
#include "common/platform.h"
#include "common/utilities.h"
#include "common/version.h"
#include "libANGLE/ResourceManager.h"
#include "libANGLE/Texture.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/gles1/FixedFunctionShaders.h"
#include "libANGLE/queryconversions.h"
#include "libANGLE/queryutils.h"
#include "libANGLE/validationES.h"

namespace gl
{

GLES1on3::GLES1on3(Context *context, State *state) : mContext(context), mGLState(state)
{

    Context *gl = mContext;

    std::vector<const char *> srcs(1);

    GLint stat;
    std::string buf(4096, 0);

    {
        GLuint drawTexVShader = gl->createShader(GL_VERTEX_SHADER);
        srcs[0]               = kGLES1DrawTexVShader;
        gl->shaderSource(drawTexVShader, 1, (const GLchar *const *)(srcs.data()), nullptr);
        gl->compileShader(drawTexVShader);

        gl->getShaderiv(drawTexVShader, GL_COMPILE_STATUS, &stat);
        if (!stat)
        {
            fprintf(stderr, "%s: drawTex vertex shader compile failed.\n", __func__);
            gl->getShaderInfoLog(drawTexVShader, 4095, nullptr, &buf[0]);
            fprintf(stderr, "%s: info log: %s\n", __func__, buf.c_str());
        }

        GLuint drawTexFShader = gl->createShader(GL_FRAGMENT_SHADER);
        srcs[0]               = kGLES1DrawTexFShader;
        gl->shaderSource(drawTexFShader, 1, (const GLchar *const *)(srcs.data()), nullptr);
        gl->compileShader(drawTexFShader);

        gl->getShaderiv(drawTexFShader, GL_COMPILE_STATUS, &stat);
        if (!stat)
        {
            fprintf(stderr, "%s: drawTex frag shader compile failed.\n", __func__);
            gl->getShaderInfoLog(drawTexFShader, 4095, nullptr, &buf[0]);
            fprintf(stderr, "%s: info log: %s\n", __func__, buf.c_str());
        }

        drawTex.program = gl->createProgram();
        gl->attachShader(drawTex.program, drawTexVShader);
        gl->attachShader(drawTex.program, drawTexFShader);
        gl->linkProgram(drawTex.program);

        {
            drawTex.xyzwNdcLoc     = gl->getUniformLocation(drawTex.program, "xyzwNdc");
            drawTex.texuvwhCropLoc = gl->getUniformLocation(drawTex.program, "texuvwhCrop");
            drawTex.texNdcDimsLoc  = gl->getUniformLocation(drawTex.program, "texNdcDims");
            drawTex.samplerLoc     = gl->getUniformLocation(drawTex.program, "tex_sampler");
        }
    }

    {  // regular draw state
        GLuint drawVShader = gl->createShader(GL_VERTEX_SHADER);
        srcs[0]            = kGLES1DrawVShader;
        gl->shaderSource(drawVShader, 1, (const GLchar *const *)(srcs.data()), nullptr);
        gl->compileShader(drawVShader);

        gl->getShaderiv(drawVShader, GL_COMPILE_STATUS, &stat);
        if (!stat)
        {
            fprintf(stderr, "%s: vertex shader compile failed.\n", __func__);
            gl->getShaderInfoLog(drawVShader, 4095, nullptr, &buf[0]);
            fprintf(stderr, "%s: info log: %s\n", __func__, buf.c_str());
        }

        GLuint drawFShader     = gl->createShader(GL_FRAGMENT_SHADER);
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
            gl->getShaderInfoLog(drawFShader, 4095, nullptr, &buf[0]);
            fprintf(stderr, "%s: info log: %s\n", __func__, buf.c_str());
        }

        mDrawState.program = gl->createProgram();
        gl->attachShader(mDrawState.program, drawVShader);
        gl->attachShader(mDrawState.program, drawFShader);
        gl->linkProgram(mDrawState.program);

        gl->getProgramiv(mDrawState.program, GL_LINK_STATUS, &stat);
        if (!stat)
        {
            fprintf(stderr, "%s: fragment shader link failed.\n", __func__);
            gl->getProgramInfoLog(mDrawState.program, 4095, nullptr, &buf[0]);
            fprintf(stderr, "%s: info log: %s\n", __func__, buf.c_str());
        }

        {
            GLES1DrawState &drawState = mDrawState;
            GLuint drawProg           = drawState.program;

            drawState.projMatrixLoc      = gl->getUniformLocation(drawProg, "projection");
            drawState.modelviewMatrixLoc = gl->getUniformLocation(drawProg, "modelview");
            drawState.textureMatrixLoc   = gl->getUniformLocation(drawProg, "texture_matrix");
            drawState.modelviewInvTrLoc  = gl->getUniformLocation(drawProg, "modelview_invtr");
            drawState.textureSampler0Loc = gl->getUniformLocation(drawProg, "tex_sampler0");
            drawState.textureCubeSampler0Loc =
                gl->getUniformLocation(drawProg, "tex_cube_sampler0");
            drawState.textureSampler1Loc = gl->getUniformLocation(drawProg, "tex_sampler1");
            drawState.textureCubeSampler1Loc =
                gl->getUniformLocation(drawProg, "tex_cube_sampler1");
            drawState.textureSampler2Loc = gl->getUniformLocation(drawProg, "tex_sampler2");
            drawState.textureCubeSampler2Loc =
                gl->getUniformLocation(drawProg, "tex_cube_sampler2");
            drawState.textureSampler3Loc = gl->getUniformLocation(drawProg, "tex_sampler3");
            drawState.textureCubeSampler3Loc =
                gl->getUniformLocation(drawProg, "tex_cube_sampler3");

            drawState.shadeModelFlatLoc = gl->getUniformLocation(drawProg, "shade_model_flat");

            drawState.enableTexture2DLoc = gl->getUniformLocation(drawProg, "enable_texture_2d");
            drawState.enableTextureCubeMapLoc =
                gl->getUniformLocation(drawProg, "enable_texture_cube_map");
            drawState.enableLightingLoc = gl->getUniformLocation(drawProg, "enable_lighting");
            drawState.enableRescaleNormalLoc =
                gl->getUniformLocation(drawProg, "enable_rescale_normal");
            drawState.enableNormalizeLoc = gl->getUniformLocation(drawProg, "enable_normalize");
            drawState.enableColorMaterialLoc =
                gl->getUniformLocation(drawProg, "enable_color_material");
            drawState.enableFogLoc = gl->getUniformLocation(drawProg, "enable_fog");
            drawState.enableReflectionMapLoc =
                gl->getUniformLocation(drawProg, "enable_reflection_map");
            drawState.enableAlphaTestLoc = gl->getUniformLocation(drawProg, "enable_alpha_test");
            drawState.enableLogicOpLoc   = gl->getUniformLocation(drawProg, "enable_logic_op");
            drawState.logicOpLoc         = gl->getUniformLocation(drawProg, "logic_op");

            drawState.alphaFuncLoc    = gl->getUniformLocation(drawProg, "alpha_func");
            drawState.alphaTestRefLoc = gl->getUniformLocation(drawProg, "alpha_test_ref");

            drawState.textureFormatLoc = gl->getUniformLocation(drawProg, "texture_format");

            drawState.textureEnvModeLoc  = gl->getUniformLocation(drawProg, "texture_env_mode");
            drawState.combineRgbLoc      = gl->getUniformLocation(drawProg, "combine_rgb");
            drawState.combineAlphaLoc    = gl->getUniformLocation(drawProg, "combine_alpha");
            drawState.src0rgbLoc         = gl->getUniformLocation(drawProg, "src0_rgb");
            drawState.src0alphaLoc       = gl->getUniformLocation(drawProg, "src0_alpha");
            drawState.src1rgbLoc         = gl->getUniformLocation(drawProg, "src1_rgb");
            drawState.src1alphaLoc       = gl->getUniformLocation(drawProg, "src1_alpha");
            drawState.src2rgbLoc         = gl->getUniformLocation(drawProg, "src2_rgb");
            drawState.src2alphaLoc       = gl->getUniformLocation(drawProg, "src2_alpha");
            drawState.op0rgbLoc          = gl->getUniformLocation(drawProg, "op0_rgb");
            drawState.op0alphaLoc        = gl->getUniformLocation(drawProg, "op0_alpha");
            drawState.op1rgbLoc          = gl->getUniformLocation(drawProg, "op1_rgb");
            drawState.op1alphaLoc        = gl->getUniformLocation(drawProg, "op1_alpha");
            drawState.op2rgbLoc          = gl->getUniformLocation(drawProg, "op2_rgb");
            drawState.op2alphaLoc        = gl->getUniformLocation(drawProg, "op2_alpha");
            drawState.textureEnvColorLoc = gl->getUniformLocation(drawProg, "texture_env_color");
            drawState.rgbScaleLoc   = gl->getUniformLocation(drawProg, "texture_env_rgb_scale");
            drawState.alphaScaleLoc = gl->getUniformLocation(drawProg, "texture_env_alpha_scale");
            drawState.pointSpriteCoordReplaceLoc =
                gl->getUniformLocation(drawProg, "texture_env_point_sprite_coord_replace");

            drawState.materialAmbientLoc  = gl->getUniformLocation(drawProg, "material_ambient");
            drawState.materialDiffuseLoc  = gl->getUniformLocation(drawProg, "material_diffuse");
            drawState.materialSpecularLoc = gl->getUniformLocation(drawProg, "material_specular");
            drawState.materialEmissiveLoc = gl->getUniformLocation(drawProg, "material_emissive");
            drawState.materialSpecularExponentLoc =
                gl->getUniformLocation(drawProg, "material_specular_exponent");

            drawState.lightModelSceneAmbientLoc =
                gl->getUniformLocation(drawProg, "light_model_scene_ambient");
            drawState.lightModelTwoSidedLoc =
                gl->getUniformLocation(drawProg, "light_model_two_sided");

            drawState.lightEnablesLoc    = gl->getUniformLocation(drawProg, "light_enables");
            drawState.lightAmbientsLoc   = gl->getUniformLocation(drawProg, "light_ambients");
            drawState.lightDiffusesLoc   = gl->getUniformLocation(drawProg, "light_diffuses");
            drawState.lightSpecularsLoc  = gl->getUniformLocation(drawProg, "light_speculars");
            drawState.lightPositionsLoc  = gl->getUniformLocation(drawProg, "light_positions");
            drawState.lightDirectionsLoc = gl->getUniformLocation(drawProg, "light_directions");
            drawState.lightSpotlightExponentsLoc =
                gl->getUniformLocation(drawProg, "light_spotlight_exponents");
            drawState.lightSpotlightCutoffAnglesLoc =
                gl->getUniformLocation(drawProg, "light_spotlight_cutoff_angles");
            drawState.lightAttenuationConstsLoc =
                gl->getUniformLocation(drawProg, "light_attenuation_consts");
            drawState.lightAttenuationLinearsLoc =
                gl->getUniformLocation(drawProg, "light_attenuation_linears");
            drawState.lightAttenuationQuadraticsLoc =
                gl->getUniformLocation(drawProg, "light_attenuation_quadratics");

            drawState.fogModeLoc    = gl->getUniformLocation(drawProg, "fog_mode");
            drawState.fogDensityLoc = gl->getUniformLocation(drawProg, "fog_density");
            drawState.fogStartLoc   = gl->getUniformLocation(drawProg, "fog_start");
            drawState.fogEndLoc     = gl->getUniformLocation(drawProg, "fog_end");
            drawState.fogColorLoc   = gl->getUniformLocation(drawProg, "fog_color");

            drawState.pointRasterizationLoc =
                gl->getUniformLocation(drawProg, "point_rasterization");
            drawState.pointSpriteEnabledLoc =
                gl->getUniformLocation(drawProg, "point_sprite_enabled");
            drawState.pointSmoothEnabledLoc =
                gl->getUniformLocation(drawProg, "point_smooth_enabled");
            drawState.pointSizeMinLoc = gl->getUniformLocation(drawProg, "point_size_min");
            drawState.pointSizeMaxLoc = gl->getUniformLocation(drawProg, "point_size_max");
            drawState.pointDistanceAttenuationLoc =
                gl->getUniformLocation(drawProg, "point_distance_attenuation");

            drawState.enableClipPlanesLoc = gl->getUniformLocation(drawProg, "enable_clip_planes");
            drawState.clipPlaneEnablesLoc = gl->getUniformLocation(drawProg, "clip_plane_enables");
            drawState.clipPlanesLoc       = gl->getUniformLocation(drawProg, "clip_planes");

            gl->useProgram(mDrawState.program);

            gl->uniform1i(drawState.textureSampler0Loc, 0);
            gl->uniform1i(drawState.textureSampler1Loc, 1);
            gl->uniform1i(drawState.textureSampler2Loc, 2);
            gl->uniform1i(drawState.textureSampler3Loc, 3);
            gl->uniform1i(drawState.textureCubeSampler0Loc, 4);
            gl->uniform1i(drawState.textureCubeSampler1Loc, 5);
            gl->uniform1i(drawState.textureCubeSampler2Loc, 6);
            gl->uniform1i(drawState.textureCubeSampler2Loc, 7);
        }
    }
}

GLES1on3::~GLES1on3()
{
}

GLint GLES1on3::vertexArrayIndex(GLenum type) const
{
    switch (type)
    {
        case GL_VERTEX_ARRAY:
        case GL_VERTEX_ARRAY_BUFFER_BINDING:
        case GL_VERTEX_ARRAY_STRIDE:
        case GL_VERTEX_ARRAY_SIZE:
        case GL_VERTEX_ARRAY_TYPE:
        case GL_VERTEX_ARRAY_POINTER:
            return 0;
        case GL_NORMAL_ARRAY:
        case GL_NORMAL_ARRAY_BUFFER_BINDING:
        case GL_NORMAL_ARRAY_STRIDE:
        case GL_NORMAL_ARRAY_TYPE:
        case GL_NORMAL_ARRAY_POINTER:
            return 1;
        case GL_COLOR_ARRAY:
        case GL_COLOR_ARRAY_BUFFER_BINDING:
        case GL_COLOR_ARRAY_STRIDE:
        case GL_COLOR_ARRAY_SIZE:
        case GL_COLOR_ARRAY_TYPE:
        case GL_COLOR_ARRAY_POINTER:
            return 2;
        case GL_POINT_SIZE_ARRAY_OES:
        case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES:
        case GL_POINT_SIZE_ARRAY_STRIDE_OES:
        case GL_POINT_SIZE_ARRAY_TYPE_OES:
        case GL_POINT_SIZE_ARRAY_POINTER_OES:
            return 3;
        case GL_TEXTURE_COORD_ARRAY:
        case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:
        case GL_TEXTURE_COORD_ARRAY_STRIDE:
        case GL_TEXTURE_COORD_ARRAY_SIZE:
        case GL_TEXTURE_COORD_ARRAY_TYPE:
        case GL_TEXTURE_COORD_ARRAY_POINTER:
            return 4 + mGLState->getActiveSampler();
    }
    return 0;
}

void GLES1on3::prepareDraw(GLenum mode)
{

    Context *gl = mContext;
    State *glS  = mGLState;

    bool logicOpEnabled = glS->getEnableFeature(GL_COLOR_LOGIC_OP);

    GLint drawBuf;
    GLint readBuf;

    if (logicOpEnabled)
    {
        gl->getIntegerv(GL_DRAW_BUFFER0_EXT, &drawBuf);
        gl->getIntegerv(GL_READ_BUFFER, &readBuf);
        fprintf(stderr, "%s: logic op enabled (warning: NYI). 0x%x 0x%x\n", __func__, drawBuf,
                readBuf);
    }

    auto &drawState = mDrawState;

    if (!glS->isClientStateEnabled(GL_NORMAL_ARRAY))
    {
        const auto normal = glS->getCurrentNormal();
        gl->vertexAttrib3f(1, normal.x, normal.y, normal.z);
    }

    if (!glS->isClientStateEnabled(GL_COLOR_ARRAY))
    {
        const auto color = glS->getCurrentColor();
        gl->vertexAttrib4f(2, color.red, color.green, color.blue, color.alpha);
    }

    if (!glS->isClientStateEnabled(GL_POINT_SIZE_ARRAY_OES))
    {
        GLfloat pointSize = glS->getPointSize();
        gl->vertexAttrib1f(3, pointSize);
    }

    for (int i = 0; i < 4; i++)
    {
        if (!glS->isTexCoordArrayEnabled(i))
        {
            const auto texcoord = glS->getTextureCoordinate(i);
            gl->vertexAttrib4f(4 + i, texcoord.s, texcoord.t, texcoord.r, texcoord.q);
        }
    }

    {
        auto proj           = glS->projMatrix();
        auto modelview      = glS->modelviewMatrix();
        auto modelviewInvTr = modelview.transpose().inverse();

        gl->uniformMatrix4fv(drawState.projMatrixLoc, 1, GL_FALSE, proj.data());
        gl->uniformMatrix4fv(drawState.modelviewMatrixLoc, 1, GL_FALSE, modelview.data());
        gl->uniformMatrix4fv(drawState.modelviewInvTrLoc, 1, GL_FALSE, modelviewInvTr.data());

        for (int i = 0; i < 4; i++)
        {
            auto tex = glS->textureMatrix(i);
            memcpy(drawState.texEnvBuffer.textureMatrices + i * 16, tex.data(), 16 * sizeof(float));
        }

        gl->uniformMatrix4fv(drawState.textureMatrixLoc, 4, GL_FALSE,
                             drawState.texEnvBuffer.textureMatrices);
    }

    {
        std::vector<int> tex2Denables   = {0, 0, 0, 0};
        std::vector<int> texCubeenables = {0, 0, 0, 0};
        std::vector<int> tex2DFormats   = {GL_RGBA, GL_RGBA, GL_RGBA, GL_RGBA};

        for (int i = 0; i < 4; i++)
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

            texCubeenables[i] = glS->isTextureTargetEnabled(GL_TEXTURE0 + i, GL_TEXTURE_CUBE_MAP);
            tex2Denables[i] =
                !texCubeenables[i] && (glS->isTextureTargetEnabled(GL_TEXTURE0 + i, GL_TEXTURE_2D));

            Texture *curr2DTexture = glS->getSamplerTexture(i, GL_TEXTURE_2D);
            if (curr2DTexture)
            {
                tex2DFormats[i] = gl::GetUnsizedFormat(
                    curr2DTexture->getFormat(GL_TEXTURE_2D, 0).info->internalFormat);
            }
            // TODO: Cube map formats
        }

        gl->uniform1iv(drawState.enableTexture2DLoc, 4, tex2Denables.data());
        gl->uniform1iv(drawState.enableTextureCubeMapLoc, 4, texCubeenables.data());
        gl->uniform1iv(drawState.textureFormatLoc, 4, tex2DFormats.data());

        // Fixed sampler locations
        gl->uniform1i(drawState.textureSampler0Loc,
                      (tex2Denables[0] || !texCubeenables[0]) ? 0 : 4);
        gl->uniform1i(drawState.textureSampler1Loc,
                      (tex2Denables[1] || !texCubeenables[1]) ? 1 : 5);
        gl->uniform1i(drawState.textureSampler2Loc,
                      (tex2Denables[2] || !texCubeenables[2]) ? 2 : 6);
        gl->uniform1i(drawState.textureSampler3Loc,
                      (tex2Denables[3] || !texCubeenables[3]) ? 3 : 7);
        gl->uniform1i(drawState.textureCubeSampler0Loc, texCubeenables[0] ? 0 : 4);
        gl->uniform1i(drawState.textureCubeSampler1Loc, texCubeenables[1] ? 1 : 5);
        gl->uniform1i(drawState.textureCubeSampler2Loc, texCubeenables[2] ? 2 : 6);
        // uniform1i(drawState.textureCubeSampler3Loc, texCubeenables[3] ? 3 : 7);

        for (int i = 0; i < 4; i++)
        {
            const auto &texEnv = glS->getTextureEnvironment(i);

            drawState.texEnvBuffer.modes[i]         = texEnv.envMode;
            drawState.texEnvBuffer.combineRgbs[i]   = texEnv.combineRgb;
            drawState.texEnvBuffer.combineAlphas[i] = texEnv.combineAlpha;

            drawState.texEnvBuffer.src0rgbs[i]   = texEnv.src0rgb;
            drawState.texEnvBuffer.src0alphas[i] = texEnv.src0alpha;
            drawState.texEnvBuffer.src1rgbs[i]   = texEnv.src1rgb;
            drawState.texEnvBuffer.src1alphas[i] = texEnv.src1alpha;
            drawState.texEnvBuffer.src2rgbs[i]   = texEnv.src2rgb;
            drawState.texEnvBuffer.src2alphas[i] = texEnv.src2alpha;

            drawState.texEnvBuffer.op0rgbs[i]   = texEnv.op0rgb;
            drawState.texEnvBuffer.op0alphas[i] = texEnv.op0alpha;
            drawState.texEnvBuffer.op1rgbs[i]   = texEnv.op1rgb;
            drawState.texEnvBuffer.op1alphas[i] = texEnv.op1alpha;
            drawState.texEnvBuffer.op2rgbs[i]   = texEnv.op2rgb;
            drawState.texEnvBuffer.op2alphas[i] = texEnv.op2alpha;

            drawState.texEnvBuffer.envColors[4 * i + 0] = texEnv.envColor.red;
            drawState.texEnvBuffer.envColors[4 * i + 1] = texEnv.envColor.green;
            drawState.texEnvBuffer.envColors[4 * i + 2] = texEnv.envColor.blue;
            drawState.texEnvBuffer.envColors[4 * i + 3] = texEnv.envColor.alpha;

            drawState.texEnvBuffer.rgbScales[i]                = texEnv.rgbScale;
            drawState.texEnvBuffer.alphaScales[i]              = texEnv.alphaScale;
            drawState.texEnvBuffer.pointSpriteCoordReplaces[i] = texEnv.pointSpriteCoordReplace;
        }

        gl->uniform1iv(drawState.textureEnvModeLoc, 4, drawState.texEnvBuffer.modes);
        gl->uniform1iv(drawState.combineRgbLoc, 4, drawState.texEnvBuffer.combineRgbs);
        gl->uniform1iv(drawState.combineAlphaLoc, 4, drawState.texEnvBuffer.combineAlphas);

        gl->uniform1iv(drawState.src0rgbLoc, 4, drawState.texEnvBuffer.src0rgbs);
        gl->uniform1iv(drawState.src0alphaLoc, 4, drawState.texEnvBuffer.src0alphas);
        gl->uniform1iv(drawState.src1rgbLoc, 4, drawState.texEnvBuffer.src1rgbs);
        gl->uniform1iv(drawState.src1alphaLoc, 4, drawState.texEnvBuffer.src1alphas);
        gl->uniform1iv(drawState.src2rgbLoc, 4, drawState.texEnvBuffer.src2rgbs);
        gl->uniform1iv(drawState.src2alphaLoc, 4, drawState.texEnvBuffer.src2alphas);

        gl->uniform1iv(drawState.op0rgbLoc, 4, drawState.texEnvBuffer.op0rgbs);
        gl->uniform1iv(drawState.op0alphaLoc, 4, drawState.texEnvBuffer.op0alphas);
        gl->uniform1iv(drawState.op1rgbLoc, 4, drawState.texEnvBuffer.op1rgbs);
        gl->uniform1iv(drawState.op1alphaLoc, 4, drawState.texEnvBuffer.op1alphas);
        gl->uniform1iv(drawState.op2rgbLoc, 4, drawState.texEnvBuffer.op2rgbs);
        gl->uniform1iv(drawState.op2alphaLoc, 4, drawState.texEnvBuffer.op2alphas);

        gl->uniform4fv(drawState.textureEnvColorLoc, 4, drawState.texEnvBuffer.envColors);
        gl->uniform1fv(drawState.rgbScaleLoc, 4, drawState.texEnvBuffer.rgbScales);
        gl->uniform1fv(drawState.alphaScaleLoc, 4, drawState.texEnvBuffer.alphaScales);
        gl->uniform1iv(drawState.pointSpriteCoordReplaceLoc, 4,
                       drawState.texEnvBuffer.pointSpriteCoordReplaces);

        gl->uniform1i(drawState.shadeModelFlatLoc, glS->getShadeModel() == GL_FLAT);
        gl->uniform1i(drawState.enableLightingLoc, glS->getEnableFeature(GL_LIGHTING));
        gl->uniform1i(drawState.enableRescaleNormalLoc, glS->getEnableFeature(GL_RESCALE_NORMAL));
        gl->uniform1i(drawState.enableNormalizeLoc, glS->getEnableFeature(GL_NORMALIZE));
        gl->uniform1i(drawState.enableColorMaterialLoc, glS->getEnableFeature(GL_COLOR_MATERIAL));

        gl->uniform1i(drawState.enableFogLoc, glS->getEnableFeature(GL_FOG));
        gl->uniform1i(drawState.enableReflectionMapLoc,
                      glS->getEnableFeature(GL_REFLECTION_MAP_OES));
        gl->uniform1i(drawState.enableAlphaTestLoc, glS->getEnableFeature(GL_ALPHA_TEST));
        gl->uniform1i(drawState.enableLogicOpLoc, glS->getEnableFeature(GL_COLOR_LOGIC_OP));

        gl->uniform1i(drawState.alphaFuncLoc, glS->getAlphaFunc());
        gl->uniform1f(drawState.alphaTestRefLoc, glS->getAlphaTestRef());

        gl->uniform1i(drawState.logicOpLoc, glS->getLogicOp());

        const auto &material = glS->getMaterialParameters();

        gl->uniform4fv(drawState.materialAmbientLoc, 1, material.ambient.data());
        gl->uniform4fv(drawState.materialDiffuseLoc, 1, material.diffuse.data());
        gl->uniform4fv(drawState.materialSpecularLoc, 1, material.specular.data());
        gl->uniform4fv(drawState.materialEmissiveLoc, 1, material.emissive.data());
        gl->uniform1f(drawState.materialSpecularExponentLoc, material.specularExponent);

        const auto &lightModel = glS->getLightModelParameters();

        gl->uniform4fv(drawState.lightModelSceneAmbientLoc, 1, lightModel.color.data());
        // TODO
        // gl->uniform1i(drawState.lightModelTwoSidedLoc, lightModel.twoSided);

        for (int i = 0; i < 8; i++)
        {
            const auto &light                        = glS->getLightParameters(i);
            drawState.lightingBuffer.lightEnables[i] = light.enabled;
            memcpy(drawState.lightingBuffer.lightAmbients + 4 * i, light.ambient.data(),
                   4 * sizeof(GLfloat));
            memcpy(drawState.lightingBuffer.lightDiffuses + 4 * i, light.diffuse.data(),
                   4 * sizeof(GLfloat));
            memcpy(drawState.lightingBuffer.lightSpeculars + 4 * i, light.specular.data(),
                   4 * sizeof(GLfloat));
            memcpy(drawState.lightingBuffer.lightPositions + 4 * i, light.position.data(),
                   4 * sizeof(GLfloat));
            memcpy(drawState.lightingBuffer.lightDirections + 3 * i, light.direction.data(),
                   3 * sizeof(GLfloat));
            drawState.lightingBuffer.spotlightExponents[i]    = light.spotlightExponent;
            drawState.lightingBuffer.spotlightCutoffAngles[i] = light.spotlightCutoffAngle;
            drawState.lightingBuffer.attenuationConsts[i]     = light.attenuationConst;
            drawState.lightingBuffer.attenuationLinears[i]    = light.attenuationLinear;
            drawState.lightingBuffer.attenuationQuadratics[i] = light.attenuationQuadratic;
        }

        gl->uniform1iv(drawState.lightEnablesLoc, 8, drawState.lightingBuffer.lightEnables);
        gl->uniform4fv(drawState.lightAmbientsLoc, 8, drawState.lightingBuffer.lightAmbients);
        gl->uniform4fv(drawState.lightDiffusesLoc, 8, drawState.lightingBuffer.lightDiffuses);
        gl->uniform4fv(drawState.lightSpecularsLoc, 8, drawState.lightingBuffer.lightSpeculars);
        gl->uniform4fv(drawState.lightPositionsLoc, 8, drawState.lightingBuffer.lightPositions);
        gl->uniform3fv(drawState.lightDirectionsLoc, 8, drawState.lightingBuffer.lightDirections);
        gl->uniform1fv(drawState.lightSpotlightExponentsLoc, 8,
                       drawState.lightingBuffer.spotlightExponents);
        gl->uniform1fv(drawState.lightSpotlightCutoffAnglesLoc, 8,
                       drawState.lightingBuffer.spotlightCutoffAngles);
        gl->uniform1fv(drawState.lightAttenuationConstsLoc, 8,
                       drawState.lightingBuffer.attenuationConsts);
        gl->uniform1fv(drawState.lightAttenuationLinearsLoc, 8,
                       drawState.lightingBuffer.attenuationLinears);
        gl->uniform1fv(drawState.lightAttenuationQuadraticsLoc, 8,
                       drawState.lightingBuffer.attenuationQuadratics);

        const auto &fog = glS->getFogParameters();

        gl->uniform1i(drawState.fogModeLoc, fog.mode);
        gl->uniform1f(drawState.fogDensityLoc, fog.density);
        gl->uniform1f(drawState.fogStartLoc, fog.start);
        gl->uniform1f(drawState.fogEndLoc, fog.end);

        gl->uniform4fv(drawState.fogColorLoc, 1, fog.color.data());

        // Point rasterization
        gl->uniform1i(drawState.pointRasterizationLoc, (GLint)(mode == GL_POINTS));
        gl->uniform1i(drawState.pointSpriteEnabledLoc, glS->getEnableFeature(GL_POINT_SPRITE_OES));
        // TODO
        // uniform1i(drawState.pointSmoothEnabledLoc, glS->getEnableFeature(GL_POINT_SMOOTH));
        GLfloat pointSizeLimits[2];
        glS->getPointSizeLimits(pointSizeLimits, pointSizeLimits + 1);
        gl->uniform1f(drawState.pointSizeMinLoc, pointSizeLimits[0]);
        gl->uniform1f(drawState.pointSizeMaxLoc, pointSizeLimits[1]);

        GLfloat pointDistanceAttenuation[4];
        glS->getPointDistanceAttenuation(pointDistanceAttenuation);
        gl->uniform4fv(drawState.pointDistanceAttenuationLoc, 1, pointDistanceAttenuation);

        bool enableClipPlanes = false;
        std::vector<int> clipPlaneEnables(6);
        std::vector<float> clipPlanes(6 * 4);
        for (int i = 0; i < 6; i++)
        {
            clipPlaneEnables[i] = glS->getEnableFeature(GL_CLIP_PLANE0 + i);
            enableClipPlanes    = enableClipPlanes || clipPlaneEnables[i];
            glS->getClipPlanef(GL_CLIP_PLANE0 + i, (GLfloat *)(clipPlanes.data() + 4 * i));
        }

        gl->uniform1i(drawState.enableClipPlanesLoc, enableClipPlanes);
        gl->uniform1iv(drawState.clipPlaneEnablesLoc, 6, clipPlaneEnables.data());
        gl->uniform4fv(drawState.clipPlanesLoc, 6, clipPlanes.data());
    }
}

}  // namespace gl
