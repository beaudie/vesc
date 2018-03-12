//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// GLES1Renderer.cpp: Implements the GLES1Renderer.

#include "libANGLE/GLES1Renderer.h"

#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/State.h"

namespace gl
{

GLES1Renderer::GLES1Renderer(Context *context, State *state) : mContext(context), mGLState(state)
{
    (void)mContext;
    (void)mGLState;
}

GLES1Renderer::~GLES1Renderer() = default;

void GLES1Renderer::alphaFunc(GLenum func, GLfloat ref)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::alphaFuncx(GLenum func, GLfixed ref)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::clearColorx(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::clearDepthx(GLfixed depth)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::clientActiveTexture(GLenum texture)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::clipPlanef(GLenum p, const GLfloat *eqn)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::clipPlanex(GLenum plane, const GLfixed *equation)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::color4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::color4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::color4x(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::colorPointer(GLint size, GLenum type, GLsizei stride, const void *ptr)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::depthRangex(GLfixed n, GLfixed f)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::disableClientState(GLenum clientState)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::enableClientState(GLenum clientState)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::fogf(GLenum pname, GLfloat param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::fogfv(GLenum pname, const GLfloat *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::fogx(GLenum pname, GLfixed param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::fogxv(GLenum pname, const GLfixed *param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::frustumf(GLfloat left,
                             GLfloat right,
                             GLfloat bottom,
                             GLfloat top,
                             GLfloat zNear,
                             GLfloat zFar)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::frustumx(GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::getClipPlanef(GLenum plane, GLfloat *equation)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::getClipPlanex(GLenum plane, GLfixed *equation)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::getFixedv(GLenum pname, GLfixed *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::getLightfv(GLenum light, GLenum pname, GLfloat *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::getLightxv(GLenum light, GLenum pname, GLfixed *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::getMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::getMaterialxv(GLenum face, GLenum pname, GLfixed *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::getPointerv(GLenum pname, void **params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::getTexEnvfv(GLenum env, GLenum pname, GLfloat *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::getTexEnviv(GLenum env, GLenum pname, GLint *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::getTexEnvxv(GLenum target, GLenum pname, GLfixed *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::getTexParameterxv(TextureType target, GLenum pname, GLfixed *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::lightModelf(GLenum pname, GLfloat param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::lightModelfv(GLenum pname, const GLfloat *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::lightModelx(GLenum pname, GLfixed param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::lightModelxv(GLenum pname, const GLfixed *param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::lightf(GLenum light, GLenum pname, GLfloat param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::lightfv(GLenum light, GLenum pname, const GLfloat *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::lightx(GLenum light, GLenum pname, GLfixed param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::lightxv(GLenum light, GLenum pname, const GLfixed *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::lineWidthx(GLfixed width)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::loadIdentity()
{
    UNIMPLEMENTED();
}

void GLES1Renderer::loadMatrixf(const GLfloat *m)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::loadMatrixx(const GLfixed *m)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::logicOp(GLenum opcode)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::materialf(GLenum face, GLenum pname, GLfloat param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::materialfv(GLenum face, GLenum pname, const GLfloat *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::materialx(GLenum face, GLenum pname, GLfixed param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::materialxv(GLenum face, GLenum pname, const GLfixed *param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::matrixMode(GLenum mode)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::multMatrixf(const GLfloat *m)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::multMatrixx(const GLfixed *m)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::multiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::multiTexCoord4x(GLenum texture, GLfixed s, GLfixed t, GLfixed r, GLfixed q)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::normal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::normal3x(GLfixed nx, GLfixed ny, GLfixed nz)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::normalPointer(GLenum type, GLsizei stride, const void *ptr)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::orthof(GLfloat left,
                           GLfloat right,
                           GLfloat bottom,
                           GLfloat top,
                           GLfloat zNear,
                           GLfloat zFar)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::orthox(GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::pointParameterf(GLenum pname, GLfloat param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::pointParameterfv(GLenum pname, const GLfloat *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::pointParameterx(GLenum pname, GLfixed param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::pointParameterxv(GLenum pname, const GLfixed *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::pointSize(GLfloat size)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::pointSizex(GLfixed size)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::polygonOffsetx(GLfixed factor, GLfixed units)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::popMatrix()
{
    UNIMPLEMENTED();
}

void GLES1Renderer::pushMatrix()
{
    UNIMPLEMENTED();
}

void GLES1Renderer::rotatef(float angle, float x, float y, float z)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::rotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::sampleCoveragex(GLclampx value, GLboolean invert)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::scalef(float x, float y, float z)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::scalex(GLfixed x, GLfixed y, GLfixed z)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::shadeModel(GLenum mode)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::texCoordPointer(GLint size, GLenum type, GLsizei stride, const void *ptr)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::texEnvf(GLenum target, GLenum pname, GLfloat param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::texEnvfv(GLenum target, GLenum pname, const GLfloat *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::texEnvi(GLenum target, GLenum pname, GLint param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::texEnviv(GLenum target, GLenum pname, const GLint *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::texEnvx(GLenum target, GLenum pname, GLfixed param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::texEnvxv(GLenum target, GLenum pname, const GLfixed *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::texParameterx(TextureType target, GLenum pname, GLfixed param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::texParameterxv(TextureType target, GLenum pname, const GLfixed *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::translatef(float x, float y, float z)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::translatex(GLfixed x, GLfixed y, GLfixed z)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::vertexPointer(GLint size, GLenum type, GLsizei stride, const void *ptr)
{
    UNIMPLEMENTED();
}

// GL_OES_draw_texture
void GLES1Renderer::drawTexf(float x, float y, float z, float width, float height)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::drawTexfv(const GLfloat *coords)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::drawTexi(GLint x, GLint y, GLint z, GLint width, GLint height)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::drawTexiv(const GLint *coords)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::drawTexs(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::drawTexsv(const GLshort *coords)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::drawTexx(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::drawTexxv(const GLfixed *coords)
{
    UNIMPLEMENTED();
}

// GL_OES_matrix_palette
void GLES1Renderer::currentPaletteMatrix(GLuint matrixpaletteindex)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::loadPaletteFromModelViewMatrix()
{
    UNIMPLEMENTED();
}

void GLES1Renderer::matrixIndexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::weightPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    UNIMPLEMENTED();
}

// GL_OES_point_size_array
void GLES1Renderer::pointSizePointer(GLenum type, GLsizei stride, const void *ptr)
{
    UNIMPLEMENTED();
}

// GL_OES_query_matrix
GLbitfield GLES1Renderer::queryMatrixx(GLfixed *mantissa, GLint *exponent)
{
    UNIMPLEMENTED();
    return 0;
}

// GL_OES_texture_cube_map
void GLES1Renderer::getTexGenfv(GLenum coord, GLenum pname, GLfloat *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::getTexGeniv(GLenum coord, GLenum pname, GLint *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::getTexGenxv(GLenum coord, GLenum pname, GLfixed *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::texGenf(GLenum coord, GLenum pname, GLfloat param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::texGenfv(GLenum coord, GLenum pname, const GLfloat *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::texGeni(GLenum coord, GLenum pname, GLint param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::texGeniv(GLenum coord, GLenum pname, const GLint *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::texGenx(GLenum coord, GLenum pname, GLfixed param)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::texGenxv(GLenum coord, GLenum pname, const GLint *params)
{
    UNIMPLEMENTED();
}

// GL_OES_framebuffer_object

void GLES1Renderer::bindFramebuffer(GLenum target, GLuint framebuffer)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::bindRenderbuffer(GLenum target, GLuint renderbuffer)
{
    UNIMPLEMENTED();
}

GLenum GLES1Renderer::checkFramebufferStatus(GLenum target)
{
    UNIMPLEMENTED();
    return GL_FRAMEBUFFER_UNSUPPORTED;
}

void GLES1Renderer::deleteFramebuffers(GLsizei n, const GLuint *framebuffers)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::deleteRenderbuffers(GLsizei n, const GLuint *renderbuffers)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::framebufferRenderbuffer(GLenum target,
                                            GLenum attachment,
                                            GLenum renderbuffertarget,
                                            GLuint renderbuffer)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::framebufferTexture2D(GLenum target,
                                         GLenum attachment,
                                         GLenum textarget,
                                         GLuint texture,
                                         GLint level)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::genFramebuffers(GLsizei n, GLuint *framebuffers)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::genRenderbuffers(GLsizei n, GLuint *renderbuffers)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::generateMipmap(GLenum target)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::getFramebufferAttachmentParameteriv(GLenum target,
                                                        GLenum attachment,
                                                        GLenum pname,
                                                        GLint *params)
{
    UNIMPLEMENTED();
}

void GLES1Renderer::getRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    UNIMPLEMENTED();
}

GLboolean GLES1Renderer::isFramebuffer(GLuint framebuffer)
{
    UNIMPLEMENTED();
    return GL_FALSE;
}

GLboolean GLES1Renderer::isRenderbuffer(GLuint renderbuffer)
{
    UNIMPLEMENTED();
    return GL_FALSE;
}

void GLES1Renderer::renderbufferStorage(GLenum target,
                                        GLenum internalformat,
                                        GLsizei width,
                                        GLsizei height)
{
    UNIMPLEMENTED();
}

}  // namespace gl
