//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// libGLESv2.cpp: Implements the exported OpenGL ES 2.0 functions.

#include "angle_gl.h"

#include "libGLESv2/entry_points_gles_1_0_autogen.h"
#include "libGLESv2/entry_points_gles_2_0_autogen.h"
#include "libGLESv2/entry_points_gles_3_0_autogen.h"
#include "libGLESv2/entry_points_gles_3_1_autogen.h"
#include "libGLESv2/entry_points_gles_ext_autogen.h"

#include "common/event_tracer.h"

extern "C" {

void GL_APIENTRY glActiveTexture(GLenum texture)
{
    return gl::ActiveTexture(texture);
}

void GL_APIENTRY glAttachShader(GLuint program, GLuint shader)
{
    return gl::AttachShader(program, shader);
}

void GL_APIENTRY glBindAttribLocation(GLuint program, GLuint index, const GLchar *name)
{
    return gl::BindAttribLocation(program, index, name);
}

void GL_APIENTRY glBindBuffer(GLenum target, GLuint buffer)
{
    return gl::BindBuffer(target, buffer);
}

void GL_APIENTRY glBindFramebuffer(GLenum target, GLuint framebuffer)
{
    return gl::BindFramebuffer(target, framebuffer);
}

void GL_APIENTRY glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
    return gl::BindRenderbuffer(target, renderbuffer);
}

void GL_APIENTRY glBindTexture(GLenum target, GLuint texture)
{
    return gl::BindTexture(target, texture);
}

void GL_APIENTRY glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    return gl::BlendColor(red, green, blue, alpha);
}

void GL_APIENTRY glBlendEquation(GLenum mode)
{
    return gl::BlendEquation(mode);
}

void GL_APIENTRY glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
    return gl::BlendEquationSeparate(modeRGB, modeAlpha);
}

void GL_APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    return gl::BlendFunc(sfactor, dfactor);
}

void GL_APIENTRY glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    return gl::BlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
}

void GL_APIENTRY glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage)
{
    return gl::BufferData(target, size, data, usage);
}

void GL_APIENTRY glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data)
{
    return gl::BufferSubData(target, offset, size, data);
}

GLenum GL_APIENTRY glCheckFramebufferStatus(GLenum target)
{
    return gl::CheckFramebufferStatus(target);
}

void GL_APIENTRY glClear(GLbitfield mask)
{
    return gl::Clear(mask);
}

void GL_APIENTRY glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    return gl::ClearColor(red, green, blue, alpha);
}

void GL_APIENTRY glClearDepthf(GLfloat depth)
{
    return gl::ClearDepthf(depth);
}

void GL_APIENTRY glClearStencil(GLint s)
{
    return gl::ClearStencil(s);
}

void GL_APIENTRY glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    return gl::ColorMask(red, green, blue, alpha);
}

void GL_APIENTRY glCompileShader(GLuint shader)
{
    return gl::CompileShader(shader);
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
    return gl::CompressedTexImage2D(target, level, internalformat, width, height, border, imageSize,
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
    return gl::CompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format,
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
    return gl::CopyTexImage2D(target, level, internalformat, x, y, width, height, border);
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
    return gl::CopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

GLuint GL_APIENTRY glCreateProgram(void)
{
    return gl::CreateProgram();
}

GLuint GL_APIENTRY glCreateShader(GLenum type)
{
    return gl::CreateShader(type);
}

void GL_APIENTRY glCullFace(GLenum mode)
{
    return gl::CullFace(mode);
}

void GL_APIENTRY glDeleteBuffers(GLsizei n, const GLuint *buffers)
{
    return gl::DeleteBuffers(n, buffers);
}

void GL_APIENTRY glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers)
{
    return gl::DeleteFramebuffers(n, framebuffers);
}

void GL_APIENTRY glDeleteProgram(GLuint program)
{
    return gl::DeleteProgram(program);
}

void GL_APIENTRY glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers)
{
    return gl::DeleteRenderbuffers(n, renderbuffers);
}

void GL_APIENTRY glDeleteShader(GLuint shader)
{
    return gl::DeleteShader(shader);
}

void GL_APIENTRY glDeleteTextures(GLsizei n, const GLuint *textures)
{
    return gl::DeleteTextures(n, textures);
}

void GL_APIENTRY glDepthFunc(GLenum func)
{
    return gl::DepthFunc(func);
}

void GL_APIENTRY glDepthMask(GLboolean flag)
{
    return gl::DepthMask(flag);
}

void GL_APIENTRY glDepthRangef(GLfloat n, GLfloat f)
{
    return gl::DepthRangef(n, f);
}

void GL_APIENTRY glDetachShader(GLuint program, GLuint shader)
{
    return gl::DetachShader(program, shader);
}

void GL_APIENTRY glDisable(GLenum cap)
{
    return gl::Disable(cap);
}

void GL_APIENTRY glDisableVertexAttribArray(GLuint index)
{
    return gl::DisableVertexAttribArray(index);
}

void GL_APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    return gl::DrawArrays(mode, first, count);
}

void GL_APIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
    return gl::DrawElements(mode, count, type, indices);
}

void GL_APIENTRY glEnable(GLenum cap)
{
    return gl::Enable(cap);
}

void GL_APIENTRY glEnableVertexAttribArray(GLuint index)
{
    return gl::EnableVertexAttribArray(index);
}

void GL_APIENTRY glFinish(void)
{
    return gl::Finish();
}

void GL_APIENTRY glFlush(void)
{
    return gl::Flush();
}

void GL_APIENTRY glFramebufferRenderbuffer(GLenum target,
                                           GLenum attachment,
                                           GLenum renderbuffertarget,
                                           GLuint renderbuffer)
{
    return gl::FramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}

void GL_APIENTRY glFramebufferTexture2D(GLenum target,
                                        GLenum attachment,
                                        GLenum textarget,
                                        GLuint texture,
                                        GLint level)
{
    return gl::FramebufferTexture2D(target, attachment, textarget, texture, level);
}

void GL_APIENTRY glFrontFace(GLenum mode)
{
    return gl::FrontFace(mode);
}

void GL_APIENTRY glGenBuffers(GLsizei n, GLuint *buffers)
{
    return gl::GenBuffers(n, buffers);
}

void GL_APIENTRY glGenerateMipmap(GLenum target)
{
    return gl::GenerateMipmap(target);
}

void GL_APIENTRY glGenFramebuffers(GLsizei n, GLuint *framebuffers)
{
    return gl::GenFramebuffers(n, framebuffers);
}

void GL_APIENTRY glGenRenderbuffers(GLsizei n, GLuint *renderbuffers)
{
    return gl::GenRenderbuffers(n, renderbuffers);
}

void GL_APIENTRY glGenTextures(GLsizei n, GLuint *textures)
{
    return gl::GenTextures(n, textures);
}

void GL_APIENTRY glGetActiveAttrib(GLuint program,
                                   GLuint index,
                                   GLsizei bufsize,
                                   GLsizei *length,
                                   GLint *size,
                                   GLenum *type,
                                   GLchar *name)
{
    return gl::GetActiveAttrib(program, index, bufsize, length, size, type, name);
}

void GL_APIENTRY glGetActiveUniform(GLuint program,
                                    GLuint index,
                                    GLsizei bufsize,
                                    GLsizei *length,
                                    GLint *size,
                                    GLenum *type,
                                    GLchar *name)
{
    return gl::GetActiveUniform(program, index, bufsize, length, size, type, name);
}

void GL_APIENTRY glGetAttachedShaders(GLuint program,
                                      GLsizei maxcount,
                                      GLsizei *count,
                                      GLuint *shaders)
{
    return gl::GetAttachedShaders(program, maxcount, count, shaders);
}

GLint GL_APIENTRY glGetAttribLocation(GLuint program, const GLchar *name)
{
    return gl::GetAttribLocation(program, name);
}

void GL_APIENTRY glGetBooleanv(GLenum pname, GLboolean *params)
{
    return gl::GetBooleanv(pname, params);
}

void GL_APIENTRY glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    return gl::GetBufferParameteriv(target, pname, params);
}

GLenum GL_APIENTRY glGetError(void)
{
    return gl::GetError();
}

void GL_APIENTRY glGetFloatv(GLenum pname, GLfloat *params)
{
    return gl::GetFloatv(pname, params);
}

void GL_APIENTRY glGetFramebufferAttachmentParameteriv(GLenum target,
                                                       GLenum attachment,
                                                       GLenum pname,
                                                       GLint *params)
{
    return gl::GetFramebufferAttachmentParameteriv(target, attachment, pname, params);
}

void GL_APIENTRY glGetIntegerv(GLenum pname, GLint *params)
{
    return gl::GetIntegerv(pname, params);
}

void GL_APIENTRY glGetProgramiv(GLuint program, GLenum pname, GLint *params)
{
    return gl::GetProgramiv(program, pname, params);
}

void GL_APIENTRY glGetProgramInfoLog(GLuint program,
                                     GLsizei bufsize,
                                     GLsizei *length,
                                     GLchar *infolog)
{
    return gl::GetProgramInfoLog(program, bufsize, length, infolog);
}

void GL_APIENTRY glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    return gl::GetRenderbufferParameteriv(target, pname, params);
}

void GL_APIENTRY glGetShaderiv(GLuint shader, GLenum pname, GLint *params)
{
    return gl::GetShaderiv(shader, pname, params);
}

void GL_APIENTRY glGetShaderInfoLog(GLuint shader,
                                    GLsizei bufsize,
                                    GLsizei *length,
                                    GLchar *infolog)
{
    return gl::GetShaderInfoLog(shader, bufsize, length, infolog);
}

void GL_APIENTRY glGetShaderPrecisionFormat(GLenum shadertype,
                                            GLenum precisiontype,
                                            GLint *range,
                                            GLint *precision)
{
    return gl::GetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
}

void GL_APIENTRY glGetShaderSource(GLuint shader, GLsizei bufsize, GLsizei *length, GLchar *source)
{
    return gl::GetShaderSource(shader, bufsize, length, source);
}

const GLubyte *GL_APIENTRY glGetString(GLenum name)
{
    return gl::GetString(name);
}

void GL_APIENTRY glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    return gl::GetTexParameterfv(target, pname, params);
}

void GL_APIENTRY glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
    return gl::GetTexParameteriv(target, pname, params);
}

void GL_APIENTRY glGetUniformfv(GLuint program, GLint location, GLfloat *params)
{
    return gl::GetUniformfv(program, location, params);
}

void GL_APIENTRY glGetUniformiv(GLuint program, GLint location, GLint *params)
{
    return gl::GetUniformiv(program, location, params);
}

GLint GL_APIENTRY glGetUniformLocation(GLuint program, const GLchar *name)
{
    return gl::GetUniformLocation(program, name);
}

void GL_APIENTRY glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params)
{
    return gl::GetVertexAttribfv(index, pname, params);
}

void GL_APIENTRY glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params)
{
    return gl::GetVertexAttribiv(index, pname, params);
}

void GL_APIENTRY glGetVertexAttribPointerv(GLuint index, GLenum pname, void **pointer)
{
    return gl::GetVertexAttribPointerv(index, pname, pointer);
}

void GL_APIENTRY glHint(GLenum target, GLenum mode)
{
    return gl::Hint(target, mode);
}

GLboolean GL_APIENTRY glIsBuffer(GLuint buffer)
{
    return gl::IsBuffer(buffer);
}

GLboolean GL_APIENTRY glIsEnabled(GLenum cap)
{
    return gl::IsEnabled(cap);
}

GLboolean GL_APIENTRY glIsFramebuffer(GLuint framebuffer)
{
    return gl::IsFramebuffer(framebuffer);
}

GLboolean GL_APIENTRY glIsProgram(GLuint program)
{
    return gl::IsProgram(program);
}

GLboolean GL_APIENTRY glIsRenderbuffer(GLuint renderbuffer)
{
    return gl::IsRenderbuffer(renderbuffer);
}

GLboolean GL_APIENTRY glIsShader(GLuint shader)
{
    return gl::IsShader(shader);
}

GLboolean GL_APIENTRY glIsTexture(GLuint texture)
{
    return gl::IsTexture(texture);
}

void GL_APIENTRY glLineWidth(GLfloat width)
{
    return gl::LineWidth(width);
}

void GL_APIENTRY glLinkProgram(GLuint program)
{
    return gl::LinkProgram(program);
}

void GL_APIENTRY glPixelStorei(GLenum pname, GLint param)
{
    return gl::PixelStorei(pname, param);
}

void GL_APIENTRY glPolygonOffset(GLfloat factor, GLfloat units)
{
    return gl::PolygonOffset(factor, units);
}

void GL_APIENTRY glReadPixels(GLint x,
                              GLint y,
                              GLsizei width,
                              GLsizei height,
                              GLenum format,
                              GLenum type,
                              void *pixels)
{
    return gl::ReadPixels(x, y, width, height, format, type, pixels);
}

void GL_APIENTRY glReleaseShaderCompiler(void)
{
    return gl::ReleaseShaderCompiler();
}

void GL_APIENTRY glRenderbufferStorage(GLenum target,
                                       GLenum internalformat,
                                       GLsizei width,
                                       GLsizei height)
{
    return gl::RenderbufferStorage(target, internalformat, width, height);
}

void GL_APIENTRY glSampleCoverage(GLfloat value, GLboolean invert)
{
    return gl::SampleCoverage(value, invert);
}

void GL_APIENTRY glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    return gl::Scissor(x, y, width, height);
}

void GL_APIENTRY glShaderBinary(GLsizei n,
                                const GLuint *shaders,
                                GLenum binaryformat,
                                const void *binary,
                                GLsizei length)
{
    return gl::ShaderBinary(n, shaders, binaryformat, binary, length);
}

void GL_APIENTRY glShaderSource(GLuint shader,
                                GLsizei count,
                                const GLchar *const *string,
                                const GLint *length)
{
    return gl::ShaderSource(shader, count, string, length);
}

void GL_APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
    return gl::StencilFunc(func, ref, mask);
}

void GL_APIENTRY glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
    return gl::StencilFuncSeparate(face, func, ref, mask);
}

void GL_APIENTRY glStencilMask(GLuint mask)
{
    return gl::StencilMask(mask);
}

void GL_APIENTRY glStencilMaskSeparate(GLenum face, GLuint mask)
{
    return gl::StencilMaskSeparate(face, mask);
}

void GL_APIENTRY glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
    return gl::StencilOp(fail, zfail, zpass);
}

void GL_APIENTRY glStencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
    return gl::StencilOpSeparate(face, fail, zfail, zpass);
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
    return gl::TexImage2D(target, level, internalformat, width, height, border, format, type,
                          pixels);
}

void GL_APIENTRY glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    return gl::TexParameterf(target, pname, param);
}

void GL_APIENTRY glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    return gl::TexParameterfv(target, pname, params);
}

void GL_APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param)
{
    return gl::TexParameteri(target, pname, param);
}

void GL_APIENTRY glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    return gl::TexParameteriv(target, pname, params);
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
    return gl::TexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void GL_APIENTRY glUniform1f(GLint location, GLfloat x)
{
    return gl::Uniform1f(location, x);
}

void GL_APIENTRY glUniform1fv(GLint location, GLsizei count, const GLfloat *v)
{
    return gl::Uniform1fv(location, count, v);
}

void GL_APIENTRY glUniform1i(GLint location, GLint x)
{
    return gl::Uniform1i(location, x);
}

void GL_APIENTRY glUniform1iv(GLint location, GLsizei count, const GLint *v)
{
    return gl::Uniform1iv(location, count, v);
}

void GL_APIENTRY glUniform2f(GLint location, GLfloat x, GLfloat y)
{
    return gl::Uniform2f(location, x, y);
}

void GL_APIENTRY glUniform2fv(GLint location, GLsizei count, const GLfloat *v)
{
    return gl::Uniform2fv(location, count, v);
}

void GL_APIENTRY glUniform2i(GLint location, GLint x, GLint y)
{
    return gl::Uniform2i(location, x, y);
}

void GL_APIENTRY glUniform2iv(GLint location, GLsizei count, const GLint *v)
{
    return gl::Uniform2iv(location, count, v);
}

void GL_APIENTRY glUniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    return gl::Uniform3f(location, x, y, z);
}

void GL_APIENTRY glUniform3fv(GLint location, GLsizei count, const GLfloat *v)
{
    return gl::Uniform3fv(location, count, v);
}

void GL_APIENTRY glUniform3i(GLint location, GLint x, GLint y, GLint z)
{
    return gl::Uniform3i(location, x, y, z);
}

void GL_APIENTRY glUniform3iv(GLint location, GLsizei count, const GLint *v)
{
    return gl::Uniform3iv(location, count, v);
}

void GL_APIENTRY glUniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    return gl::Uniform4f(location, x, y, z, w);
}

void GL_APIENTRY glUniform4fv(GLint location, GLsizei count, const GLfloat *v)
{
    return gl::Uniform4fv(location, count, v);
}

void GL_APIENTRY glUniform4i(GLint location, GLint x, GLint y, GLint z, GLint w)
{
    return gl::Uniform4i(location, x, y, z, w);
}

void GL_APIENTRY glUniform4iv(GLint location, GLsizei count, const GLint *v)
{
    return gl::Uniform4iv(location, count, v);
}

void GL_APIENTRY glUniformMatrix2fv(GLint location,
                                    GLsizei count,
                                    GLboolean transpose,
                                    const GLfloat *value)
{
    return gl::UniformMatrix2fv(location, count, transpose, value);
}

void GL_APIENTRY glUniformMatrix3fv(GLint location,
                                    GLsizei count,
                                    GLboolean transpose,
                                    const GLfloat *value)
{
    return gl::UniformMatrix3fv(location, count, transpose, value);
}

void GL_APIENTRY glUniformMatrix4fv(GLint location,
                                    GLsizei count,
                                    GLboolean transpose,
                                    const GLfloat *value)
{
    return gl::UniformMatrix4fv(location, count, transpose, value);
}

void GL_APIENTRY glUseProgram(GLuint program)
{
    return gl::UseProgram(program);
}

void GL_APIENTRY glValidateProgram(GLuint program)
{
    return gl::ValidateProgram(program);
}

void GL_APIENTRY glVertexAttrib1f(GLuint indx, GLfloat x)
{
    return gl::VertexAttrib1f(indx, x);
}

void GL_APIENTRY glVertexAttrib1fv(GLuint indx, const GLfloat *values)
{
    return gl::VertexAttrib1fv(indx, values);
}

void GL_APIENTRY glVertexAttrib2f(GLuint indx, GLfloat x, GLfloat y)
{
    return gl::VertexAttrib2f(indx, x, y);
}

void GL_APIENTRY glVertexAttrib2fv(GLuint indx, const GLfloat *values)
{
    return gl::VertexAttrib2fv(indx, values);
}

void GL_APIENTRY glVertexAttrib3f(GLuint indx, GLfloat x, GLfloat y, GLfloat z)
{
    return gl::VertexAttrib3f(indx, x, y, z);
}

void GL_APIENTRY glVertexAttrib3fv(GLuint indx, const GLfloat *values)
{
    return gl::VertexAttrib3fv(indx, values);
}

void GL_APIENTRY glVertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    return gl::VertexAttrib4f(indx, x, y, z, w);
}

void GL_APIENTRY glVertexAttrib4fv(GLuint indx, const GLfloat *values)
{
    return gl::VertexAttrib4fv(indx, values);
}

void GL_APIENTRY glVertexAttribPointer(GLuint indx,
                                       GLint size,
                                       GLenum type,
                                       GLboolean normalized,
                                       GLsizei stride,
                                       const void *ptr)
{
    return gl::VertexAttribPointer(indx, size, type, normalized, stride, ptr);
}

void GL_APIENTRY glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    return gl::Viewport(x, y, width, height);
}

void GL_APIENTRY glReadBuffer(GLenum mode)
{
    return gl::ReadBuffer(mode);
}

void GL_APIENTRY glDrawRangeElements(GLenum mode,
                                     GLuint start,
                                     GLuint end,
                                     GLsizei count,
                                     GLenum type,
                                     const void *indices)
{
    return gl::DrawRangeElements(mode, start, end, count, type, indices);
}

void GL_APIENTRY glTexImage3D(GLenum target,
                              GLint level,
                              GLint internalformat,
                              GLsizei width,
                              GLsizei height,
                              GLsizei depth,
                              GLint border,
                              GLenum format,
                              GLenum type,
                              const void *pixels)
{
    return gl::TexImage3D(target, level, internalformat, width, height, depth, border, format, type,
                          pixels);
}

void GL_APIENTRY glTexSubImage3D(GLenum target,
                                 GLint level,
                                 GLint xoffset,
                                 GLint yoffset,
                                 GLint zoffset,
                                 GLsizei width,
                                 GLsizei height,
                                 GLsizei depth,
                                 GLenum format,
                                 GLenum type,
                                 const void *pixels)
{
    return gl::TexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format,
                             type, pixels);
}

void GL_APIENTRY glCopyTexSubImage3D(GLenum target,
                                     GLint level,
                                     GLint xoffset,
                                     GLint yoffset,
                                     GLint zoffset,
                                     GLint x,
                                     GLint y,
                                     GLsizei width,
                                     GLsizei height)
{
    return gl::CopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

void GL_APIENTRY glCompressedTexImage3D(GLenum target,
                                        GLint level,
                                        GLenum internalformat,
                                        GLsizei width,
                                        GLsizei height,
                                        GLsizei depth,
                                        GLint border,
                                        GLsizei imageSize,
                                        const void *data)
{
    return gl::CompressedTexImage3D(target, level, internalformat, width, height, depth, border,
                                    imageSize, data);
}

void GL_APIENTRY glCompressedTexSubImage3D(GLenum target,
                                           GLint level,
                                           GLint xoffset,
                                           GLint yoffset,
                                           GLint zoffset,
                                           GLsizei width,
                                           GLsizei height,
                                           GLsizei depth,
                                           GLenum format,
                                           GLsizei imageSize,
                                           const void *data)
{
    return gl::CompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height,
                                       depth, format, imageSize, data);
}

void GL_APIENTRY glGenQueries(GLsizei n, GLuint *ids)
{
    return gl::GenQueries(n, ids);
}

void GL_APIENTRY glDeleteQueries(GLsizei n, const GLuint *ids)
{
    return gl::DeleteQueries(n, ids);
}

GLboolean GL_APIENTRY glIsQuery(GLuint id)
{
    return gl::IsQuery(id);
}

void GL_APIENTRY glBeginQuery(GLenum target, GLuint id)
{
    return gl::BeginQuery(target, id);
}

