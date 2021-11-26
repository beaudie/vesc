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
    ASSERT_NE(EGL_NO_DEVICE_EXT, device);
    EXPECT_TRUE(IsEGLDeviceExtensionEnabled(device, "EGL_ANGLE_device_vulkan"));

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_INSTANCE_ANGLE, &result));
    VkInstance instance = reinterpret_cast<VkInstance>(result);
    ASSERT_NE(instance, VK_NULL_HANDLE);

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_PHYSICAL_DEVICE_ANGLE, &result));
    VkPhysicalDevice physical_device = reinterpret_cast<VkPhysicalDevice>(result);
    ASSERT_NE(physical_device, VK_NULL_HANDLE);

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_DEVICE_ANGLE, &result));
    VkDevice vk_device = reinterpret_cast<VkDevice>(result);
    ASSERT_NE(vk_device, VK_NULL_HANDLE);

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_QUEUE_ANGLE, &result));
    VkQueue queue = reinterpret_cast<VkQueue>(result);
    ASSERT_NE(queue, VK_NULL_HANDLE);

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_QUEUE_FAMILIY_INDEX_ANGLE, &result));

    {
        EXPECT_EGL_TRUE(
            eglQueryDeviceAttribEXT(device, EGL_VULKAN_DEVICE_EXTENSIONS_ANGLE, &result));
        const char *const *extensions = reinterpret_cast<const char *const *>(result);
        ASSERT_NE(extensions, nullptr);
        int extension_count = 0;
        while (extensions[extension_count])
        {
            extension_count++;
        }
        ASSERT_NE(extension_count, 0);
    }

    {
        EXPECT_EGL_TRUE(
            eglQueryDeviceAttribEXT(device, EGL_VULKAN_INSTANCE_EXTENSIONS_ANGLE, &result));
        const char *const *extensions = reinterpret_cast<const char *const *>(result);
        ASSERT_NE(extensions, nullptr);
        int extension_count = 0;
        while (extensions[extension_count])
        {
            extension_count++;
        }
        ASSERT_NE(extension_count, 0);
    }

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_FEATURES_ANGLE, &result));
    const VkPhysicalDeviceFeatures2KHR *features =
        reinterpret_cast<const VkPhysicalDeviceFeatures2KHR *>(result);
    ASSERT_NE(features, nullptr);
    ASSERT_EQ(features->sType, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2);

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_GET_INSTANCE_PROC_ADDR, &result));
    PFN_vkGetInstanceProcAddr get_instance_proc_addr =
        reinterpret_cast<PFN_vkGetInstanceProcAddr>(result);
    ASSERT_NE(get_instance_proc_addr, nullptr);
}

TEST_P(VulkanImageTest, VulkanImage)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    EXPECT_TRUE(IsEGLDisplayExtensionEnabled(display, "EGL_ANGLE_vulkan_image"));
    EXPECT_TRUE(IsGLExtensionEnabled("GL_ANGLE_vulkan_image"));

    constexpr uint kWidth = 256;
    constexpr uint kHeigt = 256;
    GLuint texture       = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeigt, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_NO_ERROR();

    EGLContext context    = window->getContext();
    EGLImageKHR egl_image = eglCreateImageKHR(display, context, EGL_GL_TEXTURE_2D_KHR,
                                              reinterpret_cast<EGLClientBuffer>(texture), nullptr);
    ASSERT_NE(egl_image, EGL_NO_IMAGE_KHR);

    VkImage vk_image = VK_NULL_HANDLE;
    VkImageCreateInfo info = {};
    EXPECT_EGL_TRUE(eglExportVkImageANGLE(display, egl_image, &vk_image, &info));
    ASSERT_NE(vk_image, VK_NULL_HANDLE);
    ASSERT_EQ(info.sType, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
    ASSERT_EQ(info.pNext, nullptr);
    ASSERT_EQ(info.imageType, VK_IMAGE_TYPE_2D);
    ASSERT_EQ(info.format, VK_FORMAT_R8G8B8A8_UNORM);
    ASSERT_EQ(info.extent.width, kWidth);
    ASSERT_EQ(info.extent.height, kHeigt);
    ASSERT_EQ(info.extent.depth, 1u);
    ASSERT_EQ(info.queueFamilyIndexCount, 0u);
    ASSERT_EQ(info.pQueueFamilyIndices, nullptr);
    ASSERT_EQ(info.initialLayout, VK_IMAGE_LAYOUT_UNDEFINED);

    EXPECT_EGL_TRUE(eglDestroyImageKHR(display, egl_image));

    GLenum layout = GL_NONE;
    glReleaseTexturesANGLE(1, &texture, &layout);
    EXPECT_GL_NO_ERROR();
    
    glAcquireTexturesANGLE(1, &texture, &layout);
    EXPECT_GL_NO_ERROR();

    glDeleteTextures(1, &texture);
    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(VulkanImageTest);

}  // namespace angle
