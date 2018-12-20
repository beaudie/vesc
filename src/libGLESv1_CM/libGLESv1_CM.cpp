//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// libGLESv1_CM.cpp: Implements the exported OpenGL ES 1.0 functions.

#include "common/system_utils.h"

#include <memory>

#if !defined(ANGLE_USE_GLES1_CM_LOADER)
#    error This file only works with dynamic loading.
#endif  // !defined(ANGLE_USE_EGL_LOADER)

#include "libGLESv1_CM/gles_loader_autogen.h"

namespace
{
bool gLoaded = false;
std::unique_ptr<angle::Library> gEntryPointsLib;

angle::GenericProc KHRONOS_APIENTRY GlobalLoad(const char *symbol)
{
    return reinterpret_cast<angle::GenericProc>(gEntryPointsLib->getSymbol(symbol));
}

void EnsureLoaded()
{
    if (gLoaded)
        return;

    gEntryPointsLib.reset(angle::OpenSharedLibrary(ANGLE_GLESV2_LIBRARY_NAME));
    angle::LoadGLES(GlobalLoad);
    if (!_glGetString)
    {
        fprintf(stderr, "Error loading EGL entry points.\n");
    }
    else
    {
        gLoaded = true;
    }
}
}  // anonymous namespace

extern "C" {

void GL_APIENTRY glAlphaFunc(GLenum func, GLfloat ref)
{
    EnsureLoaded();
    return _glAlphaFunc(func, ref);
}

void GL_APIENTRY glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    EnsureLoaded();
    return _glClearColor(red, green, blue, alpha);
}

void GL_APIENTRY glClearDepthf(GLfloat d)
{
    EnsureLoaded();
    return _glClearDepthf(d);
}

void GL_APIENTRY glClipPlanef(GLenum p, const GLfloat *eqn)
{
    EnsureLoaded();
    return _glClipPlanef(p, eqn);
}

void GL_APIENTRY glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    EnsureLoaded();
    return _glColor4f(red, green, blue, alpha);
}

void GL_APIENTRY glDepthRangef(GLfloat n, GLfloat f)
{
    EnsureLoaded();
    return _glDepthRangef(n, f);
}

void GL_APIENTRY glFogf(GLenum pname, GLfloat param)
{
    EnsureLoaded();
    return _glFogf(pname, param);
}

void GL_APIENTRY glFogfv(GLenum pname, const GLfloat *params)
{
    EnsureLoaded();
    return _glFogfv(pname, params);
}

void GL_APIENTRY glFrustumf(GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f)
{
    EnsureLoaded();
    return _glFrustumf(l, r, b, t, n, f);
}

void GL_APIENTRY glGetClipPlanef(GLenum plane, GLfloat *equation)
{
    EnsureLoaded();
    return _glGetClipPlanef(plane, equation);
}

void GL_APIENTRY glGetFloatv(GLenum pname, GLfloat *data)
{
    EnsureLoaded();
    return _glGetFloatv(pname, data);
}

void GL_APIENTRY glGetLightfv(GLenum light, GLenum pname, GLfloat *params)
{
    EnsureLoaded();
    return _glGetLightfv(light, pname, params);
}

void GL_APIENTRY glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
    EnsureLoaded();
    return _glGetMaterialfv(face, pname, params);
}

void GL_APIENTRY glGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params)
{
    EnsureLoaded();
    return _glGetTexEnvfv(target, pname, params);
}

void GL_APIENTRY glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    EnsureLoaded();
    return _glGetTexParameterfv(target, pname, params);
}

void GL_APIENTRY glLightModelf(GLenum pname, GLfloat param)
{
    EnsureLoaded();
    return _glLightModelf(pname, param);
}

void GL_APIENTRY glLightModelfv(GLenum pname, const GLfloat *params)
{
    EnsureLoaded();
    return _glLightModelfv(pname, params);
}

void GL_APIENTRY glLightf(GLenum light, GLenum pname, GLfloat param)
{
    EnsureLoaded();
    return _glLightf(light, pname, param);
}

void GL_APIENTRY glLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
    EnsureLoaded();
    return _glLightfv(light, pname, params);
}

void GL_APIENTRY glLineWidth(GLfloat width)
{
    EnsureLoaded();
    return _glLineWidth(width);
}

void GL_APIENTRY glLoadMatrixf(const GLfloat *m)
{
    EnsureLoaded();
    return _glLoadMatrixf(m);
}

