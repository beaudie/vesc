# Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This .gypi describes all of the sources and dependencies to build a
# unified "angle_end2end_tests" target, which contains all of the
# tests that exercise the ANGLE implementation. It requires a parent
# target to include this gypi in an executable target containing a
# gtest harness in a main.cpp.

{
    'dependencies':
    [
        '<(angle_path)/src/angle.gyp:libANGLE',
        '<(angle_path)/src/angle.gyp:libEGL',
        '<(angle_path)/src/angle.gyp:libGLESv2',
        '<(angle_path)/tests/tests.gyp:angle_test_support',
        '<(angle_path)/util/util.gyp:angle_util',
    ],
    'include_dirs':
    [
        '../include',
        'angle_tests',
    ],
    'includes':
    [
        # TODO(kbr): move these to angle_unittests.gypi.
        'angle_implementation_unit_tests/angle_implementation_unit_tests.gypi',
    ],
    'sources':
    [
        '<!@(python <(angle_path)/enumerate_files.py <(angle_path)/tests/angle_tests <(angle_path)/tests/standalone_tests -types *.cpp *.h *.inl -excludes <(angle_path)/tests/angle_tests/angle_test_main.cpp <(angle_path)/tests/standalone_tests/standalone_test_main.cpp)',
    ],
}
