//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_USEINTERFACEBLOCKFIELDS_H_
#define COMPILER_TRANSLATOR_USEINTERFACEBLOCKFIELDS_H_

#include <GLSLANG/ShaderLang.h>

class TIntermNode;

typedef std::vector<sh::InterfaceBlock> UseBlockFieldList;

// This function cannot currently initialize structures containing arrays for an ESSL 1.00 backend.
void UseInterfaceBlockFields(TIntermNode *root, const UseBlockFieldList &blocks);

#endif  // COMPILER_TRANSLATOR_USEINTERFACEBLOCKFIELDS_H_
