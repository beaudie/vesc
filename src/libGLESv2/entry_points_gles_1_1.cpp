// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// entry_points_gles_1_1_autogen.cpp:
//   Defines the GLES 1.1 entry points.

#include "libANGLE/Context.h"
#include "libGLESv2/global_state.h"

namespace gl
{

void GL_APIENTRY ShadeModel(GLenum mode) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->shadeModel(mode);

    }
}

void GL_APIENTRY MatrixMode(GLenum mode) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->matrixMode(mode);

    }
}

void GL_APIENTRY LoadIdentity(void) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->loadIdentity();

    }
}

void GL_APIENTRY LoadMatrixf(const GLfloat* m) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->loadMatrixf(m);

    }
}

void GL_APIENTRY PushMatrix(void) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->pushMatrix();

    }
}

void GL_APIENTRY PopMatrix(void) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->popMatrix();

    }
}

void GL_APIENTRY MultMatrixf(const GLfloat* m) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->multMatrixf(m);

    }
}

void GL_APIENTRY Orthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->orthof(left, right, bottom, top, zNear, zFar);

    }
}

void GL_APIENTRY Frustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->frustumf(left, right, bottom, top, zNear, zFar);

    }
}

void GL_APIENTRY TexEnvf(GLenum target, GLenum pname, GLfloat param) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->texEnvf(target, pname, param);

    }
}

void GL_APIENTRY TexEnvfv(GLenum target, GLenum pname, const GLfloat* params) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->texEnvfv(target, pname, params);

    }
}

void GL_APIENTRY TexEnvi(GLenum target, GLenum pname, GLint param) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->texEnvi(target, pname, param);

    }
}

void GL_APIENTRY TexEnviv(GLenum target, GLenum pname, const GLint* params) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->texEnviv(target, pname, params);

    }
}

void GL_APIENTRY GetTexEnvfv(GLenum env, GLenum pname, GLfloat* params) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->getTexEnvfv(env, pname, params);

    }
}

void GL_APIENTRY GetTexEnviv(GLenum env, GLenum pname, GLint* params) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->getTexEnviv(env, pname, params);

    }
}

void GL_APIENTRY TexGenf(GLenum coord, GLenum pname, GLfloat param) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->texGenf(coord, pname, param);

    }
}

void GL_APIENTRY TexGenfv(GLenum coord, GLenum pname, const GLfloat* params) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->texGenfv(coord, pname, params);

    }
}

void GL_APIENTRY TexGeni(GLenum coord, GLenum pname, GLint param) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->texGeni(coord, pname, param);

    }
}

void GL_APIENTRY TexGeniv(GLenum coord, GLenum pname, const GLint* params) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->texGeniv(coord, pname, params);

    }
}

void GL_APIENTRY GetTexGeniv(GLenum coord, GLenum pname, GLint* params) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->getTexGeniv(coord, pname, params);

    }
}

void GL_APIENTRY GetTexGenfv(GLenum coord, GLenum pname, GLfloat* params) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->getTexGenfv(coord, pname, params);

    }
}

void GL_APIENTRY Materialf(GLenum face, GLenum pname, GLfloat param) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->materialf(face, pname, param);

    }
}

void GL_APIENTRY Materialfv(GLenum face, GLenum pname, const GLfloat* params) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->materialfv(face, pname, params);

    }
}

void GL_APIENTRY GetMaterialfv(GLenum face, GLenum pname, GLfloat* params) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->getMaterialfv(face, pname, params);

    }
}

void GL_APIENTRY LightModelf(GLenum pname, GLfloat param) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->lightModelf(pname, param);

    }
}

void GL_APIENTRY LightModelfv(GLenum pname, const GLfloat* params) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->lightModelfv(pname, params);

    }
}

void GL_APIENTRY Lightf(GLenum light, GLenum pname, GLfloat param) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->lightf(light, pname, param);

    }
}

void GL_APIENTRY Lightfv(GLenum light, GLenum pname, const GLfloat* params) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->lightfv(light, pname, params);

    }
}

void GL_APIENTRY GetLightfv(GLenum light, GLenum pname, GLfloat* params) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->getLightfv(light, pname, params);

    }
}

void GL_APIENTRY MultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->multiTexCoord4f(target, s, t, r, q);

    }
}

void GL_APIENTRY Normal3f(GLfloat nx, GLfloat ny, GLfloat nz) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->normal3f(nx, ny, nz);

    }
}

void GL_APIENTRY Fogf(GLenum pname, GLfloat param) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->fogf(pname, param);

    }
}

void GL_APIENTRY Fogfv(GLenum pname, const GLfloat* params) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->fogfv(pname, params);

    }
}

void GL_APIENTRY EnableClientState(GLenum clientState) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->enableClientState(clientState);

    }
}

void GL_APIENTRY DisableClientState(GLenum clientState) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->disableClientState(clientState);

    }
}

void GL_APIENTRY DrawTexOES(float x, float y, float z, float width, float height) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->drawTexOES(x, y, z, width, height);

    }
}

void GL_APIENTRY Rotatef(float angle, float x, float y, float z) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->rotatef(angle, x, y, z);

    }
}

void GL_APIENTRY Scalef(float x, float y, float z) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->scalef(x, y, z);

    }
}

void GL_APIENTRY Translatef(float x, float y, float z) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->translatef(x, y, z);

    }
}

void GL_APIENTRY Color4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->color4f(red, green, blue, alpha);

    }
}

void GL_APIENTRY ClientActiveTexture(GLenum texture) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->clientActiveTexture(texture);

    }
}

void GL_APIENTRY VertexPointer(GLint size, GLenum type, GLsizei stride, const void* ptr) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->vertexPointer(size, type, stride, ptr);

    }
}

void GL_APIENTRY NormalPointer(GLenum type, GLsizei stride, const void* ptr) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->normalPointer(type, stride, ptr);

    }
}

void GL_APIENTRY ColorPointer(GLint size, GLenum type, GLsizei stride, const void* ptr) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->colorPointer(size, type, stride, ptr);

    }
}

void GL_APIENTRY PointSizePointerOES(GLenum type, GLsizei stride, const void* ptr) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->pointSizePointerOES(type, stride, ptr);

    }
}

void GL_APIENTRY TexCoordPointer(GLint size, GLenum type, GLsizei stride, const void* ptr) {

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->texCoordPointer(size, type, stride, ptr);

    }
}


} // namespace gl
