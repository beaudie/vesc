//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Based on CubeMapActivity.java from The Android Open Source Project ApiDemos
// https://android.googlesource.com/platform/development/+/refs/heads/master/samples/ApiDemos/src/com/example/android/apis/graphics/CubeMapActivity.java

#pragma once

#define RES 60

#include <cmath>

const float pi = 3.1415926535897f;

static void generateTorus(GLuint *vertexBuffer, GLuint *indexBuffer, GLsizei *indexCount)
{
    std::vector<GLushort> indices;
    for (GLushort y = 0; y < RES; y++)
    {
        for (GLushort x = 0; x < RES; x++)
        {
            GLushort a = y * (RES + 1) + x;
            GLushort b = y * (RES + 1) + x + 1;
            GLushort c = (y + 1) * (RES + 1) + x;
            GLushort d = (y + 1) * (RES + 1) + x + 1;

            indices.push_back(a);
            indices.push_back(c);
            indices.push_back(b);

            indices.push_back(b);
            indices.push_back(c);
            indices.push_back(d);
        }
    }
    *indexCount = static_cast<GLsizei>(indices.size());

    std::vector<GLfloat> vertices;
    for (uint32_t j = 0; j <= RES; j++)
    {
        float angleV = pi * 2 * j / RES;
        float cosV   = cosf(angleV);
        float sinV   = sinf(angleV);
        for (uint32_t i = 0; i <= RES; i++)
        {
            float angleU = pi * 2 * i / RES;
            float cosU   = cosf(angleU);
            float sinU   = sinf(angleU);
            float d      = 3.0f + 0.75f * cosU;

            float x = d * cosV;
            float y = d * (-sinV);
            float z = 0.75f * sinU;

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            float nx = cosV * cosU;
            float ny = -sinV * cosU;
            float nz = sinU;

            float length = sqrtf(nx * nx + ny * ny + nz * nz);
            nx /= length;
            ny /= length;
            nz /= length;

            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
        }
    }

    glGenBuffers(1, vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, *vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(),
                 GL_STATIC_DRAW);

    glGenBuffers(1, indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(),
                 GL_STATIC_DRAW);
}
