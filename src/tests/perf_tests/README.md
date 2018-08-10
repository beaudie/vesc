# ANGLE Performance Tests

`angle_perftests` is a standalone testing suite that contains targeted tests for OpenGL, Vulkan and ANGLE internal classes. The tests currently run on the Chromium ANGLE infrastructure and report results to the [Chromium perf dashboard](https://chromeperf.appspot.com/report).

You can also build your own dashboards. For example, a comparison of ANGLE's back-end draw call performance on Windows NVIDIA can be found [at this link](https://chromeperf.appspot.com/report?sid=1fdf94a308f52b6bf02c08f6f36e87ca0d0075e2d2eefc61e6cf90c919c1643a&start_rev=577814&end_rev=582136). Note that this link is not kept current.

## Running the Tests

You can follow the usual instructions to [check out and build ANGLE](../../../doc/DevSetup.md). Build the `angle_perftests` target. You can choose individual tests to run with `--gtest_filter=*TestName*`. You might need to experiment to find the right sub-test and its name. Note that all test scores are higher-is-better. You should also ensure `is_debug=false` in your build. Running with `dcheck_always_on` or debug validation enabled is not recommended.

Variance can be a problem when benchmarking. We have a test harness to run a single test in an infinite loop and print some statistics to help mitigate variance. See [`scripts/perf_test_runner.py`](https://chromium.googlesource.com/angle/angle/+/master/scripts/perf_test_runner.py). To use the script first compile `angle_perftests` into a folder with the word `Release` in it. Then provide the name of the test as the argument to the script. The script will automatically pick up the most current `angle_perftests` and run in an infinite loop.

## Test Breakdown

* [`DrawCallPerfBenchmark`](DrawCallPerf.cpp): Runs a tight loop around DrawArarys calls.
  * `validation_only`: Skips all rendering.
  * `render_to_texture`: Render to a user Framebuffer instead of the default FBO.
  * `vbo_change`: Applies a Vertex Array change between each draw.
  * `tex_change`: Applies a Texture change between each draw.
* [`UniformsBenchmark`](UniformsPerf.cpp): Tests performance of updating various uniforms counts followed by a DrawArrays call.
    * `vec4`: Tests `vec4` Uniforms.
    * `matrix`: Tests using Matrix uniforms instead of `vec4`.
    * `multiprogram`: Tests switching Programs between updates and draws.
    * `repeating`: Skip the update of uniforms before each draw call.
* [`DrawElementsPerfBenchmark`](DrawElementsPerf.cpp): Similar to `DrawCallPerfBenchmark` but for indexed DrawElements calls.
* [`BindingsBenchmark`](BindingPerf.cpp): Tests Buffer binding performance. Does no draw call operations.
* [`TexSubImageBenchmark`](TexSubImage.cpp): Tests `glTexSubImage` update performance.
* [`BufferSubDataBenchmark`](BufferSubData.cpp): Tests `glBufferSubData` update performance.
* [`TextureSamplingBenchmark`](TextureSampling.cpp): Tests Texture sampling performance.
* [`TextureBenchmark`](TexturesPerf.cpp): Tests Texture state change performance.
* [`LinkProgramBenchmark`](LinkProgramPerfTest.cpp): Tests performance of `glLinkProgram`.

Many other tests can be found that have documentation in their classes.
