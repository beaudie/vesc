//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ReplaceForAdvancedBlendEquation.h: Find any blend equation, replace it with the corresponding
// equation function and add the operation for the blending job.
//

#ifndef COMPILER_TRANSLATOR_TREEUTIL_REPLACEFORADVANCEDBLENDEQUATION_H_
#define COMPILER_TRANSLATOR_TREEUTIL_REPLACEFORADVANCEDBLENDEQUATION_H_

#include "common/angleutils.h"

namespace sh
{

class TCompiler;
class TInfoSinkBase;
class TIntermBlock;
class TSymbolTable;
class SpecConst;
class DriverUniform;
struct ShaderVariable;
struct AdvancedBlendEquation;

// Replace each blend equation qualifier to the corresponding blending function, declare the input
// attachment variable and store a subpassLoad value to it. Then operate the blending job using the
// input attachment value and the fragment out value, and assign it to the output.
ANGLE_NO_DISCARD bool AddBlendMainCaller(TCompiler *compiler,
                                         TInfoSinkBase &sink,
                                         TIntermBlock *root,
                                         TSymbolTable *symbolTable,
                                         SpecConst *specConst,
                                         const DriverUniform *driverUniforms,
                                         std::vector<ShaderVariable> *uniforms,
                                         const AdvancedBlendEquation &blendEquation);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEUTIL_REPLACEFORADVANCEDBLENDEQUATION_H_
