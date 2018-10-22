//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ANGLEPerfTests:
//   Base class for google test performance tests
//

#include "ANGLEPerfTest.h"
#include "third_party/perf/perf_test.h"
#include "third_party/trace_event/trace_event.h"

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>

#include <json/json.h>

namespace
{
constexpr size_t kInitialTraceEventBufferSize = 50000;

void EmptyPlatformMethod(angle::PlatformMethods *, const char *)
{
}

void OverrideWorkaroundsD3D(angle::PlatformMethods *platform, angle::WorkaroundsD3D *workaroundsD3D)
{
    auto *angleRenderTest = static_cast<ANGLERenderTest *>(platform->context);
    angleRenderTest->overrideWorkaroundsD3D(workaroundsD3D);
}

angle::TraceEventHandle AddTraceEvent(angle::PlatformMethods *platform,
                                      char phase,
                                      const unsigned char *categoryEnabledFlag,
                                      const char *name,
                                      unsigned long long id,
                                      double timestamp,
                                      int numArgs,
                                      const char **argNames,
                                      const unsigned char *argTypes,
                                      const unsigned long long *argValues,
                                      unsigned char flags)
{
    ANGLERenderTest *renderTest     = static_cast<ANGLERenderTest *>(platform->context);
    std::vector<TraceEvent> &buffer = renderTest->getTraceEventBuffer();
    if (phase != 'c')
    {
        buffer.emplace_back(phase, name, timestamp);
    }
    else
    {
        // For clock sync events, the issue_ts argument will be the gpuTimestamp
        double gpuTimestamp = timestamp;
        for (int i = 0; i < numArgs; ++i)
        {
            if (strcmp(argNames[i], "issue_ts") == 0)
            {
                gl::TraceEvent::TraceValueUnion value;
                value.m_uint = argValues[i];
                gpuTimestamp = value.m_double;
                break;
            }
        }
        buffer.emplace_back(phase, name, timestamp, gpuTimestamp);
    }
    return buffer.size();
}

const unsigned char *GetTraceCategoryEnabledFlag(angle::PlatformMethods *platform,
                                                 const char *categoryName)
{
    constexpr static unsigned char kNonZero = 1;
    return &kNonZero;
}

void UpdateTraceEventDuration(angle::PlatformMethods *platform,
                              const unsigned char *categoryEnabledFlag,
                              const char *name,
                              angle::TraceEventHandle eventHandle)
{
    // Not implemented.
}

double MonotonicallyIncreasingTime(angle::PlatformMethods *platform)
{
    ANGLERenderTest *renderTest = static_cast<ANGLERenderTest *>(platform->context);
    // Move the time origin to the first call to this function, to avoid generating unnecessarily
    // large timestamps.
    static double origin = renderTest->getTimer()->getAbsoluteTime();
    return renderTest->getTimer()->getAbsoluteTime() - origin;
}

void DumpTraceEventsToJSONFile(const std::vector<TraceEvent> &traceEvents,
                               const char *outputFileName)
{
    Json::Value eventsValue(Json::arrayValue);

    double lastGpuSyncTime   = 0;
    double lastGpuSyncDiff   = 0;
    double gpuSyncDriftSlope = 0;

    for (size_t i = 0; i < traceEvents.size(); ++i)
    {
        const TraceEvent &traceEvent = traceEvents[i];

        const char *name = traceEvent.name;
        double timestamp = traceEvent.timestamp;
        char phase       = traceEvent.phase;

        if (traceEvent.phase == 'c')
        {
            // Special handling of clock sync event.  Note that this is not per-spec, and is
            // currently a hack to synchronize CPU and GPU times, taking clock drift into account.

            // Note: syncs are currently generated in order, and out-of-order sync events are not
            // supported.
            ASSERT(lastGpuSyncTime <= traceEvent.gpuTimestamp);

            lastGpuSyncTime = traceEvent.gpuTimestamp;
            lastGpuSyncDiff = traceEvent.timestamp - traceEvent.gpuTimestamp;

            gpuSyncDriftSlope = 0;

            // Search for the next clock sync event
            for (size_t j = i + 1; j < traceEvents.size(); ++j)
            {
                if (traceEvents[j].phase == 'c')
                {
                    double nextGpuSyncTime = traceEvents[j].gpuTimestamp;
                    double nextGpuSyncDiff = traceEvents[j].timestamp - traceEvents[j].gpuTimestamp;

                    gpuSyncDriftSlope =
                        (nextGpuSyncDiff - lastGpuSyncDiff) / (nextGpuSyncTime - lastGpuSyncTime);

                    break;
                }
            }

            // Don't write the clock sync event to file.  Instead, create an instant event for it.
            name  = "GPU Clock Sync";
            phase = TRACE_EVENT_PHASE_INSTANT;
        }

        Json::Value value(Json::objectValue);

        std::stringstream phaseName;
        phaseName << phase;

        // Note: as a way to distinguish CPU and GPU events, GPU timestamps are negated.
        bool isGpuTimestamp = timestamp < 0;
        if (isGpuTimestamp)
        {
            timestamp = -timestamp;

            // Note: syncs are currently generated in order and during GPU idle time, and
            // out-of-order sync events and gpu events are not supported.
            ASSERT(timestamp >= lastGpuSyncTime);

            // Account for clock drift.
            timestamp += lastGpuSyncDiff + gpuSyncDriftSlope * (timestamp - lastGpuSyncTime);
        }

        unsigned long long microseconds =
            static_cast<unsigned long long>(timestamp * 1000.0 * 1000.0);

        value["name"] = name;
        value["cat"]  = isGpuTimestamp ? "gpu.angle.gpu" : "gpu.angle";
        value["ph"]   = phaseName.str();
        value["ts"]   = microseconds;
        value["pid"]  = "ANGLE";
        value["tid"]  = isGpuTimestamp ? "GPU" : "CPU";

        eventsValue.append(value);
    }

    Json::Value root(Json::objectValue);
    root["traceEvents"] = eventsValue;

    std::ofstream outFile;
    outFile.open(outputFileName);

    Json::StyledWriter styledWrite;
    outFile << styledWrite.write(root);

    outFile.close();
}
}  // anonymous namespace

