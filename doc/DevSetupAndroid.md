# ANGLE for Android

Building ANGLE for Android is heavily dependent on the Chromium toolchain. It is not currently possible to build ANGLE for Android without a Chromium checkout. See http://anglebug.com/2344 for more details on why.

From a Linux platform (the only platform that Chromium for Android supports), follow the steps in
[Checking out and building Chromium for Android](https://chromium.googlesource.com/chromium/src/+/master/docs/android_build_instructions.md).

Name your output directories `out/Debug` and `out/Release`, because Chromium GPU tests look for browser binaries in these folders. Replacing `out` with other names seems to be OK when working with multiple build configurations.

It's best to use a build configuration of some Android bot on [GPU.FYI waterfall](https://ci.chromium.org/p/chromium/g/chromium.gpu.fyi/console).
 - Look for `generate_build_files` step output of that bot.
 - Remove `goma_dir` flag.
 - For example, these are the build flags from Nexus 5X bot:
    ```
    build_angle_deqp_tests = true
    dcheck_always_on = true
    ffmpeg_branding = "Chrome"
    is_component_build = false
    is_debug = false
    proprietary_codecs = true
    symbol_level = 1
    target_cpu = "arm64"          # Nexus 5X is 64 bit, remove this on 32 bit only devices
    target_os = "android"
    use_goma = true               # Remove this if you don't have goma
    ```
    Additional flags to build the Vulkan backend, enable only if running on Android O or higher:
    ```
    android32_ndk_api_level = 26
    android64_ndk_api_level = 26
    ```

These ANGLE targets are supported:
`ninja -C out/Release translator libEGL libGLESv2 angle_unittests angle_end2end_tests angle_white_box_tests angle_deqp_gles2_tests angle_deqp_gles3_tests angle_deqp_egl_tests angle_perftests angle_white_box_perftests angle_apks`

In order to run ANGLE tests, prepend `bin/run_` to the test name, for example: `./out/Release/bin/run_angle_unittests`.

Additional details are in [Android Test Instructions](https://chromium.googlesource.com/chromium/src/+/master/docs/testing/android_test_instructions.md).

**dEQP Note**: Running the tests not using the test runner is tricky, but is necessary in order to get a complete TestResults.qpa from the dEQP tests (since the runner shards the tests, only the results of the last shard will be available when using the test runner). First, use the runner to install the APK, test data and test expectations on the device. After the tests start running, the test runner can be stopped with Ctrl+C. Then, run
```
adb shell am start -a android.intent.action.MAIN -n org.chromium.native_test/.NativeUnitTestNativeActivity -e org.chromium.native_test.NativeTest.StdoutFile /sdcard/chromium_tests_root/out.txt
```
After the tests finish, get the results with
```
adb pull /sdcard/chromium_tests_root/third_party/angle/third_party/deqp/src/data/TestResults.qpa .
```
Note: this location might change, one can double-check with `adb logcat -d | grep qpa`.

In order to run GPU telemetry tests, build `chrome_public_apk` target. Then follow [GPU Testing](http://www.chromium.org/developers/testing/gpu-testing#TOC-Running-the-GPU-Tests-Locally) doc, using `--browser=android-chromium` argument. Make sure to set your `CHROMIUM_OUT_DIR` environment variable, so that your browser is found, otherwise the stock one will run.

Also, follow [How to build ANGLE in Chromium for dev](BuildingAngleForChromiumDevelopment.md) to work with Top of Tree ANGLE in Chromium.

## Using ANGLE as the Android GLES driver

Starting with Android 10 (Q), ANGLE can be loaded as the GLES driver.

`== Important Note ==` ANGLE built this way can only be used for *DEBUGGABLE APPS* (i.e. [marked debuggable](https://developer.android.com/guide/topics/manifest/application-element#debug) in the manifest) or users with *ROOT ACCESS* (i.e. a [userdebug](https://source.android.com/setup/build/building) build).

To build the ANGLE APK, you must should first bootstrap your build following the steps [above](#ANGLE-for-Android).  The steps below will result in an APK that contains the ANGLE libraries and can be installed on any Android 10+ build.

Apps can be opted in to ANGLE [one at a time](#ANGLE-for-a-single-GLES-app), in [groups](#ANGLE-for-multiple-GLES-apps), or [globally](#ANGLE-for-all-GLES-apps), but they must be launched by the Java runtime since the libraries are discovered within an installed package.  This means ANGLE cannot be used by native executables or SurfaceFlinger, at this time.

## Build the ANGLE APK

To build ANGLE for Android, use the following `gn args`
```
target_os = "android"
target_cpu = "arm64"
android32_ndk_api_level = 26
android64_ndk_api_level = 26
dcheck_always_on = true
ffmpeg_branding = "Chrome"
is_component_build = false
proprietary_codecs = true
symbol_level = 1
ignore_elf32_limitations = true
angle_enable_vulkan = true
angle_libs_suffix = "_angle"

# Set the following to false for Release build
is_debug = true
```
Build with the following command, presuming you've configured in `out/Debug`
```
autoninja -C out/Debug angle_apks
```
## Install the ANGLE APK
```
adb install out/Debug/apks/AngleLibraries.apk
```
You can verify installation by looking for the package name:
```
adb shell pm path com.chromium.angle
package:/data/app/com.chromium.angle-HpkUceNFjoLYKPbIVxFWLQ==/base.apk
```
## Selecting ANGLE as the GLES driver

For debuggable applications or root users, you can load ANGLE libraries from the installed package.
```
adb shell settings put global angle_debug_package com.chromium.angle
```
Remember that ANGLE can only be used by applications launched by the Java runtime.
## ANGLE for a *single* GLES app
```
adb shell settings put global angle_gl_driver_selection_pkgs <package name>
adb shell settings put global angle_gl_driver_selection_values angle
```
## ANGLE for *multiple* GLES apps
Similar to selecting a single app, you can select multiple applications by listing their package names and driver choice in comma separated lists.  Note the lists must be the same length, one `angle` per package name.
```
adb shell settings put global angle_gl_driver_selection_pkgs <package name 1>,<package name 2>,<package name 3>,...
adb shell settings put global angle_gl_driver_selection_values angle,angle,angle,...
```
## ANGLE for *all* GLES apps
```
adb shell settings put global angle_gl_driver_all_angle 1
```
## Check for success
Check to see that ANGLE was loaded by your application:
```
adb logcat -d | grep ANGLE
V GraphicsEnvironment: ANGLE developer option for <package name>: angle
I GraphicsEnvironment: ANGLE package enabled: com.chromium.angle
I ANGLE   : Version (2.1.0.f87fac56d22f), Renderer (Vulkan 1.1.87(Adreno (TM) 615 (0x06010501)))
```
## Clean up
Settings persist across reboots, so it is a good idea to delete them when finished.
```
adb shell settings delete global angle_debug_package
adb shell settings delete global angle_gl_driver_all_angle
adb shell settings delete global angle_gl_driver_selection_pkgs
adb shell settings delete global angle_gl_driver_selection_values
```
## Troubleshooting
If your application is not debuggable, or you are not root, you will see an error like this in the log
```
adb logcat -d | grep ANGLE
V GraphicsEnvironment: ANGLE developer option for <package name>: angle
E GraphicsEnvironment: Invalid number of ANGLE packages. Required: 1, Found: 0
E GraphicsEnvironment: Failed to find ANGLE package.
```
Double check that you are root, or that your application is [marked debuggable](https://developer.android.com/guide/topics/manifest/application-element#debug).