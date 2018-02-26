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

void GLES1on3::getPointerv(GLenum pname, void **params)
{
    switch (pname)
    {
        case GL_VERTEX_ARRAY_POINTER:
        case GL_NORMAL_ARRAY_POINTER:
        case GL_COLOR_ARRAY_POINTER:
        case GL_TEXTURE_COORD_ARRAY_POINTER:
        case GL_POINT_SIZE_ARRAY_POINTER_OES:
            mContext->getVertexAttribPointerv((GLuint)vertexArrayIndex(pname),
                                              GL_VERTEX_ATTRIB_ARRAY_POINTER, params);
            return;
    }

    mGLState->getPointerv(pname, params);
}

// GL_OES_framebuffer_object

void GLES1on3::getFramebufferAttachmentParameteriv(GLenum target,
                                                   GLenum attachment,
                                                   GLenum pname,
                                                   GLint *params)
{
    mContext->getFramebufferAttachmentParameteriv(target, attachment, pname, params);
}

void GLES1on3::getRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    mContext->getRenderbufferParameteriv(target, pname, params);
}

void GLES1on3::framebufferTexture2D(GLenum target,
                                    GLenum attachment,
                                    GLenum textarget,
                                    GLuint texture,
                                    GLint level)
{
    mContext->framebufferTexture2D(target, attachment, textarget, texture, level);
}

void GLES1on3::framebufferRenderbuffer(GLenum target,
                                       GLenum attachment,
                                       GLenum renderbuffertarget,
                                       GLuint renderbuffer)
{
    mContext->framebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}

void GLES1on3::bindFramebuffer(GLenum target, GLuint framebuffer)
{
    mContext->bindFramebuffer(target, framebuffer);
}

void GLES1on3::bindRenderbuffer(GLenum target, GLuint renderbuffer)
{
    mContext->bindRenderbuffer(target, renderbuffer);
}

void GLES1on3::renderbufferStorage(GLenum target,
                                   GLenum internalformat,
                                   GLsizei width,
                                   GLsizei height)
{
    mContext->renderbufferStorage(target, internalformat, width, height);
}

GLenum GLES1on3::checkFramebufferStatus(GLenum target)
{
    return mContext->checkFramebufferStatus(target);
}

void GLES1on3::deleteFramebuffers(GLsizei n, const GLuint *framebuffers)
{
    mContext->deleteFramebuffers(n, framebuffers);
}

void GLES1on3::deleteRenderbuffers(GLsizei n, const GLuint *renderbuffers)
{
    mContext->deleteRenderbuffers(n, renderbuffers);
}

void GLES1on3::genFramebuffers(GLsizei n, GLuint *framebuffers)
{
    mContext->genFramebuffers(n, framebuffers);
}

void GLES1on3::genRenderbuffers(GLsizei n, GLuint *renderbuffers)
{
    mContext->genRenderbuffers(n, renderbuffers);
}

GLboolean GLES1on3::isFramebuffer(GLuint framebuffer)
{
    return mContext->isFramebuffer(framebuffer);
}

GLboolean GLES1on3::isRenderbuffer(GLuint renderbuffer)
{
    return mContext->isRenderbuffer(renderbuffer);
}

void GLES1on3::generateMipmap(GLenum target)
{
    mContext->generateMipmap(target);
}

// GLES 1 conversion macros

#define X2F(x) (((float)(x)) / 65536.0f)
#define X2D(x) (((double)(x)) / 65536.0)
#define X2I(x) ((x) / 65536)
#define B2S(x) ((short)x)

#define F2X(d)                                 \
    ((d) > 32767.65535 ? 32767 * 65536 + 65535 \
                       : (d) < -32768.65535 ? -32768 * 65536 + 65535 : ((GLfixed)((d)*65536)))

#define I2X(d) ((d)*65536)

void GLES1on3::alphaFunc(GLenum func, GLfloat ref)
{
    mGLState->alphaFunc(func, ref);
}

