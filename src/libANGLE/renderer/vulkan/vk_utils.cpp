//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_utils:
//    Helper functions for the Vulkan Renderer.
//

#include "libANGLE/renderer/vulkan/vk_utils.h"

#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/BufferVk.h"
#include "libANGLE/renderer/vulkan/CommandGraph.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"

namespace
{
VkImageUsageFlags GetStagingBufferUsageFlags(rx::vk::StagingUsage usage)
{
    switch (usage)
    {
        case rx::vk::StagingUsage::Read:
            return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        case rx::vk::StagingUsage::Write:
            return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        case rx::vk::StagingUsage::Both:
            return (VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        default:
            UNREACHABLE();
            return 0;
    }
}

constexpr gl::Rectangle kMaxSizedScissor(0,
                                         0,
                                         std::numeric_limits<int>::max(),
                                         std::numeric_limits<int>::max());
}  // anonymous namespace

namespace angle
{
egl::Error ToEGL(Result result, rx::DisplayVk *displayVk, EGLint errorCode)
{
    if (result != angle::Result::Continue)
    {
        return displayVk->getEGLError(errorCode);
    }
    else
    {
        return egl::NoError();
    }
}
}  // namespace angle

namespace rx
{
// Mirrors std_validation_str in loader.c
const char *g_VkStdValidationLayerName = "VK_LAYER_LUNARG_standard_validation";
const char *g_VkValidationLayerNames[] = {
    "VK_LAYER_GOOGLE_threading", "VK_LAYER_LUNARG_parameter_validation",
    "VK_LAYER_LUNARG_object_tracker", "VK_LAYER_LUNARG_core_validation",
    "VK_LAYER_GOOGLE_unique_objects"};

bool HasValidationLayer(const std::vector<VkLayerProperties> &layerProps, const char *layerName)
{
    for (const auto &layerProp : layerProps)
    {
        if (std::string(layerProp.layerName) == layerName)
        {
            return true;
        }
    }

    return false;
}

bool HasStandardValidationLayer(const std::vector<VkLayerProperties> &layerProps)
{
    return HasValidationLayer(layerProps, g_VkStdValidationLayerName);
}

bool HasValidationLayers(const std::vector<VkLayerProperties> &layerProps)
{
    for (const char *layerName : g_VkValidationLayerNames)
    {
        if (!HasValidationLayer(layerProps, layerName))
        {
            return false;
        }
    }

    return true;
}

angle::Result FindAndAllocateCompatibleMemory(vk::Context *context,
                                              const vk::MemoryProperties &memoryProperties,
                                              VkMemoryPropertyFlags requestedMemoryPropertyFlags,
                                              VkMemoryPropertyFlags *memoryPropertyFlagsOut,
                                              const VkMemoryRequirements &memoryRequirements,
                                              vk::DeviceMemory *deviceMemoryOut)
{
    uint32_t memoryTypeIndex = 0;
    ANGLE_TRY(memoryProperties.findCompatibleMemoryIndex(context, memoryRequirements,
                                                         requestedMemoryPropertyFlags,
                                                         memoryPropertyFlagsOut, &memoryTypeIndex));

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.memoryTypeIndex      = memoryTypeIndex;
    allocInfo.allocationSize       = memoryRequirements.size;

    ANGLE_VK_TRY(context, deviceMemoryOut->allocate(context->getDevice(), allocInfo));
    return angle::Result::Continue;
}

template <typename T>
angle::Result AllocateBufferOrImageMemory(vk::Context *context,
                                          VkMemoryPropertyFlags requestedMemoryPropertyFlags,
                                          VkMemoryPropertyFlags *memoryPropertyFlagsOut,
                                          T *bufferOrImage,
                                          vk::DeviceMemory *deviceMemoryOut)
{
    const vk::MemoryProperties &memoryProperties = context->getRenderer()->getMemoryProperties();

    // Call driver to determine memory requirements.
    VkMemoryRequirements memoryRequirements;
    bufferOrImage->getMemoryRequirements(context->getDevice(), &memoryRequirements);

    ANGLE_TRY(FindAndAllocateCompatibleMemory(context, memoryProperties,
                                              requestedMemoryPropertyFlags, memoryPropertyFlagsOut,
                                              memoryRequirements, deviceMemoryOut));
    ANGLE_VK_TRY(context, bufferOrImage->bindMemory(context->getDevice(), *deviceMemoryOut));
    return angle::Result::Continue;
}

const char *g_VkLoaderLayersPathEnv = "VK_LAYER_PATH";
const char *g_VkICDPathEnv          = "VK_ICD_FILENAMES";

const char *VulkanResultString(VkResult result)
{
    switch (result)
    {
        case VK_SUCCESS:
            return "Command successfully completed.";
        case VK_NOT_READY:
            return "A fence or query has not yet completed.";
        case VK_TIMEOUT:
            return "A wait operation has not completed in the specified time.";
        case VK_EVENT_SET:
            return "An event is signaled.";
        case VK_EVENT_RESET:
            return "An event is unsignaled.";
        case VK_INCOMPLETE:
            return "A return array was too small for the result.";
        case VK_SUBOPTIMAL_KHR:
            return "A swapchain no longer matches the surface properties exactly, but can still be "
                   "used to present to the surface successfully.";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "A host memory allocation has failed.";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "A device memory allocation has failed.";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "Initialization of an object could not be completed for implementation-specific "
                   "reasons.";
        case VK_ERROR_DEVICE_LOST:
            return "The logical or physical device has been lost.";
        case VK_ERROR_MEMORY_MAP_FAILED:
            return "Mapping of a memory object has failed.";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "A requested layer is not present or could not be loaded.";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "A requested extension is not supported.";
        case VK_ERROR_FEATURE_NOT_PRESENT:
            return "A requested feature is not supported.";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "The requested version of Vulkan is not supported by the driver or is otherwise "
                   "incompatible for implementation-specific reasons.";
        case VK_ERROR_TOO_MANY_OBJECTS:
            return "Too many objects of the type have already been created.";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return "A requested format is not supported on this device.";
        case VK_ERROR_SURFACE_LOST_KHR:
            return "A surface is no longer available.";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            return "The requested window is already connected to a VkSurfaceKHR, or to some other "
                   "non-Vulkan API.";
        case VK_ERROR_OUT_OF_DATE_KHR:
            return "A surface has changed in such a way that it is no longer compatible with the "
                   "swapchain.";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            return "The display used by a swapchain does not use the same presentable image "
                   "layout, or is incompatible in a way that prevents sharing an image.";
        case VK_ERROR_VALIDATION_FAILED_EXT:
            return "The validation layers detected invalid API usage.";
        default:
            return "Unknown vulkan error code.";
    }
}

bool GetAvailableValidationLayers(const std::vector<VkLayerProperties> &layerProps,
                                  bool mustHaveLayers,
                                  VulkanLayerVector *enabledLayerNames)
{
    if (HasStandardValidationLayer(layerProps))
    {
        enabledLayerNames->push_back(g_VkStdValidationLayerName);
    }
    else if (HasValidationLayers(layerProps))
    {
        for (const char *layerName : g_VkValidationLayerNames)
        {
            enabledLayerNames->push_back(layerName);
        }
    }
    else
    {
        // Generate an error if the layers were explicitly requested, warning otherwise.
        if (mustHaveLayers)
        {
            ERR() << "Vulkan validation layers are missing.";
        }
        else
        {
            WARN() << "Vulkan validation layers are missing.";
        }

        return false;
    }

    return true;
}

namespace vk
{
VkImageAspectFlags GetDepthStencilAspectFlags(const angle::Format &format)
{
    return (format.depthBits > 0 ? VK_IMAGE_ASPECT_DEPTH_BIT : 0) |
           (format.stencilBits > 0 ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
}

VkImageAspectFlags GetFormatAspectFlags(const angle::Format &format)
{
    return (format.redBits > 0 ? VK_IMAGE_ASPECT_COLOR_BIT : 0) |
           GetDepthStencilAspectFlags(format);
}

VkImageAspectFlags GetDepthStencilAspectFlagsForCopy(bool copyDepth, bool copyStencil)
{
    return copyDepth ? VK_IMAGE_ASPECT_DEPTH_BIT
                     : 0 | copyStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0;
}

// Context implementation.
Context::Context(RendererVk *renderer) : mRenderer(renderer) {}

Context::~Context() {}

VkDevice Context::getDevice() const
{
    return mRenderer->getDevice();
}

// MemoryProperties implementation.
MemoryProperties::MemoryProperties() : mMemoryProperties{0} {}

void MemoryProperties::init(VkPhysicalDevice physicalDevice)
{
    ASSERT(mMemoryProperties.memoryTypeCount == 0);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &mMemoryProperties);
    ASSERT(mMemoryProperties.memoryTypeCount > 0);
}

void MemoryProperties::destroy()
{
    mMemoryProperties = {0};
}

angle::Result MemoryProperties::findCompatibleMemoryIndex(
    Context *context,
    const VkMemoryRequirements &memoryRequirements,
    VkMemoryPropertyFlags requestedMemoryPropertyFlags,
    VkMemoryPropertyFlags *memoryPropertyFlagsOut,
    uint32_t *typeIndexOut) const
{
    ASSERT(mMemoryProperties.memoryTypeCount > 0 && mMemoryProperties.memoryTypeCount <= 32);

    // Find a compatible memory pool index. If the index doesn't change, we could cache it.
    // Not finding a valid memory pool means an out-of-spec driver, or internal error.
    // TODO(jmadill): Determine if it is possible to cache indexes.
    // TODO(jmadill): More efficient memory allocation.
    for (size_t memoryIndex : angle::BitSet32<32>(memoryRequirements.memoryTypeBits))
    {
        ASSERT(memoryIndex < mMemoryProperties.memoryTypeCount);

        if ((mMemoryProperties.memoryTypes[memoryIndex].propertyFlags &
             requestedMemoryPropertyFlags) == requestedMemoryPropertyFlags)
        {
            *memoryPropertyFlagsOut = mMemoryProperties.memoryTypes[memoryIndex].propertyFlags;
            *typeIndexOut           = static_cast<uint32_t>(memoryIndex);
            return angle::Result::Continue;
        }
    }

    // TODO(jmadill): Add error message to error.
    context->handleError(VK_ERROR_INCOMPATIBLE_DRIVER, __FILE__, ANGLE_FUNCTION, __LINE__);
    return angle::Result::Stop;
}

// StagingBuffer implementation.
StagingBuffer::StagingBuffer() : mSize(0) {}

void StagingBuffer::destroy(VkDevice device)
{
    mBuffer.destroy(device);
    mDeviceMemory.destroy(device);
    mSize = 0;
}

angle::Result StagingBuffer::init(Context *context, VkDeviceSize size, StagingUsage usage)
{
    VkBufferCreateInfo createInfo    = {};
    createInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.flags                 = 0;
    createInfo.size                  = size;
    createInfo.usage                 = GetStagingBufferUsageFlags(usage);
    createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices   = nullptr;

    VkMemoryPropertyFlags flags =
        (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    ANGLE_VK_TRY(context, mBuffer.init(context->getDevice(), createInfo));
    VkMemoryPropertyFlags flagsOut = 0;
    ANGLE_TRY(AllocateBufferMemory(context, flags, &flagsOut, &mBuffer, &mDeviceMemory));
    mSize = static_cast<size_t>(size);
    return angle::Result::Continue;
}

void StagingBuffer::dumpResources(Serial serial, std::vector<vk::GarbageObject> *garbageQueue)
{
    mBuffer.dumpResources(serial, garbageQueue);
    mDeviceMemory.dumpResources(serial, garbageQueue);
}

angle::Result AllocateBufferMemory(vk::Context *context,
                                   VkMemoryPropertyFlags requestedMemoryPropertyFlags,
                                   VkMemoryPropertyFlags *memoryPropertyFlagsOut,
                                   Buffer *buffer,
                                   DeviceMemory *deviceMemoryOut)
{
    return AllocateBufferOrImageMemory(context, requestedMemoryPropertyFlags,
                                       memoryPropertyFlagsOut, buffer, deviceMemoryOut);
}

angle::Result AllocateImageMemory(vk::Context *context,
                                  VkMemoryPropertyFlags memoryPropertyFlags,
                                  Image *image,
                                  DeviceMemory *deviceMemoryOut)
{
    VkMemoryPropertyFlags memoryPropertyFlagsOut = 0;
    return AllocateBufferOrImageMemory(context, memoryPropertyFlags, &memoryPropertyFlagsOut, image,
                                       deviceMemoryOut);
}

angle::Result InitShaderAndSerial(Context *context,
                                  ShaderAndSerial *shaderAndSerial,
                                  const uint32_t *shaderCode,
                                  size_t shaderCodeSize)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.flags                    = 0;
    createInfo.codeSize                 = shaderCodeSize;
    createInfo.pCode                    = shaderCode;

    ANGLE_VK_TRY(context, shaderAndSerial->get().init(context->getDevice(), createInfo));
    shaderAndSerial->updateSerial(context->getRenderer()->issueShaderSerial());
    return angle::Result::Continue;
}

// GarbageObject implementation.
GarbageObject::GarbageObject()
    : mSerial(), mHandleType(HandleType::Invalid), mHandle(VK_NULL_HANDLE)
{}

GarbageObject::GarbageObject(const GarbageObject &other) = default;

GarbageObject &GarbageObject::operator=(const GarbageObject &other) = default;

bool GarbageObject::destroyIfComplete(VkDevice device, Serial completedSerial)
{
    if (completedSerial >= mSerial)
    {
        destroy(device);
        return true;
    }

    return false;
}

void GarbageObject::destroy(VkDevice device)
{
    switch (mHandleType)
    {
        case HandleType::Semaphore:
            vkDestroySemaphore(device, reinterpret_cast<VkSemaphore>(mHandle), nullptr);
            break;
        case HandleType::CommandBuffer:
            // Command buffers are pool allocated.
            UNREACHABLE();
            break;
        case HandleType::Event:
            vkDestroyEvent(device, reinterpret_cast<VkEvent>(mHandle), nullptr);
            break;
        case HandleType::Fence:
            vkDestroyFence(device, reinterpret_cast<VkFence>(mHandle), nullptr);
            break;
        case HandleType::DeviceMemory:
            vkFreeMemory(device, reinterpret_cast<VkDeviceMemory>(mHandle), nullptr);
            break;
        case HandleType::Buffer:
            vkDestroyBuffer(device, reinterpret_cast<VkBuffer>(mHandle), nullptr);
            break;
        case HandleType::BufferView:
            vkDestroyBufferView(device, reinterpret_cast<VkBufferView>(mHandle), nullptr);
            break;
        case HandleType::Image:
            vkDestroyImage(device, reinterpret_cast<VkImage>(mHandle), nullptr);
            break;
        case HandleType::ImageView:
            vkDestroyImageView(device, reinterpret_cast<VkImageView>(mHandle), nullptr);
            break;
        case HandleType::ShaderModule:
            vkDestroyShaderModule(device, reinterpret_cast<VkShaderModule>(mHandle), nullptr);
            break;
        case HandleType::PipelineLayout:
            vkDestroyPipelineLayout(device, reinterpret_cast<VkPipelineLayout>(mHandle), nullptr);
            break;
        case HandleType::RenderPass:
            vkDestroyRenderPass(device, reinterpret_cast<VkRenderPass>(mHandle), nullptr);
            break;
        case HandleType::Pipeline:
            vkDestroyPipeline(device, reinterpret_cast<VkPipeline>(mHandle), nullptr);
            break;
        case HandleType::DescriptorSetLayout:
            vkDestroyDescriptorSetLayout(device, reinterpret_cast<VkDescriptorSetLayout>(mHandle),
                                         nullptr);
            break;
        case HandleType::Sampler:
            vkDestroySampler(device, reinterpret_cast<VkSampler>(mHandle), nullptr);
            break;
        case HandleType::DescriptorPool:
            vkDestroyDescriptorPool(device, reinterpret_cast<VkDescriptorPool>(mHandle), nullptr);
            break;
        case HandleType::Framebuffer:
            vkDestroyFramebuffer(device, reinterpret_cast<VkFramebuffer>(mHandle), nullptr);
            break;
        case HandleType::CommandPool:
            vkDestroyCommandPool(device, reinterpret_cast<VkCommandPool>(mHandle), nullptr);
            break;
        case HandleType::QueryPool:
            vkDestroyQueryPool(device, reinterpret_cast<VkQueryPool>(mHandle), nullptr);
            break;
        default:
            UNREACHABLE();
            break;
    }
}

//size_t CustomCommandBuffer::commandSize(CommandID cmdType)
//{
//    return sizeof(CommandHeader) + CommandParamSize[cmdType];
//}

// Allocate/initialize memory for the command and return pointer to Cmd Header
void *CustomCommandBuffer::initCommand(CommandID cmdID, size_t variableSize)
{
    size_t paramSize = CommandParamSizes[static_cast<uint32_t>(cmdID)];
    size_t completeSize = sizeof(CommandHeader) + paramSize + variableSize;
    CommandHeader *header = static_cast<CommandHeader*>(mAllocator.allocate(completeSize));
    // Update cmd ID in header
    header->id = cmdID;
    // Update mHead ptr
    mHead = (mHead == nullptr) ? header : mHead;
    // Update prev cmd's "next" ptr and mLast ptr
    if (mLast)
    {
        mLast->next = header;
    }
    // Update mLast ptr
    mLast = header;

    return static_cast<void*>(reinterpret_cast<char*>(header) + sizeof(CommandHeader));
}

void *CustomCommandBuffer::getCmdVariableDataPtr(CommandID cmdID, void *paramPtr)
{
    return static_cast<void*>(reinterpret_cast<char*>(paramPtr) + CommandParamSizes[static_cast<uint32_t>(cmdID)]);
}

void CustomCommandBuffer::bindDescriptorSets(VkPipelineBindPoint bindPoint,
                                             const PipelineLayout& layout,
                                             uint32_t firstSet,
                                             uint32_t descriptorSetCount,
                                             const VkDescriptorSet* descriptorSets,
                                             uint32_t dynamicOffsetCount,
                                             const uint32_t* dynamicOffsets)
{
    size_t descSize = descriptorSetCount*sizeof(VkDescriptorSet);
    size_t offsetSize = dynamicOffsetCount*sizeof(uint32_t);
    size_t varSize =  descSize + offsetSize;
    void *basePtr = initCommand(CommandID::CMD_ID_BIND_DESCRIPTOR_SETS, varSize);
    BindDescriptorSetParams* paramStruct = static_cast<BindDescriptorSetParams*>(basePtr);
    // Copy params into memory
    paramStruct->bindPoint = bindPoint;
    paramStruct->layout = layout.getHandle();
    paramStruct->firstSet = firstSet;
    paramStruct->descriptorSetCount = descriptorSetCount;
    paramStruct->dynamicOffsetCount = dynamicOffsetCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::CMD_ID_BIND_DESCRIPTOR_SETS, basePtr);
    memcpy(ptrData, descriptorSets, descSize);
    paramStruct->descriptorSets = static_cast<const VkDescriptorSet*>(ptrData);
    ptrData = reinterpret_cast<char*>(ptrData) + descSize;
    memcpy(ptrData, dynamicOffsets, offsetSize);
    paramStruct->dynamicOffsets = static_cast<const uint32_t*>(ptrData);
}

void CustomCommandBuffer::bindIndexBuffer(const VkBuffer &buffer, VkDeviceSize offset, VkIndexType indexType)
{
    BindIndexBufferParams* paramStruct = reinterpret_cast<BindIndexBufferParams*>(initCommand(CommandID::CMD_ID_BIND_INDEX_BUFFER, 0));
    paramStruct->buffer = buffer;
    paramStruct->offset = offset;
    paramStruct->indexType = indexType;
}

void CustomCommandBuffer::bindPipeline(VkPipelineBindPoint pipelineBindPoint, const Pipeline &pipeline)
{
    BindPipelineParams* paramStruct = reinterpret_cast<BindPipelineParams*>(initCommand(CommandID::CMD_ID_BIND_PIPELINE, 0));
    paramStruct->pipelineBindPoint = pipelineBindPoint;
    paramStruct->pipeline = pipeline.getHandle();
}

void CustomCommandBuffer::bindVertexBuffers(uint32_t firstBinding,
                           uint32_t bindingCount,
                           const VkBuffer *buffers,
                           const VkDeviceSize *offsets)
{
    size_t buffSize = bindingCount*sizeof(VkBuffer);
    size_t offsetSize = bindingCount*sizeof(VkDeviceSize);
    size_t varSize =  buffSize + offsetSize;
    void *basePtr = initCommand(CommandID::CMD_ID_BIND_VERTEX_BUFFERS, varSize);
    BindVertexBuffersParams* paramStruct = reinterpret_cast<BindVertexBuffersParams*>(basePtr);
    // Copy params
    paramStruct->firstBinding = firstBinding;
    paramStruct->bindingCount = bindingCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::CMD_ID_BIND_VERTEX_BUFFERS, basePtr);
    memcpy(ptrData, buffers, buffSize);
    paramStruct->buffers = static_cast<const VkBuffer *>(ptrData);
    ptrData = reinterpret_cast<char*>(ptrData) + buffSize;
    memcpy(ptrData, offsets, offsetSize);
    paramStruct->offsets = static_cast<const VkDeviceSize*>(ptrData);
}

void CustomCommandBuffer::blitImage(const Image &srcImage,
                   VkImageLayout srcImageLayout,
                   const Image &dstImage,
                   VkImageLayout dstImageLayout,
                   uint32_t regionCount,
                   VkImageBlit *pRegions,
                   VkFilter filter)
{
    size_t regionSize = regionCount*sizeof(VkImageBlit);
    void *basePtr = initCommand(CommandID::CMD_ID_BLIT_IMAGE, regionSize);
    BlitImageParams* paramStruct = reinterpret_cast<BlitImageParams*>(basePtr);
    paramStruct->srcImage = srcImage.getHandle();
    paramStruct->srcImageLayout = srcImageLayout;
    paramStruct->dstImage = dstImage.getHandle();
    paramStruct->dstImageLayout = dstImageLayout;
    paramStruct->regionCount = regionCount;
    paramStruct->filter = filter;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::CMD_ID_BLIT_IMAGE, basePtr);
    memcpy(ptrData, pRegions, regionSize);
    paramStruct->pRegions = reinterpret_cast<VkImageBlit*>(ptrData);
}

