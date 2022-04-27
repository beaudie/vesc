# Copyright 2022 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import contextlib
import functools
import glob
import json
import logging
import os
import pathlib
import posixpath
import random
import re
import subprocess
import tarfile
import tempfile
import threading
import time

import angle_path_util
import angle_test_util


def _ApkPath(suite_name):
    return os.path.join('%s_apk' % suite_name, '%s-debug.apk' % suite_name)


def ApkFileExists(suite_name):
    return os.path.exists(_ApkPath(suite_name))


def _Run(cmd):
    logging.debug('Executing command: %s', cmd)
    startupinfo = None
    if hasattr(subprocess, 'STARTUPINFO'):
        # Prevent console window popping up on Windows
        startupinfo = subprocess.STARTUPINFO()
        startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        startupinfo.wShowWindow = subprocess.SW_HIDE
    output = subprocess.check_output(cmd, startupinfo=startupinfo)
    return output


@functools.lru_cache()
def _FindAdb():
    platform_tools = (
        pathlib.Path(angle_path_util.ANGLE_ROOT_DIR) / 'third_party' / 'android_sdk' / 'public' /
        'platform-tools')

    if platform_tools.exists():
        adb = str(platform_tools / 'adb')
    else:
        adb = 'adb'

    adb_info = subprocess.check_output([adb, '--version']).decode()
    logging.info('adb --version: %s', adb_info)

    return adb


def _AdbRun(args):
    return _Run([_FindAdb()] + args)


def _AdbShell(cmd):
    return _Run([_FindAdb(), 'shell', cmd])


def _GetAdbRoot():
    _AdbRun(['root'])

    for _ in range(20):
        time.sleep(0.5)
        try:
            id_out = _AdbShell('id').decode('ascii')
            if 'uid=0(root)' in id_out:
                return
        except Exception:
            continue
    raise Exception("adb root failed")


def _ReadDeviceFile(device_path):
    with _TempLocalFile() as tempfile_path:
        _AdbRun(['pull', device_path, tempfile_path])
        with open(tempfile_path, 'rb') as f:
            return f.read()


def _RemoveDeviceFile(device_path):
    _AdbShell('rm -f ' + device_path + ' || true')  # ignore errors


def _AddRestrictedTracesJson():
    _AdbShell('mkdir -p /sdcard/chromium_tests_root/')

    def add(tar, fn):
        assert (fn.startswith('../../'))
        tar.add(fn, arcname=fn.replace('../../', ''))

    with _TempLocalFile() as tempfile_path:
        with tarfile.open(tempfile_path, 'w', format=tarfile.GNU_FORMAT) as tar:
            for f in glob.glob('../../src/tests/restricted_traces/*/*.json', recursive=True):
                add(tar, f)
            add(tar, '../../src/tests/restricted_traces/restricted_traces.json')
        _AdbRun(['push', tempfile_path, '/sdcard/chromium_tests_root/t.tar'])

    _AdbShell('r=/sdcard/chromium_tests_root; tar -xf $r/t.tar -C $r/ && rm $r/t.tar')


def _AddDeqpFiles():
    _AdbShell('mkdir -p /sdcard/chromium_tests_root/')

    def add(tar, fn):
        assert (fn.startswith('../../'))
        tar.add(fn, arcname=fn.replace('../../', ''))

    with _TempLocalFile() as tempfile_path:
        with tarfile.open(tempfile_path, 'w', format=tarfile.GNU_FORMAT, dereference=True) as tar:
            for f in glob.glob('../../src/tests/deqp_support/*.txt', recursive=False):
                add(tar, f)

            tar.add('gen/vk_gl_cts_data')
            add(
                tar,
                '../../third_party/VK-GL-CTS/src/external/openglcts/data/mustpass/gles/aosp_mustpass/main/gles3-master.txt'
            )
        _AdbRun(['push', tempfile_path, '/sdcard/chromium_tests_root/t.tar'])

    _AdbShell('r=/sdcard/chromium_tests_root; tar -xf $r/t.tar -C $r/ && rm $r/t.tar')


def PrepareTestSuite(suite_name):
    _GetAdbRoot()

    apk_path = _ApkPath(suite_name)
    logging.info('Installing apk path=%s size=%s' % (apk_path, os.path.getsize(apk_path)))

    _AdbRun(['install', '-r', '-d', apk_path])

    permissions = [
        'android.permission.CAMERA', 'android.permission.CHANGE_CONFIGURATION',
        'android.permission.READ_EXTERNAL_STORAGE', 'android.permission.RECORD_AUDIO',
        'android.permission.WRITE_EXTERNAL_STORAGE'
    ]
    _AdbShell('p=com.android.angle.test;'
              'for q in %s;do pm grant "$p" "$q";done;' % ' '.join(permissions))

    if suite_name == 'angle_perftests':
        _AddRestrictedTracesJson()

    if 'deqp' in suite_name:
        _AddDeqpFiles()


def PrepareRestrictedTraces(traces):
    start = time.time()
    total_size = 0
    for trace in traces:
        path_from_root = 'src/tests/restricted_traces/' + trace + '/' + trace + '.angledata.gz'
        local_path = '../../' + path_from_root
        total_size += os.path.getsize(local_path)
        _AdbRun(['push', local_path, '/sdcard/chromium_tests_root/' + path_from_root])

    logging.info('Pushed %d trace files (%.1fMB) in %.1fs', len(traces), total_size / 1e6,
                 time.time() - start)


def DumpFrequncies():
    try:
        freqs = _AdbShell('cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_cur_freq')
        logging.info('cpu frequencies:\n%s', freqs.decode())
    except Exception:
        logging.info('cpu frequencies failed')

    try:
        freqs = _AdbShell('cat /sys/devices/platform/1c500000.mali/clock_info')
        logging.info('gpu frequencies:\n%s', freqs.decode())
    except Exception:
        logging.info('gpu frequencies failed')

    try:
        top = _AdbShell('top -m 10 -bn 1')
        logging.info('top cpu:\n%s', top.decode())
    except Exception:
        logging.info('top failed')

    try:
        top = _AdbShell('top -m 10 -bn 1 -s 10')
        logging.info('top memory:\n%s', top.decode())
    except Exception:
        logging.info('top failed')


def _RandomHex():
    return hex(random.randint(0, 2**64))[2:]