void GL_APIENTRY glEndQuery(GLenum target)
{
    return gl::EndQuery(target);
}

void GL_APIENTRY glGetQueryiv(GLenum target, GLenum pname, GLint *params)
{
    return gl::GetQueryiv(target, pname, params);
}

void GL_APIENTRY glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params)
{
    return gl::GetQueryObjectuiv(id, pname, params);
}

GLboolean GL_APIENTRY glUnmapBuffer(GLenum target)
{
    return gl::UnmapBuffer(target);
}

void GL_APIENTRY glGetBufferPointerv(GLenum target, GLenum pname, void **params)
{
    return gl::GetBufferPointerv(target, pname, params);
}

void GL_APIENTRY glDrawBuffers(GLsizei n, const GLenum *bufs)
{
    return gl::DrawBuffers(n, bufs);
}

void GL_APIENTRY glUniformMatrix2x3fv(GLint location,
                                      GLsizei count,
                                      GLboolean transpose,
                                      const GLfloat *value)
{
    return gl::UniformMatrix2x3fv(location, count, transpose, value);
}

void GL_APIENTRY glUniformMatrix3x2fv(GLint location,
                                      GLsizei count,
                                      GLboolean transpose,
                                      const GLfloat *value)
{
    return gl::UniformMatrix3x2fv(location, count, transpose, value);
}

void GL_APIENTRY glUniformMatrix2x4fv(GLint location,
                                      GLsizei count,
                                      GLboolean transpose,
                                      const GLfloat *value)
{
    return gl::UniformMatrix2x4fv(location, count, transpose, value);
}

void GL_APIENTRY glUniformMatrix4x2fv(GLint location,
                                      GLsizei count,
                                      GLboolean transpose,
                                      const GLfloat *value)
{
    return gl::UniformMatrix4x2fv(location, count, transpose, value);
}

void GL_APIENTRY glUniformMatrix3x4fv(GLint location,
                                      GLsizei count,
                                      GLboolean transpose,
                                      const GLfloat *value)
{
    return gl::UniformMatrix3x4fv(location, count, transpose, value);
}

void GL_APIENTRY glUniformMatrix4x3fv(GLint location,
                                      GLsizei count,
                                      GLboolean transpose,
                                      const GLfloat *value)
{
    return gl::UniformMatrix4x3fv(location, count, transpose, value);
}

void GL_APIENTRY glBlitFramebuffer(GLint srcX0,
                                   GLint srcY0,
                                   GLint srcX1,
                                   GLint srcY1,
                                   GLint dstX0,
                                   GLint dstY0,
                                   GLint dstX1,
                                   GLint dstY1,
                                   GLbitfield mask,
                                   GLenum filter)
{
    return gl::BlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask,
                               filter);
}

void GL_APIENTRY glRenderbufferStorageMultisample(GLenum target,
                                                  GLsizei samples,
                                                  GLenum internalformat,
                                                  GLsizei width,
                                                  GLsizei height)
{
    return gl::RenderbufferStorageMultisample(target, samples, internalformat, width, height);
}

void GL_APIENTRY glFramebufferTextureLayer(GLenum target,
                                           GLenum attachment,
                                           GLuint texture,
                                           GLint level,
                                           GLint layer)
{
    return gl::FramebufferTextureLayer(target, attachment, texture, level, layer);
}

void *GL_APIENTRY glMapBufferRange(GLenum target,
                                   GLintptr offset,
                                   GLsizeiptr length,
                                   GLbitfield access)
{
    return gl::MapBufferRange(target, offset, length, access);
}

void GL_APIENTRY glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
    return gl::FlushMappedBufferRange(target, offset, length);
}

void GL_APIENTRY glBindVertexArray(GLuint array)
{
    return gl::BindVertexArray(array);
}

void GL_APIENTRY glDeleteVertexArrays(GLsizei n, const GLuint *arrays)
{
    return gl::DeleteVertexArrays(n, arrays);
}

void GL_APIENTRY glGenVertexArrays(GLsizei n, GLuint *arrays)
{
    return gl::GenVertexArrays(n, arrays);
}

GLboolean GL_APIENTRY glIsVertexArray(GLuint array)
{
    return gl::IsVertexArray(array);
}

void GL_APIENTRY glGetIntegeri_v(GLenum target, GLuint index, GLint *data)
{
    return gl::GetIntegeri_v(target, index, data);
}

void GL_APIENTRY glBeginTransformFeedback(GLenum primitiveMode)
{
    return gl::BeginTransformFeedback(primitiveMode);
}

void GL_APIENTRY glEndTransformFeedback(void)
{
    return gl::EndTransformFeedback();
}

void GL_APIENTRY
glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    return gl::BindBufferRange(target, index, buffer, offset, size);
}

void GL_APIENTRY glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
    return gl::BindBufferBase(target, index, buffer);
}

void GL_APIENTRY glTransformFeedbackVaryings(GLuint program,
                                             GLsizei count,
                                             const GLchar *const *varyings,
                                             GLenum bufferMode)
{
    return gl::TransformFeedbackVaryings(program, count, varyings, bufferMode);
}

void GL_APIENTRY glGetTransformFeedbackVarying(GLuint program,
                                               GLuint index,
                                               GLsizei bufSize,
                                               GLsizei *length,
                                               GLsizei *size,
                                               GLenum *type,
                                               GLchar *name)
{
    return gl::GetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
}

void GL_APIENTRY
glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    return gl::VertexAttribIPointer(index, size, type, stride, pointer);
}

void GL_APIENTRY glGetVertexAttribIiv(GLuint index, GLenum pname, GLint *params)
{
    return gl::GetVertexAttribIiv(index, pname, params);
}

void GL_APIENTRY glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint *params)
{
    return gl::GetVertexAttribIuiv(index, pname, params);
}

void GL_APIENTRY glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    return gl::VertexAttribI4i(index, x, y, z, w);
}

void GL_APIENTRY glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
    return gl::VertexAttribI4ui(index, x, y, z, w);
}

void GL_APIENTRY glVertexAttribI4iv(GLuint index, const GLint *v)
{
    return gl::VertexAttribI4iv(index, v);
}

void GL_APIENTRY glVertexAttribI4uiv(GLuint index, const GLuint *v)
{
    return gl::VertexAttribI4uiv(index, v);
}

void GL_APIENTRY glGetUniformuiv(GLuint program, GLint location, GLuint *params)
{
    return gl::GetUniformuiv(program, location, params);
}

GLint GL_APIENTRY glGetFragDataLocation(GLuint program, const GLchar *name)
{
    return gl::GetFragDataLocation(program, name);
}

void GL_APIENTRY glUniform1ui(GLint location, GLuint v0)
{
    return gl::Uniform1ui(location, v0);
}

void GL_APIENTRY glUniform2ui(GLint location, GLuint v0, GLuint v1)
{
    return gl::Uniform2ui(location, v0, v1);
}

void GL_APIENTRY glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    return gl::Uniform3ui(location, v0, v1, v2);
}

void GL_APIENTRY glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    return gl::Uniform4ui(location, v0, v1, v2, v3);
}

void GL_APIENTRY glUniform1uiv(GLint location, GLsizei count, const GLuint *value)
{
    return gl::Uniform1uiv(location, count, value);
}

void GL_APIENTRY glUniform2uiv(GLint location, GLsizei count, const GLuint *value)
{
    return gl::Uniform2uiv(location, count, value);
}

void GL_APIENTRY glUniform3uiv(GLint location, GLsizei count, const GLuint *value)
{
    return gl::Uniform3uiv(location, count, value);
}

void GL_APIENTRY glUniform4uiv(GLint location, GLsizei count, const GLuint *value)
{
    return gl::Uniform4uiv(location, count, value);
}

void GL_APIENTRY glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint *value)
{
    return gl::ClearBufferiv(buffer, drawbuffer, value);
}

void GL_APIENTRY glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint *value)
{
    return gl::ClearBufferuiv(buffer, drawbuffer, value);
}

void GL_APIENTRY glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat *value)
{
    return gl::ClearBufferfv(buffer, drawbuffer, value);
}

void GL_APIENTRY glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    return gl::ClearBufferfi(buffer, drawbuffer, depth, stencil);
}

const GLubyte *GL_APIENTRY glGetStringi(GLenum name, GLuint index)
{
    return gl::GetStringi(name, index);
}

void GL_APIENTRY glCopyBufferSubData(GLenum readTarget,
                                     GLenum writeTarget,
                                     GLintptr readOffset,
                                     GLintptr writeOffset,
                                     GLsizeiptr size)
{
    return gl::CopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
}

void GL_APIENTRY glGetUniformIndices(GLuint program,
                                     GLsizei uniformCount,
                                     const GLchar *const *uniformNames,
                                     GLuint *uniformIndices)
{
    return gl::GetUniformIndices(program, uniformCount, uniformNames, uniformIndices);
}

void GL_APIENTRY glGetActiveUniformsiv(GLuint program,
                                       GLsizei uniformCount,
                                       const GLuint *uniformIndices,
                                       GLenum pname,
                                       GLint *params)
{
    return gl::GetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
}

GLuint GL_APIENTRY glGetUniformBlockIndex(GLuint program, const GLchar *uniformBlockName)
{
    return gl::GetUniformBlockIndex(program, uniformBlockName);
}

void GL_APIENTRY glGetActiveUniformBlockiv(GLuint program,
                                           GLuint uniformBlockIndex,
                                           GLenum pname,
                                           GLint *params)
{
    return gl::GetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
}

void GL_APIENTRY glGetActiveUniformBlockName(GLuint program,
                                             GLuint uniformBlockIndex,
                                             GLsizei bufSize,
                                             GLsizei *length,
                                             GLchar *uniformBlockName)
{
    return gl::GetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length,
                                         uniformBlockName);
}

void GL_APIENTRY glUniformBlockBinding(GLuint program,
                                       GLuint uniformBlockIndex,
                                       GLuint uniformBlockBinding)
{
    return gl::UniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
}

void GL_APIENTRY glDrawArraysInstanced(GLenum mode,
                                       GLint first,
                                       GLsizei count,
                                       GLsizei instanceCount)
{
    return gl::DrawArraysInstanced(mode, first, count, instanceCount);
}

void GL_APIENTRY glDrawElementsInstanced(GLenum mode,
                                         GLsizei count,
                                         GLenum type,
                                         const void *indices,
                                         GLsizei instanceCount)
{
    return gl::DrawElementsInstanced(mode, count, type, indices, instanceCount);
}

GLsync GL_APIENTRY glFenceSync(GLenum condition, GLbitfield flags)
{
    return gl::FenceSync(condition, flags);
}

GLboolean GL_APIENTRY glIsSync(GLsync sync)
{
    return gl::IsSync(sync);
}

void GL_APIENTRY glDeleteSync(GLsync sync)
{
    return gl::DeleteSync(sync);
}

GLenum GL_APIENTRY glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    return gl::ClientWaitSync(sync, flags, timeout);
}

void GL_APIENTRY glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    return gl::WaitSync(sync, flags, timeout);
}

void GL_APIENTRY glGetInteger64v(GLenum pname, GLint64 *params)
{
    return gl::GetInteger64v(pname, params);
}

void GL_APIENTRY
glGetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values)
{
    return gl::GetSynciv(sync, pname, bufSize, length, values);
}

void GL_APIENTRY glGetInteger64i_v(GLenum target, GLuint index, GLint64 *data)
{
    return gl::GetInteger64i_v(target, index, data);
}

void GL_APIENTRY glGetBufferParameteri64v(GLenum target, GLenum pname, GLint64 *params)
{
    return gl::GetBufferParameteri64v(target, pname, params);
}

void GL_APIENTRY glGenSamplers(GLsizei count, GLuint *samplers)
{
    return gl::GenSamplers(count, samplers);
}

void GL_APIENTRY glDeleteSamplers(GLsizei count, const GLuint *samplers)
{
    return gl::DeleteSamplers(count, samplers);
}

GLboolean GL_APIENTRY glIsSampler(GLuint sampler)
{
    return gl::IsSampler(sampler);
}

void GL_APIENTRY glBindSampler(GLuint unit, GLuint sampler)
{
    return gl::BindSampler(unit, sampler);
}

void GL_APIENTRY glSamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
    return gl::SamplerParameteri(sampler, pname, param);
}

void GL_APIENTRY glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint *param)
{
    return gl::SamplerParameteriv(sampler, pname, param);
}

void GL_APIENTRY glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
    return gl::SamplerParameterf(sampler, pname, param);
}

void GL_APIENTRY glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat *param)
{
    return gl::SamplerParameterfv(sampler, pname, param);
}

void GL_APIENTRY glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint *params)
{
    return gl::GetSamplerParameteriv(sampler, pname, params);
}

void GL_APIENTRY glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat *params)
{
    return gl::GetSamplerParameterfv(sampler, pname, params);
}

void GL_APIENTRY glVertexAttribDivisor(GLuint index, GLuint divisor)
{
    return gl::VertexAttribDivisor(index, divisor);
}

void GL_APIENTRY glBindTransformFeedback(GLenum target, GLuint id)
{
    return gl::BindTransformFeedback(target, id);
}

void GL_APIENTRY glDeleteTransformFeedbacks(GLsizei n, const GLuint *ids)
{
    return gl::DeleteTransformFeedbacks(n, ids);
}

void GL_APIENTRY glGenTransformFeedbacks(GLsizei n, GLuint *ids)
{
    return gl::GenTransformFeedbacks(n, ids);
}

GLboolean GL_APIENTRY glIsTransformFeedback(GLuint id)
{
    return gl::IsTransformFeedback(id);
}

void GL_APIENTRY glPauseTransformFeedback(void)
{
    return gl::PauseTransformFeedback();
}

void GL_APIENTRY glResumeTransformFeedback(void)
{
    return gl::ResumeTransformFeedback();
}

void GL_APIENTRY glGetProgramBinary(GLuint program,
                                    GLsizei bufSize,
                                    GLsizei *length,
                                    GLenum *binaryFormat,
                                    void *binary)
{
    return gl::GetProgramBinary(program, bufSize, length, binaryFormat, binary);
}

void GL_APIENTRY glProgramBinary(GLuint program,
                                 GLenum binaryFormat,
                                 const void *binary,
                                 GLsizei length)
{
    return gl::ProgramBinary(program, binaryFormat, binary, length);
}

void GL_APIENTRY glProgramParameteri(GLuint program, GLenum pname, GLint value)
{
    return gl::ProgramParameteri(program, pname, value);
}

void GL_APIENTRY glInvalidateFramebuffer(GLenum target,
                                         GLsizei numAttachments,
                                         const GLenum *attachments)
{
    return gl::InvalidateFramebuffer(target, numAttachments, attachments);
}

void GL_APIENTRY glInvalidateSubFramebuffer(GLenum target,
                                            GLsizei numAttachments,
                                            const GLenum *attachments,
                                            GLint x,
                                            GLint y,
                                            GLsizei width,
                                            GLsizei height)
{
    return gl::InvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height);
}

void GL_APIENTRY
glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    return gl::TexStorage2D(target, levels, internalformat, width, height);
}

void GL_APIENTRY glTexStorage3D(GLenum target,
                                GLsizei levels,
                                GLenum internalformat,
                                GLsizei width,
                                GLsizei height,
                                GLsizei depth)
{
    return gl::TexStorage3D(target, levels, internalformat, width, height, depth);
}

void GL_APIENTRY glGetInternalformativ(GLenum target,
                                       GLenum internalformat,
                                       GLenum pname,
                                       GLsizei bufSize,
                                       GLint *params)
{
    return gl::GetInternalformativ(target, internalformat, pname, bufSize, params);
}

void GL_APIENTRY glBlitFramebufferANGLE(GLint srcX0,
                                        GLint srcY0,
                                        GLint srcX1,
                                        GLint srcY1,
                                        GLint dstX0,
                                        GLint dstY0,
                                        GLint dstX1,
                                        GLint dstY1,
                                        GLbitfield mask,
                                        GLenum filter)
{
    return gl::BlitFramebufferANGLE(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask,
                                    filter);
}

void GL_APIENTRY glRenderbufferStorageMultisampleANGLE(GLenum target,
                                                       GLsizei samples,
                                                       GLenum internalformat,
                                                       GLsizei width,
                                                       GLsizei height)
{
    return gl::RenderbufferStorageMultisampleANGLE(target, samples, internalformat, width, height);
}

void GL_APIENTRY glDiscardFramebufferEXT(GLenum target,
                                         GLsizei numAttachments,
                                         const GLenum *attachments)
{
    return gl::DiscardFramebufferEXT(target, numAttachments, attachments);
}

void GL_APIENTRY glDeleteFencesNV(GLsizei n, const GLuint *fences)
{
    return gl::DeleteFencesNV(n, fences);
}

void GL_APIENTRY glGenFencesNV(GLsizei n, GLuint *fences)
{
    return gl::GenFencesNV(n, fences);
}

GLboolean GL_APIENTRY glIsFenceNV(GLuint fence)
{
    return gl::IsFenceNV(fence);
}

GLboolean GL_APIENTRY glTestFenceNV(GLuint fence)
{
    return gl::TestFenceNV(fence);
}

void GL_APIENTRY glGetFenceivNV(GLuint fence, GLenum pname, GLint *params)
{
    return gl::GetFenceivNV(fence, pname, params);
}

void GL_APIENTRY glFinishFenceNV(GLuint fence)
{
    return gl::FinishFenceNV(fence);
}

void GL_APIENTRY glSetFenceNV(GLuint fence, GLenum condition)
{
    return gl::SetFenceNV(fence, condition);
}

void GL_APIENTRY glGetTranslatedShaderSourceANGLE(GLuint shader,
                                                  GLsizei bufsize,
                                                  GLsizei *length,
                                                  GLchar *source)
{
    return gl::GetTranslatedShaderSourceANGLE(shader, bufsize, length, source);
}

void GL_APIENTRY glTexStorage2DEXT(GLenum target,
                                   GLsizei levels,
                                   GLenum internalformat,
                                   GLsizei width,
                                   GLsizei height)
{
    return gl::TexStorage2DEXT(target, levels, internalformat, width, height);
}

GLenum GL_APIENTRY glGetGraphicsResetStatusEXT(void)
{
    return gl::GetGraphicsResetStatusEXT();
}

void GL_APIENTRY glReadnPixelsEXT(GLint x,
                                  GLint y,
                                  GLsizei width,
                                  GLsizei height,
                                  GLenum format,
                                  GLenum type,
                                  GLsizei bufSize,
                                  void *data)
{
    return gl::ReadnPixelsEXT(x, y, width, height, format, type, bufSize, data);
}

void GL_APIENTRY glGetnUniformfvEXT(GLuint program, GLint location, GLsizei bufSize, float *params)
{
    return gl::GetnUniformfvEXT(program, location, bufSize, params);
}

void GL_APIENTRY glGetnUniformivEXT(GLuint program, GLint location, GLsizei bufSize, GLint *params)
{
    return gl::GetnUniformivEXT(program, location, bufSize, params);
}

void GL_APIENTRY glGenQueriesEXT(GLsizei n, GLuint *ids)
{
    return gl::GenQueriesEXT(n, ids);
}

void GL_APIENTRY glDeleteQueriesEXT(GLsizei n, const GLuint *ids)
{
    return gl::DeleteQueriesEXT(n, ids);
}

GLboolean GL_APIENTRY glIsQueryEXT(GLuint id)
{
    return gl::IsQueryEXT(id);
}

void GL_APIENTRY glBeginQueryEXT(GLenum target, GLuint id)
{
    return gl::BeginQueryEXT(target, id);
}

void GL_APIENTRY glEndQueryEXT(GLenum target)
{
    return gl::EndQueryEXT(target);
}

void GL_APIENTRY glQueryCounterEXT(GLuint id, GLenum target)
{
    return gl::QueryCounterEXT(id, target);
}

void GL_APIENTRY glGetQueryivEXT(GLenum target, GLenum pname, GLint *params)
{
    return gl::GetQueryivEXT(target, pname, params);
}

void GL_APIENTRY glGetQueryObjectivEXT(GLuint id, GLenum pname, GLint *params)
{
    return gl::GetQueryObjectivEXT(id, pname, params);
}

void GL_APIENTRY glGetQueryObjectuivEXT(GLuint id, GLenum pname, GLuint *params)
{
    return gl::GetQueryObjectuivEXT(id, pname, params);
}

void GL_APIENTRY glGetQueryObjecti64vEXT(GLuint id, GLenum pname, GLint64 *params)
{
    return gl::GetQueryObjecti64vEXT(id, pname, params);
}

void GL_APIENTRY glGetQueryObjectui64vEXT(GLuint id, GLenum pname, GLuint64 *params)
{
    return gl::GetQueryObjectui64vEXT(id, pname, params);
}

void GL_APIENTRY glDrawBuffersEXT(GLsizei n, const GLenum *bufs)
{
    return gl::DrawBuffersEXT(n, bufs);
}

void GL_APIENTRY glDrawArraysInstancedANGLE(GLenum mode,
                                            GLint first,
                                            GLsizei count,
                                            GLsizei primcount)
{
    return gl::DrawArraysInstancedANGLE(mode, first, count, primcount);
}

void GL_APIENTRY glDrawElementsInstancedANGLE(GLenum mode,
                                              GLsizei count,
                                              GLenum type,
                                              const void *indices,
                                              GLsizei primcount)
{
    return gl::DrawElementsInstancedANGLE(mode, count, type, indices, primcount);
}

void GL_APIENTRY glVertexAttribDivisorANGLE(GLuint index, GLuint divisor)
{
    return gl::VertexAttribDivisorANGLE(index, divisor);
}

void GL_APIENTRY glGetProgramBinaryOES(GLuint program,
                                       GLsizei bufSize,
                                       GLsizei *length,
                                       GLenum *binaryFormat,
                                       void *binary)
{
    return gl::GetProgramBinaryOES(program, bufSize, length, binaryFormat, binary);
}

void GL_APIENTRY glProgramBinaryOES(GLuint program,
                                    GLenum binaryFormat,
                                    const void *binary,
                                    GLint length)
{
    return gl::ProgramBinaryOES(program, binaryFormat, binary, length);
}

void *GL_APIENTRY glMapBufferOES(GLenum target, GLenum access)
{
    return gl::MapBufferOES(target, access);
}

GLboolean GL_APIENTRY glUnmapBufferOES(GLenum target)
{
    return gl::UnmapBufferOES(target);
}

