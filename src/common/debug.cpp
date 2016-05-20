//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// debug.cpp: Debugging utilities.

#include "common/debug.h"
#include "common/platform.h"
#include "common/angleutils.h"
#include "common/Optional.h"

#include <stdarg.h>
#include <vector>
#include <fstream>
#include <cstdio>
#include <iostream>

namespace gl
{

namespace
{
enum DebugTraceOutputType
{
   DebugTraceOutputTypeNone,
   DebugTraceOutputTypeSetMarker,
   DebugTraceOutputTypeBeginEvent
};

class FormattedMessage final : angle::NonCopyable
{
  public:
    FormattedMessage(const char *format, va_list vararg) : mFormat(format)
    {
        va_copy(mVarArg, vararg);
    }

    const char *get()
    {
        if (!mMessage.valid())
        {
            mMessage = FormatString(mFormat, mVarArg);
        }
        return mMessage.value().c_str();
    }

    size_t length()
    {
        get();
        return mMessage.value().length();
    }

  private:
    const char *mFormat;
    va_list mVarArg;
    Optional<std::string> mMessage;
};

DebugAnnotator *g_debugAnnotator = nullptr;

void output(bool traceInDebugOnly, MessageType messageType, DebugTraceOutputType outputType,
            const char *format, va_list vararg)
{
    if (DebugAnnotationsActive())
    {
        static std::vector<char> buffer(512);
        size_t len = FormatStringIntoVector(format, vararg, buffer);
        std::wstring formattedWideMessage(buffer.begin(), buffer.begin() + len);

        ASSERT(g_debugAnnotator != nullptr);
        switch (outputType)
        {
          case DebugTraceOutputTypeNone:
            break;
          case DebugTraceOutputTypeBeginEvent:
            g_debugAnnotator->beginEvent(formattedWideMessage.c_str());
            break;
          case DebugTraceOutputTypeSetMarker:
            g_debugAnnotator->setMarker(formattedWideMessage.c_str());
            break;
        }
    }

    FormattedMessage formattedMessage(format, vararg);
    UNUSED_VARIABLE(formattedMessage);

    if (messageType == MESSAGE_ERR)
    {
        std::cerr << formattedMessage.get();
#if !defined(NDEBUG) && defined(_MSC_VER)
        OutputDebugStringA(formattedMessage.get());
#endif
    }

#if defined(ANGLE_ENABLE_DEBUG_TRACE)
#if defined(NDEBUG)
    if (traceInDebugOnly)
    {
        return;
    }
#endif // NDEBUG
    static std::ofstream file(TRACE_OUTPUT_FILE, std::ofstream::app);
    if (file)
    {
        file.write(formattedMessage.get(), formattedMessage.length());
        file.flush();
    }

#if defined(ANGLE_ENABLE_DEBUG_TRACE_TO_DEBUGGER)
    OutputDebugStringA(formattedMessage.get());
#endif // ANGLE_ENABLE_DEBUG_TRACE_TO_DEBUGGER

#endif // ANGLE_ENABLE_DEBUG_TRACE
}

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

void trace(bool traceInDebugOnly, MessageType messageType, const char *format, ...)
{
    va_list vararg;
    va_start(vararg, format);
    output(traceInDebugOnly, messageType, DebugTraceOutputTypeSetMarker, format, vararg);
    va_end(vararg);
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
    output(true, MESSAGE_EVENT, DebugTraceOutputTypeBeginEvent, format, vararg);
    va_end(vararg);
}

ScopedPerfEventHelper::~ScopedPerfEventHelper()
{
    if (DebugAnnotationsActive())
    {
        g_debugAnnotator->endEvent();
    }
}

}
