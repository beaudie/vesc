#! /usr/bin/env python3
#
# Copyright 2022 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# run_angle_android_test.py:
#   Runs ANGLE tests using android_helper wrapper, primarily for angle_perftests.
#   Example:
#   (cd out/Android; ../../src/tests/py_utils/run_angle_android_test.py \
#     --filter=TracePerfTest.Run/native_words_with_friends_2 \
#     --no-warmup --steps-per-trial 1 --trials 1)

import argparse
import logging

import android_helper
import fnmatch


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', '--filter', type=str, help='Test filter.')
    parser.add_argument('--test-suite', help='Test suite to run.', default='angle_perftests')
    parser.add_argument('--list-tests', help='List tests.', action='store_true')
    parser.add_argument('-l', '--log', help='Logging level.', default='info')

    args, extra_flags = parser.parse_known_args()

    logging.basicConfig(level=args.log.upper())

    assert not any('--gtest_filter' in a for a in extra_flags)

    android_helper.PrepareTestSuite(args.test_suite)

    tests = android_helper.ListTests()
    if args.filter:
        tests = [test for test in tests if fnmatch.fnmatch(test, args.filter)]

    if args.list_tests:
        for test in tests:
            print(test)
        return

    traces = set(android_helper.GetTraceFromTestName(test) for test in tests)
    android_helper.PrepareRestrictedTraces(traces)

    android_helper.RunTests(['--gtest_filter=' + args.filter] + extra_flags)


if __name__ == '__main__':
    main()
