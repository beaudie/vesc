//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// wgpu_wgsl_util.h: Utilities to manipulate previously translated WGSL.
//

#ifndef LIBANGLE_RENDERER_WGPU_WGPU_WGSL_UTIL_H_
#define LIBANGLE_RENDERER_WGPU_WGPU_WGSL_UTIL_H_

#include "libANGLE/Program.h"

namespace rx
{
namespace webgpu
{
// Replaces location markers in the WGSL source with actual locations, for
// `shaderVars` which is a vector of either gl::ProgramInputs or gl::ProgramOutputs.
void WgslAssignLocations(std::string &shaderSource,
                         const std::vector<gl::ProgramInput> &shaderVars);
void WgslAssignLocations(std::string &shaderSource,
                         const std::vector<gl::ProgramOutput> &shaderVars);
void WgslAssignLocations(std::string &shaderSource,
                         const gl::ProgramMergedVaryings &mergedVaryings);
}  // namespace webgpu
}  // namespace rx

#endif /* LIBANGLE_RENDERER_WGPU_WGPU_WGSL_UTIL_H_ */
