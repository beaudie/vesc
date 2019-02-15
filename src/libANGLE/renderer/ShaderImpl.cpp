//
// Copyright (c) 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderImpl.cpp: Implementation methods of ShaderImpl

#include "libANGLE/renderer/ShaderImpl.h"

#include "libANGLE/Context.h"
#include "libANGLE/WorkerThread.h"

namespace rx
{

WaitableCompileEventImpl::WaitableCompileEventImpl(
    std::shared_ptr<angle::WaitableEvent> waitableEvent,
    std::shared_ptr<angle::WorkerThreadPool> workerThreadPool,
    std::shared_ptr<gl::TranslateTask> translateTask,
    gl::PostTranslateFunctor &&postTranslateFunctor,
    gl::PostTranslateImplFunctor &&postTranslateImplFunctor)
    : mWaitableEvent(waitableEvent),
      mWorkerThreadPool(workerThreadPool),
      mTranslateTask(translateTask),
      mPostTranslateFunctor(postTranslateFunctor),
      mPostTranslateImplFunctor(postTranslateImplFunctor)
{}

WaitableCompileEventImpl::~WaitableCompileEventImpl() = default;

void WaitableCompileEventImpl::wait()
{
    mWaitableEvent->wait();
    mPostTranslateFunctor(mTranslateTask, std::move(mPostTranslateImplFunctor));
}

bool WaitableCompileEventImpl::isReady()
{
    return mWaitableEvent->isReady();
}

std::shared_ptr<angle::WaitableEvent> ShaderImpl::compileImpl(
    gl::TranslateTaskConstructor &&translateTaskConstructor,
    gl::PostTranslateFunctor &&postTranslateFunctor,
    const gl::Context *context,
    std::stringstream &&shaderSourceStream,
    ShCompileOptions compileOptions)
{
    auto dummyTranslateImplFunctor = [](const std::string &, std::string *) { return true; };

    auto translateTask =
        translateTaskConstructor(compileOptions, std::move(shaderSourceStream), std::string(),
                                 std::move(dummyTranslateImplFunctor));

    auto postTranslateImplFunctor = [](std::shared_ptr<gl::TranslateTask> translateTask,
                                       std::string *infoLog) { return true; };

    auto workerThreadPool = context->getWorkerThreadPool();

    return std::make_shared<WaitableCompileEventImpl>(
        workerThreadPool->postWorkerTask(translateTask), workerThreadPool, translateTask,
        std::move(postTranslateFunctor), std::move(postTranslateImplFunctor));
}

}  // namespace rx
