#!/usr/bin/python3

# Copyright 2019 Google Inc.  All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys
import platform
import subprocess

CLANG_FORMAT_SH = 'clang_format.sh'
CLANG_FORMAT_BAT = 'clang_format.bat'


def main():
    script_dir = os.path.dirname(__file__)

    system = platform.system()
    exe = None
    if system == 'Linux':
        exe = os.path.join(script_dir, CLANG_FORMAT_SH)
    elif system == 'Windows':
        print('Platform %s not supported' % system, file=sys.stderr)
        # TODO:
        # exe = os.path.join(script_dir, CLANG_FORMAT_BAT)
    else:
        print('Platform %s not supported' % system, file=sys.stderr)

    if exe:
        subprocess.run(exe, check=True)
    else:
        exit(1)


if __name__ == '__main__':
    main()
