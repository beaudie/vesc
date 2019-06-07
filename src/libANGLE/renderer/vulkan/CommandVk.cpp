#include "libANGLE/renderer/vulkan/CommandVk.h"

namespace rx
{

namespace vk
{

PersistantCommandBuffer::~PersistantCommandBuffer()
{
    ASSERT(!valid());
}

bool PersistantCommandBuffer::valid() const
{
    return priv::CommandBuffer::valid() && mPoolAllocateId != NULL_POOL_ALLOCATED_ID &&
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

PersistantCommandPool::~PersistantCommandPool()
{
    ASSERT(mHandle == VK_NULL_HANDLE && mFreeBufferList.empty());
}

void PersistantCommandPool::destroy(VkDevice device)
{
    if (valid())
    {
        vkFreeCommandBuffers(device, this->getHandle(), MAX_BUFFERS, mCmdBuffers);
        vkDestroyCommandPool(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
    for (PersistantCommandBuffer &cmdBuf : mFreeBufferList)
    {
        cmdBuf.destroy(device);
    }
    mFreeBufferList.clear();
}

PersistantCommandPool::PersistantCommandPool(VkCommandBufferLevel bufferLevel)
    : mBufferLevel(bufferLevel)
{}

VkResult PersistantCommandPool::init(VkDevice device, const VkCommandPoolCreateInfo &createInfo)
{
    ASSERT(!valid());
    VkResult res = vkCreateCommandPool(device, &createInfo, nullptr, &mHandle);
    if (res != VK_SUCCESS)
    {
        return res;
    }

    VkCommandBufferAllocateInfo commandBufferInfo = {};
    commandBufferInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferInfo.commandPool                 = this->getHandle();
    commandBufferInfo.level                       = mBufferLevel;
    commandBufferInfo.commandBufferCount          = MAX_BUFFERS;
    vkAllocateCommandBuffers(device, &commandBufferInfo, mCmdBuffers);

    for (uint32_t i = 0; i < MAX_BUFFERS; i++)
    {
        mFreeBufferList.emplace_back(
            PersistantCommandBuffer(this, mBufferLevel, i, mCmdBuffers[i]));
    }

    return res;
}

angle::Result PersistantCommandPool::alloc(PersistantCommandBuffer &buffer)
{
    ASSERT(!mFreeBufferList.empty());

    buffer = std::move(mFreeBufferList.front());
    mFreeBufferList.pop_front();

    buffer.mInflight = true;
    return angle::Result::Continue;
}

angle::Result PersistantCommandPool::collect(PersistantCommandBuffer &&buffer)
{
    ASSERT(buffer.mOwnerPool == this);
    ASSERT(buffer.mInflight == true);
    ASSERT(buffer.mPoolAllocateId != PersistantCommandBuffer::NULL_POOL_ALLOCATED_ID);
    ASSERT(buffer.mPoolAllocateId < (int32_t)MAX_BUFFERS);
    ASSERT(buffer.mBufferLevel == this->mBufferLevel);

    buffer.mInflight = false;

    mFreeBufferList.emplace_front(std::move(buffer));

    return angle::Result::Continue;
}

}  // namespace vk

}  // namespace rx