void CustomCommandBuffer::copyBuffer(const VkBuffer &srcBuffer,
                    const VkBuffer &destBuffer,
                    uint32_t regionCount,
                    const VkBufferCopy *regions)
{
    size_t regionSize = regionCount*sizeof(VkBufferCopy);
    void *basePtr = initCommand(CommandID::CMD_ID_COPY_BUFFER, regionSize);
    CopyBufferParams* paramStruct = reinterpret_cast<CopyBufferParams*>(basePtr);
    paramStruct->srcBuffer = srcBuffer;
    paramStruct->destBuffer = destBuffer;
    paramStruct->regionCount = regionCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::CMD_ID_COPY_BUFFER, basePtr);
    memcpy(ptrData, regions, regionSize);
    paramStruct->regions = static_cast<const VkBufferCopy*>(ptrData);
}

void CustomCommandBuffer::copyBufferToImage(VkBuffer srcBuffer,
                           const Image &dstImage,
                           VkImageLayout dstImageLayout,
                           uint32_t regionCount,
                           const VkBufferImageCopy *regions)
{
    size_t regionSize = regionCount*sizeof(VkBufferImageCopy);
    void *basePtr = initCommand(CommandID::CMD_ID_COPY_BUFFER_TO_IMAGE, regionSize);
    CopyBufferToImageParams* paramStruct = reinterpret_cast<CopyBufferToImageParams*>(basePtr);
    paramStruct->srcBuffer = srcBuffer;
    paramStruct->dstImage = dstImage.getHandle();
    paramStruct->dstImageLayout = dstImageLayout;
    paramStruct->regionCount = regionCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::CMD_ID_COPY_BUFFER_TO_IMAGE, basePtr);
    memcpy(ptrData, regions, regionSize);
    paramStruct->regions = static_cast<const VkBufferImageCopy*>(ptrData);
}

