import argparse
import json
import logging
import os
import pathlib
import sys

PY_UTILS = str(pathlib.Path(__file__).resolve().parent / 'py_utils')
if PY_UTILS not in sys.path:
    os.stat(PY_UTILS) and sys.path.insert(0, PY_UTILS)
import android_helper


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--suite',
        help='Test suite binary.',
        choices=['angle_system_info_test'])
    parser.add_argument('-l', '--log', help='Logging level.', default='warn')
    parser.add_argument('--output-directory')
    parser.add_argument('--wrapper-script-args')
    parser.add_argument('--runtime-deps-path')

    args, extra_args = parser.parse_known_args(sys.argv[2:])

    logging.basicConfig(level=args.log.upper())

    os.chdir(args.output_directory)

    android_helper.Initialize(args.suite)
    assert android_helper.IsAndroid()

    if args.suite == 'angle_system_info_test':
        print(json.dumps(android_helper.AngleSystemInfo(extra_args), indent=2))


if __name__ == "__main__":
    sys.exit(main())
