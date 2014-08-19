//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef SAMPLE_UTIL_SIMPLE_BENCHMARK_H
#define SAMPLE_UTIL_SIMPLE_BENCHMARK_H

#include "SampleApplication.h"

class SimpleBenchmark : public SampleApplication
{
public:
    SimpleBenchmark(const std::string& name, size_t width, size_t height,
        EGLint glesMajorVersion = 2, EGLint requestedRenderer = EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE);
    virtual ~SimpleBenchmark() { };

    virtual int drawIterations() const { return 10; }
    virtual double runTimeSeconds() const { return 10.0; }

    virtual bool initializeBenchmark() { return true; }
    virtual void destroyBenchmark() { }

    virtual void stepBenchmark(float dt, double totalTime) { }

    virtual void beginDrawBenchmark() { }
    virtual void drawBenchmark() = 0;
    virtual void endDrawBenchmark() { }

private:
    bool initialize();
    void destroy();

    void step(float dt, double totalTime);
    void draw();

    int mNumFrames;
};

template <class Benchmark>
int RunMultiRendererBenchmark()
{
    EGLint renderers[] = {
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE,
        0
    };
    
    int result;

    for (int i = 0; renderers[i] != 0; ++i) {
        Benchmark benchmark11(renderers[i]);
        result = benchmark11.run();
        if (result != 0) { return result; }
    }

    return 0;
}

#endif // SAMPLE_UTIL_SIMPLE_BENCHMARK_H
