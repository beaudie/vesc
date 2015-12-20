//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Error.inl: Inline definitions of egl::Error and gl::Error classes which encapsulate API errors
// and optional error messages.

#include "common/angleutils.h"

#include <cstdarg>

namespace gl
{

Error::Error(GLenum errorCode)
    : mCode(errorCode)
{
}

Error::Error(const Error &other)
    : mCode(other.mCode)
{
    if (other.mMessage.get() != nullptr)
    {
        createMessageString();
        *mMessage = *(other.mMessage);
    }
}

Error::Error(Error &&other)
    : mCode(other.mCode),
      mMessage(std::move(other.mMessage))
{
}

Error &Error::operator=(const Error &other)
{
    mCode = other.mCode;

    if (other.mMessage.get() != nullptr)
    {
        createMessageString();
        *mMessage = *(other.mMessage);
    }
    else
    {
        mMessage.release();
    }

    return *this;
}

Error &Error::operator=(Error &&other)
{
    if (this != &other)
    {
        mCode = other.mCode;
        mMessage = std::move(other.mMessage);
    }

    return *this;
}

GLenum Error::getCode() const
{
    return mCode;
}

bool Error::isError() const
{
    return (mCode != GL_NO_ERROR);
}

}

namespace egl
{

Error::Error(EGLint errorCode)
    : mCode(errorCode),
      mID(0)
{
}

Error::Error(const Error &other)
    : mCode(other.mCode),
      mID(other.mID)
{
    if (other.mMessage.get() != nullptr)
    {
        createMessageString();
        *mMessage = *(other.mMessage);
    }
}

Error::Error(Error &&other)
    : mCode(other.mCode),
      mID(other.mID),
      mMessage(std::move(other.mMessage))
{
}

Error &Error::operator=(const Error &other)
{
    mCode = other.mCode;
    mID = other.mID;

    if (other.mMessage != nullptr)
    {
        createMessageString();
        *mMessage = *(other.mMessage);
    }
    else
    {
        mMessage.release();
    }

    return *this;
}

Error &Error::operator=(Error &&other)
{
    if (this != &other)
    {
        mCode = other.mCode;
        mID = other.mID;
        mMessage = std::move(other.mMessage);
    }

    return *this;
}

EGLint Error::getCode() const
{
    return mCode;
}

EGLint Error::getID() const
{
    return mID;
}

bool Error::isError() const
{
    return (mCode != EGL_SUCCESS);
}

}