void CustomCommandBuffer::copyImage(const Image &srcImage,
                   VkImageLayout srcImageLayout,
                   const Image &dstImage,
                   VkImageLayout dstImageLayout,
                   uint32_t regionCount,
                   const VkImageCopy *regions)
{
    size_t regionSize = regionCount*sizeof(VkImageCopy);
    void *basePtr = initCommand(CommandID::CMD_ID_COPY_IMAGE, regionSize);
    CopyImageParams* paramStruct = reinterpret_cast<CopyImageParams*>(basePtr);
    paramStruct->srcImage = srcImage.getHandle();
    paramStruct->srcImageLayout = srcImageLayout;
    paramStruct->dstImage = dstImage.getHandle();
    paramStruct->dstImageLayout = dstImageLayout;
    paramStruct->regionCount = regionCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::CMD_ID_COPY_IMAGE, basePtr);
    memcpy(ptrData, regions, regionSize);
    paramStruct->regions = static_cast<const VkImageCopy*>(ptrData);
}

void CustomCommandBuffer::copyImageToBuffer(const Image &srcImage,
                           VkImageLayout srcImageLayout,
                           VkBuffer dstBuffer,
                           uint32_t regionCount,
                           const VkBufferImageCopy *regions)
{
    size_t regionSize = regionCount*sizeof(VkBufferImageCopy);
    void *basePtr = initCommand(CommandID::CMD_ID_COPY_IMAGE_TO_BUFFER, regionSize);
    CopyImageToBufferParams* paramStruct = reinterpret_cast<CopyImageToBufferParams*>(basePtr);
    paramStruct->srcImage = srcImage.getHandle();
    paramStruct->srcImageLayout = srcImageLayout;
    paramStruct->dstBuffer = dstBuffer;
    paramStruct->regionCount = regionCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::CMD_ID_COPY_IMAGE_TO_BUFFER, basePtr);
    memcpy(ptrData, regions, regionSize);
    paramStruct->regions = static_cast<const VkBufferImageCopy*>(ptrData);
}

