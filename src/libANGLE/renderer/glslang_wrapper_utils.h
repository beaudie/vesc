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
    bool emulateBresenhamLines              = false;
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
struct SpirvBlob
{
    // SPIR-V output without patches.
    std::vector<uint32_t> code;
    // Location of OpEntryPoint, to facilitate SPIR-V patching.
    size_t entryPointOffset = 0;
    // SPIR-V patch to bring back removed inactive varyings.
    SpirvPatch inactiveVaryingsPatch;
};

using GlslangErrorCallback = std::function<angle::Result(GlslangError)>;

// Information for each shader interface variable.  Not all fields are relevant to each shader
// interface variable.  For example opaque uniforms require a set and binding index, while vertex
// attributes require a location.
struct ShaderInterfaceVariableInfo
{
    ShaderInterfaceVariableInfo();

    static constexpr uint32_t kInvalid = std::numeric_limits<uint32_t>::max();

    // Used for interface blocks and opaque uniforms.
    uint32_t descriptorSet = kInvalid;
    uint32_t binding       = kInvalid;
    // Used for vertex attributes, fragment shader outputs and varyings.  There could be different
    // variables that share the same name, such as a vertex attribute and a fragment output.  They
    // will share this object since they have the same name, but will find possibly different
    // locations in their respective slots.
    gl::ShaderMap<uint32_t> location;
    gl::ShaderMap<uint32_t> component;
    // The stages this shader interface variable is active.
    gl::ShaderBitSet activeStages;
    // Used for transform feedback extension to decorate vertex shader output.
    uint32_t xfbBuffer = kInvalid;
    uint32_t xfbOffset = kInvalid;
    uint32_t xfbStride = kInvalid;
};

using ShaderInterfaceVariableInfoMap = std::unordered_map<std::string, ShaderInterfaceVariableInfo>;

void GlslangInitialize();
void GlslangRelease();

// Get the mapped sampler name after the soure is transformed by GlslangGetShaderSource()
std::string GlslangGetMappedSamplerName(const std::string &originalName);

// Transform the shader sources if necessary.  At the same time, shader interface variable
// qualifiers (set, binding, location etc) are stored in variableInfoMap to be set during a SPIR-V
// transformation.  This is a transitory step towards link-time shader compilation, at which point
// GlslangGetShaderSpirvCode will also be called by this function.
void GlslangGetShaderSource(const GlslangSourceOptions &options,
                            const gl::ProgramState &programState,
                            const gl::ProgramLinkedResources &resources,
                            gl::ShaderMap<std::string> *shaderSourcesOut,
                            ShaderInterfaceVariableInfoMap *variableInfoMapOut);

// Compile the shaders and apply a SPIR-V transformation.  Inactive varyings are removed and kept
// separately as a "patch" to the unpatched SPIR-V.  These varyings can be patched back in (using
// GlslangApplySpirvPatches) for Program Pipeline Objects, that may decide they are no longer
// inactive.
angle::Result GlslangGetShaderSpirvCode(GlslangErrorCallback callback,
                                        const gl::Caps &glCaps,
                                        const gl::ShaderMap<std::string> &shaderSources,
                                        const ShaderInterfaceVariableInfoMap &variableInfoMap,
                                        gl::ShaderMap<SpirvBlob> *spirvBlobsOut);

// Apply patches to the unpatched SPIR-V code (currently, only inactive varyings).  Creates a copy
// of the unpatched SPIR-V (as it assumes at least one patch is necessary) and applies the necessary
// patches.  Note that since the patch hunks reference the offset in SPIR-V before patching, all
// necessary patches should be applied at once, and cannot be done one at a time.
void GlslangApplySpirvPatches(bool inactiveVaryings,
                              const gl::ShaderMap<SpirvBlob> &spirvBlobs,
                              gl::ShaderMap<std::vector<uint32_t>> *patchedSpirvBlobsOut);

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GLSLANG_WRAPPER_UTILS_H_
