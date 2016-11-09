//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/RemoveInvariantDeclaration.h"

#include "angle_gl.h"
#include "compiler/translator/IntermNode.h"

namespace sh
{

namespace
{

// An AST traverser that removes invariant declaration for input in fragment shader
// when GLSL >= 4.20 and for output in vertex shader when GLSL < 4.2.
class RemoveInvariantDeclarationTraverser : public TIntermTraverser
{
  public:
    RemoveInvariantDeclarationTraverser(GLenum shaderType)
        : TIntermTraverser(true, false, false), mShaderType(shaderType)
    {
    }

  private:
    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        if (node->getOp() == EOpInvariantDeclaration)
        {
            for (TIntermNode *&child : *node->getSequence())
            {
                TIntermTyped *typed = child->getAsTyped();
                if (typed)
                {
                    TQualifier qualifier = typed->getQualifier();
                    if ((mShaderType == GL_FRAGMENT_SHADER && qualifier == EvqVaryingIn) ||
                        (mShaderType == GL_VERTEX_SHADER &&
                         (qualifier == EvqVertexOut || qualifier == EvqSmoothOut ||
                          qualifier == EvqCentroidOut || qualifier == EvqFlatOut)))
                    {
                        TIntermSequence emptyReplacement;
                        mMultiReplacements.push_back(NodeReplaceWithMultipleEntry(
                            getParentNode()->getAsBlock(), node, emptyReplacement));
                        return false;
                    }
                }
            }
        }
        return true;
    }

    GLenum mShaderType;
};

}  // anonymous namespace

void RemoveInvariantDeclaration(TIntermNode *root, GLenum shaderType)
{
    RemoveInvariantDeclarationTraverser traverser(shaderType);
    root->traverse(&traverser);
    traverser.updateTree();
}

}  // namespace sh
