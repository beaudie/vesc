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
#include <ostream>
#include <vector>

#if defined(ANGLE_PLATFORM_ANDROID)
#include <android/log.h>
#endif

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

bool ShouldCreateLogMessage(LogSeverity severity)
{
#if defined(ANGLE_TRACE_ENABLED)
    return true;
#elif defined(ANGLE_ENABLE_ASSERTS)
    return severity != LOG_EVENT;
#else
    return false;
#endif
}

}  // namespace

namespace priv
{

bool ShouldCreatePlatformLogMessage(LogSeverity severity)
{
#if defined(ANGLE_TRACE_ENABLED)
    return true;
#else
    return severity != LOG_EVENT;
#endif
}

// This is never instantiated, it's just used for EAT_STREAM_PARAMETERS to an object of the correct
// type on the LHS of the unused part of the ternary operator.
std::ostream *gSwallowStream;
}  // namespace priv

bool DebugAnnotationsActive()
{
#if defined(ANGLE_ENABLE_DEBUG_ANNOTATIONS)
    return g_debugAnnotator != nullptr && g_debugAnnotator->getStatus();
#else
    return false;
#endif
}

bool DebugAnnotationsInitialized()
{
    return g_debugAnnotator != nullptr;
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

LogMessage::LogMessage(const char *function, int line, LogSeverity severity)
    : mFunction(function), mLine(line), mSeverity(severity)
{
    // EVENT() does not require additional function(line) info.
    if (mSeverity != LOG_EVENT)
    {
        mStream << mFunction << "(" << mLine << "): ";
    }
}

LogMessage::~LogMessage()
{
    if (DebugAnnotationsInitialized() && (mSeverity == LOG_ERR || mSeverity == LOG_WARN))
    {
        g_debugAnnotator->logMessage(*this);
    }
    else
    {
        Trace(getSeverity(), getMessage().c_str());
    }
}

void Trace(LogSeverity severity, const char *message)
{
    if (!ShouldCreateLogMessage(severity))
    {
        return;
    }

    std::string str(message);

    if (DebugAnnotationsActive())
    {
        std::wstring formattedWideMessage(str.begin(), str.end());

        switch (severity)
        {
            case LOG_EVENT:
                g_debugAnnotator->beginEvent(formattedWideMessage.c_str());
                break;
            default:
                g_debugAnnotator->setMarker(formattedWideMessage.c_str());
                break;
        }
    }

    if (severity == LOG_ERR || severity == LOG_WARN
#if defined(ANGLE_ENABLE_DEBUG_VERBOSE)
        || severity == LOG_VERBOSE
#endif
        )
    {
#if defined(ANGLE_PLATFORM_ANDROID)
        __android_log_print((severity == LOG_ERR)
                                ? ANDROID_LOG_ERROR
                                : (severity == LOG_WARN) ? ANDROID_LOG_WARN : ANDROID_LOG_VERBOSE,
                            "ANGLE", "%s: %s\n", LogSeverityName(severity), str.c_str());
#else
        // Note: we use fprintf because <iostream> includes static initializers.
        fprintf((severity == LOG_ERR) ? stderr : stdout, "%s: %s\n", LogSeverityName(severity),
                str.c_str());
#endif
    }

#if defined(ANGLE_PLATFORM_WINDOWS) && \
    (defined(ANGLE_ENABLE_DEBUG_TRACE_TO_DEBUGGER) || !defined(NDEBUG))
#if !defined(ANGLE_ENABLE_DEBUG_TRACE_TO_DEBUGGER)
    if (severity == LOG_ERR)
#endif  // !defined(ANGLE_ENABLE_DEBUG_TRACE_TO_DEBUGGER)
    {
        OutputDebugStringA(str.c_str());
    }
#endif

#if defined(ANGLE_ENABLE_DEBUG_TRACE)
#if defined(NDEBUG)
    if (severity == LOG_EVENT || severity == LOG_WARN)
    {
        return;
    }
#endif  // defined(NDEBUG)
    static std::ofstream file(TRACE_OUTPUT_FILE, std::ofstream::app);
    if (file)
    {
        file << LogSeverityName(severity) << ": " << str << std::endl;
        file.flush();
    }
#endif  // defined(ANGLE_ENABLE_DEBUG_TRACE)
}

LogSeverity LogMessage::getSeverity() const
{
    return mSeverity;
}

std::string LogMessage::getMessage() const
{
    return mStream.str();
}

#if defined(ANGLE_PLATFORM_WINDOWS)
priv::FmtHexHelper<HRESULT> FmtHR(HRESULT value)
{
    return priv::FmtHexHelper<HRESULT>("HRESULT: ", value);
}

priv::FmtHexHelper<DWORD> FmtErr(DWORD value)
{
    return priv::FmtHexHelper<DWORD>("error: ", value);
}
#endif  // defined(ANGLE_PLATFORM_WINDOWS)

std::string decodeEglAttr(EGLint attr)
{
    switch (attr)
    {
        // EGL Version 1.1
        case EGL_ALPHA_SIZE:
            return std::string("EGL_ALPHA_SIZE");
        case EGL_BAD_ACCESS:
            return std::string("EGL_BAD_ACCESS");
        case EGL_BAD_ALLOC:
            return std::string("EGL_BAD_ALLOC");
        case EGL_BAD_ATTRIBUTE:
            return std::string("EGL_BAD_ATTRIBUTE");
        case EGL_BAD_CONFIG:
            return std::string("EGL_BAD_CONFIG");
        case EGL_BAD_CONTEXT:
            return std::string("EGL_BAD_CONTEXT");
        case EGL_BAD_CURRENT_SURFACE:
            return std::string("EGL_BAD_CURRENT_SURFACE");
        case EGL_BAD_DISPLAY:
            return std::string("EGL_BAD_DISPLAY");
        case EGL_BAD_MATCH:
            return std::string("EGL_BAD_MATCH");
        case EGL_BAD_NATIVE_PIXMAP:
            return std::string("EGL_BAD_NATIVE_PIXMAP");
        case EGL_BAD_NATIVE_WINDOW:
            return std::string("EGL_BAD_NATIVE_WINDOW");
        case EGL_BAD_PARAMETER:
            return std::string("EGL_BAD_PARAMETER");
        case EGL_BAD_SURFACE:
            return std::string("EGL_BAD_SURFACE");
        case EGL_BLUE_SIZE:
            return std::string("EGL_BLUE_SIZE");
        case EGL_BUFFER_SIZE:
            return std::string("EGL_BUFFER_SIZE");
        case EGL_CONFIG_CAVEAT:
            return std::string("EGL_CONFIG_CAVEAT");
        case EGL_CONFIG_ID:
            return std::string("EGL_CONFIG_ID");
        case EGL_CORE_NATIVE_ENGINE:
            return std::string("EGL_CORE_NATIVE_ENGINE");
        case EGL_DEPTH_SIZE:
            return std::string("EGL_DEPTH_SIZE");
        case EGL_DONT_CARE:
            return std::string("EGL_DONT_CARE");
        case EGL_DRAW:
            return std::string("EGL_DRAW");
        case EGL_EXTENSIONS:
            return std::string("EGL_EXTENSIONS");
        case EGL_FALSE:
            return std::string("EGL_FALSE");
        case EGL_GREEN_SIZE:
            return std::string("EGL_GREEN_SIZE");
        case EGL_HEIGHT:
            return std::string("EGL_HEIGHT");
        case EGL_LARGEST_PBUFFER:
            return std::string("EGL_LARGEST_PBUFFER");
        case EGL_LEVEL:
            return std::string("EGL_LEVEL");
        case EGL_MAX_PBUFFER_HEIGHT:
            return std::string("EGL_MAX_PBUFFER_HEIGHT");
        case EGL_MAX_PBUFFER_PIXELS:
            return std::string("EGL_MAX_PBUFFER_PIXELS");
        case EGL_MAX_PBUFFER_WIDTH:
            return std::string("EGL_MAX_PBUFFER_WIDTH");
        case EGL_NATIVE_RENDERABLE:
            return std::string("EGL_NATIVE_RENDERABLE");
        case EGL_NATIVE_VISUAL_ID:
            return std::string("EGL_NATIVE_VISUAL_ID");
        case EGL_NATIVE_VISUAL_TYPE:
            return std::string("EGL_NATIVE_VISUAL_TYPE");
        case EGL_NONE:
            return std::string("EGL_NONE");
        /* following are same as EGL_NONE, need more context
         * to distinquish properly.
    case EGL_NO_CONTEXT:
        return std::string("EGL_NO_CONTEXT");
    case EGL_NO_DISPLAY:
        return std::string("EGL_NO_DISPLAY");
    case EGL_NO_SURFACE:
        return std::string("EGL_NO_SURFACE");
        */
        case EGL_NON_CONFORMANT_CONFIG:
            return std::string("EGL_NON_CONFORMANT_CONFIG");
        case EGL_NOT_INITIALIZED:
            return std::string("EGL_NOT_INITIALIZED");
        /* conflicts with EGL_TRUE
    case EGL_PBUFFER_BIT:
        return std::string("EGL_PBUFFER_BIT");
        */
        case EGL_PIXMAP_BIT:
            return std::string("EGL_PIXMAP_BIT");
        case EGL_READ:
            return std::string("EGL_READ");
        case EGL_RED_SIZE:
            return std::string("EGL_RED_SIZE");
        case EGL_SAMPLES:
            return std::string("EGL_SAMPLES");
        case EGL_SAMPLE_BUFFERS:
            return std::string("EGL_SAMPLE_BUFFERS");
        case EGL_SLOW_CONFIG:
            return std::string("EGL_SLOW_CONFIG");
        case EGL_STENCIL_SIZE:
            return std::string("EGL_STENCIL_SIZE");
        case EGL_SUCCESS:
            return std::string("EGL_SUCCESS");
        case EGL_SURFACE_TYPE:
            return std::string("EGL_SURFACE_TYPE");
        case EGL_TRANSPARENT_BLUE_VALUE:
            return std::string("EGL_TRANSPARENT_BLUE_VALUE");
        case EGL_TRANSPARENT_GREEN_VALUE:
            return std::string("EGL_TRANSPARENT_GREEN_VALUE");
        case EGL_TRANSPARENT_RED_VALUE:
            return std::string("EGL_TRANSPARENT_RED_VALUE");
        case EGL_TRANSPARENT_RGB:
            return std::string("EGL_TRANSPARENT_RGB");
        /* conflicts with EGL_TRUE
    case EGL_TRANSPARENT_TYPE:
        return std::string("EGL_TRANSPARENT_TYPE");
        */
        case EGL_TRUE:
            return std::string("EGL_TRUE");
        case EGL_VENDOR:
            return std::string("EGL_VENDOR");
        case EGL_VERSION:
            return std::string("EGL_VERSION");
        case EGL_WIDTH:
            return std::string("EGL_WIDTH");
        case EGL_WINDOW_BIT:
            return std::string("EGL_WINDOW_BIT");

        // EGL Version 1.1
        case EGL_BACK_BUFFER:
            return std::string("EGL_BACK_BUFFER");
        case EGL_BIND_TO_TEXTURE_RGB:
            return std::string("EGL_BIND_TO_TEXTURE_RGB");
        case EGL_BIND_TO_TEXTURE_RGBA:
            return std::string("EGL_BIND_TO_TEXTURE_RGBA");
        case EGL_CONTEXT_LOST:
            return std::string("EGL_CONTEXT_LOST");
        case EGL_MIN_SWAP_INTERVAL:
            return std::string("EGL_MIN_SWAP_INTERVAL");
        case EGL_MAX_SWAP_INTERVAL:
            return std::string("EGL_MAX_SWAP_INTERVAL");
        case EGL_MIPMAP_TEXTURE:
            return std::string("EGL_MIPMAP_TEXTURE");
        case EGL_MIPMAP_LEVEL:
            return std::string("EGL_MIPMAP_LEVEL");
        case EGL_NO_TEXTURE:
            return std::string("EGL_NO_TEXTURE");
        case EGL_TEXTURE_2D:
            return std::string("EGL_TEXTURE_2D");
        case EGL_TEXTURE_FORMAT:
            return std::string("EGL_TEXTURE_FORMAT");
        case EGL_TEXTURE_RGB:
            return std::string("EGL_TEXTURE_RGB");
        case EGL_TEXTURE_RGBA:
            return std::string("EGL_TEXTURE_RGBA");
        case EGL_TEXTURE_TARGET:
            return std::string("EGL_TEXTURE_TARGET");

        // EGL Version 1.2
        case EGL_ALPHA_FORMAT:
            return std::string("EGL_ALPHA_FORMAT");
        case EGL_ALPHA_FORMAT_NONPRE:
            return std::string("EGL_ALPHA_FORMAT_NONPRE");
        case EGL_ALPHA_FORMAT_PRE:
            return std::string("EGL_ALPHA_FORMAT_PRE");
        case EGL_ALPHA_MASK_SIZE:
            return std::string("EGL_ALPHA_MASK_SIZE");
        case EGL_BUFFER_PRESERVED:
            return std::string("EGL_BUFFER_PRESERVED");
        case EGL_BUFFER_DESTROYED:
            return std::string("EGL_BUFFER_DESTROYED");
        case EGL_CLIENT_APIS:
            return std::string("EGL_CLIENT_APIS");
        case EGL_COLORSPACE:
            return std::string("EGL_COLORSPACE");
        case EGL_COLORSPACE_sRGB:
            return std::string("EGL_COLORSPACE_sRGB");
        case EGL_COLORSPACE_LINEAR:
            return std::string("EGL_COLORSPACE_LINEAR");
        case EGL_COLOR_BUFFER_TYPE:
            return std::string("EGL_COLOR_BUFFER_TYPE");
        case EGL_CONTEXT_CLIENT_TYPE:
            return std::string("EGL_CONTEXT_CLIENT_TYPE");
        case EGL_DISPLAY_SCALING:
            return std::string("EGL_DISPLAY_SCALING");
        case EGL_HORIZONTAL_RESOLUTION:
            return std::string("EGL_HORIZONTAL_RESOLUTION");
        case EGL_LUMINANCE_BUFFER:
            return std::string("EGL_LUMINANCE_BUFFER");
        case EGL_LUMINANCE_SIZE:
            return std::string("EGL_LUMINANCE_SIZE");
        /* conflicts with EGL_TRUE
    case EGL_OPENGL_ES_BIT:
        return std::string("EGL_OPENGL_ES_BIT");
        */
        /* conflicts with EGL_PIXMAP_BIT
    case EGL_OPENVG_BIT:
        return std::string("EGL_OPENVG_BIT");
        */
        case EGL_OPENGL_ES_API:
            return std::string("EGL_OPENGL_ES_API");
        case EGL_OPENVG_API:
            return std::string("EGL_OPENVG_API");
        case EGL_OPENVG_IMAGE:
            return std::string("EGL_OPENVG_IMAGE");
        case EGL_PIXEL_ASPECT_RATIO:
            return std::string("EGL_PIXEL_ASPECT_RATIO");
        case EGL_RENDERABLE_TYPE:
            return std::string("EGL_RENDERABLE_TYPE");
        case EGL_RENDER_BUFFER:
            return std::string("EGL_RENDER_BUFFER");
        case EGL_RGB_BUFFER:
            return std::string("EGL_RGB_BUFFER");
        case EGL_SINGLE_BUFFER:
            return std::string("EGL_SINGLE_BUFFER");
        case EGL_SWAP_BEHAVIOR:
            return std::string("EGL_SWAP_BEHAVIOR");
        /* same as EGL_DONT_CARE
    case EGL_UNKNOWN:
        return std::string("EGL_UNKNOWN");
        */
        case EGL_VERTICAL_RESOLUTION:
            return std::string("EGL_VERTICAL_RESOLUTION");

        // EGL Version 1.3
        case EGL_CONTEXT_CLIENT_VERSION:
            return std::string("EGL_CONTEXT_CLIENT_VERSION");

        // EGL version 1.4
        case EGL_MULTISAMPLE_RESOLVE_BOX_BIT:
            return std::string("EGL_MULTISAMPLE_RESOLVE_BOX_BIT");
        case EGL_MULTISAMPLE_RESOLVE:
            return std::string("EGL_MULTISAMPLE_RESOLVE");
        case EGL_MULTISAMPLE_RESOLVE_DEFAULT:
            return std::string("EGL_MULTISAMPLE_RESOLVE_DEFAULT");
        case EGL_MULTISAMPLE_RESOLVE_BOX:
            return std::string("EGL_MULTISAMPLE_RESOLVE_BOX");
        case EGL_OPENGL_API:
            return std::string("EGL_OPENGL_API");
        case EGL_OPENGL_BIT:
            return std::string("EGL_OPENGL_BIT");
        case EGL_SWAP_BEHAVIOR_PRESERVED_BIT:
            return std::string("EGL_SWAP_BEHAVIOR_PRESERVED_BIT");
        default:
            std::stringstream output;
            output << "Unknown attribute: " << attr << " (0x" << std::hex << attr << ")\n";
            return output.str();
    }
}

std::string dumpEglAttrs(const EGLint *attrList)
{
    std::stringstream output;
    output << ("EGL Attribute List\n");
    for (const EGLint *attr = attrList; attr && attr[0] != EGL_NONE; attr += 2)
    {
        output << decodeEglAttr(attr[0]).c_str() << ": " << attr[1] << " (0x" << std::hex << attr[1]
               << ")\n";
    }
    return output.str();
}

}  // namespace gl
