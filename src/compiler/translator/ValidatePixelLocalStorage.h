//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Copyright 2022 Rive
//

#ifndef COMPILER_TRANSLATOR_VALIDATE_PIXELLOCALSTORAGE_H_
#define COMPILER_TRANSLATOR_VALIDATE_PIXELLOCALSTORAGE_H_

#include "GLSLANG/ShaderVars.h"

namespace sh
{

class TIntermBlock;
class TSymbolTable;
class TDiagnostics;

enum class PixelLocalStorageUsage
{
    Invalid,
    Valid,
    Unused
};

// Populates 'diagnostics' and returns false if the shader performs any of the following illegal
// operations while pixel local storage is declared:
//
//   - discard
//
//         The shader image polyfill requires early_fragment_tests, which causes discard to interact
//         differently with the depth and stencil tests.
//
//         To ensure identical behavior across all backends (some of which may not have access to
//         early_fragment_tests), we disallow discard if pixel local storage has been declared.
//
//   - return from main
//
//         ARB_fragment_shader_interlock isn't allowed after return from main.
//
//   - assign gl_FragDepth(EXT) or gl_SampleMask
//
//         The shader image polyfill requires early_fragment_tests, which causes assignments to
//         gl_FragDepth(EXT) and gl_SampleMask to be ignored.
//
//         To ensure identical behavior across all backends, we disallow assignment to these values
//         if pixel local storage has been declared.
//
//   - pixel local storage handles as function arguments
//
//         TODO(anglebug.com/7484): Consider allowing this once it is properly supported for images.
//
//   - invalid pixel local storage binding
//
//   - duplicate pixel local storage binding index
//
//         The compiler is free to optimize under the assumption that the data under a single PLS
//         handle will not be modified in any other way during the shader invocation.
//
//   - invalid pixel local storage format
//
PixelLocalStorageUsage ValidatePixelLocalStorage(TIntermBlock *root,
                                                 GLenum shaderType,
                                                 TSymbolTable *symbolTable,
                                                 TDiagnostics *diagnostics);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_VALIDATELIMITATIONS_H_
