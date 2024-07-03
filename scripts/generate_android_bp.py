#  Copyright The ANGLE Project Authors. All rights reserved.
#  Use of this source code is governed by a BSD-style license that can be
#  found in the LICENSE file.
#
# Generates an Android.bp file from the json output of 4 'gn desc' commands.
# Invoked during Skia rolls by roll_aosp.sh. For local testing, see:
#  scripts/roll_aosp.sh --genAndroidBp

import json
import sys
import re
import os
import argparse
import functools
import collections
import unittest
import typing
from typing import List, Tuple, Set, Dict
from pprint import pprint

ROOT_TARGETS = [
    "//:libGLESv2",
    "//:libGLESv1_CM",
    "//:libEGL",
]

MIN_SDK_VERSION = '28'
TARGET_SDK_VERSION = '33'
STL = 'libc++_static'

ABI_ARM = 'arm'
ABI_ARM64 = 'arm64'
ABI_X86 = 'x86'
ABI_X64 = 'x86_64'

ABI_TARGETS = [ABI_ARM, ABI_ARM64, ABI_X86, ABI_X64]


def gn_abi(abi):
    # gn uses x64, rather than x86_64
    return 'x64' if abi == ABI_X64 else abi


# Makes dict cache-able "by reference" (assumed not to be mutated)
class BuildInfo(dict):

    def __hash__(self):
        return 0

    def __eq__(self, other):
        return self is other


class FlaggedModuleType(typing.NamedTuple):
    """
    Information required to define a soong_config_module_type with properties
    adjustable with a config flag.
    - Name of the "base" module type that's being extended
    - Name of the "flagged" module being defined
    - List of properties that change depending on the flag value
    """
    base_module_type: str
    flagged_module_type: str
    differing_properties: List[str]


def tabs(indent):
    return ' ' * (indent * 4)


def has_child_values(value):
    # Elements of the blueprint can be pruned if they are empty lists or dictionaries of empty
    # lists
    if isinstance(value, list):
        return len(value) > 0
    if isinstance(value, dict):
        for (item, item_value) in value.items():
            if has_child_values(item_value):
                return True
        return False

    # This is a value leaf node
    return True


def write_blueprint_key_value(output, name, value, indent=1):
    if not has_child_values(value):
        return

    if isinstance(value, set) or isinstance(value, list):
        value = list(sorted(set(value)))

    if isinstance(value, list):
        output.append(tabs(indent) + '%s: [' % name)
        for item in value:
            output.append(tabs(indent + 1) + '"%s",' % item)
        output.append(tabs(indent) + '],')
        return
    if isinstance(value, dict):
        if not value:
            return
        output.append(tabs(indent) + '%s: {' % name)
        for (item, item_value) in value.items():
            write_blueprint_key_value(output, item, item_value, indent + 1)
        output.append(tabs(indent) + '},')
        return
    if isinstance(value, bool):
        output.append(tabs(indent) + '%s: %s,' % (name, 'true' if value else 'false'))
        return
    output.append(tabs(indent) + '%s: "%s",' % (name, value))


def write_blueprint(output, target_type, values):
    if target_type == 'license':
        comment = """
// Added automatically by a large-scale-change that took the approach of
// 'apply every license found to every target'. While this makes sure we respect
// every license restriction, it may not be entirely correct.
//
// e.g. GPL in an MIT project might only apply to the contrib/ directory.
//
// Please consider splitting the single license below into multiple licenses,
// taking care not to lose any license_kind information, and overriding the
// default license using the 'licenses: [...]' property on targets as needed.
//
// For unused files, consider creating a 'fileGroup' with "//visibility:private"
// to attach the license to, and including a comment whether the files may be
// used in the current project.
// See: http://go/android-license-faq"""
        output.append(comment)

    output.append('')
    output.append('%s {' % target_type)
    for (key, value) in values.items():
        write_blueprint_key_value(output, key, value)
    output.append('}')


def gn_target_to_blueprint_target(target, target_info):
    if 'output_name' in target_info:
        return target_info['output_name']

    # Split the gn target name (in the form of //gn_file_path:target_name) into gn_file_path and
    # target_name
    match = re.match(r"^//([a-zA-Z0-9\-\+_/]*):([a-zA-Z0-9\-\+_.]+)$", target)
    assert match is not None

    gn_file_path = match.group(1)
    target_name = match.group(2)
    assert len(target_name) > 0

    # Clean up the gn file path to be a valid blueprint target name.
    gn_file_path = gn_file_path.replace("/", "_").replace(".", "_").replace("-", "_")

    # Generate a blueprint target name by merging the gn path and target so each target is unique.
    # Prepend the 'angle' prefix to all targets in the root path (empty gn_file_path).
    # Skip this step if the target name already starts with 'angle' to avoid target names such as 'angle_angle_common'.
    root_prefix = "angle"
    if len(gn_file_path) == 0 and not target_name.startswith(root_prefix):
        gn_file_path = root_prefix

    # Avoid names such as _angle_common if the gn_file_path is empty.
    if len(gn_file_path) > 0:
        gn_file_path += "_"

    return gn_file_path + target_name


def remap_gn_path(path):
    # TODO: pass the gn gen folder as an arg so it is future proof. b/150457277
    remap_folders = [
        ('out/Android/gen/angle/', ''),
        ('out/Android/gen/', ''),
    ]

    remapped_path = path
    for (remap_source, remap_dest) in remap_folders:
        remapped_path = remapped_path.replace(remap_source, remap_dest)

    return remapped_path


