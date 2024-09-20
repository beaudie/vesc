//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_WGSL_OUTPUT_UNIFORM_BLOCKS_H_
#define COMPILER_TRANSLATOR_WGSL_OUTPUT_UNIFORM_BLOCKS_H_

#include <variant>

#include "compiler/translator/Compiler.h"
#include "compiler/translator/IntermNode.h"

namespace sh
{

const char kDefaultUniformBlockVarName[] = "ANGLE_defaultUniformBlock";

// TODO XXX comment. For now only outputs basic uniforms.
bool OutputUniformBlocks(TCompiler *compiler, TIntermBlock *root);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_WGSL_OUTPUT_UNIFORM_BLOCKS_H_
