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

QueryVk::QueryVk(gl::QueryType type) : QueryImpl(type), mCachedResult(0), mCachedResultValid(false)
{
}

QueryVk::~QueryVk() = default;

gl::Error QueryVk::onDestroy(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);
    contextVk->getQueryPool(getType())->freeQuery(contextVk, &mQueryHelper);
    contextVk->getQueryPool(getType())->freeQuery(contextVk, &mQueryHelperTimeElapsedEnd);

    return gl::NoError();
}

gl::Error QueryVk::begin(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);

    mCachedResultValid = false;

    if (!mQueryHelper.getQueryPool())
    {
        ANGLE_TRY(contextVk->getQueryPool(getType())->allocateQuery(contextVk, &mQueryHelper));
    }

    // Note: TimeElapsed is implemented by using two Timestamp queries and taking the diff.
    if (getType() == gl::QueryType::TimeElapsed)
    {
        if (!mQueryHelperTimeElapsedEnd.getQueryPool())
        {
            ANGLE_TRY(contextVk->getQueryPool(getType())->allocateQuery(
                contextVk, &mQueryHelperTimeElapsedEnd));
        }

fprintf(stderr, "+++ queryCounter (time elapsed begin) %p, %u\n", mQueryHelper.getQueryPool(), mQueryHelper.getQuery());
        mQueryHelper.queryCounter(contextVk, mQueryHelper.getQueryPool(), mQueryHelper.getQuery());
    }
    else
    {
fprintf(stderr, "+++ beginQuery %p, %u\n", mQueryHelper.getQueryPool(), mQueryHelper.getQuery());
        mQueryHelper.beginQuery(contextVk, mQueryHelper.getQueryPool(), mQueryHelper.getQuery());
    }

    return gl::NoError();
}

gl::Error QueryVk::end(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);

    if (getType() == gl::QueryType::TimeElapsed)
    {
fprintf(stderr, "+++ queryCounter (time elapsed end) %p, %u\n", mQueryHelperTimeElapsedEnd.getQueryPool(), mQueryHelperTimeElapsedEnd.getQuery());
        mQueryHelperTimeElapsedEnd.queryCounter(contextVk,
                                                mQueryHelperTimeElapsedEnd.getQueryPool(),
                                                mQueryHelperTimeElapsedEnd.getQuery());
    }
    else
    {
fprintf(stderr, "+++ endQuery %p, %u\n", mQueryHelper.getQueryPool(), mQueryHelper.getQuery());
        mQueryHelper.endQuery(contextVk, mQueryHelper.getQueryPool(), mQueryHelper.getQuery());
    }

    return gl::NoError();
}

gl::Error QueryVk::queryCounter(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);

    mCachedResultValid = false;

    if (!mQueryHelper.getQueryPool())
    {
        ANGLE_TRY(contextVk->getQueryPool(getType())->allocateQuery(contextVk, &mQueryHelper));
    }

fprintf(stderr, "+++ queryCounter %p, %u\n", mQueryHelper.getQueryPool(), mQueryHelper.getQuery());
    mQueryHelper.queryCounter(contextVk, mQueryHelper.getQueryPool(), mQueryHelper.getQuery());

    return gl::NoError();
}

angle::Result QueryVk::getResult(const gl::Context *context, bool wait)
{
    if (mCachedResultValid)
    {
fprintf(stderr, "+++ returning cached result for %p, %u\n", mQueryHelper.getQueryPool(), mQueryHelper.getQuery());
        return angle::Result::Continue();
    }

    ContextVk *contextVk = vk::GetImpl(context);

    // glGetQueryObject* requires an implicit flush of the command buffers to guarantee execution in
    // finite time.
    // Note regarding time-elapsed: end should have been called after begin, so flushing when end
    // has pending work should flush begin too.
    if (getType() == gl::QueryType::TimeElapsed &&
        mQueryHelperTimeElapsedEnd.hasPendingWork(contextVk->getRenderer()))
    {
        ANGLE_TRY_HANDLE(context, contextVk->flush(context));

        ASSERT(!mQueryHelperTimeElapsedEnd.hasPendingWork(contextVk->getRenderer()));
        ASSERT(!mQueryHelper.hasPendingWork(contextVk->getRenderer()));
    }
    else if (mQueryHelper.hasPendingWork(contextVk->getRenderer()))
    {
        ANGLE_TRY_HANDLE(context, contextVk->flush(context));
        ASSERT(!mQueryHelper.hasPendingWork(contextVk->getRenderer()));
    }

    VkQueryResultFlags flags = (wait ? VK_QUERY_RESULT_WAIT_BIT : 0) | VK_QUERY_RESULT_64_BIT;

fprintf(stderr, "+++ get result for query %p, %u\n", mQueryHelper.getQueryPool(), mQueryHelper.getQuery());
    angle::Result result = mQueryHelper.getQueryPool()->getResults(
        contextVk, mQueryHelper.getQuery(), 1, sizeof(mCachedResult), &mCachedResult,
        sizeof(mCachedResult), flags);
    ANGLE_TRY(result);
fprintf(stderr, "    ... got %zu\n", (size_t)mCachedResult);

    uint64_t timeElapsedEnd = 0;
    if (result == angle::Result::Continue())
    {
        switch (getType())
        {
            case gl::QueryType::AnySamples:
            case gl::QueryType::AnySamplesConservative:
                // OpenGL query result in these cases is binary
                mCachedResult = !!mCachedResult;
                break;
            case gl::QueryType::Timestamp:
                break;
            case gl::QueryType::TimeElapsed:
fprintf(stderr, "+++ get result for time elapsed end query %p, %u\n", mQueryHelperTimeElapsedEnd.getQueryPool(), mQueryHelperTimeElapsedEnd.getQuery());
                result = mQueryHelperTimeElapsedEnd.getQueryPool()->getResults(
                    contextVk, mQueryHelperTimeElapsedEnd.getQuery(), 1, sizeof(timeElapsedEnd),
                    &timeElapsedEnd, sizeof(timeElapsedEnd), flags);
                ANGLE_TRY(result);
fprintf(stderr, "    ... got %zu\n", (size_t)timeElapsedEnd);

                mCachedResult = timeElapsedEnd - mCachedResult;
                break;
            default:
                UNREACHABLE();
                break;
        }

        if (result == angle::Result::Continue())
        {
            mCachedResultValid = true;
            fprintf(stderr, "+++ results are ready!\n");
        }
    }

    return angle::Result::Continue();
}

gl::Error QueryVk::getResult(const gl::Context *context, GLint *params)
{
    ANGLE_TRY(getResult(context, true));
    *params = static_cast<GLint>(mCachedResult);
    return gl::NoError();
}

gl::Error QueryVk::getResult(const gl::Context *context, GLuint *params)
{
    ANGLE_TRY(getResult(context, true));
    *params = static_cast<GLuint>(mCachedResult);
    return gl::NoError();
}

gl::Error QueryVk::getResult(const gl::Context *context, GLint64 *params)
{
    ANGLE_TRY(getResult(context, true));
    *params = static_cast<GLint64>(mCachedResult);
    return gl::NoError();
}

gl::Error QueryVk::getResult(const gl::Context *context, GLuint64 *params)
{
    ANGLE_TRY(getResult(context, true));
    *params = mCachedResult;
    return gl::NoError();
}

gl::Error QueryVk::isResultAvailable(const gl::Context *context, bool *available)
{
    ANGLE_TRY(getResult(context, false));
    *available = mCachedResultValid;

    return gl::NoError();
}

}  // namespace rx
