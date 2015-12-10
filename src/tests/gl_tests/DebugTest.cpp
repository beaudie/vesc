//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DebugTest.cpp : Tests of the GL_KHR_debug extension

#include "test_utils/ANGLETest.h"

namespace angle
{

class DebugTest : public ANGLETest
{
  protected:
    DebugTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setDebugEnabled(true);
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        mDebugExtensionAvailable = extensionEnabled("GL_KHR_debug");
        if (mDebugExtensionAvailable)
        {
            glEnable(GL_DEBUG_OUTPUT);
        }
    }

    virtual void TearDown() { ANGLETest::TearDown(); }

    bool mDebugExtensionAvailable;
};

// Test a basic flow of inserting a message and reading it back
TEST_P(DebugTest, InsertMessage)
{
    if (!mDebugExtensionAvailable)
    {
        std::cout << "Test skipped because GL_KHR_debug is not available." << std::endl;
        return;
    }

    const GLenum source       = GL_DEBUG_SOURCE_APPLICATION;
    const GLenum type         = GL_DEBUG_TYPE_OTHER;
    const GLuint id           = 1;
    const GLenum severity     = GL_DEBUG_SEVERITY_NOTIFICATION;
    const std::string message = "Message";

    glDebugMessageInsertKHR(source, type, id, severity, -1, message.c_str());

    GLint numMessages = 0;
    glGetIntegerv(GL_DEBUG_LOGGED_MESSAGES, &numMessages);
    ASSERT_EQ(1, numMessages);

    GLint messageLength = 0;
    glGetIntegerv(GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH, &messageLength);
    EXPECT_EQ(static_cast<GLint>(message.length()), messageLength);

    GLenum sourceBuf   = 0;
    GLenum typeBuf     = 0;
    GLenum idBuf       = 0;
    GLenum severityBuf = 0;
    GLsizei lengthBuf = 0;
    std::vector<char> messageBuf(messageLength + 1);
    GLuint ret = glGetDebugMessageLogKHR(1, messageBuf.size(), &sourceBuf, &typeBuf, &idBuf,
                                         &severityBuf, &lengthBuf, messageBuf.data());
    EXPECT_EQ(1u, ret);
    EXPECT_EQ(source, sourceBuf);
    EXPECT_EQ(type, typeBuf);
    EXPECT_EQ(id, idBuf);
    EXPECT_EQ(severity, severityBuf);
    EXPECT_EQ(lengthBuf, messageLength);
    EXPECT_STREQ(message.c_str(), messageBuf.data());

    glGetIntegerv(GL_DEBUG_LOGGED_MESSAGES, &numMessages);
    EXPECT_EQ(0, numMessages);

    ASSERT_GL_NO_ERROR();
}

TEST_P(DebugTest, InsertMessageMultiple)
{
    if (!mDebugExtensionAvailable)
    {
        std::cout << "Test skipped because GL_KHR_debug is not available." << std::endl;
        return;
    }

    const GLenum source       = GL_DEBUG_SOURCE_APPLICATION;
    const GLenum type         = GL_DEBUG_TYPE_OTHER;
    const GLuint startID      = 1;
    const GLenum severity     = GL_DEBUG_SEVERITY_NOTIFICATION;
    const std::string message = "Message";
    const size_t messageCount = 32;

    for (size_t i = 0; i < messageCount; i++)
    {
        glDebugMessageInsertKHR(source, type, startID + i, severity, -1, message.c_str());
    }

    GLint numMessages = 0;
    glGetIntegerv(GL_DEBUG_LOGGED_MESSAGES, &numMessages);
    ASSERT_EQ(static_cast<GLint>(messageCount), numMessages);

    GLint messageLength = 0;
    glGetIntegerv(GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH, &messageLength);
    EXPECT_EQ(static_cast<GLint>(message.length()), messageLength);

    for (size_t i = 0; i < messageCount; i++)
    {
        glGetIntegerv(GL_DEBUG_LOGGED_MESSAGES, &numMessages);
        EXPECT_EQ(messageCount - i, numMessages);

        GLenum sourceBuf   = 0;
        GLenum typeBuf     = 0;
        GLenum idBuf       = 0;
        GLenum severityBuf = 0;
        GLsizei lengthBuf = 0;
        std::vector<char> messageBuf(messageLength + 1);
        GLuint ret = glGetDebugMessageLogKHR(1, messageBuf.size(), &sourceBuf, &typeBuf, &idBuf,
                                             &severityBuf, &lengthBuf, messageBuf.data());
        EXPECT_EQ(1u, ret);
        EXPECT_EQ(source, sourceBuf);
        EXPECT_EQ(type, typeBuf);
        EXPECT_EQ(startID + i, idBuf);
        EXPECT_EQ(severity, severityBuf);
        EXPECT_EQ(lengthBuf, messageLength);
        EXPECT_STREQ(message.c_str(), messageBuf.data());
    }

    glGetIntegerv(GL_DEBUG_LOGGED_MESSAGES, &numMessages);
    EXPECT_EQ(0, numMessages);

    ASSERT_GL_NO_ERROR();
}

struct Message
{
    GLenum source;
    GLenum type;
    GLuint id;
    GLenum severity;
    std::string message;
    const void *userParam;
};

static void GL_APIENTRY Callback(GLenum source,
                                 GLenum type,
                                 GLuint id,
                                 GLenum severity,
                                 GLsizei length,
                                 const GLchar *message,
                                 const void *userParam)
{
    Message m{source, type, id, severity, std::string(message, length), userParam};
    std::vector<Message> *messages =
        reinterpret_cast<std::vector<Message> *>(const_cast<void *>(userParam));
    messages->push_back(m);
}

