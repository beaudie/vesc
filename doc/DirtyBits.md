# Dirty Bits and State Changes

OpenGL render loops typically involve changing some render states followed by
a draw call. For instance the app might change a few uniforms and invoke
`glDrawElements`.

```
for (const auto &obj : scene) {
    for (const auto &uni : obj.uniforms) {
        glUniform4fv(uni.loc, uni.data);
    }
    glDrawElements(GL_TRIANGLES, obj.eleCount, GL_UNSIGNED_SHORT, obj.eleOffset);
}
```

Another update loop may change Texture and Vertex Array state before the draw:

```
for (const auto &obj : scene) {
    glBindBuffer(GL_ARRAY_BUFFER, obj.arrayBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, obj.bufferOffset, obj.bufferSize, obj.bufferData);
    glVertexAttribPointer(obj.arrayIndex, obj.arraySize, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindTexture(GL_TEXTURE_2D, obj.texture);
    glDrawElements(GL_TRIANGLES, obj.eleCount, GL_UNSIGNED_SHORT, obj.eleOffset);
}
```

Other update looks may change render states like the blending modes, depth
test state, or Framebuffer state. In in each case ANGLE needs to validate,
track and translate these state changes to the back-end as efficiently as
possible.