void GLES1on3::alphaFuncx(GLenum func, GLfixed ref)
{
    alphaFunc(func, X2F(ref));
}

void GLES1on3::clearColorx(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
    mContext->clearColor(X2F(red), X2F(green), X2F(blue), X2F(alpha));
}

void GLES1on3::clearDepthx(GLfixed depth)
{
    mContext->clearDepthf(X2F(depth));
}

void GLES1on3::clipPlanef(GLenum p, const GLfloat *eqn)
{
    mGLState->clipPlanef(p, eqn);
}

void GLES1on3::clipPlanex(GLenum plane, const GLfixed *equation)
{
    float equationf[4];

    for (int i = 0; i < 4; i++)
    {
        equationf[i] = X2F(equation[i]);
    }

    clipPlanef(plane, equationf);
}

void GLES1on3::depthRangex(GLfixed n, GLfixed f)
{
    mGLState->setDepthRange(X2F(n), X2F(f));
}

void GLES1on3::fogx(GLenum pname, GLfixed param)
{
    fogf(pname, X2F(param));
}

void GLES1on3::fogxv(GLenum pname, const GLfixed *param)
{
    GLfloat paramf[4];
    switch (pname)
    {
        case GL_FOG_MODE:
            fogf(pname, (GLfloat)(GLenum)param[0]);
            break;
        case GL_FOG_DENSITY:
        case GL_FOG_START:
        case GL_FOG_END:
            paramf[0] = X2F(param[0]);
            fogf(pname, paramf[0]);
            break;
        case GL_FOG_COLOR:
            for (int i = 0; i < 4; i++)
            {
                paramf[i] = X2F(param[i]);
            }
            fogfv(pname, paramf);
            break;
        default:
            return;
    }
}

void GLES1on3::frustumx(GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f)
{
    frustumf(X2F(l), X2F(r), X2F(b), X2F(t), X2F(n), X2F(f));
}

void GLES1on3::getClipPlanef(GLenum plane, GLfloat *equation)
{
    mGLState->getClipPlanef(plane, equation);
}

void GLES1on3::getClipPlanex(GLenum plane, GLfixed *equation)
{
    GLfloat equationf[4];
    getClipPlanef(plane, equationf);

    for (int i = 0; i < 4; i++)
    {
        equation[i] = F2X(equationf[i]);
    }
}

void GLES1on3::getFixedv(GLenum pname, GLfixed *params)
{
    GLfloat floatBuf[16];
    GLint intBuf[16];

    GLenum nativeType;
    unsigned int numParams;
    mContext->getQueryParameterInfo(pname, &nativeType, &numParams);

    switch (nativeType)
    {
        case GL_INT:
        case GL_BOOL:
            mContext->getIntegerv(pname, intBuf);
            for (unsigned int i = 0; i < numParams; i++)
            {
                // GLES1 TODO: Only if rescale applies to this pname
                params[i] = I2X(intBuf[i]);
            }
            break;
        case GL_FLOAT:
            mContext->getFloatv(pname, floatBuf);
            for (unsigned int i = 0; i < numParams; i++)
            {
                // GLES1 TODO: Only if rescale applies to this pname
                params[i] = F2X(intBuf[i]);
            }
            break;
        default:
            UNIMPLEMENTED();
    }
}

void GLES1on3::getLightxv(GLenum light, GLenum pname, GLfixed *params)
{
    GLfloat paramsf[4];
    getLightfv(light, pname, paramsf);

    unsigned int numParams = 4;
    switch (pname)
    {
        case GL_AMBIENT:
        case GL_DIFFUSE:
        case GL_SPECULAR:
        case GL_POSITION:
            numParams = 4;
            break;
        case GL_SPOT_DIRECTION:
            numParams = 3;
            break;
        case GL_SPOT_EXPONENT:
        case GL_SPOT_CUTOFF:
        case GL_CONSTANT_ATTENUATION:
        case GL_LINEAR_ATTENUATION:
        case GL_QUADRATIC_ATTENUATION:
            numParams = 1;
            break;
        default:
            break;
    }

    for (unsigned int i = 0; i < numParams; i++)
    {
        params[i] = F2X(paramsf[i]);
    }
}

