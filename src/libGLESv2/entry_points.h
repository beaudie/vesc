//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points.h : Defines the egl and gl entry points.

#include "common/platform.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#if defined(_WIN32)
#   if defined(LIBGLESV2_IMPLEMENTATION)
#       define ANGLE_EXPORT __declspec(dllexport)
#   else
#       define ANGLE_EXPORT __declspec(dllimport)
#   endif
#elif defined(__GNUC__)
#   if defined(LIBGLESV2_IMPLEMENTATION)
#       define ANGLE_EXPORT __attribute__((visibility ("default")))
#   else
#       define ANGLE_EXPORT
#   endif
#else
#   define ANGLE_EXPORT
#endif

namespace egl
{

// EGL 1.0
ANGLE_EXPORT EGLBoolean ChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
ANGLE_EXPORT EGLBoolean CopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target);
ANGLE_EXPORT EGLContext CreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
ANGLE_EXPORT EGLSurface CreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
ANGLE_EXPORT EGLSurface CreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list);
ANGLE_EXPORT EGLSurface CreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list);
ANGLE_EXPORT EGLBoolean DestroyContext(EGLDisplay dpy, EGLContext ctx);
ANGLE_EXPORT EGLBoolean DestroySurface(EGLDisplay dpy, EGLSurface surface);
ANGLE_EXPORT EGLBoolean GetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
ANGLE_EXPORT EGLBoolean GetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
ANGLE_EXPORT EGLDisplay GetCurrentDisplay(void);
ANGLE_EXPORT EGLSurface GetCurrentSurface(EGLint readdraw);
ANGLE_EXPORT EGLDisplay GetDisplay(EGLNativeDisplayType display_id);
ANGLE_EXPORT EGLint GetError(void);
ANGLE_EXPORT EGLBoolean Initialize(EGLDisplay dpy, EGLint *major, EGLint *minor);
ANGLE_EXPORT EGLBoolean MakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
ANGLE_EXPORT EGLBoolean QueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);
ANGLE_EXPORT const char *QueryString(EGLDisplay dpy, EGLint name);
ANGLE_EXPORT EGLBoolean QuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);
ANGLE_EXPORT EGLBoolean SwapBuffers(EGLDisplay dpy, EGLSurface surface);
ANGLE_EXPORT EGLBoolean Terminate(EGLDisplay dpy);
ANGLE_EXPORT EGLBoolean WaitGL(void);
ANGLE_EXPORT EGLBoolean WaitNative(EGLint engine);

// EGL 1.1
ANGLE_EXPORT EGLBoolean BindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
ANGLE_EXPORT EGLBoolean ReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
ANGLE_EXPORT EGLBoolean SurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value);
ANGLE_EXPORT EGLBoolean SwapInterval(EGLDisplay dpy, EGLint interval);

// EGL 1.2
ANGLE_EXPORT EGLBoolean BindAPI(EGLenum api);
ANGLE_EXPORT EGLenum QueryAPI(void);
ANGLE_EXPORT EGLSurface CreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list);
ANGLE_EXPORT EGLBoolean ReleaseThread(void);
ANGLE_EXPORT EGLBoolean WaitClient(void);

// EGL 1.4
ANGLE_EXPORT EGLContext GetCurrentContext(void);

