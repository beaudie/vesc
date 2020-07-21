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
# --use_goma: uses goma for compiling and linking test. Off by default
# --gtest_filter: same as gtest_filter of Google's test framework. Default is */ES2_Vulkan
# --test_suite: test suite to execute on. Default is angle_end2end_tests
# --batch_count: number of tests in a batch. Default is 8
# --delete_generated: whether to delete the generated files and folders. On by default

import argparse
import distutils.util
import math
import multiprocessing
import os
import shlex
import shutil
import subprocess
import sys
import time

from sys import platform

DEFAULT_CAPTURE_BUILD_DIR = "out/CaptureTest"  # relative to angle folder
DEFAULT_REPLAY_BUILD_DIR = "out/ReplayTest"  # relative to angle folder
DEFAULT_FILTER = "*/ES2_Vulkan"
DEFAULT_TEST_SUITE = "angle_end2end_tests"
REPLAY_SAMPLE_FOLDER = "src/tests/capture_replay_tests"  # relative to angle folder
DEFAULT_BATCH_COUNT = 8  # number of tests batched together
TRACE_FILE_SUFFIX = "_capture_context1"  # because we only deal with 1 context right now
RESULT_TAG = "*RESULT"


class SubProcess():

    def __init__(self, command, to_main_stdout):
        parsed_command = shlex.split(command)
        # shell=False so that only 1 subprocess is spawned.
        # if shell=True, a shell probess is spawned, which in turn spawns the process running
        # the command. Since we do not have a handle to the 2nd process, we cannot terminate it.
        if not to_main_stdout:
            self.proc_handle = subprocess.Popen(
                parsed_command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=False)
        else:
            self.proc_handle = subprocess.Popen(parsed_command, shell=False)

    def BlockingRun(self):
        try:
            output = self.proc_handle.communicate()[0]
            return self.proc_handle.returncode, output
        except Exception as e:
            self.Kill()
            return -1, str(e)

    def Kill(self):
        self.proc_handle.kill()
        self.proc_handle.wait()


def CreateGnGenSubProcess(gn_path, build_dir, arguments, to_main_stdout=False):
    command = '"' + gn_path + '"' + ' gen --args="'
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
    return SubProcess(command, to_main_stdout)


def CreateAutoninjaSubProcess(autoninja_path, build_dir, target, to_main_stdout=False):
    command = '"' + autoninja_path + '" '
    command += target
    command += " -C "
    command += build_dir
    return SubProcess(command, to_main_stdout)


def GetGnAndAutoninjaAbsolutePaths():
    # get gn/autoninja absolute path because subprocess with shell=False doesn't look
    # into the PATH environment variable on Windows
    depot_tools_name = "depot_tools"
    paths = os.environ["PATH"].split(";")
    for path in paths:
        if path.endswith(depot_tools_name):
            if platform == "win32":
                return os.path.join(path, "gn.bat"), os.path.join(path, "autoninja.bat")
            else:
                return os.path.join(path, "gn"), os.path.join(path, "autoninja")
    return "", ""

# return a list of tests and their params in the form
# [(test1, params1), (test2, params2),...]
def GetTestNamesAndParams(test_exec_path, filter="*"):
    command = '"' + test_exec_path + '" --gtest_list_tests --gtest_filter=' + filter
    p = SubProcess(command, False)
    returncode, output = p.BlockingRun()

    if returncode != 0:
        print(output)
        return []
    output_lines = output.splitlines()
    tests = []
    last_testcase_name = ""
    test_name_splitter = "# GetParam() ="
    for line in output_lines:
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


def WriteGeneratedSwitchStatements(file, tests, call, params, returns=False, default_val=""):
    file.write("    switch(test)\n")
    file.write("    {\n")
    for i in range(len(tests)):
        file.write("        case " + str(i) + ":\n")
        if returns:
            file.write("            return " + tests[i].GetLabel() + "::" + call + "(" + params +
                       ");\n")
        else:
            file.write("            " + tests[i].GetLabel() + "::" + call + "(" + params + ");\n")
            file.write("            break;\n")
    file.write("        default:\n")
    if returns:
        file.write("            return " + default_val + ";\n")
    else:
        file.write("            break;\n")
    file.write("    }\n")


