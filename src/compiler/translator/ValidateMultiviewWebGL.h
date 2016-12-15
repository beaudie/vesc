//
// Copyright (c) 2002-2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_VALIDATEMULTIVIEWWEBGL_H_
#define COMPILER_TRANSLATOR_VALIDATEMULTIVIEWWEBGL_H_

namespace sh
{
class TDiagnostics;
class TIntermBlock;

// GLenum alias
typedef unsigned int GLenum;

// Check for errors and output error messages with diagnostics.
// Returns true if there are no errors.
bool ValidateMultiviewWebGL(TIntermBlock *root,
                            sh::GLenum shaderType,
                            bool multiview2,
                            TDiagnostics *diagnostics);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_VALIDATEMULTIVIEWWEBGL_H_
