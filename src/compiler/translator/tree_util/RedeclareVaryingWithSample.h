//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RedeclareVaryingWithSample.h: Find varyings at the fragment shader which doesn't have any
// auxiliary storage qulifiers or interpolation qualifiers, and add sample qualifier to the varying
//
// in highp vec4 v_position;
// -> sample in highp vec4 v_position;

#ifndef COMPILER_TRANSLATOR_TREEUTIL_REDECLAREVARYINGWITHSAMPLE_H_
#define COMPILER_TRANSLATOR_TREEUTIL_REDECLAREVARYINGWITHSAMPLE_H_

#include "common/angleutils.h"

namespace sh
{

class TCompiler;
class TIntermBlock;
class TSymbolTable;

// From the Vulkan spec:
// For fragment shader input variables decorated with neither Centroid nor Sample, the assigned
// variable may be interpolated anywhere within the fragment and a single value may be assigned to
// each sample within the fragment.
//
// So, when the case that OES_shader_multisample_interpolation extension is not enabled, we have to
// add sample keyword forcibly to assign a seperate value to the varying
ANGLE_NO_DISCARD bool RedeclareVaryingWithSample(TCompiler *compiler,
                                                 TIntermBlock *root,
                                                 TSymbolTable *symbolTable);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEUTIL_REDECLAREVARYINGWITHSAMPLE_H_
