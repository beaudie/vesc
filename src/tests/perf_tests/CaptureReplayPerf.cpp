//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CaptureReplayPerf:
//   Performance test for ANGLE replaying captured frames.
//

#include <gtest/gtest.h>
#include "common/system_utils.h"
#include "tests/perf_tests/ANGLEPerfTest.h"
#include "util/egl_loader_autogen.h"

#include "internal_captures/trex_1300_1310/trex_1300_1310_capture_context1.h"
#include "internal_captures/trex_200_210/trex_200_210_capture_context1.h"
#include "internal_captures/trex_800_810/trex_800_810_capture_context1.h"
#include "internal_captures/trex_900_910/trex_900_910_capture_context1.h"

#include <cassert>
#include <sstream>

using namespace angle;
using namespace egl_platform;

namespace
{

struct CaptureReplayPerfParams final : public RenderTestParams
{
    // Common default options
    CaptureReplayPerfParams()
    {
        majorVersion = 2;
        minorVersion = 0;
        windowWidth  = 1920;
        windowHeight = 1080;
        trackGpuTime = true;

        // Display the frame after every drawBenchmark invocation
        iterationsPerStep = 1;

        startFrame = 0;
        endFrame   = 0;

        trexEnabled = false;
    }

    std::string story() const override
    {
        std::stringstream strstr;

        strstr << RenderTestParams::story();

        if (trexEnabled)
        {
            strstr << "_trex_" << startFrame << "_" << endFrame;
        }

        return strstr.str();
    }

    int startFrame;
    int endFrame;
    bool trexEnabled;
};

std::ostream &operator<<(std::ostream &os, const CaptureReplayPerfParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

class CaptureReplayPerfBenchmark : public ANGLERenderTest,
                                   public ::testing::WithParamInterface<CaptureReplayPerfParams>
{
  public:
    CaptureReplayPerfBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  private:
    int mCurrentFrame;
};

CaptureReplayPerfBenchmark::CaptureReplayPerfBenchmark()
    : ANGLERenderTest("CaptureReplayPerf", GetParam()), mCurrentFrame(0)
{}

void CaptureReplayPerfBenchmark::initializeBenchmark()
{
    const auto &params = GetParam();

    ASSERT_TRUE(params.endFrame > params.startFrame);

    if (params.trexEnabled)
    {
        // For each case, bootstrap the capture
        switch (params.startFrame)
        {
            case 200:
                trex_200_210::SetBinaryDataDir(ANGLE_TREX_REPLAY_DATA_DIR_200_210);
                trex_200_210::SetupContext1Replay();
                break;
            case 800:
                trex_800_810::SetBinaryDataDir(ANGLE_TREX_REPLAY_DATA_DIR_800_810);
                trex_800_810::SetupContext1Replay();
                break;
            case 900:
                trex_900_910::SetBinaryDataDir(ANGLE_TREX_REPLAY_DATA_DIR_900_910);
                trex_900_910::SetupContext1Replay();
                break;
            case 1300:
                trex_1300_1310::SetBinaryDataDir(ANGLE_TREX_REPLAY_DATA_DIR_1300_1310);
                trex_1300_1310::SetupContext1Replay();
                break;
            default:
                assert(0);
                break;
        }
    }

    getWindow()->setVisible(true);
}

void CaptureReplayPerfBenchmark::destroyBenchmark() {}

void CaptureReplayPerfBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    // Compute the current frame, looping from startFrame to endFrame.
    uint32_t frame = params.startFrame + (mCurrentFrame % (params.endFrame - params.startFrame));

    startGpuTimer();
    if (params.trexEnabled)
    {
        switch (params.startFrame)
        {
            case 200:
                trex_200_210::ReplayContext1Frame(frame);
                break;
            case 800:
                trex_800_810::ReplayContext1Frame(frame);
                break;
            case 900:
                trex_900_910::ReplayContext1Frame(frame);
                break;
            case 1300:
                trex_1300_1310::ReplayContext1Frame(frame);
                break;
            default:
                assert(0);
                break;
        }
    }
    stopGpuTimer();

    mCurrentFrame++;
}

CaptureReplayPerfParams TRexReplayPerfOpenGLOrGLESParams_200_210()
{
    CaptureReplayPerfParams params;
    params.eglParameters = OPENGL_OR_GLES();
    params.trexEnabled   = true;

    params.startFrame = 200;
    params.endFrame   = 210;

    return params;
}

CaptureReplayPerfParams TRexReplayPerfOpenGLOrGLESParams_800_810()
{
    CaptureReplayPerfParams params;
    params.eglParameters = OPENGL_OR_GLES();
    params.trexEnabled   = true;

    params.startFrame = 800;
    params.endFrame   = 810;

    return params;
}

CaptureReplayPerfParams TRexReplayPerfOpenGLOrGLESParams_900_910()
{
    CaptureReplayPerfParams params;
    params.eglParameters = OPENGL_OR_GLES();
    params.trexEnabled   = true;

    params.startFrame = 900;
    params.endFrame   = 910;

    return params;
}

CaptureReplayPerfParams TRexReplayPerfOpenGLOrGLESParams_1300_1310()
{
    CaptureReplayPerfParams params;
    params.eglParameters = OPENGL_OR_GLES();
    params.trexEnabled   = true;

    params.startFrame = 1300;
    params.endFrame   = 1310;

    return params;
}

TEST_P(CaptureReplayPerfBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(CaptureReplayPerfBenchmark,
                       TRexReplayPerfOpenGLOrGLESParams_200_210(),
                       TRexReplayPerfOpenGLOrGLESParams_800_810(),
                       TRexReplayPerfOpenGLOrGLESParams_900_910(),
                       TRexReplayPerfOpenGLOrGLESParams_1300_1310());

}  // anonymous namespace