void GLES1on3::getMaterialxv(GLenum face, GLenum pname, GLfixed *params)
{
    GLfloat paramsf[4];
    getMaterialfv(face, pname, paramsf);

    unsigned int numParams = 4;

    switch (pname)
    {
        case GL_AMBIENT:
        case GL_DIFFUSE:
        case GL_SPECULAR:
        case GL_EMISSION:
            numParams = 4;
            break;
        case GL_SHININESS:
            numParams = 1;
            break;
        default:
            break;
    }

    for (unsigned int i = 0; i < numParams; i++)
    {
        params[i] = F2X(paramsf[i]);
    }
}

void GLES1on3::getTexEnvxv(GLenum target, GLenum pname, GLfixed *params)
{
    GLfloat paramsf[4];
    getTexEnvfv(target, pname, paramsf);

    unsigned int numParams = 1;
    bool rescale           = true;
    switch (pname)
    {
        case GL_TEXTURE_ENV_MODE:
            numParams = 1;
            rescale   = false;
            break;
        case GL_TEXTURE_ENV_COLOR:
            numParams = 4;
            break;
        case GL_COMBINE_RGB:
            numParams = 3;
            break;
        case GL_COMBINE_ALPHA:
        case GL_RGB_SCALE:
        case GL_ALPHA_SCALE:
            numParams = 1;
            break;
    }

    if (rescale)
    {
        for (unsigned int i = 0; i < numParams; i++)
        {
            params[i] = F2X(paramsf[i]);
        }
    }
    else
    {
        for (unsigned int i = 0; i < numParams; i++)
        {
            params[i] = (GLfixed)params[i];
        }
    }
}

void GLES1on3::getTexParameterxv(GLenum target, GLenum pname, GLfixed *params)
{
    GLfloat paramsf[4];

    mContext->getTexParameterfv(target, pname, paramsf);

    unsigned int numParams = 1;
    bool rescale           = true;

    switch (pname)
    {
        case GL_TEXTURE_MAG_FILTER:
        case GL_TEXTURE_MIN_FILTER:
        case GL_TEXTURE_WRAP_S:
        case GL_TEXTURE_WRAP_T:
        case GL_TEXTURE_WRAP_R:
        case GL_TEXTURE_IMMUTABLE_FORMAT:
        case GL_TEXTURE_USAGE_ANGLE:
        case GL_TEXTURE_SWIZZLE_R:
        case GL_TEXTURE_SWIZZLE_G:
        case GL_TEXTURE_SWIZZLE_B:
        case GL_TEXTURE_SWIZZLE_A:
        case GL_TEXTURE_COMPARE_MODE:
        case GL_TEXTURE_COMPARE_FUNC:
            numParams = 1;
            rescale   = false;
            break;
        case GL_TEXTURE_MAX_ANISOTROPY_EXT:
        case GL_TEXTURE_IMMUTABLE_LEVELS:
        case GL_TEXTURE_BASE_LEVEL:
        case GL_TEXTURE_MAX_LEVEL:
        case GL_TEXTURE_MIN_LOD:
        case GL_TEXTURE_MAX_LOD:
        case GL_GENERATE_MIPMAP:
        case GL_TEXTURE_SRGB_DECODE_EXT:
            numParams = 1;
            rescale   = true;
            break;
        case GL_TEXTURE_CROP_RECT_OES:
            numParams = 4;
            rescale   = true;
            break;
        default:
            UNREACHABLE();
            break;
    }

    if (rescale)
    {
        for (unsigned int i = 0; i < numParams; i++)
        {
            params[i] = F2X(paramsf[i]);
        }
    }
    else
    {
        for (unsigned int i = 0; i < numParams; i++)
        {
            params[i] = (GLfixed)params[i];
        }
    }
}

