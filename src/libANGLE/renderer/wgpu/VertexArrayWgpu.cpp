//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VertexArrayWgpu.cpp:
//    Implements the class methods for VertexArrayWgpu.
//

#include "libANGLE/renderer/wgpu/VertexArrayWgpu.h"

#include "common/debug.h"
#include "libANGLE/renderer/wgpu/ContextWgpu.h"

namespace rx
{

VertexArrayWgpu::VertexArrayWgpu(const gl::VertexArrayState &data) : VertexArrayImpl(data) {}

angle::Result VertexArrayWgpu::syncState(const gl::Context *context,
                                         const gl::VertexArray::DirtyBits &dirtyBits,
                                         gl::VertexArray::DirtyAttribBitsArray *attribBits,
                                         gl::VertexArray::DirtyBindingBitsArray *bindingBits)
{
    ASSERT(dirtyBits.any());

    ContextWgpu *contextWgpu = GetImplAs<ContextWgpu>(context);

    const std::vector<gl::VertexAttribute> &attribs = mState.getVertexAttributes();
    const std::vector<gl::VertexBinding> &bindings  = mState.getVertexBindings();
    bool syncedAttribs                              = false;

    for (auto iter = dirtyBits.begin(), endIter = dirtyBits.end(); iter != endIter; ++iter)
    {
        size_t dirtyBit = *iter;
        switch (dirtyBit)
        {
            case gl::VertexArray::DIRTY_BIT_LOST_OBSERVATION:
            case gl::VertexArray::DIRTY_BIT_ELEMENT_ARRAY_BUFFER:
            case gl::VertexArray::DIRTY_BIT_ELEMENT_ARRAY_BUFFER_DATA:
                break;

#define ANGLE_VERTEX_DIRTY_ATTRIB_FUNC(INDEX)                                     \
    case gl::VertexArray::DIRTY_BIT_ATTRIB_0 + INDEX:                             \
        ANGLE_TRY(syncDirtyAttrib(contextWgpu, attribs[INDEX],                    \
                                  bindings[attribs[INDEX].bindingIndex], INDEX)); \
        (*attribBits)[INDEX].reset();                                             \
        syncedAttribs = true;                                                     \
        break;

                ANGLE_VERTEX_INDEX_CASES(ANGLE_VERTEX_DIRTY_ATTRIB_FUNC)

#define ANGLE_VERTEX_DIRTY_BINDING_FUNC(INDEX)                                    \
    case gl::VertexArray::DIRTY_BIT_BINDING_0 + INDEX:                            \
        ANGLE_TRY(syncDirtyAttrib(contextWgpu, attribs[INDEX],                    \
                                  bindings[attribs[INDEX].bindingIndex], INDEX)); \
        (*bindingBits)[INDEX].reset();                                            \
        syncedAttribs = true;                                                     \
        break;

                ANGLE_VERTEX_INDEX_CASES(ANGLE_VERTEX_DIRTY_BINDING_FUNC)

#define ANGLE_VERTEX_DIRTY_BUFFER_DATA_FUNC(INDEX)                                \
    case gl::VertexArray::DIRTY_BIT_BUFFER_DATA_0 + INDEX:                        \
        ANGLE_TRY(syncDirtyAttrib(contextWgpu, attribs[INDEX],                    \
                                  bindings[attribs[INDEX].bindingIndex], INDEX)); \
        syncedAttribs = true;                                                     \
        break;

                ANGLE_VERTEX_INDEX_CASES(ANGLE_VERTEX_DIRTY_BUFFER_DATA_FUNC)
            default:
                break;
        }
    }

    if (syncedAttribs)
    {
        contextWgpu->setVertexAttributes(mCurrentAttribs);
    }
    return angle::Result::Continue;
}

std::vector<VertexBufferUpdate> VertexArrayWgpu::getBuffersToSet()
{
    std::vector<VertexBufferUpdate> bufferUpdate;
    for (uint32_t i = 0; i < mCurrentAttribs.size(); i++)
    {
        if (mCurrentAttribs[i].enabled)
        {
            bufferUpdate.push_back(VertexBufferUpdate(i, mCurrentArrayBuffers[i]));
        }
    }
    return bufferUpdate;
}

angle::Result VertexArrayWgpu::syncDirtyAttrib(ContextWgpu *contextWgpu,
                                               const gl::VertexAttribute &attrib,
                                               const gl::VertexBinding &binding,
                                               size_t attribIndex)
{
    if (attrib.enabled)
    {
        const webgpu::Format &webgpuFormat =
            contextWgpu->getFormat(attrib.format->glInternalFormat);
        SetBitField(mCurrentAttribs[attribIndex].format, webgpuFormat.getActualWgpuVertexFormat());
        gl::Buffer *bufferGl                        = binding.getBuffer().get();
        mCurrentAttribs[attribIndex].enabled        = true;
        mCurrentAttribs[attribIndex].shaderLocation = 0;
        mCurrentAttribs[attribIndex].stride         = binding.getStride();
        if (bufferGl && bufferGl->getSize() > 0)
        {
            SetBitField(mCurrentAttribs[attribIndex].offset,
                        reinterpret_cast<uintptr_t>(attrib.pointer));
            BufferWgpu *bufferWgpu            = GetImplAs<BufferWgpu>(bufferGl);
            mCurrentArrayBuffers[attribIndex] = &(bufferWgpu->getBuffer());
        }
        else
        {
            SetBitField(mCurrentAttribs[attribIndex].offset, binding.getOffset());
            mCurrentArrayBuffers[attribIndex] = nullptr;
        }
    }
    return angle::Result::Continue;
}

}  // namespace rx