class Test():

    def __init__(self, full_test_name, params):
        self.full_test_name = full_test_name
        self.params = params
        self.capture_proc = None

    def __str__(self):
        return self.full_test_name + " Params: " + self.params

    def Run(self, test_exe_path):
        os.environ["ANGLE_CAPTURE_LABEL"] = self.GetLabel()
        command = '"' + test_exe_path + '" --gtest_filter=' + self.full_test_name
        self.capture_proc = SubProcess(command, False)
        return self.capture_proc.BlockingRun()

    def TerminateSubprocesses(self):
        if self.capture_proc and self.capture_proc.proc_handle.poll() == None:
            self.capture_proc.Kill()

    def GetLabel(self):
        return self.full_test_name.replace(".", "_").replace("/", "_")


class TestBatch():

    def __init__(self, use_goma, batch_count, delete_generated):
        self.use_goma = use_goma
        self.tests = []
        self.batch_count = batch_count
        self.gn_proc = None
        self.autoninja_proc = None
        self.replay_proc = None
        self.delete_generated = delete_generated

    def Run(self, test_exe_path, trace_folder_path):
        os.environ["ANGLE_CAPTURE_ENABLED"] = "1"
        if self.delete_generated:
            ClearFolderContent(trace_folder_path)
        return [test.Run(test_exe_path) for test in self.tests]

    def BuildReplay(self, gn_path, autoninja_path, build_dir, trace_dir, replay_exec,
                    trace_folder_path, composite_file_id, tests):
        # write gni file that holds all the traces files in a list
        self.CreateGNIFile(trace_folder_path, composite_file_id, tests)
        # write header and cpp composite files, which glue the trace files with CaptureReplayTests.cpp
        self.CreateTestsCompositeFiles(trace_folder_path, composite_file_id, tests)
        if not os.path.isfile(os.path.join(build_dir, "args.gn")):
            self.gn_proc = CreateGnGenSubProcess(
                gn_path, build_dir,
                [("use_goma", self.use_goma), ("angle_build_capture_replay_tests", "true"),
                 ("angle_capture_replay_test_trace_dir", '\\"' + trace_dir + '\\"'),
                 ("angle_with_capture_by_default", "false"),
                 ("angle_capture_replay_composite_file_id", str(composite_file_id))])
            returncode, output = self.gn_proc.BlockingRun()
            if returncode != 0:
                return returncode, output

        self.autoninja_proc = CreateAutoninjaSubProcess(autoninja_path, build_dir, replay_exec)
        returncode, output = self.autoninja_proc.BlockingRun()
        if returncode != 0:
            return returncode, output
        return 0, "Built replay of " + str(self)

    def RunReplay(self, replay_exe_path):
        os.environ["ANGLE_CAPTURE_ENABLED"] = "0"
        command = '"' + replay_exe_path + '"'
        self.replay_proc = SubProcess(command, False)
        return self.replay_proc.BlockingRun()

    def AddTest(self, test):
        assert len(self.tests) <= self.batch_count
        self.tests.append(test)

    def TerminateSubprocesses(self):
        for test in self.tests:
            test.TerminateSubprocesses()
        if self.gn_proc and self.gn_proc.proc_handle.poll() == None:
            self.gn_proc.Kill()
        if self.autoninja_proc and self.autoninja_proc.proc_handle.poll() == None:
            self.autoninja_proc.Kill()
        if self.replay_proc and self.replay_proc.proc_handle.poll() == None:
            self.replay_proc.Kill()

    # gni file, which holds all the sources for a replay application
    def CreateGNIFile(self, trace_folder_path, composite_file_id, tests):
        capture_sources = []
        for test in tests:
            label = test.GetLabel()
            trace_files = [label + TRACE_FILE_SUFFIX + ".h", label + TRACE_FILE_SUFFIX + ".cpp"]
            try:
                # reads from {label}_capture_context1_files.txt and adds the traces files recorded
                # in there to the list of trace files
                f = open(os.path.join(trace_folder_path, label + TRACE_FILE_SUFFIX + "_files.txt"))
                trace_files += f.read().splitlines()
            except IOError:
                continue
            capture_sources += trace_files
        f = open(os.path.join(trace_folder_path, "traces" + str(composite_file_id) + ".gni"), "w")
        f.write("generated_sources = [\n")
        # write the list of trace files to the gni file
        for filename in capture_sources:
            f.write('    "' + filename + '",\n')
        f.write("]")
        f.close()

    # header and cpp composite files, which glue the trace files with CaptureReplayTests.cpp
    def CreateTestsCompositeFiles(self, trace_folder_path, composite_file_id, tests):
        # write CompositeTests header file
        h_filename = "CompositeTests" + str(composite_file_id) + ".h"
        h_file = open(os.path.join(trace_folder_path, h_filename), "w")
        h_file.write("#pragma once\n")
        h_file.write("#include <vector>\n")
        h_file.write("#include <string>\n")
        h_file.write("\n")

        for test in tests:
            h_file.write('#include "' + test.GetLabel() + TRACE_FILE_SUFFIX + '.h"\n')
        h_file.write("\n")

        h_file.write("using DecompressCallback = uint8_t *(*)(const std::vector<uint8_t> &);\n")
        h_file.write("\n")

        h_file.write("void SetupContext1Replay(uint32_t test);\n")
        h_file.write("void ReplayContext1Frame(uint32_t test, uint32_t frameIndex);\n")
        h_file.write("void ResetContext1Replay(uint32_t test);\n")
        h_file.write(
            "std::vector<uint8_t> GetSerializedContext1StateData(uint32_t test, uint32_t frameIndex);\n"
        )
        h_file.write(
            "void SetBinaryDataDecompressCallback(uint32_t test, DecompressCallback callback);\n")
        h_file.write("void SetBinaryDataDir(uint32_t test, const char *dataDir);\n")
        h_file.write("\n")
        h_file.write("struct TestTraceInfo {\n")
        h_file.write("    std::string testName;\n")
        h_file.write("    uint32_t replayFrameStart;\n")
        h_file.write("    uint32_t replayFrameEnd;\n")
        h_file.write("    EGLint replayDrawSurfaceWidth;\n")
        h_file.write("    EGLint replayDrawSurfaceHeight;\n")
        h_file.write("    EGLint defaultFramebufferRedBits;\n")
        h_file.write("    EGLint defaultFramebufferGreenBits;\n")
        h_file.write("    EGLint defaultFramebufferBlueBits;\n")
        h_file.write("    EGLint defaultFramebufferAlphaBits;\n")
        h_file.write("    EGLint defaultFramebufferDepthBits;\n")
        h_file.write("    EGLint defaultFramebufferStencilBits;\n")
        h_file.write("    bool isBinaryDataCompressed;\n")
        h_file.write("};\n")
        h_file.write("\n")
        h_file.write("extern std::vector<TestTraceInfo> testTraceInfos;\n")
        h_file.close()

        # write CompositeTests cpp file
        cpp_file = open(
            os.path.join(trace_folder_path, "CompositeTests" + str(composite_file_id) + ".cpp"),
            "w")
        cpp_file.write('#include "' + h_filename + '"\n')

        cpp_file.write("std::vector<TestTraceInfo> testTraceInfos =\n")
        cpp_file.write("{\n")
        for i in range(len(tests)):
            namespace = tests[i].GetLabel()
            cpp_file.write("    {")
            cpp_file.write('"' + namespace + '",')
            cpp_file.write(namespace + "::kReplayFrameStart, ")
            cpp_file.write(namespace + "::kReplayFrameEnd, ")
            cpp_file.write(namespace + "::kReplayDrawSurfaceWidth, ")
            cpp_file.write(namespace + "::kReplayDrawSurfaceHeight, ")
            cpp_file.write(namespace + "::kDefaultFramebufferRedBits, ")
            cpp_file.write(namespace + "::kDefaultFramebufferGreenBits, ")
            cpp_file.write(namespace + "::kDefaultFramebufferBlueBits, ")
            cpp_file.write(namespace + "::kDefaultFramebufferAlphaBits, ")
            cpp_file.write(namespace + "::kDefaultFramebufferDepthBits, ")
            cpp_file.write(namespace + "::kDefaultFramebufferStencilBits, ")
            cpp_file.write(namespace + "::kIsBinaryDataCompressed")
            cpp_file.write("}")
            if i < len(tests) - 1:
                cpp_file.write(",")
            cpp_file.write("\n")
        cpp_file.write("};\n")
        cpp_file.write("\n")

        cpp_file.write("void SetupContext1Replay(uint32_t test)\n")
        cpp_file.write("{\n")
        WriteGeneratedSwitchStatements(cpp_file, tests, "SetupContext1Replay", "")
        cpp_file.write("}\n")
        cpp_file.write("\n")

        cpp_file.write("void ReplayContext1Frame(uint32_t test, uint32_t frameIndex)\n")
        cpp_file.write("{\n")
        WriteGeneratedSwitchStatements(cpp_file, tests, "ReplayContext1Frame", "frameIndex")
        cpp_file.write("}\n")
        cpp_file.write("\n")

        cpp_file.write("void ResetContext1Replay(uint32_t test)\n")
        cpp_file.write("{\n")
        WriteGeneratedSwitchStatements(cpp_file, tests, "ResetContext1Replay", "")
        cpp_file.write("}\n")
        cpp_file.write("\n")

        cpp_file.write(
            "std::vector<uint8_t> GetSerializedContext1StateData(uint32_t test, uint32_t frameIndex)\n"
        )
        cpp_file.write("{\n")
        WriteGeneratedSwitchStatements(cpp_file, tests, "GetSerializedContext1StateData",
                                       "frameIndex", True, "{}")
        cpp_file.write("}\n")
        cpp_file.write("\n")

        cpp_file.write(
            "void SetBinaryDataDecompressCallback(uint32_t test, DecompressCallback callback)\n")
        cpp_file.write("{\n")
        WriteGeneratedSwitchStatements(cpp_file, tests, "SetBinaryDataDecompressCallback",
                                       "callback")
        cpp_file.write("}\n")
        cpp_file.write("\n")

        cpp_file.write("void SetBinaryDataDir(uint32_t test, const char *dataDir)\n")
        cpp_file.write("{\n")
        WriteGeneratedSwitchStatements(cpp_file, tests, "SetBinaryDataDir", "dataDir")
        cpp_file.write("}\n")

        cpp_file.close()

    def __str__(self):
        repr_str = "TestBatch:\n"
        for test in self.tests:
            repr_str += ("\t" + str(test) + "\n")
        return repr_str

    def __getitem__(self, index):
        assert index < len(self.tests)
        return self.tests[index]

    def __iter__(self):
        return iter(self.tests)


