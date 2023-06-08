#include "CapturedTest_MultiFrame_ES3_Vulkan.h"
#include "trace_fixture.h"
#include "angle_trace_gl.h"

// Private Functions

// Public Functions

void ReplayFrame(uint32_t frameIndex)
{
    switch (frameIndex)
    {
        case 1:
            ReplayFrame1();
            break;
        case 2:
            ReplayFrame2();
            break;
        case 3:
            ReplayFrame3();
            break;
        case 4:
            ReplayFrame4();
            break;
        default:
            break;
    }
}

void ResetReplay(void)
{
    // Reset context is empty because context is destroyed before end frame is reached
}

