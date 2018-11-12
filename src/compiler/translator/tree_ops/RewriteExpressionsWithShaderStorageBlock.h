//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RewriteExpressionsWithShaderStorageBlock rewrites the expressions that contain shader storage
// block calls into several simple ones that can be easily handled in the HLSL translator. After the
// AST pass, all ssbo related blocks will be like below:
//     ssbo_access_chain = ssbo_access_chain;
//     ssbo_access_chain = expr_no_ssbo;
//     lvalue_no_ssbo    = ssbo_access_chain;
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_REWRITE_EXPRESSIONS_WITH_SHADER_STORAGE_BLOCK_H_
#define COMPILER_TRANSLATOR_TREEOPS_REWRITE_EXPRESSIONS_WITH_SHADER_STORAGE_BLOCK_H_

namespace sh
{
class TIntermNode;
class TSymbolTable;

void RewriteExpressionsWithShaderStorageBlock(TIntermNode *root, TSymbolTable *symbolTable);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_REWRITE_EXPRESSIONS_WITH_SHADER_STORAGE_BLOCK_H_
