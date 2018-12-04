#!/usr/bin/python2
#
# Copyright 2017 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# generate_entry_points.py:
#   Generates the OpenGL bindings and entry point layers for ANGLE.

import sys, os, pprint, json
from datetime import date
import registry_xml

# Handle inputs/outputs for run_code_generation.py's auto_script
if len(sys.argv) == 2 and sys.argv[1] == 'inputs':

    inputs = [
        'egl.xml',
        'egl_angle_ext.xml',
        'gl.xml',
        'gl_angle_ext.xml',
        'registry_xml.py',
    ]

    print(",".join(inputs))
    sys.exit(0)

def gen_gl_loader():
    xml = registry_xml.RegistryXML("gl.xml", "gl_angle_ext.xml")

    # First run through the main GLES entry points.  Since ES2+ is the primary use
    # case, we go through those first and then add ES1-only APIs at the end.
    for major_version, minor_version in [[2, 0], [3, 0], [3, 1], [1, 0]]:
        annotation = "{}_{}".format(major_version, minor_version)
        name_prefix = "GL_ES_VERSION_"

        is_gles1 = major_version == 1
        if is_gles1:
            name_prefix = "GL_VERSION_ES_CM_"

        feature_name = "{}{}".format(name_prefix, annotation)

        xml.AddCommands(feature_name, annotation)

    xml.AddExtensionCommands(registry_xml.supported_extensions, ['gles2', 'gles1'])

    all_cmds = xml.all_cmd_names.get_all_commands()

    if registry_xml.support_EGL_ANGLE_explicit_context:
        all_cmds += [cmd + "ContextANGLE" for cmd in xml.all_cmd_names.get_all_commands()]

    loader_header_path = registry_xml.path_to(os.path.join("..", "util"), "gles_loader_autogen.h")
    with open(loader_header_path, "w") as out:

        var_protos = ["extern PFN%sPROC %s;" % (cmd.upper(), cmd) for cmd in all_cmds]
        loader_header = template_loader_h.format(
            script_name = os.path.basename(sys.argv[0]),
            data_source_name = "gl.xml and gl_angle_ext.xml",
            year = date.today().year,
            function_pointers = "\n".join(var_protos),
            api_upper = "GLES",
            api_lower = "gles",
            preamble = gles_preamble)

        out.write(loader_header)
        out.close()

    loader_source_path = registry_xml.path_to(os.path.join("..", "util"), "gles_loader_autogen.cpp")
    with open(loader_source_path, "w") as out:

        var_defs = ["PFN%sPROC %s;" % (cmd.upper(), cmd) for cmd in all_cmds]

        setter = "    %s = reinterpret_cast<PFN%sPROC>(loadProc(\"%s\"));"
        setters = [setter % (cmd, cmd.upper(), cmd) for cmd in all_cmds]

        loader_source = template_loader_cpp.format(
            script_name = os.path.basename(sys.argv[0]),
            data_source_name = "gl.xml and gl_angle_ext.xml",
            year = date.today().year,
            function_pointers = "\n".join(var_defs),
            set_pointers = "\n".join(setters),
            api_upper = "GLES",
            api_lower = "gles")

        out.write(loader_source)
        out.close()

