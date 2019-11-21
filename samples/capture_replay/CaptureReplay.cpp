//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CaptureReplay: Template for replaying a frame capture with ANGLE.

#include "SampleApplication.h"

#define xstr(a) str(a)
#define str(a) #a

// This will expand to "angle_capture_context<#>.h"
#include xstr(ANGLE_CAPTURE_REPLAY_CONTEXT_HEADER)

class CaptureReplaySample : public SampleApplication
{
  public:
    CaptureReplaySample(int argc, char **argv)
        : SampleApplication("CaptureReplaySample", argc, argv, 3, 0)
    {}

    bool initialize() override
    {
        // Set CWD to executable directory.
        std::string exeDir = angle::GetExecutableDirectory();
        if (!angle::SetCWD(exeDir.c_str()))
            return false;
        SetBinaryDataDir(ANGLE_CAPTURE_REPLAY_SAMPLE_DATA_DIR);

        // This will expand to SetupContext<#>Replay()
        ANGLE_CAPTURE_REPLAY_SETUP_CONTEXT_FUNCTION();

        eglSwapInterval(getDisplay(), 1);
        return true;
    }

    void destroy() override {}

    void draw() override
    {
        // Compute the current frame, looping from kReplayFrameStart to kReplayFrameEnd.
        uint32_t frame =
            kReplayFrameStart + (mCurrentFrame % (kReplayFrameEnd - kReplayFrameStart));

        // This will expand to ReplayContext<#>Frame(frame);
        ANGLE_CAPTURE_REPLAY_CONTEXT_FRAME_FUNCTION(frame);
        mCurrentFrame++;
    }

  private:
    uint32_t mCurrentFrame = 0;
};

int main(int argc, char **argv)
{
    CaptureReplaySample app(argc, argv);
    return app.run();
}
