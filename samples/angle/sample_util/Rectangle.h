//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef SAMPLE_UTIL_RECTANGLE_H
#define SAMPLE_UTIL_RECTANGLE_H

#include "Vector.h"

struct Rect
{
    union
    {
        struct
        {
            float x, y, w, h;
        };
        struct
        {
            Vector2 position;
            Vector2 size;
        };
        float data[4];
    };

    Rect();
    Rect(float x, float y, float w, float h);
    Rect(const Vector2 &pos, const Vector2 &size);
};


#endif // SAMPLE_UTIL_RECTANGLE_H
