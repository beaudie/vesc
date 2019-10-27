# Transform Feedback via extension

## Outline

Angle driver already has the feature of TransformFeedback. It emulates transform feedback functionalities using vertexPipelineStoresAndAtomics feature in Vulkan.

But some GPU vendor does not support this feature, So we need another implementation using VK_EXT_transform_feedback.

We also expect performance gain when we use this extension.

## Implement Pause/Resume using CounterBuffer

Vulkan extension does not provide separate APIs for `glPauseTransformFeedback`/`glEndTransformFeedback` . Instead, Vulkan introduced Counter buffer in `vkCmdBeginTransformFeedbackEXT` /`vkCmdEndTransformFeedbackEXT` as API parameters.  To pause use `vkCmdEndTransformFeedbackEXT` and provide valid buffer handles in the `pCounterBuffers` array and offsets in the `pCounterBufferOffsets` array for the implementation to save the resume points. Then to resume use `vkCmdBeginTransformFeedbackEXT` with the previous `pCounterBuffers` and `pCounterBufferOffsets` values. Between the pause and resume there needs to be a memory barrier for the counter buffers with a source access of `VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT` at pipeline stage `VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT` to a destination access of `VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT` at pipeline stage `VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT`.

## Work for glTransformFeedbackVaryings

There is no equivalent function for glTransforFeedbackVaryings in Vulkan. Vulkan specification states that the last vertex processing stage shader must be declared with the XFB execution mode.

So we need to modify gl shader code to have transform feedback qualifiers. The glsl code will be converted proper SPIR-V code.

we will add below qualifier for built-in xfb varyings.

```
out gl_PerVertex

{

layout(xfb_buffer = buffer_num, xfb_offset = offset, xfb_stride = stride) varying_type varying_name;

}
```

 And for normal xfb varyings,

```
layout(xfb_buffer = buffer_num, xfb_offset = offset, xfb_stride = stride, location = num ) out varying_type varying_name;

```

There are some corner cases we should handle.

If more than 2 built-in varyings are used in the shader, and only one varying is declared in transformFeedback varying, we can generate layout qualifier like this.

```
out gl_PerVertex

{

layout(xfb_buffer = buffer_num, xfb_offset = offset, xfb_stride = stride) varying_type varying_name1;

varying_type varying_name2;

...

}
```

ANGLE modifies gl_position.z in vertex shader for vulkan coordinates system. So, if we use gl_position value to write to XFB buffer, the values will be wrong.

To resolve this, we declare user define position varying and copy value from gl_position. The user defined position varying is used during XFB buffer writing.

```
layout(xfb_buffer = buffer_num, xfb_offset = offset, xfb_stride = stride, location = num ) out vec4 xfbANGLEPosition;

....

void main(){

...



xfbANGLEPosition = gl_Position;
(gl_Position.z = ((gl_Position.z + gl_Position.w) * 0.5));
}
```

