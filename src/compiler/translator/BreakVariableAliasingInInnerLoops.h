//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BreakVariableAliasingInInnerLoops.h: Work around for a HLSL compiler frontend optimization
//     bug where the aliasing information gets garbled.

#ifndef COMPILER_TRANSLATOR_BREAKVARIABLEALIASINGININNERLOOPS_H_
#define COMPILER_TRANSLATOR_BREAKVARIABLEALIASINGININNERLOOPS_H_

class TIntermNode;
void BreakVariableAliasingInInnerLoops(TIntermNode *root);

#endif  // COMPILER_TRANSLATOR_BREAKVARIABLEALIASINGININNERLOOPS_H_
