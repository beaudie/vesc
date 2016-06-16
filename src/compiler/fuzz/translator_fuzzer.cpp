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

static std::unordered_map<uint64_t, TCompiler*> translators;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (!ShInitialize()) {
        return 0;
    }

    if (size < 12) {
        return 0;
    }

    uint32_t type = *reinterpret_cast<const uint32_t*>(data);
    uint32_t spec = *reinterpret_cast<const uint32_t*>(data + 4);
    uint32_t options = *reinterpret_cast<const uint32_t*>(data + 8);

    size -= 12;
    data += 12;

    uint64_t key = type;
    key = key << 32 | spec;

    if (type != GL_FRAGMENT_SHADER && type != GL_VERTEX_SHADER) {
        return 0;
    }

    if (spec != SH_GLES2_SPEC && type != SH_WEBGL_SPEC && spec != SH_GLES3_SPEC && spec != SH_WEBGL2_SPEC) {
        return 0;
    }

    // TODO check string is alphanum otherwise the fuzzer buts the first twelve bytes in the rest when it is useless
    // TODO use a different pool allocator for the fuzzing? (seems that the fuzzer could not see some things because of the pool allocator)

    if (translators.find(key) == translators.end()) {
        TCompiler *translator = ConstructCompiler(type, static_cast<ShShaderSpec>(spec), SH_ESSL_OUTPUT);

        if (!translator) {
            return 0;
        }

        ShBuiltInResources resources;
        ShInitBuiltInResources(&resources);

        // Enable all the extensions to have more coverage
        resources.OES_standard_derivatives = 1;
        resources.OES_EGL_image_external = 1;
        resources.OES_EGL_image_external_essl3 = 1;
        resources.NV_EGL_stream_consumer_external = 1;
        resources.ARB_texture_rectangle = 1;
        resources.EXT_blend_func_extended = 1;
        resources.EXT_draw_buffers = 1;
        resources.EXT_frag_depth = 1;
        resources.EXT_shader_texture_lod = 1;
        resources.WEBGL_debug_shader_precision = 1;
        resources.EXT_shader_framebuffer_fetch = 1;
        resources.NV_shader_framebuffer_fetch = 1;
        resources.ARM_shader_framebuffer_fetch = 1;


        if (!translator->Init(resources))
        {
            DeleteCompiler(translator);
            return 0;
        }

        translators[key] = translator;
    }

    TCompiler* translator = translators[key];

    std::string shader;
    shader.assign(reinterpret_cast<const char*>(data), size);
    const char *shaderStrings[] = {shader.c_str()};

    translator->compile(shaderStrings, 1, options);
    return 0;
}
