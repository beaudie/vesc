#! /usr/bin/env vpython
#
# Copyright 2021 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# run_perf_test.py:
#   Runs ANGLE perf tests using some statistical averaging.

import argparse
import json
import logging
import os
import re
import sys

# Add //src/testing into sys.path for importing xvfb and test_env, and
# //src/testing/scripts for importing common.
d = os.path.dirname
THIS_DIR = d(os.path.abspath(__file__))
ANGLE_SRC_DIR = d(d(THIS_DIR))
sys.path.insert(0, os.path.join(ANGLE_SRC_DIR, 'testing'))
sys.path.insert(0, os.path.join(ANGLE_SRC_DIR, 'testing', 'scripts'))
# Handle the Chromium-relative directory as well. As long as one directory
# is valid, Python is happy.
CHROMIUM_SRC_DIR = d(d(ANGLE_SRC_DIR))
sys.path.insert(0, os.path.join(CHROMIUM_SRC_DIR, 'testing'))
sys.path.insert(0, os.path.join(CHROMIUM_SRC_DIR, 'testing', 'scripts'))

import common
import test_env
import xvfb

DEFAULT_TEST_SUITE = 'angle_perftests'
DEFAULT_LOG = 'info'
DEFAULT_SAMPLES = 5
DEFAULT_TRIALS = 3


def is_windows():
    return sys.platform == 'cygwin' or sys.platform.startswith('win')


def get_binary_name(binary):
    if is_windows():
        return '.\\%s.exe' % binary
    else:
        return './%s' % binary


def _run_and_get_output(args, cmd, env):
    lines = []
    with common.temporary_file() as tempfile_path:
        if args.xvfb:
            ret = xvfb.run_executable(cmd, env, stdoutfile=tempfile_path)
        else:
            ret = test_env.run_command_with_output(cmd, env=env, stdoutfile=tempfile_path)
        if ret:
            raise Exception('Error running test suite.')
        with open(tempfile_path) as f:
            for line in f:
                lines.append(line.strip())
    return lines


def _shard_tests(tests, shard_count, shard_index):
    return [tests[index] for index in range(shard_index, len(tests), shard_count)]


def _get_results_from_output(output, result):
    output = '\n'.join(output)
    m = re.search(r'Running (\d+) tests', output)
    if m and int(m.group(1)) > 1:
        raise Exception('Found more than one test result in output')

    # Results are reported in the format:
    # name_backend.result: story= value units.
    pattern = r'\.' + result + r':.*= ([0-9.]+)'
    logging.debug('searching for %s in output' % pattern)
    m = re.findall(pattern, output)
    if not m:
        print(output)
        raise Exception('Did not find the result "%s" in the test output' % result)

    return [float(value) for value in m]


def _get_tests_from_output(lines):
    seen_start_of_tests = False
    tests = []
    for line in lines:
        if line == 'Tests list:':
            seen_start_of_tests = True
        elif seen_start_of_tests:
            tests.append(line)
    return tests


def _truncated_list(data, n):
    """Compute a truncated list, n is truncation size"""
    if len(data) < n * 2:
        raise ValueError('list not large enough to truncate')
    return sorted(data)[n:-n]


def _mean(data):
    """Return the sample arithmetic mean of data."""
    n = len(data)
    if n < 1:
        raise ValueError('mean requires at least one data point')
    return float(sum(data)) / float(n)  # in Python 2 use sum(data)/float(n)


def _sum_of_square_deviations(data, c):
    """Return sum of square deviations of sequence data."""
    ss = sum((float(x) - c)**2 for x in data)
    return ss


