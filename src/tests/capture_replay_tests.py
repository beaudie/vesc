#! /usr/bin/env vpython3
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
# Command line arguments: run with --help for a full list.

import argparse
import concurrent.futures
import difflib
import distutils.util
import fnmatch
import getpass
import json
import logging
import math
import multiprocessing
import os
import pathlib
import queue
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import time
import traceback

SCRIPT_DIR = str(pathlib.Path(__file__).resolve().parent)
PY_UTILS = str(pathlib.Path(SCRIPT_DIR) / 'py_utils')
if PY_UTILS not in sys.path:
    os.stat(PY_UTILS) and sys.path.insert(0, PY_UTILS)
import angle_test_util

PIPE_STDOUT = True
DEFAULT_OUT_DIR = "out/CaptureReplayTest"  # relative to angle folder
DEFAULT_FILTER = "*/ES2_Vulkan_SwiftShader"
DEFAULT_TEST_SUITE = "angle_end2end_tests"
REPLAY_SAMPLE_FOLDER = "src/tests/capture_replay_tests"  # relative to angle folder
DEFAULT_BATCH_COUNT = 8  # number of tests batched together for capture
CAPTURE_FRAME_END = 1000
TRACE_FILE_SUFFIX = "_context"  # because we only deal with 1 context right now
RESULT_TAG = "*RESULT"
STATUS_MESSAGE_PERIOD = 20  # in seconds
CAPTURE_SUBPROCESS_TIMEOUT = 600  # in seconds
REPLAY_SUBPROCESS_TIMEOUT = 60  # in seconds
DEFAULT_RESULT_FILE = "results.txt"
DEFAULT_LOG_LEVEL = "info"
DEFAULT_MAX_JOBS = 8
DEFAULT_MAX_NINJA_JOBS = 1
REPLAY_BINARY = "capture_replay_tests"
if sys.platform == "win32":
    REPLAY_BINARY += ".exe"
TRACE_FOLDER = "traces"

EXIT_SUCCESS = 0
EXIT_FAILURE = 1
REPLAY_INITIALIZATION_FAILURE = -1
REPLAY_SERIALIZATION_FAILURE = -2

switch_case_without_return_template = """\
        case {case}:
            {namespace}::{call}({params});
            break;
"""

switch_case_with_return_template = """\
        case {case}:
            return {namespace}::{call}({params});
"""

default_case_without_return_template = """\
        default:
            break;"""
default_case_with_return_template = """\
        default:
            return {default_val};"""


def winext(name, ext):
    return ("%s.%s" % (name, ext)) if sys.platform == "win32" else name

class SubProcess():

    def __init__(self, command, logger, env=os.environ, pipe_stdout=PIPE_STDOUT):
        # shell=False so that only 1 subprocess is spawned.
        # if shell=True, a shell process is spawned, which in turn spawns the process running
        # the command. Since we do not have a handle to the 2nd process, we cannot terminate it.
        if pipe_stdout:
            self.proc_handle = subprocess.Popen(
                command, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=False)
        else:
            self.proc_handle = subprocess.Popen(command, env=env, shell=False)
        self._logger = logger

    def Join(self, timeout):
        self._logger.debug('Joining with subprocess %d, timeout %s' % (self.Pid(), str(timeout)))
        output = self.proc_handle.communicate(timeout=timeout)[0]
        if output:
            output = output.decode('utf-8')
        else:
            output = ''
        return self.proc_handle.returncode, output

    def Pid(self):
        return self.proc_handle.pid

    def Kill(self):
        self.proc_handle.terminate()
        self.proc_handle.wait()


# class that manages all child processes of a process. Any process thats spawns subprocesses
# should have this. This object is created inside the main process, and each worker process.
class ChildProcessesManager():

    @classmethod
    def _GetGnAbsolutePaths(self):
        return os.path.join('third_party', 'depot_tools', winext('gn', 'bat'))

    @classmethod
    def _GetAutoNinjaAbsolutePaths(self):
        return os.path.join('third_party', 'depot_tools', 'autoninja.py')

    def __init__(self, args, logger, ninja_lock):
        # a dictionary of Subprocess, with pid as key
        self.subprocesses = {}
        # list of Python multiprocess.Process handles
        self.workers = []

        self._gn_path = self._GetGnAbsolutePaths()
        self._autoninja_path = self._GetAutoNinjaAbsolutePaths()
        self._logger = logger
        self._ninja_lock = ninja_lock
        self.runtimes = {}
        self._args = args

    def RunSubprocess(self, command, env=None, pipe_stdout=True, timeout=None):
        proc = SubProcess(command, self._logger, env, pipe_stdout)
        self._logger.debug('Created subprocess: %s with pid %d' % (' '.join(command), proc.Pid()))
        self.subprocesses[proc.Pid()] = proc
        start_time = time.time()
        try:
            returncode, output = self.subprocesses[proc.Pid()].Join(timeout)
            elapsed_time = time.time() - start_time
            cmd_name = os.path.basename(command[0])
            self.runtimes.setdefault(cmd_name, 0.0)
            self.runtimes[cmd_name] += elapsed_time
            self.RemoveSubprocess(proc.Pid())
            if returncode != 0:
                return -1, output
            return returncode, output
        except KeyboardInterrupt:
            raise
        except subprocess.TimeoutExpired as e:
            self.RemoveSubprocess(proc.Pid())
            return -2, str(e)
        except Exception as e:
            self.RemoveSubprocess(proc.Pid())
            return -1, str(e)

    def RemoveSubprocess(self, subprocess_id):
        assert subprocess_id in self.subprocesses
        self.subprocesses[subprocess_id].Kill()
        del self.subprocesses[subprocess_id]

    def AddWorker(self, worker):
        self.workers.append(worker)

    def KillAll(self):
        for subprocess_id in self.subprocesses:
            self.subprocesses[subprocess_id].Kill()
        for worker in self.workers:
            worker.terminate()
            worker.join()
            worker.close()  # to release file descriptors immediately
        self.subprocesses = {}
        self.workers = []

    def JoinWorkers(self):
        for worker in self.workers:
            worker.join()
            worker.close()
        self.workers = []

    def IsAnyWorkerAlive(self):
        return any([worker.is_alive() for worker in self.workers])

    def GetRemainingWorkers(self):
        count = 0
        for worker in self.workers:
            if worker.is_alive():
                count += 1
        return count

    def RunGNGen(self, build_dir, pipe_stdout, extra_gn_args=[]):
        gn_args = [('angle_with_capture_by_default', 'true')] + extra_gn_args
        if self._args.use_reclient:
            gn_args.append(('use_remoteexec', 'true'))
        if not self._args.debug:
            gn_args.append(('is_debug', 'false'))
            gn_args.append(('symbol_level', '1'))
            gn_args.append(('angle_assert_always_on', 'true'))
        if self._args.asan:
            gn_args.append(('is_asan', 'true'))
        args_str = ' '.join(['%s=%s' % (k, v) for (k, v) in gn_args])
        cmd = [self._gn_path, 'gen', '--args=%s' % args_str, build_dir]
        self._logger.info(' '.join(cmd))
        return self.RunSubprocess(cmd, pipe_stdout=pipe_stdout)

    def RunAutoNinja(self, build_dir, target, pipe_stdout):
        cmd = [sys.executable, self._autoninja_path, '-C', build_dir, target]
        with self._ninja_lock:
            self._logger.info(' '.join(cmd))
            return self.RunSubprocess(cmd, pipe_stdout=pipe_stdout)


