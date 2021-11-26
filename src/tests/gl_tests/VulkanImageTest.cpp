//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VulkanImageTest.cpp : Tests of images allocated externally using Vulkan.

#include "test_utils/ANGLETest.h"

#include "common/debug.h"
#include "test_utils/VulkanExternalHelper.h"
#include "test_utils/gl_raii.h"

namespace angle
{

class VulkanImageTest : public ANGLETest
{
  protected:
    VulkanImageTest()
    {
        setWindowWidth(1);
        setWindowHeight(1);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

TEST_P(VulkanImageTest, DeviceVulkan)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    EXPECT_TRUE(IsEGLClientExtensionEnabled("EGL_EXT_device_query"));
    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    EGLAttrib result = 0;
    EXPECT_EGL_TRUE(eglQueryDisplayAttribEXT(display, EGL_DEVICE_EXT, &result));

    EGLDeviceEXT device = reinterpret_cast<EGLDeviceEXT>(result);
    EXPECT_NE(EGL_NO_DEVICE_EXT, device);
    EXPECT_TRUE(IsEGLDeviceExtensionEnabled(device, "EGL_ANGLE_device_vulkan"));

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_INSTANCE_ANGLE, &result));
    VkInstance instance = reinterpret_cast<VkInstance>(result);
    EXPECT_NE(instance, static_cast<VkInstance>(VK_NULL_HANDLE));

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_PHYSICAL_DEVICE_ANGLE, &result));
    VkPhysicalDevice physical_device = reinterpret_cast<VkPhysicalDevice>(result);
    EXPECT_NE(physical_device, static_cast<VkPhysicalDevice>(VK_NULL_HANDLE));

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_DEVICE_ANGLE, &result));
    VkDevice vk_device = reinterpret_cast<VkDevice>(result);
    EXPECT_NE(vk_device, static_cast<VkDevice>(VK_NULL_HANDLE));

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_QUEUE_ANGLE, &result));
    VkQueue queue = reinterpret_cast<VkQueue>(result);
    EXPECT_NE(queue, static_cast<VkQueue>(VK_NULL_HANDLE));

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_QUEUE_FAMILIY_INDEX_ANGLE, &result));

    {
        EXPECT_EGL_TRUE(
            eglQueryDeviceAttribEXT(device, EGL_VULKAN_DEVICE_EXTENSIONS_ANGLE, &result));
        const char *const *extensions = reinterpret_cast<const char *const *>(result);
        EXPECT_NE(extensions, nullptr);
        int extension_count = 0;
        while (extensions[extension_count])
        {
            extension_count++;
        }
        EXPECT_NE(extension_count, 0);
    }

    {
        EXPECT_EGL_TRUE(
            eglQueryDeviceAttribEXT(device, EGL_VULKAN_INSTANCE_EXTENSIONS_ANGLE, &result));
        const char *const *extensions = reinterpret_cast<const char *const *>(result);
        EXPECT_NE(extensions, nullptr);
        int extension_count = 0;
        while (extensions[extension_count])
        {
            extension_count++;
        }
        EXPECT_NE(extension_count, 0);
    }

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_FEATURES_ANGLE, &result));
    const VkPhysicalDeviceFeatures2KHR *features =
        reinterpret_cast<const VkPhysicalDeviceFeatures2KHR *>(result);
    EXPECT_NE(features, nullptr);
    EXPECT_EQ(features->sType, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2);

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_GET_INSTANCE_PROC_ADDR, &result));
    PFN_vkGetInstanceProcAddr get_instance_proc_addr =
        reinterpret_cast<PFN_vkGetInstanceProcAddr>(result);
    EXPECT_NE(get_instance_proc_addr, nullptr);
}

TEST_P(VulkanImageTest, VulkanImage)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    EXPECT_TRUE(IsEGLDisplayExtensionEnabled(display, "EGL_ANGLE_vulkan_image"));
    EXPECT_TRUE(IsGLExtensionEnabled("GL_ANGLE_vulkan_image"));

    constexpr GLuint kWidth = 32u;
    constexpr GLuint kHeigt = 32u;
    GLuint texture          = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    std::vector<uint32_t> pixels(kWidth * kHeigt);
    for (auto &pixel : pixels)
        pixel = 0xcfcfcfcf;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeigt, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 pixels.data());
    EXPECT_GL_NO_ERROR();

    EGLContext context    = window->getContext();
    EGLImageKHR egl_image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR,
                                              reinterpret_cast<EGLClientBuffer>(texture), nullptr);
    EXPECT_NE(egl_image, EGL_NO_IMAGE_KHR);

    VkImage vk_image       = VK_NULL_HANDLE;
    VkImageCreateInfo info = {};
    EXPECT_EGL_TRUE(eglExportVkImageANGLE(display, egl_image, &vk_image, &info));
    EXPECT_NE(vk_image, static_cast<VkImage>(VK_NULL_HANDLE));
    EXPECT_EQ(info.sType, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
    EXPECT_EQ(info.pNext, nullptr);
    EXPECT_EQ(info.imageType, VK_IMAGE_TYPE_2D);
    EXPECT_EQ(info.format, VK_FORMAT_R8G8B8A8_UNORM);
    EXPECT_EQ(info.extent.width, kWidth);
    EXPECT_EQ(info.extent.height, kHeigt);
    EXPECT_EQ(info.extent.depth, 1u);
    EXPECT_EQ(info.queueFamilyIndexCount, 0u);
    EXPECT_EQ(info.pQueueFamilyIndices, nullptr);
    EXPECT_EQ(info.initialLayout, VK_IMAGE_LAYOUT_UNDEFINED);

    GLenum layout = GL_NONE;
    glReleaseTexturesANGLE(1, &texture, &layout);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(layout, static_cast<GLenum>(GL_LAYOUT_TRANSFER_DST_EXT));

    VulkanExternalHelper helper;
    helper.initializeFromANGLE();
    std::vector<uint32_t> pixels_out(kWidth * kHeigt);
    helper.readPixels(vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, info.format, {}, info.extent,
                      pixels_out.data(), pixels_out.size() * sizeof(uint32_t));
    for (const auto &pixel : pixels_out)
    {
        EXPECT_EQ(pixel, 0xcfcfcfcf);
    }

    layout = GL_LAYOUT_TRANSFER_SRC_EXT;
    glAcquireTexturesANGLE(1, &texture, &layout);
    EXPECT_GL_NO_ERROR();

    GLuint framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    
    // clear framebuffer with white color.
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebuffer);
    EXPECT_GL_NO_ERROR();

    glReleaseTexturesANGLE(1, &texture, &layout);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(layout, static_cast<GLenum>(GL_LAYOUT_TRANSFER_DST_EXT));

    helper.readPixels(vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, info.format, {}, info.extent,
                      pixels_out.data(), pixels_out.size() * sizeof(uint32_t));
    for (const auto &pixel : pixels_out)
    {
        EXPECT_EQ(pixel, 0xffffffff);
    }

    layout = GL_LAYOUT_TRANSFER_SRC_EXT;
    glAcquireTexturesANGLE(1, &texture, &layout);
    EXPECT_GL_NO_ERROR();

    EXPECT_EGL_TRUE(eglDestroyImageKHR(display, egl_image));

    glDeleteTextures(1, &texture);
    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(VulkanImageTest);

}  // namespace angle
