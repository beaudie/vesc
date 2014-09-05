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
           << " - " << count << " sprites - size " << size
           << " - " << numVaryings << " varyings";

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
    std::stringstream vstrstr;

    vstrstr << "attribute vec2 vPosition;\n"
               "uniform float uPointSize;\n";

    for (unsigned int varCount = 0; varCount < mParams.numVaryings; varCount++)
    {
        vstrstr << "varying vec4 v" << varCount << ";\n";
    }

    vstrstr << "void main()\n"
               "{\n";

    for (unsigned int varCount = 0; varCount < mParams.numVaryings; varCount++)
    {
        vstrstr << "    v" << varCount << " = vec4(1.0);\n";
    }

    vstrstr << "    gl_Position = vec4(vPosition, 0, 1.0);\n"
               "    gl_PointSize = uPointSize;\n"
               "}";

    std::stringstream fstrstr;

    fstrstr << "precision mediump float;\n";

    for (unsigned int varCount = 0; varCount < mParams.numVaryings; varCount++)
    {
        fstrstr << "varying vec4 v" << varCount << ";\n";
    }

    fstrstr << "void main()\n"
               "{\n"
               "    vec4 colorOut = vec4(1.0, 0.0, 0.0, 1.0);\n";

    for (unsigned int varCount = 0; varCount < mParams.numVaryings; varCount++)
    {
        fstrstr << "colorOut.r += v" << varCount << ".r;\n";
    }

    fstrstr << "    gl_FragColor = colorOut;\n"
               "}\n";

    mProgram = CompileProgram(vstrstr.str(), fstrstr.str());
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
    glBufferData(GL_ARRAY_BUFFER, mParams.count * sizeof(float) * 2, vertexPositions, GL_STATIC_DRAW);

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

    glDisable(GL_DEPTH_TEST);

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
