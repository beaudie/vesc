//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VulkanFormatTablesTest:
//   Tests to validate our Vulkan support tables match hardware support.
//

#include "libANGLE/Context.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_instantiate.h"

using namespace angle;

namespace
{

class VulkanFormatTablesTest : public ANGLETest
{
};

// This test enumerates all GL formats - for each, it queries the Vulkan support for
// using it as a texture, filterable, and a render target. It checks this against our
// speed-optimized baked tables, and validates they would give the same result.
TEST_P(VulkanFormatTablesTest, TestFormatSupport)
{
    ASSERT_EQ(EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE, GetParam().getRenderer());

    // Hack the angle!
    const gl::Context *context = reinterpret_cast<gl::Context *>(getEGLWindow()->getContext());
    auto *contextVk            = rx::GetImplAs<rx::ContextVk>(context);
    rx::RendererVk *renderer   = contextVk->getRenderer();

    const gl::FormatSet &allFormats = gl::GetAllSizedInternalFormats();
    for (GLenum internalFormat : allFormats)
    {
        const rx::vk::Format &vkFormat = renderer->getFormat(internalFormat);

        // Similar loop as when we build caps in vk_caps_utils.cpp, but query using
        // vkGetPhysicalDeviceImageFormatProperties instead of vkGetPhysicalDeviceFormatProperties
        // and verify we have all the same caps.
        if (!vkFormat.isDefined())
        {
            // This hasn't been defined in our vk_format_map.json yet so the caps won't be filled.
            continue;
        }

        const gl::TextureCaps &textureCaps = renderer->getNativeTextureCaps().get(internalFormat);

        // Now lets verify that that agaisnt vulkan.
        VkImageFormatProperties imageProperties;

        // isTexturable?
        VkResult result = vkGetPhysicalDeviceImageFormatProperties(
            renderer->getPhysicalDevice(), vkFormat.vkTextureFormat, VK_IMAGE_TYPE_2D,
            VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0,
            &imageProperties);

        bool isTexturable = result != VK_SUCCESS;
        EXPECT_EQ(textureCaps.texturable, isTexturable);

        // isFilterable?
        result = vkGetPhysicalDeviceImageFormatProperties(
            renderer->getPhysicalDevice(), vkFormat.vkTextureFormat, VK_IMAGE_TYPE_2D,
            VK_IMAGE_TILING_OPTIMAL
            /* We assume all our caps are computed agaisnt optimal + linear */,
            VK_IMAGE_USAGE_SAMPLED_BIT, 0, &imageProperties);

        bool isFilterable = result == VK_SUCCESS;
        EXPECT_EQ(textureCaps.filterable, isFilterable);

        // isRenderable?
        result = vkGetPhysicalDeviceImageFormatProperties(
            renderer->getPhysicalDevice(), vkFormat.vkTextureFormat, VK_IMAGE_TYPE_2D,
            VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &imageProperties);

        bool isRenderable = result == VK_SUCCESS;
        EXPECT_EQ(textureCaps.renderable, isRenderable);
    }
}

ANGLE_INSTANTIATE_TEST(VulkanFormatTablesTest, ES2_VULKAN());

}  // anonymous namespace
