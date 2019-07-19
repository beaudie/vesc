#!/usr/bin/python3

# Copyright 2019 Google Inc.  All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys
import platform
import re
import subprocess

# inplace change and use style from .clang-format
CLANG_FORMAT_ARGS = ['-i', '-style=file']


def main(directory):
    system = platform.system()
    clang_format_exe = None
    if system == 'Linux':
        clang_format_exe = 'clang-format'
    elif system == 'Windows':
        clang_format_exe = 'clang-format.bat'
    else:
        print('Platform %s not supported' % system, file=sys.stderr)

    if not clang_format_exe:
        exit(1)

    partial_cmd = [clang_format_exe] + CLANG_FORMAT_ARGS

    for subdir, _, files in os.walk(directory):
        if re.search(r'third_party', subdir):
            continue

        for f in files:
            if f.endswith(('.c', '.h', '.cpp', '.hpp')):
                f_abspath = os.path.join(subdir, f)
                print("Applying clang-format on ", f_abspath)
                subprocess.run(partial_cmd + [f_abspath], check=True)


if __name__ == '__main__':
    if len(sys.argv) > 2:
        print('Too mang args', file=sys.stderr)

    elif len(sys.argv) == 2:
        main(os.path.join(os.getcwd(), sys.argv[1]))

    else:
        main(os.getcwd())
