#!/usr/bin/env python

#  Copyright 2018 The ANGLE Project Authors. All rights reserved.
#  Use of this source code is governed by a BSD-style license that can be
#  found in the LICENSE file.

# This script adapted from Perfetto
# (https://android.googlesource.com/platform/external/perfetto)

# This tool will create an Android.bp file for the Android Soong build system.
# The input to the tool is a list of GN labels for which to generate Android.bp:
#
# Generating Android.bp needs to be done from within a Chromium build:
#   cd <chromium>/src
#   gen_angle_android_bp.py //third_party/angle:libGLESv2 //third_party/angle:libEGL
#
# This will output an Android.bp that can be copied to the angle directory
# within Android.
#
# The dependencies for the GN labels are squashed to the generated
# Android.bp target, except for actions which get their own genrule. Some
# libraries are also mapped to their Android equivalents -- see |builtin_deps|.

import argparse
import errno
import json
import logging
import os
import re
import shutil
import subprocess
import sys

# Default targets to translate to the blueprint file.
default_targets = [
]

# Defines a custom init_rc argument to be applied to the corresponding output
# blueprint target.
target_initrc = {
}

# Arguments for the GN output directory.
gn_args = 'target_os="android" target_cpu="arm" is_debug=false build_with_android=true'

# All module names are prefixed with this string to avoid collisions.
module_prefix = 'angle_'

# Shared libraries which are directly translated to Android system equivalents.
library_whitelist = [
    'android',
    'binder',
    'log',
    'services',
    'utils',
]

# Name of the module which settings such as compiler flags for all other
# modules.
defaults_module = module_prefix + 'defaults'

# Location of the project in the Android source tree.
tree_path = 'external/angle'

# Compiler flags which are passed through to the blueprint.
cflag_whitelist = r'^-DANGLE.*$'

# Compiler flags which are passed through to the blueprint.
cflag_blacklist = [
]

# Compiler defines which are passed through to the blueprint.
define_whitelist = r'^ANGLE.*$'

# Shared libraries which are not in PDK.
library_not_in_pdk = {
    'libandroid',
    'libservices',
}


def enable_gmock(module):
    module.static_libs.append('libgmock')


def enable_gtest_prod(module):
    module.static_libs.append('libgtest_prod')


def enable_gtest(module):
    assert module.type == 'cc_test'


def enable_protobuf_full(module):
    module.shared_libs.append('libprotobuf-cpp-full')


def enable_protobuf_lite(module):
    module.shared_libs.append('libprotobuf-cpp-lite')


def enable_protoc_lib(module):
    module.shared_libs.append('libprotoc')


def enable_libunwind(module):
    # libunwind is disabled on Darwin so we cannot depend on it.
    pass


# Android equivalents for third-party libraries that the upstream project
# depends on.
builtin_deps = {
}

# ----------------------------------------------------------------------------
# End of configuration.
# ----------------------------------------------------------------------------


class Error(Exception):
    pass


class ThrowingArgumentParser(argparse.ArgumentParser):
    def __init__(self, context):
        super(ThrowingArgumentParser, self).__init__()
        self.context = context

    def error(self, message):
        raise Error('%s: %s' % (self.context, message))


class Module(object):
    """A single module (e.g., cc_binary, cc_test) in a blueprint."""

    def __init__(self, mod_type, name):
        self.type = mod_type
        self.name = name
        self.srcs = []
        self.comment = None
        self.shared_libs = []
        self.static_libs = []
        self.tools = []
        self.tool_files = []
        self.cmd = None
        self.init_rc = []
        self.out = []
        self.export_include_dirs = []
        self.generated_headers = []
        self.export_generated_headers = []
        self.defaults = []
        self.cflags = set()
        self.local_include_dirs = []
        self.sdk_version = None
        self.stl = None

    def to_string(self, output):
        if self.comment:
            output.append('// %s' % self.comment)
        output.append('%s {' % self.type)
        self._output_field(output, 'name')
        self._output_field(output, 'srcs')
        self._output_field(output, 'shared_libs')
        self._output_field(output, 'static_libs')
        self._output_field(output, 'tools')
        self._output_field(output, 'tool_files')
        self._output_field(output, 'cmd', sort=False)
        self._output_field(output, 'init_rc')
        self._output_field(output, 'out')
        self._output_field(output, 'export_include_dirs')
        self._output_field(output, 'generated_headers')
        self._output_field(output, 'export_generated_headers')
        self._output_field(output, 'stl')
        self._output_field(output, 'defaults')
        self._output_field(output, 'cflags')
        self._output_field(output, 'local_include_dirs')
        self._output_field(output, 'sdk_version')
        if any(name in library_not_in_pdk for name in self.shared_libs):
            output.append('  product_variables: {')
            output.append('    pdk: {')
            output.append('      enabled: false,')
            output.append('    },')
            output.append('  },')
        output.append('}')
        output.append('')

    def _output_field(self, output, name, sort=True):
        value = getattr(self, name)
        if not value:
            return
        if isinstance(value, set):
            value = sorted(value)
        if isinstance(value, list):
            output.append('  %s: [' % name)
            for item in sorted(value) if sort else value:
                output.append('    "%s",' % item)
            output.append('  ],')
        else:
            output.append('  %s: "%s",' % (name, value))

    def add_local_includes(self, include_dirs):
        for dir in include_dirs:
            if dir.startswith('//third_party/angle/'):
                include_path = label_to_path(dir).rstrip('/')
                if not include_path in self.local_include_dirs:
                    self.local_include_dirs.append(include_path)


