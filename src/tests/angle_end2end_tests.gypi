# Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This .gypi describes all of the sources and dependencies to build a
# unified "angle_end2end_tests" target, which contains all of the
# tests that exercise the ANGLE implementation. It requires a parent
# target to include this gypi in an executable target containing a
# gtest harness in a main.cpp.

{
    'variables':
    {
        # This file list will be shared with the GN build.
        'angle_end2end_tests_sources':
        [
            'gl_tests/BlendMinMaxTest.cpp',
            'gl_tests/BlitFramebufferANGLETest.cpp',
            'gl_tests/BufferDataTest.cpp',
            'gl_tests/ClearTest.cpp',
            'gl_tests/CompressedTextureTest.cpp',
            'gl_tests/CubeMapTextureTest.cpp',
            'gl_tests/DepthStencilFormatsTest.cpp',
            'gl_tests/DrawBuffersTest.cpp',
            'gl_tests/FenceSyncTests.cpp',
            'gl_tests/FramebufferFormatsTest.cpp',
            'gl_tests/FramebufferRenderMipmapTest.cpp',
            'gl_tests/GLSLTest.cpp',
            'gl_tests/IncompleteTextureTest.cpp',
            'gl_tests/IndexedPointsTest.cpp',
            'gl_tests/InstancingTest.cpp',
            'gl_tests/LineLoopTest.cpp',
            'gl_tests/MaxTextureSizeTest.cpp',
            'gl_tests/MipmapTest.cpp',
            'gl_tests/media/pixel.inl',
            'gl_tests/PBOExtensionTest.cpp',
            'gl_tests/PointSpritesTest.cpp',
            'gl_tests/ProgramBinaryTest.cpp',
            'gl_tests/ReadPixelsTest.cpp',
            'gl_tests/RendererTest.cpp',
            'gl_tests/SimpleOperationTest.cpp',
            'gl_tests/SRGBTextureTest.cpp',
            'gl_tests/SwizzleTest.cpp',
            'gl_tests/TextureTest.cpp',
            'gl_tests/TransformFeedbackTest.cpp',
            'gl_tests/UniformBufferTest.cpp',
            'gl_tests/UniformTest.cpp',
            'gl_tests/UnpackAlignmentTest.cpp',
            'gl_tests/UnpackRowLength.cpp',
            'gl_tests/VertexAttributeTest.cpp',
            'gl_tests/ViewportTest.cpp',
            'egl_tests/EGLQueryContextTest.cpp',
            'test_utils/ANGLETest.cpp',
            'test_utils/ANGLETest.h',
            'test_utils/angle_test_configs.h',
        ],
        'angle_end2end_tests_win_sources':
        [
            # TODO(cwallez) for Linux, requires a portable implementation of sleep
            'gl_tests/OcclusionQueriesTest.cpp',
            # TODO(cwallez) for Linux, requires implementation of eglBindTexImage for pbuffers
            'gl_tests/PbufferTest.cpp',
            'gl_tests/QueryDisplayAttribTest.cpp',
            # TODO(cwallez) for Linux, remove the reliance on the ANGLE_platform_angle_d3d extension
            'egl_tests/EGLSurfaceTest.cpp',
            # TODO(cwallez) for Linux, requires a portable implementation of threads
            'egl_tests/EGLThreadTest.cpp',
        ],
    },
    'dependencies':
    [
        '<(angle_path)/src/angle.gyp:libANGLE',
        '<(angle_path)/src/angle.gyp:libEGL',
        '<(angle_path)/src/angle.gyp:libGLESv2',
        '<(angle_path)/src/tests/tests.gyp:angle_test_support',
        '<(angle_path)/util/util.gyp:angle_util',
    ],
    'include_dirs':
    [
        '<(angle_path)/include',
        '<(angle_path)/src/tests'
    ],
    'sources':
    [
        '<@(angle_end2end_tests_sources)',
    ],
    'conditions':
    [
        ['OS=="win"',
        {
            'sources':
            [
                '<@(angle_end2end_tests_win_sources)',
            ],
        }],
    ]
}
