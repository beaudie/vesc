//
// Copyright (c) 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GlslangWrapper: Wrapper for Khronos's glslang compiler.
//

#include "libANGLE/renderer/metal/GlslangWrapper.h"

namespace rx
{

// static
void GlslangWrapperMtl::Initialize()
{
    UNIMPLEMENTED();
}

// static
void GlslangWrapperMtl::Release()
{
    UNIMPLEMENTED();
}

// static
void GlslangWrapperMtl::GetShaderSource(const gl::ProgramState &programState,
                                        const gl::ProgramLinkedResources &resources,
                                        std::string *vertexSourceOut,
                                        std::string *fragmentSourceOut)
{
    UNIMPLEMENTED();
}

// static
angle::Result GlslangWrapperMtl::GetShaderCode(mtl::ErrorHandler *context,
                                               const gl::Caps &glCaps,
                                               bool enableLineRasterEmulation,
                                               const std::string &vertexSource,
                                               const std::string &fragmentSource,
                                               std::vector<uint32_t> *vertexCodeOut,
                                               std::vector<uint32_t> *fragmentCodeOut)
{
    UNIMPLEMENTED();
    return angle::Result::Stop;
}

}  // namespace rx