def ClearFolderContent(path):
    all_files = []
    for f in os.listdir(path):
        if os.path.isfile(os.path.join(path, f)):
            os.remove(os.path.join(path, f))


def CanRunReplay(label, path):
    required_trace_files = {
        label + TRACE_FILE_SUFFIX + ".h", label + TRACE_FILE_SUFFIX + ".cpp",
        label + TRACE_FILE_SUFFIX + "_files.txt"
    }
    required_trace_files_count = 0
    frame_files_count = 0
    for f in os.listdir(path):
        if not os.path.isfile(os.path.join(path, f)):
            continue
        if f in required_trace_files:
            required_trace_files_count += 1
        elif f.startswith(label + TRACE_FILE_SUFFIX + "_frame"):
            frame_files_count += 1
        elif f.startswith(label +
                          TRACE_FILE_SUFFIX[:-1]) and not f.startswith(label + TRACE_FILE_SUFFIX):
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


def RunTests(job_queue, gn_path, autoninja_path, capture_build_dir, replay_build_dir, test_exec,
             replay_exec, trace_dir, result_list):
    trace_folder_path = os.path.join(REPLAY_SAMPLE_FOLDER, trace_dir)
    test_exec_path = os.path.join(capture_build_dir, test_exec)
    replay_exec_path = os.path.join(replay_build_dir, replay_exec)
    os.environ["ANGLE_CAPTURE_OUT_DIR"] = trace_folder_path
    composite_file_id = 1
    while not job_queue.empty():
        test_batch = None
        try:
            test_batch = job_queue.get()
            print("Running " + str(test_batch))
            sys.stdout.flush()
            test_results = test_batch.Run(test_exec_path, trace_folder_path)
            continued_tests = []
            for i in range(len(test_results)):
                returncode = test_results[i][0]
                output = test_results[i][1]
                if returncode != 0 or not CanRunReplay(test_batch[i].GetLabel(),
                                                       trace_folder_path):
                    # if a test stops here, appends the result to the result list
                    result_list.append((test_batch[i].full_test_name, "Skipped",
                    "Skipping replay since capture didn't produce appropriate files or has crashed. " \
                    + "STDOUT: " + output))
                else:
                    # otherwise, adds it to the list of continued tests
                    continued_tests.append(test_batch[i])

            returncode, output = test_batch.BuildReplay(gn_path, autoninja_path, replay_build_dir,
                                                        trace_dir, replay_exec, trace_folder_path,
                                                        composite_file_id, continued_tests)
            if not test_batch.delete_generated:
                composite_file_id += 1
            if returncode != 0:
                for test in continued_tests:
                    result_list.append(
                        (test.full_test_name, "Skipped",
                         "Skipping batch replays since failing to build batch replays. STDOUT: " +
                         output))
                continue
            returncode, output = test_batch.RunReplay(replay_exec_path)
            if returncode != 0:
                for test in continued_tests:
                    result_list.append((test.full_test_name, "Failed",
                                        "Failing batch run. STDOUT: " + output + str(returncode)))
            else:
                output_lines = output.splitlines()
                for output_line in output_lines:
                    words = output_line.split(" ")
                    if len(words) == 3 and words[0] == RESULT_TAG:
                        if int(words[2]) == 0:
                            result_list.append((words[1], "Passed", ""))
                        else:
                            result_list.append((words[1], "Failed", ""))
        except Exception as e:
            print(e)
            sys.stdout.flush()
            if test_batch:
                test_batch.TerminateSubprocesses()


