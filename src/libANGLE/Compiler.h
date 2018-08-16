//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Compiler.h: Defines the gl::Compiler class, abstracting the ESSL compiler
// that a GL context holds.

#ifndef LIBANGLE_COMPILER_H_
#define LIBANGLE_COMPILER_H_

#include <list>

#include "GLSLANG/ShaderLang.h"
#include "common/PackedEnums.h"
#include "libANGLE/Error.h"
#include "libANGLE/RefCountObject.h"

namespace rx
{
class CompilerImpl;
class GLImplFactory;
}

namespace gl
{
class ContextState;
class CompilerInstance;

class Compiler final : public RefCountObjectNoID
{
  public:
    Compiler(rx::GLImplFactory *implFactory, const ContextState &data);

    std::unique_ptr<CompilerInstance> getInstance(ShaderType shaderType);
    void putInstance(std::unique_ptr<CompilerInstance> instance);
    ShShaderOutput getShaderOutputType() const { return mOutputType; }

  private:
    ~Compiler() override;
    std::unique_ptr<rx::CompilerImpl> mImplementation;
    ShShaderSpec mSpec;
    ShShaderOutput mOutputType;
    ShBuiltInResources mResources;
    ShaderMap<std::list<std::unique_ptr<CompilerInstance>>> mPools;
    // To know when to call sh::Initialize and sh::Finalize.
    static unsigned int mActiveCompilers;
};

class CompilerInstance final : public angle::NonCopyable
{
  public:
    CompilerInstance(ShHandle handle, ShShaderOutput outputType, ShaderType shaderType);
    ~CompilerInstance();

    ShHandle getHandle();
    ShaderType getShaderType() const;
    const std::string &getBuiltinResourcesString();
    ShShaderOutput getShaderOutputType() const;

  private:
    ShHandle mHandle;
    ShShaderOutput mOutputType;
    ShaderType mShaderType;
};

}  // namespace gl

#endif  // LIBANGLE_COMPILER_H_
