//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "SimpleBenchmark.h"
#include <iostream>

using namespace std;

SimpleBenchmark::SimpleBenchmark(const std::string& name, size_t width, size_t height, EGLint glesMajorVersion, EGLint requestedRenderer)
    : SampleApplication(name, width, height, glesMajorVersion, requestedRenderer),
      mNumFrames(0)
{
    cout << "========= " << name << " - ";
    switch (requestedRenderer) {
      case EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE: cout << "D3D11"; break;
      case EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE: cout << "D3D9"; break;
      default: cout << "UNKNOWN RENDERER (" << requestedRenderer << ")"; break;
    }
    cout << " =========" << endl;
}

bool SimpleBenchmark::initialize()
{
    return initializeBenchmark();
} 

void SimpleBenchmark::destroy()
{
    double totalTime = mTimer->getElapsedTime();
    cout << " - total time: " << totalTime << " sec" << endl;
    cout << " - frames: " << mNumFrames << endl;
    cout << " - average frame time: " << 1000.0 * totalTime / mNumFrames << " msec" << endl;
    cout << "=========" << endl << endl;

    destroyBenchmark();
}

void SimpleBenchmark::step(float dt, double totalTime)
{
    stepBenchmark(dt, totalTime);
}

void SimpleBenchmark::draw()
{
    if (mTimer->getElapsedTime() > runTimeSeconds()) {
        exit();
        return;
    }

    ++mNumFrames;

    beginDrawBenchmark();

    for (int i = 0; i < drawIterations(); ++i)
    {
        drawBenchmark();
    }

    endDrawBenchmark();
}
