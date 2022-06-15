#! /usr/bin/env python3
#
# Copyright 2022 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# android_helper_tests: Self-tests for android_helper.py

import argparse
import pathlib
import sys

PY_UTILS = str(pathlib.Path(__file__).resolve().parents[1] / 'py_utils')
if PY_UTILS not in sys.path:
    os.stat(PY_UTILS) and sys.path.insert(0, PY_UTILS)

import angle_test_util
import android_helper

DEFAULT_LOG = 'info'


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--isolated-script-test-output', type=str)
    parser.add_argument('--isolated-script-test-perf-output', type=str)
    parser.add_argument('-f', '--isolated-script-test-filter', '--filter', type=str)
    parser.add_argument(
        '-l', '--log', help='Log output level. Default is %s.' % DEFAULT_LOG, default=DEFAULT_LOG)

    args, extra_flags = parser.parse_known_args()
    angle_test_util.setupLogging(args.log.upper())

    android_helper.PrepareTestSuite('angle_perftests')
    android_helper.RunSmokeTest()


if __name__ == '__main__':
    sys.exit(main())
