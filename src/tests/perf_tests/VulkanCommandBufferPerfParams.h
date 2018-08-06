// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VulkanCommandBufferPerfParams:
//   Implementations of various command buffer use cases for performance tests.
#ifndef COMMAND_BUFFER_PARAMS_H
#define COMMAND_BUFFER_PARAMS_H

#include <iostream>
#include <sstream>
#include "ANGLEPerfTest.h"
#include "test_utils/third_party/vulkan_command_buffer_utils.h"

typedef void (*commandBufferImpl)(sample_info &info,
                                  VkClearValue *clear_values,
                                  VkFence drawFence,
                                  VkSemaphore imageAcquiredSemaphore);

struct CommandBufferTestParams
{
    commandBufferImpl CBImplementation;
    std::string suffix;
    int frames = 1;
};

CommandBufferTestParams PrimaryCBParams();
CommandBufferTestParams PrimaryCB2Params();
CommandBufferTestParams SecondaryCBParams();

void primaryCommandBufferBenchmark(sample_info &info,
                                   VkClearValue *clear_values,
                                   VkFence drawFence,
                                   VkSemaphore imageAcquiredSemaphore);
void primaryCommandBufferBenchmark2(sample_info &info,
                                    VkClearValue *clear_values,
                                    VkFence drawFence,
                                    VkSemaphore imageAcquiredSemaphore);
void secondaryCommandBufferBenchmark(sample_info &info,
                                     VkClearValue *clear_values,
                                     VkFence drawFence,
                                     VkSemaphore imageAcquiredSemaphore);

#endif
