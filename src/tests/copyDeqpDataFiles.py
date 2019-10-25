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

dataDirectories = [
    "data/",
    "external/graphicsfuzz/data/",
]

excludedDirectories = [
    ".git",
]

excludedFilenames = [
    "LICENSE",
]

# Dictionary of parts of paths that need to be replaced
# Key: Part of path to be replaced
# Value: What to replace it with
pathReplacements = {"external/graphicsfuzz/": ""}


def printHelp():
    print('''
Usage:
  python copyDeqpDataFiles.py <dEQP directory> <output dEQP data directory>

The copyDeqpDataFiles script generates a BUILD.gn copy() command to embed tests/BUILD.gn.

Arguments:
  <dEQP directory>                The full dEQP path
                                  (~/code/chromium/src/third_party/angle/third_party/deqp/src).
  <output dEQP data directory>    The full output data path (optional)
                                  (~/code/chromium/src/third_party/angle/out/LinuxDebug)
                                  If this is not specified, the GN copy() command to include
                                  the files is output to the command line.
    ''')


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
    if numArgs == 0 or numArgs > 2:
        printHelp()
        sys.exit()

    outputDataBlockOnly = True
    deqpSrcDir = argv[0]
    if numArgs == 2:
        outDir = argv[1]
        outputDataBlockOnly = False

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
                relativeDirectory = updateDirectory(relativeDirectory)
                dataFiles.append(os.path.join(relativeDirectory, filename))
                if not outputDataBlockOnly:
                    os.path.join(outDir, relativeDirectory)
                    copyFile(os.path.join(root, filename), outDir, relativeDirectory, filename)

    if outputDataBlockOnly:
        # Copy this output into 'src/tests/BUILD.gn'
        dataFiles.sort()
        print("      data = []")
        print("      if (_api == \"gles2\") {")
        print("        data += [")
        for dataFile in dataFiles:
            if "data/gles2/" in dataFile:
                print("          \"$root_gen_dir/" + dataFile + "\",")
        print("        ]")
        print("      } else if (_api == \"gles3\") {")
        print("        data += [")
        for dataFile in dataFiles:
            if "data/gles3/" in dataFile:
                print("          \"$root_gen_dir/" + dataFile + "\",")
        print("        ]")
        print("      } else if (_api == \"gles31\") {")
        print("        data += [")
        for dataFile in dataFiles:
            if "data/gles31/" in dataFile:
                print("          \"$root_gen_dir/" + dataFile + "\",")
        print("        ]")
        print("      } else {")
        print(
            "        # Make sure we include something so that angle_deqp_libtester_main.cpp can find something."
        )
        print("        data += [ \"$root_gen_dir/data/gles2/shaders/misc.test\" ]")
        print("      }")


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
