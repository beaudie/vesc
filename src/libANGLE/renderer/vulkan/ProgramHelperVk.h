//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramHelperVk.h: Defines the interface that ProgramVk and ProgramPipelineVk
// must implement to help users of those objects interact with them.

#ifndef LIBANGLE_PROGRAMHELPERVK_H_
#define LIBANGLE_PROGRAMHELPERVK_H_

#include "common/PackedEnums.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/glslang_wrapper_utils.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"

namespace rx
{

class ShaderInfo final : angle::NonCopyable
{
  public:
    ShaderInfo();
    ~ShaderInfo();

    angle::Result initShaders(ContextVk *contextVk,
                              const gl::ShaderMap<std::string> &shaderSources,
                              const ShaderInterfaceVariableInfoMap &variableInfoMap,
                              gl::ShaderMap<SpirvBlob> *spirvBlobsOut);
    void release(ContextVk *contextVk);

    ANGLE_INLINE bool valid() const { return mIsInitialized; }

    gl::ShaderMap<SpirvBlob> &getSpirvBlobs() { return mSpirvBlobs; }
    const gl::ShaderMap<SpirvBlob> &getSpirvBlobs() const { return mSpirvBlobs; }

  private:
    gl::ShaderMap<SpirvBlob> mSpirvBlobs;
    bool mIsInitialized = false;
};

class ProgramInfo final : angle::NonCopyable
{
  public:
    ProgramInfo();
    ~ProgramInfo();

    angle::Result initProgram(ContextVk *contextVk,
                              const ShaderInfo &shaderInfo,
                              bool enableLineRasterEmulation);
    void release(ContextVk *contextVk);

    ANGLE_INLINE bool valid() const { return mProgramHelper.valid(); }

    vk::ShaderProgramHelper *getShaderProgram() { return &mProgramHelper; }

  private:
    vk::ShaderProgramHelper mProgramHelper;
    gl::ShaderMap<vk::RefCounted<vk::ShaderAndSerial>> mShaders;
};

class ProgramHelperVk
{
  public:
    virtual ~ProgramHelperVk() = 0;

  protected:
};

}  // namespace rx

#endif  // LIBANGLE_PROGRAMHELPERVK_H_
