//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CompilerMtl.mm:
//    Implements the class methods for CompilerMtl.
//

#include "libANGLE/renderer/metal/CompilerMtl.h"

#include "common/debug.h"
#include "common/system_utils.h"

namespace rx
{

CompilerMtl::CompilerMtl() : CompilerImpl() {}

CompilerMtl::~CompilerMtl() {}

ShShaderOutput CompilerMtl::getTranslatorOutputType() const
{
    std::string genMtlWithSpirvString = angle::GetEnvironmentVar("ANGLE_GEN_MTL_WITH_SPIRV");
    bool genMtlWithSpirvBool = !genMtlWithSpirvString.empty() && (genMtlWithSpirvString == "1");
    if (genMtlWithSpirvBool)
    {
        // We want to return GL output first, we can't actually
        // get MSL code until link time. Translation time is too early
        return SH_GLSL_METAL_OUTPUT;
    }
    else
    {
        return SH_MSL_METAL_OUTPUT;
    }
}

}  // namespace rx