def GetTestsListForFilter(args, test_path, filter, logger):
    cmd = GetRunCommand(args, test_path) + ["--list-tests", "--gtest_filter=%s" % filter]
    logger.info('Getting test list from "%s"' % " ".join(cmd))
    return subprocess.check_output(cmd, text=True)


def ParseTestNamesFromTestList(output, test_expectation, also_run_skipped_for_capture_tests,
                               logger):
    output_lines = output.splitlines()
    tests = []
    seen_start_of_tests = False
    disabled = 0
    for line in output_lines:
        l = line.strip()
        if l == 'Tests list:':
            seen_start_of_tests = True
        elif l == 'End tests list.':
            break
        elif not seen_start_of_tests:
            pass
        elif not test_expectation.TestIsSkippedForCapture(l) or also_run_skipped_for_capture_tests:
            tests.append(l)
        else:
            disabled += 1

    logger.info('Found %s tests and %d disabled tests.' % (len(tests), disabled))
    return tests


def GetRunCommand(args, command):
    if args.xvfb:
        return ['vpython', 'testing/xvfb.py', command]
    else:
        return [command]


class GroupedResult():
    Passed = "Pass"
    Failed = "Fail"
    TimedOut = "Timeout"
    CompileFailed = "CompileFailed"
    CaptureFailed = "CaptureFailed"
    ReplayFailed = "ReplayFailed"
    Skipped = "Skipped"
    FailedToTrace = "FailedToTrace"

    ResultTypes = [
        Passed, Failed, TimedOut, CompileFailed, CaptureFailed, ReplayFailed, Skipped,
        FailedToTrace
    ]

    def __init__(self, resultcode, message, output, tests):
        self.resultcode = resultcode
        self.message = message
        self.output = output
        self.tests = []
        for test in tests:
            self.tests.append(test)


def BatchName(batch_or_result):
    return 'batch_%03d' % batch_or_result.batch_index


class TestBatchResult():

    display_output_lines = 20

    def __init__(self, batch_index, grouped_results, verbose):
        self.batch_index = batch_index
        self.results = {}
        for result_type in GroupedResult.ResultTypes:
            self.results[result_type] = []

        for grouped_result in grouped_results:
            for test in grouped_result.tests:
                self.results[grouped_result.resultcode].append(test.full_test_name)

        self.repr_str = ""
        self.GenerateRepresentationString(grouped_results, verbose)

    def __str__(self):
        return self.repr_str

    def GenerateRepresentationString(self, grouped_results, verbose):
        self.repr_str += BatchName(self) + "\n"
        for grouped_result in grouped_results:
            self.repr_str += grouped_result.resultcode + ": " + grouped_result.message + "\n"
            for test in grouped_result.tests:
                self.repr_str += "\t" + test.full_test_name + "\n"
            if verbose:
                self.repr_str += grouped_result.output
            else:
                if grouped_result.resultcode == GroupedResult.CompileFailed:
                    self.repr_str += TestBatchResult.ExtractErrors(grouped_result.output)
                elif grouped_result.resultcode != GroupedResult.Passed:
                    self.repr_str += grouped_result.output

    def ExtractErrors(output):
        lines = output.splitlines()
        error_lines = []
        for i in range(len(lines)):
            if ": error:" in lines[i]:
                error_lines.append(lines[i] + "\n")
                if i + 1 < len(lines):
                    error_lines.append(lines[i + 1] + "\n")
        return "".join(error_lines)


