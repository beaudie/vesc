# ANGLE on Vulkan

[Vulkan](https://www.khronos.org/vulkan/) is an explicit graphics API. It borrows a lot from similar API's like Microsoft's [D3D12](https://docs.microsoft.com/en-us/windows/desktop/direct3d12/directx-12-programming-guide) and Apple's [Metal](https://developer.apple.com/metal/). Compared to the prior generation of APIs like OpenGL or D3D11, Vulkan can offer a number of advantages:

 * Lower CPU overhead.
 * A smaller and more powerful API.
 * More direct control of the GPU.
 * Open-source tools and tests.

## Back-end Design

The [RendererVk](RendererVk.cpp) class owns global resources like the VkDevice, VkQueue, format tables and internal shaders. Each OpenGL Context is backed by a [ContextVk](ContextVk.cpp). The ContextVk manages front-end state changes and records action commands like draw calls.

### State Changes

OpenGL is a more stateful API than Vulkan. In a Vulkan program each possible render state is compiled with a shader into a [VkPipeline](TODO). We want typical OpenGL programs to be as fast as possible when they change a small number of states. For instance, rendering with the depth test enabled then disabled, changing blending modes, or changing the vertex input formats.

Recompiling a VkPipeline each state change would be prohibitively expensive. We use a two-level cache to optimize state change performance as well as the Vulkan driver's [VkPipelineCache](TODO). The first level cache is a hash map from pipeline state descriptions to compiled pipelines to accelerate query of state changes. We also crunch the [pipeline description](TODO) into 256 bytes and use a [specialized non-cyrptographic hash function](TODO) to optimize the hash query.

The second level cache is a transition table from Pipeline Cache entry to neighbouring state changes. Every state change the application makes is recorded into a compact bit mask over the entire pipeline description. When retrieving a pipeline from the cache we scan the transition table for matching pipelines. This allows us to skip the more expensive hashing and comparison step for small state changes.
