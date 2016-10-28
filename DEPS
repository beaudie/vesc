vars = {
  'android_git': 'https://android.googlesource.com',
  'chromium_git': 'https://chromium.googlesource.com',
}

deps = {
  "buildtools":
      Var('chromium_git') + '/chromium/buildtools.git@39b1db2ab4aa4b2ccaa263c29bdf63e7c1ee28aa',

  "src/tests/third_party/googlemock":
      Var('chromium_git') + "/external/googlemock.git@b2cb211e49d872101d991201362d7b97d7d69910",

  "src/tests/third_party/googletest":
      Var('chromium_git') + "/external/googletest.git@9855a87157778d39b95eccfb201a9dc90f6d61c6",

  # Cherry is a dEQP management GUI written in Go. We use it for viewing test results.
  "third_party/cherry":
      Var('android_git') + "/platform/external/cherry@d2e26b4d864ec2a6757e7f1174e464949ca5bf73",

  "third_party/deqp/src":
      Var('android_git') + "/platform/external/deqp@f4f3d8079e7a37d7675ab93583e6438d0bca0e58",

  "third_party/glslang/src":
      Var('android_git') + "/platform/external/shaderc/glslang@187254b92cef812ad77c8a27f656164b2abed74e",

  "third_party/gyp":
      Var('chromium_git') + "/external/gyp@81c2e5ff92af29bab61c982808076ddce3d200a2",

  "third_party/libpng":
      Var('android_git') + "/platform/external/libpng@094e181e79a3d6c23fd005679025058b7df1ad6c",

  "third_party/spirv-headers/src":
      Var('android_git') + "/platform/external/shaderc/spirv-headers@bd47a9abaefac00be692eae677daed1b977e625c",

  "third_party/spirv-tools-angle/src":
      Var('android_git') + "/platform/external/shaderc/spirv-tools@2d89b6a9c5b88ad34784b8275141187afcc5045b",

  "third_party/vulkan-validation-layers/src":
      Var('android_git') + "/platform/external/vulkan-validation-layers@665734bffa6e3364f2f12fe533a030f54274df66",

  "third_party/zlib":
      Var('chromium_git') + "/chromium/src/third_party/zlib@afd8c4593c010c045902f6c0501718f1823064a3",
}

hooks = [
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_win',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'buildtools/win/clang-format.exe.sha1',
    ],
  },
  {
    'name': 'clang_format_mac',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'buildtools/mac/clang-format.sha1',
    ],
  },
  {
    'name': 'clang_format_linux',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'buildtools/linux64/clang-format.sha1',
    ],
  },
  # Pull GN binaries using checked-in hashes.
  {
    'name': 'gn_win',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'buildtools/win/gn.exe.sha1',
    ],
  },
  {
    'name': 'gn_mac',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'buildtools/mac/gn.sha1',
    ],
  },
  {
    'name': 'gn_linux64',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'buildtools/linux64/gn.sha1',
    ],
  },
  {
    # A change to a .gyp, .gypi, or to GYP itself should run the generator.
    "pattern": ".",
    "action": ["python", "build/gyp_angle"],
  },
]

recursedeps = [
  # buildtools provides clang_format.
  'buildtools',
]
