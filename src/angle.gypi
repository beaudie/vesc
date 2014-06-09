# Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'variables':
    {
        'angle_code': 1,
        'angle_post_build_script%': 0,
        'angle_commit_id_script': 'commit_id.py',
        'angle_use_commit_id%': '<!(python <(angle_commit_id_script) check .)'
    },
    'includes':
    [
        'compiler.gypi',
        'libGLESv2.gypi',
        'libEGL.gypi'
    ],

    'targets':
    [
        {
            'target_name': 'copy_scripts',
            'type': 'none',
            'copies':
            [
                {
                    'destination': '<(SHARED_INTERMEDIATE_DIR)',
                    'files': [ 'copy_compiler_dll.bat', '<(angle_commit_id_script)' ],
                },
            ],
        },
    ],
    'conditions':
    [
        ['angle_use_commit_id!=0',
        {
            'targets':
            [
                {
                    'target_name': 'commit_id',
                    'type': 'none',
                    'includes': [ '../build/common_defines.gypi', ],
                    'dependencies': [ 'copy_scripts', ],
                    'actions':
                    [
                        {
                            'action_name': 'Generate ANGLE Commit ID Header',
                            'message': 'Generating ANGLE Commit ID',
                            'inputs': [ '<(SHARED_INTERMEDIATE_DIR)/<(angle_commit_id_script)', '<(angle_path)/.git/index' ],
                            'outputs': [ '<(SHARED_INTERMEDIATE_DIR)/angle_commit.h' ],
                            'msvs_cygwin_shell': 0,
                            'action': [ 'python', '<(SHARED_INTERMEDIATE_DIR)/<(angle_commit_id_script)', 'gen', '<(angle_path)', '<(SHARED_INTERMEDIATE_DIR)/angle_commit.h' ],
                        },
                    ],
                    'direct_dependent_settings':
                    {
                        'include_dirs':
                        [
                            '<(SHARED_INTERMEDIATE_DIR)',
                        ],
                    },
                }
            ]
        },
        { # angle_use_commit_id==0
            'targets':
            [
                {
                    'target_name': 'commit_id',
                    'type': 'none',
                    'copies':
                    [
                        {
                            'destination': '<(SHARED_INTERMEDIATE_DIR)',
                            'files': ['angle_commit.h']
                        }
                    ],
                }
            ]
        }],
        ['OS=="win"',
        {
            'targets':
            [
                {
                    'target_name': 'copy_compiler_dll',
                    'type': 'none',
                    'dependencies': [ 'copy_scripts', ],
                    'includes': [ '../build/common_defines.gypi', ],
                    'actions':
                    [
                        {
                            'action_name': 'copy_dll',
                            'message': 'Copying D3D Compiler DLL...',
                            'msvs_cygwin_shell': 0,
                            'inputs': [ 'copy_compiler_dll.bat' ],
                            'outputs': [ '<(PRODUCT_DIR)/D3DCompiler_46.dll' ],
                            'action':
                            [
                                "<(SHARED_INTERMEDIATE_DIR)/copy_compiler_dll.bat",
                                "$(PlatformName)",
                                "<(windows_sdk_path)",
                                "<(PRODUCT_DIR)"
                            ],
                        },
                    ], #actions
                },
            ], # targets
        }],
        ['angle_post_build_script!=0 and OS=="win"',
        {
            'targets':
            [
                {
                    'target_name': 'post_build',
                    'type': 'none',
                    'includes': [ '../build/common_defines.gypi', ],
                    'dependencies': [ 'libGLESv2', 'libEGL' ],
                    'actions':
                    [
                        {
                            'action_name': 'ANGLE Post-Build Script',
                            'message': 'Running <(angle_post_build_script)...',
                            'msvs_cygwin_shell': 0,
                            'inputs': [ '<(angle_post_build_script)', '<!@(["python", "<(angle_post_build_script)", "inputs", "<(angle_path)", "<(CONFIGURATION_NAME)", "$(PlatformName)", "<(PRODUCT_DIR)"])' ],
                            'outputs': [ '<!@(python <(angle_post_build_script) outputs "<(angle_path)" "<(CONFIGURATION_NAME)" "$(PlatformName)" "<(PRODUCT_DIR)")' ],
                            'action': ['python', '<(angle_post_build_script)', 'run', '<(angle_path)', '<(CONFIGURATION_NAME)', '$(PlatformName)', '<(PRODUCT_DIR)'],
                        },
                    ], #actions
                },
            ], # targets
        }],
    ] # conditions
}
