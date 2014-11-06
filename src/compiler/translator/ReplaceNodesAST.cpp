//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/ReplaceNodesAST.h"

void ReplaceNodesAST::updateTree()
{
    for (size_t ii = 0; ii < replacements.size(); ++ii)
    {
        const NodeUpdateEntry& entry = replacements[ii];
        ASSERT(entry.parent);
        bool replaced = entry.parent->replaceChildNode(
            entry.original, entry.replacement);
        ASSERT(replaced);

        // In AST traversing, a parent is visited before its children.
        // After we replace a node, if an immediate child is to
        // be replaced, we need to make sure we don't update the replaced
        // node; instead, we update the replacement node.
        for (size_t jj = ii + 1; jj < replacements.size(); ++jj)
        {
            NodeUpdateEntry& entry2 = replacements[jj];
            if (entry2.parent == entry.original)
                entry2.parent = entry.replacement;
        }
    }
}