class Test():

    def __init__(self, test_name):
        self.full_test_name = test_name
        self.params = test_name.split('/')[1]
        self.context_id = 0
        self.test_index = -1  # index of test within a test batch
        self._label = self.full_test_name.replace(".", "_").replace("/", "_")
        self.skipped_by_suite = False

    def __str__(self):
        return self.full_test_name + " Params: " + self.params

    def GetLabel(self):
        return self._label

    def CanRunReplay(self, trace_folder_path):
        test_files = []
        label = self.GetLabel()
        assert (self.context_id == 0)
        for f in os.listdir(trace_folder_path):
            if os.path.isfile(os.path.join(trace_folder_path, f)) and f.startswith(label):
                test_files.append(f)
        frame_files_count = 0
        context_header_count = 0
        context_source_count = 0
        source_json_count = 0
        context_id = 0
        for f in test_files:
            # TODO: Consolidate. http://anglebug.com/42266223
            if "_001.cpp" in f or "_001.c" in f:
                frame_files_count += 1
            elif f.endswith(".json"):
                source_json_count += 1
            elif f.endswith(".h"):
                context_header_count += 1
                if TRACE_FILE_SUFFIX in f:
                    context = f.split(TRACE_FILE_SUFFIX)[1][:-2]
                    context_id = int(context)
            # TODO: Consolidate. http://anglebug.com/42266223
            elif f.endswith(".cpp") or f.endswith(".c"):
                context_source_count += 1
        can_run_replay = frame_files_count >= 1 and context_header_count >= 1 \
            and context_source_count >= 1 and source_json_count == 1
        if not can_run_replay:
            return False
        self.context_id = context_id
        return True


def _FormatEnv(env):
    return ' '.join(['%s=%s' % (k, v) for (k, v) in env.items()])


def GetCaptureEnv(args, trace_folder_path):
    extra_env = {
        'ANGLE_CAPTURE_SERIALIZE_STATE': '1',
        'ANGLE_FEATURE_OVERRIDES_ENABLED': 'forceRobustResourceInit:forceInitShaderVariables',
        'ANGLE_FEATURE_OVERRIDES_DISABLED': 'supportsHostImageCopy',
        'ANGLE_CAPTURE_ENABLED': '1',
        'ANGLE_CAPTURE_OUT_DIR': trace_folder_path,
    }

    if args.mec > 0:
        extra_env['ANGLE_CAPTURE_FRAME_START'] = '{}'.format(args.mec)
        extra_env['ANGLE_CAPTURE_FRAME_END'] = '{}'.format(args.mec + 1)
    else:
        extra_env['ANGLE_CAPTURE_FRAME_END'] = '{}'.format(CAPTURE_FRAME_END)

    if args.expose_nonconformant_features:
        extra_env['ANGLE_FEATURE_OVERRIDES_ENABLED'] += ':exposeNonConformantExtensionsAndVersions'

    return extra_env


def UnlinkContextStateJsonFilesIfPresent(replay_build_dir, test_name):
    frame = 1
    while True:
        capture_file = "{}/{}_ContextCaptured{}.json".format(replay_build_dir, test_name, frame)
        replay_file = "{}/{}_ContextReplayed{}.json".format(replay_build_dir, test_name, frame)
        if os.path.exists(capture_file):
            os.unlink(capture_file)
        if os.path.exists(replay_file):
            os.unlink(replay_file)

        if frame > CAPTURE_FRAME_END:
            break
        frame = frame + 1