def CreateReplayBuildFolders(folder_num, replay_build_dir):
    for i in range(folder_num):
        replay_build_dir_name = replay_build_dir + str(i)
        if os.path.isdir(replay_build_dir_name):
            shutil.rmtree(replay_build_dir_name)
        os.makedirs(replay_build_dir_name)


def SafeDeleteFolder(folder_name):
    while os.path.isdir(folder_name):
        try:
            shutil.rmtree(folder_name)
        except Exception:
            pass


def DeleteReplayBuildFolders(folder_num, replay_build_dir, trace_folder):
    for i in range(folder_num):
        folder_name = replay_build_dir + str(i)
        if os.path.isdir(folder_name):
            SafeDeleteFolder(folder_name)


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
            SafeDeleteFolder(folder_path)


def main(capture_build_dir, replay_build_dir, use_goma, gtest_filter, test_exec, batch_count,
         delete_generated):
    start_time = time.time()
    # set the number of workers to be cpu_count - 1 (since the main process already takes up a CPU
    # core). Whenever a worker is available, it grabs the next job from the job queue and runs it.
    # The worker closes down when there is no more job.
    worker_count = multiprocessing.cpu_count() - 1
    cwd = SetCWDToAngleFolder()
    trace_folder = "traces"
    if not os.path.isdir(capture_build_dir):
        os.makedirs(capture_build_dir)
    CreateReplayBuildFolders(worker_count, replay_build_dir)
    CreateTraceFolders(worker_count, trace_folder)

    replay_exec = "capture_replay_tests"
    if platform == "win32":
        test_exec += ".exe"
        replay_exec += ".exe"
    gn_path, autoninja_path = GetGnAndAutoninjaAbsolutePaths()
    if gn_path == "" or autoninja_path == "":
        print("No gn or autoninja found on system")
        return
    # generate gn files
    gn_proc = CreateGnGenSubProcess(gn_path, capture_build_dir,
                                    [("use_goma", use_goma),
                                     ("angle_with_capture_by_default", "true")], True)
    returncode, output = gn_proc.BlockingRun()
    if returncode != 0:
        return
    autoninja_proc = CreateAutoninjaSubProcess(autoninja_path, capture_build_dir, test_exec, True)
    returncode, output = autoninja_proc.BlockingRun()
    if returncode != 0:
        return
    # get a list of tests
    test_names_and_params = GetTestNamesAndParams(
        os.path.join(capture_build_dir, test_exec), gtest_filter)

    # objects created by manager can be shared by multiple processes. We use it to create
    # collections that are shared by multiple processes such as job queue or result list.
    manager = multiprocessing.Manager()
    job_queue = manager.Queue()
    test_batch_num = int(math.ceil(len(test_names_and_params) / float(batch_count)))

    for batch_index in range(test_batch_num):
        batch = TestBatch(use_goma, batch_count, delete_generated)
        for test_in_batch_index in range(batch.batch_count):
            test_index = batch_index * batch.batch_count + test_in_batch_index
            if test_index >= len(test_names_and_params):
                break
            batch.AddTest(
                Test(test_names_and_params[test_index][0], test_names_and_params[test_index][1]))
        job_queue.put(batch)

    environment_vars = [("ANGLE_CAPTURE_FRAME_END", "100"), ("ANGLE_CAPTURE_SERIALIZE_STATE", "1")]
    for environment_var in environment_vars:
        os.environ[environment_var[0]] = environment_var[1]

    passed_count = 0
    failed_count = 0
    skipped_count = 0
    failed_tests = []

    # result list is created by manager and can be shared by multiple processes. Each subprocess
    # populates the result list with the results of its test runs. After all subprocesses finish,
    # the main process processes the results in the result list.
    # An item in the result list is a tuple with 3 values (testname, result, output).
    # The "result" can take 3 values "Passed", "Failed", "Skipped". The output is the stdout and
    # the stderr of the test appended together.
    result_list = manager.list()
    workers = []
    # so that we do not spawn more processes than we actually need
    worker_count = min(worker_count, test_batch_num)
    for i in range(worker_count):
        proc = multiprocessing.Process(
            target=RunTests,
            args=(job_queue, gn_path, autoninja_path, capture_build_dir, replay_build_dir + str(i),
                  test_exec, replay_exec, trace_folder + str(i), result_list))
        workers.append(proc)
        proc.start()

    for worker in workers:
        worker.join()

    for environment_var in environment_vars:
        del os.environ[environment_var[0]]
    end_time = time.time()

    print("\n\n\n")
    print("Results:")
    for result in result_list:
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
    if delete_generated:
        DeleteTraceFolders(worker_count, trace_folder)
        DeleteReplayBuildFolders(worker_count, replay_build_dir, trace_folder)
    if delete_generated and os.path.isdir(capture_build_dir):
        SafeDeleteFolder(capture_build_dir)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--capture_build_dir', default=DEFAULT_CAPTURE_BUILD_DIR)
    parser.add_argument('--replay_build_dir', default=DEFAULT_REPLAY_BUILD_DIR)
    parser.add_argument('--use_goma', default="false")
    parser.add_argument('--gtest_filter', default=DEFAULT_FILTER)
    parser.add_argument('--test_suite', default=DEFAULT_TEST_SUITE)
    parser.add_argument('--batch_count', default=DEFAULT_BATCH_COUNT)
    parser.add_argument('--delete_generated', default="true")
    args = parser.parse_args()
    main(args.capture_build_dir, args.replay_build_dir, args.use_goma, args.gtest_filter,
         args.test_suite, int(args.batch_count), distutils.util.strtobool(args.delete_generated))
