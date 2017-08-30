//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RemoveArrayLengthMethod.h:
//   Fold array length expressions, including cases where the "this" node has side effects.
//   Examples:
//     (a = b).length()
//     (func()).length()
//   becomes:
//     s0 = (a = b);
//     <constant array length>;
//     s1 = func();
//     <constant array length>;

namespace sh
{

class TIntermBlock;

void RemoveArrayLengthMethod(TIntermBlock *root);

}  // namespace sh