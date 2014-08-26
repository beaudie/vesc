//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramBinaryD3D.cpp: Defines the rx::ProgramBinaryD3D class which implements rx::ProgramBinaryImpl.

#include "libGLESv2/renderer/d3d/ProgramBinaryD3D.h"

#include "common/utilities.h"
#include "libGLESv2/ProgramBinary.h"
#include "libGLESv2/renderer/Renderer.h"
#include "libGLESv2/renderer/ShaderExecutable.h"
#include "libGLESv2/renderer/d3d/DynamicHLSL.h"
#include "libGLESv2/main.h"

namespace rx
{

ProgramBinaryD3D::ProgramBinaryD3D(rx::Renderer *renderer)
    : ProgramBinaryImpl(),
      mRenderer(renderer),
      mDynamicHLSL(NULL),
      mVertexUniformStorage(NULL),
      mFragmentUniformStorage(NULL)
{
    mDynamicHLSL = new rx::DynamicHLSL(renderer);
}

ProgramBinaryD3D::~ProgramBinaryD3D()
{
    SafeDelete(mDynamicHLSL);
}

ProgramBinaryD3D *ProgramBinaryD3D::makeProgramBinaryD3D(ProgramBinaryImpl *impl)
{
    ASSERT(HAS_DYNAMIC_TYPE(ProgramBinaryD3D*, impl));
    return static_cast<ProgramBinaryD3D*>(impl);
}

const ProgramBinaryD3D *ProgramBinaryD3D::makeProgramBinaryD3D(const ProgramBinaryImpl *impl)
{
    ASSERT(HAS_DYNAMIC_TYPE(const ProgramBinaryD3D*, impl));
    return static_cast<const ProgramBinaryD3D*>(impl);
}

void ProgramBinaryD3D::initializeUniformStorage(const std::vector<gl::LinkedUniform*> &uniforms)
{
    // Compute total default block size
    unsigned int vertexRegisters = 0;
    unsigned int fragmentRegisters = 0;
    for (size_t uniformIndex = 0; uniformIndex < uniforms.size(); uniformIndex++)
    {
        const gl::LinkedUniform &uniform = *uniforms[uniformIndex];

        if (!gl::IsSampler(uniform.type))
        {
            if (uniform.isReferencedByVertexShader())
            {
                vertexRegisters = std::max(vertexRegisters, uniform.vsRegisterIndex + uniform.registerCount);
            }
            if (uniform.isReferencedByFragmentShader())
            {
                fragmentRegisters = std::max(fragmentRegisters, uniform.psRegisterIndex + uniform.registerCount);
            }
        }
    }

    mVertexUniformStorage = mRenderer->createUniformStorage(vertexRegisters * 16u);
    mFragmentUniformStorage = mRenderer->createUniformStorage(fragmentRegisters * 16u);
}

void ProgramBinaryD3D::reset()
{
    SafeDelete(mVertexUniformStorage);
    SafeDelete(mFragmentUniformStorage);
}

}
