# ANGLE Internal Captures

The files in this directory are captures of real applications.  We host them
internally because they may contain third party IP which we don't want
to share publicly.

## Setup

In order to compile and run with these, you must be on Google's corp network
and logged in to your @google.com account.

Add the following to ANGLE's .gclient file:
```
    "custom_vars": {
      "checkout_angle_internal":"True"
    },
```
Then run gclient sync to pull down binary files from a cloud storage bucket.
```
gclient sync
```
To build the tests, add the following GN argument:
```
build_angle_capture_replay_tests = true
```
Build the angle_perftests:
```
autoninja -C out/Release angle_perftests
```
On desktop, run them like so:
```
out/Release/angle_perftests --gtest_filter=CaptureReplayPerfBenchmark*
```
On Android, run them like so:
```
out/Release/bin/run_angle_perftests --gtest_filter=CaptureReplayPerfBenchmark*
```
