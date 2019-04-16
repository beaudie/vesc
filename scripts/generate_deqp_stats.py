#!/usr/bin/python2
#
# Copyright 2019 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# generate_deqp_stats.py:
#   Checks output of deqp testers and generates stats using the GDocs API

import argparse
import json
import os
import subprocess
import sys

script_dir = sys.path[0]
root_dir = os.path.abspath(os.path.join(script_dir, '..'))

INFO_KEYS = ['Total', 'Passed', 'Failed', 'Skipped', 'Not Supported', 'Exception', 'Crashed']

# Returns the build name of the latest successful build given a bot name
# bb ls '<botname>' -n 1 -status success | awk -F "'" 'NR==1{print $2}'
def get_latest_success_build_name(bot_name):
    bb = subprocess.Popen(['bb', 'ls', bot_name, '-n', '1', '-status', 'success'], stdout=subprocess.PIPE)
    awk = subprocess.Popen(['awk', '-F', "'", "NR==1{print $2}"], stdin=bb.stdout, stdout=subprocess.PIPE)
    bb.stdout.close()
    build_name = awk.stdout.read().splitlines()[0]
    awk.stdout.close()
    return build_name

# Returns a list of step names given a build name
# bb get '<build_name>' -steps | grep 'Step "angle_' | awk -F '"' '{print $2}'
def get_step_names(build_name):
    bb = subprocess.Popen(['bb', 'get', build_name, '-steps'], stdout=subprocess.PIPE)
    grep = subprocess.Popen(['grep', 'Step "angle_'], stdin=bb.stdout, stdout=subprocess.PIPE)
    bb.stdout.close()
    awk = subprocess.Popen(['awk', '-F', '"', '{print $2}'], stdin=grep.stdout, stdout=subprocess.PIPE)
    grep.stdout.close()
    step_names  = awk.stdout.read().splitlines()
    awk.stdout.close()
    return step_names


def validate_step_info(step_info, build_name, step_name):
    computed_total = step_info['Passed'] + step_info['Failed'] + step_info['Skipped'] + step_info['Not Supported'] + step_info['Exception'] + step_info['Crashed']
    if step_info['Total'] != computed_total:
        sys.stderr.write("Step info does not sum to total for '" + build_name + "', '" + step_name + "'. Total: " + str(step_info['Total']) + " - Computed total: " + str(computed_total))

# Get a dictionary with INFO_KEYS keys, containing the total number of tests per category for a
# given build_name and step_name.
# bb log '<build_name>' '<step_name>' | grep <info_keys>
# Split each line on key and number of tests
def get_step_info(build_name, step_name):
    bb = subprocess.Popen(['bb', 'log', build_name, step_name], stdout=subprocess.PIPE)
    step_info = {}
    greparg = '\|'.join(INFO_KEYS)
    grep = subprocess.Popen(['grep', greparg], stdin=bb.stdout, stdout=subprocess.PIPE)
    bb.stdout.close()
    step_info_str = grep.stdout.read()
    grep.stdout.close()
    for key in INFO_KEYS:
        step_info[key] = 0

    for line in step_info_str.splitlines():
        line_columns = line.split(":")
        if len(line_columns) is not 2:
            sys.stderr.write("Line improperly formatted: '" + line + "'")
            continue
        key = line_columns[0].strip()
        if key not in INFO_KEYS:
            sys.stderr.write("Invalid key: '" + line + "'")
            continue
        val = int(filter(str.isdigit, line_columns[1]))
        if val is None:
            sys.stderr.write("Invalid value: '" + line + "'")
            continue
        step_info[key] += val

    validate_step_info(step_info, build_name, step_name)
    return step_info


def parse_args():
    parser = argparse.ArgumentParser(os.path.basename(sys.argv[0]))
    parser.add_argument('bot_name', help='name of the bot to parse')
    return parser.parse_args()

def main():
    os.chdir(script_dir)
    args = parse_args()
    info = {}
    build_name = get_latest_success_build_name(args.bot_name)
    info[build_name] = {}
    step_names = get_step_names(build_name)
    for step_name in step_names:
        info[build_name][step_name] = step_info = get_step_info(build_name, step_name)

    print info

if __name__ == '__main__':
    sys.exit(main())