void GL_APIENTRY glGetBufferPointervOES(GLenum target, GLenum pname, void **params)
{
    return gl::GetBufferPointervOES(target, pname, params);
}

void *GL_APIENTRY glMapBufferRangeEXT(GLenum target,
                                      GLintptr offset,
                                      GLsizeiptr length,
                                      GLbitfield access)
{
    return gl::MapBufferRangeEXT(target, offset, length, access);
}

void GL_APIENTRY glFlushMappedBufferRangeEXT(GLenum target, GLintptr offset, GLsizeiptr length)
{
    return gl::FlushMappedBufferRangeEXT(target, offset, length);
}

void GL_APIENTRY glInsertEventMarkerEXT(GLsizei length, const char *marker)
{
    return gl::InsertEventMarkerEXT(length, marker);
}

void GL_APIENTRY glPushGroupMarkerEXT(GLsizei length, const char *marker)
{
    return gl::PushGroupMarkerEXT(length, marker);
}

void GL_APIENTRY glPopGroupMarkerEXT()
{
    return gl::PopGroupMarkerEXT();
}

void GL_APIENTRY glEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image)
{
    return gl::EGLImageTargetTexture2DOES(target, image);
}

void GL_APIENTRY glEGLImageTargetRenderbufferStorageOES(GLenum target, GLeglImageOES image)
{
    return gl::EGLImageTargetRenderbufferStorageOES(target, image);
}

void GL_APIENTRY glBindVertexArrayOES(GLuint array)
{
    return gl::BindVertexArrayOES(array);
}

void GL_APIENTRY glDeleteVertexArraysOES(GLsizei n, const GLuint *arrays)
{
    return gl::DeleteVertexArraysOES(n, arrays);
}

void GL_APIENTRY glGenVertexArraysOES(GLsizei n, GLuint *arrays)
{
    return gl::GenVertexArraysOES(n, arrays);
}

GLboolean GL_APIENTRY glIsVertexArrayOES(GLuint array)
{
    return gl::IsVertexArrayOES(array);
}

void GL_APIENTRY glDebugMessageControlKHR(GLenum source,
                                          GLenum type,
                                          GLenum severity,
                                          GLsizei count,
                                          const GLuint *ids,
                                          GLboolean enabled)
{
    return gl::DebugMessageControlKHR(source, type, severity, count, ids, enabled);
}

void GL_APIENTRY glDebugMessageInsertKHR(GLenum source,
                                         GLenum type,
                                         GLuint id,
                                         GLenum severity,
                                         GLsizei length,
                                         const GLchar *buf)
{
    return gl::DebugMessageInsertKHR(source, type, id, severity, length, buf);
}

void GL_APIENTRY glDebugMessageCallbackKHR(GLDEBUGPROCKHR callback, const void *userParam)
{
    return gl::DebugMessageCallbackKHR(callback, userParam);
}

GLuint GL_APIENTRY glGetDebugMessageLogKHR(GLuint count,
                                           GLsizei bufSize,
                                           GLenum *sources,
                                           GLenum *types,
                                           GLuint *ids,
                                           GLenum *severities,
                                           GLsizei *lengths,
                                           GLchar *messageLog)
{
    return gl::GetDebugMessageLogKHR(count, bufSize, sources, types, ids, severities, lengths,
                                     messageLog);
}

void GL_APIENTRY glPushDebugGroupKHR(GLenum source,
                                     GLuint id,
                                     GLsizei length,
                                     const GLchar *message)
{
    return gl::PushDebugGroupKHR(source, id, length, message);
}

void GL_APIENTRY glPopDebugGroupKHR(void)
{
    return gl::PopDebugGroupKHR();
}

void GL_APIENTRY glObjectLabelKHR(GLenum identifier,
                                  GLuint name,
                                  GLsizei length,
                                  const GLchar *label)
{
    return gl::ObjectLabelKHR(identifier, name, length, label);
}

void GL_APIENTRY
glGetObjectLabelKHR(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    return gl::GetObjectLabelKHR(identifier, name, bufSize, length, label);
}

void GL_APIENTRY glObjectPtrLabelKHR(const void *ptr, GLsizei length, const GLchar *label)
{
    return gl::ObjectPtrLabelKHR(ptr, length, label);
}

void GL_APIENTRY glGetObjectPtrLabelKHR(const void *ptr,
                                        GLsizei bufSize,
                                        GLsizei *length,
                                        GLchar *label)
{
    return gl::GetObjectPtrLabelKHR(ptr, bufSize, length, label);
}

void GL_APIENTRY glGetPointervKHR(GLenum pname, void **params)
{
    return gl::GetPointervKHR(pname, params);
}

void GL_APIENTRY glBindUniformLocationCHROMIUM(GLuint program, GLint location, const GLchar *name)
{
    return gl::BindUniformLocationCHROMIUM(program, location, name);
}

void GL_APIENTRY glCoverageModulationCHROMIUM(GLenum components)
{
    return gl::CoverageModulationCHROMIUM(components);
}

// CHROMIUM_path_rendendering
void GL_APIENTRY glMatrixLoadfCHROMIUM(GLenum matrixMode, const GLfloat *matrix)
{
    gl::MatrixLoadfCHROMIUM(matrixMode, matrix);
}

void GL_APIENTRY glMatrixLoadIdentityCHROMIUM(GLenum matrixMode)
{
    gl::MatrixLoadIdentityCHROMIUM(matrixMode);
}

GLuint GL_APIENTRY glGenPathsCHROMIUM(GLsizei range)
{
    return gl::GenPathsCHROMIUM(range);
}

void GL_APIENTRY glDeletePathsCHROMIUM(GLuint first, GLsizei range)
{
    gl::DeletePathsCHROMIUM(first, range);
}

GLboolean GL_APIENTRY glIsPathCHROMIUM(GLuint path)
{
    return gl::IsPathCHROMIUM(path);
}

void GL_APIENTRY glPathCommandsCHROMIUM(GLuint path,
                                        GLsizei numCommands,
                                        const GLubyte *commands,
                                        GLsizei numCoords,
                                        GLenum coordType,
                                        const void *coords)
{
    gl::PathCommandsCHROMIUM(path, numCommands, commands, numCoords, coordType, coords);
}

void GL_APIENTRY glPathParameterfCHROMIUM(GLuint path, GLenum pname, GLfloat value)
{
    gl::PathParameterfCHROMIUM(path, pname, value);
}

void GL_APIENTRY glPathParameteriCHROMIUM(GLuint path, GLenum pname, GLint value)
{
    gl::PathParameteriCHROMIUM(path, pname, value);
}

void GL_APIENTRY glGetPathParameterfvCHROMIUM(GLuint path, GLenum pname, GLfloat *value)
{
    gl::GetPathParameterfvCHROMIUM(path, pname, value);
}

void GL_APIENTRY glGetPathParameterivCHROMIUM(GLuint path, GLenum pname, GLint *value)
{
    gl::GetPathParameterivCHROMIUM(path, pname, value);
}

void GL_APIENTRY glPathStencilFuncCHROMIUM(GLenum func, GLint ref, GLuint mask)
{
    gl::PathStencilFuncCHROMIUM(func, ref, mask);
}

void GL_APIENTRY glStencilFillPathCHROMIUM(GLuint path, GLenum fillMode, GLuint mask)
{
    gl::StencilFillPathCHROMIUM(path, fillMode, mask);
}

void GL_APIENTRY glStencilStrokePathCHROMIUM(GLuint path, GLint reference, GLuint mask)
{
    gl::StencilStrokePathCHROMIUM(path, reference, mask);
}

void GL_APIENTRY glCoverFillPathCHROMIUM(GLuint path, GLenum coverMode)
{
    gl::CoverFillPathCHROMIUM(path, coverMode);
}

void GL_APIENTRY glCoverStrokePathCHROMIUM(GLuint path, GLenum coverMode)
{
    gl::CoverStrokePathCHROMIUM(path, coverMode);
}

void GL_APIENTRY glStencilThenCoverFillPathCHROMIUM(GLuint path,
                                                    GLenum fillMode,
                                                    GLuint mask,
                                                    GLenum coverMode)
{
    gl::StencilThenCoverFillPathCHROMIUM(path, fillMode, mask, coverMode);
}

void GL_APIENTRY glStencilThenCoverStrokePathCHROMIUM(GLuint path,
                                                      GLint reference,
                                                      GLuint mask,
                                                      GLenum coverMode)
{
    gl::StencilThenCoverStrokePathCHROMIUM(path, reference, mask, coverMode);
}

void GL_APIENTRY glCoverFillPathInstancedCHROMIUM(GLsizei numPaths,
                                                  GLenum pathNameType,
                                                  const void *paths,
                                                  GLuint pathBase,
                                                  GLenum coverMode,
                                                  GLenum transformType,
                                                  const GLfloat *transformValues)
{
    gl::CoverFillPathInstancedCHROMIUM(numPaths, pathNameType, paths, pathBase, coverMode,
                                       transformType, transformValues);
}

void GL_APIENTRY glCoverStrokePathInstancedCHROMIUM(GLsizei numPaths,
                                                    GLenum pathNameType,
                                                    const void *paths,
                                                    GLuint pathBase,
                                                    GLenum coverMode,
                                                    GLenum transformType,
                                                    const GLfloat *transformValues)
{
    gl::CoverStrokePathInstancedCHROMIUM(numPaths, pathNameType, paths, pathBase, coverMode,
                                         transformType, transformValues);
}

void GL_APIENTRY glStencilFillPathInstancedCHROMIUM(GLsizei numPaths,
                                                    GLenum pathNameType,
                                                    const void *paths,
                                                    GLuint pathBase,
                                                    GLenum fillMode,
                                                    GLuint mask,
                                                    GLenum transformType,
                                                    const GLfloat *transformValues)
{
    gl::StencilFillPathInstancedCHROMIUM(numPaths, pathNameType, paths, pathBase, fillMode, mask,
                                         transformType, transformValues);
}

void GL_APIENTRY glStencilStrokePathInstancedCHROMIUM(GLsizei numPaths,
                                                      GLenum pathNameType,
                                                      const void *paths,
                                                      GLuint pathBase,
                                                      GLint reference,
                                                      GLuint mask,
                                                      GLenum transformType,
                                                      const GLfloat *transformValues)
{
    gl::StencilStrokePathInstancedCHROMIUM(numPaths, pathNameType, paths, pathBase, reference, mask,
                                           transformType, transformValues);
}

void GL_APIENTRY glStencilThenCoverFillPathInstancedCHROMIUM(GLsizei numPaths,
                                                             GLenum pathNameType,
                                                             const void *paths,
                                                             GLuint pathBase,
                                                             GLenum fillMode,
                                                             GLuint mask,
                                                             GLenum coverMode,
                                                             GLenum transformType,
                                                             const GLfloat *transformValues)
{
    gl::StencilThenCoverFillPathInstancedCHROMIUM(numPaths, pathNameType, paths, pathBase, fillMode,
                                                  mask, coverMode, transformType, transformValues);
}

void GL_APIENTRY glStencilThenCoverStrokePathInstancedCHROMIUM(GLsizei numPaths,
                                                               GLenum pathNameType,
                                                               const void *paths,
                                                               GLuint pathBase,
                                                               GLint reference,
                                                               GLuint mask,
                                                               GLenum coverMode,
                                                               GLenum transformType,
                                                               const GLfloat *transformValues)
{
    gl::StencilThenCoverStrokePathInstancedCHROMIUM(numPaths, pathNameType, paths, pathBase,
                                                    reference, mask, coverMode, transformType,
                                                    transformValues);
}

void GL_APIENTRY glBindFragmentInputLocationCHROMIUM(GLuint program,
                                                     GLint location,
                                                     const GLchar *name)
{
    gl::BindFragmentInputLocationCHROMIUM(program, location, name);
}

void GL_APIENTRY glProgramPathFragmentInputGenCHROMIUM(GLuint program,
                                                       GLint location,
                                                       GLenum genMode,
                                                       GLint components,
                                                       const GLfloat *coeffs)
{
    gl::ProgramPathFragmentInputGenCHROMIUM(program, location, genMode, components, coeffs);
}

// GLES 3.1
void GL_APIENTRY glDispatchCompute(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ)
{
    gl::DispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
}

void GL_APIENTRY glDispatchComputeIndirect(GLintptr indirect)
{
    gl::DispatchComputeIndirect(indirect);
}

void GL_APIENTRY glDrawArraysIndirect(GLenum mode, const void *indirect)
{
    gl::DrawArraysIndirect(mode, indirect);
}

void GL_APIENTRY glDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect)
{
    gl::DrawElementsIndirect(mode, type, indirect);
}

void GL_APIENTRY glFramebufferParameteri(GLenum target, GLenum pname, GLint param)
{
    gl::FramebufferParameteri(target, pname, param);
}

void GL_APIENTRY glGetFramebufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    gl::GetFramebufferParameteriv(target, pname, params);
}

void GL_APIENTRY glGetProgramInterfaceiv(GLuint program,
                                         GLenum programInterface,
                                         GLenum pname,
                                         GLint *params)
{
    gl::GetProgramInterfaceiv(program, programInterface, pname, params);
}

GLuint GL_APIENTRY glGetProgramResourceIndex(GLuint program,
                                             GLenum programInterface,
                                             const GLchar *name)
{
    return gl::GetProgramResourceIndex(program, programInterface, name);
}

void GL_APIENTRY glGetProgramResourceName(GLuint program,
                                          GLenum programInterface,
                                          GLuint index,
                                          GLsizei bufSize,
                                          GLsizei *length,
                                          GLchar *name)
{
    gl::GetProgramResourceName(program, programInterface, index, bufSize, length, name);
}

void GL_APIENTRY glGetProgramResourceiv(GLuint program,
                                        GLenum programInterface,
                                        GLuint index,
                                        GLsizei propCount,
                                        const GLenum *props,
                                        GLsizei bufSize,
                                        GLsizei *length,
                                        GLint *params)
{
    gl::GetProgramResourceiv(program, programInterface, index, propCount, props, bufSize, length,
                             params);
}

GLint GL_APIENTRY glGetProgramResourceLocation(GLuint program,
                                               GLenum programInterface,
                                               const GLchar *name)
{
    return gl::GetProgramResourceLocation(program, programInterface, name);
}

void GL_APIENTRY glUseProgramStages(GLuint pipeline, GLbitfield stages, GLuint program)
{
    gl::UseProgramStages(pipeline, stages, program);
}

void GL_APIENTRY glActiveShaderProgram(GLuint pipeline, GLuint program)
{
    gl::ActiveShaderProgram(pipeline, program);
}

GLuint GL_APIENTRY glCreateShaderProgramv(GLenum type, GLsizei count, const GLchar *const *strings)
{
    return gl::CreateShaderProgramv(type, count, strings);
}

void GL_APIENTRY glBindProgramPipeline(GLuint pipeline)
{
    gl::BindProgramPipeline(pipeline);
}

void GL_APIENTRY glDeleteProgramPipelines(GLsizei n, const GLuint *pipelines)
{
    gl::DeleteProgramPipelines(n, pipelines);
}

void GL_APIENTRY glGenProgramPipelines(GLsizei n, GLuint *pipelines)
{
    gl::GenProgramPipelines(n, pipelines);
}

GLboolean GL_APIENTRY glIsProgramPipeline(GLuint pipeline)
{
    return gl::IsProgramPipeline(pipeline);
}

void GL_APIENTRY glGetProgramPipelineiv(GLuint pipeline, GLenum pname, GLint *params)
{
    gl::GetProgramPipelineiv(pipeline, pname, params);
}

void GL_APIENTRY glProgramUniform1i(GLuint program, GLint location, GLint v0)
{
    gl::ProgramUniform1i(program, location, v0);
}

void GL_APIENTRY glProgramUniform2i(GLuint program, GLint location, GLint v0, GLint v1)
{
    gl::ProgramUniform2i(program, location, v0, v1);
}

void GL_APIENTRY glProgramUniform3i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
    gl::ProgramUniform3i(program, location, v0, v1, v2);
}

void GL_APIENTRY
glProgramUniform4i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    gl::ProgramUniform4i(program, location, v0, v1, v2, v3);
}

void GL_APIENTRY glProgramUniform1ui(GLuint program, GLint location, GLuint v0)
{
    gl::ProgramUniform1ui(program, location, v0);
}

void GL_APIENTRY glProgramUniform2ui(GLuint program, GLint location, GLuint v0, GLuint v1)
{
    gl::ProgramUniform2ui(program, location, v0, v1);
}

void GL_APIENTRY
glProgramUniform3ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    gl::ProgramUniform3ui(program, location, v0, v1, v2);
}

void GL_APIENTRY
glProgramUniform4ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    gl::ProgramUniform4ui(program, location, v0, v1, v2, v3);
}

void GL_APIENTRY glProgramUniform1f(GLuint program, GLint location, GLfloat v0)
{
    gl::ProgramUniform1f(program, location, v0);
}

void GL_APIENTRY glProgramUniform2f(GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
    gl::ProgramUniform2f(program, location, v0, v1);
}

void GL_APIENTRY
glProgramUniform3f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    gl::ProgramUniform3f(program, location, v0, v1, v2);
}

void GL_APIENTRY
glProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    gl::ProgramUniform4f(program, location, v0, v1, v2, v3);
}

void GL_APIENTRY glProgramUniform1iv(GLuint program,
                                     GLint location,
                                     GLsizei count,
                                     const GLint *value)
{
    gl::ProgramUniform1iv(program, location, count, value);
}

void GL_APIENTRY glProgramUniform2iv(GLuint program,
                                     GLint location,
                                     GLsizei count,
                                     const GLint *value)
{
    gl::ProgramUniform2iv(program, location, count, value);
}

void GL_APIENTRY glProgramUniform3iv(GLuint program,
                                     GLint location,
                                     GLsizei count,
                                     const GLint *value)
{
    gl::ProgramUniform3iv(program, location, count, value);
}

void GL_APIENTRY glProgramUniform4iv(GLuint program,
                                     GLint location,
                                     GLsizei count,
                                     const GLint *value)
{
    gl::ProgramUniform4iv(program, location, count, value);
}

void GL_APIENTRY glProgramUniform1uiv(GLuint program,
                                      GLint location,
                                      GLsizei count,
                                      const GLuint *value)
{
    gl::ProgramUniform1uiv(program, location, count, value);
}

void GL_APIENTRY glProgramUniform2uiv(GLuint program,
                                      GLint location,
                                      GLsizei count,
                                      const GLuint *value)
{
    gl::ProgramUniform2uiv(program, location, count, value);
}

void GL_APIENTRY glProgramUniform3uiv(GLuint program,
                                      GLint location,
                                      GLsizei count,
                                      const GLuint *value)
{
    gl::ProgramUniform3uiv(program, location, count, value);
}

void GL_APIENTRY glProgramUniform4uiv(GLuint program,
                                      GLint location,
                                      GLsizei count,
                                      const GLuint *value)
{
    gl::ProgramUniform4uiv(program, location, count, value);
}

void GL_APIENTRY glProgramUniform1fv(GLuint program,
                                     GLint location,
                                     GLsizei count,
                                     const GLfloat *value)
{
    gl::ProgramUniform1fv(program, location, count, value);
}

void GL_APIENTRY glProgramUniform2fv(GLuint program,
                                     GLint location,
                                     GLsizei count,
                                     const GLfloat *value)
{
    gl::ProgramUniform2fv(program, location, count, value);
}

void GL_APIENTRY glProgramUniform3fv(GLuint program,
                                     GLint location,
                                     GLsizei count,
                                     const GLfloat *value)
{
    gl::ProgramUniform3fv(program, location, count, value);
}

void GL_APIENTRY glProgramUniform4fv(GLuint program,
                                     GLint location,
                                     GLsizei count,
                                     const GLfloat *value)
{
    gl::ProgramUniform4fv(program, location, count, value);
}

void GL_APIENTRY glProgramUniformMatrix2fv(GLuint program,
                                           GLint location,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value)
{
    gl::ProgramUniformMatrix2fv(program, location, count, transpose, value);
}

void GL_APIENTRY glProgramUniformMatrix3fv(GLuint program,
                                           GLint location,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value)
{
    gl::ProgramUniformMatrix3fv(program, location, count, transpose, value);
}

void GL_APIENTRY glProgramUniformMatrix4fv(GLuint program,
                                           GLint location,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value)
{
    gl::ProgramUniformMatrix4fv(program, location, count, transpose, value);
}

void GL_APIENTRY glProgramUniformMatrix2x3fv(GLuint program,
                                             GLint location,
                                             GLsizei count,
                                             GLboolean transpose,
                                             const GLfloat *value)
{
    gl::ProgramUniformMatrix2x3fv(program, location, count, transpose, value);
}

void GL_APIENTRY glProgramUniformMatrix3x2fv(GLuint program,
                                             GLint location,
                                             GLsizei count,
                                             GLboolean transpose,
                                             const GLfloat *value)
{
    gl::ProgramUniformMatrix3x2fv(program, location, count, transpose, value);
}

void GL_APIENTRY glProgramUniformMatrix2x4fv(GLuint program,
                                             GLint location,
                                             GLsizei count,
                                             GLboolean transpose,
                                             const GLfloat *value)
{
    gl::ProgramUniformMatrix2x4fv(program, location, count, transpose, value);
}

void GL_APIENTRY glProgramUniformMatrix4x2fv(GLuint program,
                                             GLint location,
                                             GLsizei count,
                                             GLboolean transpose,
                                             const GLfloat *value)
{
    gl::ProgramUniformMatrix4x2fv(program, location, count, transpose, value);
}

void GL_APIENTRY glProgramUniformMatrix3x4fv(GLuint program,
                                             GLint location,
                                             GLsizei count,
                                             GLboolean transpose,
                                             const GLfloat *value)
{
    gl::ProgramUniformMatrix3x4fv(program, location, count, transpose, value);
}

void GL_APIENTRY glProgramUniformMatrix4x3fv(GLuint program,
                                             GLint location,
                                             GLsizei count,
                                             GLboolean transpose,
                                             const GLfloat *value)
{
    gl::ProgramUniformMatrix4x3fv(program, location, count, transpose, value);
}

void GL_APIENTRY glValidateProgramPipeline(GLuint pipeline)
{
    gl::ValidateProgramPipeline(pipeline);
}

void GL_APIENTRY glGetProgramPipelineInfoLog(GLuint pipeline,
                                             GLsizei bufSize,
                                             GLsizei *length,
                                             GLchar *infoLog)
{
    gl::GetProgramPipelineInfoLog(pipeline, bufSize, length, infoLog);
}

