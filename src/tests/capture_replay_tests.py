#
# Copyright 2020 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
"""
Script testing capture_replay with angle_end2end_tests
"""

# Automation script will:
# 1. Build all tests in angle_end2end with frame capture enabled
# 2. Run each test with frame capture
# 3. Build CaptureReplayTest with cpp trace files
# 4. Run CaptureReplayTest
# 5. Output the number of test successes and failures. A test succeeds if no error occurs during
# its capture and replay, and the GL states at the end of two runs match. Any unexpected failure
# will return non-zero exit code

# Run this script with Python to test capture replay on angle_end2end tests
# python path/to/capture_replay_tests.py
# Command line arguments:
# --build_dir: specifies build directory relative to angle folder.
# Default is out/CaptureReplayTestsDebug
# --verbose: off by default
# --use_goma: uses goma for compiling and linking test. Off by default
# --gtest_filter: same as gtest_filter of Google's test framework. Default is *

import argparse
import distutils.util
import os
import subprocess

from sys import platform


def RunGnGen(build_dir, arguments, is_log_showed=False):
    command = 'gn gen --args="'
    is_first_argument = True
    for argument in arguments:
        if is_first_argument:
            is_first_argument = False
        else:
            command += ' '
        command += argument[0]
        command += '='
        command += argument[1]
    command += '" '
    command += build_dir
    if is_log_showed:
        subprocess.call(command, shell=True)
    else:
        subprocess.check_output(command, shell=True)


def RunAutoninja(build_dir, target, is_log_showed=False):
    command = "autoninja "
    command += target
    command += " -C "
    command += build_dir
    if is_log_showed:
        subprocess.call(command, shell=True)
    else:
        subprocess.check_output(command, shell=True)


# return a list of tests and their params in the form
# [(test1, params1), (test2, params2),...]
def GetTestNamesAndParams(test_exec_with_ext_path, filter="*"):
    output = subprocess.check_output(
        '"' + test_exec_with_ext_path + '" --gtest_list_tests --gtest_filter=' + filter,
        shell=True,
        stderr=subprocess.PIPE).splitlines()

    tests = []
    last_testcase_name = ""
    test_name_splitter = "# GetParam() ="
    for line in output:
        if test_name_splitter in line:
            # must be a test name line
            test_name_and_params = line.split(test_name_splitter)
            tests.append((last_testcase_name + test_name_and_params[0].strip(), \
                test_name_and_params[1].strip()))
        else:
            # gtest_list_tests returns the test in this format
            # test case
            #    test name1
            #    test name2
            # Need to remember the last test case name to append to the test name
            last_testcase_name = line
    return tests


class Test():

    def __init__(self, full_test_name, params, verbose, use_goma):
        self.full_test_name = full_test_name
        self.params = params
        self.verbose = verbose
        self.use_goma = use_goma

    def __str__(self):
        return self.full_test_name + " Params: " + self.params

    def Run(self, test_exe_path):
        try:
            output = subprocess.check_output(
                '"' + test_exe_path + '" --gtest_filter=' + self.full_test_name,
                shell=True,
                stderr=subprocess.PIPE)
            if self.verbose:
                print("Ran " + self.full_test_name + " with capture")
            return (0, output)
        except subprocess.CalledProcessError as e:
            return (e.returncode, e.output)

    def BuildReplay(self, build_dir, replay_exec):
        RunGnGen(build_dir, [("use_goma", self.use_goma),
                             ("angle_with_capture_by_default", "true"),
                             ("angle_build_capture_replay_test", "true")])
        RunAutoninja(build_dir, replay_exec)
        if self.verbose:
            print("Built replay of " + self.full_test_name)

    def RunReplay(self, build_dir, replay_exec_with_ext):
        try:
            output = subprocess.check_output(
                '"' + build_dir + '/' + replay_exec_with_ext + '"',
                shell=True,
                stderr=subprocess.PIPE)
            if self.verbose:
                print("Ran replay of " + self.full_test_name)
            return (0, output)
        except subprocess.CalledProcessError as e:
            return (e.returncode, e.output)


def SetCWDToAngleFolder():
    angle_folder = "angle"
    cwd = os.path.dirname(os.path.abspath(__file__))
    cwd = cwd.split(angle_folder)[0] + angle_folder
    os.chdir(cwd)
    return cwd


def main(build_dir, verbose, use_goma, gtest_filter):
    cwd = SetCWDToAngleFolder()
    capture_out_dir = "src/tests/capture_replay_tests"  # relative to CWD, angle folder
    environment_vars = [("ANGLE_CAPTURE_FRAME_END", "0"),
                        ("ANGLE_CAPTURE_OUT_DIR", capture_out_dir)]
    test_exec = "angle_end2end_tests"
    replay_exec = "capture_replay_test"
    if platform == "win32":
        test_exec_with_ext = test_exec + ".exe"
        replay_exec_with_ext = replay_exec + ".exe"
    else:
        test_exec_with_ext = test_exec
        replay_exec_with_ext = replay_exec
    # generate gn files
    RunGnGen(build_dir, [("use_goma", use_goma), ("angle_with_capture_by_default", "true")], True)
    # build angle_end2end
    RunAutoninja(build_dir, test_exec, True)
    # get a list of tests
    test_names_and_params = GetTestNamesAndParams(build_dir + '/' + test_exec_with_ext,
                                                  gtest_filter)
    all_tests = []
    for test_name_and_params in test_names_and_params:
        all_tests.append(Test(test_name_and_params[0], test_name_and_params[1], verbose, use_goma))

    for environment_var in environment_vars:
        os.environ[environment_var[0]] = environment_var[1]

    replay_file_created_keyword = "Saved "
    for test in all_tests:
        print("*" * 30)
        os.environ["ANGLE_CAPTURE_ENABLED"] = "1"
        run_output = test.Run(build_dir + "/" + test_exec_with_ext)
        if run_output[0] == 0 and replay_file_created_keyword in run_output[1]:
            os.environ["ANGLE_CAPTURE_ENABLED"] = "0"
            test.BuildReplay(build_dir, replay_exec)
            replay_output = test.RunReplay(build_dir, replay_exec_with_ext)
            if replay_output[0] != 0:
                print("Error: " + replay_output[1])
            else:
                print("Success!")
        else:
            print("Skipping replay since capture doesn't produce any file or has crashed")

    for environment_var in environment_vars:
        del os.environ[environment_var[0]]


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    DEFAULT_BUILD_DIR = "out/CaptureReplayTestsDebug"  # relative to cwd: angle folder
    parser.add_argument('--build_dir', default=DEFAULT_BUILD_DIR)
    parser.add_argument('--verbose', default="False")
    parser.add_argument('--use_goma', default="false")
    parser.add_argument('--gtest_filter', default="*")
    args = parser.parse_args()
    main(args.build_dir, distutils.util.strtobool(args.verbose), args.use_goma, args.gtest_filter)
