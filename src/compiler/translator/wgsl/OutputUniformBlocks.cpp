//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/wgsl/OutputUniformBlocks.h"

#include "common/utilities.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/util.h"
#include "compiler/translator/wgsl/Utils.h"

namespace sh
{

bool OutputUniformBlocks(TCompiler *compiler, TIntermBlock *root)
{
    // TODO(anglebug.com/42267100): This should eventually just be handled the same way as a regular
    // UBO, like in Vulkan which create a block out of the default uniforms with a traverser:
    // https://source.chromium.org/chromium/chromium/src/+/main:third_party/angle/src/compiler/translator/spirv/TranslatorSPIRV.cpp;drc=451093bbaf7fe812bf67d27d760f3bb64c92830b;bpv=1;bpt=1;l=70?gsn=ReplaceDefaultUniformsTraverser&gs=KYTHE%3A%2F%2Fkythe%3A%2F%2Fchromium.googlesource.com%2Fcodesearch%2Fchromium%2Fsrc%2F%2Fmain%3Flang%3Dc%252B%252B%3Fpath%3Dthird_party%2Fangle%2Fsrc%2Fcompiler%2Ftranslator%2Fspirv%2FTranslatorSPIRV.cpp%23HEwy4N2CIZtIA0CMkSDXXUY_Z6FUBl3hwAtI0xw4DUA
    const std::vector<ShaderVariable> &basicUniforms = compiler->getUniforms();
    TInfoSinkBase &output                            = compiler->getInfoSink().obj;
    GlobalVars globalVars                            = FindGlobalVars(root);

    // Only output a struct at all if there are going to be members.
    bool outputStructHeader = false;
    for (const ShaderVariable &shaderVar : basicUniforms)
    {
        if (gl::IsOpaqueType(shaderVar.type))
        {
            continue;
        }
        if (!outputStructHeader)
        {
            output << "struct ANGLE_DefaultUniformBlock {\n";
            outputStructHeader = true;
        }
        output << "  ";
        // TODO(anglebug.com/42267100): some types will NOT match std140 layout here, namely matCx2,
        // bool, and arrays with stride less than 16.
        // (this ASSERT does not cover the unsupported case where there is an array of structs of
        // size < 16).
        ASSERT(gl::VariableRowCount(shaderVar.type) != 2 && shaderVar.type != GL_BOOL &&
               (!shaderVar.isArray() || shaderVar.isStruct() ||
                gl::VariableComponentCount(shaderVar.type) >= 3));
        output << shaderVar.name << " : ";

        TIntermDeclaration *declNode = globalVars.find(shaderVar.name)->second;
        const TVariable *astVar      = &ViewDeclaration(*declNode).symbol.variable();
        WriteWgslType(output, astVar->getType());

        output << "\n";
    }
    // TODO(anglebug.com/42267100): might need string replacement for @group(0) and @binding(0)
    // annotations. All WGSL resources available to shaders share the same (group, binding) ID
    // space.
    if (outputStructHeader)
    {
        output << "};\n\n"
               << "@group(0) @binding(0) var<uniform> " << kDefaultUniformBlockVarName
               << " : "
                  "ANGLE_DefaultUniformBlock;\n";
    }

    return true;
}

}  // namespace sh