void GLES1on3::shadeModel(GLenum mode)
{
    mGLState->shadeModel(mode);
}

void GLES1on3::matrixMode(GLenum mode)
{
    mGLState->matrixMode(mode);
}

void GLES1on3::loadIdentity()
{
    mGLState->loadIdentity();
}

void GLES1on3::loadMatrixf(const GLfloat *m)
{
    mGLState->loadMatrixf(m);
}

void GLES1on3::loadMatrixx(const GLfixed *m)
{
    UNIMPLEMENTED();
}

void GLES1on3::pushMatrix()
{
    mGLState->pushMatrix();
}

void GLES1on3::popMatrix()
{
    mGLState->popMatrix();
}

void GLES1on3::multMatrixf(const GLfloat *m)
{
    mGLState->multMatrixf(m);
}

void GLES1on3::orthof(GLfloat left,
                      GLfloat right,
                      GLfloat bottom,
                      GLfloat top,
                      GLfloat zNear,
                      GLfloat zFar)
{
    mGLState->orthof(left, right, bottom, top, zNear, zFar);
}

void GLES1on3::frustumf(GLfloat left,
                        GLfloat right,
                        GLfloat bottom,
                        GLfloat top,
                        GLfloat zNear,
                        GLfloat zFar)
{
    mGLState->frustumf(left, right, bottom, top, zNear, zFar);
}

void GLES1on3::texEnvf(GLenum target, GLenum pname, GLfloat param)
{
    mGLState->texEnvf(target, pname, param);
}

void GLES1on3::texEnvfv(GLenum target, GLenum pname, const GLfloat *params)
{
    mGLState->texEnvfv(target, pname, params);
}

void GLES1on3::texEnvi(GLenum target, GLenum pname, GLint param)
{
    mGLState->texEnvi(target, pname, param);
}

void GLES1on3::texEnviv(GLenum target, GLenum pname, const GLint *params)
{
    mGLState->texEnviv(target, pname, params);
}

void GLES1on3::getTexEnvfv(GLenum env, GLenum pname, GLfloat *params)
{
    mGLState->getTexEnvfv(env, pname, params);
}

void GLES1on3::getTexEnviv(GLenum env, GLenum pname, GLint *params)
{
    mGLState->getTexEnviv(env, pname, params);
}

void GLES1on3::texGenf(GLenum coord, GLenum pname, GLfloat param)
{
    UNIMPLEMENTED();
}

void GLES1on3::texGenfv(GLenum coord, GLenum pname, const GLfloat *params)
{
    UNIMPLEMENTED();
}

void GLES1on3::texGeni(GLenum coord, GLenum pname, GLint param)
{
    UNIMPLEMENTED();
}

void GLES1on3::texGeniv(GLenum coord, GLenum pname, const GLint *params)
{
    UNIMPLEMENTED();
}

void GLES1on3::texGenx(GLenum coord, GLenum pname, GLfixed param)
{
    UNIMPLEMENTED();
}

void GLES1on3::texGenxv(GLenum coord, GLenum pname, const GLint *params)
{
    UNIMPLEMENTED();
}

void GLES1on3::getTexGeniv(GLenum coord, GLenum pname, GLint *params)
{
    UNIMPLEMENTED();
}

void GLES1on3::getTexGenfv(GLenum coord, GLenum pname, GLfloat *params)
{
    UNIMPLEMENTED();
}

void GLES1on3::getTexGenxv(GLenum coord, GLenum pname, GLfixed *params)
{
    UNIMPLEMENTED();
}

void GLES1on3::materialf(GLenum face, GLenum pname, GLfloat param)
{
    mGLState->materialf(face, pname, param);
}

