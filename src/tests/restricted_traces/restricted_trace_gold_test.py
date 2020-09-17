#! /usr/bin/env python
#
# [VPYTHON:BEGIN]
# wheel: <
#  name: "infra/python/wheels/psutil/${vpython_platform}"
#  version: "version:5.2.2"
# >
# [VPYTHON:END]
#
# Copyright 2020 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# restricted_trace_gold_test.py:
#   Uses Skia Gold (https://skia.org/dev/testing/skiagold) to run pixel tests with ANGLE traces.

import argparse
import json
import os
import sys
import tempfile
import traceback

# Add //src/testing into sys.path for importing xvfb and test_env, and
# //src/testing/scripts for importing common.
# FIXME: handle both Chromium and ANGLE dir
d = os.path.dirname
THIS_DIR = d(os.path.abspath(__file__))
ANGLE_SRC_DIR = d(d(d(THIS_DIR)))
sys.path.insert(0, os.path.join(ANGLE_SRC_DIR, 'testing'))
sys.path.insert(0, os.path.join(ANGLE_SRC_DIR, 'testing', 'scripts'))

import common
import test_env
import xvfb


def IsWindows():
    return sys.platform == 'cygwin' or sys.platform.startswith('win')


DEFAULT_TEST_SUITE = 'angle_perftests'
DEFAULT_TEST_PREFIX = '--gtest_filter=TracePerfTest.Run/vulkan_'


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--isolated-script-test-output', type=str, required=True)
    parser.add_argument('--test-suite', help='Test suite to run.', default=DEFAULT_TEST_SUITE)
    parser.add_argument('--xvfb', help='Start xvfb.', action='store_true')

    args, extra_flags = parser.parse_known_args()
    env = os.environ.copy()

    if 'GTEST_SHARD_INDEX' in env or 'GTEST_TOTAL_SHARDS' in env:
        print('Sharding not yet implemented.')
        sys.exit(1)

    rc = 0
    try:
        if IsWindows():
            args.test_suite = '.\\%s.exe' % args.test_suite
        else:
            args.test_suite = './%s' % args.test_suite

        # read test set
        json_name = os.path.join(ANGLE_SRC_DIR, 'src', 'tests', 'restricted_traces',
                                 'restricted_traces.json')
        with open(json_name) as fp:
            tests = json.load(fp)

        # FIXME: need a way to remove the temp dir on exit.
        screenshot_dir = tempfile.mkdtemp(prefix='angle_trace')

        for test in tests['traces']:
            with common.temporary_file() as tempfile_path:
                # FIXME: use chromium --render-test-output-dir
                # https://source.chromium.org/chromium/chromium/src/+/master:build/android/pylib/local/device/local_device_instrumentation_test_run.py;l=577?q=local_device_instrumentation_test_run.py&ss=chromium
                cmd = [
                    args.test_suite,
                    DEFAULT_TEST_PREFIX + test,
                    '--screenshot-dir',
                    screenshot_dir,
                    '--one-frame-only',
                ] + extra_flags

                if args.xvfb:
                    rc = xvfb.run_executable(cmd, env, stdoutfile=tempfile_path)
                else:
                    rc = test_env.run_command_with_output(cmd, env=env, stdoutfile=tempfile_path)

    except Exception:
        traceback.print_exc()
        rc = 1

    return rc


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