@contextlib.contextmanager
def _TempDeviceDir():
    path = '/sdcard/Download/temp_dir-%s' % _RandomHex()
    _AdbShell('mkdir -p ' + path)
    try:
        yield path
    finally:
        _AdbShell('rm -rf ' + path)


@contextlib.contextmanager
def _TempDeviceFile():
    path = '/sdcard/Download/temp_file-%s' % _RandomHex()
    try:
        yield path
    finally:
        _AdbShell('rm -f ' + path)


@contextlib.contextmanager
def _TempLocalFile():
    fd, path = tempfile.mkstemp()
    os.close(fd)
    try:
        yield path
    finally:
        os.remove(path)


def _RunInstrumentation(flags):
    with _TempDeviceFile() as temp_device_file:
        cmd = ' '.join([
            'p=com.android.angle.test;',
            'ntr=org.chromium.native_test.NativeTestInstrumentationTestRunner;',
            'am instrument -w',
            '-e $ntr.NativeTestActivity "$p".AngleUnitTestActivity',
            '-e $ntr.ShardNanoTimeout 2400000000000',
            '-e org.chromium.native_test.NativeTest.CommandLineFlags "%s"' % ' '.join(flags),
            '-e $ntr.StdoutFile ' + temp_device_file,
            '"$p"/org.chromium.build.gtest_apk.NativeTestInstrumentationTestRunner',
        ])

        _AdbShell(cmd)
        return _ReadDeviceFile(temp_device_file)


def _DumpDebugInfo(since_time):
    logcat_output = _AdbRun(['logcat', '-t', since_time]).decode()
    logging.info('logcat:\n%s', logcat_output)

    pid_lines = [
        ln for ln in logcat_output.split('\n')
        if 'org.chromium.native_test.NativeTest.StdoutFile' in ln
    ]
    if pid_lines:
        debuggerd_output = _AdbShell('debuggerd -b %s' % pid_lines[-1].split()[2]).decode()
        logging.warning('debuggerd output:\n%s', debuggerd_output)


GLO = {}


def _AwkBenchmark():
    start = time.time()
    result = _AdbShell(
        'echo | awk \'BEGIN{ num=1000000; n=0; }END{ for (i=1; i<=num; i++) { n += sin(i); } print n,"\\n"; }\''
    ).decode().strip()
    return '%.3f' % (time.time() - start), result


def _RunInstrumentationWithTimeout(flags, timeout):
    initial_time = _AdbShell('date +"%F %T.%3N"').decode().strip()

    results = []

    logging.info('_AwkBenchmark before: %s', _AwkBenchmark())

    def run():
        results.append(_RunInstrumentation(flags))

    t = threading.Thread(target=run)
    t.daemon = True
    t.start()

    logcat_output = None

    start = time.time()
    last_niceness = None
    while True:
        time.sleep(1)
        if time.time() - start > 70:
            raise Exception('more than 70s')
        elif time.time() - start > 20:
            if not logcat_output:
                logcat_output = _AdbRun(['logcat', '-t', initial_time]).decode()
                pid_lines = [
                    ln for ln in logcat_output.split('\n')
                    if 'org.chromium.native_test.NativeTest.StdoutFile' in ln
                ]
                test_pid = pid_lines[-1].split()[2]

            #logging.info('_AwkBenchmark during: %s', _AwkBenchmark())
            try:
                out_status = _AdbShell('cat /proc/%s/status' % test_pid).decode()
                logging.info('process_status:\n%s', out_status)

                try:
                    out_strace = _AdbShell(
                        'timeout 2 strace -tt -fp %s 2>&1 | grep -E "unfinished|resumed" || true' %
                        test_pid).decode()
                    logging.info('strace:\n%s', out_strace)
                except Exception:
                    pass

                debuggerd_output = _AdbShell('debuggerd -b %s' % test_pid).decode()
                logging.warning('debuggerd output:\n%s', debuggerd_output)

            except Exception:
                pass

            # with _TempDeviceFile() as png_device_path:
            #     output_dir = GLO['render_test_output_dir']
            #     png = _AdbShell('screencap -p %s' % png_device_path)
            #     fn = 'screen%d.png' % int(time.time())
            #     _AdbRun(['pull', png_device_path, os.path.join(output_dir, fn)])
            #     logging.info('saved %s' % fn)
            #time.sleep(1)

        if not t.is_alive():
            break

    t.join(timeout=timeout)

    if t.is_alive():  # join timed out
        logging.warning('Timed out, dumping debug info')
        _DumpDebugInfo(since_time=initial_time)
        raise TimeoutError('Test run did not finish in %s seconds' % timeout)

    return results[0]


def AngleSystemInfo(args):
    PrepareTestSuite('angle_system_info_test')

    with _TempDeviceDir() as temp_dir:
        _RunInstrumentation(args + ['--render-test-output-dir=' + temp_dir])
        output_file = posixpath.join(temp_dir, 'angle_system_info.json')
        return json.loads(_ReadDeviceFile(output_file))


def ListTests():
    out_lines = _RunInstrumentation(["--list-tests"]).decode('ascii').split('\n')

    start = out_lines.index('Tests list:')
    end = out_lines.index('End tests list.')
    return out_lines[start + 1:end]


def _PullDir(device_dir, local_dir):
    files = _AdbShell('ls -1 %s' % device_dir).decode('ascii').split('\n')
    for f in files:
        f = f.strip()
        if f:
            _AdbRun(['pull', posixpath.join(device_dir, f), posixpath.join(local_dir, f)])


def _RemoveFlag(args, f):
    matches = [a for a in args if a.startswith(f + '=')]
    assert len(matches) <= 1
    if matches:
        original_value = matches[0].split('=')[1]
        args.remove(matches[0])
    else:
        original_value = None

    return original_value


def RunSmokeTest():
    test_name = 'TracePerfTest.Run/vulkan_words_with_friends_2'
    run_instrumentation_timeout = 60

    logging.info('Running smoke test (%s)', test_name)

    PrepareRestrictedTraces([GetTraceFromTestName(test_name)])

    with _TempDeviceFile() as device_test_output_path:
        flags = [
            '--gtest_filter=' + test_name, '--no-warmup', '--steps-per-trial', '1', '--trials',
            '1', '--isolated-script-test-output=' + device_test_output_path
        ]
        try:
            _RunInstrumentationWithTimeout(flags, run_instrumentation_timeout)
        except TimeoutError:
            raise Exception('Smoke test did not finish in %s seconds' %
                            run_instrumentation_timeout)

        test_output = _ReadDeviceFile(device_test_output_path)

    output_json = json.loads(test_output)
    if output_json['tests'][test_name]['actual'] != 'PASS':
        raise Exception('Smoke test (%s) failed' % test_name)

    logging.info('Smoke test passed')