class TestBatch():

    def __init__(self, args, logger, batch_index):
        self.args = args
        self.tests = []
        self.results = []
        self.logger = logger
        self.batch_index = batch_index

    def SetWorkerId(self, worker_id):
        self.trace_dir = "%s%d" % (TRACE_FOLDER, worker_id)
        self.trace_folder_path = os.path.join(REPLAY_SAMPLE_FOLDER, self.trace_dir)

    def RunWithCapture(self, args, child_processes_manager):
        test_exe_path = os.path.join(args.out_dir, 'Capture', args.test_suite)

        extra_env = GetCaptureEnv(args, self.trace_folder_path)
        env = {**os.environ.copy(), **extra_env}

        if not self.args.keep_temp_files:
            ClearFolderContent(self.trace_folder_path)
        filt = ':'.join([test.full_test_name for test in self.tests])

        cmd = GetRunCommand(args, test_exe_path)
        results_file = tempfile.mktemp()
        cmd += [
            '--gtest_filter=%s' % filt,
            '--angle-per-test-capture-label',
            '--results-file=' + results_file,
        ]
        self.logger.info('%s %s' % (_FormatEnv(extra_env), ' '.join(cmd)))

        returncode, output = child_processes_manager.RunSubprocess(
            cmd, env, timeout=CAPTURE_SUBPROCESS_TIMEOUT)

        if args.show_capture_stdout:
            self.logger.info("Capture stdout: %s" % output)

        if returncode == -1:
            self.results.append(GroupedResult(GroupedResult.CaptureFailed, "", output, self.tests))
            return False
        elif returncode == -2:
            self.results.append(GroupedResult(GroupedResult.TimedOut, "", "", self.tests))
            return False

        with open(results_file) as f:
            test_results = json.load(f)
        os.unlink(results_file)
        for test in self.tests:
            test_result = test_results['tests'][test.full_test_name]
            if test_result['actual'] == 'SKIP':
                test.skipped_by_suite = True

        return True

    def RemoveTestsThatDoNotProduceAppropriateTraceFiles(self):
        continued_tests = []
        skipped_tests = []
        failed_to_trace_tests = []
        for test in self.tests:
            if not test.CanRunReplay(self.trace_folder_path):
                if test.skipped_by_suite:
                    skipped_tests.append(test)
                else:
                    failed_to_trace_tests.append(test)
            else:
                continued_tests.append(test)
        if len(skipped_tests) > 0:
            self.results.append(
                GroupedResult(GroupedResult.Skipped, "Skipping replay since test skipped by suite",
                              "", skipped_tests))
        if len(failed_to_trace_tests) > 0:
            self.results.append(
                GroupedResult(GroupedResult.FailedToTrace,
                              "Test not skipped but failed to produce trace files", "",
                              failed_to_trace_tests))

        return continued_tests

    def BuildReplay(self, replay_build_dir, composite_file_id, tests, child_processes_manager):
        # write gni file that holds all the traces files in a list
        self.CreateTestNamesFile(composite_file_id, tests)

        gn_args = [('angle_build_capture_replay_tests', 'true'),
                   ('angle_capture_replay_test_trace_dir', '"%s"' % self.trace_dir),
                   ('angle_capture_replay_composite_file_id', str(composite_file_id))]
        returncode, output = child_processes_manager.RunGNGen(replay_build_dir, True, gn_args)
        if returncode != 0:
            self.logger.warning('GN failure output: %s' % output)
            self.results.append(
                GroupedResult(GroupedResult.CompileFailed, "Build replay failed at gn generation",
                              output, tests))
            return False
        returncode, output = child_processes_manager.RunAutoNinja(replay_build_dir, REPLAY_BINARY,
                                                                  True)
        if returncode != 0:
            self.logger.warning('Ninja failure output: %s' % output)
            self.results.append(
                GroupedResult(GroupedResult.CompileFailed, "Build replay failed at ninja", output,
                              tests))
            return False
        return True

    def RunReplay(self, args, replay_build_dir, replay_exe_path, child_processes_manager, tests):
        extra_env = {}
        if args.expose_nonconformant_features:
            extra_env[
                'ANGLE_FEATURE_OVERRIDES_ENABLED'] = 'exposeNonConformantExtensionsAndVersions'

        env = {**os.environ.copy(), **extra_env}

        for test in tests:
            UnlinkContextStateJsonFilesIfPresent(replay_build_dir, test.GetLabel())

            run_cmd = GetRunCommand(self.args, replay_exe_path) + [test.GetLabel()]
            self.logger.info('%s %s' % (_FormatEnv(extra_env), ' '.join(run_cmd)))

            returncode, output = child_processes_manager.RunSubprocess(
                run_cmd, env, timeout=REPLAY_SUBPROCESS_TIMEOUT)
            if returncode == -1:
                self.results.append(
                    GroupedResult(GroupedResult.ReplayFailed, "Replay run failed (%s)" % cmd,
                                  output, tests))
                continue
            elif returncode == -2:
                self.results.append(
                    GroupedResult(GroupedResult.TimedOut, "Replay run timed out", output, tests))
                continue

            if args.show_replay_stdout:
                self.logger.info("Replay stdout: %s" % output)

            output_lines = output.splitlines()
            passes = []
            fails = []
            count = 0
            for output_line in output_lines:
                words = output_line.split(" ")
                if len(words) == 3 and words[0] == RESULT_TAG:
                    test_name = self.FindTestByLabel(words[1])
                    result = int(words[2])
                    if result == 0:
                        passes.append(test_name)
                    elif result == REPLAY_INITIALIZATION_FAILURE:
                        fails.append(test_name)
                        self.logger.info("Initialization failure: %s" % test_name)
                    elif result == REPLAY_SERIALIZATION_FAILURE:
                        fails.append(test_name)
                        self.logger.info("Context comparison failed: %s" % test_name)
                        self.PrintContextDiff(replay_build_dir, words[1])
                    else:
                        fails.append(test_name)
                        self.logger.error("Unknown test result code: %s -> %d" %
                                          (test_name, result))
                    count += 1

            if len(passes) > 0:
                self.results.append(GroupedResult(GroupedResult.Passed, "", output, passes))
            if len(fails) > 0:
                self.results.append(GroupedResult(GroupedResult.Failed, "", output, fails))

    def PrintContextDiff(self, replay_build_dir, test_name):
        frame = 1
        found = False
        while True:
            capture_file = "{}/{}_ContextCaptured{}.json".format(replay_build_dir, test_name,
                                                                 frame)
            replay_file = "{}/{}_ContextReplayed{}.json".format(replay_build_dir, test_name, frame)
            if os.path.exists(capture_file) and os.path.exists(replay_file):
                found = True
                captured_context = open(capture_file, "r").readlines()
                replayed_context = open(replay_file, "r").readlines()
                for line in difflib.unified_diff(
                        captured_context, replayed_context, fromfile=capture_file,
                        tofile=replay_file):
                    print(line, end="")
            else:
                if frame > CAPTURE_FRAME_END:
                    break
            frame = frame + 1
        if not found:
            self.logger.error("Could not find serialization diff files for %s" % test_name)

    def FindTestByLabel(self, label):
        for test in self.tests:
            if test.GetLabel() == label:
                return test
        return None

    def AddTest(self, test):
        assert len(self.tests) <= self.args.batch_count
        test.index = len(self.tests)
        self.tests.append(test)

    def CreateTestNamesFile(self, composite_file_id, tests):
        data = {'traces': [test.GetLabel() for test in tests]}
        names_path = os.path.join(self.trace_folder_path, 'test_names_%d.json' % composite_file_id)
        with open(names_path, 'w') as f:
            f.write(json.dumps(data))

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

    def GetResults(self):
        return TestBatchResult(self.batch_index, self.results, self.args.verbose)


