#! /usr/bin/env python3
#
# Copyright 2020 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
'''
Script that re-captures the traces in the restricted trace folder. We can
use this to update traces without needing to re-run the app on a device.
'''

import argparse
import fnmatch
import json
import logging
import os
import random
import shutil
import stat
import subprocess
import sys
import tempfile

from gen_restricted_traces import read_json as read_json, write_json as write_json
from pathlib import Path

DEFAULT_TEST_SUITE = 'angle_perftests'
DEFAULT_TEST_JSON = 'restricted_traces.json'
DEFAULT_LOG_LEVEL = 'info'
DEFAULT_BACKUP_FOLDER = 'retrace-backups'

EXIT_SUCCESS = 0
EXIT_FAILURE = 1


def get_trace_json_path(trace):
    return os.path.join(get_script_dir(), trace, f'{trace}.json')


def load_trace_json(trace):
    json_file_name = get_trace_json_path(trace)
    return read_json(json_file_name)


def get_context(trace):
    """Returns the trace context number."""
    json_data = load_trace_json(trace)
    return str(json_data['WindowSurfaceContextID'])


def get_script_dir():
    return os.path.dirname(sys.argv[0])


def context_header(trace, trace_path):
    context_id = get_context(trace)
    header = '%s_context%s.h' % (trace, context_id)
    return os.path.join(trace_path, header)


def src_trace_path(trace):
    return os.path.join(get_script_dir(), trace)


def get_num_frames(json_data):
    metadata = json_data['TraceMetadata']
    return metadata['FrameEnd'] - metadata['FrameStart'] + 1


def get_gles_version(json_data):
    metadata = json_data['TraceMetadata']
    return (metadata['ContextClientMajorVersion'], metadata['ContextClientMinorVersion'])


def set_gles_version(json_data, version):
    metadata = json_data['TraceMetadata']
    metadata['ContextClientMajorVersion'] = version[0]
    metadata['ContextClientMinorVersion'] = version[1]


def save_trace_json(trace, data):
    json_file_name = get_trace_json_path(trace)
    return write_json(json_file_name, data)


def path_contains_header(path):
    for file in os.listdir(path):
        if fnmatch.fnmatch(file, '*.h'):
            return True
    return False


def chmod_directory(directory, perm):
    assert os.path.isdir(directory)
    for file in os.listdir(directory):
        fn = os.path.join(directory, file)
        os.chmod(fn, perm)


def ensure_rmdir(directory):
    if os.path.isdir(directory):
        chmod_directory(directory, stat.S_IWRITE)
        shutil.rmtree(directory)


def copy_trace_folder(old_path, new_path):
    logging.info('%s -> %s' % (old_path, new_path))
    ensure_rmdir(new_path)
    shutil.copytree(old_path, new_path)


def touch_trace_folder(trace_path):
    for file in os.listdir(trace_path):
        (Path(trace_path) / file).touch()


def backup_traces(args, traces):
    for trace in fnmatch.filter(traces, args.traces):
        trace_path = src_trace_path(trace)
        trace_backup_path = os.path.join(args.out_path, trace)
        copy_trace_folder(trace_path, trace_backup_path)


def restore_traces(args, traces):
    for trace in fnmatch.filter(traces, args.traces):
        trace_path = src_trace_path(trace)
        trace_backup_path = os.path.join(args.out_path, trace)
        if not os.path.isdir(trace_backup_path):
            logging.error('Trace folder not found at %s' % trace_backup_path)
        else:
            copy_trace_folder(trace_backup_path, trace_path)
            touch_trace_folder(trace_path)