def gn_path_to_blueprint_path(source):
    # gn uses '//' to indicate the root directory, blueprint uses the .bp file's location
    return remap_gn_path(re.sub(r'^//?', '', source))


def gn_paths_to_blueprint_paths(paths):
    rebased_paths = []
    for path in paths:
        rebased_paths.append(gn_path_to_blueprint_path(path))
    return rebased_paths


def gn_sources_to_blueprint_sources(sources):
    # Blueprints only list source files in the sources list. Headers are only referenced though
    # include paths.
    file_extension_allowlist = [
        '.c',
        '.cc',
        '.cpp',
    ]

    rebased_sources = []
    for source in sources:
        if os.path.splitext(source)[1] in file_extension_allowlist:
            rebased_sources.append(gn_path_to_blueprint_path(source))
    return rebased_sources


target_blockist = [
    '//build/config:shared_library_deps',
    '//third_party/vulkan-validation-layers/src:vulkan_clean_old_validation_layer_objects',
    '//third_party/zlib:zlib',
    '//third_party/zlib/google:compression_utils_portable',
]

third_party_target_allowlist = [
    '//third_party/abseil-cpp',
    '//third_party/glslang/src',
    '//third_party/spirv-cross/src',
    '//third_party/spirv-headers/src',
    '//third_party/spirv-tools/src',
    '//third_party/vulkan-headers/src',
    '//third_party/vulkan-loader/src',
    '//third_party/vulkan-tools/src',
    '//third_party/vulkan-utility-libraries/src',
    '//third_party/vulkan-validation-layers/src',
    '//third_party/vulkan_memory_allocator',
]

include_blocklist = [
    '//buildtools/third_party/libc++/',
    '//third_party/libc++/src/',
    '//out/Android/gen/third_party/glslang/src/include/',
    '//third_party/zlib/',
    '//third_party/zlib/google/',
]


@functools.lru_cache(maxsize=None)  # .cache() is py3.9 http://b/246559064#comment8
def gn_deps_to_blueprint_deps(abi, target, build_info):
    target_info = build_info[abi][target]
    static_libs = []
    shared_libs = []
    defaults = []
    generated_headers = []
    header_libs = []
    if 'deps' not in target_info:
        return static_libs, defaults

    for dep in target_info['deps']:
        if dep not in target_blockist and (not dep.startswith('//third_party') or any(
                dep.startswith(substring) for substring in third_party_target_allowlist)):
            dep_info = build_info[abi][dep]
            blueprint_dep_name = gn_target_to_blueprint_target(dep, dep_info)

            # Depending on the dep type, blueprints reference it differently.
            gn_dep_type = dep_info['type']
            if gn_dep_type == 'static_library':
                static_libs.append(blueprint_dep_name)
            elif gn_dep_type == 'shared_library':
                shared_libs.append(blueprint_dep_name)
            elif gn_dep_type == 'source_set' or gn_dep_type == 'group':
                defaults.append(blueprint_dep_name)
            elif gn_dep_type == 'action':
                generated_headers.append(blueprint_dep_name)

            # Blueprints do not chain linking of static libraries.
            (child_static_libs, _, _, child_generated_headers,
             _) = gn_deps_to_blueprint_deps(abi, dep, build_info)

            # Each target needs to link all child static library dependencies.
            static_libs += child_static_libs

            # Each blueprint target runs genrules in a different output directory unlike GN. If a
            # target depends on another's genrule, it wont find the outputs. Propogate generated
            # headers up the dependency stack.
            generated_headers += child_generated_headers
        elif dep == '//third_party/zlib/google:compression_utils_portable':
            # Replace zlib by Android's zlib, compression_utils_portable is the root dependency
            shared_libs.append('libz')
            static_libs.extend(['zlib_google_compression_utils_portable', 'cpufeatures'])

    return static_libs, shared_libs, defaults, generated_headers, header_libs


def gn_libs_to_blueprint_shared_libraries(target_info):
    lib_blockist = [
        'android_support',
        'unwind',
    ]

    result = []
    if 'libs' in target_info:
        for lib in target_info['libs']:
            if lib not in lib_blockist:
                android_lib = lib if '@' in lib else 'lib' + lib
                result.append(android_lib)
    return result


def gn_include_dirs_to_blueprint_include_dirs(target_info):
    result = []
    if 'include_dirs' in target_info:
        for include_dir in target_info['include_dirs']:
            if len(include_dir) > 0 and include_dir not in include_blocklist:
                result.append(gn_path_to_blueprint_path(include_dir))
    return result


def escape_quotes(string):
    return string.replace("\"", "\\\"").replace("\'", "\\\'")


def gn_cflags_to_blueprint_cflags(target_info):
    result = []

    # regexs of allowlisted cflags
    cflag_allowlist = [
        r'^-Wno-.*$',  # forward cflags that disable warnings
        r'^-fvisibility.*$',  # forward visibility (https://gcc.gnu.org/wiki/Visibility) flags for better perf on x86
        r'-mpclmul'  # forward "-mpclmul" (used by zlib)
    ]

    for cflag_type in ['cflags', 'cflags_c', 'cflags_cc']:
        if cflag_type in target_info:
            for cflag in target_info[cflag_type]:
                for allowlisted_cflag in cflag_allowlist:
                    if re.search(allowlisted_cflag, cflag):
                        result.append(cflag)

    if 'defines' in target_info:
        for define in target_info['defines']:
            # Don't emit ANGLE's CPU-bits define here, it will be part of the arch-specific
            # information later
            result.append('-D%s' % escape_quotes(define))

    return result


