//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef SAMPLE_UTIL_TGA_LOADER_HPP
#define SAMPLE_UTIL_TGA_LOADER_HPP

#include <GLES2/gl2.h>

#include <array>
#include <vector>

typedef std::array<unsigned char, 4> Pixel;

struct TGAImage
{
    size_t width;
    size_t height;
    std::vector<Pixel> data;

    TGAImage();
};

bool LoadTGAImageFromFile(const std::string &path, TGAImage *image);
GLuint LoadTextureFromTGAImage(const TGAImage &image);

#endif // SAMPLE_UTIL_TGA_LOADER_HPP
