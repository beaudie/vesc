//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ast_util.h: High-level utilities for creating AST nodes. Mostly meant to be used in AST
// transforms.

#ifndef COMPILER_TRANSLATOR_ASTUTIL_H_
#define COMPILER_TRANSLATOR_ASTUTIL_H_

#include "compiler/translator/IntermNode.h"

namespace sh
{

TIntermFunctionPrototype *CreateInternalFunctionPrototypeNode(const TType &returnType,
                                                              const char *name,
                                                              const TSymbolUniqueId &functionId);
TIntermFunctionDefinition *CreateInternalFunctionDefinitionNode(const TType &returnType,
                                                                const char *name,
                                                                TIntermBlock *functionBody,
                                                                const TSymbolUniqueId &functionId);
TIntermAggregate *CreateInternalFunctionCallNode(const TType &returnType,
                                                 const char *name,
                                                 const TSymbolUniqueId &functionId,
                                                 TIntermSequence *arguments);

TIntermTyped *CreateZero(const TType &type);
TIntermConstantUnion *CreateIndexNode(int index);
TIntermConstantUnion *CreateBool(bool value);

TIntermSymbol *ReferToGlobalSymbol(const TString &name, const TSymbolTable &symbolTable);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_ASTUTIL_H_