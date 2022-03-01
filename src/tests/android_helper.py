import logging
import tarfile
import glob
import sys
import os
import subprocess
import random
import re
import hashlib
import posixpath

d = os.path.dirname
THIS_DIR = d(os.path.abspath(__file__))
ANGLE_DIR = d(d(THIS_DIR))

sys.path.append(os.path.join(ANGLE_DIR, 'third_party', 'catapult', 'devil'))
from devil.android.sdk import adb_wrapper
from devil.android import apk_helper

sys.path.append(os.path.join(ANGLE_DIR, 'testing'))
sys.path.append(os.path.join(ANGLE_DIR, 'testing', 'scripts'))

import common


def _random_hex():
    return hex(random.randint(0, 2**64))[2:]


def run(cmd, log=False):
    logging.info('qwe2 cmd %s', cmd)
    out = subprocess.check_output(cmd)
    if log:
        for ln in out.decode('utf-8').split('\n'):
            logging.info('qwe2 out=%s', ln)
    return out


def install_apk(suite):

    adb_path = adb_wrapper.AdbWrapper.GetAdbPath()
    # constants.GetOutDirectory() seems to be set to ./. on bots and locally
    apk_path = os.path.join('%s_apk' % suite, '%s-debug.apk' % suite)
    #apk_path = '/home/romanl/Downloads/angle_perftests-debug.apk'
    logging.info('qwe2 adb_path=%s apk_path=%s apk_size=%s' %
                 (adb_path, apk_path, os.path.getsize(apk_path)))

    # FIXME:
    # [INSTALL_FAILED_UPDATE_INCOMPATIBLE: Package com.android.angle.test
    # signatures do not match previously installed version; ignoring!]
    #run([adb_path, 'uninstall', 'com.android.angle.test'])

    run([adb_path, 'install', '-r', '-d', apk_path])

    apkh = apk_helper.ToHelper(apk_path)
    permissions = apkh.GetPermissions()
    logging.info('qwe2 permissions=%s', permissions)

    def tadd(tar, f):
        assert (f.startswith('../../'))
        tar.add(f, arcname=f.replace('../../', ''))

    run([adb_path, 'shell', 'mkdir -p /sdcard/chromium_tests_root/'])

    with common.temporary_file() as tempfile_path:
        with tarfile.open(tempfile_path, 'w', format=tarfile.GNU_FORMAT) as tar:
            for f in glob.glob('../../src/tests/restricted_traces/*/*.json', recursive=True):
                tadd(tar, f)
            #tadd(tar, '../../src/tests/restricted_traces/restricted_traces.json')
        run([adb_path, 'push', tempfile_path, '/sdcard/chromium_tests_root/tart.tar'])

    run([
        adb_path, 'shell',
        'tar -xf /sdcard/chromium_tests_root/tart.tar -C /sdcard/chromium_tests_root/'
    ])

    #run([adb_path, 'shell', 'ls /sdcard/chromium_tests_root/src/tests/restricted_traces/'])

    run([
        adb_path, 'shell',
        'p=com.android.angle.test;for q in android.permission.CAMERA android.permission.CHANGE_CONFIGURATION android.permission.READ_EXTERNAL_STORAGE android.permission.RECORD_AUDIO android.permission.WRITE_EXTERNAL_STORAGE;do pm grant "$p" "$q";echo "~X~$q~X~$?~X~";done;appops set com.android.angle.test MANAGE_EXTERNAL_STORAGE allow;echo "~X~MANAGE_EXTERNAL_STORAGE~X~$?~X~"'
    ])


def _read_device_file(adb_path, path):
    out_wc = run([adb_path, 'shell', 'cat %s | wc -c' % path])
    expected_size = int(out_wc.decode('ascii').strip())
    out = run([adb_path, 'shell', 'cat %s' % path])
    if len(out) != expected_size:
        raise Exception('Sanity check failed: %d != %d' % (len(out), expected_size))
    return out


