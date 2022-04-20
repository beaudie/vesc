import argparse
import fnmatch
import logging
import os
import re
import sys


def _AddToPathIfNeeded(path):
    if path not in sys.path:
        sys.path.insert(0, path)


_AddToPathIfNeeded(os.path.abspath(os.path.join(os.path.dirname(__file__), 'py_utils')))
import android_helper

logging.basicConfig(level=logging.INFO)

parser = argparse.ArgumentParser()
parser.add_argument('--gtest_filter', type=str)
args, _ = parser.parse_known_args()

adb = android_helper.Adb()

suite = 'angle_perftests'

android_helper.PrepareTestSuite(adb, suite)

tests = android_helper.ListTests(adb)
if args.gtest_filter:
    tests = [test for test in tests if fnmatch.fnmatch(test, args.gtest_filter)]

traces = set()
for test in tests:
    m = re.match(r'TracePerfTest.Run/(native|vulkan)_(.*)', test)
    if m:
        traces.add(m.group(2))
    elif test.startswith('TracePerfTest.Run/'):
        raise Exception("Unexpected test: %s" % test)

android_helper.PrepareRestrictedTraces(adb, traces)
android_helper.RunTests(adb, suite, sys.argv[1:])