void GLES1on3::materialfv(GLenum face, GLenum pname, const GLfloat *params)
{
    mGLState->materialfv(face, pname, params);
}

void GLES1on3::getMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
    mGLState->getMaterialfv(face, pname, params);
}

void GLES1on3::lightModelf(GLenum pname, GLfloat param)
{
    mGLState->lightModelf(pname, param);
}

void GLES1on3::lightModelfv(GLenum pname, const GLfloat *params)
{
    mGLState->lightModelfv(pname, params);
}

void GLES1on3::lightModelx(GLenum pname, GLfixed param)
{
    lightModelf(pname, X2F(param));
}

void GLES1on3::lightModelxv(GLenum pname, const GLfixed *param)
{
    GLfloat paramsf[4];

    unsigned int numParams = 1;
    switch (pname)
    {
        case GL_LIGHT_MODEL_AMBIENT:
            numParams = 4;
            break;
        case GL_LIGHT_MODEL_TWO_SIDE:
            numParams = 1;
            break;
    }

    for (unsigned int i = 0; i < numParams; i++)
    {
        paramsf[i] = X2F(param[i]);
    }

    lightModelfv(pname, paramsf);
}

void GLES1on3::lightf(GLenum light, GLenum pname, GLfloat param)
{
    mGLState->lightf(light, pname, param);
}

void GLES1on3::lightfv(GLenum light, GLenum pname, const GLfloat *params)
{
    mGLState->lightfv(light, pname, params);
}

void GLES1on3::getLightfv(GLenum light, GLenum pname, GLfloat *params)
{
    mGLState->getLightfv(light, pname, params);
}

void GLES1on3::lightx(GLenum light, GLenum pname, GLfixed param)
{
    lightf(light, pname, X2F(param));
}

void GLES1on3::lightxv(GLenum light, GLenum pname, const GLfixed *params)
{
    GLfloat paramsf[4];

    unsigned int numParams = 4;
    switch (pname)
    {
        case GL_AMBIENT:
        case GL_DIFFUSE:
        case GL_SPECULAR:
        case GL_POSITION:
            numParams = 4;
            break;
        case GL_SPOT_DIRECTION:
            numParams = 3;
            break;
        case GL_SPOT_EXPONENT:
        case GL_SPOT_CUTOFF:
        case GL_CONSTANT_ATTENUATION:
        case GL_LINEAR_ATTENUATION:
        case GL_QUADRATIC_ATTENUATION:
            numParams = 1;
            break;
        default:
            break;
    }

    for (unsigned int i = 0; i < numParams; i++)
    {
        paramsf[i] = X2F(params[i]);
    }

    lightfv(light, pname, paramsf);
}

void GLES1on3::lineWidthx(GLfixed width)
{
    mContext->lineWidth(X2F(width));
}

void GLES1on3::logicOp(GLenum opcode)
{
    mGLState->logicOp(opcode);
}

void GLES1on3::materialx(GLenum face, GLenum pname, GLfixed param)
{
    materialf(face, pname, X2F(param));
}

void GLES1on3::materialxv(GLenum face, GLenum pname, const GLfixed *param)
{
    GLfloat paramsf[4];
    unsigned int numParams = 4;

    switch (pname)
    {
        case GL_AMBIENT:
        case GL_DIFFUSE:
        case GL_SPECULAR:
        case GL_EMISSION:
            numParams = 4;
            break;
        case GL_SHININESS:
            numParams = 1;
            break;
        default:
            break;
    }

    for (unsigned int i = 0; i < numParams; i++)
    {
        paramsf[i] = X2F(param[i]);
    }

    materialfv(face, pname, paramsf);
}

void GLES1on3::multMatrixx(const GLfixed *m)
{
    GLfloat mf[16];
    for (int i = 0; i < 16; i++)
    {
        mf[i] = X2F(m[i]);
    }

    multMatrixf(mf);
}

