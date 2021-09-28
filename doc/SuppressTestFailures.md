# Suppress Test Failures

ANGLE is tested by a number of test suites, including by the ANGLE CI and Try testers, and by the Chromium FYI testers.  See pointers and explanations in the [ANGLE Wrangling](https://chromium.googlesource.com/angle/angle/+/refs/heads/main/infra/ANGLEWrangling.md) documentation.

Some test failures, crashes, or timeouts can't be addressed in a timely manner.  There are a variety of methods for addressing these problems, such as:

- Suppress all failures caused by a Vulkan Validation Layer (VVL) error.
- Suppress failures and crashes, or in cases request that failing tests be retried
- Adjust timeouts

## Suppress VVL errors

Some of the ANGLE tests are run with an option that causes the test to fail if there's a VVL.  The test might otherwise pass, but will be marked as failing because of the VVL error.  It is best to fix all VVL errors, but if one cannot be fixed in a timely manner, the error can be suppressed.  It is better to suppress the VVL error than to suppress the test because of a VVL error.  In such a case, do the following:

1. File a bug about the errorfirst, and then 
2. Add the VVL error tag to the [kSkippedMessages](https://source.chromium.org/chromium/chromium/src/+/main:third_party/angle/src/libANGLE/renderer/vulkan/RendererVk.cpp;l=129?q=kSkippedMessages&ss=chromium%2Fchromium%2Fsrc) array in "RendererVk.cpp" file.  Follow the pattern for adding a comment with the associated bug in the line above the VVL tag.

## Suppress failures and crashes

Depending on the type of test, suppressing problems are handled in different ways:

- dEQP tests
  - There are a set of expectations files in the `src/tests/deqp_support` directory.
    - Notice the format of a line, and your choices for OS, driver, etc, including:
      - `FAIL` - For a test that flakes often or persistently fails
      - `SKIP` - For a test that crashes
- end2end tests
  - These expectations all live in the `src/tests/angle_end2end_tests_expectations.txt` file.
  - The format and choices are the same for the dEQP expectations.
- webgl tests
  - The [expectation file](https://source.chromium.org/chromium/chromium/src/+/main:content/test/gpu/gpu_tests/test_expectations/webgl_conformance_expectations.txt) is best edited from CodeSearch
  - In the browser, press the `Edit code` button.  This will bring up a new browser window/tab, in an editor mode.
  - Edit the expecations and then press the `Create change` (or `Update change` button if you need to change your CL later), which will create a CL.
    - Notice the format of a line, and your choices for OS, driver, etc, including:
      - `RetryOnFailure` - For a test that occasionally flakes, but won't flake more than once per test run
      - `Failure` - For a test that flakes often
      - `Skip` - For a test that crashes
  - You will need to contact an OWNER of the file to +1 your CL.

## Adjust timeouts

TBD
