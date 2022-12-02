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


def GenerateHashOfAffectedFiles():
    affectedFiles = []

    script_path = os.path.dirname(__file__)

    # search in respective subDirectoryToSearch and add matching pattern files to the affectFiles list
    subDirectoryToSearch = 'libANGLE/'
    directoryToSearch = os.path.join(script_path, subDirectoryToSearch)
    for f in os.listdir(directoryToSearch):
        if fnmatch.fnmatch(subDirectoryToSearch + f, subDirectoryToSearch + 'Program*.h'):
            affectedFiles.append(os.path.join(directoryToSearch, f))
        if fnmatch.fnmatch(subDirectoryToSearch + f, subDirectoryToSearch + 'Program*.cpp'):
            affectedFiles.append(os.path.join(directoryToSearch, f))
        if fnmatch.fnmatch(subDirectoryToSearch + f, subDirectoryToSearch + 'Shader.cpp'):
            affectedFiles.append(os.path.join(directoryToSearch, f))
        if fnmatch.fnmatch(subDirectoryToSearch + f, subDirectoryToSearch + 'Shader.h'):
            affectedFiles.append(os.path.join(directoryToSearch, f))
        if fnmatch.fnmatch(subDirectoryToSearch + f, subDirectoryToSearch + 'Uniform.h'):
            affectedFiles.append(os.path.join(directoryToSearch, f))

    subDirectoryToSearch = 'libANGLE/renderer/'
    directoryToSearch = os.path.join(script_path, subDirectoryToSearch)
    for f in os.listdir(directoryToSearch):
        if fnmatch.fnmatch(subDirectoryToSearch + f, subDirectoryToSearch + 'Program*.h'):
            affectedFiles.append(os.path.join(directoryToSearch, f))
        if fnmatch.fnmatch(subDirectoryToSearch + f, subDirectoryToSearch + 'Program*.cpp'):
            affectedFiles.append(os.path.join(directoryToSearch, f))
        if fnmatch.fnmatch(subDirectoryToSearch + f, subDirectoryToSearch + 'renderer_utils.h'):
            affectedFiles.append(os.path.join(directoryToSearch, f))

    subDirectoryToSearch = 'libANGLE/renderer/vulkan/'
    directoryToSearch = os.path.join(script_path, subDirectoryToSearch)
    for f in os.listdir(directoryToSearch):
        if fnmatch.fnmatch(subDirectoryToSearch + f, subDirectoryToSearch + 'Program*'):
            affectedFiles.append(os.path.join(directoryToSearch, f))

    subDirectoryToSearch = 'libANGLE/renderer/gl/'
    directoryToSearch = os.path.join(script_path, subDirectoryToSearch)
    for f in os.listdir(directoryToSearch):
        if fnmatch.fnmatch(subDirectoryToSearch + f, subDirectoryToSearch + 'Program*'):
            affectedFiles.append(os.path.join(directoryToSearch, f))

    subDirectoryToSearch = 'libANGLE/renderer/d3d/'
    directoryToSearch = os.path.join(script_path, subDirectoryToSearch)
    for f in os.listdir(directoryToSearch):
        if fnmatch.fnmatch(subDirectoryToSearch + f, subDirectoryToSearch + 'Program*'):
            affectedFiles.append(os.path.join(directoryToSearch, f))

    subDirectoryToSearch = 'libANGLE/renderer/null/'
    directoryToSearch = os.path.join(script_path, subDirectoryToSearch)
    for f in os.listdir(directoryToSearch):
        if fnmatch.fnmatch(subDirectoryToSearch + f, subDirectoryToSearch + 'Program*'):
            affectedFiles.append(os.path.join(directoryToSearch, f))

    subDirectoryToSearch = 'libANGLE/renderer/metal/'
    directoryToSearch = os.path.join(script_path, subDirectoryToSearch)
    for f in os.listdir(directoryToSearch):
        if fnmatch.fnmatch(subDirectoryToSearch + f, subDirectoryToSearch + 'Program*'):
            affectedFiles.append(os.path.join(directoryToSearch, f))

    subDirectoryToSearch = 'libANGLE/capture/'
    directoryToSearch = os.path.join(script_path, subDirectoryToSearch)
    for f in os.listdir(directoryToSearch):
        if fnmatch.fnmatch(subDirectoryToSearch + f, subDirectoryToSearch + 'FrameCapture.h'):
            affectedFiles.append(os.path.join(directoryToSearch, f))

    subDirectoryToSearch = '../include/platform/'
    directoryToSearch = os.path.join(script_path, subDirectoryToSearch)
    for f in os.listdir(directoryToSearch):
        if fnmatch.fnmatch(subDirectoryToSearch + f, subDirectoryToSearch + 'Feature.h'):
            affectedFiles.append(os.path.join(directoryToSearch, f))
        if fnmatch.fnmatch(subDirectoryToSearch + f,
                           subDirectoryToSearch + 'FrontendFeatures_autogen.h'):
            affectedFiles.append(os.path.join(directoryToSearch, f))

    subDirectoryToSearch = 'common/'
    directoryToSearch = os.path.join(script_path, subDirectoryToSearch)
    for f in os.listdir(directoryToSearch):
        if fnmatch.fnmatch(subDirectoryToSearch + f, subDirectoryToSearch + 'PackedEnums.h'):
            affectedFiles.append(os.path.join(directoryToSearch, f))

    # generate a hash from all the matching files collected in subDirectoryToSearch
    hash_md5 = hashlib.md5()
    for filename in affectedFiles:
        with open(filename, "r") as f:
            for chunk in iter(lambda: f.read(4096), ""):
                hash_md5.update(chunk.encode())

    # return (fileHash, fileHashSize)
    return hash_md5.hexdigest(), hash_md5.digest_size


usage = """\
Usage: program_serialize_data_version.py gen <file_to_write>  - generate ANGLEShaderProgramVersion.h
"""

if len(sys.argv) < 3:
    sys.exit(usage)
output_file = sys.argv[2]
angle_shader_program_version_hash_result = GenerateHashOfAffectedFiles()

hfile = open(output_file, 'w')
hfile.write('#define ANGLE_PROGRAM_VERSION "%s"\n' % angle_shader_program_version_hash_result[0])
hfile.write('#define ANGLE_PROGRAM_VERSION_HASH_SIZE %d\n' %
            angle_shader_program_version_hash_result[1])
hfile.close()
