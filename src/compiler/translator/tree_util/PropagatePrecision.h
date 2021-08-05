//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PropagatePrecision.h: Propagates precision to AST nodes.  The shader defines precision of
// variables, either explicitly for each or through a global statement.  Intermediate results (i.e.
// AST nodes) take their precision from their operands according to GLSL rules.

#ifndef COMPILER_TRANSLATOR_TREEUTIL_PROPAGATEPRECISION_H_
#define COMPILER_TRANSLATOR_TREEUTIL_PROPAGATEPRECISION_H_

#include "common/angleutils.h"

namespace sh
{

class TIntermNode;

void PropagatePrecision(TIntermNode *root);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEUTIL_PROPAGATEPRECISION_H_
