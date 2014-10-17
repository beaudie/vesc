//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef SAMPLE_UTIL_GEOMETRY_UTILS_H
#define SAMPLE_UTIL_GEOMETRY_UTILS_H

#include "Rectangle.h"
#include "Vector.h"

#include <vector>

#include <GLES2/gl2.h>

struct SphereGeometry
{
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<GLushort> indices;
};

void CreateSphereGeometry(size_t sliceCount, float radius, SphereGeometry *result);

struct CubeGeometry
{
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> texcoords;
    std::vector<GLushort> indices;
};

void GenerateCubeGeometry(float radius, CubeGeometry *result);

class RectanglePacker
{
public:
    RectanglePacker(const Vector2 &size);
    RectanglePacker(const RectanglePacker &other);

    bool insert(const Vector2 &requiredSize, Vector2 *outPosition);

  private:
    Vector2 mSize;

    typedef std::vector<Vector2> heightVector;
    heightVector mHeights;
};

#endif // SAMPLE_UTIL_GEOMETRY_UTILS_H
