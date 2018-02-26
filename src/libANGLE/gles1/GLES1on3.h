//
// Copyright (c) 2002-2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// GLES1on3.h: Defines GLES1 emulation rendering operations on top of a GLES3
// context. Used by Context.h.

#ifndef LIBANGLE_GLES1_EMULATION_ES_H_
#define LIBANGLE_GLES1_EMULATION_ES_H_

#include <unordered_map>

#include "angle_gl.h"
#include "common/angleutils.h"
#include "libANGLE/Context.h"
#include "libANGLE/ContextState.h"
#include "libANGLE/angletypes.h"

namespace gl
{

class GLES1on3
{
  public:
    GLES1on3(Context *context, State *glState);
    ~GLES1on3();

    // OpenGL ES 1 API
    void alphaFunc(GLenum func, GLfloat ref);
    void alphaFuncx(GLenum func, GLfixed ref);
    void clearColorx(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
    void clearDepthx(GLfixed depth);
    void clientActiveTexture(GLenum texture);
    void clipPlanef(GLenum p, const GLfloat *eqn);
    void clipPlanex(GLenum plane, const GLfixed *equation);
    void color4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void color4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
    void color4x(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
    void colorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
    void depthRangex(GLfixed n, GLfixed f);
    void disableClientState(GLenum array);
    void enableClientState(GLenum array);
    void fogf(GLenum pname, GLfloat param);
    void fogfv(GLenum pname, const GLfloat *params);
    void fogx(GLenum pname, GLfixed param);
    void fogxv(GLenum pname, const GLfixed *param);
    void frustumf(GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f);
    void frustumx(GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f);
    void getClipPlanef(GLenum plane, GLfloat *equation);
    void getClipPlanex(GLenum plane, GLfixed *equation);
    void getFixedv(GLenum pname, GLfixed *params);
    void getLightfv(GLenum light, GLenum pname, GLfloat *params);
    void getLightxv(GLenum light, GLenum pname, GLfixed *params);
    void getMaterialfv(GLenum face, GLenum pname, GLfloat *params);
    void getMaterialxv(GLenum face, GLenum pname, GLfixed *params);
    void getPointerv(GLenum pname, void **params);
    void getTexEnvfv(GLenum target, GLenum pname, GLfloat *params);
    void getTexEnviv(GLenum target, GLenum pname, GLint *params);
    void getTexEnvxv(GLenum target, GLenum pname, GLfixed *params);
    void getTexParameterxv(GLenum target, GLenum pname, GLfixed *params);
    void lightModelf(GLenum pname, GLfloat param);
    void lightModelfv(GLenum pname, const GLfloat *params);
    void lightModelx(GLenum pname, GLfixed param);
    void lightModelxv(GLenum pname, const GLfixed *param);
    void lightf(GLenum light, GLenum pname, GLfloat param);
    void lightfv(GLenum light, GLenum pname, const GLfloat *params);
    void lightx(GLenum light, GLenum pname, GLfixed param);
    void lightxv(GLenum light, GLenum pname, const GLfixed *params);
    void lineWidthx(GLfixed width);
    void loadIdentity();
    void loadMatrixf(const GLfloat *m);
    void loadMatrixx(const GLfixed *m);
    void logicOp(GLenum opcode);
    void materialf(GLenum face, GLenum pname, GLfloat param);
    void materialfv(GLenum face, GLenum pname, const GLfloat *params);
    void materialx(GLenum face, GLenum pname, GLfixed param);
    void materialxv(GLenum face, GLenum pname, const GLfixed *param);
    void matrixMode(GLenum mode);
    void multMatrixf(const GLfloat *m);
    void multMatrixx(const GLfixed *m);
    void multiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
    void multiTexCoord4x(GLenum texture, GLfixed s, GLfixed t, GLfixed r, GLfixed q);
    void normal3f(GLfloat nx, GLfloat ny, GLfloat nz);
    void normal3x(GLfixed nx, GLfixed ny, GLfixed nz);
    void normalPointer(GLenum type, GLsizei stride, const void *pointer);
    void orthof(GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f);
    void orthox(GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f);
    void pointParameterf(GLenum pname, GLfloat param);
    void pointParameterfv(GLenum pname, const GLfloat *params);
    void pointParameterx(GLenum pname, GLfixed param);
    void pointParameterxv(GLenum pname, const GLfixed *params);
    void pointSize(GLfloat size);
    void pointSizex(GLfixed size);
    void polygonOffsetx(GLfixed factor, GLfixed units);
    void popMatrix();
    void pushMatrix();
    void rotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
    void rotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
    void sampleCoveragex(GLclampx value, GLboolean invert);
    void scalef(GLfloat x, GLfloat y, GLfloat z);
    void scalex(GLfixed x, GLfixed y, GLfixed z);
    void shadeModel(GLenum mode);
    void texCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
    void texEnvf(GLenum target, GLenum pname, GLfloat param);
    void texEnvfv(GLenum target, GLenum pname, const GLfloat *params);
    void texEnvi(GLenum target, GLenum pname, GLint param);
    void texEnviv(GLenum target, GLenum pname, const GLint *params);
    void texEnvx(GLenum target, GLenum pname, GLfixed param);
    void texEnvxv(GLenum target, GLenum pname, const GLfixed *params);
    void texParameterx(GLenum target, GLenum pname, GLfixed param);
    void texParameterxv(GLenum target, GLenum pname, const GLfixed *params);
    void translatef(GLfloat x, GLfloat y, GLfloat z);
    void translatex(GLfixed x, GLfixed y, GLfixed z);
    void vertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);

    // GL_OES_draw_texture
    void drawTexf(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height);
    void drawTexfv(const GLfloat *coords);
    void drawTexi(GLint x, GLint y, GLint z, GLint width, GLint height);
    void drawTexiv(const GLint *coords);
    void drawTexs(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height);
    void drawTexsv(const GLshort *coords);
    void drawTexx(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height);
    void drawTexxv(const GLfixed *coords);

    // GL_OES_matrix_palette
    void currentPaletteMatrix(GLuint matrixpaletteindex);
    void loadPaletteFromModelViewMatrix();
    void matrixIndexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
    void weightPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);

