#
# Copyright 2020 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
"""
Script testing capture_replay with angle_end2end_tests
"""

#automation script will:
#1. compile all tests in angle_end2end with framecapture enabled
#2. run each test with frame capture
#3. compile CaptureReplayTest with cpp trace files
#4. Run CaptureReplayTest
#5. Output the number of test successes and failures. A test succeeds if no error occurs during
# its capture and replay, and the GL states at the end of two runs match

# run this script with Python to test capture replay on angle_end2end tests
# python path/to/run_tests_with_framecapture.py 
# command line arguments: 
# --compile_dir: specifies compile directory relative to angle folder. default is out\Debug

import subprocess
import os
import argparse
from sys import platform

class Test():

    def __init__(self, full_test_name, params):
        self.full_test_name = full_test_name
        self.params = params

    def __str__(self):
        return self.full_test_name + " Params: " + self.params

    def Run(self, test_exe_path):
        output = subprocess.check_output(
            '"' + test_exe_path + '" --gtest_filter=' + self.full_test_name, shell=True)
        print("Ran " + self.full_test_name + " with capture")
        return output

    def CompileReplay(self, compile_dir, replay_exec):
        subprocess.check_output(
            'gn gen --args="use_goma=true angle_with_capture_by_default=true ' +
            'angle_build_capture_replay_test=true" ' +
            compile_dir,
            shell=True)
        subprocess.check_output("autoninja " + replay_exec + " -C " + compile_dir, shell=True)
        print("Compiled replay of " + self.full_test_name)

    def RunReplay(self, compile_dir, replay_exec_with_ext):
        subprocess.check_output('"' + compile_dir + '/' + replay_exec_with_ext + '"', shell=True)
        print("Played replay of " + self.full_test_name)


def SetCWDToAngleFolder():
    angle_folder = "angle"
    cwd = os.path.dirname(os.path.abspath(__file__))
    cwd = cwd.split(angle_folder)[0] + angle_folder
    os.chdir(cwd)
    return cwd


def main(COMPILE_DIR):
    CWD = SetCWDToAngleFolder()
    CAPTURE_OUT_DIR = "src/tests/capture_replay_tests"  # relative to CWD, angle folder
    environment_vars = [("ANGLE_CAPTURE_FRAME_END", "0"),
                        ("ANGLE_CAPTURE_OUT_DIR", CAPTURE_OUT_DIR)]
    TEST_EXEC = "angle_end2end_tests"
    REPLAY_EXEC = "capture_replay_test"
    if platform == "win32":
        TEST_EXEC_WITH_EXT = TEST_EXEC + ".exe"
        REPLAY_EXEC_WITH_EXT = REPLAY_EXEC + ".exe"
    else:
        TEST_EXEC_WITH_EXT = TEST_EXEC
        REPLAY_EXEC_WITH_EXT = REPLAY_EXEC
    # generate gn files
    subprocess.call(
        'gn gen --args="use_goma=true angle_with_capture_by_default=true" ' + COMPILE_DIR,
        shell=True)
    print("Generated ninja files")
    # compile angle_end2end
    subprocess.call("autoninja " + TEST_EXEC + " -C " + COMPILE_DIR, shell=True)
    print("Compiled all tests in angle_end2end")
    # get a list of tests
    output = subprocess.check_output(
        '"' + COMPILE_DIR + '/' + TEST_EXEC_WITH_EXT + '" --gtest_list_tests',
        shell=True).splitlines()

    all_tests = []
    last_testcase_name = ""
    test_name_splitter = "# GetParam() ="
    for line in output:
        if test_name_splitter in line:
            # must be a test name line
            test_name_and_params = line.split(test_name_splitter)
            all_tests.append(
                Test(last_testcase_name + test_name_and_params[0].strip(),
                     test_name_and_params[1].strip()))
        else:
            # might not be a test case name line but by the time we use it to concatenate
            # with a test name, it will have been updated correctly to a test case
            last_testcase_name = line
    for environment_var in environment_vars:
        os.environ[environment_var[0]] = environment_var[1]

    replay_file_created_keyword = "Saved "
    for test in all_tests:
        print(test)
        os.environ["ANGLE_CAPTURE_ENABLED"] = "1"
        run_output = test.Run(COMPILE_DIR + "/" + TEST_EXEC_WITH_EXT)
        if replay_file_created_keyword in run_output:
            os.environ["ANGLE_CAPTURE_ENABLED"] = "0"
            test.CompileReplay(COMPILE_DIR, REPLAY_EXEC)
            test.RunReplay(COMPILE_DIR, REPLAY_EXEC_WITH_EXT)
        else:
            print("Skipping replay since capture doesn't produce any file")

    for environment_var in environment_vars:
        del os.environ[environment_var[0]]


if __name__ == "__main__":
    DEFAULT_COMPILE_DIR = "out/Debug"  # relative to CWD: angle folder
    parser = argparse.ArgumentParser()
    parser.add_argument('--compile_dir', default=DEFAULT_COMPILE_DIR)
    args = parser.parse_args()
    main(args.compile_dir)
