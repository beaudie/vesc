//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// UniformBlockTranslatedToStructuredBuffer.h:
// Collect all uniform blocks which will been translated to StructuredBuffer on Direct3D
// backend.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_UNIFORMBLOCKTRANSLATEDTOSTRUCTUREDBUFFER_H_
#define COMPILER_TRANSLATOR_TREEOPS_UNIFORMBLOCKTRANSLATEDTOSTRUCTUREDBUFFER_H_

#include "compiler/translator/IntermNode.h"

namespace sh
{
class TIntermNode;

ANGLE_NO_DISCARD bool RecordUniformBlockTranslatedToStructuredBuffer(
    TIntermNode *root,
    std::map<int, const TInterfaceBlock *> &uniformBlockTranslatedToStructuredBuffer);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_RECORDACCESSUNIFORMBLOCKENTIREARRAYMEMBER_H_
