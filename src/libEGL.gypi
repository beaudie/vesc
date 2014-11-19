# Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    # Everything below this is duplicated in the GN build. If you change
    # anything also change angle/BUILD.gn
    'conditions':
    [
        ['OS=="win"',
        {
            'targets':
            [
                {
                    'target_name': 'libEGL',
                    'type': 'shared_library',
                    'dependencies': [ 'libGLESv2', 'libANGLE', 'commit_id' ],
                    'include_dirs':
                    [
                        '.',
                        '../include',
                        'libANGLE',
                        'libEGL',
                        'libGLESv2',
                    ],
                    'sources':
                    [
                        'libEGL/libEGL.cpp',
                        'libEGL/libEGL.def',
                        'libEGL/libEGL.rc',
                        'libEGL/main.cpp',
                        'libEGL/main.h',
                        'libEGL/resource.h',
                    ],
                    'defines':
                    [
                        'GL_APICALL=',
                        'GL_GLEXT_PROTOTYPES=',
                        'EGLAPI=',
                    ],
                    'conditions':
                    [
                        ['angle_enable_d3d9==1',
                        {
                            'defines':
                            [
                                'ANGLE_ENABLE_D3D9',
                            ],
                        }],
                        ['angle_enable_d3d11==1',
                        {
                            'defines':
                            [
                                'ANGLE_ENABLE_D3D11',
                            ],
                        }],

                        ['angle_build_winrt==1',
                        {
                            'defines':
                            [
                                'NTDDI_VERSION=NTDDI_WINBLUE',
                            ],
                            'msvs_enable_winrt' : '1',
                            'msvs_requires_importlibrary' : '1',
                            'msvs_settings':
                            {
                                'VCLinkerTool':
                                {
                                    'EnableCOMDATFolding': '1',
                                    'OptimizeReferences': '1',
                                }
                            },
                        }],
                        ['angle_build_winphone==1',
                        {
                            'msvs_enable_winphone' : '1',
                        }],
                    ],
                    'includes': [ '../build/common_defines.gypi', ],
                    'msvs_settings':
                    {
                        'VCLinkerTool':
                        {
                            'conditions':
                            [
                                ['angle_build_winrt==0',
                                {
                                    'AdditionalDependencies':
                                    [
                                        'd3d9.lib',
                                    ],
                                }],
                                ['angle_build_winrt==1',
                                {
                                    'AdditionalDependencies':
                                    [
                                        'd3d11.lib',
                                    ],
                                }],
                            ],
                        },
                    },
                },
            ],
        },
        ],
    ],
}
