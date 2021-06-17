//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "image_util/loadtextureborder.h"

namespace angle
{

void LoadA8ToRGBA8(angle::ColorF &mBorderColor) {}

void LoadA8ToR8(angle::ColorF &mBorderColor)
{
    mBorderColor.red = mBorderColor.alpha;
}

void LoadA8ToA8(angle::ColorF &mBorderColor) {}

void LoadLA8ToR8G8(angle::ColorF &mBorderColor)
{
    mBorderColor.green = mBorderColor.alpha;
}

void LoadLA8ToRGBA8(angle::ColorF &mBorderColor) {}
}  // namespace angle
