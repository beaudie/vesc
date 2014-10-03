import subprocess
import os
import sys

build_dir = "../build/Release_x64/"
test = "texsubimage"
subprocess.call(build_dir + "angle_perf_tests d3d9 perf_tests/" + test + ".json")
subprocess.call(build_dir + "angle_perf_tests d3d9 perf_tests/" + test + ".json")
subprocess.call(build_dir + "angle_perf_tests d3d9 perf_tests/" + test + ".json")
subprocess.call(build_dir + "angle_perf_tests d3d11 perf_tests/" + test + ".json")
subprocess.call(build_dir + "angle_perf_tests d3d11 perf_tests/" + test + ".json")
subprocess.call(build_dir + "angle_perf_tests d3d11 perf_tests/" + test + ".json")
subprocess.call(build_dir + "angle_perf_tests d3d9 perf_tests/" + test + ".json")
subprocess.call(build_dir + "angle_perf_tests d3d9 perf_tests/" + test + ".json")
subprocess.call(build_dir + "angle_perf_tests d3d9 perf_tests/" + test + ".json")
subprocess.call(build_dir + "angle_perf_tests d3d11 perf_tests/" + test + ".json")
subprocess.call(build_dir + "angle_perf_tests d3d11 perf_tests/" + test + ".json")
subprocess.call(build_dir + "angle_perf_tests d3d11 perf_tests/" + test + ".json")