    // GL_OES_point_size_array
    void pointSizePointer(GLenum type, GLsizei stride, const void *pointer);

    // GL_OES_query_matrix
    GLbitfield queryMatrixx(GLfixed *mantissa, GLint *exponent);

    // GL_OES_texture_cube_map
    void getTexGenfv(GLenum coord, GLenum pname, GLfloat *params);
    void getTexGeniv(GLenum coord, GLenum pname, GLint *params);
    void getTexGenxv(GLenum coord, GLenum pname, GLfixed *params);
    void texGenf(GLenum coord, GLenum pname, GLfloat param);
    void texGenfv(GLenum coord, GLenum pname, const GLfloat *params);
    void texGeni(GLenum coord, GLenum pname, GLint param);
    void texGeniv(GLenum coord, GLenum pname, const GLint *params);
    void texGenx(GLenum coord, GLenum pname, GLfixed param);
    void texGenxv(GLenum coord, GLenum pname, const GLfixed *params);

    // GL_OES_framebuffer_object
    void bindFramebuffer(GLenum target, GLuint framebuffer);
    void bindRenderbuffer(GLenum target, GLuint renderbuffer);
    GLenum checkFramebufferStatus(GLenum target);
    void deleteFramebuffers(GLsizei n, const GLuint *framebuffers);
    void deleteRenderbuffers(GLsizei n, const GLuint *renderbuffers);
    void framebufferRenderbuffer(GLenum target,
                                 GLenum attachment,
                                 GLenum renderbuffertarget,
                                 GLuint renderbuffer);
    void framebufferTexture2D(GLenum target,
                              GLenum attachment,
                              GLenum textarget,
                              GLuint texture,
                              GLint level);
    void genFramebuffers(GLsizei n, GLuint *framebuffers);
    void genRenderbuffers(GLsizei n, GLuint *renderbuffers);
    void generateMipmap(GLenum target);
    void getFramebufferAttachmentParameteriv(GLenum target,
                                             GLenum attachment,
                                             GLenum pname,
                                             GLint *params);
    void getRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params);
    GLboolean isFramebuffer(GLuint framebuffer);
    GLboolean isRenderbuffer(GLuint renderbuffer);
    void renderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);

    // Interfaces to validation / context
    static constexpr int kVertexAttribIndex = 0;
    static constexpr int kNormalAttribIndex = 1;
    static constexpr int kColorAttribIndex = 2;
    static constexpr int kPointsizeAttribIndex = 3;
    static constexpr int kTexcoordAttribIndexBase = 4;

    GLint vertexArrayIndex(GLenum type) const;
    void prepareDraw(GLenum mode);

  private:
    void getBooleanvImpl(GLenum pname, GLboolean *data);
    void getFloatvImpl(GLenum pname, GLfloat *data);
    void getIntegervImpl(GLenum pname, GLint *data);

    // The GLES3 Context/State on which we are emulating GLES1.
    Context *mContext;
    State *mGLState;

    // GLES1 emulation state
    struct GLES1DrawTexState
    {
        GLuint program;
        GLint xyzwNdcLoc;
        GLint texuvwhCropLoc;
        GLint texNdcDimsLoc;
        GLint samplerLoc;
    };

    struct LightingBuffer
    {
        GLint lightEnables[8];
        GLfloat lightAmbients[4 * 8];
        GLfloat lightDiffuses[4 * 8];
        GLfloat lightSpeculars[4 * 8];
        GLfloat lightPositions[4 * 8];
        GLfloat lightDirections[3 * 8];
        GLfloat spotlightExponents[8];
        GLfloat spotlightCutoffAngles[8];
        GLfloat attenuationConsts[8];
        GLfloat attenuationLinears[8];
        GLfloat attenuationQuadratics[8];
    };

    struct TexEnvBuffer
    {
        GLfloat textureMatrices[16 * 4];

        GLint modes[4];
        GLint combineRgbs[4];
        GLint combineAlphas[4];
        GLint src0rgbs[4];
        GLint src0alphas[4];
        GLint src1rgbs[4];
        GLint src1alphas[4];
        GLint src2rgbs[4];
        GLint src2alphas[4];
        GLint op0rgbs[4];
        GLint op0alphas[4];
        GLint op1rgbs[4];
        GLint op1alphas[4];
        GLint op2rgbs[4];
        GLint op2alphas[4];
        GLfloat envColors[4 * 4];
        GLfloat rgbScales[4];
        GLfloat alphaScales[4];
        GLint pointSpriteCoordReplaces[4];
    };

    struct GLES1DrawState
    {
        GLuint program;

        GLint projMatrixLoc;
        GLint modelviewMatrixLoc;
        GLint textureMatrixLoc;
        GLint modelviewInvTrLoc;

        GLint textureSampler0Loc;
        GLint textureCubeSampler0Loc;
        GLint textureSampler1Loc;
        GLint textureCubeSampler1Loc;
        GLint textureSampler2Loc;
        GLint textureCubeSampler2Loc;
        GLint textureSampler3Loc;
        GLint textureCubeSampler3Loc;

        GLint shadeModelFlatLoc;

        GLint enableTexture2DLoc;
        GLint enableTextureCubeMapLoc;
        GLint enableLightingLoc;
        GLint enableRescaleNormalLoc;
        GLint enableNormalizeLoc;
        GLint enableColorMaterialLoc;
        GLint enableFogLoc;
        GLint enableReflectionMapLoc;
        GLint enableAlphaTestLoc;
        GLint enableLogicOpLoc;

        GLint alphaFuncLoc;
        GLint alphaTestRefLoc;

        GLint logicOpLoc;

        GLint textureFormatLoc;
        GLint textureEnvModeLoc;
        GLint combineRgbLoc;
        GLint combineAlphaLoc;
        GLint src0rgbLoc;
        GLint src0alphaLoc;
        GLint src1rgbLoc;
        GLint src1alphaLoc;
        GLint src2rgbLoc;
        GLint src2alphaLoc;
        GLint op0rgbLoc;
        GLint op0alphaLoc;
        GLint op1rgbLoc;
        GLint op1alphaLoc;
        GLint op2rgbLoc;
        GLint op2alphaLoc;
        GLint textureEnvColorLoc;
        GLint rgbScaleLoc;
        GLint alphaScaleLoc;
        GLint pointSpriteCoordReplaceLoc;

        GLint materialAmbientLoc;
        GLint materialDiffuseLoc;
        GLint materialSpecularLoc;
        GLint materialEmissiveLoc;
        GLint materialSpecularExponentLoc;

        GLint lightModelSceneAmbientLoc;
        GLint lightModelTwoSidedLoc;

        GLint lightEnablesLoc;
        GLint lightAmbientsLoc;
        GLint lightDiffusesLoc;
        GLint lightSpecularsLoc;
        GLint lightPositionsLoc;
        GLint lightDirectionsLoc;
        GLint lightSpotlightExponentsLoc;
        GLint lightSpotlightCutoffAnglesLoc;
        GLint lightAttenuationConstsLoc;
        GLint lightAttenuationLinearsLoc;
        GLint lightAttenuationQuadraticsLoc;

        GLint fogModeLoc;
        GLint fogDensityLoc;
        GLint fogStartLoc;
        GLint fogEndLoc;
        GLint fogColorLoc;

        GLint pointRasterizationLoc;
        GLint pointSpriteEnabledLoc;
        GLint pointSmoothEnabledLoc;
        GLint pointSizeMinLoc;
        GLint pointSizeMaxLoc;
        GLint pointDistanceAttenuationLoc;

        GLint enableClipPlanesLoc;
        GLint clipPlaneEnablesLoc;
        GLint clipPlanesLoc;

        LightingBuffer lightingBuffer;
        TexEnvBuffer texEnvBuffer;
    };

    GLES1DrawTexState drawTex;
    GLES1DrawState mDrawState;
};

}  // namespace gl

#endif  // LIBANGLE_GLES1_EMULATION_ES_H_
