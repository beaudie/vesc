#include "libANGLE/renderer/vulkan/CommandVk.h"

namespace rx
{

namespace vk
{

PersistantCommandBuffer::~PersistantCommandBuffer()
{
    ASSERT(!allocated());
}

bool PersistantCommandBuffer::allocated() const
{
    return priv::CommandBuffer::valid() && mPoolAllocateId != kNullPollAllocatedId &&
           mOwnerPool != nullptr;
}

// Never destroy, it only calls the pool to collect this buffer
void PersistantCommandBuffer::releaseHandle()
{
    if (mInflight)
    {
        ASSERT(mOwnerPool);
        ASSERT(mOwnerPool->collect(std::move(*this)) == angle::Result::Continue);
    }
}

PersistantCommandPool::PersistantCommandPool(VkCommandBufferLevel bufferLevel)
    : mAllocatedBufferCount(0), mBufferLevel(bufferLevel), mVkDeviceCopy(VK_NULL_HANDLE)
{}

PersistantCommandPool::~PersistantCommandPool()
{
    ASSERT(mHandle == VK_NULL_HANDLE && mFreeBufferList.empty());
}

VkResult PersistantCommandPool::init(VkDevice device, uint32_t queueFamilyIndex)
{
    ASSERT(!valid());
    mVkDeviceCopy = device;

    // Initialize the command pool now that we know the queue family index.
    VkCommandPoolCreateInfo commandPoolInfo = {};
    commandPoolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolInfo.queueFamilyIndex        = queueFamilyIndex;
    VkResult res = vkCreateCommandPool(device, &commandPoolInfo, nullptr, &mHandle);
    if (res != VK_SUCCESS)
    {
        return res;
    }

    allocateCommandBufferArray(getInitBufferNum());
    return res;
}

void PersistantCommandPool::destroy(VkDevice device)
{
    if (valid())
    {
        ASSERT(mVkDeviceCopy == device);
        for (CommandBufferArrayHandle &bufferArrayHandle : mAllocatedBufferArrays)
        {
            vkFreeCommandBuffers(device, this->getHandle(), bufferArrayHandle.size,
                                 bufferArrayHandle.array);
            delete[] bufferArrayHandle.array;
            bufferArrayHandle.size  = 0;
            bufferArrayHandle.array = nullptr;
        }
        mAllocatedBufferArrays.clear();
        mAllocatedBufferCount = 0;

        vkDestroyCommandPool(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
    for (PersistantCommandBuffer &cmdBuf : mFreeBufferList)
    {
        cmdBuf.destroy(device);
    }
    mFreeBufferList.clear();
}

angle::Result PersistantCommandPool::alloc(PersistantCommandBuffer *bufferOutput)
{
    if (mFreeBufferList.empty())
    {
        allocateBatch();
        ASSERT(!mFreeBufferList.empty());
    }

    *bufferOutput = std::move(mFreeBufferList.front());
    mFreeBufferList.pop_front();

    bufferOutput->mInflight = true;
    return angle::Result::Continue;
}

angle::Result PersistantCommandPool::collect(PersistantCommandBuffer &&buffer)
{
    ASSERT(buffer.mOwnerPool == this);
    ASSERT(buffer.mInflight == true);
    ASSERT(buffer.mPoolAllocateId != PersistantCommandBuffer::kNullPollAllocatedId);
    ASSERT(buffer.mPoolAllocateId < (int32_t)mAllocatedBufferCount);
    ASSERT(buffer.mBufferLevel == this->mBufferLevel);

    buffer.mInflight = false;
    // VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT NOT set, CommandBuffer
    // still hold the memory resource
    vkResetCommandBuffer(buffer.mHandle, 0);

    mFreeBufferList.emplace_front(std::move(buffer));

    return angle::Result::Continue;
}

void PersistantCommandPool::allocateCommandBufferArray(uint32_t count)
{
    VkCommandBuffer *pCmdBuffers = new VkCommandBuffer[count];
    {
        ASSERT(mVkDeviceCopy);
        VkCommandBufferAllocateInfo commandBufferInfo = {};
        commandBufferInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferInfo.commandPool        = this->getHandle();
        commandBufferInfo.level              = mBufferLevel;
        commandBufferInfo.commandBufferCount = count;
        ASSERT(vkAllocateCommandBuffers(mVkDeviceCopy, &commandBufferInfo, pCmdBuffers) ==
               VK_SUCCESS);

        mAllocatedBufferArrays.emplace_back(CommandBufferArrayHandle{
            .array = pCmdBuffers,
            .size  = count,
        });
    }

    for (uint32_t i = 0; i < count; i++)
    {
        uint32_t id = mAllocatedBufferCount + i;
        mFreeBufferList.emplace_back(
            PersistantCommandBuffer(this, mBufferLevel, id, pCmdBuffers[i]));
    }

    mAllocatedBufferCount += count;
    // If we allocate too many, it mostly like would be a bug
    ASSERT(mAllocatedBufferCount < 100);
}

}  // namespace vk

}  // namespace rx