void GL_APIENTRY glBindImageTexture(GLuint unit,
                                    GLuint texture,
                                    GLint level,
                                    GLboolean layered,
                                    GLint layer,
                                    GLenum access,
                                    GLenum format)
{
    gl::BindImageTexture(unit, texture, level, layered, layer, access, format);
}

void GL_APIENTRY glGetBooleani_v(GLenum target, GLuint index, GLboolean *data)
{
    gl::GetBooleani_v(target, index, data);
}

void GL_APIENTRY glMemoryBarrier(GLbitfield barriers)
{
    gl::MemoryBarrier(barriers);
}

void GL_APIENTRY glMemoryBarrierByRegion(GLbitfield barriers)
{
    gl::MemoryBarrierByRegion(barriers);
}

void GL_APIENTRY glTexStorage2DMultisample(GLenum target,
                                           GLsizei samples,
                                           GLenum internalformat,
                                           GLsizei width,
                                           GLsizei height,
                                           GLboolean fixedsamplelocations)
{
    gl::TexStorage2DMultisample(target, samples, internalformat, width, height,
                                fixedsamplelocations);
}

void GL_APIENTRY glGetMultisamplefv(GLenum pname, GLuint index, GLfloat *val)
{
    gl::GetMultisamplefv(pname, index, val);
}

void GL_APIENTRY glSampleMaski(GLuint maskNumber, GLbitfield mask)
{
    gl::SampleMaski(maskNumber, mask);
}

void GL_APIENTRY glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params)
{
    gl::GetTexLevelParameteriv(target, level, pname, params);
}

void GL_APIENTRY glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params)
{
    gl::GetTexLevelParameterfv(target, level, pname, params);
}

void GL_APIENTRY glBindVertexBuffer(GLuint bindingindex,
                                    GLuint buffer,
                                    GLintptr offset,
                                    GLsizei stride)
{
    gl::BindVertexBuffer(bindingindex, buffer, offset, stride);
}

void GL_APIENTRY glVertexAttribFormat(GLuint attribindex,
                                      GLint size,
                                      GLenum type,
                                      GLboolean normalized,
                                      GLuint relativeoffset)
{
    gl::VertexAttribFormat(attribindex, size, type, normalized, relativeoffset);
}

void GL_APIENTRY glVertexAttribIFormat(GLuint attribindex,
                                       GLint size,
                                       GLenum type,
                                       GLuint relativeoffset)
{
    gl::VertexAttribIFormat(attribindex, size, type, relativeoffset);
}

void GL_APIENTRY glVertexAttribBinding(GLuint attribindex, GLuint bindingindex)
{
    gl::VertexAttribBinding(attribindex, bindingindex);
}

void GL_APIENTRY glVertexBindingDivisor(GLuint bindingindex, GLuint divisor)
{
    gl::VertexBindingDivisor(bindingindex, divisor);
}

void GL_APIENTRY glFramebufferTextureMultiviewLayeredANGLE(GLenum target,
                                                           GLenum attachment,
                                                           GLuint texture,
                                                           GLint level,
                                                           GLint baseViewIndex,
                                                           GLsizei numViews)
{
    gl::FramebufferTextureMultiviewLayeredANGLE(target, attachment, texture, level, baseViewIndex,
                                                numViews);
}

void GL_APIENTRY glFramebufferTextureMultiviewSideBySideANGLE(GLenum target,
                                                              GLenum attachment,
                                                              GLuint texture,
                                                              GLint level,
                                                              GLsizei numViews,
                                                              const GLint *viewportOffsets)
{
    gl::FramebufferTextureMultiviewSideBySideANGLE(target, attachment, texture, level, numViews,
                                                   viewportOffsets);
}

void GL_APIENTRY glRequestExtensionANGLE(const GLchar *name)
{
    gl::RequestExtensionANGLE(name);
}

#ifdef EGL_ANGLE_explicit_context

void GL_APIENTRY glActiveTextureContextANGLE(GLeglContext context, GLenum texture)
{
    return gl::ActiveTextureContextANGLE(context, texture);
}

void GL_APIENTRY glAttachShaderContextANGLE(GLeglContext context, GLuint program, GLuint shader)
{
    return gl::AttachShaderContextANGLE(context, program, shader);
}

void GL_APIENTRY glBindAttribLocationContextANGLE(GLeglContext context,
                                                  GLuint program,
                                                  GLuint index,
                                                  const GLchar *name)
{
    return gl::BindAttribLocationContextANGLE(context, program, index, name);
}

void GL_APIENTRY glBindBufferContextANGLE(GLeglContext context, GLenum target, GLuint buffer)
{
    return gl::BindBufferContextANGLE(context, target, buffer);
}

void GL_APIENTRY glBindFramebufferContextANGLE(GLeglContext context,
                                               GLenum target,
                                               GLuint framebuffer)
{
    return gl::BindFramebufferContextANGLE(context, target, framebuffer);
}

void GL_APIENTRY glBindRenderbufferContextANGLE(GLeglContext context,
                                                GLenum target,
                                                GLuint renderbuffer)
{
    return gl::BindRenderbufferContextANGLE(context, target, renderbuffer);
}

void GL_APIENTRY glBindTextureContextANGLE(GLeglContext context, GLenum target, GLuint texture)
{
    return gl::BindTextureContextANGLE(context, target, texture);
}

void GL_APIENTRY glBlendColorContextANGLE(GLeglContext context,
                                          GLfloat red,
                                          GLfloat green,
                                          GLfloat blue,
                                          GLfloat alpha)
{
    return gl::BlendColorContextANGLE(context, red, green, blue, alpha);
}

void GL_APIENTRY glBlendEquationContextANGLE(GLeglContext context, GLenum mode)
{
    return gl::BlendEquationContextANGLE(context, mode);
}

void GL_APIENTRY glBlendEquationSeparateContextANGLE(GLeglContext context,
                                                     GLenum modeRGB,
                                                     GLenum modeAlpha)
{
    return gl::BlendEquationSeparateContextANGLE(context, modeRGB, modeAlpha);
}

void GL_APIENTRY glBlendFuncContextANGLE(GLeglContext context, GLenum sfactor, GLenum dfactor)
{
    return gl::BlendFuncContextANGLE(context, sfactor, dfactor);
}

void GL_APIENTRY glBlendFuncSeparateContextANGLE(GLeglContext context,
                                                 GLenum srcRGB,
                                                 GLenum dstRGB,
                                                 GLenum srcAlpha,
                                                 GLenum dstAlpha)
{
    return gl::BlendFuncSeparateContextANGLE(context, srcRGB, dstRGB, srcAlpha, dstAlpha);
}

void GL_APIENTRY glBufferDataContextANGLE(GLeglContext context,
                                          GLenum target,
                                          GLsizeiptr size,
                                          const void *data,
                                          GLenum usage)
{
    return gl::BufferDataContextANGLE(context, target, size, data, usage);
}

void GL_APIENTRY glBufferSubDataContextANGLE(GLeglContext context,
                                             GLenum target,
                                             GLintptr offset,
                                             GLsizeiptr size,
                                             const void *data)
{
    return gl::BufferSubDataContextANGLE(context, target, offset, size, data);
}

GLenum GL_APIENTRY glCheckFramebufferStatusContextANGLE(GLeglContext context, GLenum target)
{
    return gl::CheckFramebufferStatusContextANGLE(context, target);
}

void GL_APIENTRY glClearContextANGLE(GLeglContext context, GLbitfield mask)
{
    return gl::ClearContextANGLE(context, mask);
}

void GL_APIENTRY glClearColorContextANGLE(GLeglContext context,
                                          GLfloat red,
                                          GLfloat green,
                                          GLfloat blue,
                                          GLfloat alpha)
{
    return gl::ClearColorContextANGLE(context, red, green, blue, alpha);
}

void GL_APIENTRY glClearDepthfContextANGLE(GLeglContext context, GLfloat depth)
{
    return gl::ClearDepthfContextANGLE(context, depth);
}

void GL_APIENTRY glClearStencilContextANGLE(GLeglContext context, GLint s)
{
    return gl::ClearStencilContextANGLE(context, s);
}

void GL_APIENTRY glColorMaskContextANGLE(GLeglContext context,
                                         GLboolean red,
                                         GLboolean green,
                                         GLboolean blue,
                                         GLboolean alpha)
{
    return gl::ColorMaskContextANGLE(context, red, green, blue, alpha);
}

void GL_APIENTRY glCompileShaderContextANGLE(GLeglContext context, GLuint shader)
{
    return gl::CompileShaderContextANGLE(context, shader);
}

void GL_APIENTRY glCompressedTexImage2DContextANGLE(GLeglContext context,
                                                    GLenum target,
                                                    GLint level,
                                                    GLenum internalformat,
                                                    GLsizei width,
                                                    GLsizei height,
                                                    GLint border,
                                                    GLsizei imageSize,
                                                    const void *data)
{
    return gl::CompressedTexImage2DContextANGLE(context, target, level, internalformat, width,
                                                height, border, imageSize, data);
}

void GL_APIENTRY glCompressedTexSubImage2DContextANGLE(GLeglContext context,
                                                       GLenum target,
                                                       GLint level,
                                                       GLint xoffset,
                                                       GLint yoffset,
                                                       GLsizei width,
                                                       GLsizei height,
                                                       GLenum format,
                                                       GLsizei imageSize,
                                                       const void *data)
{
    return gl::CompressedTexSubImage2DContextANGLE(context, target, level, xoffset, yoffset, width,
                                                   height, format, imageSize, data);
}

void GL_APIENTRY glCopyTexImage2DContextANGLE(GLeglContext context,
                                              GLenum target,
                                              GLint level,
                                              GLenum internalformat,
                                              GLint x,
                                              GLint y,
                                              GLsizei width,
                                              GLsizei height,
                                              GLint border)
{
    return gl::CopyTexImage2DContextANGLE(context, target, level, internalformat, x, y, width,
                                          height, border);
}

void GL_APIENTRY glCopyTexSubImage2DContextANGLE(GLeglContext context,
                                                 GLenum target,
                                                 GLint level,
                                                 GLint xoffset,
                                                 GLint yoffset,
                                                 GLint x,
                                                 GLint y,
                                                 GLsizei width,
                                                 GLsizei height)
{
    return gl::CopyTexSubImage2DContextANGLE(context, target, level, xoffset, yoffset, x, y, width,
                                             height);
}

GLuint GL_APIENTRY glCreateProgramContextANGLE(GLeglContext context)
{
    return gl::CreateProgramContextANGLE(context);
}

GLuint GL_APIENTRY glCreateShaderContextANGLE(GLeglContext context, GLenum type)
{
    return gl::CreateShaderContextANGLE(context, type);
}

void GL_APIENTRY glCullFaceContextANGLE(GLeglContext context, GLenum mode)
{
    return gl::CullFaceContextANGLE(context, mode);
}

void GL_APIENTRY glDeleteBuffersContextANGLE(GLeglContext context, GLsizei n, const GLuint *buffers)
{
    return gl::DeleteBuffersContextANGLE(context, n, buffers);
}

void GL_APIENTRY glDeleteFramebuffersContextANGLE(GLeglContext context,
                                                  GLsizei n,
                                                  const GLuint *framebuffers)
{
    return gl::DeleteFramebuffersContextANGLE(context, n, framebuffers);
}

void GL_APIENTRY glDeleteProgramContextANGLE(GLeglContext context, GLuint program)
{
    return gl::DeleteProgramContextANGLE(context, program);
}

void GL_APIENTRY glDeleteRenderbuffersContextANGLE(GLeglContext context,
                                                   GLsizei n,
                                                   const GLuint *renderbuffers)
{
    return gl::DeleteRenderbuffersContextANGLE(context, n, renderbuffers);
}

void GL_APIENTRY glDeleteShaderContextANGLE(GLeglContext context, GLuint shader)
{
    return gl::DeleteShaderContextANGLE(context, shader);
}

void GL_APIENTRY glDeleteTexturesContextANGLE(GLeglContext context,
                                              GLsizei n,
                                              const GLuint *textures)
{
    return gl::DeleteTexturesContextANGLE(context, n, textures);
}

void GL_APIENTRY glDepthFuncContextANGLE(GLeglContext context, GLenum func)
{
    return gl::DepthFuncContextANGLE(context, func);
}

void GL_APIENTRY glDepthMaskContextANGLE(GLeglContext context, GLboolean flag)
{
    return gl::DepthMaskContextANGLE(context, flag);
}

void GL_APIENTRY glDepthRangefContextANGLE(GLeglContext context, GLfloat n, GLfloat f)
{
    return gl::DepthRangefContextANGLE(context, n, f);
}

void GL_APIENTRY glDetachShaderContextANGLE(GLeglContext context, GLuint program, GLuint shader)
{
    return gl::DetachShaderContextANGLE(context, program, shader);
}

void GL_APIENTRY glDisableContextANGLE(GLeglContext context, GLenum cap)
{
    return gl::DisableContextANGLE(context, cap);
}

void GL_APIENTRY glDisableVertexAttribArrayContextANGLE(GLeglContext context, GLuint index)
{
    return gl::DisableVertexAttribArrayContextANGLE(context, index);
}

void GL_APIENTRY glDrawArraysContextANGLE(GLeglContext context,
                                          GLenum mode,
                                          GLint first,
                                          GLsizei count)
{
    return gl::DrawArraysContextANGLE(context, mode, first, count);
}

void GL_APIENTRY glDrawElementsContextANGLE(GLeglContext context,
                                            GLenum mode,
                                            GLsizei count,
                                            GLenum type,
                                            const void *indices)
{
    return gl::DrawElementsContextANGLE(context, mode, count, type, indices);
}

void GL_APIENTRY glEnableContextANGLE(GLeglContext context, GLenum cap)
{
    return gl::EnableContextANGLE(context, cap);
}

void GL_APIENTRY glEnableVertexAttribArrayContextANGLE(GLeglContext context, GLuint index)
{
    return gl::EnableVertexAttribArrayContextANGLE(context, index);
}

void GL_APIENTRY glFinishContextANGLE(GLeglContext context)
{
    return gl::FinishContextANGLE(context);
}

void GL_APIENTRY glFlushContextANGLE(GLeglContext context)
{
    return gl::FlushContextANGLE(context);
}

void GL_APIENTRY glFramebufferRenderbufferContextANGLE(GLeglContext context,
                                                       GLenum target,
                                                       GLenum attachment,
                                                       GLenum renderbuffertarget,
                                                       GLuint renderbuffer)
{
    return gl::FramebufferRenderbufferContextANGLE(context, target, attachment, renderbuffertarget,
                                                   renderbuffer);
}

void GL_APIENTRY glFramebufferTexture2DContextANGLE(GLeglContext context,
                                                    GLenum target,
                                                    GLenum attachment,
                                                    GLenum textarget,
                                                    GLuint texture,
                                                    GLint level)
{
    return gl::FramebufferTexture2DContextANGLE(context, target, attachment, textarget, texture,
                                                level);
}

void GL_APIENTRY glFrontFaceContextANGLE(GLeglContext context, GLenum mode)
{
    return gl::FrontFaceContextANGLE(context, mode);
}

void GL_APIENTRY glGenBuffersContextANGLE(GLeglContext context, GLsizei n, GLuint *buffers)
{
    return gl::GenBuffersContextANGLE(context, n, buffers);
}

void GL_APIENTRY glGenerateMipmapContextANGLE(GLeglContext context, GLenum target)
{
    return gl::GenerateMipmapContextANGLE(context, target);
}

void GL_APIENTRY glGenFramebuffersContextANGLE(GLeglContext context,
                                               GLsizei n,
                                               GLuint *framebuffers)
{
    return gl::GenFramebuffersContextANGLE(context, n, framebuffers);
}

void GL_APIENTRY glGenRenderbuffersContextANGLE(GLeglContext context,
                                                GLsizei n,
                                                GLuint *renderbuffers)
{
    return gl::GenRenderbuffersContextANGLE(context, n, renderbuffers);
}

void GL_APIENTRY glGenTexturesContextANGLE(GLeglContext context, GLsizei n, GLuint *textures)
{
    return gl::GenTexturesContextANGLE(context, n, textures);
}

void GL_APIENTRY glGetActiveAttribContextANGLE(GLeglContext context,
                                               GLuint program,
                                               GLuint index,
                                               GLsizei bufsize,
                                               GLsizei *length,
                                               GLint *size,
                                               GLenum *type,
                                               GLchar *name)
{
    return gl::GetActiveAttribContextANGLE(context, program, index, bufsize, length, size, type,
                                           name);
}

void GL_APIENTRY glGetActiveUniformContextANGLE(GLeglContext context,
                                                GLuint program,
                                                GLuint index,
                                                GLsizei bufsize,
                                                GLsizei *length,
                                                GLint *size,
                                                GLenum *type,
                                                GLchar *name)
{
    return gl::GetActiveUniformContextANGLE(context, program, index, bufsize, length, size, type,
                                            name);
}

void GL_APIENTRY glGetAttachedShadersContextANGLE(GLeglContext context,
                                                  GLuint program,
                                                  GLsizei maxcount,
                                                  GLsizei *count,
                                                  GLuint *shaders)
{
    return gl::GetAttachedShadersContextANGLE(context, program, maxcount, count, shaders);
}

GLint GL_APIENTRY glGetAttribLocationContextANGLE(GLeglContext context,
                                                  GLuint program,
                                                  const GLchar *name)
{
    return gl::GetAttribLocationContextANGLE(context, program, name);
}

void GL_APIENTRY glGetBooleanvContextANGLE(GLeglContext context, GLenum pname, GLboolean *params)
{
    return gl::GetBooleanvContextANGLE(context, pname, params);
}

void GL_APIENTRY glGetBufferParameterivContextANGLE(GLeglContext context,
                                                    GLenum target,
                                                    GLenum pname,
                                                    GLint *params)
{
    return gl::GetBufferParameterivContextANGLE(context, target, pname, params);
}

GLenum GL_APIENTRY glGetErrorContextANGLE(GLeglContext context)
{
    return gl::GetErrorContextANGLE(context);
}

void GL_APIENTRY glGetFloatvContextANGLE(GLeglContext context, GLenum pname, GLfloat *params)
{
    return gl::GetFloatvContextANGLE(context, pname, params);
}

void GL_APIENTRY glGetFramebufferAttachmentParameterivContextANGLE(GLeglContext context,
                                                                   GLenum target,
                                                                   GLenum attachment,
                                                                   GLenum pname,
                                                                   GLint *params)
{
    return gl::GetFramebufferAttachmentParameterivContextANGLE(context, target, attachment, pname,
                                                               params);
}

void GL_APIENTRY glGetIntegervContextANGLE(GLeglContext context, GLenum pname, GLint *params)
{
    return gl::GetIntegervContextANGLE(context, pname, params);
}

void GL_APIENTRY glGetProgramivContextANGLE(GLeglContext context,
                                            GLuint program,
                                            GLenum pname,
                                            GLint *params)
{
    return gl::GetProgramivContextANGLE(context, program, pname, params);
}

void GL_APIENTRY glGetProgramInfoLogContextANGLE(GLeglContext context,
                                                 GLuint program,
                                                 GLsizei bufsize,
                                                 GLsizei *length,
                                                 GLchar *infolog)
{
    return gl::GetProgramInfoLogContextANGLE(context, program, bufsize, length, infolog);
}

void GL_APIENTRY glGetRenderbufferParameterivContextANGLE(GLeglContext context,
                                                          GLenum target,
                                                          GLenum pname,
                                                          GLint *params)
{
    return gl::GetRenderbufferParameterivContextANGLE(context, target, pname, params);
}

void GL_APIENTRY glGetShaderivContextANGLE(GLeglContext context,
                                           GLuint shader,
                                           GLenum pname,
                                           GLint *params)
{
    return gl::GetShaderivContextANGLE(context, shader, pname, params);
}

void GL_APIENTRY glGetShaderInfoLogContextANGLE(GLeglContext context,
                                                GLuint shader,
                                                GLsizei bufsize,
                                                GLsizei *length,
                                                GLchar *infolog)
{
    return gl::GetShaderInfoLogContextANGLE(context, shader, bufsize, length, infolog);
}

void GL_APIENTRY glGetShaderPrecisionFormatContextANGLE(GLeglContext context,
                                                        GLenum shadertype,
                                                        GLenum precisiontype,
                                                        GLint *range,
                                                        GLint *precision)
{
    return gl::GetShaderPrecisionFormatContextANGLE(context, shadertype, precisiontype, range,
                                                    precision);
}

void GL_APIENTRY glGetShaderSourceContextANGLE(GLeglContext context,
                                               GLuint shader,
                                               GLsizei bufsize,
                                               GLsizei *length,
                                               GLchar *source)
{
    return gl::GetShaderSourceContextANGLE(context, shader, bufsize, length, source);
}

const GLubyte *GL_APIENTRY glGetStringContextANGLE(GLeglContext context, GLenum name)
{
    return gl::GetStringContextANGLE(context, name);
}

void GL_APIENTRY glGetTexParameterfvContextANGLE(GLeglContext context,
                                                 GLenum target,
                                                 GLenum pname,
                                                 GLfloat *params)
{
    return gl::GetTexParameterfvContextANGLE(context, target, pname, params);
}

void GL_APIENTRY glGetTexParameterivContextANGLE(GLeglContext context,
                                                 GLenum target,
                                                 GLenum pname,
                                                 GLint *params)
{
    return gl::GetTexParameterivContextANGLE(context, target, pname, params);
}

void GL_APIENTRY glGetUniformfvContextANGLE(GLeglContext context,
                                            GLuint program,
                                            GLint location,
                                            GLfloat *params)
{
    return gl::GetUniformfvContextANGLE(context, program, location, params);
}

void GL_APIENTRY glGetUniformivContextANGLE(GLeglContext context,
                                            GLuint program,
                                            GLint location,
                                            GLint *params)
{
    return gl::GetUniformivContextANGLE(context, program, location, params);
}

GLint GL_APIENTRY glGetUniformLocationContextANGLE(GLeglContext context,
                                                   GLuint program,
                                                   const GLchar *name)
{
    return gl::GetUniformLocationContextANGLE(context, program, name);
}

void GL_APIENTRY glGetVertexAttribfvContextANGLE(GLeglContext context,
                                                 GLuint index,
                                                 GLenum pname,
                                                 GLfloat *params)
{
    return gl::GetVertexAttribfvContextANGLE(context, index, pname, params);
}

void GL_APIENTRY glGetVertexAttribivContextANGLE(GLeglContext context,
                                                 GLuint index,
                                                 GLenum pname,
                                                 GLint *params)
{
    return gl::GetVertexAttribivContextANGLE(context, index, pname, params);
}

