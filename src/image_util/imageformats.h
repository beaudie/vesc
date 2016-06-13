//
// Copyright (c) 2013-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// imageformats.h: Defines image format types with functions for mip generation
// and copying.

#ifndef IMAGEUTIL_IMAGEFORMATS_H_
#define IMAGEUTIL_IMAGEFORMATS_H_

#include "libANGLE/angletypes.h"

namespace angle
{

// Several structures share functionality for reading, writing or mipmapping but the layout
// must match the texture format which the structure represents. If collapsing or typedefing
// structs in this header, make sure the functionality and memory layout is exactly the same.

struct L8
{
    unsigned char L;

    static void readColor(gl::ColorF *dst, const L8 *src);
    static void writeColor(L8 *dst, const gl::ColorF *src);
    static void average(L8 *dst, const L8 *src1, const L8 *src2);
};

struct R8
{
    unsigned char R;

    static void readColor(gl::ColorF *dst, const R8 *src);
    static void readColor(gl::ColorUI *dst, const R8 *src);
    static void writeColor(R8 *dst, const gl::ColorF *src);
    static void writeColor(R8 *dst, const gl::ColorUI *src);
    static void average(R8 *dst, const R8 *src1, const R8 *src2);
};

struct A8
{
    unsigned char A;

    static void readColor(gl::ColorF *dst, const A8 *src);
    static void writeColor(A8 *dst, const gl::ColorF *src);
    static void average(A8 *dst, const A8 *src1, const A8 *src2);
};

struct L8A8
{
    unsigned char L;
    unsigned char A;

    static void readColor(gl::ColorF *dst, const L8A8 *src);
    static void writeColor(L8A8 *dst, const gl::ColorF *src);
    static void average(L8A8 *dst, const L8A8 *src1, const L8A8 *src2);
};

struct A8L8
{
    unsigned char A;
    unsigned char L;

    static void readColor(gl::ColorF *dst, const A8L8 *src);
    static void writeColor(A8L8 *dst, const gl::ColorF *src);
    static void average(A8L8 *dst, const A8L8 *src1, const A8L8 *src2);
};

struct R8G8
{
    unsigned char R;
    unsigned char G;

    static void readColor(gl::ColorF *dst, const R8G8 *src);
    static void readColor(gl::ColorUI *dst, const R8G8 *src);
    static void writeColor(R8G8 *dst, const gl::ColorF *src);
    static void writeColor(R8G8 *dst, const gl::ColorUI *src);
    static void average(R8G8 *dst, const R8G8 *src1, const R8G8 *src2);
};

struct R8G8B8
{
    unsigned char R;
    unsigned char G;
    unsigned char B;

    static void readColor(gl::ColorF *dst, const R8G8B8 *src);
    static void readColor(gl::ColorUI *dst, const R8G8B8 *src);
    static void writeColor(R8G8B8 *dst, const gl::ColorF *src);
    static void writeColor(R8G8B8 *dst, const gl::ColorUI *src);
    static void average(R8G8B8 *dst, const R8G8B8 *src1, const R8G8B8 *src2);
};

struct B8G8R8
{
    unsigned char B;
    unsigned char G;
    unsigned char R;

    static void readColor(gl::ColorF *dst, const B8G8R8 *src);
    static void readColor(gl::ColorUI *dst, const B8G8R8 *src);
    static void writeColor(B8G8R8 *dst, const gl::ColorF *src);
    static void writeColor(B8G8R8 *dst, const gl::ColorUI *src);
    static void average(B8G8R8 *dst, const B8G8R8 *src1, const B8G8R8 *src2);
};

struct R5G6B5
{
    // OpenGL ES 2.0.25 spec Section 3.6.2: "Components are packed with the first component in the
    // most significant bits of the bitfield, and successive component occupying progressively less
    // significant locations"
    unsigned short RGB;

