#!/usr/bin/python2

# Copyright 2015 Google Inc.  All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Generate .gclient file for Angle.

Because gclient won't accept "--name ." use a different name then edit.
"""

import argparse
import subprocess
import sys


def is_linux_or_mac():
    return sys.platform.startswith('linux') or sys.platform == 'darwin'


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--minimal', help='Skips checkout out very large DEPS.', action='store_true')
    parser.add_argument(
        '--android',
        help='Check out Android DEPS.',
        action='store_true',
        default=is_linux_or_mac())
    args, extra_flags = parser.parse_known_args()

    gclient_cmd = [
        'gclient', 'config', '--name', 'change2dot', '--unmanaged',
        'https://chromium.googlesource.com/angle/angle.git'
    ]
    if args.minimal:
        gclient_cmd += ['--custom-var', 'angle_minimal_checkout=True']

    try:
        rc = subprocess.call(gclient_cmd, shell=True)
    except OSError:
        print 'could not run "%s" via shell' % gclient_cmd
        sys.exit(1)

    if rc:
        print 'failed command: "%s"' % gclient_cmd
        sys.exit(1)

    with open('.gclient') as gclient_file:
        content = gclient_file.read()

    content = content.replace('change2dot', '.')
    if args.android:
        content += 'target_os = [ \'android\' ]\n'

    with open('.gclient', 'w') as gclient_file:
        gclient_file.write(content)

    print 'created .gclient'


if __name__ == '__main__':
    main()
