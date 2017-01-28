//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// debug.cpp: Debugging utilities.

#include "common/debug.h"

#include <stdarg.h>

#include <array>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "common/angleutils.h"
#include "common/Optional.h"

namespace gl
{

namespace
{

DebugAnnotator *g_debugAnnotator = nullptr;

constexpr std::array<const char *, LOG_NUM_SEVERITIES> g_logSeverityNames = {
    {"EVENT", "WARN", "ERR"}};

constexpr const char *LogSeverityName(int severity)
{
    return (severity >= 0 && severity < LOG_NUM_SEVERITIES) ? g_logSeverityNames[severity]
                                                            : "UNKNOWN";
}

}  // namespace

bool DebugAnnotator::logMessage(LogSeverity severity,
                                const char *function,
                                int line,
                                size_t message_start,
                                const std::string &str)
{
    if (!mLogger)
    {
        return false;
    }

    std::ostringstream stream;
    stream << function << "(" << line << "): " << str.substr(message_start);
    std::string message = stream.str();

    switch (severity)
    {
        case LOG_EVENT:
            mLogger->logInfo(message.c_str());
            break;
        case LOG_WARN:
            mLogger->logWarning(message.c_str());
            break;
        case LOG_ERR:
            mLogger->logError(message.c_str());
            break;
        default:
            return false;
    }
    return true;
}

void DebugAnnotator::setLogger(angle::Platform *logger)
{
    mLogger = logger;
}

angle::Platform *DebugAnnotator::getLogger() const
{
    return mLogger;
}

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
    angle::Platform *logger = g_debugAnnotator ? g_debugAnnotator->getLogger() : nullptr;

    UninitializeDebugAnnotations();
    g_debugAnnotator = debugAnnotator;

    if (logger)
    {
        SetDebugAnnotationsLogger(logger);
    }
}

void UninitializeDebugAnnotations()
{
    angle::Platform *logger = g_debugAnnotator ? g_debugAnnotator->getLogger() : nullptr;

    // Pointer is not managed.
    g_debugAnnotator = nullptr;

    if (logger)
    {
        SetDebugAnnotationsLogger(logger);
    }
}

class DefaultDebugAnnotator : public gl::DebugAnnotator
{
  public:
    DefaultDebugAnnotator(){};
    ~DefaultDebugAnnotator() override {}
    void beginEvent(const wchar_t *eventName) override {}
    void endEvent() override {}
    void setMarker(const wchar_t *markerName) override {}
    bool getStatus() override { return false; }
};

std::unique_ptr<DefaultDebugAnnotator> g_defaultDebugAnnotator = nullptr;

void SetDebugAnnotationsLogger(angle::Platform *logger)
{
    if (!g_debugAnnotator && !logger)
    {
        return;
    }

    if (!g_debugAnnotator)
    {
        if (!g_defaultDebugAnnotator)
        {
            g_defaultDebugAnnotator.reset(new DefaultDebugAnnotator());
        }

        g_debugAnnotator = g_defaultDebugAnnotator.get();
    }

    g_debugAnnotator->setLogger(logger);
}

ScopedPerfEventHelper::ScopedPerfEventHelper(const char *format, ...)
{
#if !defined(ANGLE_ENABLE_DEBUG_TRACE)
    if (!DebugAnnotationsActive())
    {
        return;
    }
#endif  // !ANGLE_ENABLE_DEBUG_TRACE

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

namespace priv
{

bool ShouldCreateLogMessage(LogSeverity severity)
{
#if defined(ANGLE_TRACE_ENABLED)
    return true;
#elif defined(ANGLE_ENABLE_ASSERTS)
    return severity == LOG_ERR;
#else
    return false;
#endif
}

bool ShouldCreatePlatformLogMessage(LogSeverity severity)
{
#if defined(ANGLE_TRACE_ENABLED)
    return true;
#else
    return severity != LOG_EVENT;
#endif
}

LogMessage::LogMessage(const char *function, int line, LogSeverity severity)
    : mSeverity(severity), mFunction(function), mLine(line)
{
    init(function, line);
}

LogMessage::~LogMessage()
{
    mStream << std::endl;
    std::string str(mStream.str());

    // Give any log message handler first dibs on the message.
    bool handled = g_debugAnnotator != nullptr &&
                   g_debugAnnotator->logMessage(mSeverity, mFunction, mLine, mMessageStart, str);

    if (!ShouldCreateLogMessage(mSeverity))
    {
        return;
    }

    if (DebugAnnotationsActive())
    {
        std::wstring formattedWideMessage(str.begin(), str.end());

        switch (mSeverity)
        {
            case LOG_EVENT:
                g_debugAnnotator->beginEvent(formattedWideMessage.c_str());
                break;
            default:
                g_debugAnnotator->setMarker(formattedWideMessage.c_str());
                break;
        }
    }

    if (!handled && mSeverity == LOG_ERR)
    {
        std::cerr << str;
#if !defined(NDEBUG) && defined(_MSC_VER)
        OutputDebugStringA(str.c_str());
#endif  // !defined(NDEBUG) && defined(_MSC_VER)
    }

#if defined(ANGLE_ENABLE_DEBUG_TRACE)
#if defined(NDEBUG)
    if (mSeverity == LOG_EVENT || mSeverity == LOG_WARN)
    {
        return;
    }
#endif  // NDEBUG
    static std::ofstream file(TRACE_OUTPUT_FILE, std::ofstream::app);
    if (file)
    {
        file.write(str.c_str(), str.length());
        file.flush();
    }

#if defined(ANGLE_ENABLE_DEBUG_TRACE_TO_DEBUGGER)
    OutputDebugStringA(str.c_str());
#endif  // ANGLE_ENABLE_DEBUG_TRACE_TO_DEBUGGER

#endif  // ANGLE_ENABLE_DEBUG_TRACE
}

// writes the common header info to the stream
void LogMessage::init(const char *function, int line)
{
    if (mSeverity >= 0)
        mStream << LogSeverityName(mSeverity);
    else
        mStream << "VERBOSE" << -mSeverity;

    // EVENT() don't require additional function(line) info
    if (mSeverity != LOG_EVENT)
    {
        mStream << ": " << function << "(" << line << "): ";
    }

    mMessageStart = mStream.str().length();
}

}  // namespace priv

#if defined(ANGLE_PLATFORM_WINDOWS)
std::ostream &operator<<(std::ostream &os, const FmtHR &fmt)
{
    os << "HRESULT: ";
    return FmtHexInt(os, fmt.mHR);
}
#endif  // defined(ANGLE_PLATFORM_WINDOWS)

}  // namespace gl