void GL_APIENTRY glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
    EnsureLoaded();
    return _glMaterialf(face, pname, param);
}

void GL_APIENTRY glMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
    EnsureLoaded();
    return _glMaterialfv(face, pname, params);
}

void GL_APIENTRY glMultMatrixf(const GLfloat *m)
{
    EnsureLoaded();
    return _glMultMatrixf(m);
}

void GL_APIENTRY glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    EnsureLoaded();
    return _glMultiTexCoord4f(target, s, t, r, q);
}

void GL_APIENTRY glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
    EnsureLoaded();
    return _glNormal3f(nx, ny, nz);
}

void GL_APIENTRY glOrthof(GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f)
{
    EnsureLoaded();
    return _glOrthof(l, r, b, t, n, f);
}

void GL_APIENTRY glPointParameterf(GLenum pname, GLfloat param)
{
    EnsureLoaded();
    return _glPointParameterf(pname, param);
}

void GL_APIENTRY glPointParameterfv(GLenum pname, const GLfloat *params)
{
    EnsureLoaded();
    return _glPointParameterfv(pname, params);
}

void GL_APIENTRY glPointSize(GLfloat size)
{
    EnsureLoaded();
    return _glPointSize(size);
}

void GL_APIENTRY glPolygonOffset(GLfloat factor, GLfloat units)
{
    EnsureLoaded();
    return _glPolygonOffset(factor, units);
}

void GL_APIENTRY glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    EnsureLoaded();
    return _glRotatef(angle, x, y, z);
}

void GL_APIENTRY glScalef(GLfloat x, GLfloat y, GLfloat z)
{
    EnsureLoaded();
    return _glScalef(x, y, z);
}

void GL_APIENTRY glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
    EnsureLoaded();
    return _glTexEnvf(target, pname, param);
}

void GL_APIENTRY glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params)
{
    EnsureLoaded();
    return _glTexEnvfv(target, pname, params);
}

void GL_APIENTRY glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    EnsureLoaded();
    return _glTexParameterf(target, pname, param);
}

void GL_APIENTRY glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    EnsureLoaded();
    return _glTexParameterfv(target, pname, params);
}

void GL_APIENTRY glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
    EnsureLoaded();
    return _glTranslatef(x, y, z);
}

void GL_APIENTRY glActiveTexture(GLenum texture)
{
    EnsureLoaded();
    return _glActiveTexture(texture);
}

void GL_APIENTRY glAlphaFuncx(GLenum func, GLfixed ref)
{
    EnsureLoaded();
    return _glAlphaFuncx(func, ref);
}

void GL_APIENTRY glBindBuffer(GLenum target, GLuint buffer)
{
    EnsureLoaded();
    return _glBindBuffer(target, buffer);
}

void GL_APIENTRY glBindTexture(GLenum target, GLuint texture)
{
    EnsureLoaded();
    return _glBindTexture(target, texture);
}

void GL_APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    EnsureLoaded();
    return _glBlendFunc(sfactor, dfactor);
}

void GL_APIENTRY glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage)
{
    EnsureLoaded();
    return _glBufferData(target, size, data, usage);
}

void GL_APIENTRY glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data)
{
    EnsureLoaded();
    return _glBufferSubData(target, offset, size, data);
}

void GL_APIENTRY glClear(GLbitfield mask)
{
    EnsureLoaded();
    return _glClear(mask);
}

void GL_APIENTRY glClearColorx(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
    EnsureLoaded();
    return _glClearColorx(red, green, blue, alpha);
}

void GL_APIENTRY glClearDepthx(GLfixed depth)
{
    EnsureLoaded();
    return _glClearDepthx(depth);
}

void GL_APIENTRY glClearStencil(GLint s)
{
    EnsureLoaded();
    return _glClearStencil(s);
}

void GL_APIENTRY glClientActiveTexture(GLenum texture)
{
    EnsureLoaded();
    return _glClientActiveTexture(texture);
}

void GL_APIENTRY glClipPlanex(GLenum plane, const GLfixed *equation)
{
    EnsureLoaded();
    return _glClipPlanex(plane, equation);
}

void GL_APIENTRY glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    EnsureLoaded();
    return _glColor4ub(red, green, blue, alpha);
}

void GL_APIENTRY glColor4x(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
    EnsureLoaded();
    return _glColor4x(red, green, blue, alpha);
}

void GL_APIENTRY glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    EnsureLoaded();
    return _glColorMask(red, green, blue, alpha);
}