void GL_APIENTRY glGetVertexAttribPointervContextANGLE(GLeglContext context,
                                                       GLuint index,
                                                       GLenum pname,
                                                       void **pointer)
{
    return gl::GetVertexAttribPointervContextANGLE(context, index, pname, pointer);
}

void GL_APIENTRY glHintContextANGLE(GLeglContext context, GLenum target, GLenum mode)
{
    return gl::HintContextANGLE(context, target, mode);
}

GLboolean GL_APIENTRY glIsBufferContextANGLE(GLeglContext context, GLuint buffer)
{
    return gl::IsBufferContextANGLE(context, buffer);
}

GLboolean GL_APIENTRY glIsEnabledContextANGLE(GLeglContext context, GLenum cap)
{
    return gl::IsEnabledContextANGLE(context, cap);
}

GLboolean GL_APIENTRY glIsFramebufferContextANGLE(GLeglContext context, GLuint framebuffer)
{
    return gl::IsFramebufferContextANGLE(context, framebuffer);
}

GLboolean GL_APIENTRY glIsProgramContextANGLE(GLeglContext context, GLuint program)
{
    return gl::IsProgramContextANGLE(context, program);
}

GLboolean GL_APIENTRY glIsRenderbufferContextANGLE(GLeglContext context, GLuint renderbuffer)
{
    return gl::IsRenderbufferContextANGLE(context, renderbuffer);
}

GLboolean GL_APIENTRY glIsShaderContextANGLE(GLeglContext context, GLuint shader)
{
    return gl::IsShaderContextANGLE(context, shader);
}

GLboolean GL_APIENTRY glIsTextureContextANGLE(GLeglContext context, GLuint texture)
{
    return gl::IsTextureContextANGLE(context, texture);
}

void GL_APIENTRY glLineWidthContextANGLE(GLeglContext context, GLfloat width)
{
    return gl::LineWidthContextANGLE(context, width);
}

void GL_APIENTRY glLinkProgramContextANGLE(GLeglContext context, GLuint program)
{
    return gl::LinkProgramContextANGLE(context, program);
}

void GL_APIENTRY glPixelStoreiContextANGLE(GLeglContext context, GLenum pname, GLint param)
{
    return gl::PixelStoreiContextANGLE(context, pname, param);
}

void GL_APIENTRY glPolygonOffsetContextANGLE(GLeglContext context, GLfloat factor, GLfloat units)
{
    return gl::PolygonOffsetContextANGLE(context, factor, units);
}

void GL_APIENTRY glReadPixelsContextANGLE(GLeglContext context,
                                          GLint x,
                                          GLint y,
                                          GLsizei width,
                                          GLsizei height,
                                          GLenum format,
                                          GLenum type,
                                          void *pixels)
{
    return gl::ReadPixelsContextANGLE(context, x, y, width, height, format, type, pixels);
}

void GL_APIENTRY glReleaseShaderCompilerContextANGLE(GLeglContext context)
{
    return gl::ReleaseShaderCompilerContextANGLE(context);
}

void GL_APIENTRY glRenderbufferStorageContextANGLE(GLeglContext context,
                                                   GLenum target,
                                                   GLenum internalformat,
                                                   GLsizei width,
                                                   GLsizei height)
{
    return gl::RenderbufferStorageContextANGLE(context, target, internalformat, width, height);
}

void GL_APIENTRY glSampleCoverageContextANGLE(GLeglContext context, GLfloat value, GLboolean invert)
{
    return gl::SampleCoverageContextANGLE(context, value, invert);
}

void GL_APIENTRY
glScissorContextANGLE(GLeglContext context, GLint x, GLint y, GLsizei width, GLsizei height)
{
    return gl::ScissorContextANGLE(context, x, y, width, height);
}

void GL_APIENTRY glShaderBinaryContextANGLE(GLeglContext context,
                                            GLsizei n,
                                            const GLuint *shaders,
                                            GLenum binaryformat,
                                            const void *binary,
                                            GLsizei length)
{
    return gl::ShaderBinaryContextANGLE(context, n, shaders, binaryformat, binary, length);
}

void GL_APIENTRY glShaderSourceContextANGLE(GLeglContext context,
                                            GLuint shader,
                                            GLsizei count,
                                            const GLchar *const *string,
                                            const GLint *length)
{
    return gl::ShaderSourceContextANGLE(context, shader, count, string, length);
}

void GL_APIENTRY glStencilFuncContextANGLE(GLeglContext context,
                                           GLenum func,
                                           GLint ref,
                                           GLuint mask)
{
    return gl::StencilFuncContextANGLE(context, func, ref, mask);
}

void GL_APIENTRY glStencilFuncSeparateContextANGLE(GLeglContext context,
                                                   GLenum face,
                                                   GLenum func,
                                                   GLint ref,
                                                   GLuint mask)
{
    return gl::StencilFuncSeparateContextANGLE(context, face, func, ref, mask);
}

void GL_APIENTRY glStencilMaskContextANGLE(GLeglContext context, GLuint mask)
{
    return gl::StencilMaskContextANGLE(context, mask);
}

void GL_APIENTRY glStencilMaskSeparateContextANGLE(GLeglContext context, GLenum face, GLuint mask)
{
    return gl::StencilMaskSeparateContextANGLE(context, face, mask);
}

void GL_APIENTRY glStencilOpContextANGLE(GLeglContext context,
                                         GLenum fail,
                                         GLenum zfail,
                                         GLenum zpass)
{
    return gl::StencilOpContextANGLE(context, fail, zfail, zpass);
}

void GL_APIENTRY glStencilOpSeparateContextANGLE(GLeglContext context,
                                                 GLenum face,
                                                 GLenum fail,
                                                 GLenum zfail,
                                                 GLenum zpass)
{
    return gl::StencilOpSeparateContextANGLE(context, face, fail, zfail, zpass);
}

void GL_APIENTRY glTexImage2DContextANGLE(GLeglContext context,
                                          GLenum target,
                                          GLint level,
                                          GLint internalformat,
                                          GLsizei width,
                                          GLsizei height,
                                          GLint border,
                                          GLenum format,
                                          GLenum type,
                                          const void *pixels)
{
    return gl::TexImage2DContextANGLE(context, target, level, internalformat, width, height, border,
                                      format, type, pixels);
}

void GL_APIENTRY glTexParameterfContextANGLE(GLeglContext context,
                                             GLenum target,
                                             GLenum pname,
                                             GLfloat param)
{
    return gl::TexParameterfContextANGLE(context, target, pname, param);
}

void GL_APIENTRY glTexParameterfvContextANGLE(GLeglContext context,
                                              GLenum target,
                                              GLenum pname,
                                              const GLfloat *params)
{
    return gl::TexParameterfvContextANGLE(context, target, pname, params);
}

void GL_APIENTRY glTexParameteriContextANGLE(GLeglContext context,
                                             GLenum target,
                                             GLenum pname,
                                             GLint param)
{
    return gl::TexParameteriContextANGLE(context, target, pname, param);
}

void GL_APIENTRY glTexParameterivContextANGLE(GLeglContext context,
                                              GLenum target,
                                              GLenum pname,
                                              const GLint *params)
{
    return gl::TexParameterivContextANGLE(context, target, pname, params);
}

void GL_APIENTRY glTexSubImage2DContextANGLE(GLeglContext context,
                                             GLenum target,
                                             GLint level,
                                             GLint xoffset,
                                             GLint yoffset,
                                             GLsizei width,
                                             GLsizei height,
                                             GLenum format,
                                             GLenum type,
                                             const void *pixels)
{
    return gl::TexSubImage2DContextANGLE(context, target, level, xoffset, yoffset, width, height,
                                         format, type, pixels);
}

void GL_APIENTRY glUniform1fContextANGLE(GLeglContext context, GLint location, GLfloat x)
{
    return gl::Uniform1fContextANGLE(context, location, x);
}

void GL_APIENTRY glUniform1fvContextANGLE(GLeglContext context,
                                          GLint location,
                                          GLsizei count,
                                          const GLfloat *v)
{
    return gl::Uniform1fvContextANGLE(context, location, count, v);
}

void GL_APIENTRY glUniform1iContextANGLE(GLeglContext context, GLint location, GLint x)
{
    return gl::Uniform1iContextANGLE(context, location, x);
}

void GL_APIENTRY glUniform1ivContextANGLE(GLeglContext context,
                                          GLint location,
                                          GLsizei count,
                                          const GLint *v)
{
    return gl::Uniform1ivContextANGLE(context, location, count, v);
}

void GL_APIENTRY glUniform2fContextANGLE(GLeglContext context, GLint location, GLfloat x, GLfloat y)
{
    return gl::Uniform2fContextANGLE(context, location, x, y);
}

void GL_APIENTRY glUniform2fvContextANGLE(GLeglContext context,
                                          GLint location,
                                          GLsizei count,
                                          const GLfloat *v)
{
    return gl::Uniform2fvContextANGLE(context, location, count, v);
}

void GL_APIENTRY glUniform2iContextANGLE(GLeglContext context, GLint location, GLint x, GLint y)
{
    return gl::Uniform2iContextANGLE(context, location, x, y);
}

void GL_APIENTRY glUniform2ivContextANGLE(GLeglContext context,
                                          GLint location,
                                          GLsizei count,
                                          const GLint *v)
{
    return gl::Uniform2ivContextANGLE(context, location, count, v);
}

void GL_APIENTRY
glUniform3fContextANGLE(GLeglContext context, GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    return gl::Uniform3fContextANGLE(context, location, x, y, z);
}

void GL_APIENTRY glUniform3fvContextANGLE(GLeglContext context,
                                          GLint location,
                                          GLsizei count,
                                          const GLfloat *v)
{
    return gl::Uniform3fvContextANGLE(context, location, count, v);
}

void GL_APIENTRY
glUniform3iContextANGLE(GLeglContext context, GLint location, GLint x, GLint y, GLint z)
{
    return gl::Uniform3iContextANGLE(context, location, x, y, z);
}

void GL_APIENTRY glUniform3ivContextANGLE(GLeglContext context,
                                          GLint location,
                                          GLsizei count,
                                          const GLint *v)
{
    return gl::Uniform3ivContextANGLE(context, location, count, v);
}

void GL_APIENTRY glUniform4fContextANGLE(GLeglContext context,
                                         GLint location,
                                         GLfloat x,
                                         GLfloat y,
                                         GLfloat z,
                                         GLfloat w)
{
    return gl::Uniform4fContextANGLE(context, location, x, y, z, w);
}

void GL_APIENTRY glUniform4fvContextANGLE(GLeglContext context,
                                          GLint location,
                                          GLsizei count,
                                          const GLfloat *v)
{
    return gl::Uniform4fvContextANGLE(context, location, count, v);
}

void GL_APIENTRY
glUniform4iContextANGLE(GLeglContext context, GLint location, GLint x, GLint y, GLint z, GLint w)
{
    return gl::Uniform4iContextANGLE(context, location, x, y, z, w);
}

void GL_APIENTRY glUniform4ivContextANGLE(GLeglContext context,
                                          GLint location,
                                          GLsizei count,
                                          const GLint *v)
{
    return gl::Uniform4ivContextANGLE(context, location, count, v);
}

void GL_APIENTRY glUniformMatrix2fvContextANGLE(GLeglContext context,
                                                GLint location,
                                                GLsizei count,
                                                GLboolean transpose,
                                                const GLfloat *value)
{
    return gl::UniformMatrix2fvContextANGLE(context, location, count, transpose, value);
}

void GL_APIENTRY glUniformMatrix3fvContextANGLE(GLeglContext context,
                                                GLint location,
                                                GLsizei count,
                                                GLboolean transpose,
                                                const GLfloat *value)
{
    return gl::UniformMatrix3fvContextANGLE(context, location, count, transpose, value);
}

void GL_APIENTRY glUniformMatrix4fvContextANGLE(GLeglContext context,
                                                GLint location,
                                                GLsizei count,
                                                GLboolean transpose,
                                                const GLfloat *value)
{
    return gl::UniformMatrix4fvContextANGLE(context, location, count, transpose, value);
}

void GL_APIENTRY glUseProgramContextANGLE(GLeglContext context, GLuint program)
{
    return gl::UseProgramContextANGLE(context, program);
}

void GL_APIENTRY glValidateProgramContextANGLE(GLeglContext context, GLuint program)
{
    return gl::ValidateProgramContextANGLE(context, program);
}

void GL_APIENTRY glVertexAttrib1fContextANGLE(GLeglContext context, GLuint indx, GLfloat x)
{
    return gl::VertexAttrib1fContextANGLE(context, indx, x);
}

void GL_APIENTRY glVertexAttrib1fvContextANGLE(GLeglContext context,
                                               GLuint indx,
                                               const GLfloat *values)
{
    return gl::VertexAttrib1fvContextANGLE(context, indx, values);
}

void GL_APIENTRY glVertexAttrib2fContextANGLE(GLeglContext context,
                                              GLuint indx,
                                              GLfloat x,
                                              GLfloat y)
{
    return gl::VertexAttrib2fContextANGLE(context, indx, x, y);
}

void GL_APIENTRY glVertexAttrib2fvContextANGLE(GLeglContext context,
                                               GLuint indx,
                                               const GLfloat *values)
{
    return gl::VertexAttrib2fvContextANGLE(context, indx, values);
}

void GL_APIENTRY
glVertexAttrib3fContextANGLE(GLeglContext context, GLuint indx, GLfloat x, GLfloat y, GLfloat z)
{
    return gl::VertexAttrib3fContextANGLE(context, indx, x, y, z);
}

void GL_APIENTRY glVertexAttrib3fvContextANGLE(GLeglContext context,
                                               GLuint indx,
                                               const GLfloat *values)
{
    return gl::VertexAttrib3fvContextANGLE(context, indx, values);
}

void GL_APIENTRY glVertexAttrib4fContextANGLE(GLeglContext context,
                                              GLuint indx,
                                              GLfloat x,
                                              GLfloat y,
                                              GLfloat z,
                                              GLfloat w)
{
    return gl::VertexAttrib4fContextANGLE(context, indx, x, y, z, w);
}

void GL_APIENTRY glVertexAttrib4fvContextANGLE(GLeglContext context,
                                               GLuint indx,
                                               const GLfloat *values)
{
    return gl::VertexAttrib4fvContextANGLE(context, indx, values);
}

void GL_APIENTRY glVertexAttribPointerContextANGLE(GLeglContext context,
                                                   GLuint indx,
                                                   GLint size,
                                                   GLenum type,
                                                   GLboolean normalized,
                                                   GLsizei stride,
                                                   const void *ptr)
{
    return gl::VertexAttribPointerContextANGLE(context, indx, size, type, normalized, stride, ptr);
}

void GL_APIENTRY
glViewportContextANGLE(GLeglContext context, GLint x, GLint y, GLsizei width, GLsizei height)
{
    return gl::ViewportContextANGLE(context, x, y, width, height);
}

void GL_APIENTRY glReadBufferContextANGLE(GLeglContext context, GLenum mode)
{
    return gl::ReadBufferContextANGLE(context, mode);
}

void GL_APIENTRY glDrawRangeElementsContextANGLE(GLeglContext context,
                                                 GLenum mode,
                                                 GLuint start,
                                                 GLuint end,
                                                 GLsizei count,
                                                 GLenum type,
                                                 const void *indices)
{
    return gl::DrawRangeElementsContextANGLE(context, mode, start, end, count, type, indices);
}

void GL_APIENTRY glTexImage3DContextANGLE(GLeglContext context,
                                          GLenum target,
                                          GLint level,
                                          GLint internalformat,
                                          GLsizei width,
                                          GLsizei height,
                                          GLsizei depth,
                                          GLint border,
                                          GLenum format,
                                          GLenum type,
                                          const void *pixels)
{
    return gl::TexImage3DContextANGLE(context, target, level, internalformat, width, height, depth,
                                      border, format, type, pixels);
}

void GL_APIENTRY glTexSubImage3DContextANGLE(GLeglContext context,
                                             GLenum target,
                                             GLint level,
                                             GLint xoffset,
                                             GLint yoffset,
                                             GLint zoffset,
                                             GLsizei width,
                                             GLsizei height,
                                             GLsizei depth,
                                             GLenum format,
                                             GLenum type,
                                             const void *pixels)
{
    return gl::TexSubImage3DContextANGLE(context, target, level, xoffset, yoffset, zoffset, width,
                                         height, depth, format, type, pixels);
}

void GL_APIENTRY glCopyTexSubImage3DContextANGLE(GLeglContext context,
                                                 GLenum target,
                                                 GLint level,
                                                 GLint xoffset,
                                                 GLint yoffset,
                                                 GLint zoffset,
                                                 GLint x,
                                                 GLint y,
                                                 GLsizei width,
                                                 GLsizei height)
{
    return gl::CopyTexSubImage3DContextANGLE(context, target, level, xoffset, yoffset, zoffset, x,
                                             y, width, height);
}

void GL_APIENTRY glCompressedTexImage3DContextANGLE(GLeglContext context,
                                                    GLenum target,
                                                    GLint level,
                                                    GLenum internalformat,
                                                    GLsizei width,
                                                    GLsizei height,
                                                    GLsizei depth,
                                                    GLint border,
                                                    GLsizei imageSize,
                                                    const void *data)
{
    return gl::CompressedTexImage3DContextANGLE(context, target, level, internalformat, width,
                                                height, depth, border, imageSize, data);
}

void GL_APIENTRY glCompressedTexSubImage3DContextANGLE(GLeglContext context,
                                                       GLenum target,
                                                       GLint level,
                                                       GLint xoffset,
                                                       GLint yoffset,
                                                       GLint zoffset,
                                                       GLsizei width,
                                                       GLsizei height,
                                                       GLsizei depth,
                                                       GLenum format,
                                                       GLsizei imageSize,
                                                       const void *data)
{
    return gl::CompressedTexSubImage3DContextANGLE(context, target, level, xoffset, yoffset,
                                                   zoffset, width, height, depth, format, imageSize,
                                                   data);
}

void GL_APIENTRY glGenQueriesContextANGLE(GLeglContext context, GLsizei n, GLuint *ids)
{
    return gl::GenQueriesContextANGLE(context, n, ids);
}

void GL_APIENTRY glDeleteQueriesContextANGLE(GLeglContext context, GLsizei n, const GLuint *ids)
{
    return gl::DeleteQueriesContextANGLE(context, n, ids);
}

GLboolean GL_APIENTRY glIsQueryContextANGLE(GLeglContext context, GLuint id)
{
    return gl::IsQueryContextANGLE(context, id);
}

void GL_APIENTRY glBeginQueryContextANGLE(GLeglContext context, GLenum target, GLuint id)
{
    return gl::BeginQueryContextANGLE(context, target, id);
}

void GL_APIENTRY glEndQueryContextANGLE(GLeglContext context, GLenum target)
{
    return gl::EndQueryContextANGLE(context, target);
}

void GL_APIENTRY glGetQueryivContextANGLE(GLeglContext context,
                                          GLenum target,
                                          GLenum pname,
                                          GLint *params)
{
    return gl::GetQueryivContextANGLE(context, target, pname, params);
}

void GL_APIENTRY glGetQueryObjectuivContextANGLE(GLeglContext context,
                                                 GLuint id,
                                                 GLenum pname,
                                                 GLuint *params)
{
    return gl::GetQueryObjectuivContextANGLE(context, id, pname, params);
}

GLboolean GL_APIENTRY glUnmapBufferContextANGLE(GLeglContext context, GLenum target)
{
    return gl::UnmapBufferContextANGLE(context, target);
}

void GL_APIENTRY glGetBufferPointervContextANGLE(GLeglContext context,
                                                 GLenum target,
                                                 GLenum pname,
                                                 void **params)
{
    return gl::GetBufferPointervContextANGLE(context, target, pname, params);
}

void GL_APIENTRY glDrawBuffersContextANGLE(GLeglContext context, GLsizei n, const GLenum *bufs)
{
    return gl::DrawBuffersContextANGLE(context, n, bufs);
}

void GL_APIENTRY glUniformMatrix2x3fvContextANGLE(GLeglContext context,
                                                  GLint location,
                                                  GLsizei count,
                                                  GLboolean transpose,
                                                  const GLfloat *value)
{
    return gl::UniformMatrix2x3fvContextANGLE(context, location, count, transpose, value);
}

void GL_APIENTRY glUniformMatrix3x2fvContextANGLE(GLeglContext context,
                                                  GLint location,
                                                  GLsizei count,
                                                  GLboolean transpose,
                                                  const GLfloat *value)
{
    return gl::UniformMatrix3x2fvContextANGLE(context, location, count, transpose, value);
}

void GL_APIENTRY glUniformMatrix2x4fvContextANGLE(GLeglContext context,
                                                  GLint location,
                                                  GLsizei count,
                                                  GLboolean transpose,
                                                  const GLfloat *value)
{
    return gl::UniformMatrix2x4fvContextANGLE(context, location, count, transpose, value);
}

void GL_APIENTRY glUniformMatrix4x2fvContextANGLE(GLeglContext context,
                                                  GLint location,
                                                  GLsizei count,
                                                  GLboolean transpose,
                                                  const GLfloat *value)
{
    return gl::UniformMatrix4x2fvContextANGLE(context, location, count, transpose, value);
}

void GL_APIENTRY glUniformMatrix3x4fvContextANGLE(GLeglContext context,
                                                  GLint location,
                                                  GLsizei count,
                                                  GLboolean transpose,
                                                  const GLfloat *value)
{
    return gl::UniformMatrix3x4fvContextANGLE(context, location, count, transpose, value);
}

void GL_APIENTRY glUniformMatrix4x3fvContextANGLE(GLeglContext context,
                                                  GLint location,
                                                  GLsizei count,
                                                  GLboolean transpose,
                                                  const GLfloat *value)
{
    return gl::UniformMatrix4x3fvContextANGLE(context, location, count, transpose, value);
}

void GL_APIENTRY glBlitFramebufferContextANGLE(GLeglContext context,
                                               GLint srcX0,
                                               GLint srcY0,
                                               GLint srcX1,
                                               GLint srcY1,
                                               GLint dstX0,
                                               GLint dstY0,
                                               GLint dstX1,
                                               GLint dstY1,
                                               GLbitfield mask,
                                               GLenum filter)
{
    return gl::BlitFramebufferContextANGLE(context, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1,
                                           dstY1, mask, filter);
}

void GL_APIENTRY glRenderbufferStorageMultisampleContextANGLE(GLeglContext context,
                                                              GLenum target,
                                                              GLsizei samples,
                                                              GLenum internalformat,
                                                              GLsizei width,
                                                              GLsizei height)
{
    return gl::RenderbufferStorageMultisampleContextANGLE(context, target, samples, internalformat,
                                                          width, height);
}

