#!/usr/bin/env python3
# Copyright 2019 The ANGLE project authors. All Rights Reserved.
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


def executeCommandWithOutput(cmd, envVars):
    process = subprocess.Popen(
        cmd, env=envVars, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    while True:
        output = process.stdout.readline()
        print(output.strip())
        # Do something else
        return_code = process.poll()
        if return_code is not None:
            # Process has finished, read rest of the output
            for output in process.stdout.readlines():
                print(output.strip())
            break
            for output in process.stderr.readlines():
                print(output.strip())
            break
            if return_code != 0:
                sys.exit()


def cleanupDevice(envVars):
    # Cleanup the screenshot directory on the device
    process = subprocess.run(['adb', 'shell', 'rm', '-rf', ANDROID_SCREENSHOT_DIRECTORY],
                             env=envVars,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             universal_newlines=True)
    if process.returncode != 0:
        print("Failed cleanup: '" + ANDROID_SCREENSHOT_DIRECTORY + "'")
        print(process.stdout)
        print(process.stderr)
        sys.exit()


def generateScreenshots(args, serial):
    print("Generating screenshots...")
    gtest_filter = 'TracePerfTest.Run/*' + args.trace + '*'
    envVars = os.environ.copy()
    if serial is not None:
        envVars['ANDROID_SERIAL'] = serial

    # Cleanup the screenshot directory on the device to get rid of any old screenshots
    cleanupDevice(envVars)

    # Create the screenshot directory on the device
    process = subprocess.run(['adb', 'shell', 'mkdir', '-p', ANDROID_SCREENSHOT_DIRECTORY],
                             env=envVars,
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
    executeCommandWithOutput(cmd, envVars)

    # Pull the screenshots from the device
    cmd = ['adb', 'pull', ANDROID_SCREENSHOT_DIRECTORY, args.output_directory]
    executeCommandWithOutput(cmd, envVars)

    # Cleanup the screenshot directory on the device
    cleanupDevice(envVars)


def calculateRmse(args, chipset=""):
    nativePng = args.output_directory + os.path.sep + DESKTOP_SCREENSHOT_DIRECTORY + os.path.sep + 'angle_native_' + args.trace + '.png'
    vulkanPng = args.output_directory + os.path.sep + DESKTOP_SCREENSHOT_DIRECTORY + os.path.sep + 'angle_vulkan_' + args.trace + '.png'
    resultPng = args.output_directory + os.path.sep + DESKTOP_SCREENSHOT_DIRECTORY + os.path.sep + args.trace + "_" + chipset + '_RMSE_diff.png'

    envVars = os.environ.copy()
    cmd = ['compare', '-metric', 'RMSE', vulkanPng, nativePng, resultPng]
    process = subprocess.run(
        cmd, env=envVars, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    # ImageMagick returns '1' on success, rather than '0'
    if process.returncode != 1:
        print("Failed compare screenshots: '" + nativePng + "', '" + vulkanPng + "'")
        print(process.stdout)
        print(process.stderr)
        sys.exit()

    # ImageMagick outputs everything to stderr, rather than stdout
    return process.stderr.strip(), resultPng


def generateRmseScores(args):
    # Build angle_perftests to make sure we have the freshest version.
    print("Building...")

    cmd = ['autoninja', '-C', args.output_directory, PERFTESTS_EXECUTABLE]
    executeCommandWithOutput(cmd, os.environ.copy())

    # Generate the RMSE scores
    if args.arm is None and args.qc is None:
        # If not serial number was given for ARM or QC, then assume there's only a single device
        # connected and try to run on that.
        generateScreenshots(args, None)
        rmse, diffFile = calculateRmse(args)
        print("RMSE score:")
        print("  " + rmse)
        print("RMSE diff files:")
        print("  " + diffFile)
    else:
        # Otherwise run on all specified devices
        results = {}
        diffFiles = {}
        if args.arm is not None:
            generateScreenshots(args, args.arm)
            rmse, diffFile = calculateRmse(args, 'arm')
            results['ARM'] = rmse
            diffFiles['ARM'] = diffFile
        if args.qc is not None:
            generateScreenshots(args, args.qc)
            rmse, diffFile = calculateRmse(args, 'qc')
            results['QC'] = rmse
            diffFiles['QC'] = diffFile

        print("RMSE scores:")
        for key in results:
            print("  " + key + ": " + results[key])
        print("RMSE diff files:")
        for key in results:
            print("  " + key + ": " + diffFiles[key])


def main():
    p = argparse.ArgumentParser(
        description='Calculate RMSE and diff files between native and Vulkan screenshots for a specified trace. Runs on the connected device if no serial numbers are given.'
    )
    p.add_argument('trace', help='The trace to generate the RMSE score for.')
    p.add_argument(
        'output_directory',
        help='The output directory to build "angle_perftests" in and execute it from. This is also where the screenshots/ directory is created.'
    )
    p.add_argument('--arm', help='ARM device serial number.')
    p.add_argument('--qc', help='QC device serial number.')

    args = p.parse_args()

    generateRmseScores(args)


if __name__ == '__main__':
    sys.exit(main())