    static void readColor(gl::ColorF *dst, const R5G6B5 *src);
    static void writeColor(R5G6B5 *dst, const gl::ColorF *src);
    static void average(R5G6B5 *dst, const R5G6B5 *src1, const R5G6B5 *src2);
};

struct A8R8G8B8
{
    unsigned char A;
    unsigned char R;
    unsigned char G;
    unsigned char B;

    static void readColor(gl::ColorF *dst, const A8R8G8B8 *src);
    static void readColor(gl::ColorUI *dst, const A8R8G8B8 *src);
    static void writeColor(A8R8G8B8 *dst, const gl::ColorF *src);
    static void writeColor(A8R8G8B8 *dst, const gl::ColorUI *src);
    static void average(A8R8G8B8 *dst, const A8R8G8B8 *src1, const A8R8G8B8 *src2);
};

struct R8G8B8A8
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
    unsigned char A;

    static void readColor(gl::ColorF *dst, const R8G8B8A8 *src);
    static void readColor(gl::ColorUI *dst, const R8G8B8A8 *src);
    static void writeColor(R8G8B8A8 *dst, const gl::ColorF *src);
    static void writeColor(R8G8B8A8 *dst, const gl::ColorUI *src);
    static void average(R8G8B8A8 *dst, const R8G8B8A8 *src1, const R8G8B8A8 *src2);
};

struct B8G8R8A8
{
    unsigned char B;
    unsigned char G;
    unsigned char R;
    unsigned char A;

    static void readColor(gl::ColorF *dst, const B8G8R8A8 *src);
    static void readColor(gl::ColorUI *dst, const B8G8R8A8 *src);
    static void writeColor(B8G8R8A8 *dst, const gl::ColorF *src);
    static void writeColor(B8G8R8A8 *dst, const gl::ColorUI *src);
    static void average(B8G8R8A8 *dst, const B8G8R8A8 *src1, const B8G8R8A8 *src2);
};

struct B8G8R8X8
{
    unsigned char B;
    unsigned char G;
    unsigned char R;
    unsigned char X;

    static void readColor(gl::ColorF *dst, const B8G8R8X8 *src);
    static void readColor(gl::ColorUI *dst, const B8G8R8X8 *src);
    static void writeColor(B8G8R8X8 *dst, const gl::ColorF *src);
    static void writeColor(B8G8R8X8 *dst, const gl::ColorUI *src);
    static void average(B8G8R8X8 *dst, const B8G8R8X8 *src1, const B8G8R8X8 *src2);
};

struct A1R5G5B5
{
    unsigned short ARGB;

    static void readColor(gl::ColorF *dst, const A1R5G5B5 *src);
    static void writeColor(A1R5G5B5 *dst, const gl::ColorF *src);
    static void average(A1R5G5B5 *dst, const A1R5G5B5 *src1, const A1R5G5B5 *src2);
};

struct R5G5B5A1
{
    // OpenGL ES 2.0.25 spec Section 3.6.2: "Components are packed with the first component in the
    // most significant
    // bits of the bitfield, and successive component occupying progressively less significant
    // locations"
    unsigned short RGBA;

    static void readColor(gl::ColorF *dst, const R5G5B5A1 *src);
    static void writeColor(R5G5B5A1 *dst, const gl::ColorF *src);
    static void average(R5G5B5A1 *dst, const R5G5B5A1 *src1, const R5G5B5A1 *src2);
};

struct R4G4B4A4
{
    // OpenGL ES 2.0.25 spec Section 3.6.2: "Components are packed with the first component in the
    // most significant
    // bits of the bitfield, and successive component occupying progressively less significant
    // locations"
    unsigned short RGBA;

    static void readColor(gl::ColorF *dst, const R4G4B4A4 *src);
    static void writeColor(R4G4B4A4 *dst, const gl::ColorF *src);
    static void average(R4G4B4A4 *dst, const R4G4B4A4 *src1, const R4G4B4A4 *src2);
};

struct A4R4G4B4
{
    unsigned short ARGB;

    static void readColor(gl::ColorF *dst, const A4R4G4B4 *src);
    static void writeColor(A4R4G4B4 *dst, const gl::ColorF *src);
    static void average(A4R4G4B4 *dst, const A4R4G4B4 *src1, const A4R4G4B4 *src2);
};

