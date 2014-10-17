//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "Rectangle.h"

Rect::Rect()
{
    x = 0;
    y = 0;
    w = 0;
    h = 0;
}

Rect::Rect(float x_, float y_, float w_, float h_)
{
    x = x_;
    y = y_;
    w = w_;
    h = h_;
}

Rect::Rect(const Vector2 &pos_, const Vector2 &size_)
{
    x = pos_.x;
    y = pos_.y;
    w = size_.x;
    h = size_.y;
}
