//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/ExtensionGLSL.h"

#include "compiler/translator/VersionGLSL.h"

TExtensionGLSL::TExtensionGLSL(ShShaderOutput output)
    : TIntermTraverser(true, false, false)
{
    mTargetVersion = ShaderOutputTypeToGLSLVersion(output);
}


const std::set<std::string> &TExtensionGLSL::getExtensions() const
{
    return mExtensions;
}

bool TExtensionGLSL::visitUnary(Visit, TIntermUnary *node)
{
    checkOperator(node);

    return true;
}

bool TExtensionGLSL::visitAggregate(Visit, TIntermAggregate *node)
{
    checkOperator(node);

    return true;
}

void TExtensionGLSL::checkOperator(TIntermOperator *node)
{
    if (mTargetVersion < GLSL_VERSION_130)
    {
        return;
    }

    switch (node->getOp())
    {
      case EOpAbs:
        break;

      case EOpSign:
        break;

      case EOpMix:
        break;

      case EOpFloatBitsToInt:
      case EOpFloatBitsToUint:
      case EOpIntBitsToFloat:
      case EOpUintBitsToFloat:
        if (mTargetVersion < GLSL_VERSION_330)
        {
            mExtensions.insert("GL_ARB_shader_bit_encoding");
        }
        break;

      case EOpPackSnorm2x16:
      case EOpPackHalf2x16:
      case EOpUnpackSnorm2x16:
      case EOpUnpackHalf2x16:
        if (mTargetVersion < GLSL_VERSION_420)
        {
            mExtensions.insert("GL_ARB_shading_language_packing");
        }
        break;

      case EOpPackUnorm2x16:
      case EOpUnpackUnorm2x16:
        if (mTargetVersion < GLSL_VERSION_410)
        {
            mExtensions.insert("GL_ARB_shading_language_packing");
        }
        break;

      default:
        break;
    }
}
