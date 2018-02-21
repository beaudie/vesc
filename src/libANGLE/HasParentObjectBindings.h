//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// HasParentObjectBindings:
//   Helper mixin class for handling dependent state change notifications with dirty bits.
//   Used for VertexArray/Buffers and Framebuffer/FramebufferAttachments.

#ifndef LIBANGLE_HAS_PARENT_OBJECT_BINDINGS_H_
#define LIBANGLE_HAS_PARENT_OBJECT_BINDINGS_H_

#include <functional>

#include "common/angleutils.h"

namespace gl
{
class Context;
}  // namespace gl

namespace angle
{

enum class DependentStateChangeMessage
{
    FRAMEBUFFER_ATTACHMENT_DATA_CHANGE,
    FRAMEBUFFER_DIRTY_BITS,
    BUFFER_DATA_CHANGE,
    VERTEX_ARRAY_DIRTY_BITS,
};

using ParentID          = void *;
using ParentSubresource = uint32_t;
using GenericCallback   = std::function<void(const gl::Context *)>;

class HasParentObjectBindings : angle::NonCopyable
{
  public:
    HasParentObjectBindings();
    ~HasParentObjectBindings();

    void onParentAttach(ParentID parentID,
                        ParentSubresource parentSubresource,
                        DependentStateChangeMessage message,
                        const GenericCallback &callback);
    void onParentDetach(ParentID parentID,
                        ParentSubresource parentSubresource,
                        DependentStateChangeMessage message);

    void onDependentStateChange(const gl::Context *context, DependentStateChangeMessage message);

  private:
    using Attachment =
        std::tuple<ParentID, ParentSubresource, DependentStateChangeMessage, GenericCallback>;
    std::vector<Attachment> mParentAttachments;
};

}  // namespace angle

#endif  // LIBANGLE_HAS_PARENT_OBJECT_BINDINGS_H_
