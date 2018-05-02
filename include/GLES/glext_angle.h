//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// glext_angle.h: ANGLE modifications to the glext.h header file.
//  Currently we don't include this file directly, we patch glext.h
//  to include it implicitly so it is visible throughout our code.

#ifndef INCLUDE_GLES_GLEXT_ANGLE_H_
#define INCLUDE_GLES_GLEXT_ANGLE_H_

// clang-format off
#ifndef EGL_ANGLE_explicit_context
#define EGL_ANGLE_explicit_context
typedef void *GLeglContext;
// OpenGL ES 1.0
typedef void (GL_APIENTRYP PFNGLALPHAFUNCCONTEXTANGLE)(GLeglContext ctx, GLenum func, GLfloat ref);
typedef void (GL_APIENTRYP PFNGLALPHAFUNCXCONTEXTANGLE)(GLeglContext ctx, GLenum func, GLfixed ref);
typedef void (GL_APIENTRYP PFNGLCLEARCOLORXCONTEXTANGLE)(GLeglContext ctx, GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
typedef void (GL_APIENTRYP PFNGLCLEARDEPTHXCONTEXTANGLE)(GLeglContext ctx, GLfixed depth);
typedef void (GL_APIENTRYP PFNGLCLIENTACTIVETEXTURECONTEXTANGLE)(GLeglContext ctx, GLenum texture);
typedef void (GL_APIENTRYP PFNGLCLIPPLANEFCONTEXTANGLE)(GLeglContext ctx, GLenum p, const GLfloat *eqn);
typedef void (GL_APIENTRYP PFNGLCLIPPLANEXCONTEXTANGLE)(GLeglContext ctx, GLenum plane, const GLfixed *equation);
typedef void (GL_APIENTRYP PFNGLCOLOR4FCONTEXTANGLE)(GLeglContext ctx, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (GL_APIENTRYP PFNGLCOLOR4UBCONTEXTANGLE)(GLeglContext ctx, GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
typedef void (GL_APIENTRYP PFNGLCOLOR4XCONTEXTANGLE)(GLeglContext ctx, GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
typedef void (GL_APIENTRYP PFNGLCOLORPOINTERCONTEXTANGLE)(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void (GL_APIENTRYP PFNGLDEPTHRANGEXCONTEXTANGLE)(GLeglContext ctx, GLfixed n, GLfixed f);
typedef void (GL_APIENTRYP PFNGLDISABLECLIENTSTATECONTEXTANGLE)(GLeglContext ctx, GLenum array);
typedef void (GL_APIENTRYP PFNGLENABLECLIENTSTATECONTEXTANGLE)(GLeglContext ctx, GLenum array);
typedef void (GL_APIENTRYP PFNGLFOGFCONTEXTANGLE)(GLeglContext ctx, GLenum pname, GLfloat param);
typedef void (GL_APIENTRYP PFNGLFOGFVCONTEXTANGLE)(GLeglContext ctx, GLenum pname, const GLfloat *params);
typedef void (GL_APIENTRYP PFNGLFOGXCONTEXTANGLE)(GLeglContext ctx, GLenum pname, GLfixed param);
typedef void (GL_APIENTRYP PFNGLFOGXVCONTEXTANGLE)(GLeglContext ctx, GLenum pname, const GLfixed *param);
typedef void (GL_APIENTRYP PFNGLFRUSTUMFCONTEXTANGLE)(GLeglContext ctx, GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f);
typedef void (GL_APIENTRYP PFNGLFRUSTUMXCONTEXTANGLE)(GLeglContext ctx, GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f);
typedef void (GL_APIENTRYP PFNGLGETCLIPPLANEFCONTEXTANGLE)(GLeglContext ctx, GLenum plane, GLfloat *equation);
typedef void (GL_APIENTRYP PFNGLGETCLIPPLANEXCONTEXTANGLE)(GLeglContext ctx, GLenum plane, GLfixed *equation);
typedef void (GL_APIENTRYP PFNGLGETFIXEDVCONTEXTANGLE)(GLeglContext ctx, GLenum pname, GLfixed *params);
typedef void (GL_APIENTRYP PFNGLGETLIGHTFVCONTEXTANGLE)(GLeglContext ctx, GLenum light, GLenum pname, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETLIGHTXVCONTEXTANGLE)(GLeglContext ctx, GLenum light, GLenum pname, GLfixed *params);
typedef void (GL_APIENTRYP PFNGLGETMATERIALFVCONTEXTANGLE)(GLeglContext ctx, GLenum face, GLenum pname, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETMATERIALXVCONTEXTANGLE)(GLeglContext ctx, GLenum face, GLenum pname, GLfixed *params);
typedef void (GL_APIENTRYP PFNGLGETPOINTERVCONTEXTANGLE)(GLeglContext ctx, GLenum pname, void **params);
typedef void (GL_APIENTRYP PFNGLGETTEXENVFVCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETTEXENVIVCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETTEXENVXVCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLfixed *params);
typedef void (GL_APIENTRYP PFNGLGETTEXPARAMETERXVCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLfixed *params);
typedef void (GL_APIENTRYP PFNGLLIGHTMODELFCONTEXTANGLE)(GLeglContext ctx, GLenum pname, GLfloat param);
typedef void (GL_APIENTRYP PFNGLLIGHTMODELFVCONTEXTANGLE)(GLeglContext ctx, GLenum pname, const GLfloat *params);
typedef void (GL_APIENTRYP PFNGLLIGHTMODELXCONTEXTANGLE)(GLeglContext ctx, GLenum pname, GLfixed param);
typedef void (GL_APIENTRYP PFNGLLIGHTMODELXVCONTEXTANGLE)(GLeglContext ctx, GLenum pname, const GLfixed *param);
typedef void (GL_APIENTRYP PFNGLLIGHTFCONTEXTANGLE)(GLeglContext ctx, GLenum light, GLenum pname, GLfloat param);
typedef void (GL_APIENTRYP PFNGLLIGHTFVCONTEXTANGLE)(GLeglContext ctx, GLenum light, GLenum pname, const GLfloat *params);
typedef void (GL_APIENTRYP PFNGLLIGHTXCONTEXTANGLE)(GLeglContext ctx, GLenum light, GLenum pname, GLfixed param);
typedef void (GL_APIENTRYP PFNGLLIGHTXVCONTEXTANGLE)(GLeglContext ctx, GLenum light, GLenum pname, const GLfixed *params);
typedef void (GL_APIENTRYP PFNGLLINEWIDTHXCONTEXTANGLE)(GLeglContext ctx, GLfixed width);
typedef void (GL_APIENTRYP PFNGLLOADIDENTITYCONTEXTANGLE)(GLeglContext ctx);
typedef void (GL_APIENTRYP PFNGLLOADMATRIXFCONTEXTANGLE)(GLeglContext ctx, const GLfloat *m);
typedef void (GL_APIENTRYP PFNGLLOADMATRIXXCONTEXTANGLE)(GLeglContext ctx, const GLfixed *m);
typedef void (GL_APIENTRYP PFNGLLOGICOPCONTEXTANGLE)(GLeglContext ctx, GLenum opcode);
typedef void (GL_APIENTRYP PFNGLMATERIALFCONTEXTANGLE)(GLeglContext ctx, GLenum face, GLenum pname, GLfloat param);
typedef void (GL_APIENTRYP PFNGLMATERIALFVCONTEXTANGLE)(GLeglContext ctx, GLenum face, GLenum pname, const GLfloat *params);
typedef void (GL_APIENTRYP PFNGLMATERIALXCONTEXTANGLE)(GLeglContext ctx, GLenum face, GLenum pname, GLfixed param);
typedef void (GL_APIENTRYP PFNGLMATERIALXVCONTEXTANGLE)(GLeglContext ctx, GLenum face, GLenum pname, const GLfixed *param);
typedef void (GL_APIENTRYP PFNGLMATRIXMODECONTEXTANGLE)(GLeglContext ctx, GLenum mode);
typedef void (GL_APIENTRYP PFNGLMULTMATRIXFCONTEXTANGLE)(GLeglContext ctx, const GLfloat *m);
typedef void (GL_APIENTRYP PFNGLMULTMATRIXXCONTEXTANGLE)(GLeglContext ctx, const GLfixed *m);
typedef void (GL_APIENTRYP PFNGLMULTITEXCOORD4FCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (GL_APIENTRYP PFNGLMULTITEXCOORD4XCONTEXTANGLE)(GLeglContext ctx, GLenum texture, GLfixed s, GLfixed t, GLfixed r, GLfixed q);
typedef void (GL_APIENTRYP PFNGLNORMAL3FCONTEXTANGLE)(GLeglContext ctx, GLfloat nx, GLfloat ny, GLfloat nz);
typedef void (GL_APIENTRYP PFNGLNORMAL3XCONTEXTANGLE)(GLeglContext ctx, GLfixed nx, GLfixed ny, GLfixed nz);
typedef void (GL_APIENTRYP PFNGLNORMALPOINTERCONTEXTANGLE)(GLeglContext ctx, GLenum type, GLsizei stride, const void *pointer);
typedef void (GL_APIENTRYP PFNGLORTHOFCONTEXTANGLE)(GLeglContext ctx, GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f);
typedef void (GL_APIENTRYP PFNGLORTHOXCONTEXTANGLE)(GLeglContext ctx, GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f);
typedef void (GL_APIENTRYP PFNGLPOINTPARAMETERFCONTEXTANGLE)(GLeglContext ctx, GLenum pname, GLfloat param);
typedef void (GL_APIENTRYP PFNGLPOINTPARAMETERFVCONTEXTANGLE)(GLeglContext ctx, GLenum pname, const GLfloat *params);
typedef void (GL_APIENTRYP PFNGLPOINTPARAMETERXCONTEXTANGLE)(GLeglContext ctx, GLenum pname, GLfixed param);
typedef void (GL_APIENTRYP PFNGLPOINTPARAMETERXVCONTEXTANGLE)(GLeglContext ctx, GLenum pname, const GLfixed *params);
typedef void (GL_APIENTRYP PFNGLPOINTSIZECONTEXTANGLE)(GLeglContext ctx, GLfloat size);
typedef void (GL_APIENTRYP PFNGLPOINTSIZEXCONTEXTANGLE)(GLeglContext ctx, GLfixed size);
typedef void (GL_APIENTRYP PFNGLPOLYGONOFFSETXCONTEXTANGLE)(GLeglContext ctx, GLfixed factor, GLfixed units);
typedef void (GL_APIENTRYP PFNGLPOPMATRIXCONTEXTANGLE)(GLeglContext ctx);
typedef void (GL_APIENTRYP PFNGLPUSHMATRIXCONTEXTANGLE)(GLeglContext ctx);
typedef void (GL_APIENTRYP PFNGLROTATEFCONTEXTANGLE)(GLeglContext ctx, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
typedef void (GL_APIENTRYP PFNGLROTATEXCONTEXTANGLE)(GLeglContext ctx, GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
typedef void (GL_APIENTRYP PFNGLSAMPLECOVERAGEXCONTEXTANGLE)(GLeglContext ctx, GLclampx value, GLboolean invert);
typedef void (GL_APIENTRYP PFNGLSCALEFCONTEXTANGLE)(GLeglContext ctx, GLfloat x, GLfloat y, GLfloat z);
typedef void (GL_APIENTRYP PFNGLSCALEXCONTEXTANGLE)(GLeglContext ctx, GLfixed x, GLfixed y, GLfixed z);
typedef void (GL_APIENTRYP PFNGLSHADEMODELCONTEXTANGLE)(GLeglContext ctx, GLenum mode);
typedef void (GL_APIENTRYP PFNGLTEXCOORDPOINTERCONTEXTANGLE)(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void (GL_APIENTRYP PFNGLTEXENVFCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLfloat param);
typedef void (GL_APIENTRYP PFNGLTEXENVFVCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, const GLfloat *params);
typedef void (GL_APIENTRYP PFNGLTEXENVICONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLint param);
typedef void (GL_APIENTRYP PFNGLTEXENVIVCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, const GLint *params);
typedef void (GL_APIENTRYP PFNGLTEXENVXCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLfixed param);
typedef void (GL_APIENTRYP PFNGLTEXENVXVCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, const GLfixed *params);
typedef void (GL_APIENTRYP PFNGLTEXPARAMETERXCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLfixed param);
typedef void (GL_APIENTRYP PFNGLTEXPARAMETERXVCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, const GLfixed *params);
typedef void (GL_APIENTRYP PFNGLTRANSLATEFCONTEXTANGLE)(GLeglContext ctx, GLfloat x, GLfloat y, GLfloat z);
typedef void (GL_APIENTRYP PFNGLTRANSLATEXCONTEXTANGLE)(GLeglContext ctx, GLfixed x, GLfixed y, GLfixed z);
typedef void (GL_APIENTRYP PFNGLVERTEXPOINTERCONTEXTANGLE)(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
// GL_OES_draw_texture
typedef void (GL_APIENTRYP PFNGLDRAWTEXFOESCONTEXTANGLE)(GLeglContext ctx, GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height);
typedef void (GL_APIENTRYP PFNGLDRAWTEXFVOESCONTEXTANGLE)(GLeglContext ctx, const GLfloat *coords);
typedef void (GL_APIENTRYP PFNGLDRAWTEXIOESCONTEXTANGLE)(GLeglContext ctx, GLint x, GLint y, GLint z, GLint width, GLint height);
typedef void (GL_APIENTRYP PFNGLDRAWTEXIVOESCONTEXTANGLE)(GLeglContext ctx, const GLint *coords);
typedef void (GL_APIENTRYP PFNGLDRAWTEXSOESCONTEXTANGLE)(GLeglContext ctx, GLshort x, GLshort y, GLshort z, GLshort width, GLshort height);
typedef void (GL_APIENTRYP PFNGLDRAWTEXSVOESCONTEXTANGLE)(GLeglContext ctx, const GLshort *coords);
typedef void (GL_APIENTRYP PFNGLDRAWTEXXOESCONTEXTANGLE)(GLeglContext ctx, GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height);
typedef void (GL_APIENTRYP PFNGLDRAWTEXXVOESCONTEXTANGLE)(GLeglContext ctx, const GLfixed *coords);
// GL_OES_framebuffer_object
typedef void (GL_APIENTRYP PFNGLBINDFRAMEBUFFEROESCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLuint framebuffer);
typedef void (GL_APIENTRYP PFNGLBINDRENDERBUFFEROESCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLuint renderbuffer);
typedef void (GL_APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSOESCONTEXTANGLE)(GLeglContext ctx, GLenum target);
typedef void (GL_APIENTRYP PFNGLDELETEFRAMEBUFFERSOESCONTEXTANGLE)(GLeglContext ctx, GLsizei n, const GLuint *framebuffers);
typedef void (GL_APIENTRYP PFNGLDELETERENDERBUFFERSOESCONTEXTANGLE)(GLeglContext ctx, GLsizei n, const GLuint *renderbuffers);
typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFEROESCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DOESCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (GL_APIENTRYP PFNGLGENFRAMEBUFFERSOESCONTEXTANGLE)(GLeglContext ctx, GLsizei n, GLuint *framebuffers);
typedef void (GL_APIENTRYP PFNGLGENRENDERBUFFERSOESCONTEXTANGLE)(GLeglContext ctx, GLsizei n, GLuint *renderbuffers);
typedef void (GL_APIENTRYP PFNGLGENERATEMIPMAPOESCONTEXTANGLE)(GLeglContext ctx, GLenum target);
typedef void (GL_APIENTRYP PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOESCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum attachment, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETRENDERBUFFERPARAMETERIVOESCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLISFRAMEBUFFEROESCONTEXTANGLE)(GLeglContext ctx, GLuint framebuffer);
typedef void (GL_APIENTRYP PFNGLISRENDERBUFFEROESCONTEXTANGLE)(GLeglContext ctx, GLuint renderbuffer);
typedef void (GL_APIENTRYP PFNGLRENDERBUFFERSTORAGEOESCONTEXTANGLE)(GLeglContext ctx, GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
// GL_OES_matrix_palette
typedef void (GL_APIENTRYP PFNGLCURRENTPALETTEMATRIXOESCONTEXTANGLE)(GLeglContext ctx, GLuint matrixpaletteindex);
typedef void (GL_APIENTRYP PFNGLLOADPALETTEFROMMODELVIEWMATRIXOESCONTEXTANGLE)(GLeglContext ctx);
typedef void (GL_APIENTRYP PFNGLMATRIXINDEXPOINTEROESCONTEXTANGLE)(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void (GL_APIENTRYP PFNGLWEIGHTPOINTEROESCONTEXTANGLE)(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
// GL_OES_point_size_array
typedef void (GL_APIENTRYP PFNGLPOINTSIZEPOINTEROESCONTEXTANGLE)(GLeglContext ctx, GLenum type, GLsizei stride, const void *pointer);
// GL_OES_query_matrix
typedef void (GL_APIENTRYP PFNGLQUERYMATRIXXOESCONTEXTANGLE)(GLeglContext ctx, GLfixed *mantissa, GLint *exponent);
// GL_OES_texture_cube_map
typedef void (GL_APIENTRYP PFNGLGETTEXGENFVOESCONTEXTANGLE)(GLeglContext ctx, GLenum coord, GLenum pname, GLfloat *params);
typedef void (GL_APIENTRYP PFNGLGETTEXGENIVOESCONTEXTANGLE)(GLeglContext ctx, GLenum coord, GLenum pname, GLint *params);
typedef void (GL_APIENTRYP PFNGLGETTEXGENXVOESCONTEXTANGLE)(GLeglContext ctx, GLenum coord, GLenum pname, GLfixed *params);
typedef void (GL_APIENTRYP PFNGLTEXGENFOESCONTEXTANGLE)(GLeglContext ctx, GLenum coord, GLenum pname, GLfloat param);
typedef void (GL_APIENTRYP PFNGLTEXGENFVOESCONTEXTANGLE)(GLeglContext ctx, GLenum coord, GLenum pname, const GLfloat *params);
typedef void (GL_APIENTRYP PFNGLTEXGENIOESCONTEXTANGLE)(GLeglContext ctx, GLenum coord, GLenum pname, GLint param);
typedef void (GL_APIENTRYP PFNGLTEXGENIVOESCONTEXTANGLE)(GLeglContext ctx, GLenum coord, GLenum pname, const GLint *params);
typedef void (GL_APIENTRYP PFNGLTEXGENXOESCONTEXTANGLE)(GLeglContext ctx, GLenum coord, GLenum pname, GLfixed param);
typedef void (GL_APIENTRYP PFNGLTEXGENXVOESCONTEXTANGLE)(GLeglContext ctx, GLenum coord, GLenum pname, const GLfixed *params);
#ifdef GL_GLEXT_PROTOTYPES
GL_API void glAlphaFuncContextANGLE(GLeglContext ctx, GLenum func, GLfloat ref);
GL_API void glAlphaFuncxContextANGLE(GLeglContext ctx, GLenum func, GLfixed ref);
GL_API void glClearColorxContextANGLE(GLeglContext ctx, GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
GL_API void glClearDepthxContextANGLE(GLeglContext ctx, GLfixed depth);
GL_API void glClientActiveTextureContextANGLE(GLeglContext ctx, GLenum texture);
GL_API void glClipPlanefContextANGLE(GLeglContext ctx, GLenum p, const GLfloat *eqn);
GL_API void glClipPlanexContextANGLE(GLeglContext ctx, GLenum plane, const GLfixed *equation);
GL_API void glColor4fContextANGLE(GLeglContext ctx, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GL_API void glColor4ubContextANGLE(GLeglContext ctx, GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
GL_API void glColor4xContextANGLE(GLeglContext ctx, GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
GL_API void glColorPointerContextANGLE(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
GL_API void glDepthRangexContextANGLE(GLeglContext ctx, GLfixed n, GLfixed f);
GL_API void glDisableClientStateContextANGLE(GLeglContext ctx, GLenum array);
GL_API void glEnableClientStateContextANGLE(GLeglContext ctx, GLenum array);
GL_API void glFogfContextANGLE(GLeglContext ctx, GLenum pname, GLfloat param);
GL_API void glFogfvContextANGLE(GLeglContext ctx, GLenum pname, const GLfloat *params);
GL_API void glFogxContextANGLE(GLeglContext ctx, GLenum pname, GLfixed param);
GL_API void glFogxvContextANGLE(GLeglContext ctx, GLenum pname, const GLfixed *param);
GL_API void glFrustumfContextANGLE(GLeglContext ctx, GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f);
GL_API void glFrustumxContextANGLE(GLeglContext ctx, GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f);
GL_API void glGetClipPlanefContextANGLE(GLeglContext ctx, GLenum plane, GLfloat *equation);
GL_API void glGetClipPlanexContextANGLE(GLeglContext ctx, GLenum plane, GLfixed *equation);
GL_API void glGetFixedvContextANGLE(GLeglContext ctx, GLenum pname, GLfixed *params);
GL_API void glGetLightfvContextANGLE(GLeglContext ctx, GLenum light, GLenum pname, GLfloat *params);
GL_API void glGetLightxvContextANGLE(GLeglContext ctx, GLenum light, GLenum pname, GLfixed *params);
GL_API void glGetMaterialfvContextANGLE(GLeglContext ctx, GLenum face, GLenum pname, GLfloat *params);
GL_API void glGetMaterialxvContextANGLE(GLeglContext ctx, GLenum face, GLenum pname, GLfixed *params);
GL_API void glGetPointervContextANGLE(GLeglContext ctx, GLenum pname, void **params);
GL_API void glGetTexEnvfvContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfloat *params);
GL_API void glGetTexEnvivContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
GL_API void glGetTexEnvxvContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfixed *params);
GL_API void glGetTexParameterxvContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfixed *params);
GL_API void glLightModelfContextANGLE(GLeglContext ctx, GLenum pname, GLfloat param);
GL_API void glLightModelfvContextANGLE(GLeglContext ctx, GLenum pname, const GLfloat *params);
GL_API void glLightModelxContextANGLE(GLeglContext ctx, GLenum pname, GLfixed param);
GL_API void glLightModelxvContextANGLE(GLeglContext ctx, GLenum pname, const GLfixed *param);
GL_API void glLightfContextANGLE(GLeglContext ctx, GLenum light, GLenum pname, GLfloat param);
GL_API void glLightfvContextANGLE(GLeglContext ctx, GLenum light, GLenum pname, const GLfloat *params);
GL_API void glLightxContextANGLE(GLeglContext ctx, GLenum light, GLenum pname, GLfixed param);
GL_API void glLightxvContextANGLE(GLeglContext ctx, GLenum light, GLenum pname, const GLfixed *params);
GL_API void glLineWidthxContextANGLE(GLeglContext ctx, GLfixed width);
GL_API void glLoadIdentityContextANGLE(GLeglContext ctx);
GL_API void glLoadMatrixfContextANGLE(GLeglContext ctx, const GLfloat *m);
GL_API void glLoadMatrixxContextANGLE(GLeglContext ctx, const GLfixed *m);
GL_API void glLogicOpContextANGLE(GLeglContext ctx, GLenum opcode);
GL_API void glMaterialfContextANGLE(GLeglContext ctx, GLenum face, GLenum pname, GLfloat param);
GL_API void glMaterialfvContextANGLE(GLeglContext ctx, GLenum face, GLenum pname, const GLfloat *params);
GL_API void glMaterialxContextANGLE(GLeglContext ctx, GLenum face, GLenum pname, GLfixed param);
GL_API void glMaterialxvContextANGLE(GLeglContext ctx, GLenum face, GLenum pname, const GLfixed *param);
GL_API void glMatrixModeContextANGLE(GLeglContext ctx, GLenum mode);
GL_API void glMultMatrixfContextANGLE(GLeglContext ctx, const GLfloat *m);
GL_API void glMultMatrixxContextANGLE(GLeglContext ctx, const GLfixed *m);
GL_API void glMultiTexCoord4fContextANGLE(GLeglContext ctx, GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
GL_API void glMultiTexCoord4xContextANGLE(GLeglContext ctx, GLenum texture, GLfixed s, GLfixed t, GLfixed r, GLfixed q);
GL_API void glNormal3fContextANGLE(GLeglContext ctx, GLfloat nx, GLfloat ny, GLfloat nz);
GL_API void glNormal3xContextANGLE(GLeglContext ctx, GLfixed nx, GLfixed ny, GLfixed nz);
GL_API void glNormalPointerContextANGLE(GLeglContext ctx, GLenum type, GLsizei stride, const void *pointer);
GL_API void glOrthofContextANGLE(GLeglContext ctx, GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f);
GL_API void glOrthoxContextANGLE(GLeglContext ctx, GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f);
GL_API void glPointParameterfContextANGLE(GLeglContext ctx, GLenum pname, GLfloat param);
GL_API void glPointParameterfvContextANGLE(GLeglContext ctx, GLenum pname, const GLfloat *params);
GL_API void glPointParameterxContextANGLE(GLeglContext ctx, GLenum pname, GLfixed param);
GL_API void glPointParameterxvContextANGLE(GLeglContext ctx, GLenum pname, const GLfixed *params);
GL_API void glPointSizeContextANGLE(GLeglContext ctx, GLfloat size);
GL_API void glPointSizexContextANGLE(GLeglContext ctx, GLfixed size);
GL_API void glPolygonOffsetxContextANGLE(GLeglContext ctx, GLfixed factor, GLfixed units);
GL_API void glPopMatrixContextANGLE(GLeglContext ctx);
GL_API void glPushMatrixContextANGLE(GLeglContext ctx);
GL_API void glRotatefContextANGLE(GLeglContext ctx, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
GL_API void glRotatexContextANGLE(GLeglContext ctx, GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
GL_API void glSampleCoveragexContextANGLE(GLeglContext ctx, GLclampx value, GLboolean invert);
GL_API void glScalefContextANGLE(GLeglContext ctx, GLfloat x, GLfloat y, GLfloat z);
GL_API void glScalexContextANGLE(GLeglContext ctx, GLfixed x, GLfixed y, GLfixed z);
GL_API void glShadeModelContextANGLE(GLeglContext ctx, GLenum mode);
GL_API void glTexCoordPointerContextANGLE(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
GL_API void glTexEnvfContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfloat param);
GL_API void glTexEnvfvContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, const GLfloat *params);
GL_API void glTexEnviContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint param);
GL_API void glTexEnvivContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, const GLint *params);
GL_API void glTexEnvxContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfixed param);
GL_API void glTexEnvxvContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, const GLfixed *params);
GL_API void glTexParameterxContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLfixed param);
GL_API void glTexParameterxvContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, const GLfixed *params);
GL_API void glTranslatefContextANGLE(GLeglContext ctx, GLfloat x, GLfloat y, GLfloat z);
GL_API void glTranslatexContextANGLE(GLeglContext ctx, GLfixed x, GLfixed y, GLfixed z);
GL_API void glVertexPointerContextANGLE(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
// GL_OES_draw_texture
GL_API void glDrawTexfOESContextANGLE(GLeglContext ctx, GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height);
GL_API void glDrawTexfvOESContextANGLE(GLeglContext ctx, const GLfloat *coords);
GL_API void glDrawTexiOESContextANGLE(GLeglContext ctx, GLint x, GLint y, GLint z, GLint width, GLint height);
GL_API void glDrawTexivOESContextANGLE(GLeglContext ctx, const GLint *coords);
GL_API void glDrawTexsOESContextANGLE(GLeglContext ctx, GLshort x, GLshort y, GLshort z, GLshort width, GLshort height);
GL_API void glDrawTexsvOESContextANGLE(GLeglContext ctx, const GLshort *coords);
GL_API void glDrawTexxOESContextANGLE(GLeglContext ctx, GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height);
GL_API void glDrawTexxvOESContextANGLE(GLeglContext ctx, const GLfixed *coords);
// GL_OES_framebuffer_object
GL_API void glBindFramebufferOESContextANGLE(GLeglContext ctx, GLenum target, GLuint framebuffer);
GL_API void glBindRenderbufferOESContextANGLE(GLeglContext ctx, GLenum target, GLuint renderbuffer);
GL_API GLenum glCheckFramebufferStatusOESContextANGLE(GLeglContext ctx, GLenum target);
GL_API void glDeleteFramebuffersOESContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *framebuffers);
GL_API void glDeleteRenderbuffersOESContextANGLE(GLeglContext ctx, GLsizei n, const GLuint *renderbuffers);
GL_API void glFramebufferRenderbufferOESContextANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
GL_API void glFramebufferTexture2DOESContextANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GL_API void glGenFramebuffersOESContextANGLE(GLeglContext ctx, GLsizei n, GLuint *framebuffers);
GL_API void glGenRenderbuffersOESContextANGLE(GLeglContext ctx, GLsizei n, GLuint *renderbuffers);
GL_API void glGenerateMipmapOESContextANGLE(GLeglContext ctx, GLenum target);
GL_API void glGetFramebufferAttachmentParameterivOESContextANGLE(GLeglContext ctx, GLenum target, GLenum attachment, GLenum pname, GLint *params);
GL_API void glGetRenderbufferParameterivOESContextANGLE(GLeglContext ctx, GLenum target, GLenum pname, GLint *params);
GL_API GLboolean glIsFramebufferOESContextANGLE(GLeglContext ctx, GLuint framebuffer);
GL_API GLboolean glIsRenderbufferOESContextANGLE(GLeglContext ctx, GLuint renderbuffer);
GL_API void glRenderbufferStorageOESContextANGLE(GLeglContext ctx, GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
// GL_OES_matrix_palette
GL_API void glCurrentPaletteMatrixOESContextANGLE(GLeglContext ctx, GLuint matrixpaletteindex);
GL_API void glLoadPaletteFromModelViewMatrixOESContextANGLE(GLeglContext ctx);
GL_API void glMatrixIndexPointerOESContextANGLE(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
GL_API void glWeightPointerOESContextANGLE(GLeglContext ctx, GLint size, GLenum type, GLsizei stride, const void *pointer);
// GL_OES_point_size_array
GL_API void glPointSizePointerOESContextANGLE(GLeglContext ctx, GLenum type, GLsizei stride, const void *pointer);
// GL_OES_query_matrix
GL_API GLbitfield glQueryMatrixxOESContextANGLE(GLeglContext ctx, GLfixed *mantissa, GLint *exponent);
// GL_OES_texture_cube_map
GL_API void glGetTexGenfvOESContextANGLE(GLeglContext ctx, GLenum coord, GLenum pname, GLfloat *params);
GL_API void glGetTexGenivOESContextANGLE(GLeglContext ctx, GLenum coord, GLenum pname, GLint *params);
GL_API void glGetTexGenxvOESContextANGLE(GLeglContext ctx, GLenum coord, GLenum pname, GLfixed *params);
GL_API void glTexGenfOESContextANGLE(GLeglContext ctx, GLenum coord, GLenum pname, GLfloat param);
GL_API void glTexGenfvOESContextANGLE(GLeglContext ctx, GLenum coord, GLenum pname, const GLfloat *params);
GL_API void glTexGeniOESContextANGLE(GLeglContext ctx, GLenum coord, GLenum pname, GLint param);
GL_API void glTexGenivOESContextANGLE(GLeglContext ctx, GLenum coord, GLenum pname, const GLint *params);
GL_API void glTexGenxOESContextANGLE(GLeglContext ctx, GLenum coord, GLenum pname, GLfixed param);
GL_API void glTexGenxvOESContextANGLE(GLeglContext ctx, GLenum coord, GLenum pname, const GLfixed *params);
#endif /* GL_GLEXT_PROTOTYPES */
#endif /* EGL_ANGLE_explicit_context */

// clang-format on

#endif  // INCLUDE_GLES_GLEXT_ANGLE_H_