blueprint_library_target_types = {
    "static_library": "cc_library_static",
    "shared_library": "cc_library_shared",
    "source_set": "cc_defaults",
    "group": "cc_defaults",
}


def merge_bps(bps_for_abis):
    common_bp = {}
    for abi in ABI_TARGETS:
        for key, values in bps_for_abis[abi].items():
            if not isinstance(values, list):
                # Assume everything that's not a list is common to all ABIs
                common_bp[key] = values
                continue

            # Find list values that are common to all ABIs
            values_in_all_abis = set.intersection(
                *[set(bps_for_abis[abi2].get(key, [])) for abi2 in ABI_TARGETS])

            for value in values:
                if value in values_in_all_abis or key == 'defaults':  # arch-specific defaults are not supported
                    common_bp.setdefault(key, [])
                    common_bp[key].append(value)
                else:
                    common_bp.setdefault('arch', {abi3: {} for abi3 in ABI_TARGETS})
                    abi_specific = common_bp['arch'][abi]
                    abi_specific.setdefault(key, [])
                    abi_specific[key].append(value)

    return common_bp


def library_target_to_blueprint(target, build_info):
    bps_for_abis = {}
    blueprint_type = ""
    for abi in ABI_TARGETS:
        if target not in build_info[abi].keys():
            bps_for_abis[abi] = {}
            continue

        target_info = build_info[abi][target]

        blueprint_type = blueprint_library_target_types[target_info['type']]

        bp = {'name': gn_target_to_blueprint_target(target, target_info)}

        if 'sources' in target_info:
            bp['srcs'] = gn_sources_to_blueprint_sources(target_info['sources'])

        (bp['static_libs'], bp['shared_libs'], bp['defaults'], bp['generated_headers'],
         bp['header_libs']) = gn_deps_to_blueprint_deps(abi, target, build_info)
        bp['shared_libs'] += gn_libs_to_blueprint_shared_libraries(target_info)

        bp['local_include_dirs'] = gn_include_dirs_to_blueprint_include_dirs(target_info)

        bp['cflags'] = gn_cflags_to_blueprint_cflags(target_info)

        bp['defaults'].append('angle_common_library_cflags')

        bp['sdk_version'] = MIN_SDK_VERSION
        bp['stl'] = STL
        if target in ROOT_TARGETS:
            bp['defaults'].append('angle_vendor_cc_defaults')
        bps_for_abis[abi] = bp

    common_bp = merge_bps(bps_for_abis)

    return blueprint_type, common_bp


def gn_action_args_to_blueprint_args(blueprint_inputs, blueprint_outputs, args):
    # TODO: pass the gn gen folder as an arg so we know how to get from the gen path to the root
    # path. b/150457277
    remap_folders = [
        # Specific special-cases first, since the other will strip the prefixes.
        ('gen/third_party/glslang/src/include/glslang/build_info.h', 'glslang/build_info.h'),
        ('third_party/glslang/src', 'external/angle/third_party/glslang/src'),
        ('../../', ''),
        ('gen/', ''),
    ]

    result_args = []
    for arg in args:
        # Attempt to find if this arg is a path to one of the inputs. If it is, use the blueprint
        # $(location <path>) argument instead so the path gets remapped properly to the location
        # that the script is run from
        remapped_path_arg = arg
        for (remap_source, remap_dest) in remap_folders:
            remapped_path_arg = remapped_path_arg.replace(remap_source, remap_dest)

        if remapped_path_arg in blueprint_inputs or remapped_path_arg in blueprint_outputs:
            result_args.append('$(location %s)' % remapped_path_arg)
        elif os.path.basename(remapped_path_arg) in blueprint_outputs:
            result_args.append('$(location %s)' % os.path.basename(remapped_path_arg))
        else:
            result_args.append(remapped_path_arg)

    return result_args


blueprint_gen_types = {
    "action": "cc_genrule",
}


inputs_blocklist = [
    '//.git/HEAD',
]

outputs_remap = {
    'build_info.h': 'glslang/build_info.h',
}


def is_input_in_tool_files(tool_files, input):
    return input in tool_files


# special handling the {{response_file_name}} args in GN:
# see https://gn.googlesource.com/gn/+/main/docs/reference.md#var_response_file_contents
# in GN, if we use response_file_contents, the GN build system will automatically
# write contents specified in response_file_contents arg into a temporary file
# identified by {{response_file_name}}. However, Android blueprint does not have
# the matching machanism. Android blueprint does automatically generate the
# temporary file and does not recognize '{{response_file_name}}'.
# To solve the problem:
# 1) replace the '{{response_file_name}}' in command argument with the new
# temporary file name.
# 2) write the content specified in 'response_file_contents' to the new temporary
# file
# This function completes step 1) above. It checks if there are
# '{{response_file_name}}' used in the command arguments. If there are,
# the function replaces the '{{response_file_name}}' with the new temp file
# named 'gn_response_file', and returns the new temp file to indicate
# we need to complete step 2)
def handle_gn_build_arg_response_file_name(command_arg_list):
    new_temp_file_name = None
    updated_args = command_arg_list[:]
    for index, arg in enumerate(updated_args):
        if arg == '{{response_file_name}}':
            new_temp_file_name = '$(genDir)/gn_response_file'
            updated_args[index] = new_temp_file_name
    return new_temp_file_name, updated_args


