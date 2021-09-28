# Suppressing Test Failures

ANGLE is tested by a number of test suites, including by the ANGLE CI and Try testers, and by the
Chromium FYI testers.  See pointers and explanations in the
[ANGLE Wrangling](https://chromium.googlesource.com/angle/angle/+/refs/heads/main/infra/ANGLEWrangling.md) documentation.

We run a large number of tests for each ANGLE CL, both in ANGLE standalone and Chromium
configurations, both pre- and post- commit.  Some tests will failure, crash, or timeout.  If these
cannot be addressed in a timely manner, file a bug and change expectations.


## Suppressing a Vulkan Validation Layer error

Many tests are run with an option that enables the Vulkan Validation Layers (sometimes referred to
as VVL).  Validation errors will cause an otherwise-passing test to fail.

The `vulkan-deps` autoroller updates ANGLE to the top-of-tree (ToT) upstream Vulkan tools and SDK.
Sometimes validation errors are the result of bugs in the Vulkan Validation Layers, sometimes
because of bugs in ANGLE.  Therefore, investigate the cause of the error and determine if it's an
ANGLE bug or a Vulkan Validation Layer bug.  For Vulkan Validation Layer bugs, file an upstream bug
(TODO: ADD LINK), and suppress the error.  The ANGLE Wrangler will also suppress a validation error
when the `vulkan-deps` autoroller introduces a new validation error.  The ANGLE Wrangler isn't
expected to resolve the error or diagnose an upstream bug (but it is welcome as extra credit).

Suppress a validation error by doing the following:

1. File a bug at anglebug.com/new.
2. Add the VVL error tag to the
   [kSkippedMessages](https://chromium.googlesource.com/angle/angle.git/+/8f8ca06dfb903fcc8517c69142c46c05e618f40d/src/libANGLE/renderer/vulkan/RendererVk.cpp#129)
   array in `RendererVk.cpp` file.  Follow the pattern for adding a comment with the associated bug
   in the line above the VVL tag.


## Suppressing test failures and crashes

Depending on the type of test, suppressing problems are handled in different ways:

- [dEQP tests](dEQP.md)
  - There are a set of expectations files in the `src/tests/deqp_support` directory.
    - Notice the format of a line, and your choices for OS, driver, etc, including:
      - `FAIL` - For a test that flakes often or persistently fails
      - `SKIP` - For a test that crashes
      - `TIMEOUT` - For a test that is very slow and may timeout
- angle_end2end_tests
  - These expectations all live in the
    [angle_end2end_tests_expectations.txt](angle_end2end_tests_expectations.txt) file.
  - These expectations all live in the `src/tests/angle_end2end_tests_expectations.txt` file.
  - The file format is the same as for the dEQP expectations.
- WebGL conformance tests
  - There are a set of expectation files that are not in an ANGLE-only source tree; they are in
    the `content/test/gpu/gpu_tests/test_expectations` directory of a Chromium source tree.
  - For really simple edits, the expectation files can be edited from CodeSearch, as follows:
    - In the browser, press the `Edit code` button.  This will bring up a new browser window/tab, in an editor mode.
    - Edit the expecations and then press the `Create change` (or `Update change` button if you need to change your CL later), which will create a CL.
  - Notice the format of a line, and your choices for OS, driver, etc (see the top of the file for
    an explanation of the different tags that can be used), including the following results:
    - `RetryOnFailure` - For a test that occasionally flakes, but won't flake more than once per
      test run
    - `Failure` - For a test that fails consistently or flakes often
    - `Skip` - For a test that causes catastrophic failures (e.g. entire test run ends, bot
      BSDOs); `Skip` should be used very sparingly
  - You will need to contact an OWNER of the file to +1 your CL.
