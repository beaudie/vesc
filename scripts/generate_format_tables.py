#!/usr/bin/python
#
# Copyright 2017 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# generate_format_tables.py:
#   Generates the ANGLE format tables.

import os, subprocess, sys

generators = {
    'ANGLE': {
        'inputs': [
            'src/libANGLE/renderer/angle_format.py',
            'src/libANGLE/renderer/angle_format_data.json',
            'src/libANGLE/renderer/angle_format_map.json',
        ],
        'outputs': [
            'src/libANGLE/renderer/Format_table_autogen.cpp',
            'src/libANGLE/renderer/Format_ID_autogen.inl',
        ],
        'script': 'src/libANGLE/renderer/gen_angle_format_table.py',
    },
    'D3D11': {
        'inputs': [
            'src/libANGLE/renderer/angle_format.py',
            'src/libANGLE/renderer/d3d/d3d11/texture_format_data.json',
            'src/libANGLE/renderer/d3d/d3d11/texture_format_map.json',
        ],
        'outputs': [
            'src/libANGLE/renderer/d3d/d3d11/texture_format_table_autogen.cpp',
        ],
        'script': 'src/libANGLE/renderer/d3d/d3d11/gen_texture_format_table.py',
    },
    'DXGI format': {
        'inputs': [
            'src/libANGLE/renderer/angle_format.py',
            'src/libANGLE/renderer/angle_format_map.json',
            'src/libANGLE/renderer/d3d/d3d11/dxgi_format_data.json',
        ],
        'outputs': [
            'src/libANGLE/renderer/d3d/d3d11/dxgi_format_map_autogen.cpp',
        ],
        'script': 'src/libANGLE/renderer/d3d/d3d11/gen_dxgi_format_table.py',
    },
    'DXGI support': {
        'inputs': [
            'src/libANGLE/renderer/d3d/d3d11/dxgi_support_data.json',
        ],
        'outputs': [
            'src/libANGLE/renderer/d3d/d3d11/dxgi_support_table.cpp',
        ],
        'script': 'src/libANGLE/renderer/d3d/d3d11/gen_dxgi_support_tables.py',
    },
    'uniform': {
        'inputs': [],
        'outputs': [
            'src/common/uniform_type_info_autogen.cpp',
        ],
        'script': 'src/common/gen_uniform_type_table.py',
    },
    'Vulkan': {
        'inputs': [
            'src/libANGLE/renderer/angle_format.py',
            'src/libANGLE/renderer/angle_format_map.json',
            'src/libANGLE/renderer/vulkan/vk_format_map.json',
        ],
        'outputs': [
            'src/libANGLE/renderer/vulkan/vk_format_table_autogen.cpp',
        ],
        'script': 'src/libANGLE/renderer/vulkan/gen_vk_format_table.py',
    },
}

root_dir = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
any_dirty = False

for name, info in sorted(generators.iteritems()):

    # Set the CWD to the root ANGLE directory.
    os.chdir(root_dir)

    script = info['script']
    dirty = False

    for finput in info['inputs'] + [script]:
        input_mtime = os.path.getmtime(finput)
        for foutput in info['outputs']:
            if not os.path.exists(foutput):
                print('Output' + foutput + ' not found for ' + name + ' table')
                dirty = True
            else:
                output_mtime = os.path.getmtime(foutput)
                if input_mtime > output_mtime:
                    dirty = True

    if dirty:
        any_dirty = True

        # Set the CWD to the script directory.
        os.chdir(os.path.dirname(os.path.abspath(script)))

        print('Generating ' + name + ' format table')
        if subprocess.call(['python', os.path.basename(script)]) != 0:
            sys.exit(1)

if any_dirty:
    args = []
    if os.name == 'nt':
        args += ['git.bat']
    else:
        args += ['git']
    args += ['cl', 'format']
    print('Calling git cl format')
    subprocess.call(args)