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
    mBorderColor.red   = mBorderColor.alpha;
    mBorderColor.alpha = 0;
}
void LoadA8ToA8(angle::ColorF &mBorderColor) {}
void LoadLA8ToR8G8(angle::ColorF &mBorderColor)
{
    mBorderColor.green = mBorderColor.alpha;
    mBorderColor.alpha = 0;
}
void LoadLA8ToRGBA8(angle::ColorF &mBorderColor) {}
void LoadToNative(angle::ColorF &mBorderColor) {}
void LoadA32ToR32(angle::ColorF &mBorderColor) {}
void LoadA16FToRGBA16F(angle::ColorF &mBorderColor) {}
void LoadA16ToR16(angle::ColorF &mBorderColor) {}
void LoadA32FToRGBA32F(angle::ColorF &mBorderColor) {}
void LoadRGB5A1ToRGBA8(angle::ColorF &mBorderColor) {}
void LoadRGBA8ToBGRA4(angle::ColorF &mBorderColor) {}
void LoadRGBA8ToBGR5A1(angle::ColorF &mBorderColor) {}
void LoadRGB5A1ToA1RGB5(angle::ColorF &mBorderColor) {}
void LoadRGB10A2ToRGB10X2(angle::ColorF &mBorderColor) {}
void LoadLA32FToRGBA32F(angle::ColorF &mBorderColor) {}
void LoadLA16FToRGBA16F(angle::ColorF &mBorderColor) {}
void LoadLA16ToR16G16(angle::ColorF &mBorderColor) {}
void LoadL8ToR8(angle::ColorF &mBorderColor) {}
void LoadL8ToRGBA8(angle::ColorF &mBorderColor) {}
void LoadL32FToR32F(angle::ColorF &mBorderColor) {}
void LoadR32ToR24G8(angle::ColorF &mBorderColor) {}
void LoadD32ToD32F(angle::ColorF &mBorderColor) {}
void LoadD24S8ToD32FS8X24(angle::ColorF &mBorderColor) {}
void LoadD32FS8X24ToD24S8(angle::ColorF &mBorderColor) {}
void LoadRGBA4ToRGBA8(angle::ColorF &mBorderColor) {}
void LoadRGB565ToBGR565(angle::ColorF &mBorderColor) {}
void LoadD32FS8X24ToD32FS8X24(angle::ColorF &mBorderColor) {}
void LoadD32FToD32F(angle::ColorF &mBorderColor) {}
void LoadL16FToRGBA16F(angle::ColorF &mBorderColor) {}
void LoadL32FToRGBA32F(angle::ColorF &mBorderColor) {}
void LoadL16FToR16(angle::ColorF &mBorderColor) {}
void LoadRGB8ToRGBA8(angle::ColorF &mBorderColor) {}
void LoadLA32ToR32G32(angle::ColorF &mBorderColor) {}
void LoadCompressedToNative(angle::ColorF &mBorderColor) {}
void LoadETC2RGB8A1ToBC1(angle::ColorF &mBorderColor) {}
void LoadETC2RGB8A1ToRGBA8(angle::ColorF &mBorderColor) {}
void LoadEACR11ToR16F(angle::ColorF &mBorderColor) {}
void LoadEACR11ToR16(angle::ColorF &mBorderColor) {}
void LoadEACRG11ToRG16F(angle::ColorF &mBorderColor) {}
void LoadEACRG11ToRG16(angle::ColorF &mBorderColor) {}
void LoadETC2RGB8ToRGBA8(angle::ColorF &mBorderColor) {}
void LoadETC2RGB8ToBC1(angle::ColorF &mBorderColor) {}
void LoadETC2RGBA8ToRGBA8(angle::ColorF &mBorderColor) {}
void LoadEACR11SToR16F(angle::ColorF &mBorderColor) {}
void LoadEACR11SToR16(angle::ColorF &mBorderColor) {}
void LoadEACRG11SToRG16(angle::ColorF &mBorderColor) {}
void LoadETC2SRGBA8ToSRGBA8(angle::ColorF &mBorderColor) {}
void LoadETC2SRGB8ToRGBA8(angle::ColorF &mBorderColor) {}
void LoadETC2SRGB8A1ToRGBA8(angle::ColorF &mBorderColor) {}
void LoadETC2SRGB8A1ToBC1(angle::ColorF &mBorderColor) {}
void LoadETC1RGB8ToBC1(angle::ColorF &mBorderColor) {}
void LoadETC1RGB8ToRGBA8(angle::ColorF &mBorderColor) {}
void LoadETC2SRGB8ToBC1(angle::ColorF &mBorderColor) {}
void LoadEACRG11SToRG16F(angle::ColorF &mBorderColor) {}
}  // namespace angle
