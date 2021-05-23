//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// cl_utils.cpp: Helper functions for the CL front end

#include <libANGLE/cl_utils.h>

namespace cl
{

size_t GetChannelCount(cl_channel_order channelOrder)
{
    switch (channelOrder)
    {
        case CL_R:
        case CL_A:
        case CL_LUMINANCE:
        case CL_INTENSITY:
        case CL_DEPTH:
            return 1u;
        case CL_RG:
        case CL_RA:
        case CL_Rx:
            return 2u;
        case CL_RGB:
        case CL_RGx:
        case CL_sRGB:
            return 3u;
        case CL_RGBA:
        case CL_ARGB:
        case CL_BGRA:
        case CL_ABGR:
        case CL_RGBx:
        case CL_sRGBA:
        case CL_sBGRA:
        case CL_sRGBx:
            return 4u;
    }
    return 0u;
}

size_t GetElementSize(const cl_image_format &image_format)
{
    switch (image_format.image_channel_data_type)
    {
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SIGNED_INT8:
        case CL_UNSIGNED_INT8:
            return GetChannelCount(image_format.image_channel_order);
        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT16:
        case CL_UNSIGNED_INT16:
        case CL_HALF_FLOAT:
            return 2u * GetChannelCount(image_format.image_channel_order);
        case CL_SIGNED_INT32:
        case CL_UNSIGNED_INT32:
        case CL_FLOAT:
            return 4u * GetChannelCount(image_format.image_channel_order);
        case CL_UNORM_SHORT_565:
        case CL_UNORM_SHORT_555:
            return 2u;
        case CL_UNORM_INT_101010:
        case CL_UNORM_INT_101010_2:
            return 4u;
    }
    return 0u;
}

}  // namespace cl
