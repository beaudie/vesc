#!/usr/bin/env vpython3
#
# Copyright 2022 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# mesa_build.py:
#   Helper script for building Mesa in an ANGLE checkout.

import argparse
import json
import logging
import os
import shutil
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ANGLE_DIR = os.path.dirname(os.path.dirname(SCRIPT_DIR))
DEFAULT_LOG_LEVEL = 'info'
EXIT_SUCCESS = 0
EXIT_FAILURE = 1
MESON = os.path.join(ANGLE_DIR, 'third_party', 'meson', 'meson.py')
MESA_SOURCE_DIR = os.path.join(ANGLE_DIR, 'third_party', 'mesa', 'src')
LIBDRM_SOURCE_DIR = os.path.join(ANGLE_DIR, 'third_party', 'libdrm')
LIBDRM_BUILD_DIR = os.path.join(ANGLE_DIR, 'out', 'libdrm-build')
MESA_STAMP = 'mesa.stamp'
LIBDRM_STAMP = 'libdrm.stamp'

MESA_OPTIONS = [
    '-Dzstd=disabled', '-Dplatforms=x11', '-Dgallium-drivers=zink', '-Dvulkan-drivers='
]
LIBDRM_OPTIONS = [
    '-Dtests=false', '-Dintel=disabled', '-Dnouveau=disabled', '-Damdgpu=disabled',
    '-Dradeon=disabled', '-Dvmwgfx=disabled'
]


def main(raw_args):
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-l',
        '--log',
        '--log-level',
        help='Logging level. Default is %s.' % DEFAULT_LOG_LEVEL,
        default=DEFAULT_LOG_LEVEL)

    subparser = parser.add_subparsers(dest='command')

    mesa = subparser.add_parser('mesa')
    mesa.add_argument('build_dir', help='Target build directory.')
    mesa.add_argument('-j', '--jobs', help='Compile jobs.')

    compile_parser = subparser.add_parser('compile')
    compile_parser.add_argument('-j', '--jobs', help='Compile jobs.')
    compile_parser.add_argument('build_dir', help='Target build directory.')

    gni = subparser.add_parser('gni')
    gni.add_argument('output', help='Output location for gni file.')
    gni.add_argument('mesa_build_dir', help='Target Mesa build directory.')
    gni.add_argument('libdrm_build_dir', help='Target libdrm build directory.')

    libdrm = subparser.add_parser('libdrm')

    runhook_parser = subparser.add_parser('runhook')
    runhook_parser.add_argument(
        '-o', '--output', help='Output location for stamp sha1 file.', default=MESA_STAMP)

    setup_parser = subparser.add_parser('setup')
    setup_parser.add_argument('build_dir', help='Target build directory.')

    args, extra_args = parser.parse_known_args(raw_args)

    logging.basicConfig(level=args.log.upper())

    assert os.path.exists(MESON), 'Could not find meson.py: %s' % MESON

    if args.command == 'mesa':
        SetupBuild(args.build_dir, MESA_SOURCE_DIR, MESA_OPTIONS)
        Compile(args, args.build_dir)
    elif args.command == 'compile':
        Compile(args, args.build_dir)
    elif args.command == 'gni':
        GenerateGni(args)
    elif args.command == 'libdrm':
        SetupBuild(LIBDRM_BUILD_DIR, LIBDRM_SOURCE_DIR, LIBDRM_OPTIONS)
        Compile(args, LIBDRM_BUILD_DIR)
    elif args.command == 'runhook':
        RunHook(args)
    elif args.command == 'setup':
        LazySetup(args.build_dir)

    return EXIT_SUCCESS


