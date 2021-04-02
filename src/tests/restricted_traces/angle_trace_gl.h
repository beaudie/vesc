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
#define glDrawArraysIndirect(mode, indirect) t_glDrawArraysIndirect(GL_POINTS, indirect)

#if defined(glDispatchCompute)
#    undef glDispatchCompute
#endif
#define glDispatchCompute(num_groups_x, num_groups_y, num_groups_z) t_glDispatchCompute(1, 1, 1)

#endif  // ANGLE_TRACE_GL_H_