struct R16
{
    unsigned short R;

    static void readColor(gl::ColorF *dst, const R16 *src);
    static void readColor(gl::ColorUI *dst, const R16 *src);
    static void writeColor(R16 *dst, const gl::ColorF *src);
    static void writeColor(R16 *dst, const gl::ColorUI *src);
    static void average(R16 *dst, const R16 *src1, const R16 *src2);
};

struct R16G16
{
    unsigned short R;
    unsigned short G;

    static void readColor(gl::ColorF *dst, const R16G16 *src);
    static void readColor(gl::ColorUI *dst, const R16G16 *src);
    static void writeColor(R16G16 *dst, const gl::ColorF *src);
    static void writeColor(R16G16 *dst, const gl::ColorUI *src);
    static void average(R16G16 *dst, const R16G16 *src1, const R16G16 *src2);
};

struct R16G16B16
{
    unsigned short R;
    unsigned short G;
    unsigned short B;

    static void readColor(gl::ColorF *dst, const R16G16B16 *src);
    static void readColor(gl::ColorUI *dst, const R16G16B16 *src);
    static void writeColor(R16G16B16 *dst, const gl::ColorF *src);
    static void writeColor(R16G16B16 *dst, const gl::ColorUI *src);
    static void average(R16G16B16 *dst, const R16G16B16 *src1, const R16G16B16 *src2);
};

struct R16G16B16A16
{
    unsigned short R;
    unsigned short G;
    unsigned short B;
    unsigned short A;

    static void readColor(gl::ColorF *dst, const R16G16B16A16 *src);
    static void readColor(gl::ColorUI *dst, const R16G16B16A16 *src);
    static void writeColor(R16G16B16A16 *dst, const gl::ColorF *src);
    static void writeColor(R16G16B16A16 *dst, const gl::ColorUI *src);
    static void average(R16G16B16A16 *dst, const R16G16B16A16 *src1, const R16G16B16A16 *src2);
};

struct R32
{
    unsigned int R;

    static void readColor(gl::ColorF *dst, const R32 *src);
    static void readColor(gl::ColorUI *dst, const R32 *src);
    static void writeColor(R32 *dst, const gl::ColorF *src);
    static void writeColor(R32 *dst, const gl::ColorUI *src);
    static void average(R32 *dst, const R32 *src1, const R32 *src2);
};

struct R32G32
{
    unsigned int R;
    unsigned int G;

    static void readColor(gl::ColorF *dst, const R32G32 *src);
    static void readColor(gl::ColorUI *dst, const R32G32 *src);
    static void writeColor(R32G32 *dst, const gl::ColorF *src);
    static void writeColor(R32G32 *dst, const gl::ColorUI *src);
    static void average(R32G32 *dst, const R32G32 *src1, const R32G32 *src2);
};

struct R32G32B32
{
    unsigned int R;
    unsigned int G;
    unsigned int B;

    static void readColor(gl::ColorF *dst, const R32G32B32 *src);
    static void readColor(gl::ColorUI *dst, const R32G32B32 *src);
    static void writeColor(R32G32B32 *dst, const gl::ColorF *src);
    static void writeColor(R32G32B32 *dst, const gl::ColorUI *src);
    static void average(R32G32B32 *dst, const R32G32B32 *src1, const R32G32B32 *src2);
};

struct R32G32B32A32
{
    unsigned int R;
    unsigned int G;
    unsigned int B;
    unsigned int A;

    static void readColor(gl::ColorF *dst, const R32G32B32A32 *src);
    static void readColor(gl::ColorUI *dst, const R32G32B32A32 *src);
    static void writeColor(R32G32B32A32 *dst, const gl::ColorF *src);
    static void writeColor(R32G32B32A32 *dst, const gl::ColorUI *src);
    static void average(R32G32B32A32 *dst, const R32G32B32A32 *src1, const R32G32B32A32 *src2);
};

