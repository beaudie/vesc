#!/usr/bin/python
#
# Copyright 2015 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# perf_test_runner.py:
#   Helper script for running and analyzing perftest results. Runs the
#   tests in an infinite batch, printing out the mean and standard
#   deviation of the population continuously.
#

import subprocess
import sys

perftests_path = 'out/Release/angle_perftests.exe'
test_name = 'DrawCallPerfBenchmark.Run/d3d11_null'
metric = 'score'

scores = []

# Danke to http://stackoverflow.com/a/27758326
def mean(data):
    """Return the sample arithmetic mean of data."""
    n = len(data)
    if n < 1:
        raise ValueError('mean requires at least one data point')
    return float(sum(data))/float(n) # in Python 2 use sum(data)/float(n)

def _ss(data):
    """Return sum of square deviations of sequence data."""
    c = mean(data)
    ss = sum((float(x)-c)**2 for x in data)
    return ss

def pstdev(data):
    """Calculates the population standard deviation."""
    n = len(data)
    if n < 2:
        raise ValueError('variance requires at least two data points')
    ss = _ss(data)
    pvar = ss/n # the population variance
    return pvar**0.5

# Infinite loop of running the tests.
while True:
	output = subprocess.check_output([perftests_path, '--gtest_filter=' + test_name])

	start_index = output.find(metric + "=")
	if start_index == -1:
		print("Did not find test output")
		sys.exit(1)

	start_index += len(metric) + 2

	end_index = output[start_index:].find(" ")
	if end_index == -1:
		print("Error parsing output")
		sys.exit(2)

	end_index += start_index

	score = int(output[start_index:end_index])
	sys.stdout.write("score: " + str(score))

	scores.append(score)

	if (len(scores) > 1):
		sys.stdout.write(", mean: " + str(mean(scores)))
		sys.stdout.write(", stddev: " + str(pstdev(scores)))

	print("")
