//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MonomorphizeUnsupportedFunctions: Monomorphize functions that are called with
// parameters that are incompatible with both Vulkan GLSL and Metal:
//
// - Samplers in structs
// - Structs that have samplers
// - Partially subscripted array of array of samplers
// - Partially subscripted array of array of images
// - Atomic counters
// - samplerCube variables when emulating ES2's cube map sampling
// - image* variables with r32f formats (to emulate imageAtomicExchange)
//
// This transformation basically duplicates such functions, removes the
// sampler/image/atomic_counter parameters and uses the opaque uniforms used by the caller.

#ifndef COMPILER_TRANSLATOR_TREEOPS_VULKAN_MONOMORPHIZEUNSUPPORTEDFUNCTIONS_H_
#define COMPILER_TRANSLATOR_TREEOPS_VULKAN_MONOMORPHIZEUNSUPPORTEDFUNCTIONS_H_

#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"

namespace sh
{
class TIntermBlock;
class TSymbolTable;

// Types of function prameters that should trigger monomorphization. These can be combined as flags.
enum UnsupportedFunctionArguments
{
    EufaStructContainingSamplers     = 1 << 0,
    EufaArrayOfArrayOfSamplerOrImage = 1 << 1,
    EufaAtomicCounter                = 1 << 2,
    EufaSamplerCubeEmulation         = 1 << 3,
    EufaImage                        = 1 << 4,
    EufaPixelLocalStorage            = 1 << 5
};

[[nodiscard]] bool MonomorphizeUnsupportedFunctions(TCompiler *compiler,
                                                    TIntermBlock *root,
                                                    TSymbolTable *symbolTable,
                                                    ShCompileOptions compileOptions,
                                                    uint32_t unsupportedFunctionArgumentFlags = ~0);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_VULKAN_MONOMORPHIZEUNSUPPORTEDFUNCTIONS_H_
