#!/usr/bin/env python3
# Copyright 2021 The ANGLE project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.
"""Script to automatically generate RMSE scores from app traces."""

import argparse
import os
import subprocess
import sys

PERFTESTS_EXECUTABLE = 'angle_perftests'
ANDROID_SCREENSHOT_DIRECTORY = '/sdcard/chromium_tests_root/third_party/angle/screenshots'
DESKTOP_SCREENSHOT_DIRECTORY = 'screenshots'


def execute_command_with_output(cmd, env_vars):
    process = subprocess.Popen(
        cmd, env=env_vars, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    while True:
        output = process.stdout.readline()
        print(output.strip())
        # Do something else
        return_code = process.poll()
        if return_code is not None:
            # Process has finished, read rest of the output
            for output in process.stdout.readlines():
                print(output.strip())
            for output in process.stderr.readlines():
                print(output.strip())
            if return_code != 0:
                sys.exit()
            break


def cleanup_device(env_vars):
    # Cleanup the screenshot directory on the device
    process = subprocess.run(['adb', 'shell', 'rm', '-rf', ANDROID_SCREENSHOT_DIRECTORY],
                             env=env_vars,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             universal_newlines=True)
    if process.returncode != 0:
        print("Failed cleanup: '" + ANDROID_SCREENSHOT_DIRECTORY + "'")
        print(process.stdout)
        print(process.stderr)
        sys.exit()


def generate_screenshots(args, serial):
    print("Generating screenshots...")
    gtest_filter = 'TracePerfTest.Run/*' + args.trace + '*'
    env_vars = os.environ.copy()
    if serial is not None:
        env_vars['ANDROID_SERIAL'] = serial

    # Cleanup the screenshot directory on the device to get rid of any old screenshots
    cleanup_device(env_vars)

    # Create the screenshot directory on the device
    process = subprocess.run(['adb', 'shell', 'mkdir', '-p', ANDROID_SCREENSHOT_DIRECTORY],
                             env=env_vars,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             universal_newlines=True)
    if process.returncode != 0:
        print("Failed create directory: '" + ANDROID_SCREENSHOT_DIRECTORY + "'")
        print(process.stdout)
        print(process.stderr)
        sys.exit()

    # Run the perftests to generate the native/vulkan screenshots
    cmd = [
        args.output_directory + os.path.sep + PERFTESTS_EXECUTABLE,
        '--gtest_filter=' + gtest_filter, '--verbose', '--local-output', '--screenshot-dir',
        ANDROID_SCREENSHOT_DIRECTORY
    ]
    execute_command_with_output(cmd, env_vars)

    # Pull the screenshots from the device
    cmd = ['adb', 'pull', ANDROID_SCREENSHOT_DIRECTORY, args.output_directory]
    execute_command_with_output(cmd, env_vars)

    # Cleanup the screenshot directory on the device
    cleanup_device(env_vars)


def calculate_rmse(args, serial=""):
    native_png = (
        args.output_directory + os.path.sep + DESKTOP_SCREENSHOT_DIRECTORY + os.path.sep +
        'angle_native_' + args.trace + '.png')
    vulkan_png = (
        args.output_directory + os.path.sep + DESKTOP_SCREENSHOT_DIRECTORY + os.path.sep +
        'angle_vulkan_' + args.trace + '.png')
    result_png = (
        args.output_directory + os.path.sep + DESKTOP_SCREENSHOT_DIRECTORY + os.path.sep +
        args.trace + "_" + serial + '_RMSE_diff.png')

    env_vars = os.environ.copy()
    cmd = ['compare', '-metric', 'RMSE', vulkan_png, native_png, result_png]
    process = subprocess.run(
        cmd, env=env_vars, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    # ImageMagick returns '1' on success, rather than '0'
    if process.returncode != 1:
        print("Failed compare screenshots: '" + native_png + "', '" + vulkan_png + "'")
        print(process.stdout)
        print(process.stderr)
        sys.exit()

    # ImageMagick outputs everything to stderr, rather than stdout
    return process.stderr.strip(), result_png


def calculate_fuzzed_ae(args, serial=""):
    native_png = (
        args.output_directory + os.path.sep + DESKTOP_SCREENSHOT_DIRECTORY + os.path.sep +
        'angle_native_' + args.trace + '.png')
    vulkan_png = (
        args.output_directory + os.path.sep + DESKTOP_SCREENSHOT_DIRECTORY + os.path.sep +
        'angle_vulkan_' + args.trace + '.png')

    env_vars = os.environ.copy()
    fuzz_results = {}
    for fuzz in [0, 1, 2, 5, 10]:
        result_png = (
            args.output_directory + os.path.sep + DESKTOP_SCREENSHOT_DIRECTORY + os.path.sep +
            args.trace + "_" + serial + '_AE_fuzz' + str(fuzz) + '_diff.png')
        cmd = [
            'compare', '-metric', 'AE', '-fuzz',
            str(fuzz) + "%", vulkan_png, native_png, result_png
        ]
        process = subprocess.run(
            cmd,
            env=env_vars,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True)
        # ImageMagick returns '1' on success, rather than '0'
        if process.returncode != 1:
            print("Failed compare screenshots: '" + native_png + "', '" + vulkan_png + "'")
            print(process.stdout)
            print(process.stderr)
            sys.exit()

        diff = process.stderr.strip()
        fuzz_results[fuzz] = diff
        if int(diff) == 0:
            break

    # ImageMagick outputs everything to stderr, rather than stdout
    return fuzz_results, result_png


def generate_diffs(args):
    # Build angle_perftests to make sure we have the freshest version.
    print("Building...")

    cmd = ['autoninja', '-C', args.output_directory, PERFTESTS_EXECUTABLE]
    execute_command_with_output(cmd, os.environ.copy())

    # Generate the RMSE scores and AE images
    if args.serials is None:
        # If not serial number was given for ARM or QC, then assume there's only a single device
        # connected and try to run on that.
        generate_screenshots(args, None)
        rmse, diffFile = calculate_rmse(args)
        print("RMSE score:")
        print("  " + rmse)
        print("RMSE diff files:")
        print("  " + diffFile)
        fuzzed_ae, diffFile = calculate_fuzzed_ae(args)
        print("Fuzzed AE score:")
        print("  " + fuzzed_ae)
        print("Fuzzed AE diff files:")
        print("  " + diffFile)
    else:
        # Otherwise run on all specified devices
        rmse_results = {}
        rmse_diff_files = {}
        fuzzed_ae_results = {}
        fuzzed_ae_diff_files = {}
        if args.serials is not None:
            for serial in args.serials:
                generate_screenshots(args, serial)
                rmse, diffFile = calculate_rmse(args, serial)
                rmse_results[serial] = rmse
                rmse_diff_files[serial] = diffFile
                fuzz_results, diffFile = calculate_fuzzed_ae(args, serial)
                fuzzed_ae_results[serial] = fuzz_results
                fuzzed_ae_diff_files[serial] = diffFile

        # Output list of attached devices to ease corresponding to serial numbers
        env_vars = os.environ.copy()
        cmd = ['adb', 'devices', '-l']
        process = subprocess.run(
            cmd,
            env=env_vars,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True)
        print(process.stdout)

        print("RMSE scores:")
        for serial in rmse_results:
            print("  " + serial + ": " + rmse_results[serial])
        print("RMSE diff files:")
        for serial in rmse_results:
            print("  " + serial + ": " + rmse_diff_files[serial])

        print("Fuzzed AE scores:")
        for serial in fuzzed_ae_results:
            for fuzz in sorted(fuzzed_ae_results[serial].keys()):
                print("  " + serial + ": " + str(fuzz) + "% = " + fuzzed_ae_results[serial][fuzz])
        print("Fuzzed AE diff files:")
        for serial in fuzzed_ae_results:
            print("  " + serial + ": " + fuzzed_ae_diff_files[serial])


def main():
    p = argparse.ArgumentParser(
        description='Calculate RMSE and diff files between native and Vulkan screenshots for a'
        'specified trace. Runs on the connected device if no serial numbers are given.')
    p.add_argument('trace', help='The trace to generate the RMSE score for.')
    p.add_argument(
        'output_directory',
        help='The output directory to build "angle_perftests" in and execute it from. This is also'
        'where the screenshots/ directory is created. ')
    p.add_argument('--serials', nargs='+', help='List of device serial numbers.')

    args = p.parse_args()

    generate_diffs(args)


if __name__ == '__main__':
    sys.exit(main())
