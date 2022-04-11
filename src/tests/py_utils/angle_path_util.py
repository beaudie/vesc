# Copyright 2022 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import posixpath
import sys

ANGLE_ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))


def _AddToPathIfNeeded(path):
    if path not in sys.path:
        sys.path.insert(0, path)


def _DepsRootDir():
    if os.path.exists(os.path.join(ANGLE_ROOT_DIR, 'third_party', 'vulkan-deps')):
        return ANGLE_ROOT_DIR
    else:
        # Assume Chromium checkout
        return os.path.abspath(os.path.join(angle_root, '..', '..'))


def AddDepsDirToPath(posixpath_from_root):
    _AddToPathIfNeeded(os.path.join(_DepsRootDir(), *posixpath.split(posixpath_from_root)))
