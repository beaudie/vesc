//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RecordAccessUniformBlockEntireArrayMember.h:
// Collect all uniform blocks which have been accessed their entire array member.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_RECORDACCESSUNIFORMBLOCKENTIREARRAYMEMBER_H_
#define COMPILER_TRANSLATOR_TREEOPS_RECORDACCESSUNIFORMBLOCKENTIREARRAYMEMBER_H_

#include "compiler/translator/IntermNode.h"

namespace sh
{
class TIntermNode;

ANGLE_NO_DISCARD bool RecordAccessUniformBlockEntireArrayMember(
    TIntermNode *root,
    std::map<int, const TInterfaceBlock *> &accessUniformBlockEntireArrayMember);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_RECORDACCESSUNIFORMBLOCKENTIREARRAYMEMBER_H_