void CustomCommandBuffer::clearAttachments(uint32_t attachmentCount,
                          const VkClearAttachment *attachments,
                          uint32_t rectCount,
                          const VkClearRect *rects)
{
    size_t attachSize = attachmentCount*sizeof(VkClearAttachment);
    size_t rectSize = rectCount*sizeof(VkClearRect);
    size_t varSize = attachSize + rectSize;
    void *basePtr = initCommand(CommandID::CMD_ID_CLEAR_ATTACHMENTS, varSize);
    ClearAttachmentsParams* paramStruct = reinterpret_cast<ClearAttachmentsParams*>(basePtr);
    paramStruct->attachmentCount = attachmentCount;
    paramStruct->rectCount = rectCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::CMD_ID_CLEAR_ATTACHMENTS, basePtr);
    memcpy(ptrData, attachments, attachSize);
    paramStruct->attachments = static_cast<const VkClearAttachment *>(ptrData);
    ptrData = reinterpret_cast<char*>(ptrData) + attachSize;
    memcpy(ptrData, rects, rectSize);
    paramStruct->rects = static_cast<const VkClearRect *>(ptrData);
}

void CustomCommandBuffer::clearColorImage(const Image &image,
                         VkImageLayout imageLayout,
                         const VkClearColorValue &color,
                         uint32_t rangeCount,
                         const VkImageSubresourceRange *ranges)
{
    size_t rangeSize = rangeCount*sizeof(VkImageSubresourceRange);
    void *basePtr = initCommand(CommandID::CMD_ID_CLEAR_COLOR_IMAGE, rangeSize);
    ClearColorImageParams* paramStruct = reinterpret_cast<ClearColorImageParams*>(basePtr);
    paramStruct->image = image.getHandle();
    paramStruct->imageLayout = imageLayout;
    paramStruct->color = color;
    paramStruct->rangeCount = rangeCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::CMD_ID_CLEAR_COLOR_IMAGE, basePtr);
    memcpy(ptrData, ranges, rangeSize);
    paramStruct->ranges = static_cast<const VkImageSubresourceRange *>(ptrData);
}