void GL_APIENTRY glColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    EnsureLoaded();
    return _glColorPointer(size, type, stride, pointer);
}

void GL_APIENTRY glCompressedTexImage2D(GLenum target,
                                        GLint level,
                                        GLenum internalformat,
                                        GLsizei width,
                                        GLsizei height,
                                        GLint border,
                                        GLsizei imageSize,
                                        const void *data)
{
    EnsureLoaded();
    return _glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize,
                                   data);
}

void GL_APIENTRY glCompressedTexSubImage2D(GLenum target,
                                           GLint level,
                                           GLint xoffset,
                                           GLint yoffset,
                                           GLsizei width,
                                           GLsizei height,
                                           GLenum format,
                                           GLsizei imageSize,
                                           const void *data)
{
    EnsureLoaded();
    return _glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format,
                                      imageSize, data);
}

void GL_APIENTRY glCopyTexImage2D(GLenum target,
                                  GLint level,
                                  GLenum internalformat,
                                  GLint x,
                                  GLint y,
                                  GLsizei width,
                                  GLsizei height,
                                  GLint border)
{
    EnsureLoaded();
    return _glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}

void GL_APIENTRY glCopyTexSubImage2D(GLenum target,
                                     GLint level,
                                     GLint xoffset,
                                     GLint yoffset,
                                     GLint x,
                                     GLint y,
                                     GLsizei width,
                                     GLsizei height)
{
    EnsureLoaded();
    return _glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

void GL_APIENTRY glCullFace(GLenum mode)
{
    EnsureLoaded();
    return _glCullFace(mode);
}

void GL_APIENTRY glDeleteBuffers(GLsizei n, const GLuint *buffers)
{
    EnsureLoaded();
    return _glDeleteBuffers(n, buffers);
}

void GL_APIENTRY glDeleteTextures(GLsizei n, const GLuint *textures)
{
    EnsureLoaded();
    return _glDeleteTextures(n, textures);
}

void GL_APIENTRY glDepthFunc(GLenum func)
{
    EnsureLoaded();
    return _glDepthFunc(func);
}

void GL_APIENTRY glDepthMask(GLboolean flag)
{
    EnsureLoaded();
    return _glDepthMask(flag);
}

void GL_APIENTRY glDepthRangex(GLfixed n, GLfixed f)
{
    EnsureLoaded();
    return _glDepthRangex(n, f);
}

void GL_APIENTRY glDisable(GLenum cap)
{
    EnsureLoaded();
    return _glDisable(cap);
}

void GL_APIENTRY glDisableClientState(GLenum array)
{
    EnsureLoaded();
    return _glDisableClientState(array);
}

void GL_APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    EnsureLoaded();
    return _glDrawArrays(mode, first, count);
}

void GL_APIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
    EnsureLoaded();
    return _glDrawElements(mode, count, type, indices);
}

void GL_APIENTRY glEnable(GLenum cap)
{
    EnsureLoaded();
    return _glEnable(cap);
}

void GL_APIENTRY glEnableClientState(GLenum array)
{
    EnsureLoaded();
    return _glEnableClientState(array);
}

void GL_APIENTRY glFinish(void)
{
    EnsureLoaded();
    return _glFinish();
}

void GL_APIENTRY glFlush(void)
{
    EnsureLoaded();
    return _glFlush();
}

void GL_APIENTRY glFogx(GLenum pname, GLfixed param)
{
    EnsureLoaded();
    return _glFogx(pname, param);
}

void GL_APIENTRY glFogxv(GLenum pname, const GLfixed *param)
{
    EnsureLoaded();
    return _glFogxv(pname, param);
}

void GL_APIENTRY glFrontFace(GLenum mode)
{
    EnsureLoaded();
    return _glFrontFace(mode);
}

void GL_APIENTRY glFrustumx(GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f)
{
    EnsureLoaded();
    return _glFrustumx(l, r, b, t, n, f);
}

void GL_APIENTRY glGetBooleanv(GLenum pname, GLboolean *data)
{
    EnsureLoaded();
    return _glGetBooleanv(pname, data);
}

void GL_APIENTRY glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    EnsureLoaded();
    return _glGetBufferParameteriv(target, pname, params);
}

void GL_APIENTRY glGetClipPlanex(GLenum plane, GLfixed *equation)
{
    EnsureLoaded();
    return _glGetClipPlanex(plane, equation);
}