// EGL 1.5
ANGLE_EXPORT EGLSync CreateSync(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLBoolean DestroySync(EGLDisplay dpy, EGLSync sync);
ANGLE_EXPORT EGLint ClientWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTime timeout);
ANGLE_EXPORT EGLBoolean GetSyncAttrib(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib *value);
ANGLE_EXPORT EGLDisplay GetPlatformDisplay(EGLenum platform, void *native_display, const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLSurface CreatePlatformWindowSurface(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLSurface CreatePlatformPixmapSurface(EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLBoolean WaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags);

// EGL_ANGLE_query_surface_pointer
ANGLE_EXPORT EGLBoolean QuerySurfacePointerANGLE(EGLDisplay dpy, EGLSurface surface, EGLint attribute, void **value);

// EGL_NV_post_sub_buffer
ANGLE_EXPORT EGLBoolean PostSubBufferNV(EGLDisplay dpy, EGLSurface surface, EGLint x, EGLint y, EGLint width, EGLint height);

// EGL_EXT_platform_base
ANGLE_EXPORT EGLDisplay GetPlatformDisplayEXT(EGLenum platform, void *native_display, const EGLint *attrib_list);

}

namespace gl
{

// OpenGL ES 2.0
ANGLE_EXPORT void ActiveTexture(GLenum texture);
ANGLE_EXPORT void AttachShader(GLuint program, GLuint shader);
ANGLE_EXPORT void BindAttribLocation(GLuint program, GLuint index, const GLchar* name);
ANGLE_EXPORT void BindBuffer(GLenum target, GLuint buffer);
ANGLE_EXPORT void BindFramebuffer(GLenum target, GLuint framebuffer);
ANGLE_EXPORT void BindRenderbuffer(GLenum target, GLuint renderbuffer);
ANGLE_EXPORT void BindTexture(GLenum target, GLuint texture);
ANGLE_EXPORT void BlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
ANGLE_EXPORT void BlendEquation(GLenum mode);
ANGLE_EXPORT void BlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha);
ANGLE_EXPORT void BlendFunc(GLenum sfactor, GLenum dfactor);
ANGLE_EXPORT void BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
ANGLE_EXPORT void BufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
ANGLE_EXPORT void BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
ANGLE_EXPORT GLenum CheckFramebufferStatus(GLenum target);
ANGLE_EXPORT void Clear(GLbitfield mask);
ANGLE_EXPORT void ClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
ANGLE_EXPORT void ClearDepthf(GLfloat depth);
ANGLE_EXPORT void ClearStencil(GLint s);
ANGLE_EXPORT void ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
ANGLE_EXPORT void CompileShader(GLuint shader);
ANGLE_EXPORT void CompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data);
ANGLE_EXPORT void CompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data);
ANGLE_EXPORT void CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
ANGLE_EXPORT void CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
ANGLE_EXPORT GLuint CreateProgram(void);
ANGLE_EXPORT GLuint CreateShader(GLenum type);
ANGLE_EXPORT void CullFace(GLenum mode);
ANGLE_EXPORT void DeleteBuffers(GLsizei n, const GLuint* buffers);
ANGLE_EXPORT void DeleteFramebuffers(GLsizei n, const GLuint* framebuffers);
ANGLE_EXPORT void DeleteProgram(GLuint program);
ANGLE_EXPORT void DeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers);
ANGLE_EXPORT void DeleteShader(GLuint shader);
ANGLE_EXPORT void DeleteTextures(GLsizei n, const GLuint* textures);
ANGLE_EXPORT void DepthFunc(GLenum func);
ANGLE_EXPORT void DepthMask(GLboolean flag);
ANGLE_EXPORT void DepthRangef(GLfloat n, GLfloat f);
ANGLE_EXPORT void DetachShader(GLuint program, GLuint shader);
ANGLE_EXPORT void Disable(GLenum cap);
ANGLE_EXPORT void DisableVertexAttribArray(GLuint index);
ANGLE_EXPORT void DrawArrays(GLenum mode, GLint first, GLsizei count);
ANGLE_EXPORT void DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
ANGLE_EXPORT void Enable(GLenum cap);
ANGLE_EXPORT void EnableVertexAttribArray(GLuint index);
ANGLE_EXPORT void Finish(void);
ANGLE_EXPORT void Flush(void);
ANGLE_EXPORT void FramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
ANGLE_EXPORT void FramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
ANGLE_EXPORT void FrontFace(GLenum mode);
ANGLE_EXPORT void GenBuffers(GLsizei n, GLuint* buffers);
ANGLE_EXPORT void GenerateMipmap(GLenum target);
ANGLE_EXPORT void GenFramebuffers(GLsizei n, GLuint* framebuffers);
ANGLE_EXPORT void GenRenderbuffers(GLsizei n, GLuint* renderbuffers);
ANGLE_EXPORT void GenTextures(GLsizei n, GLuint* textures);
ANGLE_EXPORT void GetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
ANGLE_EXPORT void GetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
ANGLE_EXPORT void GetAttachedShaders(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders);
ANGLE_EXPORT GLint GetAttribLocation(GLuint program, const GLchar* name);
ANGLE_EXPORT void GetBooleanv(GLenum pname, GLboolean* params);
ANGLE_EXPORT void GetBufferParameteriv(GLenum target, GLenum pname, GLint* params);
ANGLE_EXPORT GLenum GetError(void);
ANGLE_EXPORT void GetFloatv(GLenum pname, GLfloat* params);
ANGLE_EXPORT void GetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params);
ANGLE_EXPORT void GetIntegerv(GLenum pname, GLint* params);
ANGLE_EXPORT void GetProgramiv(GLuint program, GLenum pname, GLint* params);
ANGLE_EXPORT void GetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog);
ANGLE_EXPORT void GetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params);
ANGLE_EXPORT void GetShaderiv(GLuint shader, GLenum pname, GLint* params);
ANGLE_EXPORT void GetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog);
ANGLE_EXPORT void GetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);
ANGLE_EXPORT void GetShaderSource(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source);
ANGLE_EXPORT const GLubyte* GetString(GLenum name);
ANGLE_EXPORT void GetTexParameterfv(GLenum target, GLenum pname, GLfloat* params);
ANGLE_EXPORT void GetTexParameteriv(GLenum target, GLenum pname, GLint* params);
ANGLE_EXPORT void GetUniformfv(GLuint program, GLint location, GLfloat* params);
ANGLE_EXPORT void GetUniformiv(GLuint program, GLint location, GLint* params);
ANGLE_EXPORT GLint GetUniformLocation(GLuint program, const GLchar* name);
ANGLE_EXPORT void GetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params);
ANGLE_EXPORT void GetVertexAttribiv(GLuint index, GLenum pname, GLint* params);
ANGLE_EXPORT void GetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid** pointer);
ANGLE_EXPORT void Hint(GLenum target, GLenum mode);
ANGLE_EXPORT GLboolean IsBuffer(GLuint buffer);
ANGLE_EXPORT GLboolean IsEnabled(GLenum cap);
ANGLE_EXPORT GLboolean IsFramebuffer(GLuint framebuffer);
ANGLE_EXPORT GLboolean IsProgram(GLuint program);
ANGLE_EXPORT GLboolean IsRenderbuffer(GLuint renderbuffer);
ANGLE_EXPORT GLboolean IsShader(GLuint shader);
ANGLE_EXPORT GLboolean IsTexture(GLuint texture);
ANGLE_EXPORT void LineWidth(GLfloat width);
ANGLE_EXPORT void LinkProgram(GLuint program);
ANGLE_EXPORT void PixelStorei(GLenum pname, GLint param);
ANGLE_EXPORT void PolygonOffset(GLfloat factor, GLfloat units);
ANGLE_EXPORT void ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
ANGLE_EXPORT void ReleaseShaderCompiler(void);
ANGLE_EXPORT void RenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
ANGLE_EXPORT void SampleCoverage(GLfloat value, GLboolean invert);
ANGLE_EXPORT void Scissor(GLint x, GLint y, GLsizei width, GLsizei height);
ANGLE_EXPORT void ShaderBinary(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length);
ANGLE_EXPORT void ShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
ANGLE_EXPORT void StencilFunc(GLenum func, GLint ref, GLuint mask);
ANGLE_EXPORT void StencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask);
ANGLE_EXPORT void StencilMask(GLuint mask);
ANGLE_EXPORT void StencilMaskSeparate(GLenum face, GLuint mask);
ANGLE_EXPORT void StencilOp(GLenum fail, GLenum zfail, GLenum zpass);
ANGLE_EXPORT void StencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
ANGLE_EXPORT void TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
ANGLE_EXPORT void TexParameterf(GLenum target, GLenum pname, GLfloat param);
ANGLE_EXPORT void TexParameterfv(GLenum target, GLenum pname, const GLfloat* params);
ANGLE_EXPORT void TexParameteri(GLenum target, GLenum pname, GLint param);
ANGLE_EXPORT void TexParameteriv(GLenum target, GLenum pname, const GLint* params);
ANGLE_EXPORT void TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
ANGLE_EXPORT void Uniform1f(GLint location, GLfloat x);
ANGLE_EXPORT void Uniform1fv(GLint location, GLsizei count, const GLfloat* v);
ANGLE_EXPORT void Uniform1i(GLint location, GLint x);
ANGLE_EXPORT void Uniform1iv(GLint location, GLsizei count, const GLint* v);
ANGLE_EXPORT void Uniform2f(GLint location, GLfloat x, GLfloat y);
ANGLE_EXPORT void Uniform2fv(GLint location, GLsizei count, const GLfloat* v);
ANGLE_EXPORT void Uniform2i(GLint location, GLint x, GLint y);
ANGLE_EXPORT void Uniform2iv(GLint location, GLsizei count, const GLint* v);
ANGLE_EXPORT void Uniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z);
ANGLE_EXPORT void Uniform3fv(GLint location, GLsizei count, const GLfloat* v);
ANGLE_EXPORT void Uniform3i(GLint location, GLint x, GLint y, GLint z);
ANGLE_EXPORT void Uniform3iv(GLint location, GLsizei count, const GLint* v);
ANGLE_EXPORT void Uniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
ANGLE_EXPORT void Uniform4fv(GLint location, GLsizei count, const GLfloat* v);
ANGLE_EXPORT void Uniform4i(GLint location, GLint x, GLint y, GLint z, GLint w);
ANGLE_EXPORT void Uniform4iv(GLint location, GLsizei count, const GLint* v);
ANGLE_EXPORT void UniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
ANGLE_EXPORT void UniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
ANGLE_EXPORT void UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
ANGLE_EXPORT void UseProgram(GLuint program);
ANGLE_EXPORT void ValidateProgram(GLuint program);
ANGLE_EXPORT void VertexAttrib1f(GLuint indx, GLfloat x);
ANGLE_EXPORT void VertexAttrib1fv(GLuint indx, const GLfloat* values);
ANGLE_EXPORT void VertexAttrib2f(GLuint indx, GLfloat x, GLfloat y);
ANGLE_EXPORT void VertexAttrib2fv(GLuint indx, const GLfloat* values);
ANGLE_EXPORT void VertexAttrib3f(GLuint indx, GLfloat x, GLfloat y, GLfloat z);
ANGLE_EXPORT void VertexAttrib3fv(GLuint indx, const GLfloat* values);
ANGLE_EXPORT void VertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
ANGLE_EXPORT void VertexAttrib4fv(GLuint indx, const GLfloat* values);
ANGLE_EXPORT void VertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);
ANGLE_EXPORT void Viewport(GLint x, GLint y, GLsizei width, GLsizei height);