void CustomCommandBuffer::clearDepthStencilImage(const Image &image,
                                VkImageLayout imageLayout,
                                const VkClearDepthStencilValue &depthStencil,
                                uint32_t rangeCount,
                                const VkImageSubresourceRange *ranges)
{
    size_t rangeSize = rangeCount*sizeof(VkImageSubresourceRange);
    void *basePtr = initCommand(CommandID::CMD_ID_CLEAR_DEPTH_STENCIL_IMAGE, rangeSize);
    ClearDepthStencilImageParams* paramStruct = reinterpret_cast<ClearDepthStencilImageParams*>(basePtr);
    paramStruct->image = image.getHandle();
    paramStruct->imageLayout = imageLayout;
    paramStruct->depthStencil = depthStencil;
    paramStruct->rangeCount = rangeCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::CMD_ID_CLEAR_DEPTH_STENCIL_IMAGE, basePtr);
    memcpy(ptrData, ranges, rangeSize);
    paramStruct->ranges = static_cast<const VkImageSubresourceRange *>(ptrData);
}

void CustomCommandBuffer::updateBuffer(const vk::Buffer &buffer,
                      VkDeviceSize dstOffset,
                      VkDeviceSize dataSize,
                      const void *data)
{
    void *basePtr = initCommand(CommandID::CMD_ID_UPDATE_BUFFER, dataSize);
    UpdateBufferParams* paramStruct = reinterpret_cast<UpdateBufferParams*>(basePtr);
    paramStruct->buffer = buffer.getHandle();
    paramStruct->dstOffset = dstOffset;
    paramStruct->dataSize = dataSize;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::CMD_ID_UPDATE_BUFFER, basePtr);
    memcpy(ptrData, data, dataSize);
    paramStruct->data = ptrData;
}

