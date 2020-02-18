# ANGLE for Android

Starting with Android 10 (Q), ANGLE can be loaded as the GLES driver for **debuggable apps** or users with **root access**.

To build ANGLE for Android, you must should first bootstrap your build following the steps in [DevSetup](DevSetup.md), specifically the sections for [Building ANGLE for Android](DevSetup.md#Building-ANGLE-for-Android).  If you can sucessfully build ANGLE using those steps, the steps in this document should work.  They will result in an APK that contains the ANGLE libraries and can be installed on any Android 10 build.

Apps can be opted in to ANGLE [one at a time](#ANGLE-for-a-single-GLES-app), in [groups](#ANGLE-for-multiple-GLES-apps), or [globally](#ANGLE-for-all-GLES-apps), but they must be launched by the Java runtime since the libraries are discovered within an installed package.  This means ANGLE cannot be used by native executables or SurfaceFlinger, at this time.

## Build the Chromium ANGLE APK

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
## Install the Chromium ANGLE APK
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