void GL_APIENTRY glFramebufferTextureLayerContextANGLE(GLeglContext context,
                                                       GLenum target,
                                                       GLenum attachment,
                                                       GLuint texture,
                                                       GLint level,
                                                       GLint layer)
{
    return gl::FramebufferTextureLayerContextANGLE(context, target, attachment, texture, level,
                                                   layer);
}

void *GL_APIENTRY glMapBufferRangeContextANGLE(GLeglContext context,
                                               GLenum target,
                                               GLintptr offset,
                                               GLsizeiptr length,
                                               GLbitfield access)
{
    return gl::MapBufferRangeContextANGLE(context, target, offset, length, access);
}

void GL_APIENTRY glFlushMappedBufferRangeContextANGLE(GLeglContext context,
                                                      GLenum target,
                                                      GLintptr offset,
                                                      GLsizeiptr length)
{
    return gl::FlushMappedBufferRangeContextANGLE(context, target, offset, length);
}

void GL_APIENTRY glBindVertexArrayContextANGLE(GLeglContext context, GLuint array)
{
    return gl::BindVertexArrayContextANGLE(context, array);
}

void GL_APIENTRY glDeleteVertexArraysContextANGLE(GLeglContext context,
                                                  GLsizei n,
                                                  const GLuint *arrays)
{
    return gl::DeleteVertexArraysContextANGLE(context, n, arrays);
}

void GL_APIENTRY glGenVertexArraysContextANGLE(GLeglContext context, GLsizei n, GLuint *arrays)
{
    return gl::GenVertexArraysContextANGLE(context, n, arrays);
}

GLboolean GL_APIENTRY glIsVertexArrayContextANGLE(GLeglContext context, GLuint array)
{
    return gl::IsVertexArrayContextANGLE(context, array);
}

void GL_APIENTRY glGetIntegeri_vContextANGLE(GLeglContext context,
                                             GLenum target,
                                             GLuint index,
                                             GLint *data)
{
    return gl::GetIntegeri_vContextANGLE(context, target, index, data);
}

void GL_APIENTRY glBeginTransformFeedbackContextANGLE(GLeglContext context, GLenum primitiveMode)
{
    return gl::BeginTransformFeedbackContextANGLE(context, primitiveMode);
}

void GL_APIENTRY glEndTransformFeedbackContextANGLE(GLeglContext context)
{
    return gl::EndTransformFeedbackContextANGLE(context);
}

void GL_APIENTRY glBindBufferRangeContextANGLE(GLeglContext context,
                                               GLenum target,
                                               GLuint index,
                                               GLuint buffer,
                                               GLintptr offset,
                                               GLsizeiptr size)
{
    return gl::BindBufferRangeContextANGLE(context, target, index, buffer, offset, size);
}

void GL_APIENTRY glBindBufferBaseContextANGLE(GLeglContext context,
                                              GLenum target,
                                              GLuint index,
                                              GLuint buffer)
{
    return gl::BindBufferBaseContextANGLE(context, target, index, buffer);
}

void GL_APIENTRY glTransformFeedbackVaryingsContextANGLE(GLeglContext context,
                                                         GLuint program,
                                                         GLsizei count,
                                                         const GLchar *const *varyings,
                                                         GLenum bufferMode)
{
    return gl::TransformFeedbackVaryingsContextANGLE(context, program, count, varyings, bufferMode);
}

void GL_APIENTRY glGetTransformFeedbackVaryingContextANGLE(GLeglContext context,
                                                           GLuint program,
                                                           GLuint index,
                                                           GLsizei bufSize,
                                                           GLsizei *length,
                                                           GLsizei *size,
                                                           GLenum *type,
                                                           GLchar *name)
{
    return gl::GetTransformFeedbackVaryingContextANGLE(context, program, index, bufSize, length,
                                                       size, type, name);
}

void GL_APIENTRY glVertexAttribIPointerContextANGLE(GLeglContext context,
                                                    GLuint index,
                                                    GLint size,
                                                    GLenum type,
                                                    GLsizei stride,
                                                    const void *pointer)
{
    return gl::VertexAttribIPointerContextANGLE(context, index, size, type, stride, pointer);
}

void GL_APIENTRY glGetVertexAttribIivContextANGLE(GLeglContext context,
                                                  GLuint index,
                                                  GLenum pname,
                                                  GLint *params)
{
    return gl::GetVertexAttribIivContextANGLE(context, index, pname, params);
}

void GL_APIENTRY glGetVertexAttribIuivContextANGLE(GLeglContext context,
                                                   GLuint index,
                                                   GLenum pname,
                                                   GLuint *params)
{
    return gl::GetVertexAttribIuivContextANGLE(context, index, pname, params);
}

void GL_APIENTRY glVertexAttribI4iContextANGLE(GLeglContext context,
                                               GLuint index,
                                               GLint x,
                                               GLint y,
                                               GLint z,
                                               GLint w)
{
    return gl::VertexAttribI4iContextANGLE(context, index, x, y, z, w);
}

void GL_APIENTRY glVertexAttribI4uiContextANGLE(GLeglContext context,
                                                GLuint index,
                                                GLuint x,
                                                GLuint y,
                                                GLuint z,
                                                GLuint w)
{
    return gl::VertexAttribI4uiContextANGLE(context, index, x, y, z, w);
}

void GL_APIENTRY glVertexAttribI4ivContextANGLE(GLeglContext context, GLuint index, const GLint *v)
{
    return gl::VertexAttribI4ivContextANGLE(context, index, v);
}

void GL_APIENTRY glVertexAttribI4uivContextANGLE(GLeglContext context,
                                                 GLuint index,
                                                 const GLuint *v)
{
    return gl::VertexAttribI4uivContextANGLE(context, index, v);
}

void GL_APIENTRY glGetUniformuivContextANGLE(GLeglContext context,
                                             GLuint program,
                                             GLint location,
                                             GLuint *params)
{
    return gl::GetUniformuivContextANGLE(context, program, location, params);
}

GLint GL_APIENTRY glGetFragDataLocationContextANGLE(GLeglContext context,
                                                    GLuint program,
                                                    const GLchar *name)
{
    return gl::GetFragDataLocationContextANGLE(context, program, name);
}

void GL_APIENTRY glUniform1uiContextANGLE(GLeglContext context, GLint location, GLuint v0)
{
    return gl::Uniform1uiContextANGLE(context, location, v0);
}

void GL_APIENTRY glUniform2uiContextANGLE(GLeglContext context,
                                          GLint location,
                                          GLuint v0,
                                          GLuint v1)
{
    return gl::Uniform2uiContextANGLE(context, location, v0, v1);
}

void GL_APIENTRY
glUniform3uiContextANGLE(GLeglContext context, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    return gl::Uniform3uiContextANGLE(context, location, v0, v1, v2);
}

void GL_APIENTRY glUniform4uiContextANGLE(GLeglContext context,
                                          GLint location,
                                          GLuint v0,
                                          GLuint v1,
                                          GLuint v2,
                                          GLuint v3)
{
    return gl::Uniform4uiContextANGLE(context, location, v0, v1, v2, v3);
}

void GL_APIENTRY glUniform1uivContextANGLE(GLeglContext context,
                                           GLint location,
                                           GLsizei count,
                                           const GLuint *value)
{
    return gl::Uniform1uivContextANGLE(context, location, count, value);
}

void GL_APIENTRY glUniform2uivContextANGLE(GLeglContext context,
                                           GLint location,
                                           GLsizei count,
                                           const GLuint *value)
{
    return gl::Uniform2uivContextANGLE(context, location, count, value);
}

void GL_APIENTRY glUniform3uivContextANGLE(GLeglContext context,
                                           GLint location,
                                           GLsizei count,
                                           const GLuint *value)
{
    return gl::Uniform3uivContextANGLE(context, location, count, value);
}

void GL_APIENTRY glUniform4uivContextANGLE(GLeglContext context,
                                           GLint location,
                                           GLsizei count,
                                           const GLuint *value)
{
    return gl::Uniform4uivContextANGLE(context, location, count, value);
}

void GL_APIENTRY glClearBufferivContextANGLE(GLeglContext context,
                                             GLenum buffer,
                                             GLint drawbuffer,
                                             const GLint *value)
{
    return gl::ClearBufferivContextANGLE(context, buffer, drawbuffer, value);
}

void GL_APIENTRY glClearBufferuivContextANGLE(GLeglContext context,
                                              GLenum buffer,
                                              GLint drawbuffer,
                                              const GLuint *value)
{
    return gl::ClearBufferuivContextANGLE(context, buffer, drawbuffer, value);
}

void GL_APIENTRY glClearBufferfvContextANGLE(GLeglContext context,
                                             GLenum buffer,
                                             GLint drawbuffer,
                                             const GLfloat *value)
{
    return gl::ClearBufferfvContextANGLE(context, buffer, drawbuffer, value);
}

void GL_APIENTRY glClearBufferfiContextANGLE(GLeglContext context,
                                             GLenum buffer,
                                             GLint drawbuffer,
                                             GLfloat depth,
                                             GLint stencil)
{
    return gl::ClearBufferfiContextANGLE(context, buffer, drawbuffer, depth, stencil);
}

const GLubyte *GL_APIENTRY glGetStringiContextANGLE(GLeglContext context, GLenum name, GLuint index)
{
    return gl::GetStringiContextANGLE(context, name, index);
}

void GL_APIENTRY glCopyBufferSubDataContextANGLE(GLeglContext context,
                                                 GLenum readTarget,
                                                 GLenum writeTarget,
                                                 GLintptr readOffset,
                                                 GLintptr writeOffset,
                                                 GLsizeiptr size)
{
    return gl::CopyBufferSubDataContextANGLE(context, readTarget, writeTarget, readOffset,
                                             writeOffset, size);
}

void GL_APIENTRY glGetUniformIndicesContextANGLE(GLeglContext context,
                                                 GLuint program,
                                                 GLsizei uniformCount,
                                                 const GLchar *const *uniformNames,
                                                 GLuint *uniformIndices)
{
    return gl::GetUniformIndicesContextANGLE(context, program, uniformCount, uniformNames,
                                             uniformIndices);
}

void GL_APIENTRY glGetActiveUniformsivContextANGLE(GLeglContext context,
                                                   GLuint program,
                                                   GLsizei uniformCount,
                                                   const GLuint *uniformIndices,
                                                   GLenum pname,
                                                   GLint *params)
{
    return gl::GetActiveUniformsivContextANGLE(context, program, uniformCount, uniformIndices,
                                               pname, params);
}

GLuint GL_APIENTRY glGetUniformBlockIndexContextANGLE(GLeglContext context,
                                                      GLuint program,
                                                      const GLchar *uniformBlockName)
{
    return gl::GetUniformBlockIndexContextANGLE(context, program, uniformBlockName);
}

void GL_APIENTRY glGetActiveUniformBlockivContextANGLE(GLeglContext context,
                                                       GLuint program,
                                                       GLuint uniformBlockIndex,
                                                       GLenum pname,
                                                       GLint *params)
{
    return gl::GetActiveUniformBlockivContextANGLE(context, program, uniformBlockIndex, pname,
                                                   params);
}

void GL_APIENTRY glGetActiveUniformBlockNameContextANGLE(GLeglContext context,
                                                         GLuint program,
                                                         GLuint uniformBlockIndex,
                                                         GLsizei bufSize,
                                                         GLsizei *length,
                                                         GLchar *uniformBlockName)
{
    return gl::GetActiveUniformBlockNameContextANGLE(context, program, uniformBlockIndex, bufSize,
                                                     length, uniformBlockName);
}

void GL_APIENTRY glUniformBlockBindingContextANGLE(GLeglContext context,
                                                   GLuint program,
                                                   GLuint uniformBlockIndex,
                                                   GLuint uniformBlockBinding)
{
    return gl::UniformBlockBindingContextANGLE(context, program, uniformBlockIndex,
                                               uniformBlockBinding);
}

void GL_APIENTRY glDrawArraysInstancedContextANGLE(GLeglContext context,
                                                   GLenum mode,
                                                   GLint first,
                                                   GLsizei count,
                                                   GLsizei instanceCount)
{
    return gl::DrawArraysInstancedContextANGLE(context, mode, first, count, instanceCount);
}

void GL_APIENTRY glDrawElementsInstancedContextANGLE(GLeglContext context,
                                                     GLenum mode,
                                                     GLsizei count,
                                                     GLenum type,
                                                     const void *indices,
                                                     GLsizei instanceCount)
{
    return gl::DrawElementsInstancedContextANGLE(context, mode, count, type, indices,
                                                 instanceCount);
}

GLsync GL_APIENTRY glFenceSyncContextANGLE(GLeglContext context, GLenum condition, GLbitfield flags)
{
    return gl::FenceSyncContextANGLE(context, condition, flags);
}

GLboolean GL_APIENTRY glIsSyncContextANGLE(GLeglContext context, GLsync sync)
{
    return gl::IsSyncContextANGLE(context, sync);
}

void GL_APIENTRY glDeleteSyncContextANGLE(GLeglContext context, GLsync sync)
{
    return gl::DeleteSyncContextANGLE(context, sync);
}

GLenum GL_APIENTRY glClientWaitSyncContextANGLE(GLeglContext context,
                                                GLsync sync,
                                                GLbitfield flags,
                                                GLuint64 timeout)
{
    return gl::ClientWaitSyncContextANGLE(context, sync, flags, timeout);
}

void GL_APIENTRY glWaitSyncContextANGLE(GLeglContext context,
                                        GLsync sync,
                                        GLbitfield flags,
                                        GLuint64 timeout)
{
    return gl::WaitSyncContextANGLE(context, sync, flags, timeout);
}

void GL_APIENTRY glGetInteger64vContextANGLE(GLeglContext context, GLenum pname, GLint64 *params)
{
    return gl::GetInteger64vContextANGLE(context, pname, params);
}

void GL_APIENTRY glGetSyncivContextANGLE(GLeglContext context,
                                         GLsync sync,
                                         GLenum pname,
                                         GLsizei bufSize,
                                         GLsizei *length,
                                         GLint *values)
{
    return gl::GetSyncivContextANGLE(context, sync, pname, bufSize, length, values);
}

void GL_APIENTRY glGetInteger64i_vContextANGLE(GLeglContext context,
                                               GLenum target,
                                               GLuint index,
                                               GLint64 *data)
{
    return gl::GetInteger64i_vContextANGLE(context, target, index, data);
}

void GL_APIENTRY glGetBufferParameteri64vContextANGLE(GLeglContext context,
                                                      GLenum target,
                                                      GLenum pname,
                                                      GLint64 *params)
{
    return gl::GetBufferParameteri64vContextANGLE(context, target, pname, params);
}

void GL_APIENTRY glGenSamplersContextANGLE(GLeglContext context, GLsizei count, GLuint *samplers)
{
    return gl::GenSamplersContextANGLE(context, count, samplers);
}

void GL_APIENTRY glDeleteSamplersContextANGLE(GLeglContext context,
                                              GLsizei count,
                                              const GLuint *samplers)
{
    return gl::DeleteSamplersContextANGLE(context, count, samplers);
}

GLboolean GL_APIENTRY glIsSamplerContextANGLE(GLeglContext context, GLuint sampler)
{
    return gl::IsSamplerContextANGLE(context, sampler);
}

void GL_APIENTRY glBindSamplerContextANGLE(GLeglContext context, GLuint unit, GLuint sampler)
{
    return gl::BindSamplerContextANGLE(context, unit, sampler);
}

void GL_APIENTRY glSamplerParameteriContextANGLE(GLeglContext context,
                                                 GLuint sampler,
                                                 GLenum pname,
                                                 GLint param)
{
    return gl::SamplerParameteriContextANGLE(context, sampler, pname, param);
}

void GL_APIENTRY glSamplerParameterivContextANGLE(GLeglContext context,
                                                  GLuint sampler,
                                                  GLenum pname,
                                                  const GLint *param)
{
    return gl::SamplerParameterivContextANGLE(context, sampler, pname, param);
}

void GL_APIENTRY glSamplerParameterfContextANGLE(GLeglContext context,
                                                 GLuint sampler,
                                                 GLenum pname,
                                                 GLfloat param)
{
    return gl::SamplerParameterfContextANGLE(context, sampler, pname, param);
}

void GL_APIENTRY glSamplerParameterfvContextANGLE(GLeglContext context,
                                                  GLuint sampler,
                                                  GLenum pname,
                                                  const GLfloat *param)
{
    return gl::SamplerParameterfvContextANGLE(context, sampler, pname, param);
}

void GL_APIENTRY glGetSamplerParameterivContextANGLE(GLeglContext context,
                                                     GLuint sampler,
                                                     GLenum pname,
                                                     GLint *params)
{
    return gl::GetSamplerParameterivContextANGLE(context, sampler, pname, params);
}

void GL_APIENTRY glGetSamplerParameterfvContextANGLE(GLeglContext context,
                                                     GLuint sampler,
                                                     GLenum pname,
                                                     GLfloat *params)
{
    return gl::GetSamplerParameterfvContextANGLE(context, sampler, pname, params);
}

void GL_APIENTRY glVertexAttribDivisorContextANGLE(GLeglContext context,
                                                   GLuint index,
                                                   GLuint divisor)
{
    return gl::VertexAttribDivisorContextANGLE(context, index, divisor);
}

void GL_APIENTRY glBindTransformFeedbackContextANGLE(GLeglContext context, GLenum target, GLuint id)
{
    return gl::BindTransformFeedbackContextANGLE(context, target, id);
}

void GL_APIENTRY glDeleteTransformFeedbacksContextANGLE(GLeglContext context,
                                                        GLsizei n,
                                                        const GLuint *ids)
{
    return gl::DeleteTransformFeedbacksContextANGLE(context, n, ids);
}

void GL_APIENTRY glGenTransformFeedbacksContextANGLE(GLeglContext context, GLsizei n, GLuint *ids)
{
    return gl::GenTransformFeedbacksContextANGLE(context, n, ids);
}

GLboolean GL_APIENTRY glIsTransformFeedbackContextANGLE(GLeglContext context, GLuint id)
{
    return gl::IsTransformFeedbackContextANGLE(context, id);
}

void GL_APIENTRY glPauseTransformFeedbackContextANGLE(GLeglContext context)
{
    return gl::PauseTransformFeedbackContextANGLE(context);
}

void GL_APIENTRY glResumeTransformFeedbackContextANGLE(GLeglContext context)
{
    return gl::ResumeTransformFeedbackContextANGLE(context);
}

void GL_APIENTRY glGetProgramBinaryContextANGLE(GLeglContext context,
                                                GLuint program,
                                                GLsizei bufSize,
                                                GLsizei *length,
                                                GLenum *binaryFormat,
                                                void *binary)
{
    return gl::GetProgramBinaryContextANGLE(context, program, bufSize, length, binaryFormat,
                                            binary);
}

void GL_APIENTRY glProgramBinaryContextANGLE(GLeglContext context,
                                             GLuint program,
                                             GLenum binaryFormat,
                                             const void *binary,
                                             GLsizei length)
{
    return gl::ProgramBinaryContextANGLE(context, program, binaryFormat, binary, length);
}

void GL_APIENTRY glProgramParameteriContextANGLE(GLeglContext context,
                                                 GLuint program,
                                                 GLenum pname,
                                                 GLint value)
{
    return gl::ProgramParameteriContextANGLE(context, program, pname, value);
}

void GL_APIENTRY glInvalidateFramebufferContextANGLE(GLeglContext context,
                                                     GLenum target,
                                                     GLsizei numAttachments,
                                                     const GLenum *attachments)
{
    return gl::InvalidateFramebufferContextANGLE(context, target, numAttachments, attachments);
}

void GL_APIENTRY glInvalidateSubFramebufferContextANGLE(GLeglContext context,
                                                        GLenum target,
                                                        GLsizei numAttachments,
                                                        const GLenum *attachments,
                                                        GLint x,
                                                        GLint y,
                                                        GLsizei width,
                                                        GLsizei height)
{
    return gl::InvalidateSubFramebufferContextANGLE(context, target, numAttachments, attachments, x,
                                                    y, width, height);
}

void GL_APIENTRY glTexStorage2DContextANGLE(GLeglContext context,
                                            GLenum target,
                                            GLsizei levels,
                                            GLenum internalformat,
                                            GLsizei width,
                                            GLsizei height)
{
    return gl::TexStorage2DContextANGLE(context, target, levels, internalformat, width, height);
}

void GL_APIENTRY glTexStorage3DContextANGLE(GLeglContext context,
                                            GLenum target,
                                            GLsizei levels,
                                            GLenum internalformat,
                                            GLsizei width,
                                            GLsizei height,
                                            GLsizei depth)
{
    return gl::TexStorage3DContextANGLE(context, target, levels, internalformat, width, height,
                                        depth);
}

void GL_APIENTRY glGetInternalformativContextANGLE(GLeglContext context,
                                                   GLenum target,
                                                   GLenum internalformat,
                                                   GLenum pname,
                                                   GLsizei bufSize,
                                                   GLint *params)
{
    return gl::GetInternalformativContextANGLE(context, target, internalformat, pname, bufSize,
                                               params);
}

void GL_APIENTRY glBlitFramebufferANGLEContextANGLE(GLeglContext context,
                                                    GLint srcX0,
                                                    GLint srcY0,
                                                    GLint srcX1,
                                                    GLint srcY1,
                                                    GLint dstX0,
                                                    GLint dstY0,
                                                    GLint dstX1,
                                                    GLint dstY1,
                                                    GLbitfield mask,
                                                    GLenum filter)
{
    return gl::BlitFramebufferANGLEContextANGLE(context, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0,
                                                dstX1, dstY1, mask, filter);
}

void GL_APIENTRY glRenderbufferStorageMultisampleANGLEContextANGLE(GLeglContext context,
                                                                   GLenum target,
                                                                   GLsizei samples,
                                                                   GLenum internalformat,
                                                                   GLsizei width,
                                                                   GLsizei height)
{
    return gl::RenderbufferStorageMultisampleANGLEContextANGLE(context, target, samples,
                                                               internalformat, width, height);
}

void GL_APIENTRY glDiscardFramebufferEXTContextANGLE(GLeglContext context,
                                                     GLenum target,
                                                     GLsizei numAttachments,
                                                     const GLenum *attachments)
{
    return gl::DiscardFramebufferEXTContextANGLE(context, target, numAttachments, attachments);
}

void GL_APIENTRY glDeleteFencesNVContextANGLE(GLeglContext context, GLsizei n, const GLuint *fences)
{
    return gl::DeleteFencesNVContextANGLE(context, n, fences);
}