void CustomCommandBuffer::pushConstants(const PipelineLayout &layout,
                       VkShaderStageFlags flag,
                       uint32_t offset,
                       uint32_t size,
                       const void *data)
{
    void *basePtr = initCommand(CommandID::CMD_ID_PUSH_CONSTANTS, size);
    PushConstantsParams* paramStruct = reinterpret_cast<PushConstantsParams*>(basePtr);
    paramStruct->layout = layout.getHandle();
    paramStruct->flag = flag;
    paramStruct->offset = offset;
    paramStruct->size = size;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::CMD_ID_PUSH_CONSTANTS, basePtr);
    memcpy(ptrData, data, size);
    paramStruct->data = ptrData;
}

void CustomCommandBuffer::setViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport *viewports)
{
    size_t viewportSize = viewportCount*sizeof(VkViewport);
    void *basePtr = initCommand(CommandID::CMD_ID_SET_VIEWPORT, viewportSize);
    SetViewportParams* paramStruct = reinterpret_cast<SetViewportParams*>(basePtr);
    paramStruct->firstViewport = firstViewport;
    paramStruct->viewportCount = viewportCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::CMD_ID_SET_VIEWPORT, basePtr);
    memcpy(ptrData, viewports, viewportSize);
    paramStruct->viewports = static_cast<const VkViewport*>(ptrData);
}

void CustomCommandBuffer::setScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D *scissors)
{
    size_t scissorSize = scissorCount*sizeof(VkRect2D);
    void *basePtr = initCommand(CommandID::CMD_ID_SET_SCISSORS, viewportSize);
    SetScissorParams* paramStruct = reinterpret_cast<SetScissorParams*>(basePtr);
    paramStruct->firstScissor = firstScissor;
    paramStruct->scissorCount = scissorCount;
    // Copy variable sized data
    void *ptrData = getCmdVariableDataPtr(CommandID::CMD_ID_SET_SCISSORS, basePtr);
    memcpy(ptrData, scissors, scissorSize);
    paramStruct->scissors = static_cast<const VkRect2D *>(ptrData);
}

void CustomCommandBuffer::draw(uint32_t vertexCount,
              uint32_t instanceCount,
              uint32_t firstVertex,
              uint32_t firstInstance)
{
    DrawParams* paramStruct = reinterpret_cast<DrawParams*>(initCommand(CommandID::CMD_ID_DRAW, 0));
    paramStruct->vertexCount = vertexCount;
    paramStruct->instanceCount = instanceCount;
    paramStruct->firstVertex = firstVertex;
    paramStruct->firstInstance = firstInstance;
}

void CustomCommandBuffer::drawIndexed(uint32_t indexCount,
                     uint32_t instanceCount,
                     uint32_t firstIndex,
                     int32_t vertexOffset,
                     uint32_t firstInstance)
{
    DrawIndexedParams* paramStruct = reinterpret_cast<DrawIndexedParams*>(initCommand(CommandID::CMD_ID_DRAW_INDEXED, 0));
    paramStruct->indexCount = indexCount;
    paramStruct->instanceCount = instanceCount;
    paramStruct->firstIndex = firstIndex;
    paramStruct->vertexOffset = vertexOffset;
    paramStruct->firstInstance = firstInstance;
}

void CustomCommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    CommandHeader *header = initCommand(CommandID::CMD_ID_BIND_INDEX_BUFFER, 0);
    XXXParams* paramStruct = reinterpret_cast<XXXParams*>(reinterpret_cast<char*>(header) + sizeof(CommandHeader));
}
#if 0
void CustomCommandBuffer::pipelineBarrier(VkPipelineStageFlags srcStageMask,
                         VkPipelineStageFlags dstStageMask,
                         VkDependencyFlags dependencyFlags,
                         uint32_t memoryBarrierCount,
                         const VkMemoryBarrier *memoryBarriers,
                         uint32_t bufferMemoryBarrierCount,
                         const VkBufferMemoryBarrier *bufferMemoryBarriers,
                         uint32_t imageMemoryBarrierCount,
                         const VkImageMemoryBarrier *imageMemoryBarriers)
{
    CommandHeader *header = initCommand(CommandID::CMD_ID_BIND_INDEX_BUFFER, 0);
    XXXParams* paramStruct = reinterpret_cast<XXXParams*>(reinterpret_cast<char*>(header) + sizeof(CommandHeader));
}

void CustomCommandBuffer::setEvent(VkEvent event, VkPipelineStageFlags stageMask)
{
    CommandHeader *header = initCommand(CommandID::CMD_ID_BIND_INDEX_BUFFER, 0);
    XXXParams* paramStruct = reinterpret_cast<XXXParams*>(reinterpret_cast<char*>(header) + sizeof(CommandHeader));
}

void CustomCommandBuffer::resetEvent(VkEvent event, VkPipelineStageFlags stageMask)
{
    CommandHeader *header = initCommand(CommandID::CMD_ID_BIND_INDEX_BUFFER, 0);
    XXXParams* paramStruct = reinterpret_cast<XXXParams*>(reinterpret_cast<char*>(header) + sizeof(CommandHeader));
}

void CustomCommandBuffer::waitEvents(uint32_t eventCount,
                    const VkEvent *events,
                    VkPipelineStageFlags srcStageMask,
                    VkPipelineStageFlags dstStageMask,
                    uint32_t memoryBarrierCount,
                    const VkMemoryBarrier *memoryBarriers,
                    uint32_t bufferMemoryBarrierCount,
                    const VkBufferMemoryBarrier *bufferMemoryBarriers,
                    uint32_t imageMemoryBarrierCount,
                    const VkImageMemoryBarrier *imageMemoryBarriers)
{
    CommandHeader *header = initCommand(CommandID::CMD_ID_BIND_INDEX_BUFFER, 0);
    XXXParams* paramStruct = reinterpret_cast<XXXParams*>(reinterpret_cast<char*>(header) + sizeof(CommandHeader));
}

