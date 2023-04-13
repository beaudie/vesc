# Copyright 2022 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import pathlib
import posixpath
import sys

ANGLE_ROOT_DIR = str(pathlib.Path(__file__).resolve().parents[3])


def AddDepsDirToPath(posixpath_from_root):
    relative_path = os.path.join(*posixpath.split(posixpath_from_root))
    full_path = os.path.join(ANGLE_ROOT_DIR, relative_path)
    if not os.path.exists(full_path):
        # Assume Chromium checkout
        chromium_root_dir = os.path.abspath(os.path.join(ANGLE_ROOT_DIR, '..', '..'))
        full_path = os.path.join(chromium_root_dir, relative_path)
        assert os.path.exists(full_path)

    if full_path not in sys.path:
        sys.path.insert(0, full_path)


def print_parent_directories():

    # Get the parent directory of the specified directory.
    current_dir = os.getcwd()
    print("Current dir: %s" % current_dir)

    current_contents = os.listdir(current_dir)
    print("Contents of current dir: ")
    print("\n".join(sorted(current_contents)))

    parent_directory = os.path.dirname(current_dir)
    print("\n\nParent dir: %s\n\n" % parent_directory)

    parent_contents = os.listdir(parent_directory)
    print("Contents of parent dir: ")
    print("\n".join(parent_contents))

    binary_dir = current_dir + '/angle_trace_tests_android_binaries__dist'
    print("\n\nBinary dir: %s\n\n" % binary_dir)

    binary_dir_contents = os.listdir(binary_dir)
    print("Contents of parent binary_dir: ")
    print("\n".join(binary_dir_contents))
