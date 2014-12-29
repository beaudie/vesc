# Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'variables':
    {
        'angle_build_conformance_tests%': '0',
        'angle_build_deqp_tests%': '0',
        # build/gyp_angle sets this to 1.
        'angle_standalone%': 0,
        'deqp_dir': 'third_party/deqp',
        'delibs_dir': 'third_party/deqp/framework/delibs',
        'libpng_dir': 'third_party/libpng',
        'zlib_dir': 'third_party/zlib',
        'deqp_msvs_compiler_opts':
        [
            '/EHsc',   # dEQP requires exceptions
            '/wd4100',
            '/wd4127', # conditional expression constant
            '/wd4244', # possible loss of data
            '/wd4245', # argument signed/unsigned mismatch
            '/wd4389', # signed/unsigned mismatch
            '/wd4510', # default constructor could not be generated
            '/wd4512',
            '/wd4610', # cannot be instantiated
            '/wd4611', # setjmp interaction non-portable
            '/wd4701', # potentially uninit used
            '/wd4702', # unreachable code
            '/wd4706',
            '/wd4996', # deprecated
        ],
        'deqp_defines':
        [
            'DEQP_SUPPORT_GLES3=1',
            'DEQP_SUPPORT_GLES2=1',
            'DEQP_SUPPORT_EGL=1',
            'DEQP_TARGET_NAME="angle"',
            'DEQP_GLES3_RUNTIME_LOAD=1',
            'DEQP_GLES2_RUNTIME_LOAD=1',
            'QP_SUPPORT_PNG=1',
        ],
        'deqp_include_dirs':
        [
            '../include',
            'third_party/libpng',
            'third_party/zlib',
            '<(delibs_dir)/debase',
            '<(delibs_dir)/decpp',
            '<(delibs_dir)/depool',
            '<(delibs_dir)/dethread',
            '<(delibs_dir)/deutil',
            '<(delibs_dir)/destream',
            '<(deqp_dir)/framework/common',
            '<(deqp_dir)/framework/qphelper',
            # TODO(jmadill): other platforms
            '<(deqp_dir)/framework/platform/win32',
            '<(deqp_dir)/framework/egl',
            '<(deqp_dir)/framework/opengl',
            '<(deqp_dir)/framework/opengl/wrapper',
            '<(deqp_dir)/framework/referencerenderer',
            '<(deqp_dir)/framework/opengl/simplereference',
            '<(deqp_dir)/framework/randomshaders',
            '<(deqp_dir)/modules/gles3',
            '<(deqp_dir)/modules/gles3/functional',
            '<(deqp_dir)/modules/gles3/accuracy',
            '<(deqp_dir)/modules/gles3/performance',
            '<(deqp_dir)/modules/gles3/stress',
            '<(deqp_dir)/modules/gles3/usecases',
            '<(deqp_dir)/modules/gles31',
            '<(deqp_dir)/modules/gles31/functional',
            '<(deqp_dir)/modules/gles31/stress',
            '<(deqp_dir)/modules/glshared',
            '<(deqp_dir)/modules/glusecases',
            '<(deqp_dir)/executor',
            '<(deqp_dir)/execserver',
        ],
    },
    'targets':
    [
        {
            'target_name': 'angle_internal_gtest',
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
            'all_dependent_settings':
            {
                'defines':
                [
                    '_VARIADIC_MAX=10',
                ],
                'include_dirs':
                [
                    'third_party/googletest',
                    'third_party/googletest/include',
                ],
            },
        },

        {
            'target_name': 'angle_internal_gmock',
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
            'all_dependent_settings':
            {
                'defines':
                [
                    '_VARIADIC_MAX=10',
                ],
                'include_dirs':
                [
                    'third_party/googlemock',
                    'third_party/googlemock/include',
                    'third_party/googletest/include',
                ],
            },
        },
        {
            'target_name': 'angle_test_support',
            'type': 'none',
            'conditions':
            [
                ['angle_standalone==1',
                {
                    'dependencies': [
                        'angle_internal_gmock',
                        'angle_internal_gtest',
                    ],
                },
                {
                    'dependencies': [
                        '<(DEPTH)/testing/gmock.gyp:gmock',
                        '<(DEPTH)/testing/gtest.gyp:gtest',
                    ],
                    'all_dependent_settings':
                    {
                        'include_dirs':
                        [
                            '<(DEPTH)/testing/gmock/include',
                            '<(DEPTH)/testing/gtest/include',
                        ],
                    },
                }],
            ],
        },
    ],

    'conditions':
    [
        ['angle_standalone==1',
        {
            # These same target names exist on the Chromium side,
            # which is forbidden, so we make them conditional on
            # ANGLE's standalone build.
            'targets':
            [
                {
                    'target_name': 'angle_unittests',
                    'type': 'executable',
                    'includes':
                    [
                        '../build/common_defines.gypi',
                        'angle_unittests.gypi',
                    ],
                    'sources':
                    [
                        'angle_unittests_main.cpp',
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
                },
            ],
            'conditions':
            [
                ['OS=="win"',
                {
                    'targets':
                    [
                        {
                            'target_name': 'angle_end2end_tests',
                            'type': 'executable',
                            'includes':
                            [
                                '../build/common_defines.gypi',
                                'angle_end2end_tests.gypi',
                            ],
                            'sources':
                            [
                                'angle_end2end_tests_main.cpp',
                            ],
                        },
                    ],
                }],
            ],
        }],
        ['OS=="win"',
        {
            'targets':
            [
                {
                    'target_name': 'angle_zlib',
                    'type': 'static_library',
                    'includes': [ '../build/common_defines.gypi', ],
                    'include_dirs':
                    [
                        '<(zlib_dir)',
                    ],
                    'direct_dependent_settings':
                    {
                        'include_dirs':
                        [
                            '<(zlib_dir)',
                        ],
                    },
                    'msvs_settings':
                    {
                        'VCCLCompilerTool':
                        {
                            'AdditionalOptions':
                            [
                                '/wd4131', # old-style declarator
                                '/wd4324', # structure was padded
                                '/wd4701', # potentially uninit used
                                '/wd4996', # deprecated
                            ],
                        },
                    },
                    'sources':
                    [
                        '<(zlib_dir)/adler32.c',
                        '<(zlib_dir)/compress.c',
                        '<(zlib_dir)/crc32.c',
                        '<(zlib_dir)/crc32.h',
                        '<(zlib_dir)/deflate.c',
                        '<(zlib_dir)/deflate.h',
                        '<(zlib_dir)/gzclose.c',
                        '<(zlib_dir)/gzguts.h',
                        '<(zlib_dir)/gzlib.c',
                        '<(zlib_dir)/gzread.c',
                        '<(zlib_dir)/gzwrite.c',
                        '<(zlib_dir)/infback.c',
                        '<(zlib_dir)/inffast.c',
                        '<(zlib_dir)/inffast.h',
                        '<(zlib_dir)/inffixed.h',
                        '<(zlib_dir)/inflate.c',
                        '<(zlib_dir)/inflate.h',
                        '<(zlib_dir)/inftrees.c',
                        '<(zlib_dir)/inftrees.h',
                        '<(zlib_dir)/mozzconf.h',
                        '<(zlib_dir)/trees.c',
                        '<(zlib_dir)/trees.h',
                        '<(zlib_dir)/uncompr.c',
                        '<(zlib_dir)/x86.h',
                        '<(zlib_dir)/zconf.h',
                        '<(zlib_dir)/zlib.h',
                        '<(zlib_dir)/zutil.c',
                        '<(zlib_dir)/zutil.h',
                        '<(zlib_dir)/simd_stub.c',
                    ],
                },

                {
                    'target_name': 'angle_libpng',
                    'type': 'static_library',
                    'includes': [ '../build/common_defines.gypi', ],
                    'dependencies':
                    [
                        'angle_zlib'
                    ],
                    'msvs_settings':
                    {
                        'VCCLCompilerTool':
                        {
                            'AdditionalOptions':
                            [
                                '/wd4018', # signed/unsigned mismatch
                                '/wd4028', # parameter differs from decl
                                '/wd4101', # unreferenced local
                                '/wd4189', # unreferenced but initted
                            ],
                        },
                    },
                    'sources':
                    [
                        '<(libpng_dir)/png.c',
                        '<(libpng_dir)/pngerror.c',
                        '<(libpng_dir)/pngget.c',
                        '<(libpng_dir)/pngmem.c',
                        '<(libpng_dir)/pngpread.c',
                        '<(libpng_dir)/pngread.c',
                        '<(libpng_dir)/pngrio.c',
                        '<(libpng_dir)/pngrtran.c',
                        '<(libpng_dir)/pngrutil.c',
                        '<(libpng_dir)/pngset.c',
                        '<(libpng_dir)/pngtrans.c',
                        '<(libpng_dir)/pngwio.c',
                        '<(libpng_dir)/pngwrite.c',
                        '<(libpng_dir)/pngwtran.c',
                        '<(libpng_dir)/pngwutil.c',
                    ],
                },

                {
                    'target_name': 'angle_deqp_decpp',
                    'type': 'static_library',
                    'msvs_settings':
                    {
                        'VCCLCompilerTool':
                        {
                            'AdditionalOptions': ['<@(deqp_msvs_compiler_opts)'],
                        },
                    },
                    'defines': ['<@(deqp_defines)'],
                    'include_dirs': ['<@(deqp_include_dirs)'],
                    'sources':
                    [
                        '<(deqp_dir)/framework/delibs/decpp/deArrayBuffer.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deBlockBuffer.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deCommandLine.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deDefs.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deDirectoryIterator.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deDynamicLibrary.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deFilePath.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deMemPool.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deMutex.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/dePoolArray.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/dePoolString.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deProcess.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deRandom.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deRingBuffer.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deSemaphore.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deSharedPtr.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deSocket.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deSTLUtil.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deStringUtil.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deThread.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deThreadLocal.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deThreadSafeRingBuffer.cpp',
                        '<(deqp_dir)/framework/delibs/decpp/deUniquePtr.cpp',
                    ],
                },

                {
                    'target_name': 'angle_deqp_libtester',
                    'type': 'shared_library',
                    'dependencies':
                    [
                        'angle_deqp_decpp',
                        'angle_libpng',
                        '../src/angle.gyp:libEGL',
                    ],
                    'defines': ['<@(deqp_defines)'],
                    'include_dirs': ['<@(deqp_include_dirs)'],
                    'msvs_settings':
                    {
                        'VCCLCompilerTool':
                        {
                            'AdditionalOptions': ['<@(deqp_msvs_compiler_opts)'],
                        },
                        'VCLinkerTool':
                        {
                            'AdditionalDependencies':
                            [
                                'dbghelp.lib',
                                'gdi32.lib',
                                'user32.lib',
                                'ws2_32.lib',
                            ],
                        },
                    },
                    'sources':
                    [
                        '<(deqp_dir)/execserver/xsDefs.cpp',
                        '<(deqp_dir)/execserver/xsExecutionServer.cpp',
                        '<(deqp_dir)/execserver/xsPosixFileReader.cpp',
                        '<(deqp_dir)/execserver/xsPosixTestProcess.cpp',
                        '<(deqp_dir)/execserver/xsProtocol.cpp',
                        '<(deqp_dir)/execserver/xsTcpServer.cpp',
                        '<(deqp_dir)/execserver/xsTestDriver.cpp',
                        '<(deqp_dir)/execserver/xsTestProcess.cpp',
                        '<(deqp_dir)/executor/xeBatchExecutor.cpp',
                        '<(deqp_dir)/executor/xeBatchResult.cpp',
                        '<(deqp_dir)/executor/xeCallQueue.cpp',
                        '<(deqp_dir)/executor/xeCommLink.cpp',
                        '<(deqp_dir)/executor/xeContainerFormatParser.cpp',
                        '<(deqp_dir)/executor/xeDefs.cpp',
                        '<(deqp_dir)/executor/xeLocalTcpIpLink.cpp',
                        '<(deqp_dir)/executor/xeTcpIpLink.cpp',
                        '<(deqp_dir)/executor/xeTestCase.cpp',
                        '<(deqp_dir)/executor/xeTestCaseListParser.cpp',
                        '<(deqp_dir)/executor/xeTestCaseResult.cpp',
                        '<(deqp_dir)/executor/xeTestLogParser.cpp',
                        '<(deqp_dir)/executor/xeTestLogWriter.cpp',
                        '<(deqp_dir)/executor/xeTestResultParser.cpp',
                        '<(deqp_dir)/executor/xeXMLParser.cpp',
                        '<(deqp_dir)/executor/xeXMLWriter.cpp',
                        '<(deqp_dir)/framework/common/tcuApp.cpp',
                        '<(deqp_dir)/framework/common/tcuBilinearImageCompare.cpp',
                        '<(deqp_dir)/framework/common/tcuCommandLine.cpp',
                        '<(deqp_dir)/framework/common/tcuCompressedTexture.cpp',
                        '<(deqp_dir)/framework/common/tcuCPUWarmup.cpp',
                        '<(deqp_dir)/framework/common/tcuDefs.cpp',
                        '<(deqp_dir)/framework/common/tcuFactoryRegistry.cpp',
                        '<(deqp_dir)/framework/common/tcuFloatFormat.cpp',
                        '<(deqp_dir)/framework/common/tcuFunctionLibrary.cpp',
                        '<(deqp_dir)/framework/common/tcuFuzzyImageCompare.cpp',
                        '<(deqp_dir)/framework/common/tcuImageCompare.cpp',
                        '<(deqp_dir)/framework/common/tcuImageIO.cpp',
                        '<(deqp_dir)/framework/common/tcuInterval.cpp',
                        '<(deqp_dir)/framework/common/tcuPlatform.cpp',
                        '<(deqp_dir)/framework/common/tcuRandomValueIterator.cpp',
                        '<(deqp_dir)/framework/common/tcuRenderTarget.cpp',
                        '<(deqp_dir)/framework/common/tcuResource.cpp',
                        '<(deqp_dir)/framework/common/tcuRGBA.cpp',
                        '<(deqp_dir)/framework/common/tcuStringTemplate.cpp',
                        '<(deqp_dir)/framework/common/tcuSurface.cpp',
                        '<(deqp_dir)/framework/common/tcuTestCase.cpp',
                        '<(deqp_dir)/framework/common/tcuTestCaseWrapper.cpp',
                        '<(deqp_dir)/framework/common/tcuTestContext.cpp',
                        '<(deqp_dir)/framework/common/tcuTestExecutor.cpp',
                        '<(deqp_dir)/framework/common/tcuTestLog.cpp',
                        '<(deqp_dir)/framework/common/tcuTestPackage.cpp',
                        '<(deqp_dir)/framework/common/tcuTexCompareVerifier.cpp',
                        '<(deqp_dir)/framework/common/tcuTexLookupVerifier.cpp',
                        '<(deqp_dir)/framework/common/tcuTexture.cpp',
                        '<(deqp_dir)/framework/common/tcuTextureUtil.cpp',
                        '<(deqp_dir)/framework/common/tcuTexVerifierUtil.cpp',
                        '<(deqp_dir)/framework/common/tcuThreadUtil.cpp',
                        '<(deqp_dir)/framework/delibs/debase/deDefs.c',
                        '<(deqp_dir)/framework/delibs/debase/deFloat16.c',
                        '<(deqp_dir)/framework/delibs/debase/deInt32.c',
                        '<(deqp_dir)/framework/delibs/debase/deInt32Test.c',
                        '<(deqp_dir)/framework/delibs/debase/deMath.c',
                        '<(deqp_dir)/framework/delibs/debase/deMemory.c',
                        '<(deqp_dir)/framework/delibs/debase/deRandom.c',
                        '<(deqp_dir)/framework/delibs/debase/deString.c',
                        '<(deqp_dir)/framework/delibs/deimage/deImage.c',
                        '<(deqp_dir)/framework/delibs/deimage/deTarga.c',
                        '<(deqp_dir)/framework/delibs/depool/deMemPool.c',
                        '<(deqp_dir)/framework/delibs/depool/dePoolArray.c',
                        '<(deqp_dir)/framework/delibs/depool/dePoolHashArray.c',
                        '<(deqp_dir)/framework/delibs/depool/dePoolHash.c',
                        '<(deqp_dir)/framework/delibs/depool/dePoolHashSet.c',
                        '<(deqp_dir)/framework/delibs/depool/dePoolHeap.c',
                        '<(deqp_dir)/framework/delibs/depool/dePoolMultiSet.c',
                        '<(deqp_dir)/framework/delibs/depool/dePoolSet.c',
                        '<(deqp_dir)/framework/delibs/depool/dePoolStringBuilder.c',
                        '<(deqp_dir)/framework/delibs/depool/dePoolTest.c',
                        '<(deqp_dir)/framework/delibs/destream/deFileStream.c',
                        '<(deqp_dir)/framework/delibs/destream/deRingbuffer.c',
                        '<(deqp_dir)/framework/delibs/destream/deStreamCpyThread.c',
                        '<(deqp_dir)/framework/delibs/destream/deThreadStream.c',
                        '<(deqp_dir)/framework/delibs/dethread/deAtomic.c',
                        '<(deqp_dir)/framework/delibs/dethread/deSingleton.c',
                        '<(deqp_dir)/framework/delibs/dethread/deThreadTest.c',
                        # TODO(jmadill): other platforms
                        '<(deqp_dir)/framework/delibs/dethread/win32/deMutexWin32.c',
                        '<(deqp_dir)/framework/delibs/dethread/win32/deSemaphoreWin32.c',
                        '<(deqp_dir)/framework/delibs/dethread/win32/deThreadLocalWin32.c',
                        '<(deqp_dir)/framework/delibs/dethread/win32/deThreadWin32.c',
                        #'<(deqp_dir)/framework/delibs/dethread/unix/deMutexUnix.c',
                        #'<(deqp_dir)/framework/delibs/dethread/unix/deNamedSemaphoreUnix.c',
                        #'<(deqp_dir)/framework/delibs/dethread/unix/deSemaphoreUnix.c',
                        #'<(deqp_dir)/framework/delibs/dethread/unix/deThreadLocalUnix.c',
                        #'<(deqp_dir)/framework/delibs/dethread/unix/deThreadUnix.c',
                        '<(deqp_dir)/framework/delibs/deutil/deClock.c',
                        '<(deqp_dir)/framework/delibs/deutil/deCommandLine.c',
                        '<(deqp_dir)/framework/delibs/deutil/deDynamicLibrary.c',
                        '<(deqp_dir)/framework/delibs/deutil/deFile.c',
                        '<(deqp_dir)/framework/delibs/deutil/deProcess.c',
                        '<(deqp_dir)/framework/delibs/deutil/deSocket.c',
                        '<(deqp_dir)/framework/delibs/deutil/deTimer.c',
                        '<(deqp_dir)/framework/delibs/deutil/deTimerTest.c',
                        '<(deqp_dir)/framework/egl/egluCallLogWrapper.cpp',
                        '<(deqp_dir)/framework/egl/egluConfigFilter.cpp',
                        '<(deqp_dir)/framework/egl/egluConfigInfo.cpp',
                        '<(deqp_dir)/framework/egl/egluDefs.cpp',
                        '<(deqp_dir)/framework/egl/egluGLContextFactory.cpp',
                        '<(deqp_dir)/framework/egl/egluGLFunctionLoader.cpp',
                        '<(deqp_dir)/framework/egl/egluHeaderWrapper.cpp',
                        '<(deqp_dir)/framework/egl/egluNativeDisplay.cpp',
                        '<(deqp_dir)/framework/egl/egluNativePixmap.cpp',
                        '<(deqp_dir)/framework/egl/egluNativeWindow.cpp',
                        '<(deqp_dir)/framework/egl/egluPlatform.cpp',
                        # TODO(jmadill): upstream this fix
                        'deqp_support/egluStaticESLibrary.cpp',
                        '<(deqp_dir)/framework/egl/egluStrUtil.cpp',
                        '<(deqp_dir)/framework/egl/egluUnique.cpp',
                        '<(deqp_dir)/framework/egl/egluUtil.cpp',
                        '<(deqp_dir)/framework/egl/tcuEgl.cpp',
                        '<(deqp_dir)/framework/opengl/gluCallLogWrapper.cpp',
                        '<(deqp_dir)/framework/opengl/gluContextFactory.cpp',
                        '<(deqp_dir)/framework/opengl/gluContextInfo.cpp',
                        '<(deqp_dir)/framework/opengl/gluDefs.cpp',
                        '<(deqp_dir)/framework/opengl/gluDrawUtil.cpp',
                        '<(deqp_dir)/framework/opengl/gluDummyRenderContext.cpp',
                        '<(deqp_dir)/framework/opengl/gluES3PlusWrapperContext.cpp',
                        '<(deqp_dir)/framework/opengl/gluFboRenderContext.cpp',
                        '<(deqp_dir)/framework/opengl/gluObjectWrapper.cpp',
                        '<(deqp_dir)/framework/opengl/gluPixelTransfer.cpp',
                        '<(deqp_dir)/framework/opengl/gluPlatform.cpp',
                        '<(deqp_dir)/framework/opengl/gluProgramInterfaceQuery.cpp',
                        '<(deqp_dir)/framework/opengl/gluRenderConfig.cpp',
                        '<(deqp_dir)/framework/opengl/gluRenderContext.cpp',
                        '<(deqp_dir)/framework/opengl/gluShaderProgram.cpp',
                        '<(deqp_dir)/framework/opengl/gluShaderUtil.cpp',
                        '<(deqp_dir)/framework/opengl/gluStateReset.cpp',
                        '<(deqp_dir)/framework/opengl/gluStrUtil.cpp',
                        '<(deqp_dir)/framework/opengl/gluTexture.cpp',
                        '<(deqp_dir)/framework/opengl/gluTextureUtil.cpp',
                        '<(deqp_dir)/framework/opengl/gluVarType.cpp',
                        '<(deqp_dir)/framework/opengl/gluVarTypeUtil.cpp',
                        '<(deqp_dir)/framework/opengl/simplereference/sglrContext.cpp',
                        '<(deqp_dir)/framework/opengl/simplereference/sglrContextUtil.cpp',
                        '<(deqp_dir)/framework/opengl/simplereference/sglrContextWrapper.cpp',
                        '<(deqp_dir)/framework/opengl/simplereference/sglrGLContext.cpp',
                        '<(deqp_dir)/framework/opengl/simplereference/sglrReferenceContext.cpp',
                        '<(deqp_dir)/framework/opengl/simplereference/sglrReferenceUtils.cpp',
                        '<(deqp_dir)/framework/opengl/simplereference/sglrShaderProgram.cpp',
                        '<(deqp_dir)/framework/opengl/wrapper/glwDefs.cpp',
                        '<(deqp_dir)/framework/opengl/wrapper/glwFunctions.cpp',
                        '<(deqp_dir)/framework/opengl/wrapper/glwInitES20Direct.cpp',
                        '<(deqp_dir)/framework/opengl/wrapper/glwInitES30Direct.cpp',
                        '<(deqp_dir)/framework/opengl/wrapper/glwInitFunctions.cpp',
                        '<(deqp_dir)/framework/opengl/wrapper/glwWrapper.cpp',
                        # TODO(jmadill): other platforms
                        'deqp_support/tcuWin32Platform.cpp',
                        'deqp_support/tcuWin32Window.cpp',
                        'deqp_support/tcuWin32EGLNativeDisplayFactory.cpp',
                        '<(deqp_dir)/framework/qphelper/qpCrashHandler.c',
                        '<(deqp_dir)/framework/qphelper/qpDebugOut.c',
                        '<(deqp_dir)/framework/qphelper/qpInfo.c',
                        '<(deqp_dir)/framework/qphelper/qpTestLog.c',
                        '<(deqp_dir)/framework/qphelper/qpWatchDog.c',
                        '<(deqp_dir)/framework/qphelper/qpXmlWriter.c',
                        '<(deqp_dir)/framework/randomshaders/rsgBinaryOps.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgBuiltinFunctions.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgDefs.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgExecutionContext.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgExpression.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgExpressionGenerator.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgFunctionGenerator.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgGeneratorState.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgNameAllocator.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgParameters.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgPrettyPrinter.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgProgramExecutor.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgProgramGenerator.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgSamplers.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgShader.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgShaderGenerator.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgStatement.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgTest.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgToken.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgUtils.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgVariable.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgVariableManager.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgVariableType.cpp',
                        '<(deqp_dir)/framework/randomshaders/rsgVariableValue.cpp',
                        '<(deqp_dir)/framework/referencerenderer/rrDefs.cpp',
                        '<(deqp_dir)/framework/referencerenderer/rrFragmentOperations.cpp',
                        '<(deqp_dir)/framework/referencerenderer/rrMultisamplePixelBufferAccess.cpp',
                        '<(deqp_dir)/framework/referencerenderer/rrPrimitivePacket.cpp',
                        '<(deqp_dir)/framework/referencerenderer/rrRasterizer.cpp',
                        '<(deqp_dir)/framework/referencerenderer/rrRenderer.cpp',
                        '<(deqp_dir)/framework/referencerenderer/rrShaders.cpp',
                        '<(deqp_dir)/framework/referencerenderer/rrShadingContext.cpp',
                        '<(deqp_dir)/framework/referencerenderer/rrVertexAttrib.cpp',
                        '<(deqp_dir)/framework/referencerenderer/rrVertexPacket.cpp',
                        '<(deqp_dir)/modules/gles3/accuracy/es3aAccuracyTests.cpp',
                        '<(deqp_dir)/modules/gles3/accuracy/es3aTextureFilteringTests.cpp',
                        '<(deqp_dir)/modules/gles3/accuracy/es3aTextureMipmapTests.cpp',
                        '<(deqp_dir)/modules/gles3/accuracy/es3aVaryingInterpolationTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fApiCase.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fASTCDecompressionCases.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fAttribLocationTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fBlendTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fBooleanStateQueryTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fBufferCopyTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fBufferMapTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fBufferObjectQueryTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fBufferWriteTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fBuiltinPrecisionTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fClippingTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fColorClearTest.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fCompressedTextureTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fDefaultVertexAttributeTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fDepthStencilClearTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fDepthStencilTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fDepthTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fDitheringTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fDrawTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fFboApiTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fFboColorbufferTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fFboCompletenessTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fFboDepthbufferTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fFboInvalidateTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fFboMultisampleTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fFboRenderTest.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fFboStateQueryTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fFboStencilbufferTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fFboTestCase.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fFboTestUtil.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fFloatStateQueryTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fFlushFinishTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fFragDepthTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fFragmentOutputTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fFragOpInteractionTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fFramebufferBlitTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fFunctionalTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fImplementationLimitTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fIndexedStateQueryTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fInstancedRenderingTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fInteger64StateQueryTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fIntegerStateQueryTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fInternalFormatQueryTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fLifetimeTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fMultisampleTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fNegativeBufferApiTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fNegativeFragmentApiTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fNegativeShaderApiTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fNegativeStateApiTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fNegativeTextureApiTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fNegativeVertexArrayApiTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fOcclusionQueryTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fPixelBufferObjectTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fPolygonOffsetTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fPrerequisiteTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fPrimitiveRestartTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fRandomFragmentOpTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fRandomShaderTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fRasterizationTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fRasterizerDiscardTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fRboStateQueryTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fReadPixelsTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fSamplerObjectTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fSamplerStateQueryTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fScissorTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fShaderApiTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fShaderBuiltinVarTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fShaderCommonFunctionTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fShaderConstExprTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fShaderDerivateTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fShaderDiscardTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fShaderFragDataTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fShaderIndexingTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fShaderInvarianceTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fShaderLoopTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fShaderMatrixTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fShaderOperatorTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fShaderPackingFunctionTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fShaderPrecisionTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fShaderReturnTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fShaderStateQueryTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fShaderStructTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fShaderSwitchTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fShaderTextureFunctionTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fStencilTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fStringQueryTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fSyncTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fTextureFilteringTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fTextureFormatTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fTextureMipmapTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fTextureShadowTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fTextureSizeTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fTextureSpecificationTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fTextureStateQueryTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fTextureSwizzleTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fTextureUnitTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fTextureWrapTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fTransformFeedbackTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fUniformApiTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fUniformBlockTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fVertexArrayObjectTests.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fVertexArrayTest.cpp',
                        '<(deqp_dir)/modules/gles3/functional/es3fVertexTextureTests.cpp',
                        '<(deqp_dir)/modules/gles3/performance/es3pBlendTests.cpp',
                        '<(deqp_dir)/modules/gles3/performance/es3pBufferDataUploadTests.cpp',
                        '<(deqp_dir)/modules/gles3/performance/es3pDepthTests.cpp',
                        '<(deqp_dir)/modules/gles3/performance/es3pPerformanceTests.cpp',
                        '<(deqp_dir)/modules/gles3/performance/es3pRedundantStateChangeTests.cpp',
                        '<(deqp_dir)/modules/gles3/performance/es3pShaderCompilationCases.cpp',
                        '<(deqp_dir)/modules/gles3/performance/es3pShaderCompilerTests.cpp',
                        '<(deqp_dir)/modules/gles3/performance/es3pShaderControlStatementTests.cpp',
                        '<(deqp_dir)/modules/gles3/performance/es3pShaderOperatorTests.cpp',
                        '<(deqp_dir)/modules/gles3/performance/es3pShaderOptimizationTests.cpp',
                        '<(deqp_dir)/modules/gles3/performance/es3pStateChangeCallTests.cpp',
                        '<(deqp_dir)/modules/gles3/performance/es3pStateChangeTests.cpp',
                        '<(deqp_dir)/modules/gles3/performance/es3pTextureCases.cpp',
                        '<(deqp_dir)/modules/gles3/performance/es3pTextureCountTests.cpp',
                        '<(deqp_dir)/modules/gles3/performance/es3pTextureFilteringTests.cpp',
                        '<(deqp_dir)/modules/gles3/performance/es3pTextureFormatTests.cpp',
                        '<(deqp_dir)/modules/gles3/stress/es3sDrawTests.cpp',
                        '<(deqp_dir)/modules/gles3/stress/es3sLongRunningShaderTests.cpp',
                        '<(deqp_dir)/modules/gles3/stress/es3sLongRunningTests.cpp',
                        '<(deqp_dir)/modules/gles3/stress/es3sLongShaderTests.cpp',
                        '<(deqp_dir)/modules/gles3/stress/es3sMemoryTests.cpp',
                        '<(deqp_dir)/modules/gles3/stress/es3sOcclusionQueryTests.cpp',
                        '<(deqp_dir)/modules/gles3/stress/es3sSpecialFloatTests.cpp',
                        '<(deqp_dir)/modules/gles3/stress/es3sStressTests.cpp',
                        '<(deqp_dir)/modules/gles3/stress/es3sSyncTests.cpp',
                        '<(deqp_dir)/modules/gles3/stress/es3sVertexArrayTests.cpp',
                        '<(deqp_dir)/modules/gles3/tes3Context.cpp',
                        '<(deqp_dir)/modules/gles3/tes3InfoTests.cpp',
                        '<(deqp_dir)/modules/gles3/tes3TestCase.cpp',
                        '<(deqp_dir)/modules/gles3/tes3TestCaseWrapper.cpp',
                        '<(deqp_dir)/modules/gles3/tes3TestPackage.cpp',
                        '<(deqp_dir)/modules/gles3/tes3TestPackageEntry.cpp',
                        '<(deqp_dir)/modules/glshared/glsAttributeLocationTests.cpp',
                        '<(deqp_dir)/modules/glshared/glsBufferTestUtil.cpp',
                        '<(deqp_dir)/modules/glshared/glsBuiltinPrecisionTests.cpp',
                        '<(deqp_dir)/modules/glshared/glsCalibration.cpp',
                        '<(deqp_dir)/modules/glshared/glsDrawTest.cpp',
                        '<(deqp_dir)/modules/glshared/glsFboCompletenessTests.cpp',
                        '<(deqp_dir)/modules/glshared/glsFboUtil.cpp',
                        '<(deqp_dir)/modules/glshared/glsFragmentOpUtil.cpp',
                        '<(deqp_dir)/modules/glshared/glsFragOpInteractionCase.cpp',
                        '<(deqp_dir)/modules/glshared/glsInteractionTestUtil.cpp',
                        '<(deqp_dir)/modules/glshared/glsLifetimeTests.cpp',
                        '<(deqp_dir)/modules/glshared/glsLongStressCase.cpp',
                        '<(deqp_dir)/modules/glshared/glsLongStressTestUtil.cpp',
                        '<(deqp_dir)/modules/glshared/glsMemoryStressCase.cpp',
                        '<(deqp_dir)/modules/glshared/glsRandomShaderCase.cpp',
                        '<(deqp_dir)/modules/glshared/glsRandomShaderProgram.cpp',
                        '<(deqp_dir)/modules/glshared/glsRandomUniformBlockCase.cpp',
                        '<(deqp_dir)/modules/glshared/glsRasterizationTestUtil.cpp',
                        '<(deqp_dir)/modules/glshared/glsSamplerObjectTest.cpp',
                        '<(deqp_dir)/modules/glshared/glsScissorTests.cpp',
                        '<(deqp_dir)/modules/glshared/glsShaderConstExprTests.cpp',
                        '<(deqp_dir)/modules/glshared/glsShaderExecUtil.cpp',
                        '<(deqp_dir)/modules/glshared/glsShaderLibraryCase.cpp',
                        '<(deqp_dir)/modules/glshared/glsShaderLibrary.cpp',
                        '<(deqp_dir)/modules/glshared/glsShaderPerformanceCase.cpp',
                        '<(deqp_dir)/modules/glshared/glsShaderPerformanceMeasurer.cpp',
                        '<(deqp_dir)/modules/glshared/glsShaderRenderCase.cpp',
                        '<(deqp_dir)/modules/glshared/glsStateChangePerfTestCases.cpp',
                        '<(deqp_dir)/modules/glshared/glsTextureBufferCase.cpp',
                        '<(deqp_dir)/modules/glshared/glsTextureTestUtil.cpp',
                        '<(deqp_dir)/modules/glshared/glsUniformBlockCase.cpp',
                        '<(deqp_dir)/modules/glshared/glsVertexArrayTests.cpp',
                        '<(deqp_dir)/modules/internal/ditBuildInfoTests.cpp',
                        '<(deqp_dir)/modules/internal/ditDelibsTests.cpp',
                        '<(deqp_dir)/modules/internal/ditFrameworkTests.cpp',
                        '<(deqp_dir)/modules/internal/ditImageCompareTests.cpp',
                        '<(deqp_dir)/modules/internal/ditImageIOTests.cpp',
                        '<(deqp_dir)/modules/internal/ditTestCase.cpp',
                        '<(deqp_dir)/modules/internal/ditTestLogTests.cpp',
                        '<(deqp_dir)/modules/internal/ditTestPackage.cpp',
                        '<(deqp_dir)/modules/internal/ditTestPackageEntry.cpp',
                    ],
                },

                {
                    'target_name': 'angle_perf_tests',
                    'type': 'executable',
                    'includes': [ '../build/common_defines.gypi', ],
                    'dependencies':
                    [
                        '../src/angle.gyp:libGLESv2',
                        '../src/angle.gyp:libEGL',
                        '../util/util.gyp:angle_util',
                        'angle_test_support',
                    ],
                    'include_dirs':
                    [
                        '../include',
                    ],
                    'sources':
                    [
                        'perf_tests/BufferSubData.cpp',
                        'perf_tests/BufferSubData.h',
                        'perf_tests/PointSprites.cpp',
                        'perf_tests/PointSprites.h',
                        'perf_tests/SimpleBenchmark.cpp',
                        'perf_tests/SimpleBenchmark.h',
                        'perf_tests/SimpleBenchmarks.cpp',
                        'perf_tests/TexSubImage.cpp',
                        'perf_tests/TexSubImage.h',
                        'perf_tests/third_party/perf/perf_test.cc',
                        'perf_tests/third_party/perf/perf_test.h',
                    ],
                },

            ],
            'conditions':
            [
                ['angle_build_conformance_tests',
                {
                    'variables':
                    {
                        'gles_conformance_tests_output_dir': '<(SHARED_INTERMEDIATE_DIR)/conformance_tests',
                        'gles_conformance_tests_input_dir': 'third_party/gles_conformance_tests/conform/GTF_ES/glsl/GTF',
                        'gles_conformance_tests_generator_script': 'gles_conformance_tests/generate_gles_conformance_tests.py',
                    },
                    'targets':
                    [
                        {
                            'target_name': 'angle_gles2_conformance_tests',
                            'type': 'executable',
                            'includes': [ '../build/common_defines.gypi', ],
                            'dependencies':
                            [
                                '../src/angle.gyp:libGLESv2',
                                '../src/angle.gyp:libEGL',
                                'third_party/gles_conformance_tests/conform/GTF_ES/glsl/GTF/es_cts.gyp:es_cts_test_data',
                                'third_party/gles_conformance_tests/conform/GTF_ES/glsl/GTF/es_cts.gyp:es2_cts',
                                'angle_test_support',
                            ],
                            'variables':
                            {
                                'gles2_conformance_tests_input_file': '<(gles_conformance_tests_input_dir)/mustpass_es20.run',
                                'gles2_conformance_tests_generated_file': '<(gles_conformance_tests_output_dir)/generated_gles2_conformance_tests.cpp',
                            },
                            'sources':
                            [
                                '<!@(python <(angle_path)/enumerate_files.py gles_conformance_tests -types *.cpp *.h *.inl)',
                                '<(gles2_conformance_tests_generated_file)',
                            ],
                            'include_dirs':
                            [
                                '../include',
                                'gles_conformance_tests',
                            ],
                            'defines':
                            [
                                'CONFORMANCE_TESTS_TYPE=CONFORMANCE_TESTS_ES2',
                            ],
                            'actions':
                            [
                                {
                                    'action_name': 'generate_gles2_conformance_tests',
                                    'message': 'Generating ES2 conformance tests...',
                                    'msvs_cygwin_shell': 0,
                                    'inputs':
                                    [
                                        '<(gles_conformance_tests_generator_script)',
                                        '<(gles2_conformance_tests_input_file)',
                                    ],
                                    'outputs':
                                    [
                                        '<(gles2_conformance_tests_generated_file)',
                                    ],
                                    'action':
                                    [
                                        'python',
                                        '<(gles_conformance_tests_generator_script)',
                                        '<(gles2_conformance_tests_input_file)',
                                        '<(gles_conformance_tests_input_dir)',
                                        '<(gles2_conformance_tests_generated_file)',
                                    ],
                                },
                            ],
                        },
                        {
                            'target_name': 'angle_gles3_conformance_tests',
                            'type': 'executable',
                            'includes': [ '../build/common_defines.gypi', ],
                            'dependencies':
                            [
                                '../src/angle.gyp:libGLESv2',
                                '../src/angle.gyp:libEGL',
                                'third_party/gles_conformance_tests/conform/GTF_ES/glsl/GTF/es_cts.gyp:es_cts_test_data',
                                'third_party/gles_conformance_tests/conform/GTF_ES/glsl/GTF/es_cts.gyp:es3_cts',
                                'angle_test_support',
                            ],
                            'variables':
                            {
                                'gles3_conformance_tests_input_file': '<(gles_conformance_tests_input_dir)/mustpass_es30.run',
                                'gles3_conformance_tests_generated_file': '<(gles_conformance_tests_output_dir)/generated_gles3_conformance_tests.cpp',
                            },
                            'sources':
                            [
                                '<!@(python <(angle_path)/enumerate_files.py gles_conformance_tests -types *.cpp *.h *.inl)',
                                '<(gles3_conformance_tests_generated_file)',
                            ],
                            'include_dirs':
                            [
                                '../include',
                                'gles_conformance_tests',
                            ],
                            'defines':
                            [
                                'CONFORMANCE_TESTS_TYPE=CONFORMANCE_TESTS_ES3',
                            ],
                            'msvs_settings':
                            {
                                'VCCLCompilerTool':
                                {
                                    # MSVS has trouble compiling this due to the obj files becoming too large.
                                    'AdditionalOptions': [ '/bigobj' ],
                                },
                            },
                            'actions':
                            [
                                {
                                    'action_name': 'generate_gles3_conformance_tests',
                                    'message': 'Generating ES3 conformance tests...',
                                    'msvs_cygwin_shell': 0,
                                    'inputs':
                                    [
                                        '<(gles_conformance_tests_generator_script)',
                                        '<(gles3_conformance_tests_input_file)',
                                    ],
                                    'outputs':
                                    [
                                        '<(gles3_conformance_tests_generated_file)',
                                    ],
                                    'action':
                                    [
                                        'python',
                                        '<(gles_conformance_tests_generator_script)',
                                        '<(gles3_conformance_tests_input_file)',
                                        '<(gles_conformance_tests_input_dir)',
                                        '<(gles3_conformance_tests_generated_file)',
                                    ],
                                },
                            ],
                        },
                    ],
                }],
                ['angle_build_deqp_tests',
                {
                    'targets':
                    [
                        {
                            'target_name': 'angle_deqp_tests',
                            'type': 'executable',
                            'includes': [ '../build/common_defines.gypi', ],
                            'dependencies':
                            [
                                '../src/angle.gyp:libGLESv2',
                                '../src/angle.gyp:libEGL',
                                'third_party/deqp/src/deqp/modules/gles3/gles3.gyp:deqp-gles3',
                                'third_party/deqp/src/deqp/framework/platform/platform.gyp:tcutil-platform',
                                'angle_test_support',
                            ],
                            'include_dirs':
                            [
                                '../include',
                                'deqp_tests',
                            ],
                            'variables':
                            {
                                'deqp_tests_output_dir': '<(SHARED_INTERMEDIATE_DIR)/deqp_tests',
                                'deqp_tests_input_file': 'deqp_tests/deqp_tests.txt',
                                'deqp_tests_generated_file': '<(deqp_tests_output_dir)/generated_deqp_tests.cpp',
                            },
                            'sources':
                            [
                                '<!@(python <(angle_path)/enumerate_files.py deqp_tests -types *.cpp *.h *.inl)',
                                '<(deqp_tests_generated_file)',
                            ],
                            'actions':
                            [
                                {
                                    'action_name': 'generate_deqp_tests',
                                    'message': 'Generating dEQP tests...',
                                    'msvs_cygwin_shell': 0,
                                    'variables':
                                    {
                                        'deqp_tests_generator_script': 'deqp_tests/generate_deqp_tests.py',
                                    },
                                    'inputs':
                                    [
                                        '<(deqp_tests_generator_script)',
                                        '<(deqp_tests_input_file)',
                                    ],
                                    'outputs':
                                    [
                                        '<(deqp_tests_generated_file)',
                                    ],
                                    'action':
                                    [
                                        'python',
                                        '<(deqp_tests_generator_script)',
                                        '<(deqp_tests_input_file)',
                                        '<(deqp_tests_generated_file)',
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
