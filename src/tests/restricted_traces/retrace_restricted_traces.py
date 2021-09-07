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
import re
import subprocess
import sys

from gen_restricted_traces import read_json as read_json

DEFAULT_TEST_SUITE = 'angle_perftests'
DEFAULT_TEST_JSON = 'restricted_traces.json'
DEFAULT_LOG_LEVEL = 'info'


def get_context(trace, trace_path):
    """Returns the trace context number."""
    json_path = '%s/%s.json' % (trace_path, trace)
    json_data = read_json(json_path)
    return str(json_data['WindowSurfaceContextID'])


def src_trace_path(trace):
    script_dir = os.path.dirname(sys.argv[0])
    return os.path.join(script_dir, trace)


def context_header(trace, trace_path):
    context_id = get_context(trace, trace_path)
    header = '%s_capture_context%s.h' % (trace, context_id)
    return os.path.join(trace_path, header)


def path_contains_header(path):
    for file in os.listdir(path):
        if fnmatch.fnmatch(file, '*.h'):
            return True
    return False


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('gn_path', help='GN build path')
    parser.add_argument('out_path', help='Output directory')
    parser.add_argument('-f', '--filter', help='Trace filter. Defaults to all.', default='*')
    parser.add_argument('-l', '--log', help='Logging level.', default=DEFAULT_LOG_LEVEL)
    parser.add_argument(
        '--no-swiftshader',
        help='Trace against native Vulkan.',
        action='store_true',
        default=False)
    parser.add_argument(
        '-n',
        '--no-overwrite',
        help='Skip traces which already exist in the out directory.',
        action='store_true')
    parser.add_argument(
        '--validation', help='Enable state serialization validation calls.', action='store_true')
    parser.add_argument(
        '--validation-expr',
        help='Validation expression, used to add more validation checkpoints.')
    parser.add_argument(
        '--limit',
        '--frame-limit',
        type=int,
        help='Limits the number of captured frames to produce a shorter trace than the original.')
    args, extra_flags = parser.parse_known_args()

    logging.basicConfig(level=args.log.upper())

    script_dir = os.path.dirname(sys.argv[0])

    # Load trace names
    traces = read_json(os.path.join(script_dir, DEFAULT_TEST_JSON))
    traces = [trace.split(' ')[0] for trace in traces['traces']]

    binary = os.path.join(args.gn_path, DEFAULT_TEST_SUITE)
    if os.name == 'nt':
        binary += '.exe'

    failures = []

    for trace in fnmatch.filter(traces, args.filter):
        logging.debug('Tracing %s' % trace)

        trace_path = os.path.abspath(os.path.join(args.out_path, trace))
        if not os.path.isdir(trace_path):
            os.makedirs(trace_path)
        elif args.no_overwrite and path_contains_header(trace_path):
            logging.info('Skipping "%s" because the out folder already exists' % trace)
            continue

        json_path = '%s/%s.json' % (trace, trace)
        json_data = read_json(json_path)
        metadata = json_data['TraceMetadata']
        num_frames = metadata['FrameEnd'] - metadata['FrameStart'] + 1

        logging.debug('Read metadata: %s' % str(metadata))

        max_steps = min(args.limit, num_frames) if args.limit else num_frames

        # We start tracing from frame 2. --retrace-mode issues a Swap() after Setup() so we can
        # accurately re-trace the MEC.
        additional_env = {
            'ANGLE_CAPTURE_LABEL': trace,
            'ANGLE_CAPTURE_OUT_DIR': trace_path,
            'ANGLE_CAPTURE_FRAME_START': '2',
            'ANGLE_CAPTURE_FRAME_END': str(num_frames + 1),
        }
        if args.validation:
            additional_env['ANGLE_CAPTURE_VALIDATION'] = '1'
            # Also turn on shader output init to ensure we have no undefined values.
            # This feature is also enabled in replay when using --validation.
            additional_env['ANGLE_FEATURE_OVERRIDES_ENABLED'] = 'forceInitShaderOutputVariables'
        if args.validation_expr:
            additional_env['ANGLE_CAPTURE_VALIDATION_EXPR'] = args.validation_expr

        env = {**os.environ.copy(), **additional_env}

        renderer = 'vulkan' if args.no_swiftshader else 'vulkan_swiftshader'

        trace_filter = '--gtest_filter=TracePerfTest.Run/%s_%s' % (renderer, trace)
        run_args = [
            binary,
            trace_filter,
            '--retrace-mode',
            '--max-steps-performed',
            str(max_steps),
            '--enable-all-trace-tests',
        ]

        print('Capturing "%s" (%d frames)...' % (trace, num_frames))
        logging.debug('Running "%s" with environment: %s' %
                      (' '.join(run_args), str(additional_env)))
        try:
            subprocess.check_call(run_args, env=env)

            header_file = context_header(trace, trace_path)

            if not os.path.exists(header_file):
                logging.error('There was a problem tracing "%s", could not find header file: %s' %
                              (trace, header_file))
                failures += [trace]
        except:
            logging.exception('There was an exception running "%s":' % trace)
            failures += [trace]

    if failures:
        print('The following traces failed to re-trace:\n')
        print('\n'.join(['  ' + trace for trace in failures]))
        return 1

    return 0


if __name__ == '__main__':
    sys.exit(main())
