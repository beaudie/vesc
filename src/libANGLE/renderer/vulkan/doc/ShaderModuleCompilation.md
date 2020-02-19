# Shader Module Compilation

ANGLE converts application shaders into Vulkan [VkShaderModules][VkShaderModule] through a series
of steps:

1. **ANGLE Internal Translation**: The initial calls to `glCompileShader` are passed to the [ANGLE
shader translator][translator]. The translator compiles application shaders into Vulkan-compatible
GLSL. Vulkan-compatible GLSL matches the [GL_KHR_vulkan_glsl][GL_KHR_vulkan_glsl] extension spec
with some additional workarounds and emulation. We emulate OpenGL's different depth range, viewport
y flipping, default uniforms, and OpenGL
[line segment rasterization](OpenGLLineSegmentRasterization.md). For more info see
[TranslatorVulkan.cpp][TranslatorVulkan.cpp]. After initial compilation the shaders are not
complete. On translation output, resources and in/out variables are given arbitrary descriptor set,
binding and location indices. The correct values are determined at link time. For the sake of
transform feedback, some markers are left in the shader for link-time substitution.

  Specialization code like [OpenGL line rasterization](OpenGLLineSegmentRasterization.md) is
translated conditioned to Vulkan specialization constants.

1. **Link-Time Compilation and Transformation**: During a call to `glLinkProgram` the Vulkan
back-end can know the necessary locations and properties to write to connect the shader stage
interfaces. We get the completed shader source using ANGLE's
[GlslangWrapperVk][GlslangWrapperVk.cpp] helper class. At this time, we use Khronos'
[glslang][glslang] to convert the Vulkan-compatible GLSL into SPIR-V, then make a transformation
pass on the SPIR-V to update the descriptor set, binding, location indices as well as add component
and various transform feedback decorations and take out inactive varyings from the shader interface.
We currently don't generate `VkShaderModules` at this time, but that could be a future
optimization.

1. **Draw-time Pipeline Creation**: Once the application records a draw call, the SPIR-V is compiled
into `VkShaderModule`s. We can create a Vulkan pipeline with the appropriate specialization
constant values.  For details please see [GlslangWrapperVk.cpp][GlslangWrapperVk.cpp].
The `VkShaderModule`s are then used to create a `VkPipeline` with the appropriate specialization
constants determined at this time. Note that we currently don't use [SPIRV-Tools][SPIRV-Tools] to
perform any SPIR-V optimization. This could be something to improve on in the future.

See the below diagram for a high-level view of the shader translation flow:

<!-- Generated from https://bramp.github.io/js-sequence-diagrams/
participant App
participant "ANGLE Front-end"
participant "Vulkan Back-end"
participant "ANGLE Translator"
participant "GlslangWrapperVk"
participant "Glslang"

App->"ANGLE Front-end": glCompileShader (VS)
"ANGLE Front-end"->"Vulkan Back-end": ShaderVk::compile
"Vulkan Back-end"->"ANGLE Translator": sh::Compile
"ANGLE Translator"->"ANGLE Front-end": return Vulkan-compatible GLSL

Note right of "ANGLE Front-end": Source is using bogus\nVulkan qualifiers to be\ncorrected at link time.

Note right of App: Same for FS, GS, etc...

App->"ANGLE Front-end": glCreateProgram (...)
App->"ANGLE Front-end": glAttachShader (...)
App->"ANGLE Front-end": glLinkProgram
"ANGLE Front-end"->"Vulkan Back-end": ProgramVk::link

Note right of "Vulkan Back-end": ProgramVk inits uniforms,\nlayouts, and descriptors.

"Vulkan Back-end"->GlslangWrapperVk: GlslangWrapperVk::GetShaderSpirvCode
GlslangWrapperVk->Glslang: GlslangToSpv
Glslang->GlslangWrapperVk: Return SPIR-V
GlslangWrapperVk->"Vulkan Back-end": return SPIR-V and patches
"Vulkan Back-end"->"ANGLE Front-end": return success

Note right of App: App execution continues...

App->"ANGLE Front-end": glDrawArrays (any draw)
"ANGLE Front-end"->"Vulkan Back-end": ContextVk::drawArrays

Note right of "Vulkan Back-end": We init VkShaderModules\nand VkPipeline then\nrecord the draw.

"Vulkan Back-end"->"ANGLE Front-end": return success
-->

![Vulkan Shader Translation Flow](https://raw.githubusercontent.com/google/angle/master/src/libANGLE/renderer/vulkan/doc/img/VulkanShaderTranslation.svg?sanitize=true)

[GL_KHR_vulkan_glsl]: https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_vulkan_glsl.txt
[glslang]: https://github.com/KhronosGroup/glslang
[GlslangWrapperVk.cpp]: https://chromium.googlesource.com/angle/angle/+/refs/heads/master/src/libANGLE/renderer/vulkan/GlslangWrapperVk.cpp
[SPIRV-Tools]: https://github.com/KhronosGroup/SPIRV-Tools
[translator]: https://chromium.googlesource.com/angle/angle/+/refs/heads/master/src/compiler/translator/
[TranslatorVulkan.cpp]: https://chromium.googlesource.com/angle/angle/+/refs/heads/master/src/compiler/translator/TranslatorVulkan.cpp
[VkShaderModule]: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkShaderModule.html
