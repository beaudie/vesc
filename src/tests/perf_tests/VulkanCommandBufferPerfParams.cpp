// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VulkanCommandBufferPerfParams:
//   Implementations of various command buffer use cases for performance tests.

#include "VulkanCommandBufferPerfParams.h"

void primaryCommandBufferBenchmark(sample_info &info,
                                   VkClearValue *clear_values,
                                   VkFence drawFence,
                                   VkSemaphore imageAcquiredSemaphore)
{
    VkResult res;

    VkRenderPassBeginInfo rp_begin;
    rp_begin.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin.pNext                    = NULL;
    rp_begin.renderPass               = info.render_pass;
    rp_begin.framebuffer              = info.framebuffers[info.current_buffer];
    rp_begin.renderArea.offset.x      = 0;
    rp_begin.renderArea.offset.y      = 0;
    rp_begin.renderArea.extent.width  = info.width;
    rp_begin.renderArea.extent.height = info.height;
    rp_begin.clearValueCount          = 2;
    rp_begin.pClearValues             = clear_values;

    VkCommandBufferBeginInfo cmd_buf_info = {};
    cmd_buf_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_buf_info.pNext                    = NULL;
    cmd_buf_info.flags                    = 0;
    cmd_buf_info.pInheritanceInfo         = NULL;

    for (int x = 0; x < NUM_BUFFERS; x++)
    {
        vkBeginCommandBuffer(info.cmds[x], &cmd_buf_info);
        vkCmdBeginRenderPass(info.cmds[x], &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(info.cmds[x], VK_PIPELINE_BIND_POINT_GRAPHICS, info.pipeline);
        vkCmdBindDescriptorSets(info.cmds[x], VK_PIPELINE_BIND_POINT_GRAPHICS, info.pipeline_layout,
                                0, NUM_DESCRIPTOR_SETS, info.desc_set.data(), 0, NULL);

        const VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(info.cmds[x], 0, 1, &info.vertex_buffer.buf, offsets);

        init_viewports_array(info, x);
        init_scissors_array(info, x);

        vkCmdDraw(info.cmds[x], 0, 1, 0, 0);
        vkCmdEndRenderPass(info.cmds[x]);
        res = vkEndCommandBuffer(info.cmds[x]);
        assert(res == VK_SUCCESS);
    }

    VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info[1]           = {};
    submit_info[0].pNext                  = NULL;
    submit_info[0].sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info[0].waitSemaphoreCount     = 1;
    submit_info[0].pWaitSemaphores        = &imageAcquiredSemaphore;
    submit_info[0].pWaitDstStageMask      = &pipe_stage_flags;
    submit_info[0].commandBufferCount     = NUM_BUFFERS;
    submit_info[0].pCommandBuffers        = info.cmds;
    submit_info[0].signalSemaphoreCount   = 0;
    submit_info[0].pSignalSemaphores      = NULL;

    // Queue the command buffer for execution
    res = vkQueueSubmit(info.graphics_queue, 1, submit_info, drawFence);
    assert(res == VK_SUCCESS);

    // Now present the image in the window

    VkPresentInfoKHR present;
    present.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pNext              = NULL;
    present.swapchainCount     = 1;
    present.pSwapchains        = &info.swap_chain;
    present.pImageIndices      = &info.current_buffer;
    present.pWaitSemaphores    = NULL;
    present.waitSemaphoreCount = 0;
    present.pResults           = NULL;

    // Make sure command buffer is finished before presenting
    do
    {
        res = vkWaitForFences(info.device, 1, &drawFence, VK_TRUE, FENCE_TIMEOUT);
    } while (res == VK_TIMEOUT);
    vkResetFences(info.device, 1, &drawFence);

    assert(res == VK_SUCCESS);
    res = vkQueuePresentKHR(info.present_queue, &present);
    assert(res == VK_SUCCESS);
}

void primaryCommandBufferBenchmark2(sample_info &info,
                                    VkClearValue *clear_values,
                                    VkFence drawFence,
                                    VkSemaphore imageAcquiredSemaphore)
{
    VkResult res;

    VkRenderPassBeginInfo rp_begin;
    rp_begin.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin.pNext                    = NULL;
    rp_begin.renderPass               = info.render_pass;
    rp_begin.framebuffer              = info.framebuffers[info.current_buffer];
    rp_begin.renderArea.offset.x      = 0;
    rp_begin.renderArea.offset.y      = 0;
    rp_begin.renderArea.extent.width  = info.width;
    rp_begin.renderArea.extent.height = info.height;
    rp_begin.clearValueCount          = 2;
    rp_begin.pClearValues             = clear_values;

    VkCommandBufferBeginInfo cmd_buf_info = {};
    cmd_buf_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_buf_info.pNext                    = NULL;
    cmd_buf_info.flags                    = 0;
    cmd_buf_info.pInheritanceInfo         = NULL;

    vkBeginCommandBuffer(info.cmd, &cmd_buf_info);
    for (int x = 0; x < NUM_BUFFERS; x++)
    {
        vkCmdBeginRenderPass(info.cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(info.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, info.pipeline);
        vkCmdBindDescriptorSets(info.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, info.pipeline_layout, 0,
                                NUM_DESCRIPTOR_SETS, info.desc_set.data(), 0, NULL);

        const VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(info.cmd, 0, 1, &info.vertex_buffer.buf, offsets);

        init_viewports(info);
        init_scissors(info);

        vkCmdDraw(info.cmd, 0, 1, 0, 0);
        vkCmdEndRenderPass(info.cmd);
    }
    res = vkEndCommandBuffer(info.cmd);
    assert(res == VK_SUCCESS);

    const VkCommandBuffer cmd_bufs[]      = {info.cmd};
    VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info[1]           = {};
    submit_info[0].pNext                  = NULL;
    submit_info[0].sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info[0].waitSemaphoreCount     = 1;
    submit_info[0].pWaitSemaphores        = &imageAcquiredSemaphore;
    submit_info[0].pWaitDstStageMask      = &pipe_stage_flags;
    submit_info[0].commandBufferCount     = 1;
    submit_info[0].pCommandBuffers        = cmd_bufs;
    submit_info[0].signalSemaphoreCount   = 0;
    submit_info[0].pSignalSemaphores      = NULL;

    // Queue the command buffer for execution
    res = vkQueueSubmit(info.graphics_queue, 1, submit_info, drawFence);
    assert(res == VK_SUCCESS);

    // Now present the image in the window

    VkPresentInfoKHR present;
    present.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pNext              = NULL;
    present.swapchainCount     = 1;
    present.pSwapchains        = &info.swap_chain;
    present.pImageIndices      = &info.current_buffer;
    present.pWaitSemaphores    = NULL;
    present.waitSemaphoreCount = 0;
    present.pResults           = NULL;

    // Make sure command buffer is finished before presenting
    do
    {
        res = vkWaitForFences(info.device, 1, &drawFence, VK_TRUE, FENCE_TIMEOUT);
    } while (res == VK_TIMEOUT);
    vkResetFences(info.device, 1, &drawFence);

    assert(res == VK_SUCCESS);
    res = vkQueuePresentKHR(info.present_queue, &present);
    assert(res == VK_SUCCESS);
}

void secondaryCommandBufferBenchmark(sample_info &info,
                                     VkClearValue *clear_values,
                                     VkFence drawFence,
                                     VkSemaphore imageAcquiredSemaphore)
{
    VkResult res;

    // Record Secondary Command Buffer
    VkCommandBufferInheritanceInfo inherit_info = {};
    inherit_info.sType                          = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inherit_info.pNext                          = NULL;
    inherit_info.renderPass                     = info.render_pass;
    inherit_info.subpass                        = 0;
    inherit_info.framebuffer                    = info.framebuffers[info.current_buffer];
    inherit_info.occlusionQueryEnable           = false;
    inherit_info.queryFlags                     = 0;
    inherit_info.pipelineStatistics             = 0;

    VkCommandBufferBeginInfo secondary_cmd_buf_info = {};
    secondary_cmd_buf_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    secondary_cmd_buf_info.pNext                    = NULL;
    secondary_cmd_buf_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT |
                                   VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    secondary_cmd_buf_info.pInheritanceInfo = &inherit_info;

    for (int x = 0; x < NUM_BUFFERS; x++)
    {
        vkBeginCommandBuffer(info.cmd2s[x], &secondary_cmd_buf_info);
        vkCmdBindPipeline(info.cmd2s[x], VK_PIPELINE_BIND_POINT_GRAPHICS, info.pipeline);
        vkCmdBindDescriptorSets(info.cmd2s[x], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                info.pipeline_layout, 0, NUM_DESCRIPTOR_SETS, info.desc_set.data(),
                                0, NULL);
        const VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(info.cmd2s[x], 0, 1, &info.vertex_buffer.buf, offsets);
        init_viewports2_array(info, x);
        init_scissors2_array(info, x);
        vkCmdDraw(info.cmd2s[x], 0, 1, 0, 0);
        vkEndCommandBuffer(info.cmd2s[x]);
    }
    // Record Secondary Command Buffer End

    // Record Primary Command Buffer Begin
    VkRenderPassBeginInfo rp_begin;
    rp_begin.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin.pNext                    = NULL;
    rp_begin.renderPass               = info.render_pass;
    rp_begin.framebuffer              = info.framebuffers[info.current_buffer];
    rp_begin.renderArea.offset.x      = 0;
    rp_begin.renderArea.offset.y      = 0;
    rp_begin.renderArea.extent.width  = info.width;
    rp_begin.renderArea.extent.height = info.height;
    rp_begin.clearValueCount          = 2;
    rp_begin.pClearValues             = clear_values;

    VkCommandBufferBeginInfo primary_cmd_buf_info = {};
    primary_cmd_buf_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    primary_cmd_buf_info.pNext                    = NULL;
    primary_cmd_buf_info.flags                    = 0;
    primary_cmd_buf_info.pInheritanceInfo         = NULL;

    vkBeginCommandBuffer(info.cmd, &primary_cmd_buf_info);
    for (int x = 0; x < NUM_BUFFERS; x++)
    {
        vkCmdBeginRenderPass(info.cmd, &rp_begin, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
        vkCmdExecuteCommands(info.cmd, 1, &info.cmd2s[x]);
        vkCmdEndRenderPass(info.cmd);
    }
    vkEndCommandBuffer(info.cmd);
    // Record Primary Command Buffer End

    const VkCommandBuffer cmd_bufs[]      = {info.cmd};
    VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info[1]           = {};
    submit_info[0].pNext                  = NULL;
    submit_info[0].sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info[0].waitSemaphoreCount     = 1;
    submit_info[0].pWaitSemaphores        = &imageAcquiredSemaphore;
    submit_info[0].pWaitDstStageMask      = &pipe_stage_flags;
    submit_info[0].commandBufferCount     = 1;
    submit_info[0].pCommandBuffers        = cmd_bufs;
    submit_info[0].signalSemaphoreCount   = 0;
    submit_info[0].pSignalSemaphores      = NULL;

    // Queue the command buffer for execution
    res = vkQueueSubmit(info.graphics_queue, 1, submit_info, drawFence);
    assert(res == VK_SUCCESS);

    // Now present the image in the window

    VkPresentInfoKHR present;
    present.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pNext              = NULL;
    present.swapchainCount     = 1;
    present.pSwapchains        = &info.swap_chain;
    present.pImageIndices      = &info.current_buffer;
    present.pWaitSemaphores    = NULL;
    present.waitSemaphoreCount = 0;
    present.pResults           = NULL;

    // Make sure command buffer is finished before presenting
    do
    {
        res = vkWaitForFences(info.device, 1, &drawFence, VK_TRUE, FENCE_TIMEOUT);
    } while (res == VK_TIMEOUT);
    vkResetFences(info.device, 1, &drawFence);

    assert(res == VK_SUCCESS);
    res = vkQueuePresentKHR(info.present_queue, &present);
    assert(res == VK_SUCCESS);
}

CommandBufferTestParams PrimaryCBParams()
{
    CommandBufferTestParams params;
    params.CBImplementation = primaryCommandBufferBenchmark;
    params.suffix           = "_PrimaryCB";
    return params;
}

CommandBufferTestParams PrimaryCB2Params()
{
    CommandBufferTestParams params;
    params.CBImplementation = primaryCommandBufferBenchmark2;
    params.suffix           = "_PrimaryCB2";
    return params;
}

CommandBufferTestParams SecondaryCBParams()
{
    CommandBufferTestParams params;
    params.CBImplementation = secondaryCommandBufferBenchmark;
    params.suffix           = "_SecondaryCB";
    return params;
}