def action_target_to_blueprint(abi, target, build_info):
    target_info = build_info[abi][target]
    blueprint_type = blueprint_gen_types[target_info['type']]

    bp = {'name': gn_target_to_blueprint_target(target, target_info)}

    # Blueprints use only one 'srcs', merge all gn inputs into one list.
    gn_inputs = []
    if 'inputs' in target_info:
        for input in target_info['inputs']:
            if input not in inputs_blocklist:
                gn_inputs.append(input)
    if 'sources' in target_info:
        gn_inputs += target_info['sources']
    # Filter out the 'script' entry since Android.bp doesn't like the duplicate entries
    if 'script' in target_info:
        gn_inputs = [
            input for input in gn_inputs
            if not is_input_in_tool_files(target_info['script'], input)
        ]

    bp_srcs = gn_paths_to_blueprint_paths(gn_inputs)

    bp['srcs'] = bp_srcs

    # genrules generate the output right into the 'root' directory. Strip any path before the
    # file name.
    bp_outputs = []
    for gn_output in target_info['outputs']:
        output = os.path.basename(gn_output)
        if output in outputs_remap.keys():
            output = outputs_remap[output]
        bp_outputs.append(output)

    bp['out'] = bp_outputs

    bp['tool_files'] = [gn_path_to_blueprint_path(target_info['script'])]

    new_temporary_gn_response_file, updated_args = handle_gn_build_arg_response_file_name(
        target_info['args'])

    if new_temporary_gn_response_file:
        # add the command 'echo $(in) > $(genDir)/gn_response_file' to
        # write $response_file_contents into the new_temporary_gn_response_file.
        cmd = ['echo $(in) >', new_temporary_gn_response_file, '&&', '$(location)'
              ] + gn_action_args_to_blueprint_args(bp_srcs, bp_outputs, updated_args)
    else:
        cmd = ['$(location)'] + gn_action_args_to_blueprint_args(bp_srcs, bp_outputs,
                                                                 target_info['args'])

    bp['cmd'] = ' '.join(cmd)

    bp['sdk_version'] = MIN_SDK_VERSION

    return blueprint_type, bp


def gn_target_to_blueprint(target, build_info):
    for abi in ABI_TARGETS:
        gn_type = build_info[abi][target]['type']
        if gn_type in blueprint_library_target_types:
            return library_target_to_blueprint(target, build_info)
        elif gn_type in blueprint_gen_types:
            return action_target_to_blueprint(abi, target, build_info)
        else:
            # Target is not used by this ABI
            continue


@functools.lru_cache(maxsize=None)
def get_gn_target_dependencies(abi, target, build_info):
    result = collections.OrderedDict()
    result[target] = 1

    for dep in build_info[abi][target]['deps']:
        if dep in target_blockist:
            # Blocklisted dep
            continue
        if dep not in build_info[abi]:
            # No info for this dep, skip it
            continue

        # Recurse
        result.update(get_gn_target_dependencies(abi, dep, build_info))

    return result


def get_angle_in_vendor_flag_config():
    blueprint_results = []

    blueprint_results.append(('soong_config_module_type', {
        'name': 'angle_config_cc_defaults',
        'module_type': 'cc_defaults',
        'config_namespace': 'angle',
        'bool_variables': ['angle_in_vendor',],
        'properties': [
            'target.android.relative_install_path',
            'vendor',
        ],
    }))

    blueprint_results.append(('soong_config_bool_variable', {
        'name': 'angle_in_vendor',
    }))

    blueprint_results.append((
        'angle_config_cc_defaults',
        {
            'name': 'angle_vendor_cc_defaults',
            'vendor': False,
            'target': {
                'android': {
                    # Android EGL loader can not load from /system/egl/${LIB}
                    # path and hence don't set the relative path so that ANGLE
                    # libraries get built into /system/${LIB}
                    'relative_install_path': '',
                },
            },
            'soong_config_variables': {
                'angle_in_vendor': {
                    'vendor': True,
                    'target': {
                        'android': {
                            'relative_install_path': 'egl',
                        },
                    },
                },
            },
        }))

    return blueprint_results


def get_angle_android_dma_buf_flag_config(flagged_module_types: List[FlaggedModuleType]):
    """
    Generates a list of Android.bp definitions for angle_android_dma_buf flag.

    flagged_module_types is a list of Soong module types that are going to be used.
    """

    blueprint_results = []

    for mod in sorted(flagged_module_types):
        blueprint_results.append(('soong_config_module_type', {
            'name': mod.flagged_module_type,
            'module_type': mod.base_module_type,
            'config_namespace': 'angle',
            'bool_variables': ['angle_android_dma_buf'],
            'properties': list(mod.differing_properties),
        }))

    blueprint_results.append(('soong_config_bool_variable', {
        'name': 'angle_android_dma_buf',
    }))

    return blueprint_results


