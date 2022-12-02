#!/usr/bin/env python3
#  Copyright 2022 The ANGLE Project Authors. All rights reserved.
#  Use of this source code is governed by a BSD-style license that can be
#  found in the LICENSE file.

# Generate ANGLEShaderProgramVersion.h with hash of files affecting data
# used in serializing/deserializing shader programs.

import hashlib
import sys
import os
import fnmatch
import ast


def GenerateHashOfAffectedFiles(libangle_files, vulkan_backend, gl_backend, d3d_backend,
                                null_backend, metal_backend):
    #def GenerateHashOfAffectedFiles(libangle_files):
    print("Yuxin Debug GenerateHashOfAffectedFiles() is called")
    script_path = os.path.dirname(__file__)
    script_parent_path = os.path.join(script_path, os.pardir)

    hash_md5 = hashlib.md5()
    for filelists in libangle_files:
        for file in filelists:
            absoluteFilePath = os.path.join(script_parent_path, file)
            with open(absoluteFilePath, 'r') as f:
                for chunk in iter(lambda: f.read(4096), ""):
                    hash_md5.update(chunk.encode())
    for file in vulkan_backend:
        fileRelativeToScript = os.path.join('libANGLE/renderer/vulkan/', file)
        absoluteFilePath = os.path.join(script_path, fileRelativeToScript)
        with open(absoluteFilePath, 'r') as f:
            for chunk in iter(lambda: f.read(4096), ""):
                hash_md5.update(chunk.encode())
    for file in gl_backend:
        fileRelativeToScript = os.path.join('libANGLE/renderer/gl/', file)
        absoluteFilePath = os.path.join(script_path, fileRelativeToScript)
        with open(absoluteFilePath, 'r') as f:
            for chunk in iter(lambda: f.read(4096), ""):
                hash_md5.update(chunk.encode())
    for filelists in d3d_backend:
        for file in filelists:
            fileRelativeToScript = os.path.join('libANGLE/renderer/d3d/', file)
            absoluteFilePath = os.path.join(script_path, fileRelativeToScript)
            with open(absoluteFilePath, 'r') as f:
                for chunk in iter(lambda: f.read(4096), ""):
                    hash_md5.update(chunk.encode())
    for file in null_backend:
        fileRelativeToScript = os.path.join('libANGLE/renderer/null/', file)
        absoluteFilePath = os.path.join(script_path, fileRelativeToScript)
        with open(absoluteFilePath, 'r') as f:
            for chunk in iter(lambda: f.read(4096), ""):
                hash_md5.update(chunk.encode())
    for file in metal_backend:
        fileRelativeToScript = os.path.join('libANGLE/renderer/metal/', file)
        absoluteFilePath = os.path.join(script_path, fileRelativeToScript)
        with open(absoluteFilePath, 'r') as f:
            for chunk in iter(lambda: f.read(4096), ""):
                hash_md5.update(chunk.encode())
    return hash_md5.hexdigest(), hash_md5.digest_size


usage = """\
Usage: program_serialize_data_version.py gen <file_to_write> <inputFileList1> <inputFileList2> <inputFileList3> ... <inputFileList6> - 
program_serialize_data_version.py gen ANGLEShaderProgramVersion.h inputFileList1 inputFileList2 inputFileList3 ... inputFileList6
"""

if len(sys.argv) < 9:
    sys.exit(usage)
output_file = sys.argv[2]
libangle_files = ast.literal_eval(sys.argv[3])
#print(libangle_files)
vulkan_backend = ast.literal_eval(sys.argv[4])
print(vulkan_backend)
gl_backend = ast.literal_eval(sys.argv[5])
d3d_backend = ast.literal_eval(sys.argv[6])
null_backend = ast.literal_eval(sys.argv[7])
metal_backend = ast.literal_eval(sys.argv[8])

angle_shader_program_version_hash_result = GenerateHashOfAffectedFiles(
    libangle_files, vulkan_backend, gl_backend, d3d_backend, null_backend, metal_backend)
#angle_shader_program_version_hash_result = GenerateHashOfAffectedFiles(libangle_files)
hfile = open(output_file, 'w')
hfile.write('#define ANGLE_PROGRAM_VERSION "%s"\n' % angle_shader_program_version_hash_result[0])
hfile.write('#define ANGLE_PROGRAM_VERSION_HASH_SIZE %d\n' %
            angle_shader_program_version_hash_result[1])
hfile.close()