def run_autoninja(args):
    autoninja_binary = 'autoninja'
    if os.name == 'nt':
        autoninja_binary += '.bat'

    autoninja_args = [autoninja_binary, '-C', args.gn_path, args.test_suite]
    logging.debug('Calling %s' % ' '.join(autoninja_args))
    if args.verbose:
        subprocess.check_call(autoninja_args)
    else:
        subprocess.check_call(autoninja_args, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def run_test_suite(args, trace, max_steps, additional_args, additional_env):
    trace_binary = os.path.join(args.gn_path, args.test_suite)
    if os.name == 'nt':
        trace_binary += '.exe'

    renderer = 'vulkan' if args.no_swiftshader else 'vulkan_swiftshader'
    trace_filter = '--gtest_filter=TracePerfTest.Run/%s_%s' % (renderer, trace)
    run_args = [
        trace_binary,
        trace_filter,
        '--max-steps-performed',
        str(max_steps),
    ] + additional_args
    if not args.no_swiftshader:
        run_args += ['--enable-all-trace-tests']

    env = {**os.environ.copy(), **additional_env}
    env_string = ' '.join(['%s=%s' % item for item in additional_env.items()])
    if env_string:
        env_string += ' '

    logging.info('%s%s' % (env_string, ' '.join(run_args)))
    if args.verbose:
        subprocess.check_call(run_args, env=env)
    else:
        subprocess.check_call(
            run_args, env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def upgrade_traces(args, traces):
    run_autoninja(args)

    failures = []

    for trace in fnmatch.filter(traces, args.traces):
        logging.debug('Tracing %s' % trace)

        trace_path = os.path.abspath(os.path.join(args.out_path, trace))
        if not os.path.isdir(trace_path):
            os.makedirs(trace_path)
        elif args.no_overwrite and path_contains_header(trace_path):
            logging.info('Skipping "%s" because the out folder already exists' % trace)
            continue

        json_data = load_trace_json(trace)
        num_frames = get_num_frames(json_data)

        metadata = json_data['TraceMetadata']
        logging.debug('Read metadata: %s' % str(metadata))

        max_steps = min(args.limit, num_frames) if args.limit else num_frames

        # We start tracing from frame 2. --retrace-mode issues a Swap() after Setup() so we can
        # accurately re-trace the MEC.
        additional_env = {
            'ANGLE_CAPTURE_LABEL': trace,
            'ANGLE_CAPTURE_OUT_DIR': trace_path,
            'ANGLE_CAPTURE_FRAME_START': '2',
            'ANGLE_CAPTURE_FRAME_END': str(max_steps + 1),
        }
        if args.validation:
            additional_env['ANGLE_CAPTURE_VALIDATION'] = '1'
            # Also turn on shader output init to ensure we have no undefined values.
            # This feature is also enabled in replay when using --validation.
            additional_env[
                'ANGLE_FEATURE_OVERRIDES_ENABLED'] = 'allocateNonZeroMemory:forceInitShaderVariables'
        if args.validation_expr:
            additional_env['ANGLE_CAPTURE_VALIDATION_EXPR'] = args.validation_expr
        if args.trim:
            additional_env['ANGLE_CAPTURE_TRIM_ENABLED'] = '1'
        if args.no_trim:
            additional_env['ANGLE_CAPTURE_TRIM_ENABLED'] = '0'

        additional_args = ['--retrace-mode']

        try:
            run_test_suite(args, trace, max_steps, additional_args, additional_env)

            json_file = get_trace_json_path(trace)
            if not os.path.exists(json_file):
                logging.error(
                    f'There was a problem tracing "{trace}", could not find json file: {json_file}'
                )
                failures += [trace]
        except subprocess.CalledProcessError:
            logging.exception('There was an exception running "%s":' % trace)
            failures += [trace]

    if failures:
        print('The following traces failed to upgrade:\n')
        print('\n'.join(['  ' + trace for trace in failures]))
        return EXIT_FAILURE

    return EXIT_SUCCESS


def validate_traces(args, traces):
    restore_traces(args, traces)
    run_autoninja(args)

    additional_args = ['--validation']
    additional_env = {
        'ANGLE_FEATURE_OVERRIDES_ENABLED': 'allocateNonZeroMemory:forceInitShaderVariables'
    }

    failures = []

    for trace in fnmatch.filter(traces, args.traces):
        json_data = load_trace_json(trace)
        num_frames = get_num_frames(json_data)
        max_steps = min(args.limit, num_frames) if args.limit else num_frames
        try:
            run_test_suite(args, trace, max_steps, additional_args, additional_env)
        except subprocess.CalledProcessError:
            logging.error('There was a failure running "%s".' % trace)
            failures += [trace]

    if failures:
        print('The following traces failed to validate:\n')
        print('\n'.join(['  ' + trace for trace in failures]))
        return EXIT_FAILURE

    return EXIT_SUCCESS


def get_min_reqs(args, traces):
    run_autoninja(args)

    env = {}
    default_args = ["--no-warmup"]

    skipped_traces = []

    for trace in fnmatch.filter(traces, args.traces):
        print(f"Finding requirements for {trace}")
        extensions = []
        json_data = load_trace_json(trace)
        num_frames = get_num_frames(json_data)
        max_steps = min(args.limit, num_frames) if args.limit else num_frames

        # exts: a list of extensions to use with run_test_suite. If empty,
        #       then run_test_suite runs with all extensions enabled by default.
        def run_test_suite_with_exts(exts):
            additional_args = default_args.copy()
            for ext in exts:
                additional_args += ['--request-ext', ext]

            try:
                run_test_suite(args, trace, max_steps, additional_args, env)
            except subprocess.CalledProcessError as error:
                return False
            return True

        original_gles_version = get_gles_version(json_data)
        original_extensions = [] if 'RequiredExtensions' not in json_data else json_data[
            'RequiredExtensions']

        def restore_trace():
            json_data['RequiredExtensions'] = original_extensions
            set_gles_version(json_data, original_gles_version)
            save_trace_json(trace, json_data)

        try:
            # Use the highest GLES version we have and empty the required
            # extensions so that previous data doesn't affect the current
            # run.
            set_gles_version(json_data, (3, 2))
            json_data['RequiredExtensions'] = []
            save_trace_json(trace, json_data)
            if not run_test_suite_with_exts([]):
                skipped_traces.append(
                    (trace, "Fails to run in default configuration on this machine"))
                restore_trace()
                continue

            # Find minimum GLES version. Run from version 3.1 backwards. We already
            # know 3.2 works because we just successfully ran with it, and more
            # traces require a later GLES version. So it should be more efficient
            # overall.
            gles_versions = [(1, 0), (1, 1), (2, 0), (3, 0), (3, 1), (3, 2)]
            min_version = None
            for idx in range(len(gles_versions) - 2, -1, -1):
                min_version = gles_versions[idx]
                set_gles_version(json_data, min_version)
                save_trace_json(trace, json_data)
                try:
                    run_test_suite(args, trace, max_steps, default_args, env)
                except subprocess.CalledProcessError as error:
                    min_version = gles_versions[idx + 1]
                    break
            set_gles_version(json_data, min_version)
            save_trace_json(trace, json_data)

            # Get the list of requestable extensions for the GLES version.
            try:
                # Get the list of requestable extensions
                with tempfile.NamedTemporaryFile() as tmp:
                    # Some operating systems will not allow a file to be open for writing
                    # by multiple processes. So close the temp file we just made before
                    # running the test suite.
                    tmp.close()
                    additional_args = ["--print-extensions-to-file", tmp.name]
                    run_test_suite(args, trace, max_steps, additional_args, env)
                    with open(tmp.name) as f:
                        for line in f:
                            extensions.append(line.strip())
            except Exception:
                skipped_traces.append(
                    (trace, "Failed to read extension list, likely that test is skipped"))
                restore_trace()
                continue

            if not run_test_suite_with_exts(extensions):
                skipped_traces.append((trace, "Requesting all extensions results in test failure"))
                restore_trace()
                continue

            # Reset RequiredExtensions so it doesn't interfere with our search
            json_data['RequiredExtensions'] = []
            save_trace_json(trace, json_data)

            # Use a divide and conquer strategy to find the required extensions.
            # Max depth is log(N) where N is the number of extensions. Expected
            # runtime is p*log(N), where p is the number of required extensions.
            # p*log(N)
            # others: A list that contains one or more required extensions,
            #         but is not actively being searched
            # exts: The list of extensions actively being searched
            def recurse_run(others, exts, depth=0):
                if len(exts) == 1:
                    return exts
                middle = int(len(exts) / 2)
                left_partition = exts[:middle]
                right_partition = exts[middle:]
                left_passed = run_test_suite_with_exts(others + left_partition)

                if depth > 0 and left_passed:
                    # We know right_passed must be False because one stack up
                    # run_test_suite(exts) returned False.
                    return recurse_run(others, left_partition)

                right_passed = run_test_suite_with_exts(others + right_partition)
                if left_passed and right_passed:
                    # Neither left nor right contain necessary extensions
                    return []
                elif left_passed:
                    # Only left contains necessary extensions
                    return recurse_run(others, left_partition, depth + 1)
                elif right_passed:
                    # Only right contains necessary extensions
                    return recurse_run(others, right_partition, depth + 1)
                else:
                    # Both left and right contain necessary extensions
                    left_reqs = recurse_run(others + right_partition, left_partition, depth + 1)
                    right_reqs = recurse_run(others + left_reqs, right_partition, depth + 1)
                    return left_reqs + right_reqs

            recurse_reqs = recurse_run([], extensions, 0)

            json_data['RequiredExtensions'] = recurse_reqs
            save_trace_json(trace, json_data)
        except BaseException as e:
            restore_trace()
            raise e

    if skipped_traces:
        print("Finished get_min_reqs, skipped traces:")
        for trace, reason in skipped_traces:
            print(f"\t{trace}: {reason}")
    else:
        print("Finished get_min_reqs for all traces specified")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-l', '--log', help='Logging level.', default=DEFAULT_LOG_LEVEL)
    parser.add_argument(
        '--test-suite',
        help='Test Suite. Default is %s' % DEFAULT_TEST_SUITE,
        default=DEFAULT_TEST_SUITE)
    parser.add_argument(
        '--no-swiftshader',
        help='Trace against native Vulkan.',
        action='store_true',
        default=False)
    parser.add_argument(
        '-v',
        '--verbose',
        help='Have subprocess output to stdout/err',
        action='store_true',
        default=False)

    subparsers = parser.add_subparsers(dest='command', required=True, help='Command to run.')

    backup_parser = subparsers.add_parser(
        'backup', help='Copies trace contents into a saved folder.')
    backup_parser.add_argument(
        'traces', help='Traces to back up. Supports fnmatch expressions.', default='*')
    backup_parser.add_argument(
        '-o',
        '--out-path',
        '--backup-path',
        help='Destination folder. Default is "%s".' % DEFAULT_BACKUP_FOLDER,
        default=DEFAULT_BACKUP_FOLDER)

    restore_parser = subparsers.add_parser(
        'restore', help='Copies traces from a saved folder to the trace folder.')
    restore_parser.add_argument(
        '-o',
        '--out-path',
        '--backup-path',
        help='Path the traces were saved. Default is "%s".' % DEFAULT_BACKUP_FOLDER,
        default=DEFAULT_BACKUP_FOLDER)
    restore_parser.add_argument(
        'traces', help='Traces to restore. Supports fnmatch expressions.', default='*')

    upgrade_parser = subparsers.add_parser(
        'upgrade', help='Re-trace existing traces, upgrading the format.')
    upgrade_parser.add_argument('gn_path', help='GN build path')
    upgrade_parser.add_argument('out_path', help='Output directory')
    upgrade_parser.add_argument(
        '-f', '--traces', '--filter', help='Trace filter. Defaults to all.', default='*')
    upgrade_parser.add_argument(
        '-n',
        '--no-overwrite',
        help='Skip traces which already exist in the out directory.',
        action='store_true')
    upgrade_parser.add_argument(
        '--validation', help='Enable state serialization validation calls.', action='store_true')
    upgrade_parser.add_argument(
        '--validation-expr',
        help='Validation expression, used to add more validation checkpoints.')
    upgrade_parser.add_argument(
        '--limit',
        '--frame-limit',
        type=int,
        help='Limits the number of captured frames to produce a shorter trace than the original.')
    upgrade_parser.add_argument(
        '--trim', action='store_true', help='Enables trace trimming. Breaks replay validation.')
    upgrade_parser.add_argument(
        '--no-trim', action='store_true', help='Disables trace trimming. Useful for validation.')
    upgrade_parser.set_defaults(trim=True)

    validate_parser = subparsers.add_parser(
        'validate', help='Runs the an updated test suite with validation enabled.')
    validate_parser.add_argument('gn_path', help='GN build path')
    validate_parser.add_argument('out_path', help='Path to the upgraded trace folder.')
    validate_parser.add_argument(
        'traces', help='Traces to validate. Supports fnmatch expressions.', default='*')
    validate_parser.add_argument(
        '--limit', '--frame-limit', type=int, help='Limits the number of tested frames.')

    get_min_reqs_parser = subparsers.add_parser(
        'get_min_reqs',
        help='Finds the minimum required extensions for a trace to successfully run.')
    get_min_reqs_parser.add_argument('gn_path', help='GN build path')
    get_min_reqs_parser.add_argument(
        '--traces',
        help='Traces to get minimum requirements for. Supports fnmatch expressions.',
        default='*')
    get_min_reqs_parser.add_argument(
        '--limit', '--frame-limit', type=int, help='Limits the number of tested frames.')

    args, extra_flags = parser.parse_known_args()

    logging.basicConfig(level=args.log.upper())

    # Load trace names
    with open(os.path.join(get_script_dir(), DEFAULT_TEST_JSON)) as f:
        traces = json.loads(f.read())

    traces = [trace.split(' ')[0] for trace in traces['traces']]

    if args.command == 'backup':
        return backup_traces(args, traces)
    elif args.command == 'restore':
        return restore_traces(args, traces)
    elif args.command == 'upgrade':
        return upgrade_traces(args, traces)
    elif args.command == 'validate':
        return validate_traces(args, traces)
    elif args.command == 'get_min_reqs':
        return get_min_reqs(args, traces)
    else:
        logging.fatal('Unknown command: %s' % args.command)
        return EXIT_FAILURE


if __name__ == '__main__':
    sys.exit(main())