# returns list of (blueprint module type, dict with contents)
def get_blueprint_targets_from_build_info(build_info: BuildInfo) -> List[Tuple[str, dict]]:
    targets_to_write = collections.OrderedDict()
    for abi in ABI_TARGETS:
        for root_target in ROOT_TARGETS:
            targets_to_write.update(get_gn_target_dependencies(abi, root_target, build_info))

    generated_targets = []
    for target in reversed(targets_to_write.keys()):
        generated_targets.append(gn_target_to_blueprint(target, build_info))

    return generated_targets


def dict_diff(left, right) -> Tuple[dict, dict, dict]:
    """
    Perform a recursive three-way diff of 2 dictionaries. Return a tuple with
    3 elements:
    - dictionary with elements unique to the `left` dict,
    - dictionary with elements common to both `left` and `right`,
    - dictionary with elements unique to the `right` dict.
    """
    left_only = {k: left[k] for k in set(left) - set(right)}
    right_only = {k: right[k] for k in set(right) - set(left)}
    common = {}
    for k in set(left) & set(right):
        if left[k] == right[k]:
            l = None
            c = left[k]
            r = None
        elif isinstance(left[k], list) and isinstance(right[k], list):
            l_set = set(left[k])
            r_set = set(right[k])
            l = list(sorted(l_set - r_set))
            c = list(sorted(l_set & r_set))
            r = list(sorted(r_set - l_set))
        elif isinstance(left[k], dict) and isinstance(right[k], dict):
            l, c, r = dict_diff(left[k], right[k])
        else:
            l = left[k]
            c = None
            r = right[k]

        if l:
            left_only[k] = l
        if c:
            common[k] = c
        if r:
            right_only[k] = r

    return left_only, common, right_only


# run tests with
#   python3 -m unittest scripts/generate_android_bp.py
class DictDiffTests(unittest.TestCase):

    def test_primitives_left_only(self):
        l, c, r = dict_diff({'a': 1}, {})
        self.assertEqual(l, {'a': 1})
        self.assertEqual(c, {})
        self.assertEqual(r, {})

    def test_primitives_right_only(self):
        l, c, r = dict_diff({}, {'a': 1})
        self.assertEqual(l, {})
        self.assertEqual(c, {})
        self.assertEqual(r, {'a': 1})

    def test_primitives_common(self):
        l, c, r = dict_diff({'a': 1}, {'a': 1})
        self.assertEqual(l, {})
        self.assertEqual(c, {'a': 1})
        self.assertEqual(r, {})

    def test_list_left_only(self):
        l, c, r = dict_diff({'a': [1]}, {})
        self.assertEqual(l, {'a': [1]})
        self.assertEqual(c, {})
        self.assertEqual(r, {})

    def test_list_right_only(self):
        l, c, r = dict_diff({}, {'a': [1]})
        self.assertEqual(l, {})
        self.assertEqual(c, {})
        self.assertEqual(r, {'a': [1]})

    def test_list_common(self):
        l, c, r = dict_diff({'a': [1]}, {'a': [1]})
        self.assertEqual(l, {})
        self.assertEqual(c, {'a': [1]})
        self.assertEqual(r, {})

    def test_list_common_partial(self):
        l, c, r = dict_diff({'a': [1, 2]}, {'a': [1, 3]})
        self.assertEqual(l, {'a': [2]})
        self.assertEqual(c, {'a': [1]})
        self.assertEqual(r, {'a': [3]})

    def test_dict_left_only(self):
        l, c, r = dict_diff({'a': {'b': 1}}, {})
        self.assertEqual(l, {'a': {'b': 1}})
        self.assertEqual(c, {})
        self.assertEqual(r, {})

    def test_dict_right_only(self):
        l, c, r = dict_diff({}, {'a': {'b': 1}})
        self.assertEqual(l, {})
        self.assertEqual(c, {})
        self.assertEqual(r, {'a': {'b': 1}})

    def test_dict_common(self):
        l, c, r = dict_diff({'a': {'b': 1}}, {'a': {'b': 1}})
        self.assertEqual(l, {})
        self.assertEqual(c, {'a': {'b': 1}})
        self.assertEqual(r, {})

    def test_dict_common_partial(self):
        l, c, r = dict_diff({'a': {'b': 1, 'c': 2}}, {'a': {'b': 1, 'c': 3}})
        self.assertEqual(l, {'a': {'c': 2}})
        self.assertEqual(c, {'a': {'b': 1}})
        self.assertEqual(r, {'a': {'c': 3}})