struct R8S
{
    char R;

    static void readColor(gl::ColorF *dst, const R8S *src);
    static void readColor(gl::ColorI *dst, const R8S *src);
    static void writeColor(R8S *dst, const gl::ColorF *src);
    static void writeColor(R8S *dst, const gl::ColorI *src);
    static void average(R8S *dst, const R8S *src1, const R8S *src2);
};

struct R8G8S
{
    char R;
    char G;

    static void readColor(gl::ColorF *dst, const R8G8S *src);
    static void readColor(gl::ColorI *dst, const R8G8S *src);
    static void writeColor(R8G8S *dst, const gl::ColorF *src);
    static void writeColor(R8G8S *dst, const gl::ColorI *src);
    static void average(R8G8S *dst, const R8G8S *src1, const R8G8S *src2);
};

struct R8G8B8S
{
    char R;
    char G;
    char B;

    static void readColor(gl::ColorF *dst, const R8G8B8S *src);
    static void readColor(gl::ColorI *dst, const R8G8B8S *src);
    static void writeColor(R8G8B8S *dst, const gl::ColorF *src);
    static void writeColor(R8G8B8S *dst, const gl::ColorI *src);
    static void average(R8G8B8S *dst, const R8G8B8S *src1, const R8G8B8S *src2);
};

struct R8G8B8A8S
{
    char R;
    char G;
    char B;
    char A;

    static void readColor(gl::ColorF *dst, const R8G8B8A8S *src);
    static void readColor(gl::ColorI *dst, const R8G8B8A8S *src);
    static void writeColor(R8G8B8A8S *dst, const gl::ColorF *src);
    static void writeColor(R8G8B8A8S *dst, const gl::ColorI *src);
    static void average(R8G8B8A8S *dst, const R8G8B8A8S *src1, const R8G8B8A8S *src2);
};

struct R16S
{
    short R;

    static void readColor(gl::ColorF *dst, const R16S *src);
    static void readColor(gl::ColorI *dst, const R16S *src);
    static void writeColor(R16S *dst, const gl::ColorF *src);
    static void writeColor(R16S *dst, const gl::ColorI *src);
    static void average(R16S *dst, const R16S *src1, const R16S *src2);
};

struct R16G16S
{
    short R;
    short G;

    static void readColor(gl::ColorF *dst, const R16G16S *src);
    static void readColor(gl::ColorI *dst, const R16G16S *src);
    static void writeColor(R16G16S *dst, const gl::ColorF *src);
    static void writeColor(R16G16S *dst, const gl::ColorI *src);
    static void average(R16G16S *dst, const R16G16S *src1, const R16G16S *src2);
};

struct R16G16B16S
{
    short R;
    short G;
    short B;

    static void readColor(gl::ColorF *dst, const R16G16B16S *src);
    static void readColor(gl::ColorI *dst, const R16G16B16S *src);
    static void writeColor(R16G16B16S *dst, const gl::ColorF *src);
    static void writeColor(R16G16B16S *dst, const gl::ColorI *src);
    static void average(R16G16B16S *dst, const R16G16B16S *src1, const R16G16B16S *src2);
};

struct R16G16B16A16S
{
    short R;
    short G;
    short B;
    short A;

    static void readColor(gl::ColorF *dst, const R16G16B16A16S *src);
    static void readColor(gl::ColorI *dst, const R16G16B16A16S *src);
    static void writeColor(R16G16B16A16S *dst, const gl::ColorF *src);
    static void writeColor(R16G16B16A16S *dst, const gl::ColorI *src);
    static void average(R16G16B16A16S *dst, const R16G16B16A16S *src1, const R16G16B16A16S *src2);
};

struct R32S
{
    int R;

    static void readColor(gl::ColorF *dst, const R32S *src);
    static void readColor(gl::ColorI *dst, const R32S *src);
    static void writeColor(R32S *dst, const gl::ColorF *src);
    static void writeColor(R32S *dst, const gl::ColorI *src);
    static void average(R32S *dst, const R32S *src1, const R32S *src2);
};

