// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VulkanCommandBufferPerf:
//   Performance benchmark for Vulkan Primary/Secondary Command Buffer implementations.

#include "ANGLEPerfTest.h"
#include "VulkanCommandBufferPerfParams.h"
#include "test_utils/vulkan_command_buffer_utils.h"

static const char *vertShaderText =
    "#version 400\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
    "#extension GL_ARB_shading_language_420pack : enable\n"
    "layout (std140, binding = 0) uniform bufferVals {\n"
    "    mat4 mvp;\n"
    "} myBufferVals;\n"
    "layout (location = 0) in vec4 pos;\n"
    "layout (location = 1) in vec4 inColor;\n"
    "layout (location = 0) out vec4 outColor;\n"
    "void main() {\n"
    "   outColor = inColor;\n"
    "   gl_Position = myBufferVals.mvp * pos;\n"
    "}\n";

static const char *fragShaderText =
    "#version 400\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
    "#extension GL_ARB_shading_language_420pack : enable\n"
    "layout (location = 0) in vec4 color;\n"
    "layout (location = 0) out vec4 outColor;\n"
    "void main() {\n"
    "   outColor = color;\n"
    "}\n";

class VulkanCommandBufferPerfTest : public ANGLEPerfTest,
                                    public ::testing::WithParamInterface<CommandBufferTestParams>
{
  public:
    VulkanCommandBufferPerfTest();

    void SetUp() override;
    void TearDown() override;
    void step() override;

  private:
    VkClearValue clear_values[2];
    VkSemaphore imageAcquiredSemaphore;
    VkFence drawFence;

    VkResult res;
    const bool mDepthPresent = true;
    struct sample_info mInfo;
    std::string mSample_title;
    commandBufferImpl mCBImplementation;
    int mFrames;
};

VulkanCommandBufferPerfTest::VulkanCommandBufferPerfTest()
    : ANGLEPerfTest("VulkanCommandBufferPerfTest", GetParam().suffix)
{
    mInfo             = {};
    mSample_title     = "Draw Textured Cube";
    mCBImplementation = GetParam().CBImplementation;
    mFrames           = GetParam().frames;
}

void VulkanCommandBufferPerfTest::SetUp()
{
    init_global_layer_properties(mInfo);
    init_instance_extension_names(mInfo);
    init_device_extension_names(mInfo);
    init_instance(mInfo, mSample_title.c_str());
    init_enumerate_device(mInfo);
    init_window_size(mInfo, 500, 500);
    init_connection(mInfo);
    init_window(mInfo);
    init_swapchain_extension(mInfo);
    init_device(mInfo);

    init_command_pool(mInfo);
    init_command_buffer(mInfo);         // Primary command buffer to hold secondaries
    init_command_buffer_array(mInfo);   // Array of primary command buffers
    init_command_buffer2_array(mInfo);  // Array containing all secondary buffers
    init_device_queue(mInfo);
    init_swap_chain(mInfo);
    init_depth_buffer(mInfo);
    init_uniform_buffer(mInfo);
    init_descriptor_and_pipeline_layouts(mInfo, false);
    init_renderpass(mInfo, mDepthPresent);
    init_shaders(mInfo, vertShaderText, fragShaderText);
    init_framebuffers(mInfo, mDepthPresent);
    init_vertex_buffer(mInfo, g_vb_solid_face_colors_Data, sizeof(g_vb_solid_face_colors_Data),
                       sizeof(g_vb_solid_face_colors_Data[0]), false);
    init_descriptor_pool(mInfo, false);
    init_descriptor_set(mInfo);
    init_pipeline_cache(mInfo);
    init_pipeline(mInfo, mDepthPresent);

    clear_values[0].color.float32[0]     = 0.2f;
    clear_values[0].color.float32[1]     = 0.2f;
    clear_values[0].color.float32[2]     = 0.2f;
    clear_values[0].color.float32[3]     = 0.2f;
    clear_values[1].depthStencil.depth   = 1.0f;
    clear_values[1].depthStencil.stencil = 0;

    VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
    imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    imageAcquiredSemaphoreCreateInfo.pNext = NULL;
    imageAcquiredSemaphoreCreateInfo.flags = 0;
    res = vkCreateSemaphore(mInfo.device, &imageAcquiredSemaphoreCreateInfo, NULL,
                            &imageAcquiredSemaphore);
    assert(res == VK_SUCCESS);

    VkFenceCreateInfo fenceInfo;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = NULL;
    fenceInfo.flags = 0;
    res             = vkCreateFence(mInfo.device, &fenceInfo, NULL, &drawFence);
    assert(res == VK_SUCCESS);
}

void VulkanCommandBufferPerfTest::step()
{
    for (int x = 0; x < mFrames; x++)
    {
        mInfo.current_buffer = x % mInfo.swapchainImageCount;

        // Get the index of the next available swapchain image:
        res = vkAcquireNextImageKHR(mInfo.device, mInfo.swap_chain, UINT64_MAX,
                                    imageAcquiredSemaphore, VK_NULL_HANDLE, &mInfo.current_buffer);
        // TODO: Deal with the VK_SUBOPTIMAL_KHR and VK_ERROR_OUT_OF_DATE_KHR
        // return codes
        assert(res == VK_SUCCESS);
        mCBImplementation(mInfo, clear_values, drawFence, imageAcquiredSemaphore);
    }
}

void VulkanCommandBufferPerfTest::TearDown()
{
    vkDestroySemaphore(mInfo.device, imageAcquiredSemaphore, NULL);
    vkDestroyFence(mInfo.device, drawFence, NULL);
    destroy_pipeline(mInfo);
    destroy_pipeline_cache(mInfo);
    destroy_descriptor_pool(mInfo);
    destroy_vertex_buffer(mInfo);
    destroy_framebuffers(mInfo);
    destroy_shaders(mInfo);
    destroy_renderpass(mInfo);
    destroy_descriptor_and_pipeline_layouts(mInfo);
    destroy_uniform_buffer(mInfo);
    destroy_depth_buffer(mInfo);
    destroy_swap_chain(mInfo);
    destroy_command_buffer2_array(mInfo);
    destroy_command_buffer_array(mInfo);
    destroy_command_buffer(mInfo);
    destroy_command_pool(mInfo);
    destroy_device(mInfo);
    destroy_window(mInfo);
    destroy_instance(mInfo);
    ANGLEPerfTest::TearDown();
}

TEST_P(VulkanCommandBufferPerfTest, Run)
{
    run();
}


#if defined(__ANDROID__) || defined(__linux__)
INSTANTIATE_TEST_CASE_P(,
                        VulkanCommandBufferPerfTest,
                        ::testing::Values(PrimaryCBParams(),
                                          PrimaryCB2Params(),
                                          SecondaryCBParams()));
#endif
