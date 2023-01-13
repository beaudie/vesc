import os
import pathlib
import subprocess
import sys

PY_UTILS = str(pathlib.Path(__file__).resolve().parents[1] / 'src' / 'tests' / 'py_utils')
if PY_UTILS not in sys.path:
    os.stat(PY_UTILS) and sys.path.insert(0, PY_UTILS)
import android_helper


def _run(n):
    temp_dir = '/tmp/bisect_trace'
    flags = [
        '--gtest_filter=*family_island', '--save-screenshots',
        '--render-test-output-dir=%s' % temp_dir, '--steps-per-trial=1', '--trials-per-sample=1',
        '--samples-per-test=1'
    ]
    flags_native = flags + ['--use-gl=native', '--my-idx=%d' % n]
    assert android_helper.RunTests('angle_trace_tests', flags_n, log_output=False)[0] == 0
    flags_angle = flags + ['--use-angle=vulkan', '--my-idx=%d' % n]
    assert android_helper.RunTests('angle_trace_tests', flags_a, log_output=False)[0] == 0
    compare_cmd = [
        'compare', '-metric', 'rmse',
        os.path.join(temp_dir, 'angle_native_family_island.png'),
        os.path.join(temp_dir, 'angle_vulkan_family_island.png'), 'NULL:'
    ]
    p = subprocess.run(compare_cmd, capture_output=True)
    return float(p.stderr.decode().split(' ')[1].strip('()'))


def main():
    android_helper.Initialize('angle_trace_tests')
    assert android_helper.IsAndroid()

    left, right = 0, 117
    while left < right:
        n = (left + right) // 2
        diff = _run(n)
        print(n, diff)
        if diff < 0.01:
            left = n + 1
        else:
            right = n

    _run(left)
    print(left)


if __name__ == '__main__':
    sys.exit(main())
