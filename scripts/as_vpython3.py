#!/usr/bin/env python
#
# Copyright 2022 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
""" Runs script from args with vpython3 to spawn from swarming python."""

import os
import subprocess
import sys

if __name__ == '__main__':
    root = os.path.join(os.path.dirname(os.path.realpath(__file__)), '..')
    assert sys.argv[1].startswith('//')
    script_path = os.path.join(root, sys.argv[1][2:])
    sys.exit(subprocess.call(['vpython3', script_path] + sys.argv[2:]))
