#!/usr/bin/env vpython3
# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Runs an isolated non-Telemetry ANGLE test.

The main contract is that the caller passes the arguments:

  --isolated-script-test-output=[FILENAME]
json is written to that file in the format:
https://chromium.googlesource.com/chromium/src/+/main/docs/testing/json_test_results_format.md

Optional argument:

  --isolated-script-test-filter=[TEST_NAMES]

is a double-colon-separated ("::") list of test names, to run just that subset
of tests. This list is parsed by this harness and sent down via the
--gtest_filter argument.

This script is intended to be the base command invoked by the isolate,
followed by a subsequent non-python executable. For a similar script see
run_performance_test.py.
"""

import argparse
import json
import logging
import os
import pathlib
import shutil
import sys
import tempfile
import traceback


PY_UTILS = str(pathlib.Path(__file__).resolve().parents[1] / 'src' / 'tests' / 'py_utils')
if PY_UTILS not in sys.path:
    os.stat(PY_UTILS) and sys.path.insert(0, PY_UTILS)
import android_helper
import angle_path_util
import angle_test_util

angle_path_util.AddDepsDirToPath('testing/scripts')
import common
import xvfb
import test_env

# Unfortunately we need to copy these variables from
# //src/testing/scripts/test_env.py. Importing it and using its
# get_sandbox_env breaks test runs on Linux (it seems to unset DISPLAY).
CHROME_SANDBOX_ENV = 'CHROME_DEVEL_SANDBOX'
CHROME_SANDBOX_PATH = '/opt/chromium/chrome_sandbox'


def IsWindows():
    return sys.platform == 'cygwin' or sys.platform.startswith('win')


def runTest():
    suite = 'angle_deqp_gles3_tests'
    if not android_helper.ApkFileExists(suite):
        return False

    angle_test_util.setupLogging('INFO')

    android_helper.PrepareTestSuite(suite)

    tt2 = 'dEQP.GLES3/functional_shaders_matrix_post_decrement_highp_mat2x3_float_vertex:dEQP.GLES3/functional_clipping_triangle_vertex_clip_two_clip_neg_x_and_neg_y:dEQP.GLES3/functional_ubo_single_basic_type_packed_mediump_mat3x4_fragment:dEQP.GLES3/functional_shaders_conversions_vector_to_vector_ivec2_to_bvec2_fragment:dEQP.GLES3/functional_fbo_completeness_renderable_renderbuffer_depth_red_float:dEQP.GLES3/functional_shaders_matrix_add_dynamic_highp_mat2x4_mat2x4_vertex:dEQP.GLES3/functional_ubo_single_basic_array_std140_column_major_mat4x3_fragment:dEQP.GLES3/functional_shaders_operator_binary_operator_left_shift_assign_result_mediump_uvec4_vertex:dEQP.GLES3/functional_shaders_preprocessor_operator_precedence_mul_vs_not_vertex:dEQP.GLES3/functional_shaders_matrix_add_dynamic_highp_mat3_mat3_vertex:dEQP.GLES3/functional_shaders_matrix_mul_const_mediump_mat2x3_mat4x2_vertex:dEQP.GLES3/functional_uniform_api_value_assigned_by_value_render_struct_in_array_uint_uvec4_vertex:dEQP.GLES3/functional_shaders_builtin_functions_precision_inversesqrt_highp_vertex_vec4:dEQP.GLES3/functional_shaders_indexing_matrix_subscript_mat3x2_dynamic_write_dynamic_loop_read_fragment:dEQP.GLES3/functional_shaders_declarations_invalid_declarations_in_in_fragment_main:dEQP.GLES3/functional_uniform_api_value_assigned_unused_uniforms_uint_uvec4_vertex:dEQP.GLES3/functional_buffer_map_read_usage_hints_copy_read_static_read:dEQP.GLES3/functional_shaders_derivate_fwidth_texture_float_fastest_vec3_mediump:dEQP.GLES3/functional_texture_wrap_astc_10x6_repeat_repeat_linear_not_divisible:dEQP.GLES3/functional_shaders_operator_binary_operator_mul_lowp_uint_uvec4_vertex:dEQP.GLES3/functional_fbo_completeness_renderable_texture_color0_depth24_stencil8:dEQP.GLES3/functional_shaders_struct_uniform_struct_array_vertex:dEQP.GLES3/functional_transform_feedback_basic_types_separate_lines_mediump_vec3:dEQP.GLES3/functional_shaders_conversions_scalar_to_matrix_uint_to_mat4x3_fragment:dEQP.GLES3/functional_shaders_indexing_matrix_subscript_mat2x3_static_write_dynamic_loop_read_fragment:dEQP.GLES3/functional_texture_format_sized_cube_rgb32i_npot:dEQP.GLES3/functional_vertex_arrays_single_attribute_usages_stream_draw_stride4_short_quads1:dEQP.GLES3/functional_texture_filtering_2d_array_sizes_3x7x5_linear:dEQP.GLES3/functional_uniform_api_value_assigned_by_value_render_basic_bvec3_vertex:dEQP.GLES3/functional_shaders_indexing_tmp_array_float_dynamic_write_dynamic_read_fragment:dEQP.GLES3/functional_uniform_api_value_initial_render_basic_mat4x3_vertex:dEQP.GLES3/functional_texture_filtering_3d_formats_rgba8_snorm_linear:dEQP.GLES3/functional_draw_draw_elements_instanced_indices_unaligned_user_ptr_index_int:dEQP.GLES3/functional_shaders_random_trigonometric_vertex_38:dEQP.GLES3/functional_shaders_operator_binary_operator_add_mediump_vec2_float_vertex:dEQP.GLES3/functional_draw_draw_range_elements_triangles_multiple_attributes:dEQP.GLES3/functional_shaders_matrix_div_dynamic_highp_mat2x3_mat2x3_vertex:dEQP.GLES3/functional_shaders_random_texture_fragment_135:dEQP.GLES3/functional_shaders_operator_common_functions_clamp_mediump_vec3_float_vertex:dEQP.GLES3/functional_uniform_api_value_assigned_by_pointer_get_uniform_nested_structs_arrays_uint_uvec4_vertex:dEQP.GLES3/functional_uniform_api_value_initial_render_basic_uvec3_vertex:dEQP.GLES3/functional_shaders_matrix_post_increment_highp_mat4x2_float_vertex:dEQP.GLES3/functional_shaders_swizzle_math_operations_vector_multiply_mediump_vec4_www_www_vertex:dEQP.GLES3/functional_shaders_random_scalar_conversion_combined_36:dEQP.GLES3/functional_texture_shadow_cube_nearest_mipmap_linear_less_or_equal_depth24_stencil8:dEQP.GLES3/functional_texture_filtering_2d_formats_etc1_rgb8_linear_mipmap_nearest:dEQP.GLES3/functional_shaders_matrix_pre_increment_highp_mat4_float_vertex:dEQP.GLES3/functional_texture_compressed_astc_weight_ise_5x5:dEQP.GLES3/functional_shaders_swizzles_vector_swizzles_mediump_bvec4_abgr_vertex:dEQP.GLES3/functional_shaders_random_exponential_vertex_49:dEQP.GLES3/functional_texture_wrap_eac_signed_rg11_clamp_repeat_linear_pot:dEQP.GLES3/functional_texture_swizzle_multi_channel_luminance_one_one_red_green:dEQP.GLES3/functional_fbo_render_recreate_color_rbo_rg8i_depth_stencil_rbo_depth24_stencil8:dEQP.GLES3/functional_shaders_operator_binary_operator_div_assign_result_lowp_uvec3_vertex:dEQP.GLES3/functional_texture_compressed_astc_block_size_remainder_8x6:dEQP.GLES3/functional_shaders_operator_binary_operator_div_assign_effect_lowp_uvec3_vertex:dEQP.GLES3/functional_shaders_matrix_mul_const_highp_mat2x4_mat4x2_vertex:dEQP.GLES3/functional_attribute_location_mixed_hole_uvec4:dEQP.GLES3/functional_shaders_operator_binary_operator_bitwise_or_assign_effect_mediump_uvec4_vertex:dEQP.GLES3/functional_shaders_random_scalar_conversion_vertex_92:dEQP.GLES3/functional_shaders_matrix_add_uniform_highp_mat3_mat3_vertex:dEQP.GLES3/functional_ubo_instance_array_basic_type_shared_column_major_mat2x3_fragment:dEQP.GLES3/functional_shaders_swizzles_vector_swizzles_mediump_ivec4_www_vertex:dEQP.GLES3/functional_texture_specification_basic_texsubimage2d_r8_cube:dEQP.GLES3/functional_shaders_texture_functions_textureoffset_usampler3d_vertex:dEQP.GLES3/functional_shaders_builtin_functions_precision_roundeven_lowp_fragment_vec4:dEQP.GLES3/functional_shaders_matrix_mul_dynamic_mediump_mat2x4_mat4x2_vertex:dEQP.GLES3/functional_shaders_operator_binary_operator_right_shift_assign_effect_lowp_uvec4_uint_vertex'

    for i in range(30):
        _, output = android_helper.RunTests(
            suite, ['--use-angle=vulkan', '-v', '--gtest_filter=' + tt2], log_output=False)
        for ln in output.decode().split('\n'):
            if '8 tests from dEQP (' in ln or 'functional_shaders_operator_binary_operator_right_shift_assign_effect_lowp_uvec4_uint_vertex (' in ln or 'qwe' in ln:
                logging.info(ln)

        #android_helper.DumpFrequncies()

    return True


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('executable', help='Test executable.')
    parser.add_argument('--isolated-script-test-output', type=str)
    parser.add_argument('--isolated-script-test-filter', type=str)
    parser.add_argument('--xvfb', help='Start xvfb.', action='store_true')

    # Kept for compatiblity.
    # TODO(jmadill): Remove when removed from the recipes. http://crbug.com/954415
    parser.add_argument('--isolated-script-test-perf-output', type=str)

    args, extra_flags = parser.parse_known_args()

    env = os.environ.copy()

    if runTest():
        return 1

    return 0

    if 'GTEST_TOTAL_SHARDS' in env:
        extra_flags += ['--shard-count=' + env['GTEST_TOTAL_SHARDS']]
        env.pop('GTEST_TOTAL_SHARDS')
    if 'GTEST_SHARD_INDEX' in env:
        extra_flags += ['--shard-index=' + env['GTEST_SHARD_INDEX']]
        env.pop('GTEST_SHARD_INDEX')
    if 'ISOLATED_OUTDIR' in env:
        extra_flags += ['--isolated-outdir=' + env['ISOLATED_OUTDIR']]
        env.pop('ISOLATED_OUTDIR')

    # Assume we want to set up the sandbox environment variables all the
    # time; doing so is harmless on non-Linux platforms and is needed
    # all the time on Linux.
    env[CHROME_SANDBOX_ENV] = CHROME_SANDBOX_PATH

    rc = 0
    try:
        # Consider adding stdio control flags.
        if args.isolated_script_test_output:
            extra_flags.append('--isolated-script-test-output=%s' %
                               args.isolated_script_test_output)

        if args.isolated_script_test_filter:
            filter_list = common.extract_filter_list(args.isolated_script_test_filter)
            extra_flags.append('--gtest_filter=' + ':'.join(filter_list))

        if IsWindows():
            args.executable = '.\\%s.exe' % args.executable
        else:
            args.executable = './%s' % args.executable
        with common.temporary_file() as tempfile_path:
            env['CHROME_HEADLESS'] = '1'
            cmd = [args.executable] + extra_flags

            if args.xvfb:
                rc = xvfb.run_executable(cmd, env, stdoutfile=tempfile_path)
            else:
                rc = test_env.run_command_with_output(cmd, env=env, stdoutfile=tempfile_path)

    except Exception:
        traceback.print_exc()
        rc = 1

    return rc


# This is not really a "script test" so does not need to manually add
# any additional compile targets.
def main_compile_targets(args):
    json.dump([], args.output)


if __name__ == '__main__':
    # Conform minimally to the protocol defined by ScriptTest.
    if 'compile_targets' in sys.argv:
        funcs = {
            'run': None,
            'compile_targets': main_compile_targets,
        }
        sys.exit(common.run_script(sys.argv[1:], funcs))
    sys.exit(main())