void GLES1on3::multiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    mGLState->multiTexCoord4f(target, s, t, r, q);
}

void GLES1on3::multiTexCoord4x(GLenum texture, GLfixed s, GLfixed t, GLfixed r, GLfixed q)
{
    multiTexCoord4f(texture, X2F(s), X2F(t), X2F(r), X2F(q));
}

void GLES1on3::normal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
    mGLState->normal3f(nx, ny, nz);
}

void GLES1on3::normal3x(GLfixed nx, GLfixed ny, GLfixed nz)
{
    normal3f(X2F(nx), X2F(ny), X2F(nz));
}

void GLES1on3::orthox(GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f)
{
    orthof(X2F(l), X2F(r), X2F(b), X2F(t), X2F(n), X2F(f));
}

void GLES1on3::pointParameterf(GLenum pname, GLfloat param)
{
    mGLState->pointParameterf(pname, param);
}

void GLES1on3::pointParameterfv(GLenum pname, const GLfloat *params)
{
    mGLState->pointParameterfv(pname, params);
}

void GLES1on3::pointParameterx(GLenum pname, GLfixed param)
{
    mGLState->pointParameterf(pname, X2F(param));
}

void GLES1on3::pointParameterxv(GLenum pname, const GLfixed *params)
{
    GLfloat paramsf[4];
    unsigned int numParams = 4;

    switch (pname)
    {
        case GL_POINT_SIZE_MIN:
        case GL_POINT_SIZE_MAX:
        case GL_POINT_FADE_THRESHOLD_SIZE:
            numParams = 1;
            break;
        case GL_POINT_DISTANCE_ATTENUATION:
            numParams = 3;
            break;
        default:
            break;
    }

    for (unsigned int i = 0; i < numParams; i++)
    {
        paramsf[i] = X2F(params[i]);
    }

    pointParameterfv(pname, paramsf);
}

void GLES1on3::pointSize(GLfloat size)
{
    mGLState->pointSize(size);
}

void GLES1on3::pointSizex(GLfixed size)
{
    mGLState->pointSize(X2F(size));
}

void GLES1on3::polygonOffsetx(GLfixed factor, GLfixed units)
{
    mContext->polygonOffset(X2F(factor), X2F(units));
}

void GLES1on3::sampleCoveragex(GLclampx value, GLboolean invert)
{
    UNIMPLEMENTED();
}

void GLES1on3::texEnvx(GLenum target, GLenum pname, GLfixed param)
{
    GLfloat tmpParam = (GLfloat)param;
    texEnvf(target, pname, tmpParam);
}

void GLES1on3::texEnvxv(GLenum target, GLenum pname, const GLfixed *params)
{
    GLfloat tmpParams[4];

    if (pname == GL_TEXTURE_ENV_COLOR)
    {
        for (int i = 0; i < 4; i++)
        {
            tmpParams[i] = X2F(params[i]);
        }
    }
    else
    {
        tmpParams[0] = (GLfloat)params[0];
    }

    texEnvfv(target, pname, tmpParams);
}

void GLES1on3::texParameterx(GLenum target, GLenum pname, GLfixed param)
{
    mContext->texParameterf(target, pname, (GLfloat)param);
}

void GLES1on3::texParameterxv(GLenum target, GLenum pname, const GLfixed *params)
{
    GLfloat tmpParams[4];

    if (pname == GL_TEXTURE_CROP_RECT_OES)
    {
        for (int i = 0; i < 4; ++i)
        {
            tmpParams[i] = X2F(params[i]);
        }
    }
    else
    {
        tmpParams[0] = (GLfloat)params[0];
    }

    mContext->texParameterfv(target, pname, tmpParams);
}

void GLES1on3::drawTexfv(const GLfloat *coords)
{
    UNIMPLEMENTED();
}

void GLES1on3::drawTexi(GLint x, GLint y, GLint z, GLint width, GLint height)
{
    UNIMPLEMENTED();
}

