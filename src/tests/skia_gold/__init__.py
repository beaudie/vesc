# Copyright 2020 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

d = os.path.dirname
THIS_DIR = d(os.path.abspath(__file__))
ANGLE_SRC_DIR = d(d(d(THIS_DIR)))
ANGLE_BUILD_DIR = os.path.join(ANGLE_SRC_DIR, 'build')

if os.path.exists(ANGLE_BUILD_DIR):
    if ANGLE_BUILD_DIR not in sys.path:
        sys.path.insert(0, ANGLE_BUILD_DIR)
else:
    CHROMIUM_SRC_DIR = d(d(ANGLE_SRC_DIR))
    CHROMIUM_BUILD_DIR = os.path.join(CHROMIUM_SRC_DIR, 'build')
    if CHROMIUM_BUILD_DIR not in sys.path:
        sys.path.insert(0, CHROMIUM_BUILD_DIR)