def RunTests(test_suite, args, stdoutfile=None, output_dir=None, log_output=True):
    args = args[:]
    test_output_path = _RemoveFlag(args, '--isolated-script-test-output')
    perf_output_path = _RemoveFlag(args, '--isolated-script-test-perf-output')
    test_output_dir = _RemoveFlag(args, '--render-test-output-dir')

    result = 0
    output = b''
    try:
        with contextlib.ExitStack() as stack:
            device_test_output_path = stack.enter_context(_TempDeviceFile())
            args.append('--isolated-script-test-output=' + device_test_output_path)

            if perf_output_path:
                device_perf_path = stack.enter_context(_TempDeviceFile())
                args.append('--isolated-script-test-perf-output=%s' % device_perf_path)

            if test_output_dir:
                device_output_dir = stack.enter_context(_TempDeviceDir())
                args.append('--render-test-output-dir=' + device_output_dir)

            output = _RunInstrumentationWithTimeout(args, timeout=10 * 60)

            test_output = _ReadDeviceFile(device_test_output_path)
            if test_output_path:
                with open(test_output_path, 'wb') as f:
                    f.write(test_output)

            output_json = json.loads(test_output)

            num_failures = output_json.get('num_failures_by_type', {}).get('FAIL', 0)
            interrupted = output_json.get('interrupted', True)  # Normally set to False
            if num_failures != 0 or interrupted or output_json.get('is_unexpected', False):
                logging.error('Tests failed: %s', test_output.decode())
                result = 1

            if test_output_dir:
                _PullDir(device_output_dir, test_output_dir)

            if perf_output_path:
                _AdbRun(['pull', device_perf_path, perf_output_path])

        if log_output:
            logging.info(output.decode())

        if stdoutfile:
            with open(stdoutfile, 'wb') as f:
                f.write(output)
    except Exception as e:
        logging.exception(e)
        result = 1

    return result, output


def GetTraceFromTestName(test_name):
    m = re.search(r'TracePerfTest.Run/(native|vulkan)_(.*)', test_name)
    if m:
        return m.group(2)

    if test_name.startswith('TracePerfTest.Run/'):
        raise Exception('Unexpected test: %s' % test_name)

    return None


