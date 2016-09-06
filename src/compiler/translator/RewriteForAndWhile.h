//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RewriteForAndWhile.h: rewrite for and while loops to work around
// driver bugs

#ifndef COMPILER_TRANSLATOR_REWRITEFORANDWHILE_H_
#define COMPILER_TRANSLATOR_REWRITEFORANDWHILE_H_

class TIntermNode;
namespace sh
{

void RewriteForAndWhile(TIntermNode *root, unsigned int *temporaryIndex);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_REWRITEFORANDWHILE_H_
