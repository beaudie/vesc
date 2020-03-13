//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_loader:
//    Link to the vulkan-loader library
//

#ifndef LIBANGLE_RENDERER_VULKAN_VK_LOADER_H_
#define LIBANGLE_RENDERER_VULKAN_VK_LOADER_H_

#if ANGLE_SHARED_LIBVULKAN
#    include "volk.h"
#else
#    include <vulkan/vulkan.h>
#endif

#endif  // LIBANGLE_RENDERER_VULKAN_VK_LOADER_H_