def SetupBuild(build_dir, source_dir, options):
    if not os.path.exists(build_dir):
        os.mkdir(build_dir)

    sysroot_dir = os.path.join(ANGLE_DIR, 'build', 'linux', 'debian_bullseye_amd64-sysroot')

    cflags = ' '.join([
        '--sysroot=%s' % sysroot_dir,
        '-Wno-constant-conversion',
        '-Wno-deprecated-builtins',
        '-Wno-deprecated-declarations',
        '-Wno-deprecated-non-prototype',
        '-Wno-enum-compare-conditional',
        '-Wno-enum-conversion',
        '-Wno-implicit-const-int-float-conversion',
        '-Wno-implicit-function-declaration',
        '-Wno-initializer-overrides',
        '-Wno-sometimes-uninitialized',
        '-Wno-unused-but-set-variable',
        '-Wno-unused-function',
    ])

    extra_env = {
        'CC':
            'clang',
        'CC_LD':
            'lld',
        'CXX':
            'clang++',
        'CXX_LD':
            'lld',
        'CFLAGS':
            cflags,
        'CXXFLAGS':
            cflags,
        'PKG_CONFIG_PATH':
            '%s:%s/usr/share/pkgconfig:%s/usr/lib/pkgconfig' %
            (os.path.join(LIBDRM_BUILD_DIR, 'meson-uninstalled'), sysroot_dir, sysroot_dir),
    }

    args = [source_dir, build_dir] + options
    if os.path.isdir(os.path.join(build_dir, 'meson-info')):
        args += ['--wipe']

    return Meson('setup', args, extra_env)


def Compile(args, build_dir):
    return Meson('compile', ['-C', build_dir])


def MakeEnv():
    clang_dir = os.path.join(ANGLE_DIR, 'third_party', 'llvm-build', 'Release+Asserts', 'bin')
    flex_bison_dir = os.path.join(ANGLE_DIR, 'tools', 'flex-bison')

    # TODO: Windows
    flex_bison_platform = 'linux'
    flex_bison_bin_dir = os.path.join(flex_bison_dir, flex_bison_platform)

    env = os.environ.copy()
    env['PATH'] = '%s:%s:%s' % (clang_dir, flex_bison_bin_dir, env['PATH'])
    env['BISON_PKGDATADIR'] = os.path.join(flex_bison_dir, 'third_party')
    return env


GNI_TEMPLATE = """\
# GENERATED FILE - DO NOT EDIT.
# Generated by {script_name}
#
# Copyright 2022 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# {filename}: ANGLE build information for Mesa.

angle_mesa_outputs = [
{angle_mesa_outputs}]

angle_mesa_sources = [
{angle_mesa_sources}]

angle_libdrm_outputs = [
{angle_libdrm_outputs}]

angle_libdrm_sources = [
{angle_libdrm_sources}]
"""


def GenerateGni(args):
    mesa_sources_filter = lambda target: target['type'] != 'shared library'
    mesa_outputs_filter = lambda target: target['type'] == 'shared library'
    mesa_sources, mesa_outputs = GetMesonSourcesAndOutputs(args.mesa_build_dir,
                                                           mesa_sources_filter,
                                                           mesa_outputs_filter)

    libdrm_sources_filter = lambda target: True
    libdrm_outputs_filter = lambda target: True
    libdrm_sources, libdrm_outputs = GetMesonSourcesAndOutputs(args.libdrm_build_dir,
                                                               libdrm_sources_filter,
                                                               libdrm_outputs_filter)

    fmt_list = lambda l, rp: ''.join(
        sorted(list(set(['  "%s",\n' % os.path.relpath(li, rp) for li in l]))))

    format_args = {
        'script_name': os.path.basename(__file__),
        'filename': os.path.basename(args.output),
        'angle_mesa_outputs': fmt_list(mesa_outputs, args.mesa_build_dir),
        'angle_mesa_sources': fmt_list(mesa_sources, MESA_SOURCE_DIR),
        'angle_libdrm_outputs': fmt_list(libdrm_outputs, args.libdrm_build_dir),
        'angle_libdrm_sources': fmt_list(libdrm_sources, LIBDRM_SOURCE_DIR),
    }

    gni_text = GNI_TEMPLATE.format(**format_args)

    with open(args.output, 'w') as outf:
        outf.write(gni_text)
        outf.close()
        logging.info('Saved GNI data to %s' % args.output)