void GL_APIENTRY glGenFencesNVContextANGLE(GLeglContext context, GLsizei n, GLuint *fences)
{
    return gl::GenFencesNVContextANGLE(context, n, fences);
}

GLboolean GL_APIENTRY glIsFenceNVContextANGLE(GLeglContext context, GLuint fence)
{
    return gl::IsFenceNVContextANGLE(context, fence);
}

GLboolean GL_APIENTRY glTestFenceNVContextANGLE(GLeglContext context, GLuint fence)
{
    return gl::TestFenceNVContextANGLE(context, fence);
}

void GL_APIENTRY glGetFenceivNVContextANGLE(GLeglContext context,
                                            GLuint fence,
                                            GLenum pname,
                                            GLint *params)
{
    return gl::GetFenceivNVContextANGLE(context, fence, pname, params);
}

void GL_APIENTRY glFinishFenceNVContextANGLE(GLeglContext context, GLuint fence)
{
    return gl::FinishFenceNVContextANGLE(context, fence);
}

void GL_APIENTRY glSetFenceNVContextANGLE(GLeglContext context, GLuint fence, GLenum condition)
{
    return gl::SetFenceNVContextANGLE(context, fence, condition);
}

void GL_APIENTRY glGetTranslatedShaderSourceANGLEContextANGLE(GLeglContext context,
                                                              GLuint shader,
                                                              GLsizei bufsize,
                                                              GLsizei *length,
                                                              GLchar *source)
{
    return gl::GetTranslatedShaderSourceANGLEContextANGLE(context, shader, bufsize, length, source);
}

void GL_APIENTRY glTexStorage2DEXTContextANGLE(GLeglContext context,
                                               GLenum target,
                                               GLsizei levels,
                                               GLenum internalformat,
                                               GLsizei width,
                                               GLsizei height)
{
    return gl::TexStorage2DEXTContextANGLE(context, target, levels, internalformat, width, height);
}

GLenum GL_APIENTRY glGetGraphicsResetStatusEXTContextANGLE(GLeglContext context)
{
    return gl::GetGraphicsResetStatusEXTContextANGLE(context);
}

void GL_APIENTRY glReadnPixelsEXTContextANGLE(GLeglContext context,
                                              GLint x,
                                              GLint y,
                                              GLsizei width,
                                              GLsizei height,
                                              GLenum format,
                                              GLenum type,
                                              GLsizei bufSize,
                                              void *data)
{
    return gl::ReadnPixelsEXTContextANGLE(context, x, y, width, height, format, type, bufSize,
                                          data);
}

void GL_APIENTRY glGetnUniformfvEXTContextANGLE(GLeglContext context,
                                                GLuint program,
                                                GLint location,
                                                GLsizei bufSize,
                                                float *params)
{
    return gl::GetnUniformfvEXTContextANGLE(context, program, location, bufSize, params);
}

void GL_APIENTRY glGetnUniformivEXTContextANGLE(GLeglContext context,
                                                GLuint program,
                                                GLint location,
                                                GLsizei bufSize,
                                                GLint *params)
{
    return gl::GetnUniformivEXTContextANGLE(context, program, location, bufSize, params);
}

void GL_APIENTRY glGenQueriesEXTContextANGLE(GLeglContext context, GLsizei n, GLuint *ids)
{
    return gl::GenQueriesEXTContextANGLE(context, n, ids);
}

void GL_APIENTRY glDeleteQueriesEXTContextANGLE(GLeglContext context, GLsizei n, const GLuint *ids)
{
    return gl::DeleteQueriesEXTContextANGLE(context, n, ids);
}

GLboolean GL_APIENTRY glIsQueryEXTContextANGLE(GLeglContext context, GLuint id)
{
    return gl::IsQueryEXTContextANGLE(context, id);
}

void GL_APIENTRY glBeginQueryEXTContextANGLE(GLeglContext context, GLenum target, GLuint id)
{
    return gl::BeginQueryEXTContextANGLE(context, target, id);
}

void GL_APIENTRY glEndQueryEXTContextANGLE(GLeglContext context, GLenum target)
{
    return gl::EndQueryEXTContextANGLE(context, target);
}

void GL_APIENTRY glQueryCounterEXTContextANGLE(GLeglContext context, GLuint id, GLenum target)
{
    return gl::QueryCounterEXTContextANGLE(context, id, target);
}

void GL_APIENTRY glGetQueryivEXTContextANGLE(GLeglContext context,
                                             GLenum target,
                                             GLenum pname,
                                             GLint *params)
{
    return gl::GetQueryivEXTContextANGLE(context, target, pname, params);
}

void GL_APIENTRY glGetQueryObjectivEXTContextANGLE(GLeglContext context,
                                                   GLuint id,
                                                   GLenum pname,
                                                   GLint *params)
{
    return gl::GetQueryObjectivEXTContextANGLE(context, id, pname, params);
}

void GL_APIENTRY glGetQueryObjectuivEXTContextANGLE(GLeglContext context,
                                                    GLuint id,
                                                    GLenum pname,
                                                    GLuint *params)
{
    return gl::GetQueryObjectuivEXTContextANGLE(context, id, pname, params);
}

void GL_APIENTRY glGetQueryObjecti64vEXTContextANGLE(GLeglContext context,
                                                     GLuint id,
                                                     GLenum pname,
                                                     GLint64 *params)
{
    return gl::GetQueryObjecti64vEXTContextANGLE(context, id, pname, params);
}

void GL_APIENTRY glGetQueryObjectui64vEXTContextANGLE(GLeglContext context,
                                                      GLuint id,
                                                      GLenum pname,
                                                      GLuint64 *params)
{
    return gl::GetQueryObjectui64vEXTContextANGLE(context, id, pname, params);
}

void GL_APIENTRY glDrawBuffersEXTContextANGLE(GLeglContext context, GLsizei n, const GLenum *bufs)
{
    return gl::DrawBuffersEXTContextANGLE(context, n, bufs);
}

void GL_APIENTRY glDrawArraysInstancedANGLEContextANGLE(GLeglContext context,
                                                        GLenum mode,
                                                        GLint first,
                                                        GLsizei count,
                                                        GLsizei primcount)
{
    return gl::DrawArraysInstancedANGLEContextANGLE(context, mode, first, count, primcount);
}

void GL_APIENTRY glDrawElementsInstancedANGLEContextANGLE(GLeglContext context,
                                                          GLenum mode,
                                                          GLsizei count,
                                                          GLenum type,
                                                          const void *indices,
                                                          GLsizei primcount)
{
    return gl::DrawElementsInstancedANGLEContextANGLE(context, mode, count, type, indices,
                                                      primcount);
}

void GL_APIENTRY glVertexAttribDivisorANGLEContextANGLE(GLeglContext context,
                                                        GLuint index,
                                                        GLuint divisor)
{
    return gl::VertexAttribDivisorANGLEContextANGLE(context, index, divisor);
}

void GL_APIENTRY glGetProgramBinaryOESContextANGLE(GLeglContext context,
                                                   GLuint program,
                                                   GLsizei bufSize,
                                                   GLsizei *length,
                                                   GLenum *binaryFormat,
                                                   void *binary)
{
    return gl::GetProgramBinaryOESContextANGLE(context, program, bufSize, length, binaryFormat,
                                               binary);
}

void GL_APIENTRY glProgramBinaryOESContextANGLE(GLeglContext context,
                                                GLuint program,
                                                GLenum binaryFormat,
                                                const void *binary,
                                                GLint length)
{
    return gl::ProgramBinaryOESContextANGLE(context, program, binaryFormat, binary, length);
}

void *GL_APIENTRY glMapBufferOESContextANGLE(GLeglContext context, GLenum target, GLenum access)
{
    return gl::MapBufferOESContextANGLE(context, target, access);
}

GLboolean GL_APIENTRY glUnmapBufferOESContextANGLE(GLeglContext context, GLenum target)
{
    return gl::UnmapBufferOESContextANGLE(context, target);
}

void GL_APIENTRY glGetBufferPointervOESContextANGLE(GLeglContext context,
                                                    GLenum target,
                                                    GLenum pname,
                                                    void **params)
{
    return gl::GetBufferPointervOESContextANGLE(context, target, pname, params);
}

void *GL_APIENTRY glMapBufferRangeEXTContextANGLE(GLeglContext context,
                                                  GLenum target,
                                                  GLintptr offset,
                                                  GLsizeiptr length,
                                                  GLbitfield access)
{
    return gl::MapBufferRangeEXTContextANGLE(context, target, offset, length, access);
}

void GL_APIENTRY glFlushMappedBufferRangeEXTContextANGLE(GLeglContext context,
                                                         GLenum target,
                                                         GLintptr offset,
                                                         GLsizeiptr length)
{
    return gl::FlushMappedBufferRangeEXTContextANGLE(context, target, offset, length);
}

void GL_APIENTRY glInsertEventMarkerEXTContextANGLE(GLeglContext context,
                                                    GLsizei length,
                                                    const char *marker)
{
    return gl::InsertEventMarkerEXTContextANGLE(context, length, marker);
}

void GL_APIENTRY glPushGroupMarkerEXTContextANGLE(GLeglContext context,
                                                  GLsizei length,
                                                  const char *marker)
{
    return gl::PushGroupMarkerEXTContextANGLE(context, length, marker);
}

void GL_APIENTRY glPopGroupMarkerEXTContextANGLE(GLeglContext context)
{
    return gl::PopGroupMarkerEXTContextANGLE(context);
}

void GL_APIENTRY glEGLImageTargetTexture2DOESContextANGLE(GLeglContext context,
                                                          GLenum target,
                                                          GLeglImageOES image)
{
    return gl::EGLImageTargetTexture2DOESContextANGLE(context, target, image);
}

void GL_APIENTRY glEGLImageTargetRenderbufferStorageOESContextANGLE(GLeglContext context,
                                                                    GLenum target,
                                                                    GLeglImageOES image)
{
    return gl::EGLImageTargetRenderbufferStorageOESContextANGLE(context, target, image);
}

void GL_APIENTRY glBindVertexArrayOESContextANGLE(GLeglContext context, GLuint array)
{
    return gl::BindVertexArrayOESContextANGLE(context, array);
}

void GL_APIENTRY glDeleteVertexArraysOESContextANGLE(GLeglContext context,
                                                     GLsizei n,
                                                     const GLuint *arrays)
{
    return gl::DeleteVertexArraysOESContextANGLE(context, n, arrays);
}

void GL_APIENTRY glGenVertexArraysOESContextANGLE(GLeglContext context, GLsizei n, GLuint *arrays)
{
    return gl::GenVertexArraysOESContextANGLE(context, n, arrays);
}

GLboolean GL_APIENTRY glIsVertexArrayOESContextANGLE(GLeglContext context, GLuint array)
{
    return gl::IsVertexArrayOESContextANGLE(context, array);
}

void GL_APIENTRY glDebugMessageControlKHRContextANGLE(GLeglContext context,
                                                      GLenum source,
                                                      GLenum type,
                                                      GLenum severity,
                                                      GLsizei count,
                                                      const GLuint *ids,
                                                      GLboolean enabled)
{
    return gl::DebugMessageControlKHRContextANGLE(context, source, type, severity, count, ids,
                                                  enabled);
}

void GL_APIENTRY glDebugMessageInsertKHRContextANGLE(GLeglContext context,
                                                     GLenum source,
                                                     GLenum type,
                                                     GLuint id,
                                                     GLenum severity,
                                                     GLsizei length,
                                                     const GLchar *buf)
{
    return gl::DebugMessageInsertKHRContextANGLE(context, source, type, id, severity, length, buf);
}

void GL_APIENTRY glDebugMessageCallbackKHRContextANGLE(GLeglContext context,
                                                       GLDEBUGPROCKHR callback,
                                                       const void *userParam)
{
    return gl::DebugMessageCallbackKHRContextANGLE(context, callback, userParam);
}

GLuint GL_APIENTRY glGetDebugMessageLogKHRContextANGLE(GLeglContext context,
                                                       GLuint count,
                                                       GLsizei bufSize,
                                                       GLenum *sources,
                                                       GLenum *types,
                                                       GLuint *ids,
                                                       GLenum *severities,
                                                       GLsizei *lengths,
                                                       GLchar *messageLog)
{
    return gl::GetDebugMessageLogKHRContextANGLE(context, count, bufSize, sources, types, ids,
                                                 severities, lengths, messageLog);
}

void GL_APIENTRY glPushDebugGroupKHRContextANGLE(GLeglContext context,
                                                 GLenum source,
                                                 GLuint id,
                                                 GLsizei length,
                                                 const GLchar *message)
{
    return gl::PushDebugGroupKHRContextANGLE(context, source, id, length, message);
}

void GL_APIENTRY glPopDebugGroupKHRContextANGLE(GLeglContext context)
{
    return gl::PopDebugGroupKHRContextANGLE(context);
}

void GL_APIENTRY glObjectLabelKHRContextANGLE(GLeglContext context,
                                              GLenum identifier,
                                              GLuint name,
                                              GLsizei length,
                                              const GLchar *label)
{
    return gl::ObjectLabelKHRContextANGLE(context, identifier, name, length, label);
}

void GL_APIENTRY glGetObjectLabelKHRContextANGLE(GLeglContext context,
                                                 GLenum identifier,
                                                 GLuint name,
                                                 GLsizei bufSize,
                                                 GLsizei *length,
                                                 GLchar *label)
{
    return gl::GetObjectLabelKHRContextANGLE(context, identifier, name, bufSize, length, label);
}

void GL_APIENTRY glObjectPtrLabelKHRContextANGLE(GLeglContext context,
                                                 const void *ptr,
                                                 GLsizei length,
                                                 const GLchar *label)
{
    return gl::ObjectPtrLabelKHRContextANGLE(context, ptr, length, label);
}

void GL_APIENTRY glGetObjectPtrLabelKHRContextANGLE(GLeglContext context,
                                                    const void *ptr,
                                                    GLsizei bufSize,
                                                    GLsizei *length,
                                                    GLchar *label)
{
    return gl::GetObjectPtrLabelKHRContextANGLE(context, ptr, bufSize, length, label);
}

void GL_APIENTRY glGetPointervKHRContextANGLE(GLeglContext context, GLenum pname, void **params)
{
    return gl::GetPointervKHRContextANGLE(context, pname, params);
}

void GL_APIENTRY glBindUniformLocationCHROMIUMContextANGLE(GLeglContext context,
                                                           GLuint program,
                                                           GLint location,
                                                           const GLchar *name)
{
    return gl::BindUniformLocationCHROMIUMContextANGLE(context, program, location, name);
}

void GL_APIENTRY glCoverageModulationCHROMIUMContextANGLE(GLeglContext context, GLenum components)
{
    return gl::CoverageModulationCHROMIUMContextANGLE(context, components);
}

// CHROMIUM_path_rendendering
void GL_APIENTRY glMatrixLoadfCHROMIUMContextANGLE(GLeglContext context,
                                                   GLenum matrixMode,
                                                   const GLfloat *matrix)
{
    gl::MatrixLoadfCHROMIUMContextANGLE(context, matrixMode, matrix);
}

void GL_APIENTRY glMatrixLoadIdentityCHROMIUMContextANGLE(GLeglContext context, GLenum matrixMode)
{
    gl::MatrixLoadIdentityCHROMIUMContextANGLE(context, matrixMode);
}

GLuint GL_APIENTRY glGenPathsCHROMIUMContextANGLE(GLeglContext context, GLsizei range)
{
    return gl::GenPathsCHROMIUMContextANGLE(context, range);
}

void GL_APIENTRY glDeletePathsCHROMIUMContextANGLE(GLeglContext context,
                                                   GLuint first,
                                                   GLsizei range)
{
    gl::DeletePathsCHROMIUMContextANGLE(context, first, range);
}

GLboolean GL_APIENTRY glIsPathCHROMIUMContextANGLE(GLeglContext context, GLuint path)
{
    return gl::IsPathCHROMIUMContextANGLE(context, path);
}

void GL_APIENTRY glPathCommandsCHROMIUMContextANGLE(GLeglContext context,
                                                    GLuint path,
                                                    GLsizei numCommands,
                                                    const GLubyte *commands,
                                                    GLsizei numCoords,
                                                    GLenum coordType,
                                                    const void *coords)
{
    gl::PathCommandsCHROMIUMContextANGLE(context, path, numCommands, commands, numCoords, coordType,
                                         coords);
}

void GL_APIENTRY glPathParameterfCHROMIUMContextANGLE(GLeglContext context,
                                                      GLuint path,
                                                      GLenum pname,
                                                      GLfloat value)
{
    gl::PathParameterfCHROMIUMContextANGLE(context, path, pname, value);
}

void GL_APIENTRY glPathParameteriCHROMIUMContextANGLE(GLeglContext context,
                                                      GLuint path,
                                                      GLenum pname,
                                                      GLint value)
{
    gl::PathParameteriCHROMIUMContextANGLE(context, path, pname, value);
}

void GL_APIENTRY glGetPathParameterfvCHROMIUMContextANGLE(GLeglContext context,
                                                          GLuint path,
                                                          GLenum pname,
                                                          GLfloat *value)
{
    gl::GetPathParameterfvCHROMIUMContextANGLE(context, path, pname, value);
}

void GL_APIENTRY glGetPathParameterivCHROMIUMContextANGLE(GLeglContext context,
                                                          GLuint path,
                                                          GLenum pname,
                                                          GLint *value)
{
    gl::GetPathParameterivCHROMIUMContextANGLE(context, path, pname, value);
}

void GL_APIENTRY glPathStencilFuncCHROMIUMContextANGLE(GLeglContext context,
                                                       GLenum func,
                                                       GLint ref,
                                                       GLuint mask)
{
    gl::PathStencilFuncCHROMIUMContextANGLE(context, func, ref, mask);
}

void GL_APIENTRY glStencilFillPathCHROMIUMContextANGLE(GLeglContext context,
                                                       GLuint path,
                                                       GLenum fillMode,
                                                       GLuint mask)
{
    gl::StencilFillPathCHROMIUMContextANGLE(context, path, fillMode, mask);
}

void GL_APIENTRY glStencilStrokePathCHROMIUMContextANGLE(GLeglContext context,
                                                         GLuint path,
                                                         GLint reference,
                                                         GLuint mask)
{
    gl::StencilStrokePathCHROMIUMContextANGLE(context, path, reference, mask);
}

void GL_APIENTRY glCoverFillPathCHROMIUMContextANGLE(GLeglContext context,
                                                     GLuint path,
                                                     GLenum coverMode)
{
    gl::CoverFillPathCHROMIUMContextANGLE(context, path, coverMode);
}

void GL_APIENTRY glCoverStrokePathCHROMIUMContextANGLE(GLeglContext context,
                                                       GLuint path,
                                                       GLenum coverMode)
{
    gl::CoverStrokePathCHROMIUMContextANGLE(context, path, coverMode);
}

void GL_APIENTRY glStencilThenCoverFillPathCHROMIUMContextANGLE(GLeglContext context,
                                                                GLuint path,
                                                                GLenum fillMode,
                                                                GLuint mask,
                                                                GLenum coverMode)
{
    gl::StencilThenCoverFillPathCHROMIUMContextANGLE(context, path, fillMode, mask, coverMode);
}

void GL_APIENTRY glStencilThenCoverStrokePathCHROMIUMContextANGLE(GLeglContext context,
                                                                  GLuint path,
                                                                  GLint reference,
                                                                  GLuint mask,
                                                                  GLenum coverMode)
{
    gl::StencilThenCoverStrokePathCHROMIUMContextANGLE(context, path, reference, mask, coverMode);
}

void GL_APIENTRY glCoverFillPathInstancedCHROMIUMContextANGLE(GLeglContext context,
                                                              GLsizei numPaths,
                                                              GLenum pathNameType,
                                                              const void *paths,
                                                              GLuint pathBase,
                                                              GLenum coverMode,
                                                              GLenum transformType,
                                                              const GLfloat *transformValues)
{
    gl::CoverFillPathInstancedCHROMIUMContextANGLE(context, numPaths, pathNameType, paths, pathBase,
                                                   coverMode, transformType, transformValues);
}

void GL_APIENTRY glCoverStrokePathInstancedCHROMIUMContextANGLE(GLeglContext context,
                                                                GLsizei numPaths,
                                                                GLenum pathNameType,
                                                                const void *paths,
                                                                GLuint pathBase,
                                                                GLenum coverMode,
                                                                GLenum transformType,
                                                                const GLfloat *transformValues)
{
    gl::CoverStrokePathInstancedCHROMIUMContextANGLE(context, numPaths, pathNameType, paths,
                                                     pathBase, coverMode, transformType,
                                                     transformValues);
}

void GL_APIENTRY glStencilFillPathInstancedCHROMIUMContextANGLE(GLeglContext context,
                                                                GLsizei numPaths,
                                                                GLenum pathNameType,
                                                                const void *paths,
                                                                GLuint pathBase,
                                                                GLenum fillMode,
                                                                GLuint mask,
                                                                GLenum transformType,
                                                                const GLfloat *transformValues)
{
    gl::StencilFillPathInstancedCHROMIUMContextANGLE(context, numPaths, pathNameType, paths,
                                                     pathBase, fillMode, mask, transformType,
                                                     transformValues);
}

void GL_APIENTRY glStencilStrokePathInstancedCHROMIUMContextANGLE(GLeglContext context,
                                                                  GLsizei numPaths,
                                                                  GLenum pathNameType,
                                                                  const void *paths,
                                                                  GLuint pathBase,
                                                                  GLint reference,
                                                                  GLuint mask,
                                                                  GLenum transformType,
                                                                  const GLfloat *transformValues)
{
    gl::StencilStrokePathInstancedCHROMIUMContextANGLE(context, numPaths, pathNameType, paths,
                                                       pathBase, reference, mask, transformType,
                                                       transformValues);
}

void GL_APIENTRY
glStencilThenCoverFillPathInstancedCHROMIUMContextANGLE(GLeglContext context,
                                                        GLsizei numPaths,
                                                        GLenum pathNameType,
                                                        const void *paths,
                                                        GLuint pathBase,
                                                        GLenum fillMode,
                                                        GLuint mask,
                                                        GLenum coverMode,
                                                        GLenum transformType,
                                                        const GLfloat *transformValues)
{
    gl::StencilThenCoverFillPathInstancedCHROMIUMContextANGLE(
        context, numPaths, pathNameType, paths, pathBase, fillMode, mask, coverMode, transformType,
        transformValues);
}