class TestExpectation():
    # tests that must not be run as list
    skipped_for_capture_tests = {}
    skipped_for_capture_tests_re = {}

    # test expectations for tests that do not pass
    non_pass_results = {}

    # tests that must run in a one-test batch
    run_single = {}
    run_single_re = {}

    flaky_tests = []

    non_pass_re = {}

    result_map = {
        "FAIL": GroupedResult.Failed,
        "TIMEOUT": GroupedResult.TimedOut,
        "COMPILE_FAIL": GroupedResult.CompileFailed,
        "NOT_RUN": GroupedResult.Skipped,
        "SKIP_FOR_CAPTURE": GroupedResult.Skipped,
        "PASS": GroupedResult.Passed,
    }

    def __init__(self, args):
        expected_results_filename = "capture_replay_expectations.txt"
        expected_results_path = os.path.join(REPLAY_SAMPLE_FOLDER, expected_results_filename)
        self._asan = args.asan
        with open(expected_results_path, "rt") as f:
            for line in f:
                l = line.strip()
                if l != "" and not l.startswith("#"):
                    self.ReadOneExpectation(l, args.debug)

    def _CheckTagsWithConfig(self, tags, config_tags):
        for tag in tags:
            if tag not in config_tags:
                return False
        return True

    def ReadOneExpectation(self, line, is_debug):
        (testpattern, result) = line.split('=')
        (test_info_string, test_name_string) = testpattern.split(':')
        test_name = test_name_string.strip()
        test_info = test_info_string.strip().split()
        result_stripped = result.strip()

        tags = []
        if len(test_info) > 1:
            tags = test_info[1:]

        config_tags = [GetPlatformForSkip()]
        if self._asan:
            config_tags += ['ASAN']
        if is_debug:
            config_tags += ['DEBUG']

        if self._CheckTagsWithConfig(tags, config_tags):
            test_name_regex = re.compile('^' + test_name.replace('*', '.*') + '$')
            if result_stripped == 'COMPILE_FAIL':
                self.run_single[test_name] = self.result_map[result_stripped]
                self.run_single_re[test_name] = test_name_regex
            if result_stripped == 'SKIP_FOR_CAPTURE' or result_stripped == 'TIMEOUT':
                self.skipped_for_capture_tests[test_name] = self.result_map[result_stripped]
                self.skipped_for_capture_tests_re[test_name] = test_name_regex
            elif result_stripped == 'FLAKY':
                self.flaky_tests.append(test_name_regex)
            else:
                self.non_pass_results[test_name] = self.result_map[result_stripped]
                self.non_pass_re[test_name] = test_name_regex

    def TestIsSkippedForCapture(self, test_name):
        return any(p.match(test_name) for p in self.skipped_for_capture_tests_re.values())

    def TestNeedsToRunSingle(self, test_name):
        if any(p.match(test_name) for p in self.run_single_re.values()):
            return True

        return self.TestIsSkippedForCapture(test_name)

    def Filter(self, test_list, run_all_tests):
        result = {}
        for t in test_list:
            for key in self.non_pass_results.keys():
                if self.non_pass_re[key].match(t) is not None:
                    result[t] = self.non_pass_results[key]
            for key in self.run_single.keys():
                if self.run_single_re[key].match(t) is not None:
                    result[t] = self.run_single[key]
            if run_all_tests:
                for [key, r] in self.skipped_for_capture_tests.items():
                    if self.skipped_for_capture_tests_re[key].match(t) is not None:
                        result[t] = r
        return result

    def IsFlaky(self, test_name):
        for flaky in self.flaky_tests:
            if flaky.match(test_name) is not None:
                return True
        return False


def ClearFolderContent(path):
    all_files = []
    for f in os.listdir(path):
        if os.path.isfile(os.path.join(path, f)):
            os.remove(os.path.join(path, f))

def SetCWDToAngleFolder():
    cwd = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
    os.chdir(cwd)
    return cwd


def CleanupAfterReplay(replay_build_dir, test_labels):
    # Remove files that have test labels in the file name, .e.g:
    # ClearTest_ClearIsClamped_ES2_Vulkan_SwiftShader.dll.pdb
    for build_file in os.listdir(replay_build_dir):
        if any(label in build_file for label in test_labels):
            os.unlink(os.path.join(replay_build_dir, build_file))


def RunTests(args, worker_id, job_queue, result_list, message_queue, logger, ninja_lock):
    replay_build_dir = os.path.join(args.out_dir, 'Replay%d' % worker_id)
    replay_exec_path = os.path.join(replay_build_dir, REPLAY_BINARY)

    child_processes_manager = ChildProcessesManager(args, logger, ninja_lock)
    # used to differentiate between multiple composite files when there are multiple test batchs
    # running on the same worker and --deleted_trace is set to False
    composite_file_id = 1
    while not job_queue.empty():
        try:
            test_batch = job_queue.get()
            logger.info('Starting {} ({} tests) on worker {}. Unstarted jobs: {}'.format(
                BatchName(test_batch), len(test_batch.tests), worker_id, job_queue.qsize()))

            test_batch.SetWorkerId(worker_id)

            success = test_batch.RunWithCapture(args, child_processes_manager)
            if not success:
                result_list.append(test_batch.GetResults())
                logger.error('Failed RunWithCapture: %s', str(test_batch.GetResults()))
                continue
            continued_tests = test_batch.RemoveTestsThatDoNotProduceAppropriateTraceFiles()
            if len(continued_tests) == 0:
                result_list.append(test_batch.GetResults())
                logger.info('No tests to replay: %s', str(test_batch.GetResults()))
                continue
            success = test_batch.BuildReplay(replay_build_dir, composite_file_id, continued_tests,
                                             child_processes_manager)
            if args.keep_temp_files:
                composite_file_id += 1
            if not success:
                result_list.append(test_batch.GetResults())
                logger.error('Failed BuildReplay: %s', str(test_batch.GetResults()))
                continue
            test_batch.RunReplay(args, replay_build_dir, replay_exec_path, child_processes_manager,
                                 continued_tests)
            result_list.append(test_batch.GetResults())
            if not args.keep_temp_files:
                CleanupAfterReplay(replay_build_dir, [test.GetLabel() for test in continued_tests])
            logger.info('Finished RunReplay: %s', str(test_batch.GetResults()))
        except KeyboardInterrupt:
            child_processes_manager.KillAll()
            raise
        except queue.Empty:
            child_processes_manager.KillAll()
            break
        except Exception as e:
            logger.error('RunTestsException: %s\n%s' % (repr(e), traceback.format_exc()))
            child_processes_manager.KillAll()
            pass
    message_queue.put(child_processes_manager.runtimes)
    child_processes_manager.KillAll()


