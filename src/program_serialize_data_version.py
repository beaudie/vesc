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

script_path = os.path.dirname(__file__)
script_parent_path = os.path.join(script_path, os.pardir)


def GenerateHashOfAffectedFiles(angle_code_files):
    hash_md5 = hashlib.md5()
    for file in angle_code_files:
        if file == "":
            continue
        absoluteFilePath = os.path.join(script_parent_path, file)
        with open(absoluteFilePath, 'r') as f:
            for chunk in iter(lambda: f.read(4096), ""):
                hash_md5.update(chunk.encode())
    return hash_md5.hexdigest(), hash_md5.digest_size


usage = """\
Usage: program_serialize_data_version.py gen <file_to_write> <list_of_input_file_list>
program_serialize_data_version.py gen ANGLEShaderProgramVersion.h $angle_code_affecting_program_serialize
"""

#test the script works
if len(sys.argv) < 4:
    sys.exit(usage)
output_file = sys.argv[2]
input_file = sys.argv[3]
input_file_absolute_path = os.path.join(script_parent_path, input_file)
with open(input_file_absolute_path, "r") as input_files:
    angle_code_files = input_files.read().split('\n')
angle_shader_program_version_hash_result = GenerateHashOfAffectedFiles(angle_code_files)
hfile = open(output_file, 'w')
hfile.write('#define ANGLE_PROGRAM_VERSION "%s"\n' % angle_shader_program_version_hash_result[0])
hfile.write('#define ANGLE_PROGRAM_VERSION_HASH_SIZE %d\n' %
            angle_shader_program_version_hash_result[1])
hfile.close()