def gen_egl_loader():

    supported_egl_extensions = [
        "EGL_ANDROID_blob_cache",
        "EGL_ANGLE_d3d_share_handle_client_buffer",
        "EGL_ANGLE_device_d3d",
        "EGL_ANGLE_query_surface_pointer",
        "EGL_ANGLE_surface_d3d_texture_2d_share_handle",
        "EGL_ANGLE_window_fixed_size",
        "EGL_EXT_create_context_robustness",
        "EGL_EXT_device_base",
        "EGL_EXT_device_enumeration",
        "EGL_EXT_device_query",
        "EGL_EXT_platform_base",
        "EGL_EXT_platform_device",
        "EGL_KHR_debug",
        "EGL_KHR_stream",
        "EGL_KHR_stream_consumer_gltexture",
        "EGL_NV_stream_consumer_gltexture_yuv",
    ]

    xml = registry_xml.RegistryXML("egl.xml", "gl_angle_ext.xml")

    # First run through the main GLES entry points.  Since ES2+ is the primary use
    # case, we go through those first and then add ES1-only APIs at the end.
    for major_version, minor_version in [[1, 0], [1, 1], [1, 2], [1, 3], [1, 4], [1, 5]]:
        annotation = "{}_{}".format(major_version, minor_version)
        name_prefix = "EGL_VERSION_"

        feature_name = "{}{}".format(name_prefix, annotation)

        xml.AddCommands(feature_name, annotation)

    xml.AddExtensionCommands(supported_egl_extensions, ['egl'])

    all_cmds = xml.all_cmd_names.get_all_commands()

    loader_header_path = registry_xml.path_to(os.path.join("..", "util"), "egl_loader_autogen.h")
    with open(loader_header_path, "w") as out:

        var_protos = ["extern PFN%sPROC %s;" % (cmd.upper(), cmd) for cmd in all_cmds]
        loader_header = template_loader_h.format(
            script_name = os.path.basename(sys.argv[0]),
            data_source_name = "egl.xml and egl_angle_ext.xml",
            year = date.today().year,
            function_pointers = "\n".join(var_protos),
            api_upper = "EGL",
            api_lower = "egl",
            preamble = egl_preamble)

        out.write(loader_header)
        out.close()

    loader_source_path = registry_xml.path_to(os.path.join("..", "util"), "egl_loader_autogen.cpp")
    with open(loader_source_path, "w") as out:

        var_defs = ["PFN%sPROC %s;" % (cmd.upper(), cmd) for cmd in all_cmds]

        setter = "    %s = reinterpret_cast<PFN%sPROC>(loadProc(\"%s\"));"
        setters = [setter % (cmd, cmd.upper(), cmd) for cmd in all_cmds]

        loader_source = template_loader_cpp.format(
            script_name = os.path.basename(sys.argv[0]),
            data_source_name = "egl.xml and egl_angle_ext.xml",
            year = date.today().year,
            function_pointers = "\n".join(var_defs),
            set_pointers = "\n".join(setters),
            api_upper = "EGL",
            api_lower = "egl")

        out.write(loader_source)
        out.close()

# Generate simple function loader for the tests.
def main():
    gen_gl_loader()
    gen_egl_loader()

gles_preamble = """#if defined(GL_GLES_PROTOTYPES)
#undef GL_GLES_PROTOTYPES
#endif  // defined(GL_GLES_PROTOTYPES)

#if defined(GL_GLEXT_PROTOTYPES)
#undef GL_GLEXT_PROTOTYPES
#endif  // defined(GL_GLEXT_PROTOTYPES)

#define GL_GLES_PROTOTYPES 0

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
"""

egl_preamble = """#if defined(EGL_EGL_PROTOTYPES)
#undef EGL_EGL_PROTOTYPES
#endif  // defined(EGL_EGL_PROTOTYPES)

#if defined(EGL_EGLEXT_PROTOTYPES)
#undef  EGL_EGLEXT_PROTOTYPES
#endif  // defined(EGL_EGLEXT_PROTOTYPES)

#include <EGL/egl.h>
#include <EGL/eglext.h>
"""

template_loader_h = """// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright {year} The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// {api_lower}_loader_autogen.h:
//   Simple {api_upper} function loader.

#ifndef UTIL_{api_upper}_LOADER_AUTOGEN_H_
#define UTIL_{api_upper}_LOADER_AUTOGEN_H_

{preamble}
{function_pointers}

namespace angle
{{
using GenericProc = void (*)();
using LoadProc = GenericProc (*)(const char *);
void Load{api_upper}(LoadProc loadProc);
}}  // namespace angle

#endif  // UTIL_{api_upper}_LOADER_AUTOGEN_H_
"""

template_loader_cpp = """// GENERATED FILE - DO NOT EDIT.
// Generated by {script_name} using data from {data_source_name}.
//
// Copyright {year} The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// {api_lower}_loader_autogen.cpp:
//   Simple {api_upper} function loader.

#include "util/{api_lower}_loader_autogen.h"

{function_pointers}

namespace angle
{{
void Load{api_upper}(LoadProc loadProc)
{{
{set_pointers}
}}
}}  // namespace angle
"""

if __name__ == '__main__':
    sys.exit(main())