def SafeDeleteFolder(folder_name):
    while os.path.isdir(folder_name):
        try:
            shutil.rmtree(folder_name)
        except KeyboardInterrupt:
            raise
        except PermissionError:
            pass


def DeleteReplayBuildFolders(folder_num, replay_build_dir, trace_folder):
    for i in range(folder_num):
        folder_name = replay_build_dir + str(i)
        if os.path.isdir(folder_name):
            SafeDeleteFolder(folder_name)


def CreateTraceFolders(folder_num):
    for i in range(folder_num):
        folder_name = TRACE_FOLDER + str(i)
        folder_path = os.path.join(REPLAY_SAMPLE_FOLDER, folder_name)
        if os.path.isdir(folder_path):
            shutil.rmtree(folder_path)
        os.makedirs(folder_path)


def DeleteTraceFolders(folder_num):
    for i in range(folder_num):
        folder_name = TRACE_FOLDER + str(i)
        folder_path = os.path.join(REPLAY_SAMPLE_FOLDER, folder_name)
        if os.path.isdir(folder_path):
            SafeDeleteFolder(folder_path)


def GetPlatformForSkip():
    # yapf: disable
    # we want each pair on one line
    platform_map = { 'win32' : 'WIN',
                     'linux' : 'LINUX' }
    # yapf: enable
    return platform_map.get(sys.platform, 'UNKNOWN')


def RunInParallel(f, lst, max_workers, stop_event):
    with concurrent.futures.ThreadPoolExecutor(max_workers=max_workers) as executor:
        future_to_arg = {executor.submit(f, arg): arg for arg in lst}
        try:
            for future in concurrent.futures.as_completed(future_to_arg):
                yield future, future_to_arg[future]
        except KeyboardInterrupt:
            stop_event.set()
            raise


