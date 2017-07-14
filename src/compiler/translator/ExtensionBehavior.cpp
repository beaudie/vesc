//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ExtensionBehavior.cpp: Extension name enumeration and data structures for storing extension
// behavior.

#include "compiler/translator/ExtensionBehavior.h"

#include "common/debug.h"

#define GET_EXTENSION_NAME_CASE(ext) \
    case TExtension::ext:            \
        return "GL_" #ext;

#define RETURN_EXTENSION_IF_MATCHES(ext)       \
    if (strcmp(extWithoutGLPrefix, #ext) == 0) \
    {                                          \
        return TExtension::ext;                \
    }

namespace sh
{

const char *GetExtensionNameString(TExtension extension)
{
    switch (extension)
    {
        GET_EXTENSION_NAME_CASE(ARB_texture_rectangle)
        GET_EXTENSION_NAME_CASE(ARM_shader_framebuffer_fetch)
        GET_EXTENSION_NAME_CASE(EXT_blend_func_extended)
        GET_EXTENSION_NAME_CASE(EXT_draw_buffers)
        GET_EXTENSION_NAME_CASE(EXT_frag_depth)
        GET_EXTENSION_NAME_CASE(EXT_shader_framebuffer_fetch)
        GET_EXTENSION_NAME_CASE(EXT_shader_texture_lod)
        GET_EXTENSION_NAME_CASE(EXT_YUV_target)
        GET_EXTENSION_NAME_CASE(NV_EGL_stream_consumer_external)
        GET_EXTENSION_NAME_CASE(NV_shader_framebuffer_fetch)
        GET_EXTENSION_NAME_CASE(OES_EGL_image_external)
        GET_EXTENSION_NAME_CASE(OES_EGL_image_external_essl3)
        GET_EXTENSION_NAME_CASE(OES_geometry_shader)
        GET_EXTENSION_NAME_CASE(OES_standard_derivatives)
        GET_EXTENSION_NAME_CASE(OVR_multiview)
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

    RETURN_EXTENSION_IF_MATCHES(ARB_texture_rectangle)
    RETURN_EXTENSION_IF_MATCHES(ARM_shader_framebuffer_fetch)
    RETURN_EXTENSION_IF_MATCHES(EXT_blend_func_extended)
    RETURN_EXTENSION_IF_MATCHES(EXT_draw_buffers)
    RETURN_EXTENSION_IF_MATCHES(EXT_frag_depth)
    RETURN_EXTENSION_IF_MATCHES(EXT_shader_framebuffer_fetch)
    RETURN_EXTENSION_IF_MATCHES(EXT_shader_texture_lod)
    RETURN_EXTENSION_IF_MATCHES(EXT_YUV_target)
    RETURN_EXTENSION_IF_MATCHES(NV_EGL_stream_consumer_external)
    RETURN_EXTENSION_IF_MATCHES(NV_shader_framebuffer_fetch)
    RETURN_EXTENSION_IF_MATCHES(OES_EGL_image_external)
    RETURN_EXTENSION_IF_MATCHES(OES_EGL_image_external_essl3)
    RETURN_EXTENSION_IF_MATCHES(OES_geometry_shader)
    RETURN_EXTENSION_IF_MATCHES(OES_standard_derivatives)
    RETURN_EXTENSION_IF_MATCHES(OVR_multiview)

    return TExtension::UNDEFINED;
}

}  // namespace sh
