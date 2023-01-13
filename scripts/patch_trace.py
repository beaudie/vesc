import argparse
import os
import shutil
import sys


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--path', type=str)

    args, extra_flags = parser.parse_known_args()
    assert os.path.exists(args.path)
    orig_file = args.path + '.orig'
    if not os.path.exists(orig_file):
        shutil.copyfile(args.path, orig_file)

    with open(orig_file, 'r') as f:
        lines = f.readlines()

    with open(args.path, 'w') as fo:
        state = 0
        idx = 0
        for l in lines:
            fo.write(l)
            if state == 0 and 'void ReplayContext3Frame1()' in l:
                state = 1
            if state == 1 and l.startswith('{'):
                state = 2
            if state == 2 and l.startswith('}'):
                state = 3

            if state == 2 and l.strip().startswith('glDraw'):
                fo.write('    if (CheckIdx(%d)) return;\n\n' % idx)
                idx += 1

    print('last idx:', idx - 1)


if __name__ == '__main__':
    sys.exit(main())
