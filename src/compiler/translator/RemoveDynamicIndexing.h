//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RemoveDynamicIndexing is an AST traverser to remove dynamic indexing of vectors and matrices,
// replacing them with calls to a function that chooses which component to return.
//

#ifndef COMPILER_TRANSLATOR_REMOVEDYNAMICINDEXING_H_
#define COMPILER_TRANSLATOR_REMOVEDYNAMICINDEXING_H_

class TIntermNode;

void RemoveDynamicIndexing(TIntermNode *root, unsigned int *temporaryIndex);

#endif   // COMPILER_TRANSLATOR_REMOVEDYNAMICINDEXING_H_