void GL_APIENTRY
glStencilThenCoverStrokePathInstancedCHROMIUMContextANGLE(GLeglContext context,
                                                          GLsizei numPaths,
                                                          GLenum pathNameType,
                                                          const void *paths,
                                                          GLuint pathBase,
                                                          GLint reference,
                                                          GLuint mask,
                                                          GLenum coverMode,
                                                          GLenum transformType,
                                                          const GLfloat *transformValues)
{
    gl::StencilThenCoverStrokePathInstancedCHROMIUMContextANGLE(
        context, numPaths, pathNameType, paths, pathBase, reference, mask, coverMode, transformType,
        transformValues);
}

void GL_APIENTRY glBindFragmentInputLocationCHROMIUMContextANGLE(GLeglContext context,
                                                                 GLuint program,
                                                                 GLint location,
                                                                 const GLchar *name)
{
    gl::BindFragmentInputLocationCHROMIUMContextANGLE(context, program, location, name);
}

void GL_APIENTRY glProgramPathFragmentInputGenCHROMIUMContextANGLE(GLeglContext context,
                                                                   GLuint program,
                                                                   GLint location,
                                                                   GLenum genMode,
                                                                   GLint components,
                                                                   const GLfloat *coeffs)
{
    gl::ProgramPathFragmentInputGenCHROMIUMContextANGLE(context, program, location, genMode,
                                                        components, coeffs);
}

// GLES 3.1
void GL_APIENTRY glDispatchComputeContextANGLE(GLeglContext context,
                                               GLuint numGroupsX,
                                               GLuint numGroupsY,
                                               GLuint numGroupsZ)
{
    gl::DispatchComputeContextANGLE(context, numGroupsX, numGroupsY, numGroupsZ);
}

void GL_APIENTRY glDispatchComputeIndirectContextANGLE(GLeglContext context, GLintptr indirect)
{
    gl::DispatchComputeIndirectContextANGLE(context, indirect);
}

void GL_APIENTRY glDrawArraysIndirectContextANGLE(GLeglContext context,
                                                  GLenum mode,
                                                  const void *indirect)
{
    gl::DrawArraysIndirectContextANGLE(context, mode, indirect);
}

void GL_APIENTRY glDrawElementsIndirectContextANGLE(GLeglContext context,
                                                    GLenum mode,
                                                    GLenum type,
                                                    const void *indirect)
{
    gl::DrawElementsIndirectContextANGLE(context, mode, type, indirect);
}

void GL_APIENTRY glFramebufferParameteriContextANGLE(GLeglContext context,
                                                     GLenum target,
                                                     GLenum pname,
                                                     GLint param)
{
    gl::FramebufferParameteriContextANGLE(context, target, pname, param);
}

void GL_APIENTRY glGetFramebufferParameterivContextANGLE(GLeglContext context,
                                                         GLenum target,
                                                         GLenum pname,
                                                         GLint *params)
{
    gl::GetFramebufferParameterivContextANGLE(context, target, pname, params);
}

void GL_APIENTRY glGetProgramInterfaceivContextANGLE(GLeglContext context,
                                                     GLuint program,
                                                     GLenum programInterface,
                                                     GLenum pname,
                                                     GLint *params)
{
    gl::GetProgramInterfaceivContextANGLE(context, program, programInterface, pname, params);
}

GLuint GL_APIENTRY glGetProgramResourceIndexContextANGLE(GLeglContext context,
                                                         GLuint program,
                                                         GLenum programInterface,
                                                         const GLchar *name)
{
    return gl::GetProgramResourceIndexContextANGLE(context, program, programInterface, name);
}

void GL_APIENTRY glGetProgramResourceNameContextANGLE(GLeglContext context,
                                                      GLuint program,
                                                      GLenum programInterface,
                                                      GLuint index,
                                                      GLsizei bufSize,
                                                      GLsizei *length,
                                                      GLchar *name)
{
    gl::GetProgramResourceNameContextANGLE(context, program, programInterface, index, bufSize,
                                           length, name);
}

void GL_APIENTRY glGetProgramResourceivContextANGLE(GLeglContext context,
                                                    GLuint program,
                                                    GLenum programInterface,
                                                    GLuint index,
                                                    GLsizei propCount,
                                                    const GLenum *props,
                                                    GLsizei bufSize,
                                                    GLsizei *length,
                                                    GLint *params)
{
    gl::GetProgramResourceivContextANGLE(context, program, programInterface, index, propCount,
                                         props, bufSize, length, params);
}

GLint GL_APIENTRY glGetProgramResourceLocationContextANGLE(GLeglContext context,
                                                           GLuint program,
                                                           GLenum programInterface,
                                                           const GLchar *name)
{
    return gl::GetProgramResourceLocationContextANGLE(context, program, programInterface, name);
}

void GL_APIENTRY glUseProgramStagesContextANGLE(GLeglContext context,
                                                GLuint pipeline,
                                                GLbitfield stages,
                                                GLuint program)
{
    gl::UseProgramStagesContextANGLE(context, pipeline, stages, program);
}

void GL_APIENTRY glActiveShaderProgramContextANGLE(GLeglContext context,
                                                   GLuint pipeline,
                                                   GLuint program)
{
    gl::ActiveShaderProgramContextANGLE(context, pipeline, program);
}

GLuint GL_APIENTRY glCreateShaderProgramvContextANGLE(GLeglContext context,
                                                      GLenum type,
                                                      GLsizei count,
                                                      const GLchar *const *strings)
{
    return gl::CreateShaderProgramvContextANGLE(context, type, count, strings);
}

void GL_APIENTRY glBindProgramPipelineContextANGLE(GLeglContext context, GLuint pipeline)
{
    gl::BindProgramPipelineContextANGLE(context, pipeline);
}

void GL_APIENTRY glDeleteProgramPipelinesContextANGLE(GLeglContext context,
                                                      GLsizei n,
                                                      const GLuint *pipelines)
{
    gl::DeleteProgramPipelinesContextANGLE(context, n, pipelines);
}

void GL_APIENTRY glGenProgramPipelinesContextANGLE(GLeglContext context,
                                                   GLsizei n,
                                                   GLuint *pipelines)
{
    gl::GenProgramPipelinesContextANGLE(context, n, pipelines);
}

GLboolean GL_APIENTRY glIsProgramPipelineContextANGLE(GLeglContext context, GLuint pipeline)
{
    return gl::IsProgramPipelineContextANGLE(context, pipeline);
}

void GL_APIENTRY glGetProgramPipelineivContextANGLE(GLeglContext context,
                                                    GLuint pipeline,
                                                    GLenum pname,
                                                    GLint *params)
{
    gl::GetProgramPipelineivContextANGLE(context, pipeline, pname, params);
}

void GL_APIENTRY glProgramUniform1iContextANGLE(GLeglContext context,
                                                GLuint program,
                                                GLint location,
                                                GLint v0)
{
    gl::ProgramUniform1iContextANGLE(context, program, location, v0);
}

void GL_APIENTRY glProgramUniform2iContextANGLE(GLeglContext context,
                                                GLuint program,
                                                GLint location,
                                                GLint v0,
                                                GLint v1)
{
    gl::ProgramUniform2iContextANGLE(context, program, location, v0, v1);
}

void GL_APIENTRY glProgramUniform3iContextANGLE(GLeglContext context,
                                                GLuint program,
                                                GLint location,
                                                GLint v0,
                                                GLint v1,
                                                GLint v2)
{
    gl::ProgramUniform3iContextANGLE(context, program, location, v0, v1, v2);
}

void GL_APIENTRY glProgramUniform4iContextANGLE(GLeglContext context,
                                                GLuint program,
                                                GLint location,
                                                GLint v0,
                                                GLint v1,
                                                GLint v2,
                                                GLint v3)
{
    gl::ProgramUniform4iContextANGLE(context, program, location, v0, v1, v2, v3);
}

void GL_APIENTRY glProgramUniform1uiContextANGLE(GLeglContext context,
                                                 GLuint program,
                                                 GLint location,
                                                 GLuint v0)
{
    gl::ProgramUniform1uiContextANGLE(context, program, location, v0);
}

void GL_APIENTRY glProgramUniform2uiContextANGLE(GLeglContext context,
                                                 GLuint program,
                                                 GLint location,
                                                 GLuint v0,
                                                 GLuint v1)
{
    gl::ProgramUniform2uiContextANGLE(context, program, location, v0, v1);
}

void GL_APIENTRY glProgramUniform3uiContextANGLE(GLeglContext context,
                                                 GLuint program,
                                                 GLint location,
                                                 GLuint v0,
                                                 GLuint v1,
                                                 GLuint v2)
{
    gl::ProgramUniform3uiContextANGLE(context, program, location, v0, v1, v2);
}

void GL_APIENTRY glProgramUniform4uiContextANGLE(GLeglContext context,
                                                 GLuint program,
                                                 GLint location,
                                                 GLuint v0,
                                                 GLuint v1,
                                                 GLuint v2,
                                                 GLuint v3)
{
    gl::ProgramUniform4uiContextANGLE(context, program, location, v0, v1, v2, v3);
}

void GL_APIENTRY glProgramUniform1fContextANGLE(GLeglContext context,
                                                GLuint program,
                                                GLint location,
                                                GLfloat v0)
{
    gl::ProgramUniform1fContextANGLE(context, program, location, v0);
}

void GL_APIENTRY glProgramUniform2fContextANGLE(GLeglContext context,
                                                GLuint program,
                                                GLint location,
                                                GLfloat v0,
                                                GLfloat v1)
{
    gl::ProgramUniform2fContextANGLE(context, program, location, v0, v1);
}

void GL_APIENTRY glProgramUniform3fContextANGLE(GLeglContext context,
                                                GLuint program,
                                                GLint location,
                                                GLfloat v0,
                                                GLfloat v1,
                                                GLfloat v2)
{
    gl::ProgramUniform3fContextANGLE(context, program, location, v0, v1, v2);
}

void GL_APIENTRY glProgramUniform4fContextANGLE(GLeglContext context,
                                                GLuint program,
                                                GLint location,
                                                GLfloat v0,
                                                GLfloat v1,
                                                GLfloat v2,
                                                GLfloat v3)
{
    gl::ProgramUniform4fContextANGLE(context, program, location, v0, v1, v2, v3);
}

void GL_APIENTRY glProgramUniform1ivContextANGLE(GLeglContext context,
                                                 GLuint program,
                                                 GLint location,
                                                 GLsizei count,
                                                 const GLint *value)
{
    gl::ProgramUniform1ivContextANGLE(context, program, location, count, value);
}

void GL_APIENTRY glProgramUniform2ivContextANGLE(GLeglContext context,
                                                 GLuint program,
                                                 GLint location,
                                                 GLsizei count,
                                                 const GLint *value)
{
    gl::ProgramUniform2ivContextANGLE(context, program, location, count, value);
}

void GL_APIENTRY glProgramUniform3ivContextANGLE(GLeglContext context,
                                                 GLuint program,
                                                 GLint location,
                                                 GLsizei count,
                                                 const GLint *value)
{
    gl::ProgramUniform3ivContextANGLE(context, program, location, count, value);
}

void GL_APIENTRY glProgramUniform4ivContextANGLE(GLeglContext context,
                                                 GLuint program,
                                                 GLint location,
                                                 GLsizei count,
                                                 const GLint *value)
{
    gl::ProgramUniform4ivContextANGLE(context, program, location, count, value);
}

void GL_APIENTRY glProgramUniform1uivContextANGLE(GLeglContext context,
                                                  GLuint program,
                                                  GLint location,
                                                  GLsizei count,
                                                  const GLuint *value)
{
    gl::ProgramUniform1uivContextANGLE(context, program, location, count, value);
}

void GL_APIENTRY glProgramUniform2uivContextANGLE(GLeglContext context,
                                                  GLuint program,
                                                  GLint location,
                                                  GLsizei count,
                                                  const GLuint *value)
{
    gl::ProgramUniform2uivContextANGLE(context, program, location, count, value);
}

void GL_APIENTRY glProgramUniform3uivContextANGLE(GLeglContext context,
                                                  GLuint program,
                                                  GLint location,
                                                  GLsizei count,
                                                  const GLuint *value)
{
    gl::ProgramUniform3uivContextANGLE(context, program, location, count, value);
}

void GL_APIENTRY glProgramUniform4uivContextANGLE(GLeglContext context,
                                                  GLuint program,
                                                  GLint location,
                                                  GLsizei count,
                                                  const GLuint *value)
{
    gl::ProgramUniform4uivContextANGLE(context, program, location, count, value);
}

void GL_APIENTRY glProgramUniform1fvContextANGLE(GLeglContext context,
                                                 GLuint program,
                                                 GLint location,
                                                 GLsizei count,
                                                 const GLfloat *value)
{
    gl::ProgramUniform1fvContextANGLE(context, program, location, count, value);
}

void GL_APIENTRY glProgramUniform2fvContextANGLE(GLeglContext context,
                                                 GLuint program,
                                                 GLint location,
                                                 GLsizei count,
                                                 const GLfloat *value)
{
    gl::ProgramUniform2fvContextANGLE(context, program, location, count, value);
}

void GL_APIENTRY glProgramUniform3fvContextANGLE(GLeglContext context,
                                                 GLuint program,
                                                 GLint location,
                                                 GLsizei count,
                                                 const GLfloat *value)
{
    gl::ProgramUniform3fvContextANGLE(context, program, location, count, value);
}

void GL_APIENTRY glProgramUniform4fvContextANGLE(GLeglContext context,
                                                 GLuint program,
                                                 GLint location,
                                                 GLsizei count,
                                                 const GLfloat *value)
{
    gl::ProgramUniform4fvContextANGLE(context, program, location, count, value);
}

void GL_APIENTRY glProgramUniformMatrix2fvContextANGLE(GLeglContext context,
                                                       GLuint program,
                                                       GLint location,
                                                       GLsizei count,
                                                       GLboolean transpose,
                                                       const GLfloat *value)
{
    gl::ProgramUniformMatrix2fvContextANGLE(context, program, location, count, transpose, value);
}

void GL_APIENTRY glProgramUniformMatrix3fvContextANGLE(GLeglContext context,
                                                       GLuint program,
                                                       GLint location,
                                                       GLsizei count,
                                                       GLboolean transpose,
                                                       const GLfloat *value)
{
    gl::ProgramUniformMatrix3fvContextANGLE(context, program, location, count, transpose, value);
}

void GL_APIENTRY glProgramUniformMatrix4fvContextANGLE(GLeglContext context,
                                                       GLuint program,
                                                       GLint location,
                                                       GLsizei count,
                                                       GLboolean transpose,
                                                       const GLfloat *value)
{
    gl::ProgramUniformMatrix4fvContextANGLE(context, program, location, count, transpose, value);
}

void GL_APIENTRY glProgramUniformMatrix2x3fvContextANGLE(GLeglContext context,
                                                         GLuint program,
                                                         GLint location,
                                                         GLsizei count,
                                                         GLboolean transpose,
                                                         const GLfloat *value)
{
    gl::ProgramUniformMatrix2x3fvContextANGLE(context, program, location, count, transpose, value);
}

void GL_APIENTRY glProgramUniformMatrix3x2fvContextANGLE(GLeglContext context,
                                                         GLuint program,
                                                         GLint location,
                                                         GLsizei count,
                                                         GLboolean transpose,
                                                         const GLfloat *value)
{
    gl::ProgramUniformMatrix3x2fvContextANGLE(context, program, location, count, transpose, value);
}

void GL_APIENTRY glProgramUniformMatrix2x4fvContextANGLE(GLeglContext context,
                                                         GLuint program,
                                                         GLint location,
                                                         GLsizei count,
                                                         GLboolean transpose,
                                                         const GLfloat *value)
{
    gl::ProgramUniformMatrix2x4fvContextANGLE(context, program, location, count, transpose, value);
}

void GL_APIENTRY glProgramUniformMatrix4x2fvContextANGLE(GLeglContext context,
                                                         GLuint program,
                                                         GLint location,
                                                         GLsizei count,
                                                         GLboolean transpose,
                                                         const GLfloat *value)
{
    gl::ProgramUniformMatrix4x2fvContextANGLE(context, program, location, count, transpose, value);
}

void GL_APIENTRY glProgramUniformMatrix3x4fvContextANGLE(GLeglContext context,
                                                         GLuint program,
                                                         GLint location,
                                                         GLsizei count,
                                                         GLboolean transpose,
                                                         const GLfloat *value)
{
    gl::ProgramUniformMatrix3x4fvContextANGLE(context, program, location, count, transpose, value);
}

void GL_APIENTRY glProgramUniformMatrix4x3fvContextANGLE(GLeglContext context,
                                                         GLuint program,
                                                         GLint location,
                                                         GLsizei count,
                                                         GLboolean transpose,
                                                         const GLfloat *value)
{
    gl::ProgramUniformMatrix4x3fvContextANGLE(context, program, location, count, transpose, value);
}

void GL_APIENTRY glValidateProgramPipelineContextANGLE(GLeglContext context, GLuint pipeline)
{
    gl::ValidateProgramPipelineContextANGLE(context, pipeline);
}

void GL_APIENTRY glGetProgramPipelineInfoLogContextANGLE(GLeglContext context,
                                                         GLuint pipeline,
                                                         GLsizei bufSize,
                                                         GLsizei *length,
                                                         GLchar *infoLog)
{
    gl::GetProgramPipelineInfoLogContextANGLE(context, pipeline, bufSize, length, infoLog);
}

void GL_APIENTRY glBindImageTextureContextANGLE(GLeglContext context,
                                                GLuint unit,
                                                GLuint texture,
                                                GLint level,
                                                GLboolean layered,
                                                GLint layer,
                                                GLenum access,
                                                GLenum format)
{
    gl::BindImageTextureContextANGLE(context, unit, texture, level, layered, layer, access, format);
}

void GL_APIENTRY glGetBooleani_vContextANGLE(GLeglContext context,
                                             GLenum target,
                                             GLuint index,
                                             GLboolean *data)
{
    gl::GetBooleani_vContextANGLE(context, target, index, data);
}

void GL_APIENTRY glMemoryBarrierContextANGLE(GLeglContext context, GLbitfield barriers)
{
    gl::MemoryBarrierContextANGLE(context, barriers);
}

void GL_APIENTRY glMemoryBarrierByRegionContextANGLE(GLeglContext context, GLbitfield barriers)
{
    gl::MemoryBarrierByRegionContextANGLE(context, barriers);
}

void GL_APIENTRY glTexStorage2DMultisampleContextANGLE(GLeglContext context,
                                                       GLenum target,
                                                       GLsizei samples,
                                                       GLenum internalformat,
                                                       GLsizei width,
                                                       GLsizei height,
                                                       GLboolean fixedsamplelocations)
{
    gl::TexStorage2DMultisampleContextANGLE(context, target, samples, internalformat, width, height,
                                            fixedsamplelocations);
}

void GL_APIENTRY glGetMultisamplefvContextANGLE(GLeglContext context,
                                                GLenum pname,
                                                GLuint index,
                                                GLfloat *val)
{
    gl::GetMultisamplefvContextANGLE(context, pname, index, val);
}

void GL_APIENTRY glSampleMaskiContextANGLE(GLeglContext context, GLuint maskNumber, GLbitfield mask)
{
    gl::SampleMaskiContextANGLE(context, maskNumber, mask);
}

void GL_APIENTRY glGetTexLevelParameterivContextANGLE(GLeglContext context,
                                                      GLenum target,
                                                      GLint level,
                                                      GLenum pname,
                                                      GLint *params)
{
    gl::GetTexLevelParameterivContextANGLE(context, target, level, pname, params);
}

void GL_APIENTRY glGetTexLevelParameterfvContextANGLE(GLeglContext context,
                                                      GLenum target,
                                                      GLint level,
                                                      GLenum pname,
                                                      GLfloat *params)
{
    gl::GetTexLevelParameterfvContextANGLE(context, target, level, pname, params);
}

void GL_APIENTRY glBindVertexBufferContextANGLE(GLeglContext context,
                                                GLuint bindingindex,
                                                GLuint buffer,
                                                GLintptr offset,
                                                GLsizei stride)
{
    gl::BindVertexBufferContextANGLE(context, bindingindex, buffer, offset, stride);
}

void GL_APIENTRY glVertexAttribFormatContextANGLE(GLeglContext context,
                                                  GLuint attribindex,
                                                  GLint size,
                                                  GLenum type,
                                                  GLboolean normalized,
                                                  GLuint relativeoffset)
{
    gl::VertexAttribFormatContextANGLE(context, attribindex, size, type, normalized,
                                       relativeoffset);
}

void GL_APIENTRY glVertexAttribIFormatContextANGLE(GLeglContext context,
                                                   GLuint attribindex,
                                                   GLint size,
                                                   GLenum type,
                                                   GLuint relativeoffset)
{
    gl::VertexAttribIFormatContextANGLE(context, attribindex, size, type, relativeoffset);
}

void GL_APIENTRY glVertexAttribBindingContextANGLE(GLeglContext context,
                                                   GLuint attribindex,
                                                   GLuint bindingindex)
{
    gl::VertexAttribBindingContextANGLE(context, attribindex, bindingindex);
}

void GL_APIENTRY glVertexBindingDivisorContextANGLE(GLeglContext context,
                                                    GLuint bindingindex,
                                                    GLuint divisor)
{
    gl::VertexBindingDivisorContextANGLE(context, bindingindex, divisor);
}

void GL_APIENTRY glFramebufferTextureMultiviewLayeredANGLEContextANGLE(GLeglContext context,
                                                                       GLenum target,
                                                                       GLenum attachment,
                                                                       GLuint texture,
                                                                       GLint level,
                                                                       GLint baseViewIndex,
                                                                       GLsizei numViews)
{
    gl::FramebufferTextureMultiviewLayeredANGLEContextANGLE(context, target, attachment, texture,
                                                            level, baseViewIndex, numViews);
}

void GL_APIENTRY
glFramebufferTextureMultiviewSideBySideANGLEContextANGLE(GLeglContext context,
                                                         GLenum target,
                                                         GLenum attachment,
                                                         GLuint texture,
                                                         GLint level,
                                                         GLsizei numViews,
                                                         const GLint *viewportOffsets)
{
    gl::FramebufferTextureMultiviewSideBySideANGLEContextANGLE(context, target, attachment, texture,
                                                               level, numViews, viewportOffsets);
}

void GL_APIENTRY glRequestExtensionANGLEContextANGLE(GLeglContext context, const GLchar *name)
{
    gl::RequestExtensionANGLEContextANGLE(context, name);
}

#endif /* EGL_ANGLE_explicit_context */

}  // extern "C"
