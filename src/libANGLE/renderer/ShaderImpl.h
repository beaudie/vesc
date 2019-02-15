//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderImpl.h: Defines the abstract rx::ShaderImpl class.

#ifndef LIBANGLE_RENDERER_SHADERIMPL_H_
#define LIBANGLE_RENDERER_SHADERIMPL_H_

#include "common/angleutils.h"
#include "libANGLE/Shader.h"

namespace angle
{
class WaitableEvent;
}

namespace rx
{

class WaitableCompileEventImpl : public angle::WaitableEvent
{
  public:
    WaitableCompileEventImpl(std::shared_ptr<WaitableEvent> waitableEvent,
                             std::shared_ptr<gl::TranslateTask> translateTask,
                             gl::PostTranslateFunctor &&postTranslateFunctor,
                             gl::PostTranslateImplFunctor &&postTranslateImplFunctor);
    ~WaitableCompileEventImpl() override;

    void wait() override;

    bool isReady() override;

  protected:
    std::shared_ptr<WaitableEvent> mWaitableEvent;
    std::shared_ptr<gl::TranslateTask> mTranslateTask;
    gl::PostTranslateFunctor mPostTranslateFunctor;
    gl::PostTranslateImplFunctor mPostTranslateImplFunctor;
};

class ShaderImpl : angle::NonCopyable
{
  public:
    ShaderImpl(const gl::ShaderState &data) : mData(data) {}
    virtual ~ShaderImpl() {}

    virtual void destroy() {}

    virtual std::shared_ptr<angle::WaitableEvent> compile(
        gl::TranslateTaskConstructor &&translateTaskConstructor,
        gl::PostTranslateFunctor &&postTranslateFunctor,
        const gl::Context *context) = 0;

    virtual std::string getDebugInfo() const = 0;

    const gl::ShaderState &getData() const { return mData; }

  protected:
    std::shared_ptr<angle::WaitableEvent> compileImpl(
        gl::TranslateTaskConstructor &&translateTaskConstructor,
        gl::PostTranslateFunctor &&postTranslateFunctor,
        const gl::Context *context,
        std::stringstream &&shaderSourceStream,
        ShCompileOptions compileOptions);

    const gl::ShaderState &mData;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_SHADERIMPL_H_