class Blueprint(object):
    """In-memory representation of an Android.bp file."""

    def __init__(self):
        self.modules = {}

    def add_module(self, module):
        """Adds a new module to the blueprint, replacing any existing module
        with the same name.

        Args:
            module: Module instance.
        """
        self.modules[module.name] = module

    def to_string(self, output):
        for m in sorted(self.modules.itervalues(), key=lambda m: m.name):
            m.to_string(output)


def label_to_path(label):
    """Turn a GN output label (e.g., //some_dir/file.cc) into a path."""
    assert label.startswith('//third_party/angle/')
    return label[20:]


def label_to_module_name(label):
    """Turn a GN label (e.g., //third_party/angle:libGLESv2) into a module name."""
    module = re.sub(r'^//:?', '', label)
    module = re.sub(r'[^a-zA-Z0-9_]', '_', module)
    if not module.startswith(module_prefix) and label not in default_targets:
        return module_prefix + module
    return module


def label_without_toolchain(label):
    """Strips the toolchain from a GN label.

    Return a GN label (e.g //buildtools:protobuf(//gn/standalone/toolchain:
    gcc_like_host) without the parenthesised toolchain part.
    """
    return label.split('(')[0]


def is_supported_source_file(name):
    """Returns True if |name| can appear in a 'srcs' list."""
    return os.path.splitext(name)[1] in ['.c', '.cc', '.cpp']


def is_generated_by_action(desc, label):
    """Checks if a label is generated by an action.

    Returns True if a GN output label |label| is an output for any action,
    i.e., the file is generated dynamically.
    """
    for target in desc.itervalues():
        if target['type'] == 'action' and label in target['outputs']:
            return True
    return False


def apply_module_dependency(blueprint, desc, module, dep_name):
    """Recursively collect dependencies for a given module.

    Walk the transitive dependencies for a GN target and apply them to a given
    module. This effectively flattens the dependency tree so that |module|
    directly contains all the sources, libraries, etc. in the corresponding GN
    dependency tree.

    Args:
        blueprint: Blueprint instance which is being generated.
        desc: JSON GN description.
        module: Module to which dependencies should be added.
        dep_name: GN target of the dependency.
    """
    # If the dependency refers to a library which we can replace with an Android
    # equivalent, stop recursing and patch the dependency in.
    if label_without_toolchain(dep_name) in builtin_deps:
        builtin_deps[label_without_toolchain(dep_name)](module)
        return

    # Similarly some shared libraries are directly mapped to Android
    # equivalents.
    target = desc[dep_name]
    for lib in target.get('libs', []):
        android_lib = 'lib' + lib
        if lib in library_whitelist and not android_lib in module.shared_libs:
            module.shared_libs.append(android_lib)

    type = target['type']
    if type == 'action':
        create_modules_from_target(blueprint, desc, dep_name)
        module.generated_headers.append(label_to_module_name(dep_name))
    elif type == 'static_library' and label_to_module_name(
            dep_name) != module.name:
        create_modules_from_target(blueprint, desc, dep_name)
        module.static_libs.append(label_to_module_name(dep_name))
    elif type == 'shared_library' and label_to_module_name(
            dep_name) != module.name:
        module.shared_libs.append(label_to_module_name(dep_name))
    elif type in ['group', 'source_set', 'executable', 'static_library'
                  ] and 'sources' in target:
        # Ignore source files that are generated by actions since they will be
        # implicitly added by the genrule dependencies.
        module.srcs.extend(
            label_to_path(src) for src in target['sources']
            if is_supported_source_file(src)
            and not is_generated_by_action(desc, src))
    module.cflags |= _get_cflags(target)


