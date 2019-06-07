#ifndef LIBANGLE_RENDERER_VULKAN_COMMANDVK_H_
#define LIBANGLE_RENDERER_VULKAN_COMMANDVK_H_

#include <list>

#include "libANGLE/renderer/vulkan/vk_wrapper.h"

namespace rx
{

namespace vk
{

class PersistantCommandPool;

class PersistantCommandBuffer final : public priv::CommandBuffer
{
    friend class PersistantCommandPool;

  public:
    PersistantCommandBuffer()
        : priv::CommandBuffer(),
          mInflight(false),
          mPoolAllocateId(NULL_POOL_ALLOCATED_ID),
          mBufferLevel(VK_COMMAND_BUFFER_LEVEL_MAX_ENUM),
          mOwnerPool(nullptr)
    {}
    virtual ~PersistantCommandBuffer();

    // Only works to initialize empty objects
    PersistantCommandBuffer &operator=(PersistantCommandBuffer &&other)
    {
        ASSERT(!valid());
        std::swap(mHandle, other.mHandle);
        std::swap(mInflight, other.mInflight);
        std::swap(mPoolAllocateId, other.mPoolAllocateId);
        std::swap(mBufferLevel, other.mBufferLevel);
        std::swap(mOwnerPool, other.mOwnerPool);
        return *this;
    }
    PersistantCommandBuffer(PersistantCommandBuffer &&other) { *this = std::move(other); }

    virtual bool valid() const;

    void releaseHandle();

    // TODO: make base class's destroy() private

  private:
    PersistantCommandBuffer(PersistantCommandPool *pPool,
                            VkCommandBufferLevel bufLevel,
                            int32_t allocateId,
                            const VkCommandBuffer &vkBufRef)
        : priv::CommandBuffer(),
          mInflight(false),
          mPoolAllocateId(allocateId),
          mBufferLevel(bufLevel),
          mOwnerPool(pPool)
    {
        mHandle = vkBufRef;
    }

    bool mInflight;

    static const int NULL_POOL_ALLOCATED_ID = -1;
    int32_t mPoolAllocateId;

    VkCommandBufferLevel mBufferLevel;
    PersistantCommandPool *mOwnerPool;
};

class PersistantCommandPool final : WrappedObject<PersistantCommandPool, VkCommandPool>
{
  public:
    PersistantCommandPool(VkCommandBufferLevel bufferLevel);
    ~PersistantCommandPool();

    void destroy(VkDevice device);
    VkResult init(VkDevice device, const VkCommandPoolCreateInfo &createInfo);

    // TODO: reset

    // XXX: There may be some approch to provide two types of
    // CommandPool & CommandBuffer based on their level with template
    // so that a compile time check is possible
    angle::Result alloc(PersistantCommandBuffer &buffer);
    angle::Result collect(PersistantCommandBuffer &&buffer);

  private:
    // TODO: auto increase buffer
    static const uint32_t MAX_BUFFERS = 50;
    VkCommandBuffer mCmdBuffers[MAX_BUFFERS];

    std::list<PersistantCommandBuffer> mFreeBufferList;

    const VkCommandBufferLevel mBufferLevel;
};

}  // namespace vk
}  // namespace rx

#endif