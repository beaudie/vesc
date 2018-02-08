//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationES1.cpp: Validation functions for OpenGL ES 1.0 entry point parameters

#include "libANGLE/validationES1.h"

#include "common/debug.h"

namespace gl
{

bool ValidateAlphaFunc(Context *context, GLenum func, GLfloat ref)
{
    return true;
}

bool ValidateAlphaFuncx(Context *context, GLenum func, GLfixed ref)
{
    return true;
}

bool ValidateClearColorx(Context *context, GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
    return true;
}

bool ValidateClearDepthx(Context *context, GLfixed depth)
{
    return true;
}

bool ValidateClientActiveTexture(Context *context, GLenum texture)
{
    return true;
}

bool ValidateClipPlanef(Context *context, GLenum p, const GLfloat *eqn)
{
    return true;
}

bool ValidateClipPlanex(Context *context, GLenum plane, const GLfixed *equation)
{
    return true;
}

bool ValidateColor4f(Context *context, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    return true;
}

bool ValidateColor4ub(Context *context, GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    return true;
}

bool ValidateColor4x(Context *context, GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
    return true;
}

bool ValidateColorPointer(Context *context,
                          GLint size,
                          GLenum type,
                          GLsizei stride,
                          const void *pointer)
{
    return true;
}

bool ValidateCullFace(Context *context, GLenum mode)
{
    return true;
}

bool ValidateDepthRangex(Context *context, GLfixed n, GLfixed f)
{
    return true;
}

bool ValidateDisableClientState(Context *context, GLenum array)
{
    return true;
}

bool ValidateEnableClientState(Context *context, GLenum array)
{
    return true;
}

bool ValidateFogf(Context *context, GLenum pname, GLfloat param)
{
    return true;
}

bool ValidateFogfv(Context *context, GLenum pname, const GLfloat *params)
{
    return true;
}

bool ValidateFogx(Context *context, GLenum pname, GLfixed param)
{
    UNIMPLEMENTED();
    return true;
}

bool ValidateFogxv(Context *context, GLenum pname, const GLfixed *param)
{
    return true;
}

bool ValidateFrustumf(Context *context,
                      GLfloat l,
                      GLfloat r,
                      GLfloat b,
                      GLfloat t,
                      GLfloat n,
                      GLfloat f)
{
    return true;
}

bool ValidateFrustumx(Context *context,
                      GLfixed l,
                      GLfixed r,
                      GLfixed b,
                      GLfixed t,
                      GLfixed n,
                      GLfixed f)
{
    return true;
}

bool ValidateGetBufferParameteriv(Context *context, GLenum target, GLenum pname, GLint *params)
{
    return true;
}

bool ValidateGetClipPlanef(Context *context, GLenum plane, GLfloat *equation)
{
    return true;
}

bool ValidateGetClipPlanex(Context *context, GLenum plane, GLfixed *equation)
{
    return true;
}

bool ValidateGetFixedv(Context *context, GLenum pname, GLfixed *params)
{
    return true;
}

bool ValidateGetLightfv(Context *context, GLenum light, GLenum pname, GLfloat *params)
{
    return true;
}

bool ValidateGetLightxv(Context *context, GLenum light, GLenum pname, GLfixed *params)
{
    return true;
}

bool ValidateGetMaterialfv(Context *context, GLenum face, GLenum pname, GLfloat *params)
{
    return true;
}

bool ValidateGetMaterialxv(Context *context, GLenum face, GLenum pname, GLfixed *params)
{
    return true;
}

bool ValidateGetPointerv(Context *context, GLenum pname, void **params)
{
    return true;
}

bool ValidateGetTexEnvfv(Context *context, GLenum target, GLenum pname, GLfloat *params)
{
    return true;
}

bool ValidateGetTexEnviv(Context *context, GLenum target, GLenum pname, GLint *params)
{
    return true;
}

bool ValidateGetTexEnvxv(Context *context, GLenum target, GLenum pname, GLfixed *params)
{
    return true;
}

bool ValidateGetTexParameterxv(Context *context, GLenum target, GLenum pname, GLfixed *params)
{
    return true;
}

bool ValidateLightModelf(Context *context, GLenum pname, GLfloat param)
{
    return true;
}

bool ValidateLightModelfv(Context *context, GLenum pname, const GLfloat *params)
{
    return true;
}

bool ValidateLightModelx(Context *context, GLenum pname, GLfixed param)
{
    return true;
}

bool ValidateLightModelxv(Context *context, GLenum pname, const GLfixed *param)
{
    return true;
}

bool ValidateLightf(Context *context, GLenum light, GLenum pname, GLfloat param)
{
    return true;
}

bool ValidateLightfv(Context *context, GLenum light, GLenum pname, const GLfloat *params)
{
    return true;
}

bool ValidateLightx(Context *context, GLenum light, GLenum pname, GLfixed param)
{
    return true;
}

bool ValidateLightxv(Context *context, GLenum light, GLenum pname, const GLfixed *params)
{
    return true;
}

bool ValidateLineWidthx(Context *context, GLfixed width)
{
    return true;
}

bool ValidateLoadIdentity(Context *context)
{
    return true;
}

bool ValidateLoadMatrixf(Context *context, const GLfloat *m)
{
    return true;
}

bool ValidateLoadMatrixx(Context *context, const GLfixed *m)
{
    return true;
}

bool ValidateLogicOp(Context *context, GLenum opcode)
{
    return true;
}

bool ValidateMaterialf(Context *context, GLenum face, GLenum pname, GLfloat param)
{
    return true;
}

bool ValidateMaterialfv(Context *context, GLenum face, GLenum pname, const GLfloat *params)
{
    return true;
}

bool ValidateMaterialx(Context *context, GLenum face, GLenum pname, GLfixed param)
{
    return true;
}

bool ValidateMaterialxv(Context *context, GLenum face, GLenum pname, const GLfixed *param)
{
    return true;
}

bool ValidateMatrixMode(Context *context, GLenum mode)
{
    return true;
}

bool ValidateMultMatrixf(Context *context, const GLfloat *m)
{
    return true;
}

bool ValidateMultMatrixx(Context *context, const GLfixed *m)
{
    return true;
}

bool ValidateMultiTexCoord4f(Context *context,
                             GLenum target,
                             GLfloat s,
                             GLfloat t,
                             GLfloat r,
                             GLfloat q)
{
    return true;
}

bool ValidateMultiTexCoord4x(Context *context,
                             GLenum texture,
                             GLfixed s,
                             GLfixed t,
                             GLfixed r,
                             GLfixed q)
{
    return true;
}

bool ValidateNormal3f(Context *context, GLfloat nx, GLfloat ny, GLfloat nz)
{
    return true;
}

bool ValidateNormal3x(Context *context, GLfixed nx, GLfixed ny, GLfixed nz)
{
    return true;
}

bool ValidateNormalPointer(Context *context, GLenum type, GLsizei stride, const void *pointer)
{
    return true;
}

bool ValidateOrthof(Context *context,
                    GLfloat l,
                    GLfloat r,
                    GLfloat b,
                    GLfloat t,
                    GLfloat n,
                    GLfloat f)
{
    return true;
}

bool ValidateOrthox(Context *context,
                    GLfixed l,
                    GLfixed r,
                    GLfixed b,
                    GLfixed t,
                    GLfixed n,
                    GLfixed f)
{
    return true;
}

bool ValidatePointParameterf(Context *context, GLenum pname, GLfloat param)
{
    return true;
}

bool ValidatePointParameterfv(Context *context, GLenum pname, const GLfloat *params)
{
    return true;
}

bool ValidatePointParameterx(Context *context, GLenum pname, GLfixed param)
{
    return true;
}

bool ValidatePointParameterxv(Context *context, GLenum pname, const GLfixed *params)
{
    return true;
}

bool ValidatePointSize(Context *context, GLfloat size)
{
    return true;
}

bool ValidatePointSizex(Context *context, GLfixed size)
{
    return true;
}

bool ValidatePolygonOffsetx(Context *context, GLfixed factor, GLfixed units)
{
    return true;
}

bool ValidatePopMatrix(Context *context)
{
    return true;
}

bool ValidatePushMatrix(Context *context)
{
    return true;
}

bool ValidateRotatef(Context *context, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    return true;
}

bool ValidateRotatex(Context *context, GLfixed angle, GLfixed x, GLfixed y, GLfixed z)
{
    return true;
}

bool ValidateSampleCoveragex(Context *context, GLclampx value, GLboolean invert)
{
    return true;
}

bool ValidateScalef(Context *context, GLfloat x, GLfloat y, GLfloat z)
{
    return true;
}

bool ValidateScalex(Context *context, GLfixed x, GLfixed y, GLfixed z)
{
    return true;
}

bool ValidateShadeModel(Context *context, GLenum mode)
{
    return true;
}

bool ValidateTexCoordPointer(Context *context,
                             GLint size,
                             GLenum type,
                             GLsizei stride,
                             const void *pointer)
{
    return true;
}

bool ValidateTexEnvf(Context *context, GLenum target, GLenum pname, GLfloat param)
{
    return true;
}

bool ValidateTexEnvfv(Context *context, GLenum target, GLenum pname, const GLfloat *params)
{
    return true;
}

bool ValidateTexEnvi(Context *context, GLenum target, GLenum pname, GLint param)
{
    return true;
}

bool ValidateTexEnviv(Context *context, GLenum target, GLenum pname, const GLint *params)
{
    return true;
}

bool ValidateTexEnvx(Context *context, GLenum target, GLenum pname, GLfixed param)
{
    return true;
}

bool ValidateTexEnvxv(Context *context, GLenum target, GLenum pname, const GLfixed *params)
{
    return true;
}

bool ValidateTexParameterx(Context *context, GLenum target, GLenum pname, GLfixed param)
{
    return true;
}

bool ValidateTexParameterxv(Context *context, GLenum target, GLenum pname, const GLfixed *params)
{
    return true;
}

bool ValidateTranslatef(Context *context, GLfloat x, GLfloat y, GLfloat z)
{
    return true;
}

bool ValidateTranslatex(Context *context, GLfixed x, GLfixed y, GLfixed z)
{
    return true;
}

bool ValidateVertexPointer(Context *context,
                           GLint size,
                           GLenum type,
                           GLsizei stride,
                           const void *pointer)
{
    return true;
}

bool ValidateDrawTexfOES(Context *context,
                         GLfloat x,
                         GLfloat y,
                         GLfloat z,
                         GLfloat width,
                         GLfloat height)
{
    return true;
}

bool ValidateDrawTexfvOES(Context *context, const GLfloat *coords)
{
    return true;
}

bool ValidateDrawTexiOES(Context *context, GLint x, GLint y, GLint z, GLint width, GLint height)
{
    return true;
}

bool ValidateDrawTexivOES(Context *context, const GLint *coords)
{
    return true;
}

bool ValidateDrawTexsOES(Context *context,
                         GLshort x,
                         GLshort y,
                         GLshort z,
                         GLshort width,
                         GLshort height)
{
    return true;
}

bool ValidateDrawTexsvOES(Context *context, const GLshort *coords)
{
    return true;
}

bool ValidateDrawTexxOES(Context *context,
                         GLfixed x,
                         GLfixed y,
                         GLfixed z,
                         GLfixed width,
                         GLfixed height)
{
    return true;
}

bool ValidateDrawTexxvOES(Context *context, const GLfixed *coords)
{
    return true;
}

bool ValidateCurrentPaletteMatrixOES(Context *context, GLuint matrixpaletteindex)
{
    return true;
}

bool ValidateLoadPaletteFromModelViewMatrixOES(Context *context)
{
    return true;
}

bool ValidateMatrixIndexPointerOES(Context *context,
                                   GLint size,
                                   GLenum type,
                                   GLsizei stride,
                                   const void *pointer)
{
    return true;
}

bool ValidateWeightPointerOES(Context *context,
                              GLint size,
                              GLenum type,
                              GLsizei stride,
                              const void *pointer)
{
    return true;
}

bool ValidatePointSizePointerOES(Context *context, GLenum type, GLsizei stride, const void *pointer)
{
    return true;
}

bool ValidateQueryMatrixxOES(Context *context, GLfixed *mantissa, GLint *exponent)
{
    return true;
}

bool ValidateGenFramebuffersOES(Context *context, GLsizei n, GLuint *framebuffers)
{
    return true;
}

bool ValidateDeleteFramebuffersOES(Context *context, GLsizei n, const GLuint *framebuffers)
{
    return true;
}

bool ValidateGenRenderbuffersOES(Context *context, GLsizei n, GLuint *renderbuffers)
{
    return true;
}

bool ValidateDeleteRenderbuffersOES(Context *context, GLsizei n, const GLuint *renderbuffers)
{
    return true;
}

bool ValidateBindFramebufferOES(Context *context, GLenum target, GLuint framebuffer)
{
    return true;
}

bool ValidateBindRenderbufferOES(Context *context, GLenum target, GLuint renderbuffer)
{
    return true;
}

bool ValidateCheckFramebufferStatusOES(Context *context, GLenum target)
{
    return true;
}

bool ValidateFramebufferRenderbufferOES(Context *context,
                                        GLenum target,
                                        GLenum attachment,
                                        GLenum rbtarget,
                                        GLuint renderbuffer)
{
    return true;
}

bool ValidateFramebufferTexture2DOES(Context *context,
                                     GLenum target,
                                     GLenum attachment,
                                     GLenum textarget,
                                     GLuint texture,
                                     GLint level)
{
    return true;
}

bool ValidateGenerateMipmapOES(Context *context, GLenum target)
{
    return true;
}

bool ValidateGetFramebufferAttachmentParameterivOES(Context *context,
                                                    GLenum target,
                                                    GLenum attachment,
                                                    GLenum pname,
                                                    GLint *params)
{
    return true;
}

bool ValidateGetRenderbufferParameterivOES(Context *context,
                                           GLenum target,
                                           GLenum pname,
                                           GLint *params)
{
    return true;
}

bool ValidateIsFramebufferOES(Context *context, GLuint framebuffer)
{
    return true;
}

bool ValidateIsRenderbufferOES(Context *context, GLuint renderbuffer)
{
    return true;
}

bool ValidateRenderbufferStorageOES(Context *context,
                                    GLenum target,
                                    GLint internalformat,
                                    GLsizei width,
                                    GLsizei height)
{
    return true;
}

// GL_OES_texture_cube_map

bool ValidateGetTexGenfvOES(Context *context, GLenum coord, GLenum pname, GLfloat *params)
{
    return true;
}

bool ValidateGetTexGenivOES(Context *context, GLenum coord, GLenum pname, int *params)
{
    return true;
}

bool ValidateGetTexGenxvOES(Context *context, GLenum coord, GLenum pname, GLfixed *params)
{
    return true;
}

bool ValidateTexGenfvOES(Context *context, GLenum coord, GLenum pname, const GLfloat *params)
{
    return true;
}

bool ValidateTexGenivOES(Context *context, GLenum coord, GLenum pname, const GLint *param)
{
    return true;
}

bool ValidateTexGenxvOES(Context *context, GLenum coord, GLenum pname, const GLint *param)
{
    return true;
}

bool ValidateTexGenfOES(Context *context, GLenum coord, GLenum pname, GLfloat param)
{
    return true;
}

bool ValidateTexGeniOES(Context *context, GLenum coord, GLenum pname, GLint param)
{
    return true;
}

bool ValidateTexGenxOES(Context *context, GLenum coord, GLenum pname, GLfixed param)
{
    return true;
}
}
