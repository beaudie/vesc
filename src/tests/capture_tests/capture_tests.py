#! /usr/bin/env python3
#
# Copyright 2023 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

import argparse
import contextlib
import difflib
import json
import logging
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile
import time

SCRIPT_DIR = str(pathlib.Path(__file__).resolve().parent)
PY_UTILS = str(pathlib.Path(SCRIPT_DIR) / '..' / 'py_utils')
if PY_UTILS not in sys.path:
    os.stat(PY_UTILS) and sys.path.insert(0, PY_UTILS)
import angle_test_util


@contextlib.contextmanager
def temporary_dir(prefix=''):
    path = tempfile.mkdtemp(prefix=prefix)
    try:
        yield path
    finally:
        logging.info("Removing temporary directory: %s" % path)
        shutil.rmtree(path)


def diff_files(path, expected_path, fn):
    with open(path, 'rb') as f:
        content = f.read()
    with open(expected_path, 'rb') as f:
        expected_content = f.read()
    if content == expected_content:
        return False

    if fn.endswith('.gz'):
        print('Binary file is different: %s')
        return True

    diff = difflib.unified_diff(
        expected_content.decode().splitlines(),
        content.decode().splitlines(),
        fromfile=fn,
        tofile=fn,
    )
    print('Found diff in %s:', fn)
    print('\n'.join(diff))

    return True


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--isolated-script-test-output', type=str)
    parser.add_argument('--log', help='Logging level.', default='info')
    args, extra_flags = parser.parse_known_args()

    logging.basicConfig(level=args.log.upper())

    with temporary_dir() as temp_dir:
        exe = angle_test_util.ExecutablePathInCurrentDir('angle_end2end_tests')
        test_args = [
            '--gtest_filter=ActiveTextureCacheTest.UniformChangeUpdatesActiveTextureCache/ES2_Vulkan_SwiftShader',
            '--angle-per-test-capture-label'
        ]
        extra_env = {
            'ANGLE_FEATURE_OVERRIDES_ENABLED': 'forceRobustResourceInit:forceInitShaderVariables',
            'ANGLE_CAPTURE_ENABLED': '1',
            'ANGLE_CAPTURE_FRAME_END': '100',
            'ANGLE_CAPTURE_OUT_DIR': temp_dir,
        }
        subprocess.check_call([exe] + test_args, env={**os.environ.copy(), **extra_env})
        files = sorted(fn for fn in os.listdir(temp_dir))
        expected_dir = os.path.join(SCRIPT_DIR, 'expected')
        expected_files = sorted(fn for fn in os.listdir(expected_dir) if not fn.startswith('.'))
        if files != expected_files:
            logging.error('Capture produced a different set of files: %s\nDiff:\n%s\n', files,
                          '\n'.join(difflib.unified_diff(expected_files, files)))
            raise Exception('fileset diff')

        has_diffs = False
        for fn in files:
            has_diffs |= diff_files(os.path.join(temp_dir, fn), os.path.join(expected_dir, fn), fn)

        if has_diffs:
            raise Exception('file diff')

    if args.isolated_script_test_output:
        test_name = 'capture_test'
        results = {
            'tests': {
                test_name: {}
            },
            'interrupted': False,
            'seconds_since_epoch': time.time(),
            'path_delimiter': '.',
            'version': 3,
            'num_failures_by_type': {
                'FAIL': 0,
                'PASS': 0,
                'SKIP': 0,
            },
        }
        results['tests'][test_name][
            'ActiveTextureCacheTest.UniformChangeUpdatesActiveTextureCache'] = {
                'expected': 'PASS',
                'actual': 'PASS'
            }
        results['num_failures_by_type']['PASS'] += 1

        with open(args.isolated_script_test_output, 'w') as f:
            f.write(json.dumps(results, indent=2))


if __name__ == '__main__':
    sys.exit(main())
