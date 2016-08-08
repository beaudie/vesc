//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// formatutilsvk:
//   Helper for Vulkan format code.

#include "libANGLE/renderer/vulkan/Format.h"

#include "libANGLE/renderer/load_functions_table.h"

namespace rx
{

namespace vk
{

Format::Format(GLenum internalFormat,
               angle::Format::ID formatID,
               VkFormat native,
               InitializeTextureDataFunction initFunction)
    : internalFormat(internalFormat),
      format(angle::Format::Get(formatID)),
      native(native),
      dataInitializerFunction(initFunction),
      loadFunctions(angle::GetLoadFunctionsMap(internalFormat, formatID))
{
}

}  // namespace vk

}  // namespace rx