def make_genrules_for_action(blueprint, desc, target_name):
    """Generate genrules for a GN action.

    GN actions are used to dynamically generate files during the build. The
    Soong equivalent is a genrule. This function turns a specific kind of
    genrule which turns .proto files into source and header files into a pair
    equivalent genrules.

    Args:
        blueprint: Blueprint instance which is being generated.
        desc: JSON GN description.
        target_name: GN target for genrule generation.

    Returns:
        A genrule module.
    """
    target = desc[target_name]

    args = target['args']
    logging.debug("args: %s" % args)
    logging.debug("target: %s" % target)

    module = Module('genrule', label_to_module_name(target_name))

    module.tool_files = [label_to_path(target['script'])]

    module.out = [tree_path + "/id/commit.h"]
    module.srcs = ["src/commit.h"]
    module.export_include_dirs = ['.']
    module.cmd = '$(location src/commit_id.py) gen $(in) $(out)'

    return module


def _get_cflags(target):
    cflags = set(flag for flag in target.get('cflags', [])
                 if re.match(cflag_whitelist, flag) and flag not in cflag_blacklist)
    cflags |= set("-D%s" % define.replace('"', '\\"') for define in target.get('defines', [])
                  if re.match(define_whitelist, define))
    return cflags


def create_modules_from_target(blueprint, desc, target_name):
    """Generate module(s) for a given GN target.

    Given a GN target name, generate one or more corresponding modules into a
    blueprint.

    Args:
        blueprint: Blueprint instance which is being generated.
        desc: JSON GN description.
        target_name: GN target for module generation.
    """
    target = desc[target_name]
    logging.debug("target: %s, type %s" % (target_name, target['type']))
    module_name = label_to_module_name(target_name)

    if target['type'] == 'executable':
        if 'host' in target['toolchain']:
            module_type = 'cc_binary_host'
        elif target.get('testonly'):
            module_type = 'cc_test'
        else:
            module_type = 'cc_binary'
        modules = [Module(module_type, module_name)]
    elif target['type'] == 'action':
        modules = [make_genrules_for_action(blueprint, desc, target_name)]
    elif target['type'] == 'static_library':
        module = Module('cc_library_static', module_name)
        module.export_include_dirs = ['include']
        module.stl = 'c++_static'
        module.add_local_includes(target['include_dirs'])
        modules = [ module ]
    elif target['type'] == 'shared_library':
        module = Module('cc_library_shared', module_name)
        module.stl = 'c++_shared'
        module.add_local_includes(target['include_dirs'])
        modules = [ module ]
    else:
        raise Error('Unknown target type: %s' % target['type'])

    for module in modules:
        module.comment = 'GN target: %s' % target_name
        if target_name in target_initrc:
          module.init_rc = [target_initrc[target_name]]

        # Don't try to inject library/source dependencies into genrules because
        # they are not compiled in the traditional sense.
        if module.type != 'genrule':
            module.defaults = [defaults_module]
            apply_module_dependency(blueprint, desc, module, target_name)
            for dep in resolve_dependencies(desc, target_name):
                apply_module_dependency(blueprint, desc, module, dep)

        blueprint.add_module(module)


def resolve_dependencies(desc, target_name):
    """Return the transitive set of dependent-on targets for a GN target.

    Args:
        blueprint: Blueprint instance which is being generated.
        desc: JSON GN description.

    Returns:
        A set of transitive dependencies in the form of GN targets.
    """

    if label_without_toolchain(target_name) in builtin_deps:
        return set()
    target = desc[target_name]
    resolved_deps = set()
    for dep in target.get('deps', []):
        resolved_deps.add(dep)
        # Ignore the transitive dependencies of actions because they are
        # explicitly converted to genrules.
        if desc[dep]['type'] == 'action':
            continue
        # Dependencies on shared libraries shouldn't propagate any transitive
        # dependencies but only depend on the shared library target
        if desc[dep]['type'] == 'shared_library':
            continue
        resolved_deps.update(resolve_dependencies(desc, dep))
    return resolved_deps


def create_blueprint_for_targets(desc, targets):
    """Generate a blueprint for a list of GN targets."""
    blueprint = Blueprint()

    # Default settings used by all modules.
    defaults = Module('cc_defaults', defaults_module)
    defaults.local_include_dirs = ['include']
    defaults.cflags = [
        '-Wno-error=return-type',
        '-Wno-sign-compare',
        '-Wno-sign-promo',
        '-Wno-unused-parameter',
        '-Wno-non-virtual-dtor',
        '-fvisibility=hidden',
        '-Oz',
    ]
    defaults.sdk_version = '24'

    blueprint.add_module(defaults)
    for target in targets:
        create_modules_from_target(blueprint, desc, target)
    return blueprint


def repo_root():
    """Returns an absolute path to the repository root."""

    return os.path.join(
        os.path.realpath(os.path.dirname(__file__)), os.path.pardir)


def remove_last_brace_from_string(s):
    return s[:s.rfind('}')]

