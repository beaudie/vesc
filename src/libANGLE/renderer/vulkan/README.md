# ANGLE: Vulkan Back-end

ANGLE's Vulkan back-end implementation lives in this folder.

[Vulkan][Vulkan] is an explicit graphics API. It has a lot in common with other explicit APIs such
as Microsoft's [D3D12][D3D12 Guide] and Apple's [Metal][Metal]. Compared to APIs like OpenGL or
D3D11, explicit APIs can offer a number of significant benefits:

 * Lower API call CPU overhead.
 * A smaller API surface with more direct hardware control.
 * Better support for multi-core programming.
 * Vulkan in particular has open-source tooling and tests.

[Vulkan]: https://www.khronos.org/vulkan/
[D3D12 Guide]: https://docs.microsoft.com/en-us/windows/desktop/direct3d12/directx-12-programming-guide
[Metal]: https://developer.apple.com/metal/

[TOC]

## Back-end Design

The [RendererVk](RendererVk.cpp) is a singleton. RendererVk owns shared global resources like the
[VkDevice][VkDevice], [VkQueue][VkQueue], the [Vulkan format tables](vk_format_utils.h) and
[internal Vulkan shaders](shaders). The back-end creates a new [ContextVk](ContextVk.cpp) instance
to manage each allocated OpenGL Context. ContextVk processes state changes and handles action
commands like `glDrawArrays` and `glDrawElements`.

### Fast OpenGL State Transitions

Typical OpenGL programs issue a few small state change commands between draw call commands. We want
the typical app's use case to be as fast as possible so this leads to unique performance challenges.

Vulkan in quite different from OpenGL because it requires a separate compiled
[VkPipeline][VkPipeline] for each state vector. Compiling VkPipelines is multiple orders of
magnitude slower than enabling or disabling an OpenGL render state. To speed this up we use three
levels of caching when transitioning states in the Vulkan back-end.

The first level is the driver's [VkPipelineCache][VkPipelineCache]. The driver
cache reduces pipeline recompilation time significantly. But even cached
pipeline recompilations are orders of manitude slower than OpenGL state changes.

