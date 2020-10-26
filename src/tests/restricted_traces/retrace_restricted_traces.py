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

DEFAULT_TEST_SUITE = 'angle_perftests'
DEFAULT_TEST_JSON = 'restricted_traces.json'


def get_num_frames(trace):

    script_dir = os.path.dirname(sys.argv[0])
    trace_path = os.path.join(script_dir, trace)

    lo = 99999999
    hi = 0

    for file in os.listdir(trace_path):
        match = re.match(r'.+_capture_context\d_frame(\d+)\.cpp', file)
        if match:
            frame = int(match.group(1))
            if frame < lo:
                lo = frame
            if frame > hi:
                hi = frame

    return hi - lo + 1


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('gn_path', help='GN build path')
    parser.add_argument('out_path', help='Output directory')
    parser.add_argument('-f', '--filter', help='Trace filter. Defaults to all.', default='*')
    args, extra_flags = parser.parse_known_args()

    script_dir = os.path.dirname(sys.argv[0])

    # Load trace names
    with open(os.path.join(script_dir, DEFAULT_TEST_JSON)) as f:
        traces = json.loads(f.read())

    traces = traces['traces']

    binary = os.path.join(args.gn_path, DEFAULT_TEST_SUITE)
    if os.name == 'nt':
        binary += '.exe'

    for trace in fnmatch.filter(traces, args.filter):
        logging.debug('Tracing %s' % trace)

        trace_path = os.path.abspath(os.path.join(args.out_path, trace))
        if not os.path.isdir(trace_path):
            os.mkdir(trace_path)

        num_frames = get_num_frames(trace)

        env = os.environ.copy()
        env['ANGLE_CAPTURE_OUT_DIR'] = trace_path
        env['ANGLE_CAPTURE_LABEL'] = trace
        env['ANGLE_CAPTURE_FRAME_START'] = str(num_frames)
        env['ANGLE_CAPTURE_FRAME_END'] = str(num_frames + num_frames - 1)

        trace_filter = '--gtest_filter=TracePerfTest.Run/vulkan_%s' % trace
        run_args = [binary, trace_filter, '--no-warmup', '--trials=1']

        print('Capturing %s (%d frames)...' % (trace, num_frames))
        subprocess.check_call(run_args, env=env)

    return 0


if __name__ == '__main__':
    sys.exit(main())
