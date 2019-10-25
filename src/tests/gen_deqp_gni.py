#!/usr/bin/env python3
# Copyright (c) 2015-2018 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Author: Tim Van Patten <timvp@google.com>

import errno
import os
import sys
import shutil

# List of directories containing data files
dataDirectories = []

# List of directories to exclude from the copy
excludedDirectories = [
    ".git",
]

# List of files to exclude from the copy
excludedFilenames = [
    "LICENSE",
]

# Dictionary of parts of paths that need to be replaced
# Key: Part of path to be replaced
# Value: What to replace it with
pathReplacements = {}

# List of unique relative directories for the copy() command outputs
relativeDirectories = []

# VK-GL-CTS source directory
deqpSourceDirectory = "../../third_party/VK-GL-CTS/src/"

# dEQP Support Directory
deqpSupportDirectory = "deqp_support"
# BUILD.gn file to write to
buildGnFilename = "BUILD.gn"
# Path to BUILD.gn
buildGnPath = os.path.join(deqpSupportDirectory, buildGnFilename)
# GNI File to write to
gniFilename = "deqp_data.gni"


def initDataDirectories():
    dataDirectories.append(os.path.join("data", "gles2"))
    dataDirectories.append(os.path.join("data", "gles3"))
    dataDirectories.append(os.path.join("data", "gles31"))
    dataDirectories.append(os.path.join("external", "graphicsfuzz", "data", "gles3"))


def initPathReplacements():
    pathToReplace = os.path.join("external", "graphicsfuzz", "")  # Include trailing slash
    pathReplacements[pathToReplace] = ""


def createBuildGnFile():
    # Cleanup the old file
    if os.path.exists(buildGnPath):
        os.remove(buildGnPath)
    # Make the new one
    return open(buildGnPath, "w+")


def createGniFile():
    # Cleanup the old file
    if os.path.exists(gniFilename):
        os.remove(gniFilename)
    # Make the new one
    return open(gniFilename, "w+")


def writeFileHeader(fileIn):
    fileIn.write("# Copyright 2015 The ANGLE Project Authors. All rights reserved.\n")
    fileIn.write("# Use of this source code is governed by a BSD-style license that can be\n")
    fileIn.write("# found in the LICENSE file.\n")
    fileIn.write("\n")


def fixDestinationDirectory(relativeDirectory):
    for pathReplacement in pathReplacements:
        if pathReplacement in relativeDirectory:
            return relativeDirectory.replace(pathReplacement, pathReplacements[pathReplacement])
    return relativeDirectory


def main():
    # auto_script parameters.
    if len(sys.argv) > 1:
        inputs = []
        outputs = [gniFilename, buildGnPath]

        if sys.argv[1] == 'inputs':
            print(','.join(inputs))
        elif sys.argv[1] == 'outputs':
            print(','.join(outputs))
        else:
            print('Invalid script parameters')
            return 1
        return 0

    deqpSrcDir = os.path.abspath(os.path.join(sys.path[0], deqpSourceDirectory))

    initDataDirectories()
    initPathReplacements()

    dataFiles = []
    for dataDir in dataDirectories:
        dataPath = os.path.join(deqpSrcDir, dataDir)
        for root, directories, filenames in os.walk(dataPath):
            for filename in filenames:
                relativeDirectory = os.path.relpath(root, deqpSrcDir)
                # Skip any excluded directories
                if any(directory in relativeDirectory for directory in excludedDirectories):
                    continue
                # Skip any excluded files
                if any(excludedFilename in filename for excludedFilename in excludedFilenames):
                    continue
                # Record the relative directories and full paths to each data file
                if relativeDirectory not in relativeDirectories:
                    relativeDirectories.append(relativeDirectory)
                dataFiles.append(os.path.join(relativeDirectory, filename))

    dataFiles.sort()
    relativeDirectories.sort()

    #
    # BUILD.gn
    #
    buildGnFile = createBuildGnFile()
    writeFileHeader(buildGnFile)
    # Definitions
    buildGnFile.write("deqp_path = \"../../../third_party/VK-GL-CTS/src\"\n")
    # Create the copy() commands
    for relativeDirectory in relativeDirectories:
        buildGnFile.write("\n")
        buildGnFile.write("copy(\"" + relativeDirectory.replace("/", "_") + "\") {\n")
        buildGnFile.write("  sources = [\n")
        for dataFile in dataFiles:
            path, filename = os.path.split(dataFile)
            if relativeDirectory == path:
                buildGnFile.write("    \"$deqp_path/" + dataFile + "\",\n")
        buildGnFile.write("  ]\n")
        buildGnFile.write("  outputs = [\n")
        buildGnFile.write("    \"$root_gen_dir/" + fixDestinationDirectory(relativeDirectory) +
                          "/{{source_file_part}}\",\n")
        buildGnFile.write("  ]\n")
        buildGnFile.write("}\n")

    #
    # .gni
    #
    gniFile = createGniFile()
    writeFileHeader(gniFile)
    # Imports
    gniFile.write("import(\"deqp.gni\")\n")
    # Write the lists of data file dependencies
    for dataDirectory in dataDirectories:
        gniFile.write("\n")
        dataDepName = "angle_deqp_" + dataDirectory.replace(os.sep, "_")
        gniFile.write("%s = [\n" % (dataDepName))
        for dataFile in dataFiles:
            if dataDirectory + os.sep in dataFile:
                gniFile.write("  \"$root_gen_dir/" + fixDestinationDirectory(dataFile) + "\",\n")
        gniFile.write("]\n")
    gniFile.write("\n")
    gniFile.write("deqp_data_copy_targets = [\n")
    for relativeDirectory in relativeDirectories:
        gniFile.write("  \"" + deqpSupportDirectory + ":" + relativeDirectory.replace("/", "_") +
                      "\",\n")
    gniFile.write("]\n")


if __name__ == '__main__':
    sys.exit(main())