def list_tests():
    adb_path = adb_wrapper.AdbWrapper.GetAdbPath()

    run([adb_path, 'shell', 'rm -f /sdcard/Download/temp_file-db7f74576d6f5.gtest_out'])

    posixpath.join(_random_hex())

    cmd = ('p=com.android.angle.test;'
           'ntr=org.chromium.native_test.NativeTestInstrumentationTestRunner;'
           'am instrument -w -e '
           '$ntr.NativeTestActivity '
           '"$p".AngleUnitTestActivity -e $ntr.ShardNanoTimeout 2400000000000 '
           '-e org.chromium.native_test.NativeTest.CommandLineFlags '
           '"--list-tests" '
           '-e $ntr.StdoutFile '
           '/sdcard/Download/temp_file-db7f74576d6f5.gtest_out '
           '"$p"/org.chromium.build.gtest_apk.NativeTestInstrumentationTestRunner')
    run([adb_path, 'shell', cmd])

    content = _read_device_file(adb_path, '/sdcard/Download/temp_file-db7f74576d6f5.gtest_out')
    return content.decode('utf-8').split('\n')


def copy_angledata(orig_fn):
    adb_path = adb_wrapper.AdbWrapper.GetAdbPath()

    full_fn = 'src/tests/restricted_traces/' + orig_fn + '/' + orig_fn + '.angledata.gz'
    run([adb_path, 'push', '../../' + full_fn, '/sdcard/chromium_tests_root/' + full_fn])


def run_and_get_output2(args, cmd, env):
    adb_path = adb_wrapper.AdbWrapper.GetAdbPath()

    run([adb_path, 'shell', 'rm -f /sdcard/Download/temp_file-db7f74576d6f5.gtest_out'])
    actual_cmd = (
        'p=com.android.angle.test;am instrument -w -e '
        'org.chromium.native_test.NativeTestInstrumentationTestRunner.NativeTestActivity '
        '"$p".AngleUnitTestActivity -e org.chromium.native_test.NativeTestInstrumentationTestRunner.ShardNanoTimeout 2400000000000 '
        '-e org.chromium.native_test.NativeTest.CommandLineFlags '
        '"' + ' '.join(cmd[1:]) + '" '
        '-e org.chromium.native_test.NativeTestInstrumentationTestRunner.StdoutFile '
        '/sdcard/Download/temp_file-db7f74576d6f5.gtest_out '
        '"$p"/org.chromium.build.gtest_apk.NativeTestInstrumentationTestRunner')
    run([adb_path, 'shell', '( %s );echo %%RETURN_CODE=$?' % actual_cmd], log=True)

    out = _read_device_file(adb_path, '/sdcard/Download/temp_file-db7f74576d6f5.gtest_out')
    # FIXME: always success?
    return 0, out.decode('utf-8').split('\n')  # FIXME


def remove_file_from_device(fn):
    adb_path = adb_wrapper.AdbWrapper.GetAdbPath()

    run([adb_path, 'shell', 'rm -f ' + fn])


def temp_device_dir():
    adb_path = adb_wrapper.AdbWrapper.GetAdbPath()
    path = '/sdcard/temp-%s' % _random_hex()
    run([adb_path, 'shell', 'mkdir -p ' + path])
    return path


def remove_dir_from_device(fn):
    adb_path = adb_wrapper.AdbWrapper.GetAdbPath()

    run([adb_path, 'shell', 'rm -rf ' + fn])


def pull(device_fn, local_fn):
    adb_path = adb_wrapper.AdbWrapper.GetAdbPath()

    run([adb_path, 'pull', device_fn, local_fn])


def pull_dir(device_dir, local_dir):
    # pull doesn't seem to work reliably when other files are present
    adb_path = adb_wrapper.AdbWrapper.GetAdbPath()

    files = run([adb_path, 'shell', 'ls -1 %s' % device_dir]).decode('utf-8').split('\n')
    for f in files:
        f = f.strip()
        if f:
            run([adb_path, 'pull', device_dir + '/' + f, local_dir + '/' + f])


if __name__ == '__main__':
    install_apk('angle_perftests')
    for ln in list_tests():
        if 'TracePerfTest.Run/' in ln:
            print(ln)
    copy_angledata('trex_200')
    ttt = temp_device_dir()
    gtest_filter = '--gtest_filter=TracePerfTest.Run/vulkan_trex_200'
    cmd = [
        None,
        gtest_filter,
        '--render-test-output-dir=%s' % ttt,
        '--one-frame-only',
        '--verbose-logging',
        '--enable-all-trace-tests',
    ]

    _, lines = run_and_get_output2(None, cmd, None)
    for ln in lines:
        print(ln)

    remove_dir_from_device(ttt)
