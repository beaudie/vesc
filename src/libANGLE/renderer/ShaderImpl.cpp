//
// Copyright (c) 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderImpl.cpp: Implementation methods of ShaderImpl

#include "libANGLE/renderer/ShaderImpl.h"

#include "libANGLE/WorkerThread.h"

namespace rx
{

bool ShaderImpl::hasNativeParallelCompile()
{
    return false;
}

std::shared_ptr<angle::WaitableEvent> ShaderImpl::compileNativeParallel(const std::string &source)
{
    return std::make_shared<angle::WaitableEventDone>();
}

}  // namespace rx
