vars = {
  'android_git': 'https://android.googlesource.com',
  'chromium_git': 'https://chromium.googlesource.com',
}

deps = {

  # Each dep is listed twice. The normal .gclient layout as created by `fetch`
  # and as used on the bots is to have an entry for the main repo ('angle')
  # which gclient would then put in an 'angle' subdirectory. All paths are
  # relative to .gclient, so then all dep paths and hooks paths should start
  # with 'angle'. In this setup, 'angle/build' would create the directory
  # 'build' inside the 'angle' dir created for angle itself by gclient,
  # and the relative path from the angle dir to build is just 'build'.
  #
  # Angle's bootstrap.py currently uses a clever trick to set the main
  # gclient config to '.' (which gclient forbids, but which bootstrap.py works
  # around). In that setup, 'angle/build' would end up as 'angle/build' inside
  # the angle dir and the relative path from the angle dir to build is
  # 'angle/build'.
  #
  # Since things assume that the relative path from the angle directory to
  # the build directory is just 'build', check out build both at
  # 'angle/build' (for the regular build setup) and at 'build'
  # (for the build setup that bootstrap.py currently creates).
  #
  # TODO(thakis): Update angle dev docs to recommend the `fetch` setup, and
  # ask people to migrate their local standalone checkouts over the next few
  # month, then get rid of the duplicate checkouts.

  'angle/build':
    Var('chromium_git') + '/chromium/src/build.git' + '@' + '2f3b6e8ce9e783b2a09496d70eef2974506a41c8',
  'build':
    Var('chromium_git') + '/chromium/src/build.git' + '@' + '2f3b6e8ce9e783b2a09496d70eef2974506a41c8',

  'angle/buildtools':
    Var('chromium_git') + '/chromium/buildtools.git' + '@' + '461b345a815c1c745ac0534a6a4bd52d123abe68',
  'buildtools':
    Var('chromium_git') + '/chromium/buildtools.git' + '@' + '461b345a815c1c745ac0534a6a4bd52d123abe68',

  'angle/testing':
    Var('chromium_git') + '/chromium/src/testing' + '@' + '6dfa36ab2e5143fa2f7353e3af5d2935af2e61f7',
  'testing':
    Var('chromium_git') + '/chromium/src/testing' + '@' + '6dfa36ab2e5143fa2f7353e3af5d2935af2e61f7',

  # Cherry is a dEQP management GUI written in Go. We use it for viewing test results.
  'angle/third_party/cherry':
    Var('android_git') + '/platform/external/cherry' + '@' + '4f8fb08d33ca5ff05a1c638f04c85bbb8d8b52cc',
  'third_party/cherry':
    Var('android_git') + '/platform/external/cherry' + '@' + '4f8fb08d33ca5ff05a1c638f04c85bbb8d8b52cc',

  'angle/third_party/deqp/src':
    Var('android_git') + '/platform/external/deqp' + '@' + '5bc346ba2d5465a2e6094e254f12b1586fd0097f',
  'third_party/deqp/src':
    Var('android_git') + '/platform/external/deqp' + '@' + '5bc346ba2d5465a2e6094e254f12b1586fd0097f',

  'angle/third_party/glslang/src':
    Var('android_git') + '/platform/external/shaderc/glslang' + '@' + '2edde6665d9a56ead5ea0e55b4e64d9a803e6164',
  'third_party/glslang/src':
    Var('android_git') + '/platform/external/shaderc/glslang' + '@' + '2edde6665d9a56ead5ea0e55b4e64d9a803e6164',

  'angle/third_party/googletest/src':
    Var('chromium_git') + '/external/github.com/google/googletest.git' + '@' + 'd175c8bf823e709d570772b038757fadf63bc632',
  'third_party/googletest/src':
    Var('chromium_git') + '/external/github.com/google/googletest.git' + '@' + 'd175c8bf823e709d570772b038757fadf63bc632',

  'angle/third_party/libpng/src':
    Var('android_git') + '/platform/external/libpng' + '@' + '094e181e79a3d6c23fd005679025058b7df1ad6c',
  'third_party/libpng/src':
    Var('android_git') + '/platform/external/libpng' + '@' + '094e181e79a3d6c23fd005679025058b7df1ad6c',

  'angle/third_party/spirv-headers/src':
    Var('android_git') + '/platform/external/shaderc/spirv-headers' + '@' + '98b01515724c428d0f0a5d01deffcce0f5f5e61c',
  'third_party/spirv-headers/src':
    Var('android_git') + '/platform/external/shaderc/spirv-headers' + '@' + '98b01515724c428d0f0a5d01deffcce0f5f5e61c',

  'angle/third_party/spirv-tools/src':
    Var('android_git') + '/platform/external/shaderc/spirv-tools' + '@' + '9996173f363729b3a97309685dbd4d78547a63a7',
  'third_party/spirv-tools/src':
    Var('android_git') + '/platform/external/shaderc/spirv-tools' + '@' + '9996173f363729b3a97309685dbd4d78547a63a7',

  'angle/third_party/vulkan-validation-layers/src':
    Var('android_git') + '/platform/external/vulkan-validation-layers' + '@' + '354ad3ba8b88136b82b712acab9b8fcb981beaff',
  'third_party/vulkan-validation-layers/src':
    Var('android_git') + '/platform/external/vulkan-validation-layers' + '@' + '354ad3ba8b88136b82b712acab9b8fcb981beaff',

  'angle/third_party/zlib':
    Var('chromium_git') + '/chromium/src/third_party/zlib' + '@' + '24ab14872e8e068ba08cc31cc3d43bcc6d5cb832',
  'third_party/zlib':
    Var('chromium_git') + '/chromium/src/third_party/zlib' + '@' + '24ab14872e8e068ba08cc31cc3d43bcc6d5cb832',

  'angle/tools/clang':
    Var('chromium_git') + '/chromium/src/tools/clang.git' + '@' + 'e70074db10b27867e6c873adc3ac7e5f9ee0ff6e',
  'tools/clang':
    Var('chromium_git') + '/chromium/src/tools/clang.git' + '@' + 'e70074db10b27867e6c873adc3ac7e5f9ee0ff6e',

  'angle/tools/gyp':
    Var('chromium_git') + '/external/gyp' + '@' + '5e2b3ddde7cda5eb6bc09a5546a76b00e49d888f',
  'tools/gyp':
    Var('chromium_git') + '/external/gyp' + '@' + '5e2b3ddde7cda5eb6bc09a5546a76b00e49d888f',
}

