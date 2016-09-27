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
#include <string>

#include "common/angleutils.h"

#if !defined(TRACE_OUTPUT_FILE)
#define TRACE_OUTPUT_FILE "angle_debug.txt"
#endif

namespace gl
{

enum MessageType
{
    MESSAGE_TRACE,
    MESSAGE_FIXME,
    MESSAGE_ERR,
    MESSAGE_EVENT,
};

// Outputs text to the debugging log, or the debugging window
void trace(bool traceInDebugOnly, MessageType messageType, const char *format, ...);

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

}  // namespace gl

#if defined(ANGLE_ENABLE_DEBUG_TRACE) || defined(ANGLE_ENABLE_DEBUG_ANNOTATIONS)
#define ANGLE_TRACE_ENABLED
#endif

#define ANGLE_EMPTY_STATEMENT for (;;) break
#if !defined(NDEBUG) || defined(ANGLE_ENABLE_RELEASE_ASSERTS)
#define ANGLE_ENABLE_ASSERTS
#endif

// A macro to output a trace of a function call and its arguments to the debugging log
#if defined(ANGLE_TRACE_ENABLED)
#define TRACE(message, ...) gl::trace(true, gl::MESSAGE_TRACE, "trace: %s(%d): " message "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define TRACE(message, ...) (void(0))
#endif

// A macro to output a function call and its arguments to the debugging log, to denote an item in need of fixing.
#if defined(ANGLE_TRACE_ENABLED)
#define FIXME(message, ...) gl::trace(false, gl::MESSAGE_FIXME, "fixme: %s(%d): " message "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define FIXME(message, ...) (void(0))
#endif

// A macro to output a function call and its arguments to the debugging log, in case of error.
#if defined(ANGLE_TRACE_ENABLED) || defined(ANGLE_ENABLE_ASSERTS)
#define ERR(message, ...) gl::trace(false, gl::MESSAGE_ERR, "err: %s(%d): " message "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ERR(message, ...) (void(0))
#endif

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

#if !defined(NDEBUG)
#define ANGLE_ASSERT_IMPL(expression) assert(expression)
#else
// TODO(jmadill): Detect if debugger is attached and break.
#define ANGLE_ASSERT_IMPL(expression) abort()
#endif  // !defined(NDEBUG)

// A macro asserting a condition and outputting failures to the debug log
#if defined(ANGLE_ENABLE_ASSERTS)
#define ASSERT(expression)                                                                 \
    {                                                                                      \
        if (!(expression))                                                                 \
        {                                                                                  \
            ERR("\t! Assert failed in %s(%d): %s\n", __FUNCTION__, __LINE__, #expression); \
            ANGLE_ASSERT_IMPL(expression);                                                 \
        }                                                                                  \
    }                                                                                      \
    ANGLE_EMPTY_STATEMENT
#define UNUSED_ASSERTION_VARIABLE(variable)
#else
#define ASSERT(expression) (void(0))
#define UNUSED_ASSERTION_VARIABLE(variable) ((void)variable)
#endif

#define UNUSED_VARIABLE(variable) ((void)variable)

// A macro to indicate unimplemented functionality

#if defined (ANGLE_TEST_CONFIG)
#define NOASSERT_UNIMPLEMENTED 1
#endif

// Define NOASSERT_UNIMPLEMENTED to non zero to skip the assert fail in the unimplemented checks
// This will allow us to test with some automated test suites (eg dEQP) without crashing
#ifndef NOASSERT_UNIMPLEMENTED
#define NOASSERT_UNIMPLEMENTED 0
#endif

#if !defined(NDEBUG)
#define UNIMPLEMENTED() { \
    FIXME("\t! Unimplemented: %s(%d)\n", __FUNCTION__, __LINE__); \
    assert(NOASSERT_UNIMPLEMENTED); \
    } ANGLE_EMPTY_STATEMENT
#else
    #define UNIMPLEMENTED() FIXME("\t! Unimplemented: %s(%d)\n", __FUNCTION__, __LINE__)
#endif

// A macro for code which is not expected to be reached under valid assumptions
#if !defined(NDEBUG)
#define UNREACHABLE() \
    ERR("\t! Unreachable reached: %s(%d)\n", __FUNCTION__, __LINE__), assert(false)
#else
    #define UNREACHABLE() ERR("\t! Unreachable reached: %s(%d)\n", __FUNCTION__, __LINE__)
#endif

// Based on code from Chromium's base/logging.h
#if defined(COMPILER_GCC) || __clang__
#define ANGLE_LOGGING_CRASH() __builtin_trap()
#else
#define ANGLE_LOGGING_CRASH() ((void)(*(volatile char *)0 = 0))
#endif

namespace angle
{

// This class is used to explicitly ignore values in the conditional
// logging macros.  This avoids compiler warnings like "value computed
// is not used" and "statement has no effect".
class LogMessageVoidify
{
  public:
    LogMessageVoidify() {}
    // This has to be an operator with a precedence lower than << but
    // higher than ?:
    void operator&(std::ostream &) {}
};

typedef int LogSeverity;
const LogSeverity LOG_VERBOSE = -1;  // This is level 1 verbosity
// Note: the log severities are used to index into the array of names,
// see log_severity_names.
const LogSeverity LOG_INFO           = 0;
const LogSeverity LOG_WARNING        = 1;
const LogSeverity LOG_ERROR          = 2;
const LogSeverity LOG_FATAL          = 3;
const LogSeverity LOG_NUM_SEVERITIES = 4;

// This class more or less represents a particular log message.  You
// create an instance of LogMessage and then stream stuff to it.
// When you finish streaming to it, ~LogMessage is called and the
// full message gets streamed to the appropriate destination.
//
// You shouldn't actually use LogMessage's constructor to log things,
// though.  You should use the LOG() macro (and variants thereof)
// above.
class LogMessage
{
  public:
    // Used for LOG(severity).
    LogMessage(const char *file, int line, LogSeverity severity);

    std::ostream &stream();
};

}  // namespace angle

#define ANGLE_COMPACT_LOG_EX_FATAL(ClassName, ...) \
    ::angle::ClassName(__FILE__, __LINE__, ::angle::LOG_FATAL, ##__VA_ARGS__)

#define ANGLE_COMPACT_LOG_FATAL ANGLE_COMPACT_LOG_EX_FATAL(LogMessage)

// We use the preprocessor's merging operator, "##", so that, e.g.,
// LOG(INFO) becomes the token COMPACT_GOOGLE_LOG_INFO.  There's some funny
// subtle difference between ostream member streaming functions (e.g.,
// ostream::operator<<(int) and ostream non-member streaming functions
// (e.g., ::operator<<(ostream&, string&): it turns out that it's
// impossible to stream something like a string directly to an unnamed
// ostream. We employ a neat hack by calling the stream() member
// function of LogMessage which seems to avoid the problem.
#define ANGLE_LOG_STREAM(severity) ANGLE_COMPACT_LOG_##severity.stream()

#define ANGLE_EAT_STREAM_PARAMETERS \
    true ? (void)0 : ::angle::LogMessageVoidify() & ANGLE_LOG_STREAM(FATAL)

#define ANGLE_CRASH_CHECK(condition) \
    !(condition) ? ANGLE_LOGGING_CRASH() : ANGLE_EAT_STREAM_PARAMETERS

#endif   // COMMON_DEBUG_H_
