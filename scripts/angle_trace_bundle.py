#!/usr/bin/python3
#
# Copyright 2023 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# angle_trace_bundle.py:
#   Makes a zip bundle allowing to run angle traces, similarly to mb.py but
#    - trims most of the dependencies
#    - includes list_traces.sh and run_traces.sh (see --trace-names)
#    - lib.unstripped only included if --include-unstripped-libs
#    - does not depend on vpython
#    - just adds files to the zip instead of "isolate remap" with a temp dir
#
#  Example usage:
#    % gn args out/Android  # angle_restricted_traces=["agent_a", "among_us"]
#    (note: explicit build isn't necessary as it is invoked by mb isolate this script runs)
#    % scripts/angle_trace_bundle.py out/Android bundle.zip --trace-names=agent_a,among_us
#
#    (transfer the zip elsewhere)
#    % unzip bundle.zip -d bundle
#    % bundle/list_traces.sh
#    % bundle/run_traces.sh  # runs all bundled traces

import argparse
import json
import os
import subprocess
import sys
import zipfile

# {gn_dir}/angle_trace_tests has vpython in wrapper shebangs, call our runner directly
RUN_TESTS_TEMPLATE = r'''#!/bin/bash
cd "$(dirname "$0")"
python3 src/tests/angle_android_test_runner.py gtest --suite=angle_trace_tests --output-directory={gn_dir} "$@"
'''

LIST_TRACES_TEMPLATE = r'''#!/bin/bash
cd "$(dirname "$0")"
./_run_tests.sh --list-tests --filter='{filter}'
'''

RUN_TRACES_TEMPLATE = r'''#!/bin/bash
cd "$(dirname "$0")"
./_run_tests.sh --filter='{filter}' --verbose --fixed-test-time-with-warmup 10
'''


def main():
    parser = argparse.ArgumentParser(allow_abbrev=False)
    parser.add_argument('gn_dir', help='path to GN. (e.g. out/Android)')
    parser.add_argument('output_zip_file', help='output zip file')
    parser.add_argument(
        '--include-unstripped-libs', action='store_true', help='include lib.unstripped')
    parser.add_argument(
        '--trace-names',
        help='comma-separated traces that are included in the bundle',
        required=True)
    args, _ = parser.parse_known_args()

    gn_dir = os.path.join(os.path.normpath(args.gn_dir), '')
    assert os.path.sep == '/' and gn_dir.endswith('/')
    assert gn_dir[0] not in ('.', '/')  # expecting relative to angle root

    with open('src/tests/restricted_traces/restricted_traces.json') as f:
        all_traces = [t.split(' ')[0] for t in json.load(f)['traces']]

    traces = args.trace_names.split(',')
    for trace in traces:
        assert trace in all_traces, '"%s" not in restricted_traces.json' % trace
    gtest_filter = ':'.join('TraceTest.' + trace for trace in traces)

    subprocess.check_call([
        'python3', 'tools/mb/mb.py', 'isolate', gn_dir, 'angle_trace_perf_tests', '-i',
        'infra/specs/gn_isolate_map.pyl'
    ])

    with open(os.path.join(args.gn_dir, 'angle_trace_perf_tests.isolate')) as f:
        isolate_file_paths = json.load(f)['variables']['files']

    # Check that angle_restricted_traces in gn args includes all bundled traces
    isolate_filenames = set(os.path.basename(fn) for fn in isolate_file_paths)
    for trace in traces:
        assert trace + '.json' in isolate_filenames, '"%s" missing from build' % trace

    skipped_prefixes = [
        'build/',
        'src/tests/run_perf_tests.py',  # won't work as it depends on catapult
        'third_party/catapult/',
        'third_party/colorama/',
        'third_party/jdk/',
        'third_party/jinja2/',
        'third_party/logdog/',
        'third_party/r8/',
        'third_party/requests/',
        os.path.join(gn_dir, 'lib.java/'),
        os.path.join(gn_dir, 'obj/'),
    ]

    if not args.include_unstripped_libs:
        skipped_prefixes.append(os.path.join(gn_dir, 'lib.unstripped/'))

    with zipfile.ZipFile(args.output_zip_file, 'w', zipfile.ZIP_DEFLATED, allowZip64=True) as fzip:
        for fn in isolate_file_paths:
            path = os.path.normpath(os.path.join(gn_dir, fn))
            if any(path.startswith(p) for p in skipped_prefixes):
                continue

            fzip.write(path)

        def addScript(path_in_zip, contents):
            # Creates a script directly inside the zip file
            info = zipfile.ZipInfo(path_in_zip)
            info.external_attr = 0o755 << 16  # unnecessarily obscure way to chmod 755...
            fzip.writestr(info, contents)

        addScript('_run_tests.sh', RUN_TESTS_TEMPLATE.format(gn_dir=gn_dir))
        addScript('list_traces.sh',
                  LIST_TRACES_TEMPLATE.format(gn_dir=gn_dir, filter=gtest_filter))
        addScript('run_traces.sh', RUN_TRACES_TEMPLATE.format(gn_dir=gn_dir, filter=gtest_filter))

    return 0


if __name__ == '__main__':
    sys.exit(main())
