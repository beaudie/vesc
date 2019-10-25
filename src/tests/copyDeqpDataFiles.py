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


def printHelp():
    print('''
Usage:
  python copyDeqpDataFiles.py <VK-GL-CTS directory> <output VK-GL-CTS data directory>

The copyDeqpDataFiles script generates a BUILD.gn copy() command to embed tests/BUILD.gn.

Arguments:
  <dEQP directory>                The full dEQP path
                                  (~/code/chromium/src/third_party/angle/third_party/VK-GL-CTS/src).
  <output dEQP data directory>    The full output data path (optional)
                                  (~/code/chromium/src/third_party/angle/out/LinuxDebug)
                                  If this is not specified, the GN copy() command to include
                                  the files is output to the command line.

Output BUILD.gn commands example:
  ~/code/chromium/src/third_party/angle$ python src/tests/copyDeqpDataFiles.py \
      ~/code/chromium/src/third_party/angle/third_party/VK-GL-CTS/src
Copy dEQP data files example:
  ~/code/chromium/src/third_party/angle$ python src/tests/copyDeqpDataFiles.py \
      ~/code/chromium/src/third_party/angle/third_party/VK-GL-CTS/src \
      ~/code/chromium/src/third_party/angle/out/LinuxDebug/gen
    ''')


def initDataDirectories():
    dataDirectories.append(os.path.join("data"))
    dataDirectories.append(os.path.join("external", "graphicsfuzz", "data"))


def initPathReplacements():
    pathToReplace = os.path.join("external", "graphicsfuzz", "")  # Include trailing slash
    pathReplacements[pathToReplace] = ""


def updateDirectory(relativeDirectory):
    for pathReplacement in pathReplacements:
        if pathReplacement in relativeDirectory:
            return relativeDirectory.replace(pathReplacement, pathReplacements[pathReplacement])
    return relativeDirectory


def copyFile(srcPath, outDeqpDataDir, relativeDstDir, filename):
    outputDir = os.path.join(outDeqpDataDir, relativeDstDir)
    outputPath = os.path.join(outputDir, filename)
    try:
        os.makedirs(outputDir)
    except OSError as exc:
        if exc.errno == errno.EEXIST and os.path.isdir(outputDir):
            pass
        else:
            raise

    try:
        shutil.copyfile(srcPath, outputPath)
    # If source and destination are same
    except shutil.SameFileError:
        return


def main(argv):
    # print("argv: %s" % (" ".join(argv)))
    numArgs = len(argv)
    if numArgs == 0 or numArgs > 1:
        printHelp()
        sys.exit()

    deqpSrcDir = argv[0]

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
                # Update any destination paths to match what dEQP expects
                if relativeDirectory not in relativeDirectories:
                    relativeDirectories.append(relativeDirectory)
                dataFiles.append(os.path.join(relativeDirectory, filename))

    # Copy this output into 'src/tests/BUILD.gn'
    dataFiles.sort()
    relativeDirectories.sort()
    print("  # START AUTO-GENERATED CODE (copyDeqpDataFiles.py)")
    for relativeDirectory in relativeDirectories:
        print("  copy(\"" + relativeDirectory.replace("/", "_") + "\") {")
        print("    sources = [")
        for dataFile in dataFiles:
            path, filename = os.path.split(dataFile)
            if relativeDirectory == path:
                print("      \"$deqp_path/" + dataFile + "\",")
        print("    ]")
        print("    outputs = [")
        print("      \"$root_gen_dir/" + updateDirectory(relativeDirectory) +
              "/{{source_file_part}}\",")
        print("    ]")
        print("  }")
        print("")
    print("  angle_deqp_gles2_data = [")
    for dataFile in dataFiles:
        if "data/gles2/" in dataFile:
            print("          \"$root_gen_dir/" + updateDirectory(dataFile) + "\",")
    print("  ]")
    print("  angle_deqp_gles3_data = [")
    for dataFile in dataFiles:
        if "data/gles3/" in dataFile:
            print("          \"$root_gen_dir/" + updateDirectory(dataFile) + "\",")
    print("  ]")
    print("  angle_deqp_gles31_data = [")
    for dataFile in dataFiles:
        if "data/gles31/" in dataFile:
            print("          \"$root_gen_dir/" + updateDirectory(dataFile) + "\",")
    print("  ]")
    print("  # END AUTO-GENERATED CODE")
    print("")
    print("      # START AUTO-GENERATED CODE (copyDeqpDataFiles.py)")
    print("      deps = [")
    for relativeDirectory in relativeDirectories:
        print("        \":" + relativeDirectory.replace("/", "_") + "\",")
    print("      ]")
    print("      data = []")
    print("      if (_api == \"gles2\") {")
    print("        data = angle_deqp_gles2_data")
    print("      } else if (_api == \"gles3\") {")
    print("        data = angle_deqp_gles3_data")
    print("      } else if (_api == \"gles31\") {")
    print("        data = angle_deqp_gles31_data")
    print("      } else {")
    print(
        "        # Make sure we include something so that angle_deqp_libtester_main.cpp can find something."
    )
    print("        data = [ \"$root_gen_dir/data/gles2/shaders/misc.test\" ]")
    print("      }")
    print("      # END AUTO-GENERATED CODE")


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
