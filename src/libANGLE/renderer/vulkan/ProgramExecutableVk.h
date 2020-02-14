//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramExecutableVk.h: Collects the information and interfaces common to both ProgramVks and
// ProgramPipelineVks in order to execute/draw with either.

#ifndef LIBANGLE_RENDERER_VULKAN_PROGRAMEXECUTABLEVK_H_
#define LIBANGLE_RENDERER_VULKAN_PROGRAMEXECUTABLEVK_H_

#include "common/utilities.h"
#include "libANGLE/renderer/glslang_wrapper_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{

class ProgramExecutableVk
{
  public:
    ProgramExecutableVk();
    virtual ~ProgramExecutableVk();

    void reset();

    void clearVariableInfoMap();
    ShaderInterfaceVariableInfoMap &getShaderInterfaceVariableInfoMap() { return mVariableInfoMap; }

  private:
    friend class ProgramVk;
    friend class ProgramPipelineVk;

    ShaderInterfaceVariableInfoMap mVariableInfoMap;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_PROGRAMEXECUTABLEVK_H_
