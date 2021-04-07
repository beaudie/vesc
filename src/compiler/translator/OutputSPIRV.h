//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// OutputSPIRV: Generate SPIR-V from the AST.
//

#ifndef COMPILER_TRANSLATOR_OUTPUTSPIRV_H_
#define COMPILER_TRANSLATOR_OUTPUTSPIRV_H_

#include "compiler/translator/Compiler.h"

namespace sh
{
// class TIntermBlock;

bool OutputSPIRV(TCompiler *compiler, TIntermBlock *root, ShCompileOptions compileOptions
                 /* TODO: driver uniform stuff, spec const stuff, other flags sent to
                  * TOutputVulkanGLSL like array index clamping strategy, hash function, name map,
                  * symbol table, precisionEmulation, enablePrecision, compileOptions etc.
                  * Do we need shader type and shader version?  Do we need symbol table? Name map?
                  * Hash function?  May be good to initially generate the same names with _u, but
                  * when all is done, just use user names directly in output for better readability.
                  * It's a debug tag, so collisions etc are not an issue.
                  */
);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_OUTPUTSPIRV_H_
