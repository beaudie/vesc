//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_SEPARATEDECLARATIONS_H_
#define COMPILER_TRANSLATOR_TREEOPS_SEPARATEDECLARATIONS_H_

#include "common/angleutils.h"

namespace sh
{
class TCompiler;
class TIntermBlock;

// Transforms declarations so that:
//  - each declaration contains only one declarator
//  - structs always have names
//  - struct declarations never have any declarators
//  - for loop intializers never declare anything, rather declarations and their initializers are
//  moved to new enclosing block
// This is useful as an intermediate step when initialization needs to be separated from
// declaration, or when things need to be unfolded out of the initializer.
// Examples:
// Input:
//     int a[1] = int[1](1), b[1] = int[1](2);
// Output:
//     int a[1] = int[1](1);
//     int b[1] = int[1](2);
// Input:
//    struct S { vec3 d; } a, b;
// Output:
//    struct S { vec3 d; };
//    S a;
//    S b;
// Input:
//    struct { vec3 d; } a, b;
// Output:
//    struct s1234 { vec3 d; };
//    s1234 a;
//    s1234 b;
// Input:
//   for (struct S { vec3 d; } a; a.d < 4.; a.d++) ...
// Output
//    {
//       struct S { vec d; };
//       S a;
//       for (; a.d < 4.; a.d++) ...
//    }
[[nodiscard]] bool SeparateDeclarations(TCompiler &compiler, TIntermBlock &root);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_SEPARATEDECLARATIONS_H_
