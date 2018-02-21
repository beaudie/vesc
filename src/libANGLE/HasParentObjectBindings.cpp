//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// HasParentObjectBindings:
//   Helper mixin class for handling dependent state change notifications with dirty bits.
//   Used for VertexArray/Buffers and Framebuffer/FramebufferAttachments.

#include "libANGLE/HasParentObjectBindings.h"

#include "libANGLE/Context.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/VertexArray.h"

namespace angle
{

HasParentObjectBindings::HasParentObjectBindings()
{
}

HasParentObjectBindings::~HasParentObjectBindings()
{
    ASSERT(mParentAttachments.empty());
}

void HasParentObjectBindings::onParentAttach(ParentID parentID,
                                             ParentSubresource parentSubresource,
                                             DependentStateChangeMessage message,
                                             const GenericCallback &callback)
{
    if (!parentID)
        return;

    onParentDetach(parentID, parentSubresource, message);
    mParentAttachments.emplace_back(parentID, parentSubresource, message, callback);
}

void HasParentObjectBindings::onParentDetach(ParentID parentID,
                                             ParentSubresource parentSubresource,
                                             DependentStateChangeMessage message)
{
    if (!parentID)
        return;

    for (size_t objIndex = 0; objIndex < mParentAttachments.size(); ++objIndex)
    {
        if (std::get<ParentID>(mParentAttachments[objIndex]) == parentID &&
            std::get<ParentSubresource>(mParentAttachments[objIndex]) == parentSubresource &&
            std::get<DependentStateChangeMessage>(mParentAttachments[objIndex]) == message)
        {
            mParentAttachments.erase(mParentAttachments.begin() + objIndex);
            return;
        }
    }
}

void HasParentObjectBindings::onDependentStateChange(const gl::Context *context,
                                                     DependentStateChangeMessage message)
{
    for (Attachment &attachment : mParentAttachments)
    {
        if (std::get<DependentStateChangeMessage>(attachment) == message)
        {
            std::get<GenericCallback>(attachment)(context);
        }
    }
}

}  // namespace angle