def GetMesonSourcesAndOutputs(build_dir, sources_filter, output_filter):
    text_data = Meson('introspect', [build_dir, '--targets'], stdout=subprocess.PIPE)
    json_data = json.loads(text_data)
    outputs = []
    all_sources = []
    generated = []
    for target in json_data:
        generated += target['filename']
        if output_filter(target):
            outputs += target['filename']
        if sources_filter(target):
            for target_source in target['target_sources']:
                all_sources += target_source['sources']

    sources = list(filter(lambda s: (s not in generated), all_sources))

    for source in sources:
        assert os.path.exists(source), '%s does not exist' % source

    return sources, outputs


def Meson(command, args, extra_env={}, stdout=None):
    meson_cmd = ['vpython3', MESON, command] + args
    env = MakeEnv()
    for k, v in extra_env.items():
        env[k] = v
    logging.info(' '.join(['%s=%s' % (k, v) for (k, v) in extra_env.items()] + meson_cmd))
    completed = subprocess.run(meson_cmd, env=env, stdout=stdout)
    if completed.returncode != EXIT_SUCCESS:
        logging.fatal('Got error from meson')
        sys.exit(EXIT_FAILURE)
    if stdout:
        return completed.stdout


def RunHook(args):
    output = os.path.join(SCRIPT_DIR, args.output)
    Stamp(args, MESA_SOURCE_DIR, output)
    libdrm_out = os.path.join(SCRIPT_DIR, LIBDRM_STAMP)
    Stamp(args, LIBDRM_SOURCE_DIR, libdrm_out)


def Stamp(args, source_dir, output):
    commit_id = GrabOutput('git rev-parse HEAD', source_dir)
    with open(output, 'w') as outf:
        outf.write(commit_id)
        outf.close()
        logging.info('Saved git hash data to %s' % output)


def GrabOutput(command, cwd):
    return subprocess.Popen(
        command, stdout=subprocess.PIPE, shell=True, cwd=cwd).communicate()[0].strip().decode()


def LazySetup(build_dir):
    in_mesa_stamp = os.path.join(SCRIPT_DIR, MESA_STAMP)
    out_mesa_stamp = os.path.join(build_dir, 'mesa', MESA_STAMP)
    in_libdrm_stamp = os.path.join(SCRIPT_DIR, LIBDRM_STAMP)
    out_libdrm_stamp = os.path.join(build_dir, 'libdrm', LIBDRM_STAMP)

    if SameStamps(in_mesa_stamp, out_mesa_stamp) and SameStamps(in_libdrm_stamp, out_libdrm_stamp):
        logging.info('Setup up-to-date.')
        sys.exit(EXIT_SUCCESS)

    SetupBuild(os.path.join(build_dir, 'mesa'), MESA_SOURCE_DIR, MESA_OPTIONS)
    shutil.copyfile(in_mesa_stamp, out_mesa_stamp)
    SetupBuild(os.path.join(build_dir, 'libdrm'), LIBDRM_SOURCE_DIR, LIBDRM_OPTIONS)
    shutil.copyfile(in_libdrm_stamp, out_libdrm_stamp)
    logging.info('Finished setup and updated %s and %s.' % (MESA_STAMP, LIBDRM_STAMP))


def SameStamps(in_stamp, out_stamp):
    assert os.path.exists(in_stamp)
    if not os.path.exists(out_stamp):
        return False
    in_data = ReadFile(in_stamp)
    out_data = ReadFile(out_stamp)
    return in_data == out_data


def ReadFile(path):
    with open(path, 'rt') as inf:
        all_data = inf.read()
        inf.close()
        return all_data


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