def merge_targets_with_flag(default: List[Tuple[str, dict]], flagged: List[Tuple[str, dict]],
                            flag: str) -> Tuple[List[Tuple[str, dict]], List[FlaggedModuleType]]:
    """
    Given 2 lists of Android.bp module definitions and a Soong config flag name,
    generate "merged" Android.bp modules that use given flag name to
    conditionally enable differing properties depending on the flag being defined.

    Keep track of cases where introduced conditional requires defining a new
    Soong module type.

    Return a tuple with:
    - a list of possibly flagged Soong modules,
    - list of new Soong module types to define.
    """

    def reorder_dict(d: dict) -> collections.OrderedDict:
        """
        Reorder dictionary keys to minimize changes to generated Android.bp file.

        This is not strictly necessary, but makes manual inspection of generated
        changes easier.
        """
        ordered = collections.OrderedDict()
        order = [
            'name',
            'cpp_std',
            'srcs',
            'static_libs',
            'shared_libs',
            'defaults',
            'generated_headers',
            'local_include_dirs',
            'cflags',
            'out',
            'tool_files',
            'cmd',
            'sdk_version',
            'stl',
            'arch',
            'vendor',
            'target',
            'android',
            'relative_install_path',
            'soong_config_variables',
            'conditions_default',
        ]
        order += sorted([k for k in d if k not in order])

        for k in order:
            if k in d:
                if isinstance(d[k], dict):
                    ordered[k] = reorder_dict(d[k])
                else:
                    ordered[k] = d[k]

        return ordered

    def collect_property_names(d: dict) -> Set[str]:
        """
        Recursively collect all keys in a dict. In case of nested dicts, return
        dot-separated paths.

        Used to list the properties for soong_config_module_type.
        """
        names = set()
        for k, v in d.items():
            names.add(k)
            if isinstance(v, dict):
                names |= set(f'{k}.{n}' for n in collect_property_names(v))
        return names

    flagged_module_types = {}

    def merge_targets(default: dict, flagged: dict) -> Tuple[List[dict], Set[str]]:
        """
        Merge Android.bp dictionaries, putting elements unique to either
        `default` or `flagged` under relevant sections of soong_config_variables.

        Return a tuple with:
        - a merged Android.bp dict,
        - a set of dot-separated property paths that differ between default and
          flagged.
        """
        default, common, flagged = dict_diff(default, flagged)
        assert 'soong_config_variables' not in common

        if default or flagged:
            differing_properties = set()
            if 'soong_config_variables' not in common:
                common['soong_config_variables'] = {}
            common['soong_config_variables'] = {}
            common['soong_config_variables'][flag] = flagged or {}
            if default:
                common['soong_config_variables'][flag]['conditions_default'] = default

            differing_properties |= {
                v for v in collect_property_names(flagged) if 'conditions_default' not in v
            }
            differing_properties |= {
                v for v in collect_property_names(default) if 'conditions_default' not in v
            }
            return reorder_dict(common), differing_properties

        return reorder_dict(common), set()

    default_dict = {bp['name']: (t, bp) for t, bp in default}
    flagged_dict = {bp['name']: (t, bp) for t, bp in flagged}

    merged_targets = []
    for target in list(default_dict) + [t for t in flagged_dict if t not in default_dict]:
        if target in default_dict and target not in flagged_dict:
            merged_targets.append(default_dict[target])
        elif target not in default_dict and target in flagged_dict:
            merged_targets.append(flagged_dict[target])
        else:
            # The target exists in both default and flagged cases.
            # Check for differences between the variants. If there are any:
            # - Merge default and flagged targets.
            # - Add the module type to a list of ones that need flagging. We
            #   will emit a `soong_config_module_type` for it.
            # - Change Soong module type to the emitted one.
            assert default_dict[target][0] == flagged_dict[target][0], \
                   'cannot merge different Android.bp module types: {} and {}'.format(default_dict[target][0], flagged_dict[target][0])
            merged, differing_properties = merge_targets(default_dict[target][1],
                                                         flagged_dict[target][1])
            module_type = default_dict[target][0]

            if differing_properties:
                new_type = f'{flag}_{module_type}'
                if new_type not in flagged_module_types:
                    flagged_module_types[new_type] = FlaggedModuleType(
                        base_module_type=module_type,
                        flagged_module_type=new_type,
                        differing_properties=set())
                flagged_module_types[new_type].differing_properties.update(differing_properties)
                module_type = new_type

            merged_targets.append((module_type, merged))

    return merged_targets, list(flagged_module_types.values())


# run tests with
#   python3 -m unittest scripts/generate_android_bp.py
class MergeTargetsTests(unittest.TestCase):

    def test_no_flag_needed(self):
        default = {
            'name': 'default',
            'srcs': ['a.cpp'],
        }
        flagged = {
            'name': 'flagged',
            'srcs': ['a.cpp'],
        }
        merged, module_types = merge_targets_with_flag(
            default=[('cc_library', default)], flagged=[('cc_library', flagged)], flag='flag')
        self.assertEqual(len(merged), 2)
        merged_default = [(k, v) for k, v in merged if v['name'] == 'default'][0]
        merged_flagged = [(k, v) for k, v in merged if v['name'] == 'flagged'][0]
        self.assertEqual(merged_default, ('cc_library', default))
        self.assertEqual(merged_flagged, ('cc_library', flagged))

        # disjoint modules => no need to generate flagged module types
        self.assertEqual(len(module_types), 0)

    def test_merge_same(self):
        default = {
            'name': 'default',
            'srcs': ['a.cpp'],
        }
        merged, module_types = merge_targets_with_flag(
            default=[('cc_library', default)], flagged=[('cc_library', default)], flag='flag')
        self.assertEqual(len(merged), 1)
        self.assertEqual(merged[0], ('cc_library', default))

        # no differences => no need to generate flagged module types
        self.assertEqual(len(module_types), 0)

    def test_merge_differences(self):
        default = {
            'name': 'same',
            'srcs': ['default.cpp', 'common.cpp'],
            'cflags': ['default'],
            'target': {
                'android': {
                    'cflags': ['nested_default'],
                }
            }
        }
        flagged = {
            'name': 'same',
            'srcs': ['common.cpp', 'flagged.cpp'],
            'cflags': ['flagged'],
            'target': {
                'android': {
                    'cflags': ['nested_flagged'],
                }
            }
        }
        merged, module_types = merge_targets_with_flag(
            default=[('cc_library', default)], flagged=[('cc_library', flagged)], flag='flag')
        self.assertEqual(merged, [
            ('flag_cc_library', {
                'name': 'same',
                'srcs': ['common.cpp'],
                'soong_config_variables': {
                    'flag': {
                        'srcs': ['flagged.cpp'],
                        'cflags': ['flagged'],
                        'target': {
                            'android': {
                                'cflags': ['nested_flagged'],
                            }
                        },
                        'conditions_default': {
                            'srcs': ['default.cpp'],
                            'cflags': ['default'],
                            'target': {
                                'android': {
                                    'cflags': ['nested_default'],
                                }
                            }
                        }
                    }
                }
            }),
        ])
        self.assertEqual(module_types, [
            FlaggedModuleType(
                base_module_type='cc_library',
                flagged_module_type='flag_cc_library',
                differing_properties={
                    'srcs',
                    'cflags',
                    'target',
                    'target.android',
                    'target.android.cflags',
                }),
        ])


