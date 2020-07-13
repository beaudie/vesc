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
# --capture_build_dir: specifies capture build directory relative to angle folder.
# Default is out/CaptureDebug
# --replay_build_dir: specifies replay build directory relative to angle folder.
# Default is out/ReplayDebug
# --verbose: off by default
# --use_goma: uses goma for compiling and linking test. Off by default
# --gtest_filter: same as gtest_filter of Google's test framework. Default is */ES2_Vulkan
# --test_suite: test suite to execute on. Default is angle_end2end_tests

import argparse
import distutils.util
import multiprocessing
import os
import shutil
import subprocess
import time

from sys import platform

DEFAULT_CAPTURE_BUILD_DIR = "out/CaptureDebug"  # relative to angle folder
DEFAULT_REPLAY_BUILD_DIR = "out/ReplayDebug"  # relative to angle folder
DEFAULT_FILTER = "*/ES2_Vulkan"
DEFAULT_TEST_SUITE = "angle_end2end_tests"
REPLAY_SAMPLE_FOLDER = "src/tests/capture_replay_tests"  # relative to angle folder


class Logger():
    verbose = False

    @staticmethod
    def log(msg):
        if Logger.verbose:
            print(msg)


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
def GetTestNamesAndParams(test_exec_path, filter="*"):
    output = subprocess.check_output(
        '"' + test_exec_path + '" --gtest_list_tests --gtest_filter=' + filter,
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

    def __init__(self, full_test_name, params, use_goma):
        self.full_test_name = full_test_name
        self.params = params
        self.use_goma = use_goma

    def __str__(self):
        return self.full_test_name + " Params: " + self.params

    def Run(self, test_exe_path):
        try:
            output = subprocess.check_output(
                '"' + test_exe_path + '" --gtest_filter=' + self.full_test_name + \
                ' --use-angle=vulkan',
                shell=True,
                stderr=subprocess.PIPE)
            Logger.log("Ran " + self.full_test_name + " with capture")
            return (0, output)
        except subprocess.CalledProcessError as e:
            return (e.returncode, e.output)

    def BuildReplay(self, build_dir, trace_dir, replay_exec, gn_completed_list,
                    gn_completed_list_index):
        try:
            if not gn_completed_list[gn_completed_list_index]:
                RunGnGen(build_dir, [("use_goma", self.use_goma),
                                     ("angle_with_capture_by_default", "true"),
                                     ("angle_build_capture_replay_tests", "true"),
                                     ("angle_trace_directory_name", '\\"' + trace_dir + '\\"')])
                gn_completed_list[gn_completed_list_index] = True
            RunAutoninja(build_dir, replay_exec)
            Logger.log("Built replay of " + self.full_test_name)
            return (0, "Built replay of " + self.full_test_name)
        except subprocess.CalledProcessError as e:
            return (e.returncode, e.output)

    def RunReplay(self, replay_exe_path):
        try:
            output = subprocess.check_output(
                '"' + replay_exe_path + '" --use-angle=vulkan', shell=True, stderr=subprocess.PIPE)
            Logger.log("Ran replay of " + self.full_test_name)
            return (0, output)
        except subprocess.CalledProcessError as e:
            return (e.returncode, e.output)


def ClearFolderContent(path):
    all_files = []
    for f in os.listdir(path):
        if os.path.isfile(os.path.join(path, f)) and f.startswith("angle_capture_context"):
            os.remove(os.path.join(path, f))


def CanRunReplay(path):
    required_trace_files = {
        "angle_capture_context1.h", "angle_capture_context1.cpp",
        "angle_capture_context1_files.txt"
    }
    required_trace_files_count = 0
    frame_files_count = 0
    for f in os.listdir(path):
        if not os.path.isfile(os.path.join(path, f)):
            continue
        if f in required_trace_files:
            required_trace_files_count += 1
        elif f.startswith("angle_capture_context1_frame"):
            frame_files_count += 1
        elif f.startswith("angle_capture_context") and not f.startswith("angle_capture_context1"):
            # if trace_files of another context exists, then the test creates multiple contexts
            return False
    # angle_capture_context1.angledata.gz can be missing
    return required_trace_files_count == len(required_trace_files) and frame_files_count >= 1


def SetCWDToAngleFolder():
    angle_folder = "angle"
    cwd = os.path.dirname(os.path.abspath(__file__))
    cwd = cwd.split(angle_folder)[0] + angle_folder
    os.chdir(cwd)
    return cwd


def RunTest(test, capture_build_dir, replay_build_dir, test_exec, replay_exec, trace_dir,
            gn_completed_list, gn_completed_list_index, result_queue):
    trace_folder_path = os.path.join(REPLAY_SAMPLE_FOLDER, trace_dir)
    ClearFolderContent(trace_folder_path)
    os.environ["ANGLE_CAPTURE_ENABLED"] = "1"
    os.environ["ANGLE_CAPTURE_OUT_DIR"] = trace_folder_path
    run_output = test.Run(os.path.join(capture_build_dir, test_exec))
    if run_output[0] != 0 or not CanRunReplay(trace_folder_path):
        result_queue.put ((test.full_test_name, "Skipped",
        "Skipped: " + test.full_test_name + ". Skipping replay since capture" + \
            " didn't produce appropriate files or has crashed"))
        return
    os.environ["ANGLE_CAPTURE_ENABLED"] = "0"
    build_output = test.BuildReplay(replay_build_dir, trace_dir, replay_exec, gn_completed_list,
                                    gn_completed_list_index)
    if build_output[0] != 0:
        result_queue.put((test.full_test_name, "Skipped", "Skipped: " + test.full_test_name +
                          ". Skipping replay since failing to build replay"))
        return
    replay_output = test.RunReplay(os.path.join(replay_build_dir, replay_exec))
    if replay_output[0] != 0:
        result_queue.put((test.full_test_name, "Failed", replay_output[1]))
    else:
        result_queue.put((test.full_test_name, "Passed", ""))


def GetFirstAvailableCPU(cpus):
    for i in range(len(cpus)):
        if not cpus[i] or not cpus[i].is_alive():
            return i
    return -1


def CreateReplayBuildFolders(folder_num, replay_build_dir):
    for i in range(folder_num):
        replay_build_dir_name = replay_build_dir + str(i)
        if os.path.isdir(replay_build_dir_name):
            shutil.rmtree(replay_build_dir_name)
        os.makedirs(replay_build_dir_name)


def DeleteReplayBuildFolders(folder_num, replay_build_dir, trace_folder):
    for i in range(folder_num):
        folder_name = replay_build_dir + str(i)
        if os.path.isdir(folder_name):
            shutil.rmtree(folder_name)


def CreateTraceFolders(folder_num, trace_folder):
    for i in range(folder_num):
        folder_name = trace_folder + str(i)
        folder_path = os.path.join(REPLAY_SAMPLE_FOLDER, folder_name)
        if os.path.isdir(folder_path):
            shutil.rmtree(folder_path)
        os.makedirs(folder_path)


def DeleteTraceFolders(folder_num, trace_folder):
    for i in range(folder_num):
        folder_name = trace_folder + str(i)
        folder_path = os.path.join(REPLAY_SAMPLE_FOLDER, folder_name)
        if os.path.isdir(folder_path):
            shutil.rmtree(folder_path)


def main(capture_build_dir, replay_build_dir, verbose, use_goma, gtest_filter, test_exec):
    start_time = time.time()
    cpu_count = multiprocessing.cpu_count() - 1
    Logger.verbose = verbose
    cwd = SetCWDToAngleFolder()
    trace_folder = "traces"
    if not os.path.isdir(capture_build_dir):
        os.makedirs(capture_build_dir)
    CreateReplayBuildFolders(cpu_count, replay_build_dir)
    CreateTraceFolders(cpu_count, trace_folder)

    replay_exec = "capture_replay_tests"
    if platform == "win32":
        test_exec += ".exe"
        replay_exec += ".exe"
    # generate gn files
    RunGnGen(capture_build_dir, [("use_goma", use_goma),
                                 ("angle_with_capture_by_default", "true")], True)
    # build angle_end2end
    RunAutoninja(capture_build_dir, test_exec, True)
    # get a list of tests
    test_names_and_params = GetTestNamesAndParams(
        os.path.join(capture_build_dir, test_exec), gtest_filter)
    all_tests = multiprocessing.Queue()
    for test_name_and_params in test_names_and_params:
        all_tests.put(Test(test_name_and_params[0], test_name_and_params[1], use_goma))

    environment_vars = [("ANGLE_CAPTURE_FRAME_END", "100"), ("ANGLE_CAPTURE_SERIALIZE_STATE", "1")]
    for environment_var in environment_vars:
        os.environ[environment_var[0]] = environment_var[1]

    passed_count = 0
    failed_count = 0
    skipped_count = 0
    failed_tests = []

    manager = multiprocessing.Manager()
    gn_completed_list = manager.list()
    result_queue = manager.Queue()
    [gn_completed_list.append(False) for i in range(cpu_count)]

    cpus = [None for i in range(cpu_count)]
    while not all_tests.empty():
        test = all_tests.get()
        first_available_cpu = GetFirstAvailableCPU(cpus)
        while first_available_cpu == -1:
            time.sleep(0.01)
            first_available_cpu = GetFirstAvailableCPU(cpus)
        print("Running " + test.full_test_name)
        proc = multiprocessing.Process(
            target=RunTest,
            args=(test, capture_build_dir, replay_build_dir + str(first_available_cpu), test_exec,
                  replay_exec, trace_folder + str(first_available_cpu), gn_completed_list,
                  first_available_cpu, result_queue))
        if cpus[first_available_cpu]:
            cpus[first_available_cpu].join()
        cpus[first_available_cpu] = proc
        proc.start()
    [cpu.join() for cpu in cpus if cpu]
    for environment_var in environment_vars:
        del os.environ[environment_var[0]]
    end_time = time.time()

    print("\n\n\n")
    print("Results:")
    while not result_queue.empty():
        result = result_queue.get()
        output_string = result[1] + ": " + result[0] + ". "
        if result[1] == "Skipped":
            output_string += result[2]
            skipped_count += 1
        elif result[1] == "Failed":
            output_string += result[2]
            failed_tests.append(result[0])
            failed_count += 1
        else:
            passed_count += 1
        print(output_string)

    print("\n\n")
    print("Elapsed time: " + str(end_time - start_time) + " seconds")
    print("Passed: "+ str(passed_count) + " Failed: " + str(failed_count) + \
    " Skipped: " + str(skipped_count))
    print("Failed tests:")
    for failed_test in failed_tests:
        print("\t" + failed_test)
    DeleteTraceFolders(cpu_count, trace_folder)
    DeleteReplayBuildFolders(cpu_count, replay_build_dir, trace_folder)
    if os.path.isdir(capture_build_dir):
        shutil.rmtree(capture_build_dir)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--capture_build_dir', default=DEFAULT_CAPTURE_BUILD_DIR)
    parser.add_argument('--replay_build_dir', default=DEFAULT_REPLAY_BUILD_DIR)
    parser.add_argument('--verbose', default="False")
    parser.add_argument('--use_goma', default="false")
    parser.add_argument('--gtest_filter', default=DEFAULT_FILTER)
    parser.add_argument('--test_suite', default=DEFAULT_TEST_SUITE)
    args = parser.parse_args()
    main(args.capture_build_dir, args.replay_build_dir, distutils.util.strtobool(args.verbose),
         args.use_goma, args.gtest_filter, args.test_suite)
