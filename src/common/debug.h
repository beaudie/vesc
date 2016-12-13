//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// debug.h: Debugging utilities.

#ifndef COMMON_DEBUG_H_
#define COMMON_DEBUG_H_

#include <assert.h>
#include <stdio.h>
#include <sstream>
#include <string>

#include "common/angleutils.h"

#if !defined(TRACE_OUTPUT_FILE)
#define TRACE_OUTPUT_FILE "angle_debug.txt"
#endif

namespace gl
{

// Pairs a D3D begin event with an end event.
class ScopedPerfEventHelper : angle::NonCopyable
{
  public:
    ScopedPerfEventHelper(const char* format, ...);
    ~ScopedPerfEventHelper();
};

// Wraps the D3D9/D3D11 debug annotation functions.
class DebugAnnotator : angle::NonCopyable
{
  public:
    DebugAnnotator() { };
    virtual ~DebugAnnotator() { };
    virtual void beginEvent(const wchar_t *eventName) = 0;
    virtual void endEvent() = 0;
    virtual void setMarker(const wchar_t *markerName) = 0;
    virtual bool getStatus() = 0;
};

void InitializeDebugAnnotations(DebugAnnotator *debugAnnotator);
void UninitializeDebugAnnotations();
bool DebugAnnotationsActive();

// This class is used to explicitly ignore values in the conditional logging macros. This avoids
// compiler warnings like "value computed is not used" and "statement has no effect".
class LogMessageVoidify
{
  public:
    LogMessageVoidify() {}
    // This has to be an operator with a precedence lower than << but higher than ?:
    void operator&(std::ostream &) {}
};

// This can be any ostream, it is unused, but needs to be a valid reference.
std::ostream &DummyStream();

// Used by ANGLE_LOG_IS_ON to lazy-evaluate stream arguments.
bool ShouldCreateLogMessage(int severity);

// Sets the Log Message Handler that gets passed every log message before
// it's sent to other log destinations (if any).
// Returns true to signal that it handled the message and the message
// should not be sent to other log destinations.
typedef bool (*LogMessageHandlerFunction)(int severity,
                                          const char *function,
                                          int line,
                                          size_t message_start,
                                          const std::string &str);
void SetLogMessageHandler(LogMessageHandlerFunction handler);

typedef int LogSeverity;
// Note: the log severities are used to index into the array of names,
// see log_severity_names.
const LogSeverity LOG_EVENT          = 0;
const LogSeverity LOG_WARN           = 1;
const LogSeverity LOG_ERR            = 2;
const LogSeverity LOG_NUM_SEVERITIES = 3;

// A few definitions of macros that don't generate much code. These are used
// by ANGLE_LOG(). Since these are used all over our code, it's
// better to have compact code for these operations.
#define ANGLE_COMPACT_GOOGLE_LOG_EX_EVENT(ClassName, ...) \
    ::gl::ClassName(__FUNCTION__, __LINE__, ::gl::LOG_EVENT, ##__VA_ARGS__)
#define ANGLE_COMPACT_GOOGLE_LOG_EX_WARN(ClassName, ...) \
    ::gl::ClassName(__FUNCTION__, __LINE__, ::gl::LOG_WARN, ##__VA_ARGS__)
#define ANGLE_COMPACT_GOOGLE_LOG_EX_ERR(ClassName, ...) \
    ::gl::ClassName(__FUNCTION__, __LINE__, ::gl::LOG_ERR, ##__VA_ARGS__)

#define ANGLE_COMPACT_GOOGLE_LOG_EVENT ANGLE_COMPACT_GOOGLE_LOG_EX_EVENT(LogMessage)
#define ANGLE_COMPACT_GOOGLE_LOG_WARN ANGLE_COMPACT_GOOGLE_LOG_EX_WARN(LogMessage)
#define ANGLE_COMPACT_GOOGLE_LOG_ERR ANGLE_COMPACT_GOOGLE_LOG_EX_ERR(LogMessage)

#define ANGLE_LOG_IS_ON(severity) (::gl::ShouldCreateLogMessage(::gl::LOG_##severity))

// Helper macro which avoids evaluating the arguments to a stream if the condition doesn't hold.
// Condition is evaluated once and only once.
#define ANGLE_LAZY_STREAM(stream, condition) \
    !(condition) ? static_cast<void>(0) : ::gl::LogMessageVoidify() & (stream)

// We use the preprocessor's merging operator, "##", so that, e.g.,
// ANGLE_LOG(EVENT) becomes the token ANGLE_COMPACT_GOOGLE_LOG_EVENT.  There's some funny
// subtle difference between ostream member streaming functions (e.g.,
// ostream::operator<<(int) and ostream non-member streaming functions
// (e.g., ::operator<<(ostream&, string&): it turns out that it's
// impossible to stream something like a string directly to an unnamed
// ostream. We employ a neat hack by calling the stream() member
// function of LogMessage which seems to avoid the problem.
#define ANGLE_LOG_STREAM(severity) ANGLE_COMPACT_GOOGLE_LOG_##severity.stream()

#define ANGLE_LOG(severity) ANGLE_LAZY_STREAM(ANGLE_LOG_STREAM(severity), ANGLE_LOG_IS_ON(severity))

// This class more or less represents a particular log message.  You
// create an instance of LogMessage and then stream stuff to it.
// When you finish streaming to it, ~LogMessage is called and the
// full message gets streamed to the appropriate destination.
//
// You shouldn't actually use LogMessage's constructor to log things,
// though.  You should use the ANGLE_LOG() macro above.
class LogMessage : angle::NonCopyable
{
  public:
    // Used for ANGLE_LOG(severity).
    LogMessage(const char *function, int line, LogSeverity severity);

    ~LogMessage();

    std::ostream &stream() { return stream_; }

  private:
    void Init(const char *function, int line);

    LogSeverity severity_;
    std::ostringstream stream_;
    size_t message_start_;  // Offset of the start of the message (past prefix info).
    // The function and line information passed in to the constructor.
    const char *function_;
    const int line_;
};
}  // namespace gl

#if defined(ANGLE_ENABLE_DEBUG_TRACE) || defined(ANGLE_ENABLE_DEBUG_ANNOTATIONS)
#define ANGLE_TRACE_ENABLED
#endif

#define ANGLE_EMPTY_STATEMENT for (;;) break
#if !defined(NDEBUG) || defined(ANGLE_ENABLE_RELEASE_ASSERTS)
#define ANGLE_ENABLE_ASSERTS
#endif

#define WARN() ANGLE_LOG(WARN)
#define ERR() ANGLE_LOG(ERR)

// A macro to log a performance event around a scope.
#if defined(ANGLE_TRACE_ENABLED)
#if defined(_MSC_VER)
#define EVENT(message, ...) gl::ScopedPerfEventHelper scopedPerfEventHelper ## __LINE__("%s" message "\n", __FUNCTION__, __VA_ARGS__);
#else
#define EVENT(message, ...) gl::ScopedPerfEventHelper scopedPerfEventHelper("%s" message "\n", __FUNCTION__, ##__VA_ARGS__);
#endif // _MSC_VER
#else
#define EVENT(message, ...) (void(0))
#endif

#if defined(ANGLE_TRACE_ENABLED)
#undef ANGLE_TRACE_ENABLED
#endif

#if defined(COMPILER_GCC) || defined(__clang__)
#define ANGLE_CRASH() __builtin_trap()
#else
#define ANGLE_CRASH() ((void)(*(volatile char *)0 = 0))
#endif

#if !defined(NDEBUG)
#define ANGLE_ASSERT_IMPL(expression) assert(expression)
#else
// TODO(jmadill): Detect if debugger is attached and break.
#define ANGLE_ASSERT_IMPL(expression) ANGLE_CRASH()
#endif  // !defined(NDEBUG)

#if defined(NDEBUG) && !defined(ANGLE_ENABLE_ASSERTS)
#define ANGLE_ASSERTS_ON 0
#else
#define ANGLE_ASSERTS_ON 1
#endif

// A macro asserting a condition and outputting failures to the debug log
#if ANGLE_ASSERTS_ON
#define ASSERT(expression)                                                                         \
    (expression ? static_cast<void>(0) : ((ERR() << "\t! Assert failed in " << __FUNCTION__ << "(" \
                                                 << __LINE__ << "): " << #expression),             \
                                          ANGLE_ASSERT_IMPL(expression)))
#else
#define ASSERT(condition)                                                           \
    ANGLE_LAZY_STREAM(::gl::DummyStream(), ANGLE_ASSERTS_ON ? !(condition) : false) \
        << "Check failed: " #condition ". "
#endif  // ANGLE_ASSERTS_ON

#define UNUSED_VARIABLE(variable) ((void)variable)

// A macro to indicate unimplemented functionality
#ifndef NOASSERT_UNIMPLEMENTED
#define NOASSERT_UNIMPLEMENTED 1
#endif

#define UNIMPLEMENTED()                                                           \
    {                                                                             \
        ERR() << "\t! Unimplemented: " << __FUNCTION__ << "(" << __LINE__ << ")"; \
        ASSERT(NOASSERT_UNIMPLEMENTED);                                           \
    }                                                                             \
    ANGLE_EMPTY_STATEMENT

// A macro for code which is not expected to be reached under valid assumptions
#define UNREACHABLE()                                                                  \
    ((ERR() << "\t! Unreachable reached: " << __FUNCTION__ << "(" << __LINE__ << ")"), \
     ASSERT(false))

#endif   // COMMON_DEBUG_H_
