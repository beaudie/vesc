//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_DECLAREANDINITMULTIVIEWVARS_H_
#define COMPILER_TRANSLATOR_DECLAREANDINITMULTIVIEWVARS_H_

class TIntermBlock;

namespace sh
{

// This function applies the following modifications to the AST:
// 1) Replaces every occurrence of gl_InstanceID with gl_InstanceIDImpostor.
// 2) Adds declarations of the global variables gl_ViewID_OVR and gl_InstanceIDImpostor
// 3) Initializes gl_ViewID_OVR and gl_InstanceIDImpostor depending on the number of views.
// The pass should be executed before any variables get collected.
void DeclareAndInitMultiviewVars(TIntermBlock *root, unsigned numberOfViews);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_DECLAREANDINITMULTIVIEWVARS_H_