def get_text_description(gn_out, target_name):
    try:
        desc = subprocess.check_output(
            ['gn', 'desc', '--format=json', gn_out, target_name])
    except subprocess.CalledProcessError as e:
        logging.debug("e.retcode = %s" % e.returncode)
        logging.debug("e.cmd = %s" % e.cmd)
        logging.debug("e.output = %s" % e.output)
    postString = desc.split("\n",1)[1];
    trimDesc = remove_last_brace_from_string(postString)
    return trimDesc

def get_json_description(gn_out, target_name):
    try:
        text_desc = subprocess.check_output(
            ['gn', 'desc', '--format=json', gn_out, target_name])
    except subprocess.CalledProcessError as e:
        logging.debug("e.retcode = %s" % e.returncode)
        logging.debug("e.cmd = %s" % e.cmd)
        logging.debug("e.output = %s" % e.output)
    return json.loads(text_desc)

def load_deps(desc, gn_out, target_name, indent="  "):
    """Extracts dependencies from the given target json description
       and recursively extracts json descriptions.

       desc: json description that includes dependencies
       gn_out: GN output file with configuration info
       target_name: name of target in desc to lookup deps
       indent: Print with indent to show recursion depth
    """
    target = desc[target_name]
    text_descriptions = []
    for dep in target.get('deps', []):
        logging.debug("dep: %s%s" % (indent,dep))
        new_desc = get_text_description(gn_out, dep)
        text_descriptions.append(new_desc)
        dep_desc = json.loads("{ " + new_desc + " }")
        text_descriptions.extend(load_deps(dep_desc, dep, indent+"  "))
    return text_descriptions

def load_json_deps(desc, gn_out, target_name, all_desc, indent="  "):
    """Extracts dependencies from the given target json description
       and recursively extracts json descriptions.

       desc: json description for target_name that includes dependencies
       gn_out: GN output file with configuration info
       target_name: name of target in desc to lookup deps
       all_desc: dependent descriptions added here
       indent: Print with indent to show recursion depth
    """
    target = desc[target_name]
    text_descriptions = []
    for dep in target.get('deps', []):
        if dep not in all_desc:
            logging.debug("dep: %s%s" % (indent,dep))
            new_desc = get_json_description(gn_out, dep)
            all_desc[dep] = new_desc[dep]
            load_json_deps(new_desc, gn_out, dep, all_desc, indent+"  ")
        else:
            logging.debug("dup: %s%s" % (indent,dep))

def create_build_description(gn_out, targets):
    """Creates the JSON build description by running GN."""

    logging.debug("targets = %s" % targets)
    json_descriptions = {}
    for target in targets:
        logging.debug("target: %s" % (target))
        target_desc = get_json_description(gn_out, target)
        if (target in target_desc and target not in json_descriptions):
            json_descriptions[target] = target_desc[target]
            load_json_deps(target_desc, gn_out, target, json_descriptions)
        else:
            logging.debug("Invalid target: %s" % target)
    return json_descriptions


def main():
    logging.basicConfig(stream=sys.stderr, level=logging.WARN)
    parser = argparse.ArgumentParser(
        description='Generate Android.bp from a GN description.')
    parser.add_argument(
        '--gn_out',
        help=
        'GN output config to use (e.g., out/Default or out/Debug.)',
        default='out/Default',
    )
    parser.add_argument(
        '--desc',
        help=
        'GN description (e.g., gn desc out --format=json --all-toolchains "//*"'
    )
    parser.add_argument(
        '--extras',
        help='Extra targets to include at the end of the Blueprint file',
        default=os.path.join(repo_root(), 'Android.bp.extras'),
    )
    parser.add_argument(
        '--output',
        help='Blueprint file to create',
        default='Android.bp',
    )
    parser.add_argument(
        '--desc_output',
        help='json dependency description file to create',
    )
    parser.add_argument(
        'targets',
        nargs=argparse.REMAINDER,
        help='Targets to include in the blueprint (e.g., "//libEGL")')
    args = parser.parse_args()

    if args.desc:
        with open(args.desc) as f:
            desc = json.load(f)
    else:
        desc = create_build_description(args.gn_out, args.targets)
        if args.desc_output:
            fh = open(args.desc_output,"w")
            fh.write(json.dumps(desc, indent=4, sort_keys=True))
            fh.close()

    logging.debug("json_descriptions.keys() = %s" % desc.keys())

    blueprint = create_blueprint_for_targets(desc, args.targets
                                             or default_targets)
    output = [
        """// This file is automatically generated by %s. Do not edit.""" % (__file__)
    ]
    blueprint.to_string(output)
    if os.path.exists(args.extras):
        with open(args.extras, 'r') as r:
            for line in r:
                output.append(line.rstrip("\n\r"))
    with open(args.output, 'w') as f:
        f.write('\n'.join(output))

    print("Output written to: %s" % args.output)


if __name__ == '__main__':
    sys.exit(main())
