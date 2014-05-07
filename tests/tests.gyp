# Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'variables':
    {
        'angle_build_conformance_tests%': '0',
    },
    'targets':
    [
        {
            'target_name': 'gtest',
            'type': 'static_library',
            'includes': [ '../build/common_defines.gypi', ],
            'include_dirs':
            [
                'third_party/googletest',
                'third_party/googletest/include',
            ],
            'sources':
            [
                'third_party/googletest/src/gtest-all.cc',
            ],
            'defines':
            [
                '_VARIADIC_MAX=10',
            ],
            'direct_dependent_settings':
            {
                'defines':
                [
                    '_VARIADIC_MAX=10',
                ],
            },
        },

        {
            'target_name': 'gmock',
            'type': 'static_library',
            'includes': [ '../build/common_defines.gypi', ],
            'include_dirs':
            [
                'third_party/googlemock',
                'third_party/googlemock/include',
                'third_party/googletest/include',
            ],
            'sources':
            [
                'third_party/googlemock/src/gmock-all.cc',
            ],
            'defines':
            [
                '_VARIADIC_MAX=10',
            ],
            'direct_dependent_settings':
            {
                'defines':
                [
                    '_VARIADIC_MAX=10',
                ],
            },
        },

        {
            'target_name': 'preprocessor_tests',
            'type': 'executable',
            'dependencies':
            [
                '../src/angle.gyp:preprocessor',
                'gtest',
                'gmock',
            ],
            'include_dirs':
            [
                '../src/compiler/preprocessor',
                'third_party/googletest/include',
                'third_party/googlemock/include',
            ],
            'includes':
            [
                '../build/common_defines.gypi',
                'preprocessor_tests/preprocessor_tests.gypi',
            ],
            'sources':
            [
                'preprocessor_tests/preprocessor_test_main.cpp',
            ],
        },

        {
            'target_name': 'compiler_tests',
            'type': 'executable',
            'dependencies':
            [
                '../src/angle.gyp:translator_static',
                'gtest',
            ],
            'include_dirs':
            [
                '../include',
                '../src',
                'third_party/googletest/include',
            ],
            'includes':
            [
                '../build/common_defines.gypi',
                'compiler_tests/compiler_tests.gypi',
            ],
            'sources':
            [
                'compiler_tests/compiler_test_main.cpp',
            ],
        },
    ],

    'conditions':
    [
        ['OS=="win"',
        {
            'targets':
            [
                {
                    'target_name': 'angle_tests',
                    'type': 'executable',
                    'includes': [ '../build/common_defines.gypi', ],
                    'dependencies':
                    [
                        '../src/angle.gyp:libGLESv2',
                        '../src/angle.gyp:libEGL',
                        'gtest',
                    ],
                    'include_dirs':
                    [
                        '../include',
                        'angle_tests',
                        'third_party/googletest/include',
                    ],
                    'sources':
                    [
                        '<!@(python <(angle_path)/enumerate_files.py angle_tests -types *.cpp *.h *.inl)'
                    ],
                },
                {
                    'target_name': 'standalone_tests',
                    'type': 'executable',
                    'includes': [ '../build/common_defines.gypi', ],
                    'dependencies':
                    [
                        'gtest',
                        'gmock',
                    ],
                    'include_dirs':
                    [
                        '../include',
                        'angle_tests',
                        'third_party/googletest/include',
                        'third_party/googlemock/include',
                    ],
                    'sources':
                    [
                        '<!@(python <(angle_path)/enumerate_files.py standalone_tests -types *.cpp *.h)'
                    ],
                },
            ],
            'conditions':
            [
                ['angle_build_conformance_tests',
                {
                    'variables':
                    {
                        'es_conformance_tests_output_dir': '<(SHARED_INTERMEDIATE_DIR)/conformance_tests',
                        'es_conformance_tests_input_dir': 'third_party/es_cts/conform/GTF_ES/glsl/GTF',
                        'es_conformance_tests_generator_script': 'conformance_tests/generate_conformance_tests.py',
                    },
                    'targets':
                    [
                        {
                            'target_name': 'es2_conformance_tests',
                            'type': 'executable',
                            'includes': [ '../build/common_defines.gypi', ],
                            'dependencies':
                            [
                                '../src/angle.gyp:libGLESv2',
                                '../src/angle.gyp:libEGL',
                                'gtest',
                                'third_party/es_cts/conform/GTF_ES/glsl/GTF/es_cts.gyp:es_cts_test_data',
                                'third_party/es_cts/conform/GTF_ES/glsl/GTF/es_cts.gyp:es2_cts',
                            ],
                            'variables':
                            {
                                'es2_conformance_tests_input_file': '<(es_conformance_tests_input_dir)/mustpass_es20.run',
                                'es2_conformance_tests_generated_file': '<(es_conformance_tests_output_dir)/generated_es2_conformance_tests.cpp',
                            },
                            'sources':
                            [
                                '<!@(python <(angle_path)/enumerate_files.py conformance_tests -types *.cpp *.h *.inl)',
                                '<(es2_conformance_tests_generated_file)',
                            ],
                            'include_dirs':
                            [
                                '../include',
                                'conformance_tests',
                                'third_party/googletest/include',
                            ],
                            'defines':
                            [
                                'CONFORMANCE_TESTS_TYPE=CONFORMANCE_TESTS_ES2',
                            ],
                            'actions':
                            [
                                {
                                    'action_name': 'generate_es2_conformance_tests',
                                    'message': 'Generating ES2 conformance tests...',
                                    'msvs_cygwin_shell': 0,
                                    'inputs':
                                    [
                                        '<(es_conformance_tests_generator_script)',
                                        '<(es2_conformance_tests_input_file)',
                                    ],
                                    'outputs':
                                    [
                                        '<(es2_conformance_tests_generated_file)',
                                    ],
                                    'action':
                                    [
                                        'python',
                                        '<(es_conformance_tests_generator_script)',
                                        '<(es2_conformance_tests_input_file)',
                                        '<(es_conformance_tests_input_dir)',
                                        '<(es2_conformance_tests_generated_file)',
                                    ],
                                },
                            ],
                        },
                        {
                            'target_name': 'es3_conformance_tests',
                            'type': 'executable',
                            'includes': [ '../build/common_defines.gypi', ],
                            'dependencies':
                            [
                                '../src/angle.gyp:libGLESv2',
                                '../src/angle.gyp:libEGL',
                                'gtest',
                                'third_party/es_cts/conform/GTF_ES/glsl/GTF/es_cts.gyp:es_cts_test_data',
                                'third_party/es_cts/conform/GTF_ES/glsl/GTF/es_cts.gyp:es3_cts',
                            ],
                            'variables':
                            {
                                'es3_conformance_tests_input_file': '<(es_conformance_tests_input_dir)/mustpass_es30.run',
                                'es3_conformance_tests_generated_file': '<(es_conformance_tests_output_dir)/generated_es3_conformance_tests.cpp',
                            },
                            'sources':
                            [
                                '<!@(python <(angle_path)/enumerate_files.py conformance_tests -types *.cpp *.h *.inl)',
                                '<(es3_conformance_tests_generated_file)',
                            ],
                            'include_dirs':
                            [
                                '../include',
                                'conformance_tests',
                                'third_party/googletest/include',
                            ],
                            'defines':
                            [
                                'CONFORMANCE_TESTS_TYPE=CONFORMANCE_TESTS_ES3',
                            ],
                            'actions':
                            [
                                {
                                    'action_name': 'generate_es3_conformance_tests',
                                    'message': 'Generating ES3 conformance tests...',
                                    'msvs_cygwin_shell': 0,
                                    'inputs':
                                    [
                                        '<(es_conformance_tests_generator_script)',
                                        '<(es3_conformance_tests_input_file)',
                                    ],
                                    'outputs':
                                    [
                                        '<(es3_conformance_tests_generated_file)',
                                    ],
                                    'action':
                                    [
                                        'python',
                                        '<(es_conformance_tests_generator_script)',
                                        '<(es3_conformance_tests_input_file)',
                                        '<(es_conformance_tests_input_dir)',
                                        '<(es3_conformance_tests_generated_file)',
                                    ],
                                },
                            ],
                        },
                    ],
                }],
            ],
        }],
    ],
}
