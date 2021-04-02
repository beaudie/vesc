//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Sets up the GL bindings for the trace tests.

#ifndef ANGLE_TRACE_GL_H_
#define ANGLE_TRACE_GL_H_

#include "trace_egl_loader_autogen.h"
#include "trace_gles_loader_autogen.h"

// For CPU performance benchmark
#if defined(glViewport)
#    undef glViewport
#endif
#define glViewport(x, y, width, height) t_glViewport(x, y, 1, 1)

#if defined(glDrawElements)
#    undef glDrawElements
#endif
#define glDrawElements(mode, count, type, indices) t_glDrawElements(GL_POINTS, 1, type, indices)

#if defined(glDrawElementsInstanced)
#    undef glDrawElementsInstanced
#endif
#define glDrawElementsInstanced(mode, count, type, indices, instancecount) \
    t_glDrawElementsInstanced(GL_POINTS, 1, type, indices, 1)

#if defined(glDrawElementsBaseVertexOES)
#    undef glDrawElementsBaseVertexOES
#endif
#define glDrawElementsBaseVertexOES(mode, count, type, indices, basevertex) \
    t_glDrawElementsBaseVertexOES(GL_POINTS, 1, type, indices, basevertex)

#if defined(glDrawRangeElements)
#    undef glDrawRangeElements
#endif
#define glDrawRangeElements(mode, start, end, count, type, indices) \
    t_glDrawRangeElements(GL_POINTS, start, end, 1, type, indices)

#if defined(glDrawArrays)
#    undef glDrawArrays
#endif
#define glDrawArrays(mode, first, count) t_glDrawArrays(GL_POINTS, first, 1)

#if defined(glDrawArraysInstanced)
#    undef glDrawArraysInstanced
#endif
#define glDrawArraysInstanced(mode, first, count, instancecount) \
    t_glDrawArraysInstanced(GL_POINTS, first, 1, 1)

#if defined(glDrawArraysIndirect)
#    undef glDrawArraysIndirect
#endif
#define glDrawArraysIndirect(mode, indirect) t_glDrawArraysInstanced(GL_POINTS, 0, 1, 1)

#if defined(glDispatchCompute)
#    undef glDispatchCompute
#endif
#define glDispatchCompute(num_groups_x, num_groups_y, num_groups_z) t_glDispatchCompute(1, 1, 1)

// Minimize the data copying APIs
#if defined(glBufferData)
#    undef glBufferData
#endif
#define glBufferData(target, size, data, usage) t_glBufferData(target, size, nullptr, usage)

#if defined(glBufferSubData)
#    undef glBufferSubData
#endif
#define glBufferSubData(target, offset, size, data) t_glBufferSubData(target, offset, 1, data)

#if defined(glTexImage2D)
#    undef glTexImage2D
#endif
#define glTexImage2D(target, level, internalformat, width, height, border, format, type, data) \
    t_glTexImage2D(target, level, internalformat, width, height, border, format, type, nullptr)

#if defined(glTexImage3D)
#    undef glTexImage3D
#endif
#define glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, \
                     data)                                                                      \
    t_glTexImage3D(target, level, internalformat, width, height, depth, border, format, type,   \
                   nullptr)

#if defined(glTexSubImage2D)
#    undef glTexSubImage2D
#endif
#define glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels) \
    t_glTexSubImage2D(target, level, xoffset, yoffset, 1, 1, format, type, pixels)

#if defined(glTexSubImage3D)
#    undef glTexSubImage3D
#endif
#define glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, \
                        type, pixels)                                                           \
    t_glTexSubImage3D(target, level, xoffset, yoffset, zoffset, 1, 1, 1, format, type, pixels)

// Noop glGenerateMipmap
#if defined(glGenerateMipmap)
#    undef glGenerateMipmap
#endif
#define glGenerateMipmap(target)

#endif  // ANGLE_TRACE_GL_H_
