//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <stddef.h>
#include <stdint.h>
#include <unordered_map>
#include <iostream>

#include "compiler/translator/Compiler.h"
#include "angle_gl.h"

static std::unordered_map<uint64_t, TCompiler *> translators;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    // Reserve some size for future compile options
    const size_t kHeaderSize = 128;

    if (size <= kHeaderSize)
    {
        return 0;
    }

    // Make sure the rest of data will be a valid C string so that we don't have to copy it.
    if (data[size - 1] != 0)
    {
        return 0;
    }

    uint32_t type    = *reinterpret_cast<const uint32_t *>(data);
    uint32_t spec    = *reinterpret_cast<const uint32_t *>(data + 4);
    uint32_t output  = *reinterpret_cast<const uint32_t *>(data + 8);
    uint32_t options = *reinterpret_cast<const uint32_t *>(data + 12);

    if (type != GL_FRAGMENT_SHADER && type != GL_VERTEX_SHADER)
    {
        return 0;
    }

    if (spec != SH_GLES2_SPEC && type != SH_WEBGL_SPEC && spec != SH_GLES3_SPEC &&
        spec != SH_WEBGL2_SPEC)
    {
        return 0;
    }

    std::vector<uint32_t> validOutputs;
    validOutputs.push_back(SH_ESSL_OUTPUT);
    validOutputs.push_back(SH_GLSL_COMPATIBILITY_OUTPUT);
    validOutputs.push_back(SH_GLSL_130_OUTPUT);
    validOutputs.push_back(SH_GLSL_140_OUTPUT);
    validOutputs.push_back(SH_GLSL_150_CORE_OUTPUT);
    validOutputs.push_back(SH_GLSL_330_CORE_OUTPUT);
    validOutputs.push_back(SH_GLSL_400_CORE_OUTPUT);
    validOutputs.push_back(SH_GLSL_410_CORE_OUTPUT);
    validOutputs.push_back(SH_GLSL_420_CORE_OUTPUT);
    validOutputs.push_back(SH_GLSL_430_CORE_OUTPUT);
    validOutputs.push_back(SH_GLSL_440_CORE_OUTPUT);
    validOutputs.push_back(SH_GLSL_450_CORE_OUTPUT);
    validOutputs.push_back(SH_HLSL_OUTPUT);
    validOutputs.push_back(SH_HLSL9_OUTPUT);
    validOutputs.push_back(SH_HLSL11_OUTPUT);
    validOutputs.push_back(SH_HLSL_3_0_OUTPUT);
    validOutputs.push_back(SH_HLSL_4_1_OUTPUT);
    validOutputs.push_back(SH_HLSL_4_0_FL9_3_OUTPUT);
    bool found = false;
    for (auto valid : validOutputs)
    {
        found = found || valid == output;
    }
    if (!found)
    {
        return 0;
    }

    size -= kHeaderSize;
    data += kHeaderSize;

    if (!ShInitialize())
    {
        return 0;
    }

    TCompiler *translator =
        ConstructCompiler(type, static_cast<ShShaderSpec>(spec), SH_ESSL_OUTPUT);

    if (!translator)
    {
        return 0;
    }

    ShBuiltInResources resources;
    ShInitBuiltInResources(&resources);

    // Enable all the extensions to have more coverage
    resources.OES_standard_derivatives        = 1;
    resources.OES_EGL_image_external          = 1;
    resources.OES_EGL_image_external_essl3    = 1;
    resources.NV_EGL_stream_consumer_external = 1;
    resources.ARB_texture_rectangle           = 1;
    resources.EXT_blend_func_extended         = 1;
    resources.EXT_draw_buffers                = 1;
    resources.EXT_frag_depth                  = 1;
    resources.EXT_shader_texture_lod          = 1;
    resources.WEBGL_debug_shader_precision    = 1;
    resources.EXT_shader_framebuffer_fetch    = 1;
    resources.NV_shader_framebuffer_fetch     = 1;
    resources.ARM_shader_framebuffer_fetch    = 1;

    if (!translator->Init(resources))
    {
        DeleteCompiler(translator);
        return 0;
    }

    const char *shaderStrings[] = {reinterpret_cast<const char *>(data)};
    translator->compile(shaderStrings, 1, options);

    DeleteCompiler(translator);
    return 0;
}