void GL_APIENTRY glGenBuffers(GLsizei n, GLuint *buffers)
{
    EnsureLoaded();
    return _glGenBuffers(n, buffers);
}

void GL_APIENTRY glGenTextures(GLsizei n, GLuint *textures)
{
    EnsureLoaded();
    return _glGenTextures(n, textures);
}

GLenum GL_APIENTRY glGetError(void)
{
    EnsureLoaded();
    return _glGetError();
}

void GL_APIENTRY glGetFixedv(GLenum pname, GLfixed *params)
{
    EnsureLoaded();
    return _glGetFixedv(pname, params);
}

void GL_APIENTRY glGetIntegerv(GLenum pname, GLint *data)
{
    EnsureLoaded();
    return _glGetIntegerv(pname, data);
}

void GL_APIENTRY glGetLightxv(GLenum light, GLenum pname, GLfixed *params)
{
    EnsureLoaded();
    return _glGetLightxv(light, pname, params);
}

void GL_APIENTRY glGetMaterialxv(GLenum face, GLenum pname, GLfixed *params)
{
    EnsureLoaded();
    return _glGetMaterialxv(face, pname, params);
}

void GL_APIENTRY glGetPointerv(GLenum pname, void **params)
{
    EnsureLoaded();
    return _glGetPointerv(pname, params);
}

const GLubyte *GL_APIENTRY glGetString(GLenum name)
{
    EnsureLoaded();
    return _glGetString(name);
}

void GL_APIENTRY glGetTexEnviv(GLenum target, GLenum pname, GLint *params)
{
    EnsureLoaded();
    return _glGetTexEnviv(target, pname, params);
}

void GL_APIENTRY glGetTexEnvxv(GLenum target, GLenum pname, GLfixed *params)
{
    EnsureLoaded();
    return _glGetTexEnvxv(target, pname, params);
}

void GL_APIENTRY glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
    EnsureLoaded();
    return _glGetTexParameteriv(target, pname, params);
}

void GL_APIENTRY glGetTexParameterxv(GLenum target, GLenum pname, GLfixed *params)
{
    EnsureLoaded();
    return _glGetTexParameterxv(target, pname, params);
}

void GL_APIENTRY glHint(GLenum target, GLenum mode)
{
    EnsureLoaded();
    return _glHint(target, mode);
}

GLboolean GL_APIENTRY glIsBuffer(GLuint buffer)
{
    EnsureLoaded();
    return _glIsBuffer(buffer);
}

GLboolean GL_APIENTRY glIsEnabled(GLenum cap)
{
    EnsureLoaded();
    return _glIsEnabled(cap);
}

GLboolean GL_APIENTRY glIsTexture(GLuint texture)
{
    EnsureLoaded();
    return _glIsTexture(texture);
}

void GL_APIENTRY glLightModelx(GLenum pname, GLfixed param)
{
    EnsureLoaded();
    return _glLightModelx(pname, param);
}

void GL_APIENTRY glLightModelxv(GLenum pname, const GLfixed *param)
{
    EnsureLoaded();
    return _glLightModelxv(pname, param);
}

void GL_APIENTRY glLightx(GLenum light, GLenum pname, GLfixed param)
{
    EnsureLoaded();
    return _glLightx(light, pname, param);
}

void GL_APIENTRY glLightxv(GLenum light, GLenum pname, const GLfixed *params)
{
    EnsureLoaded();
    return _glLightxv(light, pname, params);
}

void GL_APIENTRY glLineWidthx(GLfixed width)
{
    EnsureLoaded();
    return _glLineWidthx(width);
}

void GL_APIENTRY glLoadIdentity(void)
{
    EnsureLoaded();
    return _glLoadIdentity();
}

void GL_APIENTRY glLoadMatrixx(const GLfixed *m)
{
    EnsureLoaded();
    return _glLoadMatrixx(m);
}

void GL_APIENTRY glLogicOp(GLenum opcode)
{
    EnsureLoaded();
    return _glLogicOp(opcode);
}

void GL_APIENTRY glMaterialx(GLenum face, GLenum pname, GLfixed param)
{
    EnsureLoaded();
    return _glMaterialx(face, pname, param);
}

void GL_APIENTRY glMaterialxv(GLenum face, GLenum pname, const GLfixed *param)
{
    EnsureLoaded();
    return _glMaterialxv(face, pname, param);
}

void GL_APIENTRY glMatrixMode(GLenum mode)
{
    EnsureLoaded();
    return _glMatrixMode(mode);
}

