//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// debug.cpp: Debugging utilities.

#include "common/debug.h"

#include <stdarg.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>

#include "common/angleutils.h"
#include "common/platform.h"
#include "common/Optional.h"

namespace gl
{

namespace
{

DebugAnnotator *g_debugAnnotator = nullptr;

const char *const log_severity_names[LOG_NUM_SEVERITIES] = {"EVENT", "WARN", "ERR"};

const char *log_severity_name(int severity)
{
    if (severity >= 0 && severity < LOG_NUM_SEVERITIES)
        return log_severity_names[severity];
    return "UNKNOWN";
}

// A log message handler that gets notified of every log message we process.
LogMessageHandlerFunction log_message_handler = nullptr;
} // namespace

bool DebugAnnotationsActive()
{
#if defined(ANGLE_ENABLE_DEBUG_ANNOTATIONS)
    return g_debugAnnotator != nullptr && g_debugAnnotator->getStatus();
#else
    return false;
#endif
}

void InitializeDebugAnnotations(DebugAnnotator *debugAnnotator)
{
    UninitializeDebugAnnotations();
    g_debugAnnotator = debugAnnotator;
}

void UninitializeDebugAnnotations()
{
    // Pointer is not managed.
    g_debugAnnotator = nullptr;
}

ScopedPerfEventHelper::ScopedPerfEventHelper(const char* format, ...)
{
#if !defined(ANGLE_ENABLE_DEBUG_TRACE)
    if (!DebugAnnotationsActive())
    {
        return;
    }
#endif // !ANGLE_ENABLE_DEBUG_TRACE

    va_list vararg;
    va_start(vararg, format);
    std::vector<char> buffer(512);
    size_t len = FormatStringIntoVector(format, vararg, buffer);
    ANGLE_LOG(EVENT) << std::string(&buffer[0], len);
    va_end(vararg);
}

ScopedPerfEventHelper::~ScopedPerfEventHelper()
{
    if (DebugAnnotationsActive())
    {
        g_debugAnnotator->endEvent();
    }
}

std::ostream &DummyStream()
{
    return std::cout;
}

bool ShouldCreateLogMessage(int severity)
{
#if defined(ANGLE_ENABLE_DEBUG_TRACE) || defined(ANGLE_ENABLE_DEBUG_ANNOTATIONS)
    return true;
#endif
#if defined(ANGLE_ENABLE_ASSERTS)
    if (severity == LOG_ERR)
    {
        return true;
    }
#endif
    return false;
}

void SetLogMessageHandler(LogMessageHandlerFunction handler)
{
    log_message_handler = handler;
}

LogMessage::LogMessage(const char *function, int line, LogSeverity severity)
    : severity_(severity), function_(function), line_(line)
{
    Init(function, line);
}

LogMessage::~LogMessage()
{
    stream_ << std::endl;
    std::string str_newline(stream_.str());

    if (DebugAnnotationsActive())
    {
        std::wstring formattedWideMessage(str_newline.begin(), str_newline.end());

        switch (severity_)
        {
            case LOG_EVENT:
                g_debugAnnotator->beginEvent(formattedWideMessage.c_str());
                break;
            default:
                g_debugAnnotator->setMarker(formattedWideMessage.c_str());
                break;
        }
    }

    // Give any log message handler first dibs on the message.
    bool handled = log_message_handler &&
                   log_message_handler(severity_, function_, line_, message_start_, str_newline);

    if (!handled && severity_ == LOG_ERR)
    {
        std::cerr << str_newline;
#if !defined(NDEBUG) && defined(_MSC_VER)
        OutputDebugStringA(str_newline.c_str());
#endif  // !defined(NDEBUG) && defined(_MSC_VER)
    }

#if defined(ANGLE_ENABLE_DEBUG_TRACE)
#if defined(NDEBUG)
    if (severity_ == LOG_EVENT || severity_ == LOG_WARN)
    {
        return;
    }
#endif  // NDEBUG
    static std::ofstream file(TRACE_OUTPUT_FILE, std::ofstream::app);
    if (file)
    {
        file.write(str_newline.c_str(), str_newline.length());
        file.flush();
    }

#if defined(ANGLE_ENABLE_DEBUG_TRACE_TO_DEBUGGER)
    OutputDebugStringA(str_newline.c_str());
#endif  // ANGLE_ENABLE_DEBUG_TRACE_TO_DEBUGGER

#endif  // ANGLE_ENABLE_DEBUG_TRACE
}

// writes the common header info to the stream
void LogMessage::Init(const char *function, int line)
{
    if (severity_ >= 0)
        stream_ << log_severity_name(severity_);
    else
        stream_ << "VERBOSE" << -severity_;

    stream_ << ": " << function << "(" << line << "): ";

    message_start_ = stream_.str().length();
}

}  // namespace gl
