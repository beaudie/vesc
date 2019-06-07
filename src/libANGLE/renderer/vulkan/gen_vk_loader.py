#!/usr/bin/python
# Copyright 2019 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# gen_vk_loader.py:
#  Code generation for Vulkan loader files.
#  NOTE: don't run this script directly. Run scripts/run_code_generation.py.

import sys

sys.path.append('..')
import sys, os
import subprocess


def script_relative(path):
    return os.path.join(os.path.dirname(sys.argv[0]), path)


def link_gen_script_deps(gen_script, scripts_dir, deps):
    for dep in deps:
        # Only link if needed
        if not os.path.exists(os.path.join(script_relative(os.path.dirname(gen_script)), dep)):
            print("Linking file src, dst: %s, %s" % (os.path.join(
                scripts_dir, dep), os.path.join(os.path.dirname(gen_script), dep)))
            os.symlink(
                os.path.join(scripts_dir, dep),
                os.path.join(script_relative(os.path.dirname(gen_script)), dep))


def main():

    loader_gen_script = '../../../../third_party/vulkan-loader/src/scripts/loader_genvk.py'
    out_dir = '../../../../third_party/vk_loader_gen_files'
    vk_xml_file = '../../../../third_party/vulkan-headers/src/registry/vk.xml'
    scripts_dir = '../../../../third_party/vulkan-headers/src/registry'
    output_files = [
        'vk_extension_helper.h',
        'vk_dispatch_table_helper.h',
        'vk_object_types.h',
        'vk_layer_dispatch_table.h',
        'vk_typemap_helper.h',
        'vk_safe_struct.h',
        'vk_safe_struct.cpp',
        'vk_enum_string_helper.h',
        'vk_loader_extensions.h',
        'vk_loader_extensions.c',
    ]

    # auto_script parameters.
    if len(sys.argv) > 1:
        inputs = [
            loader_gen_script,
            out_dir,
            vk_xml_file,
            scripts_dir,
        ]
        outputs = output_files

        if sys.argv[1] == 'inputs':
            print ','.join(inputs)
        elif sys.argv[1] == 'outputs':
            print ','.join(outputs)
        else:
            print('Invalid script parameters')
            return 1
        return 0

    # Deps for gen script that we need to link from script dir
    deps = ['reg.py', 'generator.py', 'cgenerator.py']
    link_gen_script_deps(loader_gen_script, scripts_dir, deps)

    # Now generate all of the output files
    os.chdir(os.path.dirname(script_relative(loader_gen_script)))
    for outfile_name in output_files:
        print("Generating output file %s" % (outfile_name))
        if subprocess.call([
                "python",
                os.path.basename(loader_gen_script), "-o", out_dir, "-registry", vk_xml_file,
                "-scripts", scripts_dir, outfile_name, "-quiet"
        ]) != 0:
            return 1

    return 0


if __name__ == '__main__':
    sys.exit(main())