void GL_APIENTRY glMultMatrixx(const GLfixed *m)
{
    EnsureLoaded();
    return _glMultMatrixx(m);
}

void GL_APIENTRY glMultiTexCoord4x(GLenum texture, GLfixed s, GLfixed t, GLfixed r, GLfixed q)
{
    EnsureLoaded();
    return _glMultiTexCoord4x(texture, s, t, r, q);
}

void GL_APIENTRY glNormal3x(GLfixed nx, GLfixed ny, GLfixed nz)
{
    EnsureLoaded();
    return _glNormal3x(nx, ny, nz);
}

void GL_APIENTRY glNormalPointer(GLenum type, GLsizei stride, const void *pointer)
{
    EnsureLoaded();
    return _glNormalPointer(type, stride, pointer);
}

void GL_APIENTRY glOrthox(GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f)
{
    EnsureLoaded();
    return _glOrthox(l, r, b, t, n, f);
}

void GL_APIENTRY glPixelStorei(GLenum pname, GLint param)
{
    EnsureLoaded();
    return _glPixelStorei(pname, param);
}

void GL_APIENTRY glPointParameterx(GLenum pname, GLfixed param)
{
    EnsureLoaded();
    return _glPointParameterx(pname, param);
}

void GL_APIENTRY glPointParameterxv(GLenum pname, const GLfixed *params)
{
    EnsureLoaded();
    return _glPointParameterxv(pname, params);
}

void GL_APIENTRY glPointSizex(GLfixed size)
{
    EnsureLoaded();
    return _glPointSizex(size);
}

void GL_APIENTRY glPolygonOffsetx(GLfixed factor, GLfixed units)
{
    EnsureLoaded();
    return _glPolygonOffsetx(factor, units);
}

void GL_APIENTRY glPopMatrix(void)
{
    EnsureLoaded();
    return _glPopMatrix();
}

void GL_APIENTRY glPushMatrix(void)
{
    EnsureLoaded();
    return _glPushMatrix();
}

void GL_APIENTRY glReadPixels(GLint x,
                              GLint y,
                              GLsizei width,
                              GLsizei height,
                              GLenum format,
                              GLenum type,
                              void *pixels)
{
    EnsureLoaded();
    return _glReadPixels(x, y, width, height, format, type, pixels);
}

void GL_APIENTRY glRotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z)
{
    EnsureLoaded();
    return _glRotatex(angle, x, y, z);
}

void GL_APIENTRY glSampleCoverage(GLfloat value, GLboolean invert)
{
    EnsureLoaded();
    return _glSampleCoverage(value, invert);
}

void GL_APIENTRY glSampleCoveragex(GLclampx value, GLboolean invert)
{
    EnsureLoaded();
    return _glSampleCoveragex(value, invert);
}

void GL_APIENTRY glScalex(GLfixed x, GLfixed y, GLfixed z)
{
    EnsureLoaded();
    return _glScalex(x, y, z);
}

void GL_APIENTRY glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    EnsureLoaded();
    return _glScissor(x, y, width, height);
}

void GL_APIENTRY glShadeModel(GLenum mode)
{
    EnsureLoaded();
    return _glShadeModel(mode);
}

void GL_APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
    EnsureLoaded();
    return _glStencilFunc(func, ref, mask);
}

void GL_APIENTRY glStencilMask(GLuint mask)
{
    EnsureLoaded();
    return _glStencilMask(mask);
}

void GL_APIENTRY glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
    EnsureLoaded();
    return _glStencilOp(fail, zfail, zpass);
}

void GL_APIENTRY glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    EnsureLoaded();
    return _glTexCoordPointer(size, type, stride, pointer);
}

void GL_APIENTRY glTexEnvi(GLenum target, GLenum pname, GLint param)
{
    EnsureLoaded();
    return _glTexEnvi(target, pname, param);
}

void GL_APIENTRY glTexEnvx(GLenum target, GLenum pname, GLfixed param)
{
    EnsureLoaded();
    return _glTexEnvx(target, pname, param);
}

void GL_APIENTRY glTexEnviv(GLenum target, GLenum pname, const GLint *params)
{
    EnsureLoaded();
    return _glTexEnviv(target, pname, params);
}

void GL_APIENTRY glTexEnvxv(GLenum target, GLenum pname, const GLfixed *params)
{
    EnsureLoaded();
    return _glTexEnvxv(target, pname, params);
}

