//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EmulateFramebufferFetch.h: Replace input, gl_LastFragData and gl_LastFragColorARM with usages of
// input attachments.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_SPIRV_EMULATEFRAMEBUFFERFETCH_H_
#define COMPILER_TRANSLATOR_TREEOPS_SPIRV_EMULATEFRAMEBUFFERFETCH_H_

#include "common/angleutils.h"
#include "compiler/translator/Common.h"
#include "compiler/translator/Compiler.h"

namespace sh
{

class TIntermBlock;

// An index -> TVariable map, tracking the declarated input attachments.
using InputAttachmentMap = TUnorderedMap<uint32_t, const TVariable *>;

// Emulate framebuffer fetch through the use of input attachments.
[[nodiscard]] bool EmulateFramebufferFetch(TCompiler *compiler,
                                           TIntermBlock *root,
                                           InputAttachmentMap *inputAttachmentMap);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_SPIRV_REPLACEFORSHADERFRAMEBUFFERFETCH_H_
