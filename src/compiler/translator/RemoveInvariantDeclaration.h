//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_REMOVEINVARIANTDECLARATION_H_
#define COMPILER_TRANSLATOR_REMOVEINVARIANTDECLARATION_H_

#include "GLSLANG/ShaderLang.h"

class TIntermNode;
namespace sh
{

void RemoveInvariantDeclaration(TIntermNode *root, GLenum shadeType);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_REMOVEINVARIANTDECLARATION_H_