// OpenGL ES 3.0
ANGLE_EXPORT void ReadBuffer(GLenum mode);
ANGLE_EXPORT void DrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices);
ANGLE_EXPORT void TexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
ANGLE_EXPORT void TexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
ANGLE_EXPORT void CopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
ANGLE_EXPORT void CompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
ANGLE_EXPORT void CompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
ANGLE_EXPORT void GenQueries(GLsizei n, GLuint* ids);
ANGLE_EXPORT void DeleteQueries(GLsizei n, const GLuint* ids);
ANGLE_EXPORT GLboolean IsQuery(GLuint id);
ANGLE_EXPORT void BeginQuery(GLenum target, GLuint id);
ANGLE_EXPORT void EndQuery(GLenum target);
ANGLE_EXPORT void GetQueryiv(GLenum target, GLenum pname, GLint* params);
ANGLE_EXPORT void GetQueryObjectuiv(GLuint id, GLenum pname, GLuint* params);
ANGLE_EXPORT GLboolean UnmapBuffer(GLenum target);
ANGLE_EXPORT void GetBufferPointerv(GLenum target, GLenum pname, GLvoid** params);
ANGLE_EXPORT void DrawBuffers(GLsizei n, const GLenum* bufs);
ANGLE_EXPORT void UniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
ANGLE_EXPORT void UniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
ANGLE_EXPORT void UniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
ANGLE_EXPORT void UniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
ANGLE_EXPORT void UniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
ANGLE_EXPORT void UniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
ANGLE_EXPORT void BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
ANGLE_EXPORT void RenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
ANGLE_EXPORT void FramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
ANGLE_EXPORT GLvoid* MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
ANGLE_EXPORT void FlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length);
ANGLE_EXPORT void BindVertexArray(GLuint array);
ANGLE_EXPORT void DeleteVertexArrays(GLsizei n, const GLuint* arrays);
ANGLE_EXPORT void GenVertexArrays(GLsizei n, GLuint* arrays);
ANGLE_EXPORT GLboolean IsVertexArray(GLuint array);
ANGLE_EXPORT void GetIntegeri_v(GLenum target, GLuint index, GLint* data);
ANGLE_EXPORT void BeginTransformFeedback(GLenum primitiveMode);
ANGLE_EXPORT void EndTransformFeedback(void);
ANGLE_EXPORT void BindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
ANGLE_EXPORT void BindBufferBase(GLenum target, GLuint index, GLuint buffer);
ANGLE_EXPORT void TransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode);
ANGLE_EXPORT void GetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name);
ANGLE_EXPORT void VertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
ANGLE_EXPORT void GetVertexAttribIiv(GLuint index, GLenum pname, GLint* params);
ANGLE_EXPORT void GetVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params);
ANGLE_EXPORT void VertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w);
ANGLE_EXPORT void VertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
ANGLE_EXPORT void VertexAttribI4iv(GLuint index, const GLint* v);
ANGLE_EXPORT void VertexAttribI4uiv(GLuint index, const GLuint* v);
ANGLE_EXPORT void GetUniformuiv(GLuint program, GLint location, GLuint* params);
ANGLE_EXPORT GLint GetFragDataLocation(GLuint program, const GLchar *name);
ANGLE_EXPORT void Uniform1ui(GLint location, GLuint v0);
ANGLE_EXPORT void Uniform2ui(GLint location, GLuint v0, GLuint v1);
ANGLE_EXPORT void Uniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2);
ANGLE_EXPORT void Uniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
ANGLE_EXPORT void Uniform1uiv(GLint location, GLsizei count, const GLuint* value);
ANGLE_EXPORT void Uniform2uiv(GLint location, GLsizei count, const GLuint* value);
ANGLE_EXPORT void Uniform3uiv(GLint location, GLsizei count, const GLuint* value);
ANGLE_EXPORT void Uniform4uiv(GLint location, GLsizei count, const GLuint* value);
ANGLE_EXPORT void ClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value);
ANGLE_EXPORT void ClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value);
ANGLE_EXPORT void ClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value);
ANGLE_EXPORT void ClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
ANGLE_EXPORT const GLubyte* GetStringi(GLenum name, GLuint index);
ANGLE_EXPORT void CopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
ANGLE_EXPORT void GetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices);
ANGLE_EXPORT void GetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params);
ANGLE_EXPORT GLuint GetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName);
ANGLE_EXPORT void GetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params);
ANGLE_EXPORT void GetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName);
ANGLE_EXPORT void UniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
ANGLE_EXPORT void DrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount);
ANGLE_EXPORT void DrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount);
ANGLE_EXPORT GLsync FenceSync_(GLenum condition, GLbitfield flags);
ANGLE_EXPORT GLboolean IsSync(GLsync sync);
ANGLE_EXPORT void DeleteSync(GLsync sync);
ANGLE_EXPORT GLenum ClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
ANGLE_EXPORT void WaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
ANGLE_EXPORT void GetInteger64v(GLenum pname, GLint64* params);
ANGLE_EXPORT void GetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values);
ANGLE_EXPORT void GetInteger64i_v(GLenum target, GLuint index, GLint64* data);
ANGLE_EXPORT void GetBufferParameteri64v(GLenum target, GLenum pname, GLint64* params);
ANGLE_EXPORT void GenSamplers(GLsizei count, GLuint* samplers);
ANGLE_EXPORT void DeleteSamplers(GLsizei count, const GLuint* samplers);
ANGLE_EXPORT GLboolean IsSampler(GLuint sampler);
ANGLE_EXPORT void BindSampler(GLuint unit, GLuint sampler);
ANGLE_EXPORT void SamplerParameteri(GLuint sampler, GLenum pname, GLint param);
ANGLE_EXPORT void SamplerParameteriv(GLuint sampler, GLenum pname, const GLint* param);
ANGLE_EXPORT void SamplerParameterf(GLuint sampler, GLenum pname, GLfloat param);
ANGLE_EXPORT void SamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* param);
ANGLE_EXPORT void GetSamplerParameteriv(GLuint sampler, GLenum pname, GLint* params);
ANGLE_EXPORT void GetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat* params);
ANGLE_EXPORT void VertexAttribDivisor(GLuint index, GLuint divisor);
ANGLE_EXPORT void BindTransformFeedback(GLenum target, GLuint id);
ANGLE_EXPORT void DeleteTransformFeedbacks(GLsizei n, const GLuint* ids);
ANGLE_EXPORT void GenTransformFeedbacks(GLsizei n, GLuint* ids);
ANGLE_EXPORT GLboolean IsTransformFeedback(GLuint id);
ANGLE_EXPORT void PauseTransformFeedback(void);
ANGLE_EXPORT void ResumeTransformFeedback(void);
ANGLE_EXPORT void GetProgramBinary(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary);
ANGLE_EXPORT void ProgramBinary(GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length);
ANGLE_EXPORT void ProgramParameteri(GLuint program, GLenum pname, GLint value);
ANGLE_EXPORT void InvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments);
ANGLE_EXPORT void InvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height);
ANGLE_EXPORT void TexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
ANGLE_EXPORT void TexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
ANGLE_EXPORT void GetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params);