def StucknessTest(args, runs):
    suite = 'angle_deqp_gles3_tests'
    if not ApkFileExists(suite):
        return False

    GLO['render_test_output_dir'] = args.render_test_output_dir

    angle_test_util.setupLogging('INFO')

    PrepareTestSuite(suite)

    #tt2 = 'dEQP.GLES3/functional_shaders_matrix_post_decrement_highp_mat2x3_float_vertex:dEQP.GLES3/functional_clipping_triangle_vertex_clip_two_clip_neg_x_and_neg_y:dEQP.GLES3/functional_ubo_single_basic_type_packed_mediump_mat3x4_fragment:dEQP.GLES3/functional_shaders_conversions_vector_to_vector_ivec2_to_bvec2_fragment:dEQP.GLES3/functional_fbo_completeness_renderable_renderbuffer_depth_red_float:dEQP.GLES3/functional_shaders_matrix_add_dynamic_highp_mat2x4_mat2x4_vertex:dEQP.GLES3/functional_ubo_single_basic_array_std140_column_major_mat4x3_fragment:dEQP.GLES3/functional_shaders_operator_binary_operator_left_shift_assign_result_mediump_uvec4_vertex:dEQP.GLES3/functional_shaders_preprocessor_operator_precedence_mul_vs_not_vertex:dEQP.GLES3/functional_shaders_matrix_add_dynamic_highp_mat3_mat3_vertex:dEQP.GLES3/functional_shaders_matrix_mul_const_mediump_mat2x3_mat4x2_vertex:dEQP.GLES3/functional_uniform_api_value_assigned_by_value_render_struct_in_array_uint_uvec4_vertex:dEQP.GLES3/functional_shaders_builtin_functions_precision_inversesqrt_highp_vertex_vec4:dEQP.GLES3/functional_shaders_indexing_matrix_subscript_mat3x2_dynamic_write_dynamic_loop_read_fragment:dEQP.GLES3/functional_shaders_declarations_invalid_declarations_in_in_fragment_main:dEQP.GLES3/functional_uniform_api_value_assigned_unused_uniforms_uint_uvec4_vertex:dEQP.GLES3/functional_buffer_map_read_usage_hints_copy_read_static_read:dEQP.GLES3/functional_shaders_derivate_fwidth_texture_float_fastest_vec3_mediump:dEQP.GLES3/functional_texture_wrap_astc_10x6_repeat_repeat_linear_not_divisible:dEQP.GLES3/functional_shaders_operator_binary_operator_mul_lowp_uint_uvec4_vertex:dEQP.GLES3/functional_fbo_completeness_renderable_texture_color0_depth24_stencil8:dEQP.GLES3/functional_shaders_struct_uniform_struct_array_vertex:dEQP.GLES3/functional_transform_feedback_basic_types_separate_lines_mediump_vec3:dEQP.GLES3/functional_shaders_conversions_scalar_to_matrix_uint_to_mat4x3_fragment:dEQP.GLES3/functional_shaders_indexing_matrix_subscript_mat2x3_static_write_dynamic_loop_read_fragment:dEQP.GLES3/functional_texture_format_sized_cube_rgb32i_npot:dEQP.GLES3/functional_vertex_arrays_single_attribute_usages_stream_draw_stride4_short_quads1:dEQP.GLES3/functional_texture_filtering_2d_array_sizes_3x7x5_linear:dEQP.GLES3/functional_uniform_api_value_assigned_by_value_render_basic_bvec3_vertex:dEQP.GLES3/functional_shaders_indexing_tmp_array_float_dynamic_write_dynamic_read_fragment:dEQP.GLES3/functional_uniform_api_value_initial_render_basic_mat4x3_vertex:dEQP.GLES3/functional_texture_filtering_3d_formats_rgba8_snorm_linear:dEQP.GLES3/functional_draw_draw_elements_instanced_indices_unaligned_user_ptr_index_int:dEQP.GLES3/functional_shaders_random_trigonometric_vertex_38:dEQP.GLES3/functional_shaders_operator_binary_operator_add_mediump_vec2_float_vertex:dEQP.GLES3/functional_draw_draw_range_elements_triangles_multiple_attributes:dEQP.GLES3/functional_shaders_matrix_div_dynamic_highp_mat2x3_mat2x3_vertex:dEQP.GLES3/functional_shaders_random_texture_fragment_135:dEQP.GLES3/functional_shaders_operator_common_functions_clamp_mediump_vec3_float_vertex:dEQP.GLES3/functional_uniform_api_value_assigned_by_pointer_get_uniform_nested_structs_arrays_uint_uvec4_vertex:dEQP.GLES3/functional_uniform_api_value_initial_render_basic_uvec3_vertex:dEQP.GLES3/functional_shaders_matrix_post_increment_highp_mat4x2_float_vertex:dEQP.GLES3/functional_shaders_swizzle_math_operations_vector_multiply_mediump_vec4_www_www_vertex:dEQP.GLES3/functional_shaders_random_scalar_conversion_combined_36:dEQP.GLES3/functional_texture_shadow_cube_nearest_mipmap_linear_less_or_equal_depth24_stencil8:dEQP.GLES3/functional_texture_filtering_2d_formats_etc1_rgb8_linear_mipmap_nearest:dEQP.GLES3/functional_shaders_matrix_pre_increment_highp_mat4_float_vertex:dEQP.GLES3/functional_texture_compressed_astc_weight_ise_5x5:dEQP.GLES3/functional_shaders_swizzles_vector_swizzles_mediump_bvec4_abgr_vertex:dEQP.GLES3/functional_shaders_random_exponential_vertex_49:dEQP.GLES3/functional_texture_wrap_eac_signed_rg11_clamp_repeat_linear_pot:dEQP.GLES3/functional_texture_swizzle_multi_channel_luminance_one_one_red_green:dEQP.GLES3/functional_fbo_render_recreate_color_rbo_rg8i_depth_stencil_rbo_depth24_stencil8:dEQP.GLES3/functional_shaders_operator_binary_operator_div_assign_result_lowp_uvec3_vertex:dEQP.GLES3/functional_texture_compressed_astc_block_size_remainder_8x6:dEQP.GLES3/functional_shaders_operator_binary_operator_div_assign_effect_lowp_uvec3_vertex:dEQP.GLES3/functional_shaders_matrix_mul_const_highp_mat2x4_mat4x2_vertex:dEQP.GLES3/functional_attribute_location_mixed_hole_uvec4:dEQP.GLES3/functional_shaders_operator_binary_operator_bitwise_or_assign_effect_mediump_uvec4_vertex:dEQP.GLES3/functional_shaders_random_scalar_conversion_vertex_92:dEQP.GLES3/functional_shaders_matrix_add_uniform_highp_mat3_mat3_vertex:dEQP.GLES3/functional_ubo_instance_array_basic_type_shared_column_major_mat2x3_fragment:dEQP.GLES3/functional_shaders_swizzles_vector_swizzles_mediump_ivec4_www_vertex:dEQP.GLES3/functional_texture_specification_basic_texsubimage2d_r8_cube:dEQP.GLES3/functional_shaders_texture_functions_textureoffset_usampler3d_vertex:dEQP.GLES3/functional_shaders_builtin_functions_precision_roundeven_lowp_fragment_vec4:dEQP.GLES3/functional_shaders_matrix_mul_dynamic_mediump_mat2x4_mat4x2_vertex:dEQP.GLES3/functional_shaders_operator_binary_operator_right_shift_assign_effect_lowp_uvec4_uint_vertex'
    tt2 = 'dEQP.GLES3/functional_uniform_api_value_assigned_by_pointer_get_uniform_basic_sampler2D_vertex:dEQP.GLES3/functional_shaders_operator_unary_operator_post_decrement_effect_highp_ivec2_fragment:dEQP.GLES3/functional_shaders_matrix_add_const_mediump_mat3x4_mat3x4_fragment:dEQP.GLES3/functional_draw_random_0:dEQP.GLES3/functional_shaders_operator_binary_operator_bitwise_and_assign_result_mediump_uvec3_fragment:dEQP.GLES3/functional_fragment_ops_blend_fbo_srgb_rgb_func_alpha_func_dst_one_minus_constant_color_src_alpha_saturate:dEQP.GLES3/functional_shaders_random_exponential_fragment_78:dEQP.GLES3/functional_texture_wrap_astc_10x6_mirror_clamp_nearest_divisible:dEQP.GLES3/functional_fragment_out_array_int_rg16i_highp_ivec4:dEQP.GLES3/functional_fragment_ops_blend_default_framebuffer_equation_src_func_dst_func_subtract_one_minus_constant_alpha_one_minus_constant_alpha:dEQP.GLES3/functional_shaders_operator_binary_operator_add_highp_ivec4_fragment:dEQP.GLES3/functional_transform_feedback_random_full_array_capture_interleaved_lines_4:dEQP.GLES3/functional_shaders_texture_functions_invalid_textureprojgradoffset_sampler2darray_vec4_vec2_vec2_ivec2_vertex:dEQP.GLES3/functional_shaders_constants_int_hexadecimal_0_vertex:dEQP.GLES3/functional_texture_compressed_astc_endpoint_ise_12x12_srgb:dEQP.GLES3/functional_texture_wrap_astc_10x5_repeat_clamp_nearest_not_divisible:dEQP.GLES3/functional_fragment_out_array_uint_r16ui_highp_uvec4:dEQP.GLES3/functional_shaders_operator_common_functions_min_highp_vec2_float_fragment:dEQP.GLES3/functional_ubo_single_basic_type_shared_column_major_mediump_mat2x4_both:dEQP.GLES3/functional_texture_mipmap_3d_basic_linear_linear_clamp:dEQP.GLES3/functional_shaders_arrays_invalid_constructor_c_style3_vertex:dEQP.GLES3/functional_shaders_texture_functions_textureproj_sampler2d_vec4_fixed_fragment:dEQP.GLES3/functional_shaders_operator_binary_operator_right_shift_assign_result_lowp_uvec3_int_fragment:dEQP.GLES3/functional_shaders_operator_binary_operator_div_highp_ivec4_fragment:dEQP.GLES3/functional_shaders_random_trigonometric_vertex_60:dEQP.GLES3/functional_shaders_operator_binary_operator_bitwise_xor_assign_result_highp_uvec4_uint_fragment:dEQP.GLES3/functional_shaders_builtin_functions_precision_floor_lowp_fragment_scalar:dEQP.GLES3/functional_shaders_swizzles_vector_swizzles_mediump_ivec4_r_fragment:dEQP.GLES3/functional_texture_shadow_2d_linear_never_depth24_stencil8:dEQP.GLES3/functional_texture_specification_basic_teximage3d_rgba8i_3d:dEQP.GLES3/functional_shaders_builtin_functions_precision_tan_mediump_vertex_scalar:dEQP.GLES3/functional_shaders_operator_binary_operator_left_shift_mediump_uvec3_fragment:dEQP.GLES3/functional_shaders_builtin_functions_precision_matrixcompmult_highp_vertex_mat3x2:dEQP.GLES3/functional_ubo_random_all_per_block_buffers_46:dEQP.GLES3/functional_shaders_operator_binary_operator_bitwise_xor_assign_effect_highp_uvec4_fragment:dEQP.GLES3/functional_texture_format_unsized_rgba_unsigned_byte_2d_array_pot:dEQP.GLES3/functional_shaders_conversions_vector_to_scalar_vec4_to_float_vertex:dEQP.GLES3/functional_shaders_operator_unary_operator_pre_decrement_result_highp_uvec4_fragment:dEQP.GLES3/functional_shaders_conversions_vector_combine_bvec2_int_to_vec3_vertex:dEQP.GLES3/functional_ubo_single_basic_type_shared_mediump_ivec3_both:dEQP.GLES3/functional_clipping_triangle_vertex_clip_three_clip_pos_x_pos_z_and_neg_x_pos_y_pos_z_and_pos_x_neg_y_neg_z:dEQP.GLES3/functional_shaders_matrix_pre_decrement_lowp_mat2x3_float_fragment:dEQP.GLES3/functional_state_query_internal_format_r8_samples:dEQP.GLES3/functional_shaders_matrix_mul_const_highp_mat2_mat2_fragment:dEQP.GLES3/functional_uniform_api_info_query_indices_active_uniformsiv_basic_array_sampler2D_vertex:dEQP.GLES3/functional_shaders_swizzles_vector_swizzles_mediump_ivec4_qqq_fragment:dEQP.GLES3/functional_transform_feedback_basic_types_separate_triangles_mediump_mat2:dEQP.GLES3/functional_shaders_functions_invalid_inout_local_vertex:dEQP.GLES3/functional_texture_filtering_2d_array_sizes_63x63x63_nearest_mipmap_linear:dEQP.GLES3/functional_fbo_color_texcube_rgba32ui:dEQP.GLES3/functional_fragment_ops_blend_fbo_srgb_equation_src_func_dst_func_add_one_minus_dst_alpha_constant_color:dEQP.GLES3/functional_ubo_random_nested_structs_arrays_1:dEQP.GLES3/functional_shaders_invalid_implicit_conversions_invalid_implicit_conversions_div_ivec3_uvec2_ivec3_fragment:dEQP.GLES3/functional_transform_feedback_basic_types_separate_triangles_mediump_ivec4:dEQP.GLES3/functional_shaders_operator_binary_operator_bitwise_and_assign_effect_mediump_uvec3_fragment:dEQP.GLES3/functional_uniform_api_value_assigned_by_pointer_get_uniform_basic_array_first_elem_without_brackets_mat4_row_major_vertex:dEQP.GLES3/functional_shaders_swizzle_math_operations_vector_divide_mediump_vec3_z_z_fragment:dEQP.GLES3/functional_ubo_single_nested_struct_array_per_block_buffer_packed_fragment:dEQP.GLES3/functional_shaders_matrix_mul_dynamic_mediump_mat3x4_mat2x3_fragment:dEQP.GLES3/functional_shaders_invalid_implicit_conversions_invalid_implicit_conversions_mul_float_ivec3_float_fragment:dEQP.GLES3/functional_draw_random_183:dEQP.GLES3/functional_transform_feedback_array_separate_lines_mediump_mat4x3:dEQP.GLES3/functional_vertex_arrays_single_attribute_usages_dynamic_draw_stride17_byte_quads1:dEQP.GLES3/functional_shaders_builtin_functions_precision_asin_highp_fragment_scalar:dEQP.GLES3/functional_shaders_indexing_varying_array_vec3_dynamic_loop_write_static_read:dEQP.GLES3/functional_uniform_api_value_assigned_by_pointer_get_uniform_basic_int_vertex:dEQP.GLES3/functional_uniform_api_value_assigned_by_value_get_uniform_basic_array_bvec4_api_float_vertex:dEQP.GLES3/functional_fbo_completeness_renderable_renderbuffer_depth_r32ui:dEQP.GLES3/functional_shaders_random_scalar_conversion_combined_93:dEQP.GLES3/functional_shaders_operator_angle_and_trigonometry_atan2_mediump_vec2_fragment:dEQP.GLES3/functional_shaders_preprocessor_recursion_recursion_4_fragment:dEQP.GLES3/functional_shaders_matrix_mul_dynamic_lowp_mat2_mat2_fragment:dEQP.GLES3/functional_shaders_matrix_mul_uniform_highp_mat4x3_mat2x4_fragment:dEQP.GLES3/functional_shaders_operator_binary_operator_mul_highp_vec2_fragment:dEQP.GLES3/functional_fragment_ops_blend_fbo_srgb_rgb_func_alpha_func_dst_src_alpha_saturate_one_minus_dst_color:dEQP.GLES3/functional_shaders_operator_common_functions_clamp_highp_vec3_fragment:dEQP.GLES3/functional_ubo_instance_array_basic_type_shared_row_major_mat4x2_fragment:dEQP.GLES3/functional_fbo_color_tex2darray_r8:dEQP.GLES3/functional_attribute_location_mixed_max_attributes_mat2:dEQP.GLES3/functional_shaders_matrix_div_assign_lowp_mat3x4_float_fragment:dEQP.GLES3/functional_shaders_derivate_dfdx_nicest_fbo_float_vec2_mediump:dEQP.GLES3/functional_shaders_operator_binary_operator_div_highp_vec2_fragment:dEQP.GLES3/functional_shaders_operator_binary_operator_left_shift_assign_result_highp_ivec4_uvec4_fragment:dEQP.GLES3/functional_shaders_invalid_implicit_conversions_invalid_implicit_conversions_add_int_uvec4_int_fragment:dEQP.GLES3/functional_shaders_derivate_fwidth_texture_msaa4_float_mediump:dEQP.GLES3/functional_texture_specification_basic_teximage3d_rg8ui_3d:dEQP.GLES3/functional_texture_format_sized_2d_rgb8ui_pot:dEQP.GLES3/functional_shaders_builtin_functions_common_isnan_float_highp_vertex:dEQP.GLES3/functional_shaders_random_texture_fragment_108:dEQP.GLES3/functional_fragment_ops_depth_stencil_stencil_ops_decr_wrap_zero_zero:dEQP.GLES3/functional_shaders_builtin_functions_common_roundeven_vec3_highp_vertex:dEQP.GLES3/functional_texture_units_4_units_only_2d_array_7:dEQP.GLES3/functional_shaders_swizzle_math_operations_vector_subtract_mediump_ivec4_wzy_zyw_fragment:dEQP.GLES3/functional_buffer_write_basic_pixel_unpack_static_copy:dEQP.GLES3/functional_fbo_completeness_renderable_renderbuffer_depth_rgb8_snorm:dEQP.GLES3/functional_shaders_swizzle_math_operations_vector_subtract_mediump_vec4_www_www_fragment:dEQP.GLES3/functional_shaders_swizzles_vector_swizzles_mediump_bvec2_xy_fragment:dEQP.GLES3/functional_buffer_map_write_usage_hints_transform_feedback_static_read:dEQP.GLES3/functional_shaders_operator_binary_operator_bitwise_or_lowp_uint_uvec3_fragment:dEQP.GLES3/functional_texture_vertex_2d_array_wrap_repeat_mirror:dEQP.GLES3/functional_shaders_preprocessor_semantic_ops_as_arguments_fragment:dEQP.GLES3/functional_fragment_out_basic_fixed_srgb8_alpha8_highp_vec4:dEQP.GLES3/functional_negative_api_vertex_array_vertex_attrib_pointer:dEQP.GLES3/functional_primitive_restart_begin_restart_triangle_strip_unsigned_int_draw_elements_instanced:dEQP.GLES3/functional_ubo_single_basic_type_packed_lowp_mat2_vertex:dEQP.GLES3/functional_shaders_operator_angle_and_trigonometry_cos_mediump_vec2_fragment:dEQP.GLES3/functional_shaders_operator_binary_operator_right_shift_highp_ivec4_fragment:dEQP.GLES3/functional_texture_specification_texstorage2d_format_rgb16ui_cube:dEQP.GLES3/functional_shaders_invalid_implicit_conversions_invalid_implicit_conversions_mul_ivec4_uint_ivec4_fragment:dEQP.GLES3/functional_texture_wrap_astc_4x4_srgb_clamp_clamp_nearest_divisible:dEQP.GLES3/functional_clipping_triangle_vertex_clip_one_clip_pos_x:dEQP.GLES3/functional_shaders_swizzle_math_operations_vector_subtract_mediump_ivec2_xy_yx_fragment:dEQP.GLES3/functional_ubo_instance_array_basic_type_std140_ivec3_both:dEQP.GLES3/functional_shaders_return_output_write_always_fragment:dEQP.GLES3/functional_fragment_ops_blend_fbo_srgb_equation_src_func_dst_func_subtract_one_minus_dst_color_one_minus_constant_alpha:dEQP.GLES3/functional_texture_mipmap_2d_basic_linear_linear_clamp_npot:dEQP.GLES3/functional_primitive_restart_end_restart_duplicate_restarts_triangles_unsigned_short_draw_elements_instanced:dEQP.GLES3/functional_shaders_operator_int_compare_greaterThan_mediump_ivec3_fragment:dEQP.GLES3/functional_texture_shadow_2d_array_nearest_never_depth24_stencil8:dEQP.GLES3/functional_shaders_random_trigonometric_vertex_47:dEQP.GLES3/functional_shaders_operator_binary_operator_div_assign_result_highp_vec2_float_fragment:dEQP.GLES3/functional_shaders_constants_uint_from_int_vertex:dEQP.GLES3/functional_shaders_invalid_implicit_conversions_invalid_implicit_conversions_mul_vec3_int_vec3_fragment:dEQP.GLES3/functional_shaders_indexing_matrix_subscript_mat4x3_static_loop_write_static_read_vertex:dEQP.GLES3/functional_shaders_operator_geometric_distance_mediump_vec4_fragment:dEQP.GLES3/functional_transform_feedback_random_interleaved_lines_4:dEQP.GLES3/functional_shaders_random_exponential_vertex_82:dEQP.GLES3/functional_shaders_conditionals_if_mixed_if_elseif_else_vertex:dEQP.GLES3/functional_attribute_location_mixed_relink_hole_mat4x4:dEQP.GLES3/functional_shaders_operator_binary_operator_div_assign_effect_mediump_ivec4_int_fragment:dEQP.GLES3/functional_shaders_operator_binary_operator_right_shift_assign_effect_highp_uvec4_uint_fragment:dEQP.GLES3/functional_shaders_random_basic_expression_fragment_73:dEQP.GLES3/functional_texture_compressed_astc_random_8x5_srgb:dEQP.GLES3/functional_primitive_restart_basic_triangles_unsigned_byte_draw_elements_instanced:dEQP.GLES3/functional_shaders_linkage_uniform_basic_precision_conflict_4:dEQP.GLES3/functional_shaders_indexing_matrix_subscript_mat2x3_dynamic_loop_write_static_loop_read_vertex:dEQP.GLES3/functional_shaders_conversions_vector_combine_int_ivec2_int_to_vec4_vertex:dEQP.GLES3/functional_shaders_arrays_constructor_float4_vertex:dEQP.GLES3/functional_shaders_conversions_scalar_to_matrix_uint_to_mat4_vertex:dEQP.GLES3/functional_vertex_arrays_single_attribute_first_byte_first6_offset1_stride2_quads256:dEQP.GLES3/functional_shaders_operator_binary_operator_div_mediump_int_ivec4_fragment:dEQP.GLES3/functional_ubo_single_nested_struct_mixed_matrix_packing_per_block_buffer_std140_block_row_major_matrix_column_major_matrixarray_column_major_instance_array_both:dEQP.GLES3/functional_fragment_ops_blend_default_framebuffer_rgb_func_alpha_func_dst_one_minus_src_color_src_color:dEQP.GLES3/functional_shaders_matrix_add_const_mediump_mat4x3_mat4x3_fragment:dEQP.GLES3/functional_shaders_texture_functions_texturegradoffset_sampler3d_float_vertex:dEQP.GLES3/functional_shaders_invalid_implicit_conversions_invalid_implicit_conversions_mul_vec2_ivec2_vec2_fragment:dEQP.GLES3/functional_fragment_out_basic_fixed_rg8_highp_vec4:dEQP.GLES3/functional_shaders_random_swizzle_fragment_27:dEQP.GLES3/functional_ubo_instance_array_basic_type_std140_column_major_mat4x3_both:dEQP.GLES3/functional_shaders_operator_binary_operator_sub_highp_ivec4_fragment:dEQP.GLES3/functional_shaders_operator_binary_operator_right_shift_highp_uvec4_ivec4_fragment:dEQP.GLES3/functional_fbo_completeness_renderable_renderbuffer_depth_rgba16ui:dEQP.GLES3/functional_shaders_random_basic_expression_vertex_29:dEQP.GLES3/functional_fragment_out_array_fixed_srgb8_alpha8_highp_vec4:dEQP.GLES3/functional_buffer_map_read_usage_hints_transform_feedback_static_read:dEQP.GLES3/functional_texture_wrap_astc_8x5_clamp_clamp_nearest_divisible:dEQP.GLES3/functional_texture_specification_basic_texsubimage2d_rgb32ui_2d:dEQP.GLES3/functional_texture_size_cube_16x16_rgba4444:dEQP.GLES3/functional_fbo_invalidate_sub_unbind_blit_color:dEQP.GLES3/functional_shaders_scoping_valid_while_condition_variable_hides_global_variable_fragment:dEQP.GLES3/functional_ubo_single_basic_array_std140_mat2x3_fragment:dEQP.GLES3/functional_transform_feedback_array_element_separate_points_mediump_uvec2:dEQP.GLES3/functional_read_pixels_rowlength_rgba_uint_19:dEQP.GLES3/functional_negative_api_texture_teximage2d_invalid_buffer_target:dEQP.GLES3/functional_shaders_invalid_implicit_conversions_invalid_implicit_conversions_sub_vec2_ivec2_vec2_fragment:dEQP.GLES3/functional_texture_wrap_astc_4x4_srgb_repeat_clamp_nearest_not_divisible:dEQP.GLES3/functional_buffer_map_write_usage_hints_copy_read_stream_read:dEQP.GLES3/functional_shaders_random_conditionals_combined_13:dEQP.GLES3/functional_shaders_operator_binary_operator_div_assign_effect_highp_float_fragment:dEQP.GLES3/functional_fragment_out_basic_uint_rg16ui_highp_uvec4:dEQP.GLES3/functional_shaders_operator_binary_operator_bitwise_xor_assign_result_lowp_uvec3_uint_fragment:dEQP.GLES3/functional_shaders_swizzles_vector_swizzles_mediump_ivec4_abba_fragment:dEQP.GLES3/functional_shaders_matrix_sub_const_mediump_mat3x2_mat3x2_fragment:dEQP.GLES3/functional_texture_specification_teximage3d_pbo_rgb8_skip_images_3d:dEQP.GLES3/functional_texture_format_sized_3d_rgb32ui_pot:dEQP.GLES3/functional_fragment_ops_blend_fbo_srgb_rgb_func_alpha_func_src_src_alpha_src_color:dEQP.GLES3/functional_shaders_loops_short_circuit_while_fragment:dEQP.GLES3/functional_fragment_ops_blend_fbo_srgb_equation_src_func_dst_func_subtract_src_color_constant_color:dEQP.GLES3/functional_vertex_arrays_single_attribute_strides_int2_10_10_10_user_ptr_stride32_components4_quads1:dEQP.GLES3/functional_ubo_random_basic_arrays_4:dEQP.GLES3/functional_shaders_random_all_features_fragment_84:dEQP.GLES3/functional_ubo_random_all_per_block_buffers_10:dEQP.GLES3/functional_shaders_builtin_functions_precision_distance_highp_vertex_scalar:dEQP.GLES3/functional_buffer_map_write_explicit_flush_copy_read_all:dEQP.GLES3/functional_shaders_operator_binary_operator_left_shift_lowp_uvec3_uint_fragment:dEQP.GLES3/functional_fbo_blit_depth_stencil_depth32f_stencil8_depth_only:dEQP.GLES3/functional_shaders_keywords_keywords_mat3x2_vertex:dEQP.GLES3/functional_shaders_conversions_vector_combine_uint_float_to_uvec2_vertex:dEQP.GLES3/functional_negative_api_texture_texstorage2d_invalid_astc_target:dEQP.GLES3/functional_shaders_texture_functions_invalid_texelfetch_usamplercube_ivec3_int_vertex:dEQP.GLES3/functional_texture_filtering_3d_combinations_nearest_nearest_clamp_repeat_repeat:dEQP.GLES3/functional_fragment_out_basic_int_rgba8i_highp_ivec4:dEQP.GLES3/functional_shaders_preprocessor_undefined_identifiers_undefined_identifier_8_fragment:dEQP.GLES3/functional_vertex_arrays_single_attribute_output_types_byte_components2_vec2_quads1:dEQP.GLES3/functional_texture_wrap_astc_8x8_srgb_mirror_clamp_nearest_divisible:dEQP.GLES3/functional_shaders_operator_binary_operator_bitwise_and_highp_uint_uvec4_fragment:dEQP.GLES3/functional_texture_filtering_3d_combinations_linear_mipmap_linear_nearest_repeat_clamp_repeat:dEQP.GLES3/functional_state_query_integers_subpixel_bits_getfloat:dEQP.GLES3/functional_shaders_operator_common_functions_clamp_highp_uint_fragment:dEQP.GLES3/functional_shaders_functions_misc_multi_arg_int_vertex:dEQP.GLES3/functional_fbo_render_shared_colorbuffer_tex2d_r11f_g11f_b10f_depth_stencil_rbo_depth24_stencil8:dEQP.GLES3/functional_shaders_random_trigonometric_vertex_72:dEQP.GLES3/functional_shaders_matrix_div_assign_lowp_mat4x3_float_fragment:dEQP.GLES3/functional_fragment_ops_depth_stencil_stencil_ops_decr_decr_zero:dEQP.GLES3/functional_shaders_swizzle_math_operations_vector_subtract_mediump_ivec3_zzz_zzz_fragment:dEQP.GLES3/functional_implementation_limits_max_color_attachments:dEQP.GLES3/functional_shaders_operator_unary_operator_post_increment_result_highp_ivec4_fragment:dEQP.GLES3/functional_buffer_write_basic_copy_read_static_copy:dEQP.GLES3/functional_transform_feedback_array_element_interleaved_lines_mediump_mat2x4:dEQP.GLES3/functional_fragment_ops_depth_stencil_stencil_ops_invert_incr_wrap_invert:dEQP.GLES3/functional_shaders_texture_functions_textureprojgrad_sampler2d_vec3_float_vertex:dEQP.GLES3/functional_shaders_operator_float_compare_greaterThanEqual_mediump_vec2_fragment:dEQP.GLES3/functional_shaders_random_scalar_conversion_fragment_61:dEQP.GLES3/functional_shaders_operator_common_functions_clamp_highp_uvec2_uint_fragment:dEQP.GLES3/functional_shaders_conversions_vector_to_vector_ivec3_to_vec3_vertex:dEQP.GLES3/functional_fbo_blit_rect_out_of_bounds_reverse_dst_x_linear:dEQP.GLES3/functional_shaders_operator_binary_operator_add_assign_effect_lowp_ivec3_int_fragment:dEQP.GLES3/functional_state_query_shader_shader_type:dEQP.GLES3/functional_draw_draw_arrays_instanced_lines_multiple_attributes:dEQP.GLES3/functional_fragment_ops_blend_fbo_srgb_rgb_func_alpha_func_dst_constant_alpha_one_minus_constant_color:dEQP.GLES3/functional_fragment_ops_blend_default_framebuffer_equation_src_func_dst_func_add_constant_color_one_minus_src_alpha:dEQP.GLES3/functional_shaders_builtin_functions_precision_inversesqrt_highp_vertex_scalar:dEQP.GLES3/functional_shaders_constant_expressions_builtin_functions_exponential_exp2_vec3_fragment:dEQP.GLES3/functional_shaders_operator_binary_operator_right_shift_assign_effect_mediump_ivec3_fragment:dEQP.GLES3/functional_shaders_indexing_vector_subscript_vec3_dynamic_loop_subscript_write_direct_read_vertex:dEQP.GLES3/functional_shaders_invalid_implicit_conversions_invalid_implicit_conversions_add_ivec4_uint_ivec4_fragment:dEQP.GLES3/functional_fbo_color_tex3d_rgba32ui:dEQP.GLES3/functional_shaders_operator_geometric_faceforward_mediump_vec3_fragment:dEQP.GLES3/functional_shaders_functions_qualifiers_in_highp_int_vertex:dEQP.GLES3/functional_shaders_operator_selection_mediump_ivec3_fragment:dEQP.GLES3/functional_fragment_ops_blend_fbo_srgb_equation_src_func_dst_func_reverse_subtract_one_minus_dst_alpha_dst_color:dEQP.GLES3/functional_shaders_operator_binary_operator_right_shift_mediump_uvec3_ivec3_fragment:dEQP.GLES3/functional_shaders_operator_binary_operator_mul_mediump_uvec4_uint_fragment:dEQP.GLES3/functional_shaders_preprocessor_conditionals_ifdef_3_fragment:dEQP.GLES3/functional_texture_shadow_cube_nearest_mipmap_nearest_never_depth24_stencil8:dEQP.GLES3/functional_texture_wrap_etc2_eac_rgba8_repeat_clamp_nearest_npot:dEQP.GLES3/functional_fbo_render_shared_colorbuffer_tex2d_r8_depth_stencil_rbo_depth24_stencil8:dEQP.GLES3/functional_shaders_matrix_mul_assign_lowp_mat4_float_fragment:dEQP.GLES3/functional_uniform_api_value_assigned_by_value_get_uniform_struct_in_array_uint_uvec4_vertex:dEQP.GLES3/functional_shaders_preprocessor_expressions_or_fragment:dEQP.GLES3/functional_texture_filtering_2d_array_combinations_linear_linear_mirror_repeat:dEQP.GLES3/functional_fragment_out_array_uint_rg16ui_highp_uvec4:dEQP.GLES3/functional_shaders_matrix_pre_increment_lowp_mat4x2_float_fragment:dEQP.GLES3/functional_shaders_keywords_keywords_while_vertex:dEQP.GLES3/functional_shaders_operator_unary_operator_post_increment_effect_highp_ivec4_fragment:dEQP.GLES3/functional_ubo_random_nested_structs_14:dEQP.GLES3/functional_texture_filtering_2d_sizes_127x99_nearest:dEQP.GLES3/functional_shaders_operator_binary_operator_div_lowp_ivec3_int_fragment:dEQP.GLES3/functional_shaders_qualification_order_variables_invalid_precision_storage_invariant:dEQP.GLES3/functional_fragment_ops_blend_fbo_srgb_rgb_func_alpha_func_dst_one_minus_src_color_src_color:dEQP.GLES3/functional_shaders_texture_functions_texturelodoffset_isampler2d_fragment:dEQP.GLES3/functional_attribute_location_mixed_hole_uint:dEQP.GLES3/functional_fragment_ops_blend_fbo_srgb_rgb_equation_alpha_equation_add_reverse_subtract:dEQP.GLES3/functional_shaders_preprocessor_expressions_defined_3_fragment:dEQP.GLES3/functional_texture_wrap_etc2_rgb8_punchthrough_alpha1_clamp_clamp_nearest_pot:dEQP.GLES3/functional_uniform_api_value_initial_get_uniform_basic_ivec2_vertex'
    #tt2 = ':'.join(sorted(tt2.split(':')))

    for i in range(runs):
        deqp_flags = [
            #'--deqp-log-images=disable',
            '--deqp-log-shader-sources=disable',
            '--deqp-log-flush=disable',
        ]
        exit_code, output = RunTests(
            suite, ['--use-angle=vulkan', '-v', '--gtest_filter=' + tt2] + deqp_flags,
            log_output=False)
        for ln in output.decode().split('\n'):
            if 'tests from dEQP (' in ln or 'functional_shaders_operator_binary_operator_right_shift_assign_effect_lowp_uvec4_uint_vertex (' in ln or 'qwe' in ln:
                logging.info(ln)

        if exit_code:
            break

    return True
