//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "texture_utils.h"
#include <array>
#include <vector>

#include <inttypes.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GLuint CreateSimpleTexture2D()
{
    // Use tightly packed data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Generate a texture object
    GLuint texture;
    glGenTextures(1, &texture);

    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, texture);

    // Load the texture: 2x2 Image, 3 bytes per pixel (R, G, B)
    const size_t width = 2;
    const size_t height = 2;
    GLubyte pixels[width * height * 3] =
    {
        255,   0,   0, // Red
          0, 255,   0, // Green
          0,   0, 255, // Blue
        255, 255,   0, // Yellow
    };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // Set the filtering mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return texture;
}

GLuint CreateSimpleTexture2DAlpha1()
{
    // Use tightly packed data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Generate a texture object
    GLuint texture;
    glGenTextures(1, &texture);

    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, texture);

    // Load the texture: 2x2 Image, 4 bytes per pixel (R, G, B, A)
    const size_t width = 2;
    const size_t height = 2;
    GLubyte pixels[width * height * 4] =
    {
        255,   0,   0, 255, // Red
          0, 255,   0, 255, // Green
          0,   0, 255, 255, // Blue
        255, 255,   0, 255, // Yellow
    };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Set the filtering mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return texture;
}

GLuint CreateSimpleTexture2DAlphaHalf()
{
    // Use tightly packed data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Generate a texture object
    GLuint texture;
    glGenTextures(1, &texture);

    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, texture);

    // Load the texture: 2x2 Image, 4 bytes per pixel (R, G, B, A)
    const size_t width = 2;
    const size_t height = 2;
    GLubyte pixels[width * height * 4] =
    {
        255,   0,   0, 127, // Red
          0, 255,   0, 127, // Green
          0,   0, 255, 127, // Blue
        255, 255,   0, 127, // Yellow
    };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Set the filtering mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return texture;
}

GLuint CreateSimpleTextureCubemap()
{
    // Generate a texture object
    GLuint texture;
    glGenTextures(1, &texture);

    // Bind the texture object
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    // Load the texture faces
    GLubyte pixels[6][3] =
    {
        // Face 0 - Red
        { 255, 0, 0 },
        // Face 1 - Green,
        { 0, 255, 0 },
        // Face 3 - Blue
        { 0, 0, 255 },
        // Face 4 - Yellow
        { 255, 255, 0 },
        // Face 5 - Purple
        { 255, 0, 255 },
        // Face 6 - White
        { 255, 255, 255 }
    };

    for (size_t i = 0; i < 6; i++)
    {
        glTexImage2D(static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, GL_RGB, 1, 1, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, &pixels[i]);
    }

    // Set the filtering mode
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return texture;
}

GLuint CreateMipMappedTexture2D()
{
    // Texture object handle
    const size_t width = 256;
    const size_t height = 256;
    std::array<GLubyte, width * height * 3> pixels;

    const size_t checkerSize = 8;
    for (size_t y = 0; y < height; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            GLubyte rColor = 0;
            GLubyte bColor = 0;

            if ((x / checkerSize) % 2 == 0)
            {
                rColor = 255 * ((y / checkerSize) % 2);
                bColor = 255 * (1 - ((y / checkerSize) % 2));
            }
            else
            {
                bColor = 255 * ((y / checkerSize) % 2);
                rColor = 255 * (1 - ((y / checkerSize) % 2));
            }

            pixels[(y * height + x) * 3] = rColor;
            pixels[(y * height + x) * 3 + 1] = 0;
            pixels[(y * height + x) * 3 + 2] = bColor;
        }
    }

    // Generate a texture object
    GLuint texture;
    glGenTextures(1, &texture);

    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, texture);

    // Load mipmap level 0
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Generate mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set the filtering mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texture;
}

GLuint CreateTextureFromPng(const char* fn,
                            unsigned* _width,
                            unsigned* _height) {

    FILE *fp = 0;
    unsigned char header[8];
    png_structp p = 0;
    png_infop pi = 0;

    png_uint_32 width, height;
    int bitdepth, colortype, imethod, cmethod, fmethod;

    p = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!p) return 0;

    pi = png_create_info_struct(p);
    if (!pi) return 0;

    fp = fopen(fn, "rb");
    if (!fp) return 0;

    if (fread(header, 8, 1, fp) != 1) {
        if (fp) fclose(fp);
        return 0;
    }

    if (png_sig_cmp(header, 0, 8)) {
        if (fp) fclose(fp);
        return 0;
    }

    if (setjmp(png_jmpbuf(p))) {
        png_destroy_read_struct(&p, &pi, 0);
        if (fp) fclose(fp);
        return 0;
    }

    png_init_io(p, fp);
    png_set_sig_bytes(p, 8);
    png_read_info(p, pi);
    png_get_IHDR(p, pi, &width, &height, &bitdepth, &colortype,
                 &imethod, &cmethod, &fmethod);

    switch(colortype){
    case PNG_COLOR_TYPE_PALETTE:
        png_set_palette_to_rgb(p);
        break;
    case PNG_COLOR_TYPE_RGB:
        if(png_get_valid(p, pi, PNG_INFO_tRNS)) {
            png_set_tRNS_to_alpha(p);
        } else {
            png_set_filler(p, 0xff, PNG_FILLER_AFTER);
        }
        break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
        // That's what we want; nothing to do here
        break;
    case PNG_COLOR_TYPE_GRAY:
        if(bitdepth < 8) {
            png_set_expand_gray_1_2_4_to_8(p);
        }
        break;
    default:
        if (fp) fclose(fp);
        return 0;
    }

    if (bitdepth == 16) {
        png_set_strip_16(p);
    }

    std::vector<unsigned char> imgData(width * height * 4, 0);
    std::vector<unsigned char*> rowptrs(height, 0);
    for (unsigned int i = 0; i < height; i++) {
        rowptrs[i] = imgData.data() + ((width * 4) * i);
    }

    png_read_image(p, rowptrs.data());
    png_destroy_read_struct(&p, &pi, 0);

    fclose(fp);
    fp = 0;

    *_width = width;
    *_height = height;

    // Use tightly packed data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Generate a texture object
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, imgData.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}
