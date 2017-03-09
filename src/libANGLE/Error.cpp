//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Error.cpp: Implements the egl::Error and gl::Error classes which encapsulate API errors
// and optional error messages.

#include "libANGLE/Error.h"

#include "common/angleutils.h"
#include "common/debug.h"

#include <cstdarg>

namespace gl
{

Error::Error(GLenum errorCode, GLuint id, std::string &&message)
    : mCode(errorCode), mID(id), mMessage(new std::string(std::move(message)))
{
}

void Error::createMessageString() const
{
    if (!mMessage)
    {
        mMessage.reset(new std::string);
    }
}

const std::string &Error::getMessage() const
{
    createMessageString();
    return *mMessage;
}

bool Error::operator==(const Error &other) const
{
    if (mCode != other.mCode)
        return false;

    // TODO(jmadill): Compare extended error codes instead of strings.
    if ((!mMessage || !other.mMessage) && (!mMessage != !other.mMessage))
        return false;

    return (*mMessage == *other.mMessage);
}

bool Error::operator!=(const Error &other) const
{
    return !(*this == other);
}

std::ostream &operator<<(std::ostream &os, const Error &err)
{
    return gl::FmtHexShort(os, err.getCode());
}

namespace priv
{
template <GLenum EnumT>
ErrorStream<EnumT>::ErrorStream() : mID(EnumT)
{
}

template <GLenum EnumT>
ErrorStream<EnumT>::ErrorStream(GLuint id) : mID(id)
{
}

template <GLenum EnumT>
ErrorStream<EnumT>::operator gl::Error()
{
    return Error(EnumT, mID, mErrorStream.str());
}

template class ErrorStream<GL_INVALID_ENUM>;
template class ErrorStream<GL_INVALID_VALUE>;
template class ErrorStream<GL_INVALID_OPERATION>;
template class ErrorStream<GL_STACK_OVERFLOW>;
template class ErrorStream<GL_STACK_UNDERFLOW>;
template class ErrorStream<GL_OUT_OF_MEMORY>;
template class ErrorStream<GL_INVALID_FRAMEBUFFER_OPERATION>;

}  // namespace priv

}  // namespace gl

namespace egl
{

Error::Error(EGLint errorCode, EGLint id, std::string &&message)
    : mCode(errorCode), mID(id), mMessage(new std::string(std::move(message)))
{
}

void Error::createMessageString() const
{
    if (!mMessage)
    {
        mMessage.reset(new std::string);
    }
}

const std::string &Error::getMessage() const
{
    createMessageString();
    return *mMessage;
}

std::ostream &operator<<(std::ostream &os, const Error &err)
{
    return gl::FmtHexShort(os, err.getCode());
}
namespace priv
{
template <GLenum EnumT>
ErrorStream<EnumT>::ErrorStream() : mID(EnumT)
{
}

template <GLenum EnumT>
ErrorStream<EnumT>::ErrorStream(GLuint id) : mID(id)
{
}

template <GLenum EnumT>
ErrorStream<EnumT>::operator egl::Error()
{
    return Error(EnumT, mID, mErrorStream.str());
}

template class ErrorStream<EGL_NOT_INITIALIZED>;
template class ErrorStream<EGL_BAD_ACCESS>;
template class ErrorStream<EGL_BAD_ALLOC>;
template class ErrorStream<EGL_BAD_ATTRIBUTE>;
template class ErrorStream<EGL_BAD_CONFIG>;
template class ErrorStream<EGL_BAD_CONTEXT>;
template class ErrorStream<EGL_BAD_CURRENT_SURFACE>;
template class ErrorStream<EGL_BAD_DISPLAY>;
template class ErrorStream<EGL_BAD_MATCH>;
template class ErrorStream<EGL_BAD_NATIVE_WINDOW>;
template class ErrorStream<EGL_BAD_PARAMETER>;
template class ErrorStream<EGL_BAD_SURFACE>;
template class ErrorStream<EGL_CONTEXT_LOST>;
template class ErrorStream<EGL_BAD_STREAM_KHR>;
template class ErrorStream<EGL_BAD_STATE_KHR>;
template class ErrorStream<EGL_BAD_DEVICE_EXT>;

}  // namespace priv

}  // namespace egl