void CustomCommandBuffer::resetQueryPool(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount)
{
    CommandHeader *header = initCommand(CommandID::CMD_ID_BIND_INDEX_BUFFER, 0);
    XXXParams* paramStruct = reinterpret_cast<XXXParams*>(reinterpret_cast<char*>(header) + sizeof(CommandHeader));
}

void CustomCommandBuffer::beginQuery(VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags)
{
    CommandHeader *header = initCommand(CommandID::CMD_ID_BIND_INDEX_BUFFER, 0);
    XXXParams* paramStruct = reinterpret_cast<XXXParams*>(reinterpret_cast<char*>(header) + sizeof(CommandHeader));
}

void CustomCommandBuffer::endQuery(VkQueryPool queryPool, uint32_t query)
{
    CommandHeader *header = initCommand(CommandID::CMD_ID_BIND_INDEX_BUFFER, 0);
    XXXParams* paramStruct = reinterpret_cast<XXXParams*>(reinterpret_cast<char*>(header) + sizeof(CommandHeader));
}

void CustomCommandBuffer::writeTimestamp(VkPipelineStageFlagBits pipelineStage,
                        VkQueryPool queryPool,
                        uint32_t query)
{
    CommandHeader *header = initCommand(CommandID::CMD_ID_BIND_INDEX_BUFFER, 0);
    XXXParams* paramStruct = reinterpret_cast<XXXParams*>(reinterpret_cast<char*>(header) + sizeof(CommandHeader));
}

// Parse the cmds in this cmd buffer into given primary cmd buffer
void CustomCommandBuffer::parse(VkCommandBuffer cmdBuffer)
{
    
}
#endif
}  // namespace vk

// VK_EXT_debug_utils
PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT   = nullptr;
PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = nullptr;

// VK_EXT_debug_report
PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT   = nullptr;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = nullptr;

// VK_KHR_get_physical_device_properties2
PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR = nullptr;

#if defined(ANGLE_PLATFORM_FUCHSIA)
// VK_FUCHSIA_imagepipe_surface
PFN_vkCreateImagePipeSurfaceFUCHSIA vkCreateImagePipeSurfaceFUCHSIA = nullptr;
#endif

#define GET_FUNC(vkName)                                                                   \
    do                                                                                     \
    {                                                                                      \
        vkName = reinterpret_cast<PFN_##vkName>(vkGetInstanceProcAddr(instance, #vkName)); \
        ASSERT(vkName);                                                                    \
    } while (0)

void InitDebugUtilsEXTFunctions(VkInstance instance)
{
    GET_FUNC(vkCreateDebugUtilsMessengerEXT);
    GET_FUNC(vkDestroyDebugUtilsMessengerEXT);
}

void InitDebugReportEXTFunctions(VkInstance instance)
{
    GET_FUNC(vkCreateDebugReportCallbackEXT);
    GET_FUNC(vkDestroyDebugReportCallbackEXT);
}

void InitGetPhysicalDeviceProperties2KHRFunctions(VkInstance instance)
{
    GET_FUNC(vkGetPhysicalDeviceProperties2KHR);
}

#if defined(ANGLE_PLATFORM_FUCHSIA)
void InitImagePipeSurfaceFUCHSIAFunctions(VkInstance instance)
{
    GET_FUNC(vkCreateImagePipeSurfaceFUCHSIA);
}
#endif

#undef GET_FUNC

