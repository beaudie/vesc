//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Debug.cpp: Defines debug state used for GL_KHR_debug

#include "libANGLE/Debug.h"

#include "common/debug.h"

namespace gl
{

Debug::Debug()
    : mOutputEnabled(false),
      mCallbackFunction(nullptr),
      mCallbackUserParam(nullptr),
      mMessages(),
      mMaxLoggedMessages(0),
      mOutputSynchronous(false),
      mGroups()
{
}

void Debug::setMaxLoggedMessages(GLuint maxLoggedMessages)
{
    mMaxLoggedMessages = maxLoggedMessages;
}

void Debug::setOutputEnabled(bool enabled)
{
    mOutputEnabled = enabled;
}

bool Debug::isOutputEnabled() const
{
    return mOutputEnabled;
}

void Debug::setOutputSynchronous(bool synchronous)
{
    mOutputSynchronous = synchronous;
}

bool Debug::isOutputSynchronous() const
{
    return mOutputSynchronous;
}

void Debug::setCallback(GLDEBUGPROCKHR callback, const void *userParam)
{
    mCallbackFunction  = callback;
    mCallbackUserParam = userParam;
}

GLDEBUGPROCKHR Debug::getCallback() const
{
    return mCallbackFunction;
}

const void *Debug::getUserParam() const
{
    return mCallbackUserParam;
}

void Debug::insertMessage(GLenum source,
                          GLenum type,
                          GLuint id,
                          GLenum severity,
                          const std::string &message)
{
    if (!isMessageEnabled(source, type, id, severity))
    {
        return;
    }

    if (mCallbackFunction != nullptr)
    {
        // TODO(geofflang) Check the synchronous flag and potentially flush messages from another
        // thread.
        mCallbackFunction(source, type, id, severity, message.length(), message.c_str(),
                          mCallbackUserParam);
    }
    else
    {
        Message m;
        m.source   = source;
        m.type     = type;
        m.id       = id;
        m.severity = severity;
        m.message  = message;

        mMessages.push_back(m);
    }
}

size_t Debug::getMessages(GLuint count,
                          GLsizei bufSize,
                          GLenum *sources,
                          GLenum *types,
                          GLuint *ids,
                          GLenum *severities,
                          GLsizei *lengths,
                          GLchar *messageLog)
{
    size_t messageCount       = 0;
    size_t messageStringIndex = 0;
    while (messageCount <= count && !mMessages.empty())
    {
        const Message &m = mMessages.front();

        if (messageLog != nullptr)
        {
            // Check that this message can fit in the message buffer
            if (messageStringIndex + m.message.length() + 1 > static_cast<size_t>(bufSize))
            {
                break;
            }

            std::copy(m.message.begin(), m.message.end(), messageLog + messageStringIndex);
            messageStringIndex += m.message.length();

            messageLog[messageStringIndex] = '\0';
            messageStringIndex += 1;
        }

        if (sources != nullptr)
        {
            sources[messageCount] = m.source;
        }

        if (types != nullptr)
        {
            types[messageCount] = m.type;
        }

        if (ids != nullptr)
        {
            ids[messageCount] = m.id;
        }

        if (severities != nullptr)
        {
            severities[messageCount] = m.severity;
        }

        if (lengths != nullptr)
        {
            lengths[messageCount] = m.message.length();
        }

        messageCount++;
    }

    return messageCount;
}

size_t Debug::getNextMessageLength() const
{
    return mMessages.empty() ? 0 : mMessages.front().message.length();
}

size_t Debug::getMessageCount() const
{
    return mMessages.size();
}

void Debug::setMessageControl(GLenum source,
                              GLenum type,
                              GLenum severity,
                              GLsizei count,
                              const GLuint *ids,
                              GLboolean enabled)
{
    // TODO
}

void Debug::pushGroup(GLenum source, GLuint id, const std::string &message)
{
    Group g;
    g.source  = source;
    g.id      = id;
    g.message = message;
    mGroups.push(g);

    insertMessage(source, GL_DEBUG_TYPE_PUSH_GROUP, id, GL_DEBUG_SEVERITY_NOTIFICATION, message);
}

void Debug::popGroup()
{
    ASSERT(!mGroups.empty());

    const Group &g = mGroups.top();
    insertMessage(g.source, GL_DEBUG_TYPE_POP_GROUP, g.id, GL_DEBUG_SEVERITY_NOTIFICATION,
                  g.message);

    mGroups.pop();
}

size_t Debug::getGroupStackDepth() const
{
    // Add an additional stack item for the default debug group
    return mGroups.size() + 1;
}

bool Debug::isMessageEnabled(GLenum source, GLenum type, GLuint id, GLenum severity) const
{
    // TODO
    return mOutputEnabled;
}
}
