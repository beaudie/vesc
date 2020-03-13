//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// util_gl.cpp: Helpers for ANGLE tests that use common GL patterns.

#include "util/util_gl.h"

#include "common/utilities.h"

#include <iostream>

namespace angle
{
namespace
{
void KHRONOS_APIENTRY DebugMessageCallback(GLenum source,
                                           GLenum type,
                                           GLuint id,
                                           GLenum severity,
                                           GLsizei length,
                                           const GLchar *message,
                                           const void *userParam)
{
    std::string sourceText   = gl::GetDebugMessageSourceString(source);
    std::string typeText     = gl::GetDebugMessageTypeString(type);
    std::string severityText = gl::GetDebugMessageSeverityString(severity);
    std::cerr << sourceText << ", " << typeText << ", " << severityText << ": " << message << "\n";
}
}  // namespace

void InitDebugMessageCallback(const void *userParam)
{
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    // Enable medium and high priority messages.
    glDebugMessageControlKHR(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr,
                             GL_TRUE);
    glDebugMessageControlKHR(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr,
                             GL_TRUE);
    // Disable low and notification priority messages.
    glDebugMessageControlKHR(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr,
                             GL_FALSE);
    glDebugMessageControlKHR(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr,
                             GL_FALSE);
    // Disable medium priority performance messages to reduce spam.
    glDebugMessageControlKHR(GL_DONT_CARE, GL_DEBUG_TYPE_PERFORMANCE, GL_DEBUG_SEVERITY_MEDIUM, 0,
                             nullptr, GL_FALSE);
    glDebugMessageCallbackKHR(DebugMessageCallback, userParam);
}
}  // namespace angle