bool g_OnlyOneRunFrame = false;
bool gEnableTrace      = false;
const char *gTraceFile = "ANGLETrace.json";

ANGLEPerfTest::ANGLEPerfTest(const std::string &name, const std::string &suffix)
    : mName(name),
      mSuffix(suffix),
      mTimer(CreateTimer()),
      mRunTimeSeconds(5.0),
      mSkipTest(false),
      mNumStepsPerformed(0),
      mRunning(true)
{
}

ANGLEPerfTest::~ANGLEPerfTest()
{
    SafeDelete(mTimer);
}

void ANGLEPerfTest::run()
{
    if (mSkipTest)
    {
        return;
    }

    mTimer->start();
    while (mRunning)
    {
        step();
        if (mRunning)
        {
            ++mNumStepsPerformed;
        }
        if (mTimer->getElapsedTime() > mRunTimeSeconds || g_OnlyOneRunFrame)
        {
            mRunning = false;
        }
    }
    finishTest();
    mTimer->stop();
}

void ANGLEPerfTest::printResult(const std::string &trace, double value, const std::string &units, bool important) const
{
    perf_test::PrintResult(mName, mSuffix, trace, value, units, important);
}

void ANGLEPerfTest::printResult(const std::string &trace, size_t value, const std::string &units, bool important) const
{
    perf_test::PrintResult(mName, mSuffix, trace, value, units, important);
}

void ANGLEPerfTest::SetUp()
{
}

void ANGLEPerfTest::TearDown()
{
    if (mSkipTest)
    {
        return;
    }
    double relativeScore = static_cast<double>(mNumStepsPerformed) / mTimer->getElapsedTime();
    printResult("score", static_cast<size_t>(std::round(relativeScore)), "score", true);
}

double ANGLEPerfTest::normalizedTime(size_t value) const
{
    return static_cast<double>(value) / static_cast<double>(mNumStepsPerformed);
}

std::string RenderTestParams::suffix() const
{
    switch (getRenderer())
    {
        case EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE:
            return "_d3d11";
        case EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE:
            return "_d3d9";
        case EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE:
            return "_gl";
        case EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE:
            return "_gles";
        case EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE:
            return "_default";
        case EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE:
            return "_vulkan";
        default:
            assert(0);
            return "_unk";
    }
}

ANGLERenderTest::ANGLERenderTest(const std::string &name, const RenderTestParams &testParams)
    : ANGLEPerfTest(name, testParams.suffix()),
      mTestParams(testParams),
      mEGLWindow(createEGLWindow(testParams)),
      mOSWindow(nullptr)
{
    // Try to ensure we don't trigger allocation during execution.
    mTraceEventBuffer.reserve(kInitialTraceEventBufferSize);
}

