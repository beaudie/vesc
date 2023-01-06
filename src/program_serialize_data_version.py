#!/usr/bin/env python3
#  Copyright 2022 The ANGLE Project Authors. All rights reserved.
#  Use of this source code is governed by a BSD-style license that can be
#  found in the LICENSE file.

# Generate ANGLEShaderProgramVersion.h with hash of files affecting data
# used in serializing/deserializing shader programs.

import hashlib
import argparse
import shlex


def GenerateHashOfAffectedFiles(angle_code_files, relative_path, add_relative_path_to_build_dir):
    hash_md5 = hashlib.md5()
    for file in angle_code_files:
        assert (file != "")
        if add_relative_path_to_build_dir:
            file = relative_path + file
        with open(file, 'r', encoding='utf-8') as f:
            for chunk in iter(lambda: f.read(4096), ""):
                hash_md5.update(chunk.encode())
    return hash_md5.hexdigest(), hash_md5.digest_size


parser = argparse.ArgumentParser(description='Generate the file ANGLEShaderProgramVersion.h')
parser.add_argument(
    'output_file',
    help='path (relative to build directory) to output file name, stores ANGLE_PROGRAM_VERSION and ANGLE_PROGRAM_VERSION_HASH_SIZE'
)
parser.add_argument(
    'response_file_name_with_path',
    help='path (relative to build directory) to response file name. The response file stores a list of program files that ANGLE_PROGRAM_VERSION hashes over. See https://gn.googlesource.com/gn/+/main/docs/reference.md#var_response_file_contents'
)
parser.add_argument(
    '--response_file_name',
    help='name of the response file itself, not including relative path to build directory',
    required=False)
parser.add_argument(
    '--add_relative_path_to_build_dir',
    help='for files listed in {{response_file_name}}, whether we need to add the relative path to the project build directory',
    required=False)
args = parser.parse_args()

output_file = args.output_file

response_file_name_with_path = args.response_file_name_with_path

response_file_name = ''
if (args.response_file_name):
    response_file_name = args.response_file_name

add_relative_path_to_build_dir = False
if (args.add_relative_path_to_build_dir):
    add_relative_path_to_build_dir = args.add_relative_path_to_build_dir

relative_path = ''
if (add_relative_path_to_build_dir):
    relative_path = response_file_name_with_path.partition(response_file_name)[0]

with open(response_file_name_with_path, "r") as input_files_for_hash_generation:
    angle_code_files = shlex.split(input_files_for_hash_generation)
angle_shader_program_version_hash_result = GenerateHashOfAffectedFiles(
    angle_code_files, relative_path, add_relative_path_to_build_dir)
hfile = open(output_file, 'w')
hfile.write('#define ANGLE_PROGRAM_VERSION "%s"\n' % angle_shader_program_version_hash_result[0])
hfile.write('#define ANGLE_PROGRAM_VERSION_HASH_SIZE %d\n' %
            angle_shader_program_version_hash_result[1])
hfile.close()