TEST_P(DebugTest, DebugCallback)
{
    if (!mDebugExtensionAvailable)
    {
        std::cout << "Test skipped because GL_KHR_debug is not available." << std::endl;
        return;
    }

    std::vector<Message> messages;

    glDebugMessageCallbackKHR(Callback, &messages);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    const GLenum source       = GL_DEBUG_SOURCE_APPLICATION;
    const GLenum type         = GL_DEBUG_TYPE_OTHER;
    const GLuint id           = 1;
    const GLenum severity     = GL_DEBUG_SEVERITY_NOTIFICATION;
    const std::string message = "Message";

    glDebugMessageInsertKHR(source, type, id, severity, -1, message.c_str());

    GLint numMessages = 0;
    glGetIntegerv(GL_DEBUG_LOGGED_MESSAGES, &numMessages);
    EXPECT_EQ(0, numMessages);

    ASSERT_EQ(1u, messages.size());

    const Message &m = messages.front();
    EXPECT_EQ(source, m.source);
    EXPECT_EQ(type, m.type);
    EXPECT_EQ(id, m.id);
    EXPECT_EQ(severity, m.severity);
    EXPECT_EQ(message, m.message);

    glGetIntegerv(GL_DEBUG_LOGGED_MESSAGES, &numMessages);
    EXPECT_EQ(0, numMessages);

    ASSERT_GL_NO_ERROR();
}

// Test usage of message control.  Example taken from GL_KHR_debug spec.
TEST_P(DebugTest, MessageControl1)
{
    if (!mDebugExtensionAvailable)
    {
        std::cout << "Test skipped because GL_KHR_debug is not available." << std::endl;
        return;
    }

    std::vector<Message> messages;

    glDebugMessageCallbackKHR(Callback, &messages);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    // Setup of the default active debug group: Filter everything in
    glDebugMessageControlKHR(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);

    // Generate a debug marker debug output message
    glDebugMessageInsertKHR(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 100,
                            GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Message 1");

    // Push debug group 1
    glPushDebugGroupKHR(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Message 2");

    // Setup of the debug group 1: Filter everything out
    glDebugMessageControlKHR(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);

    // This message won't appear in the debug output log of
    glDebugMessageInsertKHR(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 100,
                            GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Message 3");

    // Pop debug group 1, restore the volume control of the default debug group.
    glPopDebugGroupKHR();

    // Generate a debug marker debug output message
    glDebugMessageInsertKHR(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 100,
                            GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Message 5");

    // Expected debug output from the GL implementation
    // Message 1
    // Message 2
    // Message 2
    // Message 5
    EXPECT_EQ(4u, messages.size());
    EXPECT_STREQ(messages[0].message.c_str(), "Message 1");
    EXPECT_STREQ(messages[1].message.c_str(), "Message 2");
    EXPECT_STREQ(messages[2].message.c_str(), "Message 2");
    EXPECT_STREQ(messages[3].message.c_str(), "Message 5");
}

// Test usage of message control.  Example taken from GL_KHR_debug spec.
TEST_P(DebugTest, MessageControl2)
{
    if (!mDebugExtensionAvailable)
    {
        std::cout << "Test skipped because GL_KHR_debug is not available." << std::endl;
        return;
    }

    std::vector<Message> messages;

    glDebugMessageCallbackKHR(Callback, &messages);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    // Setup the control of de debug output for the default debug group
    glDebugMessageControlKHR(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
    glDebugMessageControlKHR(GL_DEBUG_SOURCE_THIRD_PARTY, GL_DONT_CARE, GL_DONT_CARE, 0, NULL,
                             GL_FALSE);
    std::vector<GLuint> ids0 = {1234, 2345, 3456, 4567};
    glDebugMessageControlKHR(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE,
                             static_cast<GLuint>(ids0.size()), ids0.data(), GL_FALSE);
    glDebugMessageControlKHR(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_PORTABILITY, GL_DONT_CARE,
                             static_cast<GLuint>(ids0.size()), ids0.data(), GL_FALSE);

    // Push debug group 1
    // Inherit of the default debug group debug output volume control
    // Filtered out by glDebugMessageControl
    glPushDebugGroupKHR(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Message 1");

    // In this section of the code, we are interested in performances.
    glDebugMessageControlKHR(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_PERFORMANCE, GL_DONT_CARE,
                             0, NULL, GL_TRUE);
    // But we already identify that some messages are not really useful for us.
    std::vector<GLuint> ids1 = {5678, 6789};
    glDebugMessageControlKHR(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE,
                             static_cast<GLuint>(ids1.size()), ids1.data(), GL_FALSE);

    glDebugMessageInsertKHR(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_PERFORMANCE, 1357,
                            GL_DEBUG_SEVERITY_MEDIUM, -1, "Message 2");
    glDebugMessageInsertKHR(GL_DEBUG_SOURCE_THIRD_PARTY,  // We still filter out these messages.
                            GL_DEBUG_TYPE_OTHER, 3579, GL_DEBUG_SEVERITY_MEDIUM, -1, "Message 3");

    glPopDebugGroupKHR();

    // Expected debug output from the GL implementation
    // Message 2
    EXPECT_EQ(1u, messages.size());
    EXPECT_STREQ(messages[0].message.c_str(), "Message 2");
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(DebugTest, ES2_D3D9(), ES2_D3D11(), ES3_D3D11(), ES2_OPENGL(), ES3_OPENGL());
}