ANGLERenderTest::ANGLERenderTest(const std::string &name,
                                 const RenderTestParams &testParams,
                                 const std::vector<std::string> &extensionPrerequisites)
    : ANGLEPerfTest(name, testParams.suffix()),
      mTestParams(testParams),
      mEGLWindow(createEGLWindow(testParams)),
      mOSWindow(nullptr),
      mExtensionPrerequisites(extensionPrerequisites)
{
}

ANGLERenderTest::~ANGLERenderTest()
{
    SafeDelete(mOSWindow);
    SafeDelete(mEGLWindow);
}

void ANGLERenderTest::SetUp()
{
    ANGLEPerfTest::SetUp();

    mOSWindow = CreateOSWindow();
    ASSERT(mEGLWindow != nullptr);
    mEGLWindow->setSwapInterval(0);

    mPlatformMethods.overrideWorkaroundsD3D = OverrideWorkaroundsD3D;
    mPlatformMethods.logError               = EmptyPlatformMethod;
    mPlatformMethods.logWarning             = EmptyPlatformMethod;
    mPlatformMethods.logInfo                = EmptyPlatformMethod;
    mPlatformMethods.addTraceEvent               = AddTraceEvent;
    mPlatformMethods.getTraceCategoryEnabledFlag = GetTraceCategoryEnabledFlag;
    mPlatformMethods.updateTraceEventDuration    = UpdateTraceEventDuration;
    mPlatformMethods.monotonicallyIncreasingTime = MonotonicallyIncreasingTime;
    mPlatformMethods.context                = this;
    mEGLWindow->setPlatformMethods(&mPlatformMethods);

    if (!mOSWindow->initialize(mName, mTestParams.windowWidth, mTestParams.windowHeight))
    {
        FAIL() << "Failed initializing OSWindow";
        return;
    }

    if (!mEGLWindow->initializeGL(mOSWindow))
    {
        FAIL() << "Failed initializing EGLWindow";
        return;
    }

    if (!areExtensionPrerequisitesFulfilled())
    {
        mSkipTest = true;
    }

    if (mSkipTest)
    {
        std::cout << "Test skipped due to missing extension." << std::endl;
        return;
    }

    initializeBenchmark();
}

void ANGLERenderTest::TearDown()
{
    destroyBenchmark();

    mEGLWindow->destroyGL();
    mOSWindow->destroy();

    // Dump trace events to json file.
    if (gEnableTrace)
    {
        DumpTraceEventsToJSONFile(mTraceEventBuffer, gTraceFile);
    }

    ANGLEPerfTest::TearDown();
}

void ANGLERenderTest::step()
{
    // Clear events that the application did not process from this frame
    Event event;
    bool closed = false;
    while (popEvent(&event))
    {
        // If the application did not catch a close event, close now
        if (event.Type == Event::EVENT_CLOSED)
        {
            closed = true;
        }
    }

    if (closed)
    {
        abortTest();
    }
    else
    {
        drawBenchmark();
        // Swap is needed so that the GPU driver will occasionally flush its internal command queue
        // to the GPU. The null device benchmarks are only testing CPU overhead, so they don't need
        // to swap.
        if (mTestParams.eglParameters.deviceType != EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE)
        {
            mEGLWindow->swap();
        }
        mOSWindow->messageLoop();
    }
}

void ANGLERenderTest::finishTest()
{
    if (mTestParams.eglParameters.deviceType != EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE)
    {
        glFinish();
    }
}

bool ANGLERenderTest::popEvent(Event *event)
{
    return mOSWindow->popEvent(event);
}

OSWindow *ANGLERenderTest::getWindow()
{
    return mOSWindow;
}

bool ANGLERenderTest::areExtensionPrerequisitesFulfilled() const
{
    for (const auto &extension : mExtensionPrerequisites)
    {
        if (!CheckExtensionExists(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)),
                                  extension))
        {
            return false;
        }
    }
    return true;
}

void ANGLERenderTest::setWebGLCompatibilityEnabled(bool webglCompatibility)
{
    mEGLWindow->setWebGLCompatibilityEnabled(webglCompatibility);
}

void ANGLERenderTest::setRobustResourceInit(bool enabled)
{
    mEGLWindow->setRobustResourceInit(enabled);
}

std::vector<TraceEvent> &ANGLERenderTest::getTraceEventBuffer()
{
    return mTraceEventBuffer;
}

// static
EGLWindow *ANGLERenderTest::createEGLWindow(const RenderTestParams &testParams)
{
    return new EGLWindow(testParams.majorVersion, testParams.minorVersion,
                         testParams.eglParameters);
}
