# Copyright (c) 2010 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'variables':
    {
        'angle_build_scripts_path': '..',
    },
    'targets':
    [
        {
            'target_name': 'essl_to_glsl',
            'type': 'executable',
            'includes': [ '../build/common_defines.gypi', ],
            'dependencies': [ '../src/angle.gyp:translator', ],
            'include_dirs': [ '../include', ],
            'sources': [ '<!@(python <(angle_build_scripts_path)/enumerate_files.py translator -types *.cpp *.h)' ],
        },
    ],
    'conditions':
    [
        ['OS=="win"',
        {
            'targets':
            [
                {
                    'target_name': 'essl_to_hlsl',
                    'type': 'executable',
                    'includes': [ '../build/common_defines.gypi', ],
                    'dependencies': [ '../src/angle.gyp:translator', ],
                    'include_dirs':
                    [
                        '../include',
                        '../src',
                    ],
                    'sources':
                    [
                        '<!@(python <(angle_build_scripts_path)/enumerate_files.py translator -types *.cpp *.h)',
                    ],
                },

                {
                    'target_name': 'dds_to_header',
                    'type': 'executable',
                    'includes': [ '../build/common_defines.gypi', ],
                    'sources':
                    [
                        '<!@(python <(angle_build_scripts_path)/enumerate_files.py dds_to_header -types *.cpp)',
                    ],
                },

                {
                    'target_name': 'es_util',
                    'type': 'static_library',
                    'includes': [ '../build/common_defines.gypi', ],
                    'dependencies':
                    [
                        '../src/angle.gyp:libEGL',
                        '../src/angle.gyp:libGLESv2',
                    ],
                    'include_dirs':
                    [
                        'gles2_book/Common',
                        '../include',
                    ],
                    'sources':
                    [
                        '<!@(python <(angle_build_scripts_path)/enumerate_files.py gles2_book/Common -types *.c *.h)'
                    ],
                    'direct_dependent_settings':
                    {
                        'include_dirs':
                        [
                            'gles2_book/Common',
                            '../include',
                        ],
                    },
                },

                {
                    'target_name': 'hello_triangle',
                    'type': 'executable',
                    'includes': [ '../build/common_defines.gypi', ],
                    'dependencies': [ 'es_util' ],
                    'sources': [ '<!@(python <(angle_build_scripts_path)/enumerate_files.py gles2_book/Hello_Triangle -types *.c *.h)' ],
                },

                {
                    'target_name': 'mip_map_2d',
                    'type': 'executable',
                    'includes': [ '../build/common_defines.gypi', ],
                    'dependencies': [ 'es_util' ],
                    'sources': [ '<!@(python <(angle_build_scripts_path)/enumerate_files.py gles2_book/MipMap2D -types *.c *.h)' ],
                },

                {
                    'target_name': 'multi_texture',
                    'type': 'executable',
                    'includes': [ '../build/common_defines.gypi', ],
                    'dependencies': [ 'es_util' ],
                    'sources': [ '<!@(python <(angle_build_scripts_path)/enumerate_files.py gles2_book/MultiTexture -types *.c *.h)' ],
                    'copies':
                    [
                        {
                            'destination': '<(PRODUCT_DIR)',
                            'files': [ '<!@(python <(angle_build_scripts_path)/enumerate_files.py gles2_book/MultiTexture -types *.tga)' ],
                        }
                    ]
                },

                {
                    'target_name': 'particle_system',
                    'type': 'executable',
                    'includes': [ '../build/common_defines.gypi', ],
                    'dependencies': [ 'es_util' ],
                    'sources': [ '<!@(python <(angle_build_scripts_path)/enumerate_files.py gles2_book/ParticleSystem -types *.c *.h)' ],
                    'copies':
                    [
                        {
                            'destination': '<(PRODUCT_DIR)',
                            'files': [ '<!@(python <(angle_build_scripts_path)/enumerate_files.py gles2_book/ParticleSystem -types *.tga)' ],
                        }
                    ]
                },

                {
                    'target_name': 'simple_texture_2d',
                    'type': 'executable',
                    'includes': [ '../build/common_defines.gypi', ],
                    'dependencies': [ 'es_util' ],
                    'sources': [ '<!@(python <(angle_build_scripts_path)/enumerate_files.py gles2_book/Simple_Texture2D -types *.c *.h)' ],
                },

                {
                    'target_name': 'simple_texture_cubemap',
                    'type': 'executable',
                    'includes': [ '../build/common_defines.gypi', ],
                    'dependencies': [ 'es_util' ],
                    'sources': [ '<!@(python <(angle_build_scripts_path)/enumerate_files.py gles2_book/Simple_TextureCubemap -types *.c *.h)' ],
                },

                {
                    'target_name': 'simple_vertex_shader',
                    'type': 'executable',
                    'includes': [ '../build/common_defines.gypi', ],
                    'dependencies': [ 'es_util' ],
                    'sources': [ '<!@(python <(angle_build_scripts_path)/enumerate_files.py gles2_book/Simple_VertexShader -types *.c *.h)' ],
                },

                {
                    'target_name': 'stencil_test',
                    'type': 'executable',
                    'includes': [ '../build/common_defines.gypi', ],
                    'dependencies': [ 'es_util' ],
                    'sources': [ '<!@(python <(angle_build_scripts_path)/enumerate_files.py gles2_book/Stencil_Test -types *.c *.h)' ],
                },

                {
                    'target_name': 'texture_wrap',
                    'type': 'executable',
                    'includes': [ '../build/common_defines.gypi', ],
                    'dependencies': [ 'es_util' ],
                    'sources': [ '<!@(python <(angle_build_scripts_path)/enumerate_files.py gles2_book/TextureWrap -types *.c *.h)' ],
                },

                {
                    'target_name': 'post_sub_buffer',
                    'type': 'executable',
                    'includes': [ '../build/common_defines.gypi', ],
                    'dependencies': [ 'es_util' ],
                    'sources': [ '<!@(python <(angle_build_scripts_path)/enumerate_files.py gles2_book/PostSubBuffer -types *.c *.h)' ],
                },
            ],
        }
        ],
    ],
}
