//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// loadimage_astc.cpp: Decodes ASTC encoded textures.

#include <astcenc.h>

#include "image_util/loadimage.h"

namespace angle
{

void LoadASTCToRGBA8Inner(size_t width,
                          size_t height,
                          size_t depth,
                          uint32_t blockWidth,
                          uint32_t blockHeight,
                          const uint8_t *input,
                          size_t inputRowPitch,
                          size_t inputDepthPitch,
                          uint8_t *output,
                          size_t outputRowPitch,
                          size_t outputDepthPitch)
{
    astcenc_config config;

    static const unsigned int block_z = 1;

    static const float quality           = ASTCENC_PRE_MEDIUM;
    static const astcenc_profile profile = ASTCENC_PRF_LDR;

    astcenc_error status;
    status = astcenc_config_init(profile, blockWidth, blockHeight, block_z, quality,
                                 ASTCENC_FLG_DECOMPRESS_ONLY, &config);
    if (status != ASTCENC_SUCCESS)
    {
        WARN() << "astcenc config init failed: " << astcenc_get_error_string(status);
        return;
    }

    static const unsigned int thread_count = 1;

    astcenc_context *context;
    status = astcenc_context_alloc(&config, thread_count, &context);
    if (status != ASTCENC_SUCCESS)
    {
        WARN() << "Could not allocate astcenc context: " << astcenc_get_error_string(status);
        return;
    }

    // Compute the number of ASTC blocks in each dimension
    uint32_t block_count_x = ((uint32_t)width + config.block_x - 1) / config.block_x;
    uint32_t block_count_y = ((uint32_t)height + config.block_y - 1) / config.block_y;

    // Space needed for 16 bytes of output per compressed block
    size_t comp_len = block_count_x * block_count_y * 16;

    astcenc_image image;
    image.dim_x     = (uint32_t)width;
    image.dim_y     = (uint32_t)height;
    image.dim_z     = 1;
    image.data_type = ASTCENC_TYPE_U8;
    image.data      = reinterpret_cast<void **>(&output);

    static const astcenc_swizzle swizzle{ASTCENC_SWZ_R, ASTCENC_SWZ_G, ASTCENC_SWZ_B,
                                         ASTCENC_SWZ_A};

    status = astcenc_decompress_image(context, input, comp_len, &image, &swizzle, 0);
    if (status != ASTCENC_SUCCESS)
    {
        WARN() << "astcenc decompress failed: " << astcenc_get_error_string(status);
    }

    astcenc_context_free(context);
}
}  // namespace angle
