//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramExecutableVk.cpp: Collects the information and interfaces common to both ProgramVks and
// ProgramPipelineVks in order to execute/draw with either.

#include "libANGLE/renderer/vulkan/ProgramExecutableVk.h"

namespace rx
{

ProgramExecutableVk::ProgramExecutableVk() {}

ProgramExecutableVk::~ProgramExecutableVk() = default;

void ProgramExecutableVk::reset()
{
    clearVariableInfoMap();
}

void ProgramExecutableVk::clearVariableInfoMap()
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        mVariableInfoMap[shaderType].clear();
    }
}

}  // namespace rx
