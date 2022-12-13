#!/usr/bin/env python3
#  Copyright 2022 The ANGLE Project Authors. All rights reserved.
#  Use of this source code is governed by a BSD-style license that can be
#  found in the LICENSE file.

# Generate ANGLEShaderProgramVersion.h with hash of files affecting data
# used in serializing/deserializing shader programs.

import hashlib
import sys
import os
import ast


def GenerateHashOfAffectedFiles(angle_common_code, vulkan_backend_dir, vulkan_sources,
                                gl_backend_dir, gl_sources, d3d_backend_dir, d3d_sources,
                                null_backend_dir, null_sources, metal_backend_dir, metal_sources):
    script_path = os.path.dirname(__file__)
    script_parent_path = os.path.join(script_path, os.pardir)

    hash_md5 = hashlib.md5()
    for file in angle_common_code:
        absoluteFilePath = os.path.join(script_parent_path, file)
        with open(absoluteFilePath, 'r') as f:
            for chunk in iter(lambda: f.read(4096), ""):
                hash_md5.update(chunk.encode())
    for file in vulkan_sources:
        relativeFilePath = os.path.join(vulkan_backend_dir, file)
        absoluteFilePath = os.path.join(script_parent_path, relativeFilePath)
        with open(absoluteFilePath, 'r') as f:
            for chunk in iter(lambda: f.read(4096), ""):
                hash_md5.update(chunk.encode())
    for file in gl_sources:
        relativeFilePath = os.path.join(gl_backend_dir, file)
        absoluteFilePath = os.path.join(script_parent_path, relativeFilePath)
        with open(absoluteFilePath, 'r') as f:
            for chunk in iter(lambda: f.read(4096), ""):
                hash_md5.update(chunk.encode())
    for file in d3d_sources:
        relativeFilePath = os.path.join(d3d_backend_dir, file)
        absoluteFilePath = os.path.join(script_parent_path, relativeFilePath)
        with open(absoluteFilePath, 'r') as f:
            for chunk in iter(lambda: f.read(4096), ""):
                hash_md5.update(chunk.encode())
    for file in null_sources:
        relativeFilePath = os.path.join(null_backend_dir, file)
        absoluteFilePath = os.path.join(script_parent_path, relativeFilePath)
        with open(absoluteFilePath, 'r') as f:
            for chunk in iter(lambda: f.read(4096), ""):
                hash_md5.update(chunk.encode())
    for file in metal_sources:
        relativeFilePath = os.path.join(metal_backend_dir, file)
        absoluteFilePath = os.path.join(script_parent_path, relativeFilePath)
        with open(absoluteFilePath, 'r') as f:
            for chunk in iter(lambda: f.read(4096), ""):
                hash_md5.update(chunk.encode())
    return hash_md5.hexdigest(), hash_md5.digest_size


usage = """\
Usage: program_serialize_data_version.py gen <file_to_write> <list_of_input_file_list>
program_serialize_data_version.py gen ANGLEShaderProgramVersion.h $angle_code_affecting_program_serialize
"""

if len(sys.argv) < 14:
    sys.exit(usage)
output_file = sys.argv[2]
angle_common_code = ast.literal_eval(sys.argv[3])
vulkan_backend_dir = sys.argv[4]
vulkan_sources = ast.literal_eval(sys.argv[5])
gl_backend_dir = sys.argv[6]
gl_sources = ast.literal_eval(sys.argv[7])
d3d_backend_dir = sys.argv[8]
d3d_sources = ast.literal_eval(sys.argv[9])
null_backend_dir = sys.argv[10]
null_sources = ast.literal_eval(sys.argv[11])
metal_backend_dir = sys.argv[12]
metal_sources = ast.literal_eval(sys.argv[13])
angle_shader_program_version_hash_result = GenerateHashOfAffectedFiles(
    angle_common_code, vulkan_backend_dir, vulkan_sources, gl_backend_dir, gl_sources,
    d3d_backend_dir, d3d_sources, null_backend_dir, null_sources, metal_backend_dir, metal_sources)
hfile = open(output_file, 'w')
hfile.write('#define ANGLE_PROGRAM_VERSION "%s"\n' % angle_shader_program_version_hash_result[0])
hfile.write('#define ANGLE_PROGRAM_VERSION_HASH_SIZE %d\n' %
            angle_shader_program_version_hash_result[1])
hfile.close()