// GL_ANGLE_framebuffer_blit
ANGLE_EXPORT void BlitFramebufferANGLE(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

// GL_ANGLE_framebuffer_multisample
ANGLE_EXPORT void RenderbufferStorageMultisampleANGLE(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);

// GL_NV_fence
ANGLE_EXPORT void DeleteFencesNV(GLsizei n, const GLuint* fences);
ANGLE_EXPORT void GenFencesNV(GLsizei n, GLuint* fences);
ANGLE_EXPORT GLboolean IsFenceNV(GLuint fence);
ANGLE_EXPORT GLboolean TestFenceNV(GLuint fence);
ANGLE_EXPORT void GetFenceivNV(GLuint fence, GLenum pname, GLint *params);
ANGLE_EXPORT void FinishFenceNV(GLuint fence);
ANGLE_EXPORT void SetFenceNV(GLuint fence, GLenum condition);

// GL_ANGLE_translated_shader_source
ANGLE_EXPORT void GetTranslatedShaderSourceANGLE(GLuint shader, GLsizei bufsize, GLsizei *length, GLchar *source);

// GL_EXT_texture_storage
ANGLE_EXPORT void TexStorage2DEXT(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);

// GL_EXT_robustness
ANGLE_EXPORT GLenum GetGraphicsResetStatusEXT(void);
ANGLE_EXPORT void ReadnPixelsEXT(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
ANGLE_EXPORT void GetnUniformfvEXT(GLuint program, GLint location, GLsizei bufSize, float *params);
ANGLE_EXPORT void GetnUniformivEXT(GLuint program, GLint location, GLsizei bufSize, GLint *params);

// GL_EXT_occlusion_query_boolean
ANGLE_EXPORT void GenQueriesEXT(GLsizei n, GLuint *ids);
ANGLE_EXPORT void DeleteQueriesEXT(GLsizei n, const GLuint *ids);
ANGLE_EXPORT GLboolean IsQueryEXT(GLuint id);
ANGLE_EXPORT void BeginQueryEXT(GLenum target, GLuint id);
ANGLE_EXPORT void EndQueryEXT(GLenum target);
ANGLE_EXPORT void GetQueryivEXT(GLenum target, GLenum pname, GLint *params);
ANGLE_EXPORT void GetQueryObjectuivEXT(GLuint id, GLenum pname, GLuint *params);

// GL_EXT_draw_buffers
ANGLE_EXPORT void DrawBuffersEXT(GLsizei n, const GLenum *bufs);

// GL_ANGLE_instanced_arrays
ANGLE_EXPORT void DrawArraysInstancedANGLE(GLenum mode, GLint first, GLsizei count, GLsizei primcount);
ANGLE_EXPORT void DrawElementsInstancedANGLE(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
ANGLE_EXPORT void VertexAttribDivisorANGLE(GLuint index, GLuint divisor);

// GL_OES_get_program_binary
ANGLE_EXPORT void GetProgramBinaryOES(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary);
ANGLE_EXPORT void ProgramBinaryOES(GLuint program, GLenum binaryFormat, const GLvoid *binary, GLint length);

// GL_OES_mapbuffer
ANGLE_EXPORT void *MapBufferOES(GLenum target, GLenum access);
ANGLE_EXPORT GLboolean UnmapBufferOES(GLenum target);
ANGLE_EXPORT void GetBufferPointervOES(GLenum target, GLenum pname, GLvoid **params);

// GL_EXT_map_buffer_range
ANGLE_EXPORT void *MapBufferRangeEXT(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
ANGLE_EXPORT void FlushMappedBufferRangeEXT(GLenum target, GLintptr offset, GLsizeiptr length);

}
