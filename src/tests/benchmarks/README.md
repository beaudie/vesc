# ANGLE Benchmarking Tests

`angle_benchmarks` is an interface through which various third party benchmarks are run, and their
results are transformed in a way ANGLE can use.

The tests will run on the Chromium ANGLE infrastructure and report results to the
[Chromium perf dashboard](https://chromeperf.appspot.com/report).

## Running the Tests

You can follow the usual instructions to [check out and build ANGLE](../../../doc/DevSetup.md).
Build the `angle_benchmark` target. Note that all test scores are higher-is-better. You should also
ensure `is_debug=false` in your build. Running with `dcheck_always_on` or debug validation enabled
is not recommended.

### Choosing the Test to Run

You can choose individual tests to run with `--gtest_filter=*TestName*`. To select a particular
ANGLE back-end, add the name of the back-end to the test filter. For example:
`glmark2Benchmark.Run/gl` or `Glmark2Benchmark.Run/vulkan`.

## Test Breakdown

* [`glmark2`](glmark2.cpp): Runs the glmark2 benchmark.