struct R32G32S
{
    int R;
    int G;

    static void readColor(gl::ColorF *dst, const R32G32S *src);
    static void readColor(gl::ColorI *dst, const R32G32S *src);
    static void writeColor(R32G32S *dst, const gl::ColorF *src);
    static void writeColor(R32G32S *dst, const gl::ColorI *src);
    static void average(R32G32S *dst, const R32G32S *src1, const R32G32S *src2);
};

struct R32G32B32S
{
    int R;
    int G;
    int B;

    static void readColor(gl::ColorF *dst, const R32G32B32S *src);
    static void readColor(gl::ColorI *dst, const R32G32B32S *src);
    static void writeColor(R32G32B32S *dst, const gl::ColorF *src);
    static void writeColor(R32G32B32S *dst, const gl::ColorI *src);
    static void average(R32G32B32S *dst, const R32G32B32S *src1, const R32G32B32S *src2);
};

struct R32G32B32A32S
{
    int R;
    int G;
    int B;
    int A;

    static void readColor(gl::ColorF *dst, const R32G32B32A32S *src);
    static void readColor(gl::ColorI *dst, const R32G32B32A32S *src);
    static void writeColor(R32G32B32A32S *dst, const gl::ColorF *src);
    static void writeColor(R32G32B32A32S *dst, const gl::ColorI *src);
    static void average(R32G32B32A32S *dst, const R32G32B32A32S *src1, const R32G32B32A32S *src2);
};

struct A16B16G16R16F
{
    unsigned short A;
    unsigned short R;
    unsigned short G;
    unsigned short B;

    static void readColor(gl::ColorF *dst, const A16B16G16R16F *src);
    static void writeColor(A16B16G16R16F *dst, const gl::ColorF *src);
    static void average(A16B16G16R16F *dst, const A16B16G16R16F *src1, const A16B16G16R16F *src2);
};

struct R16G16B16A16F
{
    unsigned short R;
    unsigned short G;
    unsigned short B;
    unsigned short A;

    static void readColor(gl::ColorF *dst, const R16G16B16A16F *src);
    static void writeColor(R16G16B16A16F *dst, const gl::ColorF *src);
    static void average(R16G16B16A16F *dst, const R16G16B16A16F *src1, const R16G16B16A16F *src2);
};

struct R16F
{
    unsigned short R;

    static void readColor(gl::ColorF *dst, const R16F *src);
    static void writeColor(R16F *dst, const gl::ColorF *src);
    static void average(R16F *dst, const R16F *src1, const R16F *src2);
};

struct A16F
{
    unsigned short A;

    static void readColor(gl::ColorF *dst, const A16F *src);
    static void writeColor(A16F *dst, const gl::ColorF *src);
    static void average(A16F *dst, const A16F *src1, const A16F *src2);
};

struct L16F
{
    unsigned short L;

    static void readColor(gl::ColorF *dst, const L16F *src);
    static void writeColor(L16F *dst, const gl::ColorF *src);
    static void average(L16F *dst, const L16F *src1, const L16F *src2);
};

struct L16A16F
{
    unsigned short L;
    unsigned short A;

    static void readColor(gl::ColorF *dst, const L16A16F *src);
    static void writeColor(L16A16F *dst, const gl::ColorF *src);
    static void average(L16A16F *dst, const L16A16F *src1, const L16A16F *src2);
};

struct R16G16F
{
    unsigned short R;
    unsigned short G;

    static void readColor(gl::ColorF *dst, const R16G16F *src);
    static void writeColor(R16G16F *dst, const gl::ColorF *src);
    static void average(R16G16F *dst, const R16G16F *src1, const R16G16F *src2);
};

struct R16G16B16F
{
    unsigned short R;
    unsigned short G;
    unsigned short B;

    static void readColor(gl::ColorF *dst, const R16G16B16F *src);
    static void writeColor(R16G16B16F *dst, const gl::ColorF *src);
    static void average(R16G16B16F *dst, const R16G16B16F *src1, const R16G16B16F *src2);
};