namespace gl_vk
{

VkFilter GetFilter(const GLenum filter)
{
    switch (filter)
    {
        case GL_LINEAR_MIPMAP_LINEAR:
        case GL_LINEAR_MIPMAP_NEAREST:
        case GL_LINEAR:
            return VK_FILTER_LINEAR;
        case GL_NEAREST_MIPMAP_LINEAR:
        case GL_NEAREST_MIPMAP_NEAREST:
        case GL_NEAREST:
            return VK_FILTER_NEAREST;
        default:
            UNIMPLEMENTED();
            return VK_FILTER_MAX_ENUM;
    }
}

VkSamplerMipmapMode GetSamplerMipmapMode(const GLenum filter)
{
    switch (filter)
    {
        case GL_LINEAR:
        case GL_LINEAR_MIPMAP_LINEAR:
        case GL_NEAREST_MIPMAP_LINEAR:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        case GL_NEAREST:
        case GL_NEAREST_MIPMAP_NEAREST:
        case GL_LINEAR_MIPMAP_NEAREST:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        default:
            UNIMPLEMENTED();
            return VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
    }
}

VkSamplerAddressMode GetSamplerAddressMode(const GLenum wrap)
{
    switch (wrap)
    {
        case GL_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case GL_MIRRORED_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case GL_CLAMP_TO_BORDER:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case GL_CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        default:
            UNIMPLEMENTED();
            return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
    }
}

VkRect2D GetRect(const gl::Rectangle &source)
{
    return {{source.x, source.y},
            {static_cast<uint32_t>(source.width), static_cast<uint32_t>(source.height)}};
}

VkPrimitiveTopology GetPrimitiveTopology(gl::PrimitiveMode mode)
{
    switch (mode)
    {
        case gl::PrimitiveMode::Triangles:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case gl::PrimitiveMode::Points:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case gl::PrimitiveMode::Lines:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case gl::PrimitiveMode::LineStrip:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case gl::PrimitiveMode::TriangleFan:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        case gl::PrimitiveMode::TriangleStrip:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case gl::PrimitiveMode::LineLoop:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        default:
            UNREACHABLE();
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    }
}

VkCullModeFlags GetCullMode(const gl::RasterizerState &rasterState)
{
    if (!rasterState.cullFace)
    {
        return VK_CULL_MODE_NONE;
    }

    switch (rasterState.cullMode)
    {
        case gl::CullFaceMode::Front:
            return VK_CULL_MODE_FRONT_BIT;
        case gl::CullFaceMode::Back:
            return VK_CULL_MODE_BACK_BIT;
        case gl::CullFaceMode::FrontAndBack:
            return VK_CULL_MODE_FRONT_AND_BACK;
        default:
            UNREACHABLE();
            return VK_CULL_MODE_NONE;
    }
}

VkFrontFace GetFrontFace(GLenum frontFace, bool invertCullFace)
{
    // Invert CW and CCW to have the same behavior as OpenGL.
    switch (frontFace)
    {
        case GL_CW:
            return invertCullFace ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
        case GL_CCW:
            return invertCullFace ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
        default:
            UNREACHABLE();
            return VK_FRONT_FACE_CLOCKWISE;
    }
}

VkSampleCountFlagBits GetSamples(GLint sampleCount)
{
    switch (sampleCount)
    {
        case 0:
        case 1:
            return VK_SAMPLE_COUNT_1_BIT;
        case 2:
            return VK_SAMPLE_COUNT_2_BIT;
        case 4:
            return VK_SAMPLE_COUNT_4_BIT;
        case 8:
            return VK_SAMPLE_COUNT_8_BIT;
        case 16:
            return VK_SAMPLE_COUNT_16_BIT;
        case 32:
            return VK_SAMPLE_COUNT_32_BIT;
        default:
            UNREACHABLE();
            return VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
    }
}

VkComponentSwizzle GetSwizzle(const GLenum swizzle)
{
    switch (swizzle)
    {
        case GL_ALPHA:
            return VK_COMPONENT_SWIZZLE_A;
        case GL_RED:
            return VK_COMPONENT_SWIZZLE_R;
        case GL_GREEN:
            return VK_COMPONENT_SWIZZLE_G;
        case GL_BLUE:
            return VK_COMPONENT_SWIZZLE_B;
        case GL_ZERO:
            return VK_COMPONENT_SWIZZLE_ZERO;
        case GL_ONE:
            return VK_COMPONENT_SWIZZLE_ONE;
        default:
            UNREACHABLE();
            return VK_COMPONENT_SWIZZLE_IDENTITY;
    }
}

void GetOffset(const gl::Offset &glOffset, VkOffset3D *vkOffset)
{
    vkOffset->x = glOffset.x;
    vkOffset->y = glOffset.y;
    vkOffset->z = glOffset.z;
}

void GetExtent(const gl::Extents &glExtent, VkExtent3D *vkExtent)
{
    vkExtent->width  = glExtent.width;
    vkExtent->height = glExtent.height;
    vkExtent->depth  = glExtent.depth;
}

VkImageType GetImageType(gl::TextureType textureType)
{
    switch (textureType)
    {
        case gl::TextureType::_2D:
        case gl::TextureType::_2DArray:
        case gl::TextureType::_2DMultisample:
        case gl::TextureType::_2DMultisampleArray:
        case gl::TextureType::CubeMap:
        case gl::TextureType::External:
            return VK_IMAGE_TYPE_2D;
        case gl::TextureType::_3D:
            return VK_IMAGE_TYPE_3D;
        default:
            // We will need to implement all the texture types for ES3+.
            UNIMPLEMENTED();
            return VK_IMAGE_TYPE_MAX_ENUM;
    }
}

VkImageViewType GetImageViewType(gl::TextureType textureType)
{
    switch (textureType)
    {
        case gl::TextureType::_2D:
        case gl::TextureType::_2DMultisample:
        case gl::TextureType::External:
            return VK_IMAGE_VIEW_TYPE_2D;
        case gl::TextureType::_2DArray:
        case gl::TextureType::_2DMultisampleArray:
            return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case gl::TextureType::_3D:
            return VK_IMAGE_VIEW_TYPE_3D;
        case gl::TextureType::CubeMap:
            return VK_IMAGE_VIEW_TYPE_CUBE;
        default:
            // We will need to implement all the texture types for ES3+.
            UNIMPLEMENTED();
            return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    }
}

VkColorComponentFlags GetColorComponentFlags(bool red, bool green, bool blue, bool alpha)
{
    return (red ? VK_COLOR_COMPONENT_R_BIT : 0) | (green ? VK_COLOR_COMPONENT_G_BIT : 0) |
           (blue ? VK_COLOR_COMPONENT_B_BIT : 0) | (alpha ? VK_COLOR_COMPONENT_A_BIT : 0);
}

void GetViewport(const gl::Rectangle &viewport,
                 float nearPlane,
                 float farPlane,
                 bool invertViewport,
                 GLint renderAreaHeight,
                 VkViewport *viewportOut)
{
    viewportOut->x        = static_cast<float>(viewport.x);
    viewportOut->y        = static_cast<float>(viewport.y);
    viewportOut->width    = static_cast<float>(viewport.width);
    viewportOut->height   = static_cast<float>(viewport.height);
    viewportOut->minDepth = gl::clamp01(nearPlane);
    viewportOut->maxDepth = gl::clamp01(farPlane);

    if (invertViewport)
    {
        viewportOut->y      = static_cast<float>(renderAreaHeight - viewport.y);
        viewportOut->height = -viewportOut->height;
    }
}

void GetScissor(const gl::State &glState,
                bool invertViewport,
                const gl::Rectangle &renderArea,
                VkRect2D *scissorOut)
{
    if (glState.isScissorTestEnabled())
    {
        gl::Rectangle clippedRect;
        if (!gl::ClipRectangle(glState.getScissor(), renderArea, &clippedRect))
        {
            memset(scissorOut, 0, sizeof(VkRect2D));
            return;
        }

        *scissorOut = gl_vk::GetRect(clippedRect);

        if (invertViewport)
        {
            scissorOut->offset.y =
                renderArea.height - scissorOut->offset.y - scissorOut->extent.height;
        }
    }
    else
    {
        // If the scissor test isn't enabled, we can simply use a really big scissor that's
        // certainly larger than the current surface using the maximum size of a 2D texture
        // for the width and height.
        *scissorOut = gl_vk::GetRect(kMaxSizedScissor);
    }
}
}  // namespace gl_vk
}  // namespace rx