void GL_APIENTRY glTexImage2D(GLenum target,
                              GLint level,
                              GLint internalformat,
                              GLsizei width,
                              GLsizei height,
                              GLint border,
                              GLenum format,
                              GLenum type,
                              const void *pixels)
{
    EnsureLoaded();
    return _glTexImage2D(target, level, internalformat, width, height, border, format, type,
                         pixels);
}

void GL_APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param)
{
    EnsureLoaded();
    return _glTexParameteri(target, pname, param);
}

void GL_APIENTRY glTexParameterx(GLenum target, GLenum pname, GLfixed param)
{
    EnsureLoaded();
    return _glTexParameterx(target, pname, param);
}

void GL_APIENTRY glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    EnsureLoaded();
    return _glTexParameteriv(target, pname, params);
}

void GL_APIENTRY glTexParameterxv(GLenum target, GLenum pname, const GLfixed *params)
{
    EnsureLoaded();
    return _glTexParameterxv(target, pname, params);
}

void GL_APIENTRY glTexSubImage2D(GLenum target,
                                 GLint level,
                                 GLint xoffset,
                                 GLint yoffset,
                                 GLsizei width,
                                 GLsizei height,
                                 GLenum format,
                                 GLenum type,
                                 const void *pixels)
{
    EnsureLoaded();
    return _glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void GL_APIENTRY glTranslatex(GLfixed x, GLfixed y, GLfixed z)
{
    EnsureLoaded();
    return _glTranslatex(x, y, z);
}

void GL_APIENTRY glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    EnsureLoaded();
    return _glVertexPointer(size, type, stride, pointer);
}

void GL_APIENTRY glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    EnsureLoaded();
    return _glViewport(x, y, width, height);
}

// GL_OES_draw_texture
void GL_APIENTRY glDrawTexfOES(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height)
{
    EnsureLoaded();
    return _glDrawTexfOES(x, y, z, width, height);
}

void GL_APIENTRY glDrawTexfvOES(const GLfloat *coords)
{
    EnsureLoaded();
    return _glDrawTexfvOES(coords);
}

void GL_APIENTRY glDrawTexiOES(GLint x, GLint y, GLint z, GLint width, GLint height)
{
    EnsureLoaded();
    return _glDrawTexiOES(x, y, z, width, height);
}

void GL_APIENTRY glDrawTexivOES(const GLint *coords)
{
    EnsureLoaded();
    return _glDrawTexivOES(coords);
}

void GL_APIENTRY glDrawTexsOES(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height)
{
    EnsureLoaded();
    return _glDrawTexsOES(x, y, z, width, height);
}

void GL_APIENTRY glDrawTexsvOES(const GLshort *coords)
{
    EnsureLoaded();
    return _glDrawTexsvOES(coords);
}

void GL_APIENTRY glDrawTexxOES(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height)
{
    EnsureLoaded();
    return _glDrawTexxOES(x, y, z, width, height);
}

void GL_APIENTRY glDrawTexxvOES(const GLfixed *coords)
{
    EnsureLoaded();
    return _glDrawTexxvOES(coords);
}

// GL_OES_matrix_palette
void GL_APIENTRY glCurrentPaletteMatrixOES(GLuint matrixpaletteindex)
{
    EnsureLoaded();
    return _glCurrentPaletteMatrixOES(matrixpaletteindex);
}

void GL_APIENTRY glLoadPaletteFromModelViewMatrixOES()
{
    EnsureLoaded();
    return _glLoadPaletteFromModelViewMatrixOES();
}

void GL_APIENTRY glMatrixIndexPointerOES(GLint size,
                                         GLenum type,
                                         GLsizei stride,
                                         const void *pointer)
{
    EnsureLoaded();
    return _glMatrixIndexPointerOES(size, type, stride, pointer);
}

void GL_APIENTRY glWeightPointerOES(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    EnsureLoaded();
    return _glWeightPointerOES(size, type, stride, pointer);
}

// GL_OES_point_size_array
void GL_APIENTRY glPointSizePointerOES(GLenum type, GLsizei stride, const void *pointer)
{
    EnsureLoaded();
    return _glPointSizePointerOES(type, stride, pointer);
}

// GL_OES_query_matrix
GLbitfield GL_APIENTRY glQueryMatrixxOES(GLfixed *mantissa, GLint *exponent)
{
    EnsureLoaded();
    return _glQueryMatrixxOES(mantissa, exponent);
}

}  // extern "C"