def RunProcess(cmd, env, stop_event, timeout):
    stdout = [None]

    def _Reader(process):
        stdout[0] = process.stdout.read().decode()

    process = subprocess.Popen(cmd, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    t = threading.Thread(target=_Reader, args=(process,))
    t.start()
    time_start = time.time()
    while True:
        time.sleep(0.1)
        if process.poll() is not None:
            t.join()
            return process.returncode, stdout[0]
        if time.time() - time_start > timeout:
            process.kill()
            t.join()
            return subprocess.TimeoutExpired, stdout[0]
        if stop_event.is_set():
            process.kill()
            t.join()
            return None, stdout[0]


def RunCaptureInParallel(args, trace_folder_path, test_names, worker_count, skipped_by_suite):
    n = args.batch_count
    test_batches = [test_names[i:i + n] for i in range(0, len(test_names), n)]

    extra_env = GetCaptureEnv(args, trace_folder_path)
    env = {**os.environ.copy(), **extra_env}
    test_exe_path = os.path.join(args.out_dir, 'Capture', args.test_suite)

    stop_event = threading.Event()

    def _RunCapture(tests):
        filt = ':'.join(tests)

        results_file = tempfile.mktemp()
        cmd = [
            test_exe_path,
            '--gtest_filter=%s' % filt,
            '--angle-per-test-capture-label',
            '--results-file=' + results_file,
        ]

        test_results = None
        try:
            rc, stdout = RunProcess(cmd, env, stop_event, CAPTURE_SUBPROCESS_TIMEOUT)
            if rc == 0:
                with open(results_file) as f:
                    test_results = json.load(f)
        finally:
            try:
                os.unlink(results_file)
            except Exception:
                pass

        return rc, test_results, stdout

    capture_failed = False
    for (future, tests) in RunInParallel(_RunCapture, test_batches, worker_count, stop_event):
        rc, test_results, stdout = future.result()

        if rc == subprocess.TimeoutExpired:
            logging.error('Capture failed - timed out after %ss\nTests: %s\nPartial stdout:\n%s',
                          CAPTURE_SUBPROCESS_TIMEOUT, ':'.join(tests), stdout)
            capture_failed = True
            continue

        if rc != 0:
            logging.error('Capture failed.\nTests: %s\nStdout:\n%s\n', ':'.join(tests), stdout)
            capture_failed = True
            continue

        for test_name, res in test_results['tests'].items():
            if res['actual'] == 'SKIP':
                skipped_by_suite.add(test_name)

    return not capture_failed


def RunReplayInParallel(args, replay_build_dir, replay_tests, expected_results, labels_to_tests,
                        worker_count):
    extra_env = {}
    if args.expose_nonconformant_features:
        extra_env['ANGLE_FEATURE_OVERRIDES_ENABLED'] = 'exposeNonConformantExtensionsAndVersions'
    env = {**os.environ.copy(), **extra_env}

    stop_event = threading.Event()

    def _RunReplay(test):
        replay_exe_path = os.path.join(replay_build_dir, REPLAY_BINARY)
        cmd = [replay_exe_path, test.replace(".", "_").replace("/", "_")]
        return RunProcess(cmd, env, stop_event, REPLAY_SUBPROCESS_TIMEOUT)

    tref = time.time()
    logging.info('Tests to replay: %s', len(replay_tests))

    replay_failed = False
    for (future, test) in RunInParallel(_RunReplay, replay_tests, worker_count, stop_event):
        expected_to_pass = expected_results[test] == GroupedResult.Passed
        rc, stdout = future.result()
        if rc == subprocess.TimeoutExpired:
            if expected_to_pass:
                logging.error('Replay failed - timed out after %ss\nTest: %s\nPartial stdout:\n%s',
                              REPLAY_SUBPROCESS_TIMEOUT, test, stdout)
                replay_failed = True
            else:
                logging.info('Ignoring replay timeout due to expectation: %s [expected %s]', test,
                             expected_results[test])
            continue

        if rc != 0:
            if expected_to_pass:
                logging.error('Replay failed.\nTest: %s\nStdout:\n%s\n', test, p.stdout.decode())
                replay_failed = True
            else:
                logging.info('Ignoring replay failure due to expectation: %s [expected %s]', test,
                             expected_results[test])
            continue

        output_lines = stdout.splitlines()
        for output_line in output_lines:
            words = output_line.split(" ")
            if len(words) == 3 and words[0] == RESULT_TAG:
                test_name = labels_to_tests[words[1]]
                result = int(words[2])

                if result == 0:
                    pass
                elif result == REPLAY_INITIALIZATION_FAILURE:
                    if expected_to_pass:
                        replay_failed = True
                        logging.error('Replay failed. Initialization failure: %s' % test_name)
                    else:
                        logging.info(
                            'Ignoring replay failure due to expectation: %s [expected %s]', test,
                            expected_results[test])
                elif result == REPLAY_SERIALIZATION_FAILURE:
                    if expected_to_pass:
                        replay_failed = True
                        logging.error('Replay failed. Context comparison failed: %s' % test_name)
                        #FIXME self.PrintContextDiff(replay_build_dir, words[1])
                    else:
                        logging.info(
                            'Ignoring replay context diff due to expectation: %s [expected %s]',
                            test, expected_results[test])
                else:
                    replay_failed = True
                    logging.error('Replay failed. Unknown result code: %s -> %d' %
                                  (test_name, result))

    logging.info('Replay time: %.1fs', time.time() - tref)

    return not replay_failed


def main(args):
    logger = multiprocessing.log_to_stderr()
    logger.setLevel(level=args.log.upper())

    angle_test_util.SetupLogging(args.log.upper())

    is_bot = getpass.getuser() == 'chrome-bot'

    if is_bot:
        # bots need different re-client auth settings than developers b/319246651
        os.environ["RBE_use_gce_credentials"] = "true"
        os.environ["RBE_use_application_default_credentials"] = "false"
        os.environ["RBE_automatic_auth"] = "false"
        os.environ["RBE_experimental_credentials_helper"] = ""
        os.environ["RBE_experimental_credentials_helper_args"] = ""

    if sys.platform == 'linux' and is_bot:
        logger.warning('Test is currently a no-op https://anglebug.com/42264614')
        return EXIT_SUCCESS

    ninja_lock = multiprocessing.Semaphore(args.max_ninja_jobs)
    child_processes_manager = ChildProcessesManager(args, logger, ninja_lock)

    try:
        worker_count = min(os.cpu_count(), args.max_jobs)

        trace_dir = "%s%d" % (TRACE_FOLDER, 0)
        trace_folder_path = os.path.join(REPLAY_SAMPLE_FOLDER, trace_dir)
        #os.makedirs(trace_folder_path)
        CreateTraceFolders(1)

        capture_build_dir = os.path.normpath(r'%s/Capture' % args.out_dir)

        tref = time.time()

        returncode, output = child_processes_manager.RunGNGen(capture_build_dir, False)
        if returncode != 0:
            logger.error(output)
            return EXIT_FAILURE

        returncode, output = child_processes_manager.RunAutoNinja(capture_build_dir,
                                                                  args.test_suite, False)
        if returncode != 0:
            logger.error(output)
            return EXIT_FAILURE

        logger.info('Capture build time: %.1fs', time.time() - tref)

        test_path = os.path.join(capture_build_dir, args.test_suite)
        test_list = GetTestsListForFilter(args, test_path, args.filter, logger)
        test_expectation = TestExpectation(args)
        test_names = ParseTestNamesFromTestList(test_list, test_expectation,
                                                args.also_run_skipped_for_capture_tests, logger)
        test_expectation_for_list = test_expectation.Filter(
            test_names, args.also_run_skipped_for_capture_tests)

        # FIXME: run_single are COMPILE_FAIL
        test_names = [t for t in test_names if not test_expectation.TestNeedsToRunSingle(t)]

        skipped_by_suite = set()
        if not RunCaptureInParallel(args, trace_folder_path, test_names, worker_count,
                                    skipped_by_suite):
            return EXIT_FAILURE

        logger.info('Capture time: %.1fs', time.time() - tref)

        labels_to_tests = {t.replace(".", "_").replace("/", "_"): t for t in test_names}
        out_files = {t: [] for t in test_names}

        for f in os.listdir(trace_folder_path):
            if f == 'test_names_1.json':  # FIXME
                continue
            label, ext = f.split('.', 1)
            # _001.cpp, _002.cpp etc
            m = re.match(r'(.*)_\d\d\d\.cpp', f)
            if m:
                label = m.group(1)
            assert label in labels_to_tests, label

            out_files[labels_to_tests[label]].append(f)

        replay_tests = []
        for test_name, replay_files in out_files.items():
            if test_name not in skipped_by_suite:
                assert replay_files, 'Test missing replay files: %s' % test_name
                replay_tests.append(test_name)

        composite_file_id = 1
        names_path = os.path.join(trace_folder_path, 'test_names_%d.json' % composite_file_id)
        with open(names_path, 'w') as f:
            f.write(
                json.dumps(
                    {'traces': [t.replace(".", "_").replace("/", "_") for t in replay_tests]}))

        replay_build_dir = os.path.join(args.out_dir, 'Replay%d' % 0)

        tref = time.time()

        gn_args = [('angle_build_capture_replay_tests', 'true'),
                   ('angle_capture_replay_test_trace_dir', '"%s"' % trace_dir),
                   ('angle_capture_replay_composite_file_id', str(composite_file_id))]
        returncode, output = child_processes_manager.RunGNGen(replay_build_dir, True, gn_args)
        if returncode != 0:
            logger.error(output)
            return EXIT_FAILURE
        returncode, output = child_processes_manager.RunAutoNinja(replay_build_dir, REPLAY_BINARY,
                                                                  False)
        if returncode != 0:
            logger.error(output)
            return EXIT_FAILURE

        logger.info('Replay build time: %.1fs', time.time() - tref)

        expected_results = {}
        for test in replay_tests:
            expected_result = test_expectation_for_list.get(test, GroupedResult.Passed)
            if test_expectation.IsFlaky(test):
                expected_result = 'Flaky'
            expected_results[test] = expected_result

        if not RunReplayInParallel(args, replay_build_dir, replay_tests, expected_results,
                                   labels_to_tests, worker_count):
            return EXIT_FAILURE

        if not args.keep_temp_files:
            CleanupAfterReplay(replay_build_dir, list(labels_to_tests.keys()))
    finally:
        child_processes_manager.KillAll()

    return EXIT_SUCCESS

    try:
        # delete generated folders if --keep-temp-files flag is set to false
        if args.purge:
            DeleteTraceFolders(worker_count)
            if os.path.isdir(args.out_dir):
                SafeDeleteFolder(args.out_dir)

        # Try hard to ensure output is finished before ending the test.
        logging.shutdown()
        sys.stdout.flush()
        time.sleep(2.0)
        return retval

    except KeyboardInterrupt:
        child_processes_manager.KillAll()
        return EXIT_FAILURE


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--out-dir',
        default=DEFAULT_OUT_DIR,
        help='Where to build ANGLE for capture and replay. Relative to the ANGLE folder. Default is "%s".'
        % DEFAULT_OUT_DIR)
    parser.add_argument(
        '-f',
        '--filter',
        '--gtest_filter',
        default=DEFAULT_FILTER,
        help='Same as GoogleTest\'s filter argument. Default is "%s".' % DEFAULT_FILTER)
    parser.add_argument(
        '--test-suite',
        default=DEFAULT_TEST_SUITE,
        help='Test suite binary to execute. Default is "%s".' % DEFAULT_TEST_SUITE)
    parser.add_argument(
        '--batch-count',
        default=DEFAULT_BATCH_COUNT,
        type=int,
        help='Number of tests in a batch. Default is %d.' % DEFAULT_BATCH_COUNT)
    parser.add_argument(
        '--keep-temp-files',
        action='store_true',
        help='Whether to keep the temp files and folders. Off by default')
    parser.add_argument('--purge', help='Purge all build directories on exit.')
    parser.add_argument(
        '--use-reclient',
        default=False,
        action='store_true',
        help='Set use_remoteexec=true in args.gn.')
    parser.add_argument(
        '--output-to-file',
        action='store_true',
        help='Whether to write output to a result file. Off by default')
    parser.add_argument(
        '--result-file',
        default=DEFAULT_RESULT_FILE,
        help='Name of the result file in the capture_replay_tests folder. Default is "%s".' %
        DEFAULT_RESULT_FILE)
    parser.add_argument('-v', '--verbose', action='store_true', help='Shows full test output.')
    parser.add_argument(
        '-l',
        '--log',
        default=DEFAULT_LOG_LEVEL,
        help='Controls the logging level. Default is "%s".' % DEFAULT_LOG_LEVEL)
    parser.add_argument(
        '-j',
        '--max-jobs',
        default=DEFAULT_MAX_JOBS,
        type=int,
        help='Maximum number of test processes. Default is %d.' % DEFAULT_MAX_JOBS)
    parser.add_argument(
        '-M',
        '--mec',
        default=0,
        type=int,
        help='Enable mid execution capture starting at specified frame, (default: 0 = normal capture)'
    )
    parser.add_argument(
        '-a',
        '--also-run-skipped-for-capture-tests',
        action='store_true',
        help='Also run tests that are disabled in the expectations by SKIP_FOR_CAPTURE')
    parser.add_argument(
        '--max-ninja-jobs',
        type=int,
        default=DEFAULT_MAX_NINJA_JOBS,
        help='Maximum number of concurrent ninja jobs to run at once.')
    parser.add_argument('--xvfb', action='store_true', help='Run with xvfb.')
    parser.add_argument('--asan', action='store_true', help='Build with ASAN.')
    parser.add_argument(
        '-E',
        '--expose-nonconformant-features',
        action='store_true',
        help='Expose non-conformant features to advertise GLES 3.2')
    parser.add_argument(
        '--show-capture-stdout', action='store_true', help='Print test stdout during capture.')
    parser.add_argument(
        '--show-replay-stdout', action='store_true', help='Print test stdout during replay.')
    parser.add_argument('--debug', action='store_true', help='Debug builds (default is Release).')
    args = parser.parse_args()
    if args.debug and (args.out_dir == DEFAULT_OUT_DIR):
        args.out_dir = args.out_dir + "Debug"

    if sys.platform == "win32":
        args.test_suite += ".exe"
    if args.output_to_file:
        logging.basicConfig(level=args.log.upper(), filename=args.result_file)
    else:
        logging.basicConfig(level=args.log.upper())

    sys.exit(main(args))
