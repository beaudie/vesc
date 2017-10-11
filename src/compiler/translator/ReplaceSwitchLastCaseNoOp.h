//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ReplaceSwitchLastCaseNoOp.h: In case the last case of a switch statement is a certain
// type of a no-op statement, replace it with another statement that won't be pruned from the AST
// and that platform compilers will accept.

#ifndef COMPILER_TRANSLATOR_REPLACESWITCHLASTCASENOOP_H_
#define COMPILER_TRANSLATOR_REPLACESWITCHLASTCASENOOP_H_

namespace sh
{
class TIntermBlock;
class TSymbolTable;

void ReplaceSwitchLastCaseNoOp(TIntermBlock *statementList, TSymbolTable *symbolTable);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_REPLACESWITCHLASTCASENOOP_H_
