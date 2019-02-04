//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// glmark2:
//   Runs the glmark2 benchmark.
//

#include <gtest/gtest.h>

#include <stdio.h>
#include <sstream>

#include "../perf_tests/third_party/perf/perf_test.h"
#include "ANGLEPerfTestArgs.h"
#include "common/platform.h"
#include "common/system_utils.h"
#include "test_utils/angle_test_configs.h"
#include "test_utils/angle_test_instantiate.h"

using namespace angle;
using namespace args;

namespace
{

class glmark2Benchmark : public testing::Test,
                         public ::testing::WithParamInterface<PlatformParameters>
{
  public:
    glmark2Benchmark()
    {
        switch (GetParam().getRenderer())
        {
            case EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE:
                mBackend = "gl";
                break;
            case EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE:
                mBackend = "vulkan";
                break;
            default:
                break;
        }
    }

    void run()
    {
        Optional<std::string> cwd = GetCWD();

        // Set the current working directory to the executable's, as the data path of glmark2 is
        // set relative to that path.
        std::string executableDir = GetExecutableDirectory();
        SetCWD(executableDir.c_str());
        SetEnvironmentVar("ANGLE_DEFAULT_PLATFORM", mBackend);

        std::vector<const char *> args = {
            "glmark2_angle",
        };
        if (OneFrame())
        {
            args.push_back("--validate");
        }
        args.push_back(nullptr);

        std::string output;
        int exitCode;

        bool success = RunApp(args, &output, nullptr, &exitCode);

        // Restore the current working directory for the next tests.
        if (cwd.valid())
        {
            SetCWD(cwd.value().c_str());
        }

        ASSERT_TRUE(success);
        ASSERT_EQ(exitCode, EXIT_SUCCESS);

        parseOutput(output);
    }

  private:
    void parseOutput(const std::string &output)
    {
        // Output is in the following format:
        //
        // =======================================================
        //     glmark2 2017.07
        // =======================================================
        //     OpenGL Information
        //     GL_VENDOR:     ...
        //     GL_RENDERER:   ...
        //     GL_VERSION:    ...
        // =======================================================
        // [test] config: FPS: uint FrameTime: float ms
        // [test] config: Not Supported
        // ...
        // =======================================================
        //                                   glmark2 Score: uint
        // =======================================================
        //
        // This function skips the header, prints one line for each test/config line where there's
        // an FPS value, and finally prints the overall score.
        std::istringstream sin(output);
        std::string line;

        // Expect ==== at the top of the header
        std::getline(sin, line);
        ASSERT_EQ(line[0], '=');

        // Skip one line
        std::getline(sin, line);

        // Expect ==== in the middle of the header
        std::getline(sin, line);
        ASSERT_EQ(line[0], '=');

        // Skip four lines
        std::getline(sin, line);
        std::getline(sin, line);
        std::getline(sin, line);
        std::getline(sin, line);

        // Expect ==== at the bottom of the header
        std::getline(sin, line);
        ASSERT_EQ(line[0], '=');

        // Read configs until the top of the footer is reached
        while (std::getline(sin, line) && line[0] != '=')
        {
            // Parse the line
            std::istringstream lin(line);

            std::string testName, testConfig;
            lin >> testName >> testConfig;
            EXPECT_TRUE(lin);

            std::string fpsTag, frametimeTag;
            size_t fps;
            float frametime;

            lin >> fpsTag >> fps >> frametimeTag >> frametime;

            // If the line is not in `FPS: uint FrameTime: Float ms` format, the test is not
            // supported.  It will be skipped.
            if (!lin)
            {
                continue;
            }

            EXPECT_EQ(fpsTag, "FPS:");
            EXPECT_EQ(frametimeTag, "FrameTime:");

            perf_test::PrintResult(testName + testConfig, mBackend, "fps", fps, "", true);
        }

        // Get the score line: `glmark2 Score: uint`
        std::string glmark2Tag, scoreTag;
        size_t score;
        sin >> glmark2Tag >> scoreTag >> score;
        EXPECT_TRUE(sin);
        EXPECT_EQ(glmark2Tag, "glmark2");
        EXPECT_EQ(scoreTag, "Score:");

        perf_test::PrintResult("glmark2", mBackend, "score", score, "", true);
    }

    const char *mBackend = "invalid";
};

TEST_P(glmark2Benchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(glmark2Benchmark, ES2_OPENGLES(), ES2_VULKAN());

}  // namespace
