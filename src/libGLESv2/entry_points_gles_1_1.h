// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// entry_points_gles_1_1_autogen.h:
//   Defines the GLES 1.1 entry points.

#ifndef LIBGLESV2_ENTRYPOINTSGLES11_AUTOGEN_H_
#define LIBGLESV2_ENTRYPOINTSGLES11_AUTOGEN_H_

#include <GLES/gl.h>
#include <GLES/glext.h>

#include <export.h>

namespace gl
{
ANGLE_EXPORT void GL_APIENTRY ShadeModel(GLenum mode);
ANGLE_EXPORT void GL_APIENTRY MatrixMode(GLenum mode);
ANGLE_EXPORT void GL_APIENTRY LoadIdentity(void);
ANGLE_EXPORT void GL_APIENTRY LoadMatrixf(const GLfloat* m);
ANGLE_EXPORT void GL_APIENTRY PushMatrix(void);
ANGLE_EXPORT void GL_APIENTRY PopMatrix(void);
ANGLE_EXPORT void GL_APIENTRY MultMatrixf(const GLfloat* m);
ANGLE_EXPORT void GL_APIENTRY Orthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
ANGLE_EXPORT void GL_APIENTRY Frustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
ANGLE_EXPORT void GL_APIENTRY TexEnvf(GLenum target, GLenum pname, GLfloat param);
ANGLE_EXPORT void GL_APIENTRY TexEnvfv(GLenum target, GLenum pname, const GLfloat* params);
ANGLE_EXPORT void GL_APIENTRY TexEnvi(GLenum target, GLenum pname, GLint param);
ANGLE_EXPORT void GL_APIENTRY TexEnviv(GLenum target, GLenum pname, const GLint* params);
ANGLE_EXPORT void GL_APIENTRY GetTexEnvfv(GLenum env, GLenum pname, GLfloat* params);
ANGLE_EXPORT void GL_APIENTRY GetTexEnviv(GLenum env, GLenum pname, GLint* params);
ANGLE_EXPORT void GL_APIENTRY TexGenf(GLenum coord, GLenum pname, GLfloat param);
ANGLE_EXPORT void GL_APIENTRY TexGenfv(GLenum coord, GLenum pname, const GLfloat* params);
ANGLE_EXPORT void GL_APIENTRY TexGeni(GLenum coord, GLenum pname, GLint param);
ANGLE_EXPORT void GL_APIENTRY TexGeniv(GLenum coord, GLenum pname, const GLint* params);
ANGLE_EXPORT void GL_APIENTRY GetTexGeniv(GLenum coord, GLenum pname, GLint* params);
ANGLE_EXPORT void GL_APIENTRY GetTexGenfv(GLenum coord, GLenum pname, GLfloat* params);
ANGLE_EXPORT void GL_APIENTRY Materialf(GLenum face, GLenum pname, GLfloat param);
ANGLE_EXPORT void GL_APIENTRY Materialfv(GLenum face, GLenum pname, const GLfloat* params);
ANGLE_EXPORT void GL_APIENTRY GetMaterialfv(GLenum face, GLenum pname, GLfloat* params);
ANGLE_EXPORT void GL_APIENTRY LightModelf(GLenum pname, GLfloat param);
ANGLE_EXPORT void GL_APIENTRY LightModelfv(GLenum pname, const GLfloat* params);
ANGLE_EXPORT void GL_APIENTRY Lightf(GLenum light, GLenum pname, GLfloat param);
ANGLE_EXPORT void GL_APIENTRY Lightfv(GLenum light, GLenum pname, const GLfloat* params);
ANGLE_EXPORT void GL_APIENTRY GetLightfv(GLenum light, GLenum pname, GLfloat* params);
ANGLE_EXPORT void GL_APIENTRY MultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
ANGLE_EXPORT void GL_APIENTRY Normal3f(GLfloat nx, GLfloat ny, GLfloat nz);
ANGLE_EXPORT void GL_APIENTRY Fogf(GLenum pname, GLfloat param);
ANGLE_EXPORT void GL_APIENTRY Fogfv(GLenum pname, const GLfloat* params);
ANGLE_EXPORT void GL_APIENTRY EnableClientState(GLenum clientState);
ANGLE_EXPORT void GL_APIENTRY DisableClientState(GLenum clientState);
ANGLE_EXPORT void GL_APIENTRY DrawTexOES(float x, float y, float z, float width, float height);
ANGLE_EXPORT void GL_APIENTRY Rotatef(float angle, float x, float y, float z);
ANGLE_EXPORT void GL_APIENTRY Scalef(float x, float y, float z);
ANGLE_EXPORT void GL_APIENTRY Translatef(float x, float y, float z);
ANGLE_EXPORT void GL_APIENTRY Color4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
ANGLE_EXPORT void GL_APIENTRY ClientActiveTexture(GLenum texture);
ANGLE_EXPORT void GL_APIENTRY VertexPointer(GLint size, GLenum type, GLsizei stride, const void* ptr);
ANGLE_EXPORT void GL_APIENTRY NormalPointer(GLenum type, GLsizei stride, const void* ptr);
ANGLE_EXPORT void GL_APIENTRY ColorPointer(GLint size, GLenum type, GLsizei stride, const void* ptr);
ANGLE_EXPORT void GL_APIENTRY PointSizePointerOES(GLenum type, GLsizei stride, const void* ptr);
ANGLE_EXPORT void GL_APIENTRY TexCoordPointer(GLint size, GLenum type, GLsizei stride, const void* ptr);

} // namespace gl

#endif  // LIBGLESV2_ENTRYPOINTSGLES11_AUTOGEN_H_
