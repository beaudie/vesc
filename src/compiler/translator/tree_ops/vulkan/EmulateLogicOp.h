//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EmulateLogicOp.h: Emulate logicOp by implicitly reading back from the color attachment (as an
// input attachment) and apply the logic op equation based on a uniform.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_VULKAN_EMULATELOGICOP_H_
#define COMPILER_TRANSLATOR_TREEOPS_VULKAN_EMULATELOGICOP_H_

#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"

namespace sh
{

class TCompiler;
class TIntermBlock;
class TSymbolTable;
class DriverUniform;
struct ShaderVariable;

// Declares the necessary input attachment (if not already for framebuffer fetch), loads from it and
// implements logic op equations.  A driver uniform is used to select which logic op to use at
// runtime.  This driver uniform aliases the advanced blend equation.
[[nodiscard]] bool EmulateLogicOp(TCompiler *compiler,
                                  const ShCompileOptions &compileOptions,
                                  TIntermBlock *root,
                                  TSymbolTable *symbolTable,
                                  const DriverUniform *driverUniforms,
                                  std::vector<ShaderVariable> *uniforms);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_VULKAN_EMULATELOGICOP_H_
