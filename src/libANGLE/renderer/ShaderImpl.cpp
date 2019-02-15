//
// Copyright (c) 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderImpl.cpp: Implementation methods of ShaderImpl

#include "libANGLE/renderer/ShaderImpl.h"

#include "libANGLE/Context.h"

namespace rx
{

WaitableCompileEvent::WaitableCompileEvent(
    std::shared_ptr<angle::WaitableEvent> waitableEvent,
    std::shared_ptr<angle::WorkerThreadPool> workerThreadPool,
    gl::ShCompilerInstance &&compilerInstance)
    : mWaitableEvent(waitableEvent),
      mWorkerThreadPool(workerThreadPool),
      mCompilerInstance(std::move(compilerInstance))
{}

WaitableCompileEvent::~WaitableCompileEvent()
{
    mWaitableEvent.reset();
    mWorkerThreadPool.reset();
}

void WaitableCompileEvent::wait()
{
    mWaitableEvent->wait();
}

bool WaitableCompileEvent::isReady()
{
    return mWaitableEvent->isReady();
}

gl::ShCompilerInstance &WaitableCompileEvent::getCompilerInstance()
{
    return mCompilerInstance;
}

const std::string &WaitableCompileEvent::getInfoLog()
{
    return mInfoLog;
}

class TranslateTask : public angle::Closure
{
  public:
    TranslateTask(ShHandle handle, ShCompileOptions options, const std::string &source)
        : mHandle(handle), mOptions(options), mSource(source), mResult(false)
    {}

    void operator()() override
    {
        std::vector<const char *> srcStrings;
        srcStrings.push_back(mSource.c_str());

        mResult = sh::Compile(mHandle, &srcStrings[0], srcStrings.size(), mOptions);
    }

    bool getResult() { return mResult; }

  private:
    ShHandle mHandle;
    ShCompileOptions mOptions;
    std::string mSource;
    bool mResult;
};

class WaitableCompileEventImpl final : public WaitableCompileEvent
{
  public:
    WaitableCompileEventImpl(std::shared_ptr<angle::WaitableEvent> waitableEvent,
                             std::shared_ptr<angle::WorkerThreadPool> workerThreadPool,
                             gl::ShCompilerInstance &&compilerInstance,
                             std::shared_ptr<TranslateTask> translateTask)
        : WaitableCompileEvent(waitableEvent, workerThreadPool, std::move(compilerInstance)),
          mTranslateTask(translateTask)
    {}

    bool getResult() override { return mTranslateTask->getResult(); }

    bool postTranslate(std::string *infoLog) override { return true; }

  private:
    std::shared_ptr<TranslateTask> mTranslateTask;
};

std::shared_ptr<WaitableCompileEvent> ShaderImpl::compileImpl(
    const gl::Context *context,
    gl::ShCompilerInstance &&compilerInstance,
    const std::string &source,
    ShCompileOptions compileOptions)
{
    auto workerThreadPool = context->getWorkerThreadPool();
    auto translateTask =
        std::make_shared<TranslateTask>(compilerInstance.getHandle(), compileOptions, source);

    return std::make_shared<WaitableCompileEventImpl>(
        workerThreadPool->postWorkerTask(translateTask), workerThreadPool,
        std::move(compilerInstance), translateTask);
}

}  // namespace rx
