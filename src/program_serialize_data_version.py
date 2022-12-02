#!/usr/bin/env python3
#  Copyright 2022 The ANGLE Project Authors. All rights reserved.
#  Use of this source code is governed by a BSD-style license that can be
#  found in the LICENSE file.

# Generate ANGLEShaderProgramVersion.h with hash of files affecting data
# used in serializing/deserializing shader programs.

import hashlib
import sys
import re  #library for regex
import os


def GenerateHashOfAffectedFiles():
    affectedFiles = []

    script_path = os.path.dirname(__file__)

    directoryToSearch = os.path.join(script_path, 'libANGLE')
    filesSet1 = [
        f for f in os.listdir(directoryToSearch)
        if os.path.isfile(os.path.join(directoryToSearch, f))
    ]
    for f in filesSet1:
        if re.match(r'^libANGLE/Program.*\.h$', 'libANGLE/' + f):
            affectedFiles.append(os.path.join(directoryToSearch, f))
            # print(f)
        if re.match(r'^libANGLE/Program.*\.cpp$', 'libANGLE/' + f):
            affectedFiles.append(os.path.join(directoryToSearch, f))
            # print(f)
        if re.match(r'^libANGLE/Shader.cpp$', 'libANGLE/' + f):
            affectedFiles.append(os.path.join(directoryToSearch, f))
            # print(f)
        if re.match(r'^libANGLE/Shader.h$', 'libANGLE/' + f):
            affectedFiles.append(os.path.join(directoryToSearch, f))
            # print(f)
        if re.match(r'^libANGLE/Uniform.h$', 'libANGLE/' + f):
            affectedFiles.append(os.path.join(directoryToSearch, f))
            # print(f)

    directoryToSearch = os.path.join(script_path, 'libANGLE/renderer')
    filesSet2 = [
        f for f in os.listdir(directoryToSearch)
        if os.path.isfile(os.path.join(directoryToSearch, f))
    ]
    for f in filesSet2:
        if re.match(r'^libANGLE/renderer/Program.*\.h$', 'libANGLE/renderer/' + f):
            affectedFiles.append(os.path.join(directoryToSearch, f))
            # print(f)
        if re.match(r'^libANGLE/renderer/Program.*\.cpp$', 'libANGLE/renderer/' + f):
            affectedFiles.append(os.path.join(directoryToSearch, f))
            # print(f)
        if re.match(r'^libANGLE/renderer/renderer_utils.h$', 'libANGLE/renderer/' + f):
            affectedFiles.append(os.path.join(directoryToSearch, f))
            # print(f)

    directoryToSearch = os.path.join(script_path, 'libANGLE/renderer/vulkan')
    fileSet3 = [
        f for f in os.listdir(directoryToSearch)
        if os.path.isfile(os.path.join(directoryToSearch, f))
    ]
    for f in fileSet3:
        if re.match(r'libANGLE/renderer/.*/Program.*', 'libANGLE/renderer/vulkan/' + f):
            affectedFiles.append(os.path.join(directoryToSearch, f))
            # print(f)

    directoryToSearch = os.path.join(script_path, 'libANGLE/renderer/gl')
    fileSet4 = [
        f for f in os.listdir(directoryToSearch)
        if os.path.isfile(os.path.join(directoryToSearch, f))
    ]
    for f in fileSet4:
        if re.match(r'libANGLE/renderer/.*/Program.*', 'libANGLE/renderer/gl/' + f):
            affectedFiles.append(os.path.join(directoryToSearch, f))
            # print(f)

    directoryToSearch = os.path.join(script_path, 'libANGLE/renderer/d3d')
    fileSet5 = [
        f for f in os.listdir(directoryToSearch)
        if os.path.isfile(os.path.join(directoryToSearch, f))
    ]
    for f in fileSet5:
        if re.match(r'libANGLE/renderer/.*/Program.*', 'libANGLE/renderer/d3d/' + f):
            affectedFiles.append(os.path.join(directoryToSearch, f))
            # print(f)

    directoryToSearch = os.path.join(script_path, 'libANGLE/renderer/null')
    fileSet6 = [
        f for f in os.listdir(directoryToSearch)
        if os.path.isfile(os.path.join(directoryToSearch, f))
    ]
    for f in fileSet6:
        if re.match(r'libANGLE/renderer/.*/Program.*', 'libANGLE/renderer/null/' + f):
            affectedFiles.append(os.path.join(directoryToSearch, f))
            # print(f)

    directoryToSearch = os.path.join(script_path, 'libANGLE/renderer/metal')
    fileSet7 = [
        f for f in os.listdir(directoryToSearch)
        if os.path.isfile(os.path.join(directoryToSearch, f))
    ]
    for f in fileSet7:
        if re.match(r'libANGLE/renderer/.*/Program.*', 'libANGLE/renderer/metal/' + f):
            affectedFiles.append(os.path.join(directoryToSearch, f))
            # print(f)

    directoryToSearch = os.path.join(script_path, 'libANGLE/capture')
    fileSet8 = [
        f for f in os.listdir(directoryToSearch)
        if os.path.isfile(os.path.join(directoryToSearch, f))
    ]
    for f in fileSet8:
        if re.match(r'libANGLE/capture/FrameCapture.h$', 'libANGLE/capture/' + f):
            affectedFiles.append(os.path.join(directoryToSearch, f))
            # print(f)

    directoryToSearch = os.path.join(script_path, '../include/platform')
    fileSet9 = [
        f for f in os.listdir(directoryToSearch)
        if os.path.isfile(os.path.join(directoryToSearch, f))
    ]
    for f in fileSet9:
        if re.match(r'../include/platform/Feature.h$', '../include/platform/' + f):
            affectedFiles.append(os.path.join(directoryToSearch, f))
            # print(f)
        if re.match(r'../include/platform/FrontendFeatures_autogen.h$',
                    '../include/platform/' + f):
            affectedFiles.append(os.path.join(directoryToSearch, f))
            # print(f)

    directoryToSearch = os.path.join(script_path, 'common')
    fileSet10 = [
        f for f in os.listdir(directoryToSearch)
        if os.path.isfile(os.path.join(directoryToSearch, f))
    ]
    for f in fileSet10:
        if re.match(r'common/PackedEnums.h$', 'common/' + f):
            affectedFiles.append(os.path.join(directoryToSearch, f))
            # print(f)

    hash_md5 = hashlib.md5()

    for filename in affectedFiles:
        print('"' + filename + '",')
        with open(filename, "r") as f:
            for chunk in iter(lambda: f.read(4096), ""):
                hash_md5.update(chunk.encode())
    return hash_md5.hexdigest(), hash_md5.digest_size


usage = """\
Usage: program_serialize_data_version.py gen <file_to_write>  - generate ANGLEShaderProgramVersion.h
"""

print("Yuxin debug main() is called")
if len(sys.argv) < 3:
    sys.exit(usage)
output_file = sys.argv[2]
angle_shader_program_version_hash_result = GenerateHashOfAffectedFiles()
print(angle_shader_program_version_hash_result)

hfile = open(output_file, 'w')
print(output_file)
hfile.write('#define ANGLE_PROGRAM_VERSION "%s"\n' % angle_shader_program_version_hash_result[0])
hfile.write('#define ANGLE_PROGRAM_VERSION_HASH_SIZE %d\n' %
            angle_shader_program_version_hash_result[1])
hfile.close()
