# Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This .gypi describes all of the sources and dependencies to build a
# unified "angle_unittests" target, which contains all of ANGLE's
# tests that don't require a fully functional ANGLE in order to run
# (compiler tests, preprocessor tests, etc.). It requires a parent
# target to include this gypi in an executable target containing a
# gtest harness in a main.cpp.

{
    'dependencies':
    [
        '<(angle_path)/src/angle.gyp:preprocessor',
        '<(angle_path)/src/angle.gyp:translator_static',
        '<(angle_path)/tests/tests.gyp:angle_test_support',
    ],
    'includes':
    [
        'compiler_tests/compiler_tests.gypi',
        'preprocessor_tests/preprocessor_tests.gypi',
    ],
    'msvs_settings':
    {
        'VCLinkerTool':
        {
            'conditions':
            [
                ['angle_build_winrt==1',
                {
                    'AdditionalDependencies':
                    [
                        'runtimeobject.lib',
                    ],
                }],
            ],
        },
    },
}
