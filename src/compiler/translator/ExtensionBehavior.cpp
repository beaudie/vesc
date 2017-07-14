//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ExtensionBehavior.cpp: Extension name enumeration and data structures for storing extension
// behavior.

#include "compiler/translator/ExtensionBehavior.h"

#include "common/debug.h"

#define LIST_EXTENSIONS                \
    X(ARB_texture_rectangle)           \
    X(ARM_shader_framebuffer_fetch)    \
    X(EXT_blend_func_extended)         \
    X(EXT_draw_buffers)                \
    X(EXT_frag_depth)                  \
    X(EXT_shader_framebuffer_fetch)    \
    X(EXT_shader_texture_lod)          \
    X(EXT_YUV_target)                  \
    X(NV_EGL_stream_consumer_external) \
    X(NV_shader_framebuffer_fetch)     \
    X(OES_EGL_image_external)          \
    X(OES_EGL_image_external_essl3)    \
    X(OES_geometry_shader)             \
    X(OES_standard_derivatives)        \
    X(OVR_multiview)

namespace sh
{

const char *GetExtensionNameString(TExtension extension)
{
    switch (extension)
    {
#define X(ext)            \
    case TExtension::ext: \
        return "GL_" #ext;

        LIST_EXTENSIONS
#undef X
        default:
            UNREACHABLE();
            return "";
    }
}

TExtension GetExtensionByName(const char *extension)
{
    // If first characters of the extension don't equal "GL_", early out.
    if (strncmp(extension, "GL_", 3) != 0)
    {
        return TExtension::UNDEFINED;
    }
    const char *extWithoutGLPrefix = extension + 3;

#define X(ext)                                 \
    if (strcmp(extWithoutGLPrefix, #ext) == 0) \
    {                                          \
        return TExtension::ext;                \
    }
    LIST_EXTENSIONS
#undef X

    return TExtension::UNDEFINED;
}

const char *GetBehaviorString(TBehavior b)
{
    switch (b)
    {
        case EBhRequire:
            return "require";
        case EBhEnable:
            return "enable";
        case EBhWarn:
            return "warn";
        case EBhDisable:
            return "disable";
        default:
            return nullptr;
    }
}

bool IsExtensionEnabled(const TExtensionBehavior &extBehavior, TExtension extension)
{
    ASSERT(extension != TExtension::UNDEFINED);
    auto iter = extBehavior.find(extension);
    return iter != extBehavior.end() && (iter->second == EBhEnable || iter->second == EBhRequire);
}

}  // namespace sh