def _coefficient_of_variation(data):
    """Calculates the population coefficient of variation."""
    n = len(data)
    if n < 2:
        raise ValueError('variance requires at least two data points')
    c = _mean(data)
    ss = _sum_of_square_deviations(data, c)
    pvar = ss / n  # the population variance
    stddev = (pvar**0.5)  # population standard deviation
    return stddev / c


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--isolated-script-test-output', type=str)
    parser.add_argument('--isolated-script-test-perf-output', type=str)
    parser.add_argument(
        '-f', '--filter', '--isolated-script-test-filter', type=str, help='Test filter.')
    parser.add_argument('--test-suite', help='Test suite to run.', default=DEFAULT_TEST_SUITE)
    parser.add_argument('--xvfb', help='Use xvfb.', action='store_true')
    parser.add_argument(
        '--shard-count',
        help='Number of shards for test splitting. Default is 1.',
        type=int,
        default=1)
    parser.add_argument(
        '--shard-index',
        help='Index of the current shard for test splitting. Default is 0.',
        type=int,
        default=0)
    parser.add_argument(
        '-l', '--log', help='Log output level. Default is %s.' % DEFAULT_LOG, default=DEFAULT_LOG)
    parser.add_argument(
        '-s',
        '--samples-per-test',
        help='Number of samples to run per test. Default is %d.' % DEFAULT_SAMPLES,
        type=int,
        default=DEFAULT_SAMPLES)
    parser.add_argument(
        '-t',
        '--trials-per-sample',
        help='Number of trials to run per sample. Default is %d.' % DEFAULT_TRIALS,
        type=int,
        default=DEFAULT_TRIALS)
    parser.add_argument(
        '--steps-per-trial', help='Fixed number of steps to run per trial.', type=int)

    args, extra_flags = parser.parse_known_args()
    logging.basicConfig(level=args.log.upper())

    # Get test list
    env = os.environ.copy()
    cmd = [get_binary_name(args.test_suite), '--list-tests']
    if args.filter:
        cmd.append('--gtest_filter=%s' % args.filter)
    lines = _run_and_get_output(args, cmd, env)
    tests = _get_tests_from_output(lines)

    # Get tests for this shard (if using sharding args)
    tests = _shard_tests(tests, args.shard_count, args.shard_index)

    # Run tests
    for test in tests:
        cmd = [get_binary_name(args.test_suite), '--gtest_filter=%s' % test]
        if args.steps_per_trial:
            steps_per_trial = args.steps_per_trial
        else:
            cmd_calibrate = cmd + ['--calibration']
            calibrate_output = _run_and_get_output(args, cmd_calibrate, env)
            steps_per_trial = _get_results_from_output(calibrate_output, 'steps_to_run')
            assert (len(steps_per_trial) == 1)
            steps_per_trial = int(steps_per_trial[0])
        logging.info('Running %s %d times with %d trials and %d steps per trial.' %
                     (test, args.samples_per_test, args.trials_per_sample, steps_per_trial))
        wall_times = []
        for sample in range(args.samples_per_test):
            cmd_run = cmd + [
                '--steps-per-trial=%d' % steps_per_trial,
                '--trials=%d' % args.trials_per_sample
            ]
            output = _run_and_get_output(args, cmd_run, env)
            sample_wall_times = _get_results_from_output(output, 'wall_time')
            logging.info('Sample %d wall_time results: %s' % (sample, str(sample_wall_times)))
            wall_times += sample_wall_times

        if len(wall_times) > 7:
            truncation_n = len(wall_times) >> 3
            logging.info('Truncation: Removing the %d highest and lowest times from wall_times.' %
                         truncation_n)
            wall_times = _truncated_list(wall_times, truncation_n)

        if len(wall_times) > 1:
            logging.info("Mean wall_time for %s is %.2f, with coefficient of variation %.2f%%" %
                         (test, _mean(wall_times),
                          (_coefficient_of_variation(wall_times) * 100.0)))

    return 0


# This is not really a "script test" so does not need to manually add
# any additional compile targets.
def main_compile_targets(args):
    json.dump([], args.output)


if __name__ == '__main__':
    # Conform minimally to the protocol defined by ScriptTest.
    if 'compile_targets' in sys.argv:
        funcs = {
            'run': None,
            'compile_targets': main_compile_targets,
        }
        sys.exit(common.run_script(sys.argv[1:], funcs))
    sys.exit(main())