def main():
    parser = argparse.ArgumentParser(
        description='Generate Android blueprints from gn descriptions.')

    for suffix in ('', '_dma_buf'):
        for abi in ABI_TARGETS:
            parser.add_argument(
                '--gn_json_' + gn_abi(abi) + suffix,
                help=gn_abi(abi) +
                ' gn desc file in json format. Generated with \'gn desc <out_dir> --format=json "*"\'.',
                required=True)

    parser.add_argument('--output', help='output file (e.g. Android.bp)')
    args = vars(parser.parse_args())

    infos = {}
    for abi in ABI_TARGETS:
        with open(args['gn_json_' + gn_abi(abi)], 'r') as f:
            infos[abi] = json.load(f)
    build_info = BuildInfo(infos)

    dma_buf_infos = {}
    for abi in ABI_TARGETS:
        with open(args['gn_json_' + gn_abi(abi) + '_dma_buf'], 'r') as f:
            dma_buf_infos[abi] = json.load(f)
    dma_buf_build_info = BuildInfo(dma_buf_infos)

    generated_targets, flagged_module_types = merge_targets_with_flag(
        default=get_blueprint_targets_from_build_info(build_info),
        flagged=get_blueprint_targets_from_build_info(dma_buf_build_info),
        flag='angle_android_dma_buf')

    blueprint_targets = []
    blueprint_targets.extend(get_angle_in_vendor_flag_config())
    blueprint_targets.extend(get_angle_android_dma_buf_flag_config(flagged_module_types))

    blueprint_targets.append((
        'cc_defaults',
        {
            'name':
                'angle_common_library_cflags',
            'cpp_std':
                'gnu++17',  # TODO(b/330910097): std::popcount missing from external/libcxx
            'cflags': [
                # Chrome and Android use different versions of Clang which support differnt warning options.
                # Ignore errors about unrecognized warning flags.
                '-Wno-unknown-warning-option',
                '-O2',
                # Override AOSP build flags to match ANGLE's CQ testing and reduce binary size
                '-fno-unwind-tables',
                # Disable stack protector to reduce cpu overhead.
                '-fno-stack-protector',
            ],
        }))

    # Move cflags that are repeated in each target to cc_defaults
    all_cflags = [set(bp['cflags']) for _, bp in generated_targets if 'cflags' in bp]
    all_target_cflags = set.intersection(*all_cflags)

    for _, bp in generated_targets:
        if 'cflags' in bp:
            bp['cflags'] = list(set(bp['cflags']) - all_target_cflags)
            if 'defaults' not in bp:
                bp['defaults'] = []
            bp['defaults'].append('angle_common_auto_cflags')

    blueprint_targets.append(('cc_defaults', {
        'name': 'angle_common_auto_cflags',
        'cflags': list(all_target_cflags),
    }))
    blueprint_targets.extend(generated_targets)

    # Add license build rules
    blueprint_targets.append(('package', {
        'default_applicable_licenses': ['external_angle_license'],
    }))
    blueprint_targets.append(('license', {
        'name':
            'external_angle_license',
        'visibility': [':__subpackages__'],
        'license_kinds': [
            'SPDX-license-identifier-Apache-2.0',
            'SPDX-license-identifier-BSD',
            'SPDX-license-identifier-GPL',
            'SPDX-license-identifier-GPL-2.0',
            'SPDX-license-identifier-GPL-3.0',
            'SPDX-license-identifier-LGPL',
            'SPDX-license-identifier-MIT',
            'SPDX-license-identifier-Zlib',
            'legacy_unencumbered',
        ],
        'license_text': [
            'LICENSE',
            'src/common/third_party/xxhash/LICENSE',
            'src/libANGLE/renderer/vulkan/shaders/src/third_party/ffx_spd/LICENSE',
            'src/tests/test_utils/third_party/LICENSE',
            'src/third_party/libXNVCtrl/LICENSE',
            'src/third_party/volk/LICENSE.md',
            'third_party/abseil-cpp/LICENSE',
            'third_party/android_system_sdk/LICENSE',
            'third_party/bazel/LICENSE',
            'third_party/colorama/LICENSE',
            'third_party/proguard/LICENSE',
            'third_party/r8/LICENSE',
            'third_party/turbine/LICENSE',
            'third_party/glslang/LICENSE',
            'third_party/glslang/src/LICENSE.txt',
            'third_party/spirv-headers/LICENSE',
            'third_party/spirv-headers/src/LICENSE',
            'third_party/spirv-tools/LICENSE',
            'third_party/spirv-tools/src/LICENSE',
            'third_party/spirv-tools/src/utils/vscode/src/lsp/LICENSE',
            'third_party/vulkan-headers/LICENSE.txt',
            'third_party/vulkan-headers/src/LICENSE.md',
            'third_party/vulkan_memory_allocator/LICENSE.txt',
            'tools/flex-bison/third_party/m4sugar/LICENSE',
            'tools/flex-bison/third_party/skeletons/LICENSE',
            'util/windows/third_party/StackWalker/LICENSE',
        ],
    }))

    # Add APKs with all of the root libraries and permissions xml
    blueprint_targets.append((
        'filegroup',
        {
            'name':
                'ANGLE_srcs',
            # We only need EmptyMainActivity.java since we just need to be able to reply to the intent
            # android.app.action.ANGLE_FOR_ANDROID to indicate ANGLE is present on the device.
            # However, the internal branch currently uses these files with patches in that branch.
            'srcs': [
                'src/android_system_settings/src/com/android/angle/MainActivity.java',
                'src/android_system_settings/src/com/android/angle/common/AngleRuleHelper.java',
                'src/android_system_settings/src/com/android/angle/common/GlobalSettings.java',
                'src/android_system_settings/src/com/android/angle/common/MainFragment.java',
                'src/android_system_settings/src/com/android/angle/common/Receiver.java',
                'src/android_system_settings/src/com/android/angle/common/SearchProvider.java',
            ],
        }))

    blueprint_targets.append(('prebuilt_etc', {
        'name': 'android.software.angle.xml',
        'src': 'android/android.software.angle.xml',
        'product_specific': True,
        'sub_dir': 'permissions',
    }))

    blueprint_targets.append((
        'java_defaults',
        {
            'name': 'ANGLE_java_defaults',
            'sdk_version': 'system_current',
            'target_sdk_version': TARGET_SDK_VERSION,
            'min_sdk_version': MIN_SDK_VERSION,
            'compile_multilib': 'both',
            'use_embedded_native_libs': True,
            'jni_libs': [
                # hack: assume ABI_ARM
                gn_target_to_blueprint_target(target, build_info[ABI_ARM][target])
                for target in ROOT_TARGETS
            ],
            'aaptflags': [
                '-0 .json',  # Don't compress *.json files
                "--extra-packages com.android.angle.common",
            ],
            'srcs': [':ANGLE_srcs'],
            'privileged': True,
            'product_specific': True,
            'owner': 'google',
            'required': ['android.software.angle.xml'],
        }))

    blueprint_targets.append(('android_library', {
        'name': 'ANGLE_library',
        'sdk_version': 'system_current',
        'target_sdk_version': TARGET_SDK_VERSION,
        'min_sdk_version': MIN_SDK_VERSION,
        'resource_dirs': ['src/android_system_settings/res',],
        'asset_dirs': ['src/android_system_settings/assets',],
        'aaptflags': ['-0 .json',],
        'manifest': 'src/android_system_settings/src/com/android/angle/AndroidManifest.xml',
        'static_libs': ['androidx.preference_preference',],
    }))

    blueprint_targets.append(('android_app', {
        'name': 'ANGLE',
        'defaults': ['ANGLE_java_defaults'],
        'manifest': 'src/android_system_settings/src/com/android/angle/AndroidManifest.xml',
        'static_libs': ['ANGLE_library'],
        'optimize': {
            'enabled': True,
            'shrink': True,
            'proguard_compatibility': False,
        },
        'asset_dirs': ['src/android_system_settings/assets',],
    }))

    blueprint_targets.append((
        'java_defaults',
        {
            'name': 'ANGLE_java_settings_defaults',
            'sdk_version': 'system_current',
            'target_sdk_version': TARGET_SDK_VERSION,
            'min_sdk_version': MIN_SDK_VERSION,
            'compile_multilib': 'both',
            'use_embedded_native_libs': True,
            'aaptflags': [
                '-0 .json',  # Don't compress *.json files
                "--extra-packages com.android.angle.common",
            ],
            'srcs': [':ANGLE_srcs'],
            'privileged': True,
            'product_specific': True,
            'owner': 'google',
            'required': ['android.software.angle.xml'],
        }))

    blueprint_targets.append(('android_app', {
        'name': 'ANGLE_settings',
        'defaults': ['ANGLE_java_settings_defaults'],
        'manifest': 'src/android_system_settings/src/com/android/angle/AndroidManifest.xml',
        'static_libs': ['ANGLE_library'],
        'optimize': {
            'enabled': True,
            'shrink': True,
            'proguard_compatibility': False,
        },
        'asset_dirs': ['src/android_system_settings/assets',],
    }))

    output = [
        """// GENERATED FILE - DO NOT EDIT.
// Generated by %s
//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//""" % sys.argv[0]
    ]
    for (blueprint_type, blueprint_data) in blueprint_targets:
        write_blueprint(output, blueprint_type, blueprint_data)

    with open(args['output'], 'w') as f:
        f.write('\n'.join(output) + '\n')


if __name__ == '__main__':
    sys.exit(main())
