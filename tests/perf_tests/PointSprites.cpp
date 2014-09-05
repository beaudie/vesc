//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "PointSprites.h"

#include <cassert>
#include <sstream>

#include "shader_utils.h"

std::string PointSpritesParams::name() const
{
    std::stringstream strstr;

    strstr << "BufferSubData - " << BenchmarkParams::name()
           << " - " << count << " sprites - size " << size;

    return strstr.str();
}

PointSpritesBenchmark::PointSpritesBenchmark(const PointSpritesParams &params)
    : SimpleBenchmark(params.name(), 1280, 720, 2, params.requestedRenderer),
      mParams(params)
{
    mDrawIterations = mParams.iterations;
    assert(mParams.iterations > 0);
}

bool PointSpritesBenchmark::initializeBenchmark()
{
    const std::string vs = SHADER_SOURCE
    (
        attribute vec2 vPosition;
        uniform float uPointSize;
        void main()
        {
            gl_Position = vPosition;
            gl_PointSize = uPointSize;
        }
    );

    const std::string fs = SHADER_SOURCE
    (
        precision mediump float;
        void main()
        {
            gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    );

    mProgram = CompileProgram(vs, fs);
    if (!mProgram)
    {
        return false;
    }

    // Use the program object
    glUseProgram(mProgram);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    float *vertexPositions = new float[mParams.count * 2];
    for (unsigned int pointIndex = 0; pointIndex < mParams.count * 2; ++pointIndex)
    {
        vertexPositions[pointIndex] = static_cast<float>(rand() % 10000) / 5000.0f - 1.0f;
    }

    glGenBuffers(1, &mBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, mParams.count * sizeof(float) * 2, 0, GL_STATIC_DRAW);

    int positionLocation = glGetAttribLocation(mProgram, "vPosition");
    if (positionLocation == -1)
    {
        return false;
    }

    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(positionLocation);

    // Set the viewport
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    int pointSizeLocation = glGetUniformLocation(mProgram, "uPointSize");
    if (pointSizeLocation == -1)
    {
        return false;
    }

    glUniform1f(pointSizeLocation, mParams.size);

    GLenum glErr = glGetError();
    if (glErr != GL_NO_ERROR)
    {
        return false;
    }

    return true;
}

void PointSpritesBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram);
    glDeleteBuffers(1, &mBuffer);
}

void PointSpritesBenchmark::beginDrawBenchmark()
{
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);
}

void PointSpritesBenchmark::drawBenchmark()
{
    for (unsigned int it = 0; it < mParams.iterations; it++)
    {
        glDrawArrays(GL_POINTS, 0, mParams.count);
    }
}
