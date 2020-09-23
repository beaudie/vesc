#! /usr/bin/env python
#
# [VPYTHON:BEGIN]
# wheel: <
#  name: "infra/python/wheels/psutil/${vpython_platform}"
#  version: "version:5.2.2"
# >
# wheel: <
#  name: "infra/python/wheels/six-py2_py3"
#  version: "version:1.10.0"
# >
# [VPYTHON:END]
#
# Copyright 2020 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# restricted_trace_gold_tests.py:
#   Uses Skia Gold (https://skia.org/dev/testing/skiagold) to run pixel tests with ANGLE traces.
#
#   Requires vpython to run standalone. Run with --help for usage instructions.

import argparse
import contextlib
import json
import os
import platform
import shutil
import sys
import tempfile
import time
import traceback

from skia_gold import angle_skia_gold_properties
from skia_gold import angle_skia_gold_session_manager

# Add //src/testing into sys.path for importing xvfb and test_env, and
# //src/testing/scripts for importing common.
d = os.path.dirname
THIS_DIR = d(os.path.abspath(__file__))
ANGLE_SRC_DIR = d(d(d(THIS_DIR)))
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


def IsWindows():
    return sys.platform == 'cygwin' or sys.platform.startswith('win')


DEFAULT_TEST_SUITE = 'angle_perftests'
DEFAULT_TEST_PREFIX = '--gtest_filter=TracePerfTest.Run/vulkan_'


@contextlib.contextmanager
def temporary_dir(prefix=''):
    path = tempfile.mkdtemp(prefix=prefix)
    try:
        yield path
    finally:
        shutil.rmtree(path)


def add_skia_gold_args(parser):
    group = parser.add_argument_group('Skia Gold Arguments')
    group.add_argument('--git-revision', help='Revision being tested.', default=None)
    group.add_argument(
        '--gerrit-issue', help='For Skia Gold integration. Gerrit issue ID.', default='')
    group.add_argument(
        '--gerrit-patchset',
        help='For Skia Gold integration. Gerrit patch set number.',
        default='')
    group.add_argument(
        '--buildbucket-id', help='For Skia Gold integration. Buildbucket build ID.', default='')
    group.add_argument(
        '--bypass-skia-gold-functionality',
        action='store_true',
        default=False,
        help='Bypass all interaction with Skia Gold, effectively disabling the '
        'image comparison portion of any tests that use Gold. Only meant to '
        'be used in case a Gold outage occurs and cannot be fixed quickly.')
    group.add_argument(
        '--no-skia-gold-failure',
        action='store_true',
        default=False,
        help='For Skia Gold integration. Always report that the test passed '
        'even if the Skia Gold image comparison reported a failure, but '
        'otherwise perform the same steps as usual.')
    group.add_argument(
        '--local-pixel-tests',
        action='store_true',
        default=None,
        help='Specifies to run the test harness in local run mode or not. When '
        'run in local mode, uploading to Gold is disabled and links to '
        'help with local debugging are output. Running in local mode also '
        'implies --no-luci-auth. If both this and --no-local-pixel-tests are '
        'left unset, the test harness will attempt to detect whether it is '
        'running on a workstation or not and set this option accordingly.')
    group.add_argument(
        '--no-local-pixel-tests',
        action='store_false',
        dest='local_pixel_tests',
        help='Specifies to run the test harness in non-local (bot) mode. When '
        'run in this mode, data is actually uploaded to Gold and triage links '
        'arge generated. If both this and --local-pixel-tests are left unset, '
        'the test harness will attempt to detect whether it is running on a '
        'workstation or not and set this option accordingly.')
    group.add_argument(
        '--no-luci-auth',
        action='store_true',
        default=False,
        help='Don\'t use the service account provided by LUCI for '
        'authentication for Skia Gold, instead relying on gsutil to be '
        'pre-authenticated. Meant for testing locally instead of on the bots.')


def run_wrapper(args, cmd, env, stdoutfile=None):
    if args.xvfb:
        return xvfb.run_executable(cmd, env, stdoutfile=stdoutfile)
    else:
        return test_env.run_command_with_output(cmd, env=env, stdoutfile=stdoutfile)


def to_hex(num):
    return hex(int(num))


def to_hex_or_none(num):
    return 'None' if num == None else to_hex(num)


def to_non_empty_string_or_none(val):
    return 'None' if val == '' else str(val)


def to_non_empty_string_or_none_dict(d, key):
    return 'None' if not key in d else to_non_empty_string_or_none(d[key])


def get_binary_name(binary):
    if IsWindows():
        return '.\\%s.exe' % binary
    else:
        return './%s' % args.test_suite


