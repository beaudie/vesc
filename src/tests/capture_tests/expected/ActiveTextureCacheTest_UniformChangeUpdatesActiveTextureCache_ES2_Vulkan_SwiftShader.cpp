#include "ActiveTextureCacheTest_UniformChangeUpdatesActiveTextureCache_ES2_Vulkan_SwiftShader.h"
#include "trace_fixture.h"
#include "angle_trace_gl.h"

EGLDisplay gEGLDisplay;

// Private Functions

void InitReplay(void)
{
    // binaryDataFileName = ActiveTextureCacheTest_UniformChangeUpdatesActiveTextureCache_ES2_Vulkan_SwiftShader.angledata.gz
    // maxClientArraySize = 0
    // maxClientArraySize = 0
    // readBufferSize = 4
    // resourceIDBufferSize = 1
    // contextID = 1
    // maxBuffer = 0
    // maxContext = 1
    // maxFenceNV = 0
    // maxFramebuffer = 0
    // maxImage = 0
    // maxMemoryObject = 0
    // maxProgramPipeline = 0
    // maxQuery = 0
    // maxRenderbuffer = 0
    // maxSampler = 0
    // maxSemaphore = 0
    // maxShaderProgram = 3
    // maxSurface = 1
    // maxSync = 0
    // maxTexture = 2
    // maxTransformFeedback = 0
    // maxVertexArray = 0
    // maxegl_Sync = 0
    InitializeReplay4("ActiveTextureCacheTest_UniformChangeUpdatesActiveTextureCache_ES2_Vulkan_SwiftShader.angledata.gz", 0, 4, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 0, 2, 0, 0, 0);
}

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
        default:
            break;
    }
}

void ResetReplay(void)
{
    // Reset context is empty because context is destroyed before end frame is reached
}

