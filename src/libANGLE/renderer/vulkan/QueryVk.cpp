//
// Copyright 2016-2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// QueryVk.cpp:
//    Implements the class methods for QueryVk.
//

#include "libANGLE/renderer/vulkan/QueryVk.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"

#include "common/debug.h"

namespace rx
{

QueryVk::QueryVk(gl::QueryType type) : QueryImpl(type)
{
}

QueryVk::~QueryVk() = default;

gl::Error QueryVk::onDestroy(const gl::Context *context)
{
    ContextVk *contextVk = static_cast<ContextVk *>(context->getImplementation());
    contextVk->getOcclusionQueryPool()->freeQuery(contextVk, &mQueryHelper);

    return gl::NoError();
}

gl::Error QueryVk::begin(const gl::Context *context)
{
    ContextVk *contextVk = static_cast<ContextVk *>(context->getImplementation());

    angle::Result result =
        contextVk->getOcclusionQueryPool()->allocateQuery(contextVk, &mQueryHelper);

    if (result != angle::Result::Continue())
        return gl::InternalError();

    mQueryHelper.beginQuery(contextVk, &mQueryHelper);

    return gl::NoError();
}

gl::Error QueryVk::end(const gl::Context *context)
{
    ContextVk *contextVk = static_cast<ContextVk *>(context->getImplementation());

    mQueryHelper.endQuery(contextVk, &mQueryHelper);

    return gl::NoError();
}

gl::Error QueryVk::queryCounter(const gl::Context *context)
{
    UNIMPLEMENTED();
    return gl::InternalError();
}

gl::Error QueryVk::getResult(const gl::Context *context, void *params, size_t paramsSize)
{
    ContextVk *contextVk = static_cast<ContextVk *>(context->getImplementation());

    angle::Result result = mQueryHelper.getQueryPool()->getResults(
        contextVk, mQueryHelper.getQuery(), 1, paramsSize, params, paramsSize,
        VK_QUERY_RESULT_WAIT_BIT | (paramsSize >= sizeof(uint64_t) ? VK_QUERY_RESULT_64_BIT : 0));

    return result == angle::Result::Stop() ? gl::InternalError() : gl::NoError();
}

gl::Error QueryVk::getResult(const gl::Context *context, GLint *params)
{
    static_assert(sizeof(GLint) >= sizeof(uint32_t),
                  "Vulkan expects a minimum of 32-bits for the results");
    return getResult(context, params, sizeof(GLint));
}

gl::Error QueryVk::getResult(const gl::Context *context, GLuint *params)
{
    static_assert(sizeof(GLuint) >= sizeof(uint32_t),
                  "Vulkan expects a minimum of 32-bits for the results");
    return getResult(context, params, sizeof(GLuint));
}

gl::Error QueryVk::getResult(const gl::Context *context, GLint64 *params)
{
    return getResult(context, params, sizeof(GLint64));
}

gl::Error QueryVk::getResult(const gl::Context *context, GLuint64 *params)
{
    return getResult(context, params, sizeof(GLuint64));
}

gl::Error QueryVk::isResultAvailable(const gl::Context *context, bool *available)
{
    ContextVk *contextVk = static_cast<ContextVk *>(context->getImplementation());

    // Make sure the command buffer for this query is submitted.  If not, *available should always
    // be false. This is because the reset command is not yet executed (it's only put in the command
    // graph), so actually checking the results may return "true" because of a previous submission.

    if (mQueryHelper.hasPendingWork(contextVk->getRenderer()))
    {
        *available = false;
        return gl::NoError();
    }

    uint32_t unusedResult;
    angle::Result result = mQueryHelper.getQueryPool()->getResults(
        contextVk, mQueryHelper.getQuery(), 1, sizeof(uint32_t), &unusedResult, sizeof(uint32_t),
        0);

    if (result == angle::Result::Stop())
    {
        *available = false;
        return gl::InternalError();
    }

    *available = result == angle::Result::Continue();
    return gl::NoError();
}

}  // namespace rx