def get_combined_hardware_identifier(gpu_info):
    """Combine all relevant hardware identifiers into a single key.

  This makes Gold forwarding more precise by allowing us to forward explicit
  configurations instead of individual components.
  """
    # vendor_id = _ToHexOrNone(img_params.vendor_id)
    # device_id = _ToHexOrNone(img_params.device_id)
    # device_string = _ToNonEmptyStrOrNone(img_params.device_string)
    # combined_hw_identifiers = ('vendor_id:{vendor_id}, '
    #                            'device_id:{device_id}, '
    #                            'device_string:{device_string}')
    # combined_hw_identifiers = combined_hw_identifiers.format(
    #     vendor_id=vendor_id, device_id=device_id, device_string=device_string)
    # return combined_hw_identifiers
    # TODO(jmadill): Implement.
    return None


def get_skia_gold_keys(args):
    """Get all the JSON metadata that will be passed to golctl."""
    # All values need to be strings, otherwise goldctl fails.

    with common.temporary_file() as tempfile_path:
        binary = get_binary_name('angle_system_info_test')
        if run_wrapper(args, [binary, '--vulkan'], {}, tempfile_path) != 0:
            raise Exception('Error getting system info.')

        with open(tempfile_path) as json_output:
            json_data = json.load(json_output)

    if not 'gpus' in json_data or len(json_data['gpus']) == 0 or not 'activeGPUIndex' in json_data:
        raise Exception('Error getting system info.')

    active_gpu = json_data['gpus'][json_data['activeGPUIndex']]

    angle_keys = {
        'vendor_id': to_hex_or_none(active_gpu['vendorId']),
        'device_id': to_hex_or_none(active_gpu['deviceId']),
        'model_name': to_non_empty_string_or_none_dict(active_gpu, 'machineModelVersion'),
        'manufacturer_name': to_non_empty_string_or_none_dict(active_gpu, 'machineManufacturer'),
        'os': to_non_empty_string_or_none(platform.system()),
        'os_version': to_non_empty_string_or_none(platform.version()),
        'driver_version': to_non_empty_string_or_none_dict(active_gpu, 'driverVersion'),
        'driver_vendor': to_non_empty_string_or_none_dict(active_gpu, 'driverVendor'),
        'combined_hardware_identifier': get_combined_hardware_identifier(active_gpu),
    }

    return angle_keys


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--isolated-script-test-output', type=str, required=True)
    parser.add_argument('--isolated-script-test-perf-output', type=str)
    parser.add_argument('--test-suite', help='Test suite to run.', default=DEFAULT_TEST_SUITE)
    parser.add_argument('--render-test-output-dir', help='Directory to store screenshots')
    parser.add_argument('--xvfb', help='Start xvfb.', action='store_true')

    add_skia_gold_args(parser)

    args, extra_flags = parser.parse_known_args()
    env = os.environ.copy()

    if 'GTEST_TOTAL_SHARDS' in env and int(env['GTEST_TOTAL_SHARDS']) != 1:
        print('Sharding not yet implemented.')
        sys.exit(1)

    results = {
        'tests': {
            'angle_restricted_trace_gold_tests': {}
        },
        'interrupted': False,
        'seconds_since_epoch': time.time(),
        'path_delimiter': '.',
        'version': 3,
        'num_failures_by_type': {
            'PASS': 0,
            'FAIL': 0,
        }
    }

    keys = get_skia_gold_keys(args)
    print(keys)

    result_tests = results['tests']['angle_restricted_trace_gold_tests']

    def run_tests(args, tests, extra_flags, env, screenshot_dir):
        for test in tests['traces']:
            with common.temporary_file() as tempfile_path:
                cmd = [
                    args.test_suite,
                    DEFAULT_TEST_PREFIX + test,
                    '--render-test-output-dir=%s' % screenshot_dir,
                    '--one-frame-only',
                ] + extra_flags

                rc = run_wrapper(args, cmd, env, tempfile_path)

                pass_fail = 'PASS' if rc == 0 else 'FAIL'
                result_tests[test] = {'expected': 'PASS', 'actual': pass_fail}
                results['num_failures_by_type'][pass_fail] += 1

        return results['num_failures_by_type']['FAIL'] == 0

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

        if args.render_test_output_dir:
            rc = run_tests(args, tests, extra_flags, env, args.render_test_output_dir)
        else:
            with temporary_dir('angle_trace_') as temp_dir:
                rc = run_tests(args, tests, extra_flags, env, temp_dir)

    except Exception:
        traceback.print_exc()
        rc = 1

    if args.isolated_script_test_output:
        with open(args.isolated_script_test_output, 'w') as out_file:
            out_file.write(json.dumps(results))

    if args.isolated_script_test_perf_output:
        with open(args.isolated_script_test_perf_output, 'w') as out_file:
            out_file.write(json.dumps({}))

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
