#!/usr/bin/python3
#
# Copyright 2023 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# angle_trace_bundle.py:
#   Makes a zip bundle allowing to run angle traces, similarly to mb.py but
#    - trims most of the dependencies
#    - includes list_traces.sh and run_trace.sh (see --trace-name)
#    - lib.unstripped only included if --include-unstripped-libs
#    - does not depend on vpython
#    - just adds files to the zip instead of "isolate remap" with a temp dir
#
#  Example usage:
#    % gn args out/Android  # angle_restricted_traces=["among_us"]
#    (note: explicit build isn't necessary as it is invoked by mb isolate this script runs)
#    % scripts/angle_trace_bundle.py out/Android angle_trace.zip --trace-name=among_us
#
#    (transfer the zip elsewhere)
#    % unzip angle_trace.zip -d angle_trace
#    % angle_trace/list_traces.sh
#    % angle_trace/run_trace.sh  # only included if --trace-name, runs that trace

import argparse
import json
import logging
import os
import subprocess
import sys
import zipfile
from pathlib import Path


# {gn_dir}/angle_trace_tests has vpython in wrapper shebangs, call our runner directly
RUN_TESTS_TEMPLATE = r'''#!/bin/bash
cd "$(dirname "$0")"
python3 src/tests/angle_android_test_runner.py gtest --suite=angle_trace_tests --output-directory={gn_dir} "$@"
'''

LIST_TRACES_TEMPLATE = r'''#!/bin/bash
cd "$(dirname "$0")"
./_run_tests.sh --list-tests
'''

RUN_TRACE_TEMPLATE = r'''#!/bin/bash
cd "$(dirname "$0")"
./_run_tests.sh --filter='TraceTest.{trace_name}' --verbose --fixed-test-time-with-warmup 10
'''

GENERATE_SCREENSHOTS_TEMPLATE = r'''#!/bin/bash
cd "$(dirname "$0")"
adb shell mkdir -p /sdcard/angle_screenshots
./_run_tests.sh --filter='TraceTest*' --verbose --run-to-key-frame --screenshot-dir /sdcard/angle_screenshots
adb pull /sdcard/angle_screenshots
'''

COMPARE_GOLDENS_TEMPLATE = r'''#!/bin/bash

# Directory Paths
goldens_dir="angle_goldens"
screenshots_dir="angle_screenshots"
diff_dir="screenshot_diffs"

# Create diffs directory if it doesn't exist
mkdir -p "$diff_dir"

# Check for ImageMagick
if ! command -v compare &> /dev/null; then
    echo "ImageMagick's 'compare' tool not found. Please install it:"
    echo "  sudo apt-get install imagemagick"
    exit 1
fi

# Error Flag
error_flag=0

# Signal handler for SIGINT (Ctrl+C)
trap "echo 'Exiting...'; exit 1" SIGINT

# Iterate through PNG images in goldens_dir
for golden_img in "$goldens_dir"/*.png; do
    filename=$(basename "$golden_img")
    screenshot_img="$screenshots_dir/$filename"

    # Check if image exists in screenshots_dir
    if [[ ! -f "$screenshot_img" ]]; then
        echo "Error: Image $filename not found in $screenshots_dir"
        error_flag=1
        continue
    fi

    # Compare images
    echo "Comparing: $filename"
    compare -metric AE -fuzz 5% "$golden_img" "$screenshot_img" "$diff_dir/diff-$filename"
    result=$?
    echo

    if [[ $result -ne 0 ]]; then
        echo "Images differ: $filename (AE: $result)"
        error_flag=1
    fi
done

# Iterate through PNG images in screenshots_dir to check for extras
for screenshot_img in "$screenshots_dir"/*.png; do
    filename=$(basename "$screenshot_img")
    golden_img="$goldens_dir/$filename"

    if [[ ! -f "$golden_img" ]]; then
        echo "Error: Image $filename found in $screenshots_dir but not in $goldens_dir"
        error_flag=1
    fi
done

if [[ $error_flag -eq 1 ]]; then
    echo "Errors were found during image comparison."
    exit 1
else
    echo "All images compared successfully!"
    exit 0
fi
'''

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('gn_dir', help='path to GN. (e.g. out/Android)')
    parser.add_argument('output_zip_file', help='output zip file')
    parser.add_argument(
        '--include-unstripped-libs', action='store_true', help='include lib.unstripped')
    parser.add_argument('--trace-name', help='trace to run from run_script.sh')
    parser.add_argument('-l', '--log', help='Logging level.', default='info')
    parser.add_argument('--goldens', help='directory containing golden images')
    args, _ = parser.parse_known_args()

    logging.basicConfig(level=args.log.upper())

    gn_dir = os.path.join(os.path.normpath(args.gn_dir), '')
    assert os.path.sep == '/' and gn_dir.endswith('/')
    assert gn_dir[0] not in ('.', '/')  # expecting relative to angle root

    subprocess.check_call([
        'python3', 'tools/mb/mb.py', 'isolate', gn_dir, 'angle_trace_perf_tests', '-i',
        'infra/specs/gn_isolate_map.pyl'
    ])

    with open(os.path.join(args.gn_dir, 'angle_trace_perf_tests.isolate')) as f:
        isolate_file_paths = json.load(f)['variables']['files']

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
        addScript('list_traces.sh', LIST_TRACES_TEMPLATE.format(gn_dir=gn_dir))

        if args.trace_name:
            addScript('run_trace.sh',
                      RUN_TRACE_TEMPLATE.format(gn_dir=gn_dir, trace_name=args.trace_name))

        if args.goldens:
            logging.info('Checking args.goldens %s' % args.goldens)

            # Include script to run traces (all or subset) and compare with goldens
            addScript('generate_screenshots.sh',
                      GENERATE_SCREENSHOTS_TEMPLATE.format(gn_dir=gn_dir))
            addScript('compare_goldens.sh', COMPARE_GOLDENS_TEMPLATE.format(gn_dir=gn_dir))

            # Create an empty directory to hold goldens
            goldens_dir = "angle_goldens/"
            fzip.writestr(goldens_dir, "")

            # Include PNG files from goldens dir
            directory_path = Path(args.goldens)
            for file_path in directory_path.iterdir():
                if file_path.suffix != ".png":
                    logging.info('Skipping %s' % file_path.name)
                    continue

                logging.info('Adding %s to bundle' % file_path.name)
                fzip.write(file_path, arcname=goldens_dir + file_path.name)

    return 0


if __name__ == '__main__':
    sys.exit(main())