void GLES1on3::drawTexiv(const GLint *coords)
{
    UNIMPLEMENTED();
}

void GLES1on3::drawTexs(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height)
{
    UNIMPLEMENTED();
}

void GLES1on3::drawTexsv(const GLshort *coords)
{
    UNIMPLEMENTED();
}

void GLES1on3::drawTexx(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height)
{
    UNIMPLEMENTED();
}

void GLES1on3::drawTexxv(const GLfixed *coords)
{
    UNIMPLEMENTED();
}

void GLES1on3::currentPaletteMatrix(GLuint matrixpaletteindex)
{
    UNIMPLEMENTED();
}

void GLES1on3::loadPaletteFromModelViewMatrix()
{
    UNIMPLEMENTED();
}

void GLES1on3::matrixIndexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    UNIMPLEMENTED();
}

void GLES1on3::weightPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    UNIMPLEMENTED();
}

GLbitfield GLES1on3::queryMatrixx(GLfixed *mantissa, GLint *exponent)
{
    UNIMPLEMENTED();
    return 0;
}

void GLES1on3::fogf(GLenum pname, GLfloat param)
{
    mGLState->fogf(pname, param);
}

void GLES1on3::fogfv(GLenum pname, const GLfloat *params)
{
    mGLState->fogfv(pname, params);
}

void GLES1on3::enableClientState(GLenum clientState)
{
    switch (clientState)
    {
        case GL_VERTEX_ARRAY:
            mContext->enableVertexAttribArray(0);
            break;
        case GL_NORMAL_ARRAY:
            mContext->enableVertexAttribArray(1);
            break;
        case GL_COLOR_ARRAY:
            mContext->enableVertexAttribArray(2);
            break;
        case GL_POINT_SIZE_ARRAY_OES:
            mContext->enableVertexAttribArray(3);
            break;
        case GL_TEXTURE_COORD_ARRAY:
            mContext->enableVertexAttribArray(4 + mGLState->getActiveSampler());
            break;
        default:
            break;
    }

    mGLState->enableClientState(clientState);
}

void GLES1on3::disableClientState(GLenum clientState)
{
    switch (clientState)
    {
        case GL_VERTEX_ARRAY:
            mContext->disableVertexAttribArray(0);
            break;
        case GL_NORMAL_ARRAY:
            mContext->disableVertexAttribArray(1);
            break;
        case GL_COLOR_ARRAY:
            mContext->disableVertexAttribArray(2);
            break;
        case GL_POINT_SIZE_ARRAY_OES:
            mContext->disableVertexAttribArray(3);
            break;
        case GL_TEXTURE_COORD_ARRAY:
            mContext->disableVertexAttribArray(4);
            break;
        default:
            break;
    }
    mGLState->disableClientState(clientState);
}

