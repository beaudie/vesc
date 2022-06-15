#! /usr/bin/env python3
#
# Copyright 2022 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# android_helper_tests: Self-tests for android_helper.py

import argparse
import json
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

    if args.isolated_script_test_output:
        results = {
            'tests': {
                'angle_android_helper_tests': {
                    'RunSmokeTest': {
                        'expected': 'PASS',
                        'actual': 'PASS',
                    },
                },
            },
            'interrupted': False,
            'seconds_since_epoch': time.time(),
            'path_delimiter': '.',
            'version': 3,
            'num_failures_by_type': {
                'FAIL': 0,
                'PASS': 1,
                'SKIP': 0,
            },
        }
        with open(args.isolated_script_test_output, 'w') as out_file:
            out_file.write(json.dumps(results, indent=2))


if __name__ == '__main__':
    sys.exit(main())