struct A32B32G32R32F
{
    float A;
    float R;
    float G;
    float B;

    static void readColor(gl::ColorF *dst, const A32B32G32R32F *src);
    static void writeColor(A32B32G32R32F *dst, const gl::ColorF *src);
    static void average(A32B32G32R32F *dst, const A32B32G32R32F *src1, const A32B32G32R32F *src2);
};

struct R32G32B32A32F
{
    float R;
    float G;
    float B;
    float A;

    static void readColor(gl::ColorF *dst, const R32G32B32A32F *src);
    static void writeColor(R32G32B32A32F *dst, const gl::ColorF *src);
    static void average(R32G32B32A32F *dst, const R32G32B32A32F *src1, const R32G32B32A32F *src2);
};

struct R32F
{
    float R;

    static void readColor(gl::ColorF *dst, const R32F *src);
    static void writeColor(R32F *dst, const gl::ColorF *src);
    static void average(R32F *dst, const R32F *src1, const R32F *src2);
};

struct A32F
{
    float A;

    static void readColor(gl::ColorF *dst, const A32F *src);
    static void writeColor(A32F *dst, const gl::ColorF *src);
    static void average(A32F *dst, const A32F *src1, const A32F *src2);
};

struct L32F
{
    float L;

    static void readColor(gl::ColorF *dst, const L32F *src);
    static void writeColor(L32F *dst, const gl::ColorF *src);
    static void average(L32F *dst, const L32F *src1, const L32F *src2);
};

struct L32A32F
{
    float L;
    float A;

    static void readColor(gl::ColorF *dst, const L32A32F *src);
    static void writeColor(L32A32F *dst, const gl::ColorF *src);
    static void average(L32A32F *dst, const L32A32F *src1, const L32A32F *src2);
};

struct R32G32F
{
    float R;
    float G;

    static void readColor(gl::ColorF *dst, const R32G32F *src);
    static void writeColor(R32G32F *dst, const gl::ColorF *src);
    static void average(R32G32F *dst, const R32G32F *src1, const R32G32F *src2);
};

struct R32G32B32F
{
    float R;
    float G;
    float B;

    static void readColor(gl::ColorF *dst, const R32G32B32F *src);
    static void writeColor(R32G32B32F *dst, const gl::ColorF *src);
    static void average(R32G32B32F *dst, const R32G32B32F *src1, const R32G32B32F *src2);
};

struct R10G10B10A2
{
    unsigned int R : 10;
    unsigned int G : 10;
    unsigned int B : 10;
    unsigned int A : 2;

    static void readColor(gl::ColorF *dst, const R10G10B10A2 *src);
    static void readColor(gl::ColorUI *dst, const R10G10B10A2 *src);
    static void writeColor(R10G10B10A2 *dst, const gl::ColorF *src);
    static void writeColor(R10G10B10A2 *dst, const gl::ColorUI *src);
    static void average(R10G10B10A2 *dst, const R10G10B10A2 *src1, const R10G10B10A2 *src2);
};

struct R9G9B9E5
{
    unsigned int R : 9;
    unsigned int G : 9;
    unsigned int B : 9;
    unsigned int E : 5;

    static void readColor(gl::ColorF *dst, const R9G9B9E5 *src);
    static void writeColor(R9G9B9E5 *dst, const gl::ColorF *src);
    static void average(R9G9B9E5 *dst, const R9G9B9E5 *src1, const R9G9B9E5 *src2);
};

struct R11G11B10F
{
    unsigned int R : 11;
    unsigned int G : 11;
    unsigned int B : 10;

    static void readColor(gl::ColorF *dst, const R11G11B10F *src);
    static void writeColor(R11G11B10F *dst, const gl::ColorF *src);
    static void average(R11G11B10F *dst, const R11G11B10F *src1, const R11G11B10F *src2);
};
}  // namespace angle

#endif  // IMAGEUTIL_IMAGEFORMATS_H_
