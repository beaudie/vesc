//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BindingPerf:
//   Performance test for binding objects
//

#include "ANGLEPerfTest.h"

#include <iostream>
#include <random>
#include <sstream>

#include "shader_utils.h"

namespace angle
{

struct BindingsParams final : public RenderTestParams
{
    BindingsParams()
    {
        // Common default params
        majorVersion = 2;
        minorVersion = 0;
        windowWidth  = 720;
        windowHeight = 720;
        iterations   = 256;

        bindingUpdates = 20;
        numObjects     = 100;
        preallocated   = false;
    }

    std::string suffix() const override;
    size_t bindingUpdates;
    size_t numObjects;
    bool preallocated;

    // static parameters
    size_t iterations;
};

std::ostream &operator<<(std::ostream &os, const BindingsParams &params)
{
    os << params.suffix().substr(1);
    return os;
}

std::string BindingsParams::suffix() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::suffix();
    strstr << "_" << bindingUpdates << "_bindings";
    if (preallocated)
    {
        strstr << "_preallocated";
    }

    return strstr.str();
}

class BindingsBenchmark : public ANGLERenderTest,
                          public ::testing::WithParamInterface<BindingsParams>
{
  public:
    BindingsBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  private:
    std::vector<GLuint> mBuffers;
    std::vector<GLenum> mBindingPoints;
};

BindingsBenchmark::BindingsBenchmark() : ANGLERenderTest("Bindings", GetParam())
{
}

void BindingsBenchmark::initializeBenchmark()
{
    const auto &params = GetParam();
    ASSERT_GT(params.iterations, 0u);

    mBuffers.resize(params.numObjects, 0);
    if (params.preallocated)
    {
        glGenBuffers(static_cast<GLsizei>(mBuffers.size()), mBuffers.data());
        for (size_t bufferIdx = 0; bufferIdx < mBuffers.size(); bufferIdx++)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mBuffers[bufferIdx]);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    mBindingPoints.push_back(GL_ARRAY_BUFFER);
    mBindingPoints.push_back(GL_ELEMENT_ARRAY_BUFFER);
    if (params.majorVersion >= 3)
    {
        mBindingPoints.push_back(GL_PIXEL_PACK_BUFFER);
        mBindingPoints.push_back(GL_PIXEL_UNPACK_BUFFER);
        mBindingPoints.push_back(GL_COPY_READ_BUFFER);
        mBindingPoints.push_back(GL_COPY_WRITE_BUFFER);
        mBindingPoints.push_back(GL_TRANSFORM_FEEDBACK_BUFFER);
        mBindingPoints.push_back(GL_UNIFORM_BUFFER);
    }
    if (params.majorVersion > 3 || (params.majorVersion == 3 && params.minorVersion >= 1))
    {
        mBindingPoints.push_back(GL_ATOMIC_COUNTER_BUFFER);
        mBindingPoints.push_back(GL_SHADER_STORAGE_BUFFER);
        mBindingPoints.push_back(GL_DRAW_INDIRECT_BUFFER);
        mBindingPoints.push_back(GL_DISPATCH_INDIRECT_BUFFER);
    }
}

void BindingsBenchmark::destroyBenchmark()
{
    const auto &params = GetParam();
    if (params.preallocated)
    {
        glDeleteBuffers(static_cast<GLsizei>(mBuffers.size()), mBuffers.data());
    }
}

void BindingsBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    for (size_t it = 0; it < params.iterations; ++it)
    {
        // Generate a buffer (if needed) and bind it to a "random" binding point
        if (!params.preallocated)
        {
            glGenBuffers(static_cast<GLsizei>(mBuffers.size()), mBuffers.data());
        }

        for (size_t bufferIdx = 0; bufferIdx < mBuffers.size(); bufferIdx++)
        {
            GLenum binding = mBindingPoints[bufferIdx % mBindingPoints.size()];
            glBindBuffer(binding, mBuffers[bufferIdx]);
        }

        // Delete all the buffers
        if (!params.preallocated)
        {
            glDeleteBuffers(static_cast<GLsizei>(mBuffers.size()), mBuffers.data());
        }
    }

    ASSERT_GL_NO_ERROR();
}

BindingsParams D3D11Params(bool preallocated)
{
    BindingsParams params;
    params.eglParameters = egl_platform::D3D11_NULL();
    params.preallocated  = preallocated;
    return params;
}

BindingsParams D3D9Params(bool preallocated)
{
    BindingsParams params;
    params.eglParameters = egl_platform::D3D9_NULL();
    params.preallocated  = preallocated;
    return params;
}

BindingsParams OpenGLParams(bool preallocated)
{
    BindingsParams params;
    params.eglParameters = egl_platform::OPENGL_NULL();
    params.preallocated  = preallocated;
    return params;
}

TEST_P(BindingsBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(BindingsBenchmark,
                       D3D11Params(false),
                       D3D11Params(true),
                       D3D9Params(false),
                       D3D9Params(true),
                       OpenGLParams(false),
                       OpenGLParams(true));

}  // namespace angle
