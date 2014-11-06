//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_REPLACE_NODES_AST_H_
#define COMPILER_REPLACE_NODES_AST_H_

#include "common/angleutils.h"
#include "compiler/translator/IntermNode.h"

// The traversers inheriting from this class should list the nodes that they
// want replaced in the member vector replacements.
// The actual replacements happen after the traverse through updateTree().

class ReplaceNodesAST : public TIntermTraverser
{
  public:
    ReplaceNodesAST() { }

    void updateTree();

  protected:
    struct NodeUpdateEntry
    {
        NodeUpdateEntry(TIntermNode *_parent,
                        TIntermNode *_original,
                        TIntermNode *_replacement)
            : parent(_parent),
              original(_original),
              replacement(_replacement) {}

        TIntermNode *parent;
        TIntermNode *original;
        TIntermNode *replacement;
    };

    // During traversing, save all the replacements that need to happen;
    // then replace them by calling updateTree().
    std::vector<NodeUpdateEntry> replacements;

  private:
    DISALLOW_COPY_AND_ASSIGN(ReplaceNodesAST);
};

#endif  // COMPILER_REPLACE_NODES_AST_H_
