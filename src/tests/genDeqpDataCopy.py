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
    print("Usage:")
    print("  python genDeqpDataCopy.py <dEQP directory>")
    print(
        "\n  The genDeqpDataCopy script generates a BUILD.gn copy() command to embed tests/BUILD.gn."
    )
    print("\nArguments: ")
    print(
        " <dEQP directory>                The dEQP directory (~/code/chromium/src/third_party/angle/third_party/deqp/src)."
    )
    print(
        " <output dEQP data directory>    The dEQP data directory (~/code/chromium/src/third_party/angle/out/LinuxDebug) in the output directory."
    )
    print(" -h                              Print this help message")


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
    if (len(argv) < 2):
        printHelp()
        sys.exit()

    deqpSrcDir = argv[0]
    outDir = argv[1]

    if (len(deqpSrcDir) == 0):
        printHelp()
        sys.exit()

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
                os.path.join(outDir, relativeDirectory)
                copyFile(os.path.join(root, filename), outDir, relativeDirectory, filename)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