hooks = [
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_win',
    'pattern': '.',
    'condition': 'host_os == "win"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'angle/buildtools/win/clang-format.exe.sha1',
    ],
  },
  {
    'name': 'clang_format_mac',
    'pattern': '.',
    'condition': 'host_os == "mac"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'angle/buildtools/mac/clang-format.sha1',
    ],
  },
  {
    'name': 'clang_format_linux',
    'pattern': '.',
    'condition': 'host_os == "linux"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'angle/buildtools/linux64/clang-format.sha1',
    ],
  },
  # Pull GN binaries using checked-in hashes.
  {
    'name': 'gn_win',
    'pattern': '.',
    'condition': 'host_os == "win"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'angle/buildtools/win/gn.exe.sha1',
    ],
  },
  {
    'name': 'gn_mac',
    'pattern': '.',
    'condition': 'host_os == "mac"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'angle/buildtools/mac/gn.sha1',
    ],
  },
  {
    'name': 'gn_linux64',
    'pattern': '.',
    'condition': 'host_os == "linux"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'angle/buildtools/linux64/gn.sha1',
    ],
  },
  {
    # Update the Windows toolchain if necessary.  Must run before 'clang' below.
    'name': 'win_toolchain',
    'pattern': '.',
    'condition': 'checkout_win',
    'action': ['python', 'angle/build/vs_toolchain.py', 'update', '--force'],
  },
  {
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    'action': ['python', 'angle/tools/clang/scripts/update.py', '--if-needed'],
  },

  # Pull rc binaries using checked-in hashes.
  {
    'name': 'rc_win',
    'pattern': '.',
    'condition': 'checkout_win and host_os == "win"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-browser-clang/rc',
                '-s', 'angle/build/toolchain/win/rc/win/rc.exe.sha1',
    ],
  },
]

recursedeps = [
  # buildtools provides clang_format.
  'angle/buildtools',
]
