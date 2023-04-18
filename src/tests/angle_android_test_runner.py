import argparse
import json
import logging
import os
import pathlib
import sys

PY_UTILS = str(pathlib.Path(__file__).resolve().parent / 'py_utils')
if PY_UTILS not in sys.path:
    os.stat(PY_UTILS) and sys.path.insert(0, PY_UTILS)
import android_helper
import angle_test_util


def main(raw_args):
    parser = argparse.ArgumentParser()
    parser.add_argument('--suite', help='Test suite binary.', choices=['angle_trace_tests'])
    parser.add_argument('-l', '--log', help='Logging level.', default='warn')
    parser.add_argument('--output-directory')
    parser.add_argument('--wrapper-script-args')
    parser.add_argument('--runtime-deps-path')
    parser.add_argument('--list-tests', help='List tests.', action='store_true')
    parser.add_argument(
        '-f',
        '--filter',
        '--isolated-script-test-filter',
        '--gtest_filter',
        type=str,
        help='Test filter.')

    args, extra_args = parser.parse_known_args(raw_args)

    logging.basicConfig(level=args.log.upper())

    os.chdir(args.output_directory)

    android_helper.Initialize(args.suite)
    assert android_helper.IsAndroid()

    rc, output, _ = android_helper.RunTests(
        args.suite, ['--list-tests', '--verbose'] + extra_args, log_output=False)
    if rc != 0:
        logging.fatal('Could not find test list from test output:\n%s' % output)
        return rc

    tests = angle_test_util.GetTestsFromOutput(output)
    if args.filter:
        tests = angle_test_util.FilterTests(tests, args.filter)

    if args.list_tests:
        for test in tests:
            print(test)
        return 0

    if args.suite == 'angle_trace_tests':
        traces = set(android_helper.GetTraceFromTestName(test) for test in tests)
        android_helper.PrepareRestrictedTraces(traces)

    flags = ['--gtest_filter=' + args.filter] if args.filter else []
    return android_helper.RunTests(args.suite, flags + extra_args)[0]


if __name__ == "__main__":
    sys.exit(main(sys.argv[2:]))
