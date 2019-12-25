//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Wrapper for Khronos glslang compiler. This file is used by Vulkan and Metal backends.
//

#ifndef LIBANGLE_RENDERER_GLSLANG_WRAPPER_UTILS_H_
#define LIBANGLE_RENDERER_GLSLANG_WRAPPER_UTILS_H_

#include <functional>

#include "libANGLE/renderer/ProgramImpl.h"

namespace rx
{
enum class GlslangError
{
    InvalidShader,
    InvalidSpirv,
};

struct GlslangSourceOptions
{
    // Uniforms set index:
    uint32_t uniformsAndXfbDescriptorSetIndex = 0;
    // Textures set index:
    uint32_t textureDescriptorSetIndex = 1;
    // Other shader resources set index:
    uint32_t shaderResourceDescriptorSetIndex = 2;
    // ANGLE driver uniforms set index:
    uint32_t driverUniformsDescriptorSetIndex = 3;

    // Binding index start for transform feedback buffers:
    uint32_t xfbBindingIndexStart = 16;

    bool useOldRewriteStructSamplers        = false;
    bool supportsTransformFeedbackExtension = false;
    bool emulateTransformFeedback           = false;
};

struct SpirvPatchHunk
{
    // Replace words [|offset|, |offset|+|size|) in unpatched SPIR-V with |contents|.  |size| can be
    // empty, so can |contents|.
    size_t offset = 0;
    size_t size   = 0;
    std::vector<uint32_t> contents;
};

struct SpirvPatch
{
    // Any possible entry point interface variable ids that need to be added by the patch.
    // Note: Currently, there is no specialization that can remove interface variables.
    std::vector<uint32_t> entryPointAdditions;
    // Hunks to be replaced.  They must be sorted by |offset|.
    std::vector<SpirvPatchHunk> hunks;
};

struct SpirvShader
{
    // SPIR-V output without special variations.
    std::vector<uint32_t> code;
    // Location of OpEntryPoint, to facilitate SPIR-V patching.
    size_t entryPointOffset = 0;
    // SPIR-V patch for line raster emulation.
    SpirvPatch lineRasterEmulationPatch;
};

using GlslangErrorCallback = std::function<angle::Result(GlslangError)>;

void GlslangInitialize();
void GlslangRelease();

// Get the mapped sampler name after the soure is transformed by GlslangGetShaderSpirvCode()
std::string GlslangGetMappedSamplerName(const std::string &originalName);

// Transform the source to include actual binding points for various shader resources (textures,
// buffers, xfb, etc), then compile it and return the SPIR-V.  Specialization constants and varyings
// with a priori known names are used to mark pieces of code that are used under special
// circumstances.  These specialization instructions are kept separately as a "patch" to the
// non-specialized SPIR-V.
//
// This function should be called at link time.
angle::Result GlslangGetShaderSpirvCode(const GlslangSourceOptions &options,
                                        GlslangErrorCallback callback,
                                        const gl::Caps &glCaps,
                                        const gl::ProgramState &programState,
                                        const gl::ProgramLinkedResources &resources,
                                        gl::ShaderMap<SpirvShader> *spirvShadersOut);

// Specialize the SPIR-V with any possible patches (currently, only line raster emulation).
// Creates a copy of the non-specialized SPIR-V (as it assumes at least one specialization is
// necessary) and applies the necessary patches.  Note that since the patch hunks reference the
// offset in SPIR-V before specialization, all necessary specializations should be applied at once,
// and cannot be done one at a time.
//
// This function should be called at draw time, and is therefore expected to have minimal overhead,
// even though it's only called in special cases.
void GlslangGetSpecializedShaderSpirvCode(
    bool enableLineRasterEmulation,
    const gl::ShaderMap<SpirvShader> &spirvShaders,
    gl::ShaderMap<std::vector<uint32_t>> *specializedSpirvShadersOut);

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GLSLANG_WRAPPER_UTILS_H_