void GLES1on3::drawTexf(float x, float y, float z, float width, float height)
{
    Context *gl = mContext;

    // get viewport
    GLint viewport[4] = {};
    gl->getIntegerv(GL_VIEWPORT, viewport);

    GLuint prog = drawTex.program;
    gl->useProgram(prog);

    // Compute screen coordinates for our texture.
    // Recenter, rescale. (e.g., [0, 0, 1080, 1920] -> [-1, -1, 1, 1])
    float xNdc = 2.0f * (float)(x - viewport[0] - viewport[2] / 2) / (float)viewport[2];
    float yNdc = 2.0f * (float)(y - viewport[1] - viewport[3] / 2) / (float)viewport[3];
    float wNdc = 2.0f * (float)width / (float)viewport[2];
    float hNdc = 2.0f * (float)height / (float)viewport[3];
    z          = z >= 1.0f ? 1.0f : z;
    z          = z <= 0.0f ? 0.0f : z;
    float zNdc = z * 2.0f - 1.0f;

    for (int i = 0; i < 4; i++)
    {
        if (mGLState->isTextureTargetEnabled(GL_TEXTURE0 + i, GL_TEXTURE_2D))
        {
            Texture *toDraw = mGLState->getSamplerTexture(i, GL_TEXTURE_2D);
            if (toDraw)
            {
                auto cropRect = toDraw->getCrop();

                float texCropU = (float)cropRect.x;
                float texCropV = (float)cropRect.y;
                float texCropW = (float)cropRect.width;
                float texCropH = (float)cropRect.height;

                float texW = (float)(toDraw->getWidth(GL_TEXTURE_2D, 0));
                float texH = (float)(toDraw->getHeight(GL_TEXTURE_2D, 0));

                float xyzwNdc[4]     = {xNdc, yNdc, zNdc, 1.0f};
                float texuvwhCrop[4] = {texCropU, texCropV, texCropW, texCropH};
                float texNdcDims[4]  = {texW, texH, wNdc, hNdc};

                gl->uniform4fv(drawTex.xyzwNdcLoc, 1, xyzwNdc);
                gl->uniform4fv(drawTex.texuvwhCropLoc, 1, texuvwhCrop);
                gl->uniform4fv(drawTex.texNdcDimsLoc, 1, texNdcDims);
            }

            gl->activeTexture(GL_TEXTURE0 + i);
            gl->uniform1i(drawTex.samplerLoc, i);
            gl->drawArraysImpl(GL_TRIANGLES, 0, 6);
        }
    }

    gl->useProgram(mDrawState.program);
}

void GLES1on3::rotatef(float angle, float x, float y, float z)
{
    mGLState->rotatef(angle, x, y, z);
}

void GLES1on3::rotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z)
{
    rotatef(X2F(angle), X2F(x), X2F(y), X2F(z));
}

void GLES1on3::scalef(float x, float y, float z)
{
    mGLState->scalef(x, y, z);
}

void GLES1on3::scalex(GLfixed x, GLfixed y, GLfixed z)
{
    scalef(X2F(x), X2F(y), X2F(z));
}

void GLES1on3::translatef(float x, float y, float z)
{
    mGLState->translatef(x, y, z);
}

void GLES1on3::translatex(GLfixed x, GLfixed y, GLfixed z)
{
    translatef(X2F(x), X2F(y), X2F(z));
}

void GLES1on3::color4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    mGLState->color4f(red, green, blue, alpha);
}

void GLES1on3::color4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    mGLState->color4f(((float)red) / 255.0f, ((float)green) / 255.0f, ((float)blue) / 255.0f,
                      ((float)alpha) / 255.0f);
}

void GLES1on3::color4x(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
    mGLState->color4f(X2F(red), X2F(green), X2F(blue), X2F(alpha));
}

void GLES1on3::clientActiveTexture(GLenum texture)
{
    mContext->activeTexture(texture);
}

void GLES1on3::vertexPointer(GLint size, GLenum type, GLsizei stride, const void *ptr)
{
    mContext->vertexAttribPointer(0, size, type, GL_FALSE, stride, ptr);
}

void GLES1on3::normalPointer(GLenum type, GLsizei stride, const void *ptr)
{
    mContext->vertexAttribPointer(1, 3, type, GL_FALSE, stride, ptr);
}

void GLES1on3::colorPointer(GLint size, GLenum type, GLsizei stride, const void *ptr)
{
    mContext->vertexAttribPointer(2, size, type, GL_FALSE, stride, ptr);
}

void GLES1on3::pointSizePointer(GLenum type, GLsizei stride, const void *ptr)
{
    mContext->vertexAttribPointer(3, 1, type, GL_FALSE, stride, ptr);
}

void GLES1on3::texCoordPointer(GLint size, GLenum type, GLsizei stride, const void *ptr)
{
    mContext->vertexAttribPointer(4 + mGLState->getActiveSampler(), size, type, GL_FALSE, stride,
                                  ptr);
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