The second level cache is an ANGLE-owned hash map from OpenGL state vectors to compiled pipelines.
See [GraphicsPipelineCache][GraphicsPipelineCache] in [vk_cache_utils.h](vk_cache_utils.h). ANGLE's
[GraphicsPipelineDesc][GraphicsPipelineDesc] class is a tightly packed 256-byte description of the
current OpenGL rendering state. We also use a [xxHash](https://github.com/Cyan4973/xxHash) for the
fastest possible hash computation. The hash map speeds up state changes considerably. But it is
still significantly slower than OpenGL implementations.

To get best performance we use a transition table from each OpenGL state vector to neighbouring
state vectors. The transition table points from GraphicsPipelineCache entries directly to
neighbouring VkPipeline objects. When the application changes state the state change bits are
recorded into a compact bit mask that covers the GraphicsPipelineDesc state vector. Then on the next
draw call we scan the transition bit mask and compare the GraphicsPipelineDesc of the current state
vector and the state vector of the cached transition. With the hash map we compute a hash over the
entire state vector and then do a 256-byte `memcmp` to guard against hash collisions. With the
transition table we will only compare as many bytes as were changed in the transition bit mask. By
skipping the expensive hashing and `memcmp` we can get as good or faster performance than native
OpenGL drivers.

Note that the current design of the transition table stores transitions in an unsorted list. If
applications map from one state to many this will slow down the transition time. This could be
improved in the future using a faster look up. For instance we could keep a sorted transition table
or use a small hash map for transitions.

[VkDevice]: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkDevice.html
[VkQueue]: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkQueue.html
[VkPipeline]: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPipeline.html
[VkPipelineCache]: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPipelineCache.html
[GraphicsPipelineCache]: https://chromium.googlesource.com/angle/angle/+/225f08bf85a368f905362cdd1366e4795680452c/src/libANGLE/renderer/vulkan/vk_cache_utils.h#498
[GraphicsPipelineDesc]: https://chromium.googlesource.com/angle/angle/+/225f08bf85a368f905362cdd1366e4795680452c/src/libANGLE/renderer/vulkan/vk_cache_utils.h#244

### Shader Module Compilation

ANGLE converts application shaders into Vulkan [VkShaderModules][VkShaderModule] through a series
of steps:

1. **ANGLE Internal Translation**: The initial calls to `glCompileShader` are passed to the [ANGLE
shader translator][translator]. The translator compiles application shaders into Vulkan-compatible
GLSL. Vulkan-compatible GLSL matches the [GL_KHR_vulkan_glsl][GL_KHR_vulkan_glsl] extension spec
with some additional workarounds and emulation. We emulate OpenGL's different depth range, viewport
y flipping, default uniforms, and OpenGL [line segment
rasterization](#opengl-line-segment-rasterization). For more info see
[TranslatorVulkan.cpp][TranslatorVulkan.cpp]. After initial compilation the shaders are not
complete. They are templated with markers that are filled in later at link time.

1. **Link-Time Translation**: During a call to `glLinkProgram` the Vulkan back-end can know the
necessary locations and properties to write to connect the shader stage interfaces. We get the
completed shader source using ANGLE's [GlslangWrapper][GlslangWrapper.cpp] helper class. We still
cannot generate `VkShaderModules` since some ANGLE features like [OpenGL line
rasterization](#opengl-line-segment-rasterization) emulation depend on draw-time information.

1. **Draw-time SPIR-V Generation**: Once the application records a draw call we use Khronos'
[glslang][glslang] to convert the Vulkan-compatible GLSL into SPIR-V with the correct draw-time
defines. The SPIR-V is then compiled into `VkShaderModules`. For details please see
[GlslangWrapper.cpp][GlslangWrapper.cpp]. The `VkShaderModules` are then used by `VkPipelines`. Note
that we currently don't use [SPIRV-Tools][SPIRV-Tools] to perform any SPIR-V optimization. This
could be something to improve on in the future.

See the below diagram for a high-level view of the shader translation flow:

<!-- Generated from https://bramp.github.io/js-sequence-diagrams/
participant App
participant "ANGLE Front-end"
participant "Vulkan Back-end"
participant "ANGLE Translator"
participant "GlslangWrapper"
participant "Glslang"

App->"ANGLE Front-end": glCompileShader (VS)
"ANGLE Front-end"->"Vulkan Back-end": ShaderVk::compile
"Vulkan Back-end"->"ANGLE Translator": sh::Compile
"ANGLE Translator"- ->"ANGLE Front-end": return Vulkan-compatible GLSL

Note right of "ANGLE Front-end": Source is templated\nwith markers to be\nfilled at link time.

Note right of App: Same for FS, GS, etc...

App->"ANGLE Front-end": glCreateProgram (...)
App->"ANGLE Front-end": glAttachShader (...)
App->"ANGLE Front-end": glLinkProgram
"ANGLE Front-end"->"Vulkan Back-end": ProgramVk::link

Note right of "Vulkan Back-end": ProgramVk inits uniforms,\nlayouts, and descriptors.

"Vulkan Back-end"->GlslangWrapper: GlslangWrapper::GetShaderSource
GlslangWrapper- ->"Vulkan Back-end": return filled-in sources

Note right of "Vulkan Back-end": Source is templated with\ndefines to be resolved at\ndraw time.

"Vulkan Back-end"- ->"ANGLE Front-end": return success

Note right of App: App execution continues...

App->"ANGLE Front-end": glDrawArrays (any draw)
"ANGLE Front-end"->"Vulkan Back-end": ContextVk::drawArrays

"Vulkan Back-end"->GlslangWrapper: GlslangWrapper::GetShaderCode (with defines)
GlslangWrapper->Glslang: GlslangToSpv
Glslang- ->"Vulkan Back-end": Return SPIR-V

Note right of "Vulkan Back-end": We init VkShaderModules\nand VkPipeline then\nrecord the draw.

"Vulkan Back-end"- ->"ANGLE Front-end": return success
-->

![Vulkan Shader Translation Flow](https://raw.githubusercontent.com/google/angle/master/src/libANGLE/renderer/vulkan/doc/img/VulkanShaderTranslation.svg?sanitize=true)

[VkShaderModule]: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkShaderModule.html
[translator]: https://chromium.googlesource.com/angle/angle/+/refs/heads/master/src/compiler/translator/
[GL_KHR_vulkan_glsl]: https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_vulkan_glsl.txt
[TranslatorVulkan.cpp]: https://chromium.googlesource.com/angle/angle/+/refs/heads/master/src/compiler/translator/TranslatorVulkan.cpp
[glslang]: https://github.com/KhronosGroup/glslang
[GlslangWrapper.cpp]: https://chromium.googlesource.com/angle/angle/+/refs/heads/master/src/libANGLE/renderer/vulkan/GlslangWrapper.cpp
[SPIRV-Tools]: https://github.com/KhronosGroup/SPIRV-Tools

### Command Graph Design

The Vulkan back-end supports deferred command execution. Deferred recording allows ANGLE to
implement command reordering, automatic RenderPass load/store ops and implicit layout transitions.
We implement this deferred command execution by recording commands into secondary command buffers.
We enforce ordering on the commands via public dependency relations and special barrier nodes. For
example: a draw call reads from a Texture and writes to a Framebuffer and its attachments. The
Framebuffer will have a dependency on the Texture as well as its attachments. Thus commands
operating on the Texture or attachments must execute before the draw calls on the Framebuffer.
As another example, query begin/end create special graph nodes that ensure commands are not
reordered around them.

Internally ANGLE uses a directed acyclic graph to enforce the ordering on command buffers. Each node
of the command graph must execute after any connected parent nodes. Each node stores deferred Vulkan
commands. Nodes also store additional parameters needed to begin a RenderPass.

OpenGL objects that record command buffers inherit from `CommandGraphResource`. That is the only
class that can create command buffers and record commands. `CommandGraphResource` APIs also enforce
ordering between commands via dependencies. They are not themselves nodes in the command graph but
each holds a reference to the graph node it is currently using to record commands.

#### Classes that Implement CommandGraphResource

- `ImageHelper` (owns vk::Image and memory)
- `BufferHelper` (owns vk::Buffer and memory)
- `FramebufferHelper` (owns vk::Framebuffer)

#### CommandGraphResource Internal API

Starting Vulkan command buffers can only be done inside a `CommandGraphResource`. Vulkan commands
either work inside or outside a RenderPass instance. RenderPasses are necessary for commands used
with a Framebuffer, such as draw calls or Framebuffer-based clears. RenderPasses also require more
state.  The `CommandGraphResource` APIs are divided between RenderPass and Writing (outside a
RenderPass) commands.  RenderPass commands are also divided between “begin” or “append” APIs.

##### Recording Commands Outside a RenderPass

```
angle::Result recordCommands(ContextVk *contextVk, CommandBuffer **commandBufferOut);
Serial getStoredQueueSerial() const;
```

The `recordCommands` API returns a started command buffer for recording new commands. If possible we
will return an existing command buffer for performance. For example, we can combine multiple layout
transitions, copies, dispatch calls etc using the same dependencies into a single command buffer.

The `getStoredQueueSerial` API is used when enqueing Vulkan objects into garbage collection.
Occasionally, it is used to know if the resource has pending operations that may necessitate a
flush, such as with mapping a buffer.

##### RenderPass APIs

```
angle::Result beginRenderPass(ContextVk *contextVk,
                              const Framebuffer &framebuffer,
                              const gl::Rectangle &renderArea,
                              const RenderPassDesc &renderPassDesc,
                              const AttachmentOpsArray &renderPassAttachmentOps,
                              const std::vector<VkClearValue> &clearValues,
                              CommandBuffer **commandBufferOut);
bool hasStartedFramebufferRenderPass(const Framebuffer &framebuffer) const;
bool appendToStartedRenderPass(Serial currentQueueSerial,
                               const Framebuffer &framebuffer,
                               const gl::Rectangle &renderArea,
                               CommandBuffer **commandBufferOut);
```

The `beginRenderPass` API is special in that it includes a bunch of extra state information that is
necessary for starting a RenderPass. `hasStartedFramebufferRenderPass` tests to see if the current
node of a Framebuffer has a RenderPass started for this framebuffer. In that case,
`appendToStartedRenderPass` can be used to append more commands to the same RenderPass.
`FramebufferVk::appendOrStartRenderPass` nicely encapsulates the necessary logic and returns a
command buffer for the framebuffer to issue draw calls to.

#### CommandGraphResource External API

Each `CommandGraphResource` keeps track of the last graph node with a command buffer that reads from
or writes to it. When a `CommandGraphResource` records commands, it will opportunistically try to
append commands to that node's command buffer. That may not be possible if a dependent command
buffer in another node is using the resource. To maintain the correct ordering of operations, the
resources create dependencies, which effectively creates an edge between their current nodes.

```
void addDependency(ContextVk *contextVk, CommandGraphResource *used);
void addDependencyAndMemoryBarrier(ContextVk *contextVk,
                                   CommandGraphResource *used,
                                   VkFlags srcAccess,
                                   VkFlags dstAccess);
```

The `addDependency` function is called on the resource that's about to record commands to ensure
what's already recorded for `used` executes beforehand. An example usage is:

```
A.recordCommands(&commandBufferA)
renderA(commandBufferA)

B.addDependency(A)

B.recordCommands(&commandBufferB)
renderB(commandBufferB)
```

In this example, what's recorded in `commandBufferA` executes before what's recorded in
`commandBufferB`. Given this property, the caller can reduce the number of secondary command buffers
by equivalently recording as such:

```
B.addDependency(A)

B.recordCommands(&commandBufferB)
renderA(commandBufferB)
renderB(commandBufferB)
```

The `addDependencyAndMemoryBarrier` is a special-cased `addDependency` that includes a memory
barrier. This is used to terminate the current command buffer and create a new node if the memory
barrier cannot be aggregated in to the current node.

#### Implementation

Vulkan image and memory barriers are created based on their usage in the GLES command timeline. This
means that if a resource is read from through two independent paths, the order of execution of the
Vulkan command buffers of those paths must necessarily be the same as specified by the GLES
commands. For example, if an image is in layout A, the first reader requires it to be in layout B
and the second reader in layout C, then the barriers issued in the Vulkan command buffers are `A->B`
in the first command buffer and `B->C` in the second. The two command buffers cannot therefore be
executed out of order.

This property allows us to restrict every node to a single child. Every `addDependency` would first
descend to the last descendant of the resource's current node, and creates the dependency there.
While the graph is being generated, the graph basically forms a forest of [disjoint
sets][DisjointSet], with each set's descendant node being the representative of the set.

```
A   B   C   D
 \ /     \ /
  E   F   G
```

In the above, each letter represents a graph node, and the edge directions are from top to bottom.
Creating a dependency between nodes of the same set is a no-op (for example between `A` and `B` or
`C` and `G`), because they would both append their commands to `E` and the order is naturally
maintained. Creating a dependency between two nodes of different sets creates an edge between the
descendants. For example, creating an edge from `F` to `E` (`ownerOfF.addDependency(ownerOfE)`, and
subsequently from a new node `H` to both `A` and `D` would result in the following graph:

```
A   B
 \ /
  E   C   D
   \   \ /
    F   G
     \ /
      H
```

[DisjointSet]: https://en.wikipedia.org/wiki/Disjoint-set_data_structure

##### Command Graph Barrier Nodes

A certain number of GLES commands imply an ordering between commands, but don't necessarily
associate with any resource. More importantly, they don't create a dependency between command
buffers that have already been recorded. Examples of such commands include query begin/end, debug
marker push/pop, sync fence set/wait etc. A special node is created for such an operation that
doesn't record any commands. These "command graph barrier nodes" set dependencies between nodes
before and after them such that the commands are not reordered around them, but they don't
participate in the disjoint set data structure. This allows disjoint sets to remain disjoint and
avoids serializing every command after the first barrier node.

##### RenderPass loadOp and storeOp

When a Framebuffer starts a RenderPass, the RenderPass loadOp and storeOp parameters are set.
However, as long as no command is recorded in the RenderPass, these values are allowed to be
modified. For example, a `glClearBuffer` may start a RenderPass where loadOp for the corresponding
draw buffer is set to `VK_ATTACHMENT_LOAD_OP_CLEAR` while the rest of the draw buffers have
`VK_ATTACHMENT_LOAD_OP_LOAD`. A subsequent `glClearBuffer` call would simply update the loadOp of
another draw buffer instead of restarting the RenderPass.

##### Graph Debugging

The command graph supports outputting a diagram using [the DOT format][DOT]. This is done through
setting `kEnableCommandGraphDiagnostics` to `true` in ContextVk.cpp. Currently, there is no runtime
flag to enable this.

If the `angle_enable_custom_vulkan_cmd_buffers` build flag is set (which is the default), a summary
of the commands recorded in each node is also output in the diagram.

[DOT]: https://en.wikipedia.org/wiki/DOT_(graph_description_language)

### OpenGL Line Segment Rasterization

OpenGL and Vulkan both render line segments as a series of pixels between two points. They differ in
which pixels cover the line.

For single sample rendering Vulkan uses an algorithm based on quad coverage. A small shape is
extruded around the line segment. Samples covered by the shape then represent the line segment. See
[the Vulkan spec][VulkanLineRaster] for more details.

OpenGL's algorithm is based on [Bresenham's line algorithm][Bresenham]. Bresenham's algorithm
selects pixels on the line between the two segment points. Note Bresenham's does not support
multisampling. When compared visually you can see the Vulkan line segment rasterization algorithm
always selects a superset of the line segment pixels rasterized in OpenGL. See this example:

![Vulkan vs OpenGL Line Rasterization][VulkanVsGLLineRaster]

The OpenGL spec defines a "diamond-exit" rule to select fragments on a line. Please refer to the 2.0
spec section 3.4.1 "Basic Line Segment Rasterization" spec for more details. To implement this rule
we inject a small computation to test if a pixel falls within the diamond in the start of the pixel
shader. If the pixel fails the diamond test we discard the fragment. Note that we only perform this
test when drawing lines. See the section on [Shader Compilation](#shader-module-compilation) for
more info. See the below diagram for an illustration of the diamond rule:

![OpenGL Diamond Rule Example][DiamondRule]

We can implement the OpenGL test by checking the intersection of the line and the medial axes of the
pixel `p`. If the length of the line segment between intersections `p` and the point center is
greater than a half-pixel for all possible `p` then the pixel is not on the segment. To solve for
`p` we use the pixel center `a` given by `gl_FragCoord` and the projection of `a` onto the line
segment `b` given by the interpolated `gl_Position`. Since `gl_Position` is not available in the
fragment shader we must add an internal position varying when drawing lines.

The full code derivation is omitted for brevity. It reduces to the following shader snippet:

```vec2 position = PositionVarying.xy / PositionVarying.w;
vec2 b = ((position * 0.5) + 0.5) * gl_Viewport.zw + gl_Viewport.xy;
vec2 ba = abs(b - gl_FragCoord.xy);
vec2 ba2 = 2.0 * (ba * ba);
vec2 bp = ba2 + ba2.yx - ba;
if (bp.x > epsilon && bp.y > epsilon)
    discard;
```

Note that we must also pass the viewport size as an internal uniform. We use a small epsilon value
to correct for cases when the line segment is perfectly parallel or perpendicular to the window. For
code please see [TranslatorVulkan.cpp][TranslatorVulkan.cpp] under
`AddLineSegmentRasterizationEmulation`.

[VulkanLineRaster]: https://www.khronos.org/registry/vulkan/specs/1.1/html/chap24.html#primsrast-lines-basic
[Bresenham]: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
[VulkanVsGLLineRaster]: doc/img/LineRasterComparison.gif
[DiamondRule]: doc/img/LineRasterPixelExample.png

### Format Tables and Format Emulation

#### Overrides and Fallbacks

The [required Vulkan format support][VulkanRequiredSupport] tables do not implement the full set of
formats needed for OpenGL conformance with extensions. ANGLE emulates missing formats using *format
overrides* and *format fallbacks*.

An *override* implements a missing GL format with a required format in all cases. For example, the
luminance texture format `L8_UNORM` does not exist in Vulkan. We override `L8_UNORM` with the
required image format `R8_UNORM`.

A *fallback* is one or more non-required formats ANGLE checks for support at runtime. For example,
`R8_UNORM` is not a required vertex buffer format. Some drivers do support `R8_UNORM` for vertex
buffers. So at runtime we check for sampled image support and fall back to `R32_FLOAT` if `R8_UNORM`
is not supported.

#### The Vulkan Format Table

Overrides and fallbacks are implemented in ANGLE's [Vulkan format
table][vk_format_table_autogen.cpp]. The table is auto-generated by
[`gen_vk_format_table.py`](gen_vk_format_table.py). We store the mapping from
[`angle::Format::ID`](../FormatID_autogen.h) to [VkFormat][VkFormat] in
[`vk_format_map.json`](vk_format_map.json). The format map also lists the overrides and fallbacks.
To update the tables please modify the format map JSON and then run
[`scripts/run_code_generation.py`][RunCodeGeneration].

The [`vk::Format`](vk_format_utils.h) class describes the information ANGLE needs for a particular
`VkFormat`. The 'ANGLE' format ID is a reference to the front-end format. The 'Image' or 'Buffer'
format are the native Vulkan formats that implement a particular front-end format for `VkImages` and
`VkBuffers`. For the above example of `R8_UNORM` overriding `L8_UNORM`, `L8_UNORM` is the ANGLE
format and `R8_UNORM` is the Image format.

For more information please see the source files.

[VulkanRequiredSupport]: https://renderdoc.org/vkspec_chunked/chap37.html#features-required-format-support
[VkFormat]: https://renderdoc.org/vkspec_chunked/chap37.html#VkFormat
[RunCodeGeneration]: ../../../../scripts/run_code_generation.py
