// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SerializeContext.h:
//   ANGLE serialize context inteface.
//
#ifndef LIBANGLE_SERIALIZE_CONTEXT_H_
#define LIBANGLE_SERIALIZE_CONTEXT_H_

#include <iostream>
#include "libANGLE/BinaryStream.h"
#include "libANGLE/Context.h"
#include "libANGLE/Framebuffer.h"

namespace gl
{

void Serialize(BinaryOutputStream *bos, Context *context);

void SerializeFramebuffer(BinaryOutputStream *bos, Context *context, Framebuffer *framebuffer);

void SerializeFramebufferState(BinaryOutputStream *bos,
                               Context *context,
                               FramebufferState *framebufferState);

void SerializeOffset(BinaryOutputStream *bos, Offset *offset);

void SerializeFramebufferAttachment(BinaryOutputStream *bos,
                                    Context *context,
                                    GLenum mode,
                                    FramebufferAttachment *framebufferAttachment);

void SerializeImageIndex(BinaryOutputStream *bos, ImageIndex *imageIndex);

void SerializeObserverBinding(BinaryOutputStream *bos, angle::ObserverBinding *observerBinding);
void SerializeSubject(gl::BinaryOutputStream *bos, angle::Subject *subject);
void SerializeObserverBindingBase(gl::BinaryOutputStream *bos,
                                  angle::ObserverBindingBase *observerBindingBase);

std::vector<uint8_t> ReadPixelsFromAttachment(Context *context,
                                              GLenum mode,
                                              const FramebufferAttachment *framebufferAttachment);

}  // namespace gl
#endif  // LIBANGLE_SERIALIZE_CONTEXT_H_
