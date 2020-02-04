//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/vulkan/ProgramHelperVk.h"

#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/GlslangWrapperVk.h"

namespace rx
{

// ShaderInfo implementation.
ShaderInfo::ShaderInfo() {}

ShaderInfo::~ShaderInfo() = default;

angle::Result ShaderInfo::initShaders(ContextVk *contextVk,
                                      const gl::ShaderMap<std::string> &shaderSources,
                                      const ShaderInterfaceVariableInfoMap &variableInfoMap,
                                      gl::ShaderMap<SpirvBlob> *spirvBlobsOut)
{
    ASSERT(!valid());

    ANGLE_TRY(GlslangWrapperVk::GetShaderCode(contextVk, contextVk->getCaps(), shaderSources,
                                              variableInfoMap, spirvBlobsOut));

    mIsInitialized = true;
    return angle::Result::Continue;
}

void ShaderInfo::release(ContextVk *contextVk)
{
    for (SpirvBlob &spirvBlob : mSpirvBlobs)
    {
        spirvBlob.clear();
    }
    mIsInitialized = false;
}

// ProgramInfo implementation.
ProgramInfo::ProgramInfo() {}

ProgramInfo::~ProgramInfo() = default;

angle::Result ProgramInfo::initProgram(ContextVk *contextVk,
                                       const ShaderInfo &shaderInfo,
                                       bool enableLineRasterEmulation)
{
    const gl::ShaderMap<SpirvBlob> &spirvBlobs = shaderInfo.getSpirvBlobs();

    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const SpirvBlob &spirvBlob = spirvBlobs[shaderType];

        if (!spirvBlob.empty())
        {
            ANGLE_TRY(vk::InitShaderAndSerial(contextVk, &mShaders[shaderType].get(),
                                              spirvBlob.data(),
                                              spirvBlob.size() * sizeof(uint32_t)));

            mProgramHelper.setShader(shaderType, &mShaders[shaderType]);
        }
    }

    if (enableLineRasterEmulation)
    {
        mProgramHelper.enableSpecializationConstant(
            sh::vk::SpecializationConstantId::LineRasterEmulation);
    }

    return angle::Result::Continue;
}

void ProgramInfo::release(ContextVk *contextVk)
{
    mProgramHelper.release(contextVk);

    for (vk::RefCounted<vk::ShaderAndSerial> &shader : mShaders)
    {
        shader.get().destroy(contextVk->getDevice());
    }
}

ProgramHelperVk::ProgramHelperVk()
    : mStorageBlockBindingsOffset(0),
      mAtomicCounterBufferBindingsOffset(0),
      mImageBindingsOffset(0),
      mDynamicBufferOffsets{}
{}

ProgramHelperVk::~ProgramHelperVk() {}

}  // namespace rx
