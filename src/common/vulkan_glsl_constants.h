//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file contains constants used by TranslatorVulkan.cpp which are needed by the Vulkan backend
// (and by extension, the metal backend).
//

#ifndef COMMON_VULKAN_GLSL_CONSTANTS_H_
#define COMMON_VULKAN_GLSL_CONSTANTS_H_

#include "common/PackedEnums.h"

namespace rx
{
namespace vk
{

// Specialization constant ids
enum class SpecializationConstantId : uint32_t
{
    LineRasterEmulation = 0,

    InvalidEnum = 1,
    EnumCount   = 1,
};

using SpecializationConstantBitSet = angle::PackedEnumBitSet<SpecializationConstantId, uint32_t>;
static_assert(sizeof(SpecializationConstantBitSet) == sizeof(uint32_t), "Unexpected size");

template <typename T>
using SpecializationConstantMap = angle::PackedEnumMap<SpecializationConstantId, T>;

}  // namespace vk
}  // namespace rx

#endif  // COMMON_VULKAN_GLSL_CONSTANTS_H_
