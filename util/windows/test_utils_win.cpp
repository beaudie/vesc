//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// test_utils_win.cpp: Implementation of OS-specific functions for Windows

#include "util/test_utils.h"

#include <stdarg.h>
#include <windows.h>
#include <array>

#include "common/angleutils.h"
#include "util/windows/third_party/StackWalker/src/StackWalker.h"

namespace angle
{
namespace
{
static const struct
{
    const char *name;
    const DWORD code;
} kExceptions[] = {
#define _(E)  \
    {         \
#        E, E \
    }
    _(EXCEPTION_ACCESS_VIOLATION),
    _(EXCEPTION_BREAKPOINT),
    _(EXCEPTION_INT_DIVIDE_BY_ZERO),
    _(EXCEPTION_STACK_OVERFLOW),
#undef _
};

class CustomStackWalker : public StackWalker
{
  public:
    CustomStackWalker() {}
    ~CustomStackWalker() {}

    void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry) override
    {
        char buffer[STACKWALK_MAX_NAMELEN];
        size_t maxLen = _TRUNCATE;
        if ((eType != lastEntry) && (entry.offset != 0))
        {
            if (entry.name[0] == 0)
                strncpy_s(entry.name, STACKWALK_MAX_NAMELEN, "(function-name not available)",
                          _TRUNCATE);
            if (entry.undName[0] != 0)
                strncpy_s(entry.name, STACKWALK_MAX_NAMELEN, entry.undName, _TRUNCATE);
            if (entry.undFullName[0] != 0)
                strncpy_s(entry.name, STACKWALK_MAX_NAMELEN, entry.undFullName, _TRUNCATE);
            if (entry.lineFileName[0] == 0)
            {
                strncpy_s(entry.lineFileName, STACKWALK_MAX_NAMELEN, "(filename not available)",
                          _TRUNCATE);
                if (entry.moduleName[0] == 0)
                    strncpy_s(entry.moduleName, STACKWALK_MAX_NAMELEN,
                              "(module-name not available)", _TRUNCATE);
                _snprintf_s(buffer, maxLen, "    %s - %p (%s): %s\n", entry.name,
                            reinterpret_cast<void *>(entry.offset), entry.moduleName,
                            entry.lineFileName);
            }
            else
                _snprintf_s(buffer, maxLen, "    %s (%s:%d)\n", entry.name, entry.lineFileName,
                            entry.lineNumber);
            buffer[STACKWALK_MAX_NAMELEN - 1] = 0;
            printf("%s", buffer);
            OutputDebugStringA(buffer);
        }
    }
};

void PrintBacktrace(CONTEXT *c)
{
    printf("Backtrace:\n");
    OutputDebugStringA("Backtrace:\n");

    CustomStackWalker sw;
    sw.ShowCallstack(GetCurrentThread(), c);
}

LONG WINAPI StackTraceCrashHandler(EXCEPTION_POINTERS *e)
{
    const DWORD code = e->ExceptionRecord->ExceptionCode;
    printf("\nCaught exception %lu", code);
    for (size_t i = 0; i < ArraySize(kExceptions); i++)
    {
        if (kExceptions[i].code == code)
        {
            printf(" %s", kExceptions[i].name);
        }
    }
    printf("\n");

    PrintBacktrace(e->ContextRecord);

    // Exit NOW.  Don't notify other threads, don't call anything registered with atexit().
    _exit(1);

    // The compiler wants us to return something.  This is what we'd do if we didn't _exit().
    return EXCEPTION_EXECUTE_HANDLER;
}

struct ScopedPipe
{
    ~ScopedPipe()
    {
        closeReadHandle();
        closeWriteHandle();
    }
    void closeReadHandle()
    {
        if (readHandle)
        {
            CloseHandle(readHandle);
            readHandle = nullptr;
        }
    }
    void closeWriteHandle()
    {
        if (writeHandle)
        {
            CloseHandle(writeHandle);
            writeHandle = nullptr;
        }
    }
    HANDLE readHandle  = nullptr;
    HANDLE writeHandle = nullptr;
};

void ReadEntireFile(HANDLE handle, std::string *out)
{
    out->clear();

    while (true)
    {
        char buffer[256];
        DWORD bytesRead;

        BOOL success = ReadFile(handle, buffer, sizeof(buffer), &bytesRead, nullptr);

        if (!success || bytesRead == 0)
        {
            break;
        }

        out->append(buffer, bytesRead);
    }
}
}  // anonymous namespace

void Sleep(unsigned int milliseconds)
{
    ::Sleep(static_cast<DWORD>(milliseconds));
}

void WriteDebugMessage(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int size = vsnprintf(nullptr, 0, format, args);
    va_end(args);

    std::vector<char> buffer(size + 2);
    va_start(args, format);
    vsnprintf(buffer.data(), size + 1, format, args);
    va_end(args);

    OutputDebugStringA(buffer.data());
}

void InitCrashHandler()
{
    SetUnhandledExceptionFilter(StackTraceCrashHandler);
}

void TerminateCrashHandler()
{
    SetUnhandledExceptionFilter(nullptr);
}

void PrintStackBacktrace()
{
    CONTEXT context;
    ZeroMemory(&context, sizeof(CONTEXT));
    RtlCaptureContext(&context);
    PrintBacktrace(&context);
}

bool RunApp(const std::vector<const char *> &args,
            std::string *stdoutOut,
            std::string *stderrOut,
            int *exitCodeOut)
{
    ScopedPipe stdoutPipe;
    ScopedPipe stderrPipe;

    SECURITY_ATTRIBUTES sa_attr;
    // Set the bInheritHandle flag so pipe handles are inherited.
    sa_attr.nLength              = sizeof(SECURITY_ATTRIBUTES);
    sa_attr.bInheritHandle       = TRUE;
    sa_attr.lpSecurityDescriptor = nullptr;

    // Create pipes for stdout and stderr.  Ensure the read handles to the pipes are not inherited.
    if (stdoutOut && !CreatePipe(&stdoutPipe.readHandle, &stdoutPipe.writeHandle, &sa_attr, 0) &&
        !SetHandleInformation(stdoutPipe.readHandle, HANDLE_FLAG_INHERIT, 0))
    {
        return false;
    }
    if (stderrOut && !CreatePipe(&stderrPipe.readHandle, &stderrPipe.writeHandle, &sa_attr, 0) &&
        !SetHandleInformation(stderrPipe.readHandle, HANDLE_FLAG_INHERIT, 0))
    {
        return false;
    }

    // Concat the nicely separated arguments into one string so the application has to reparse it.
    // We don't support quotation and spaces in arguments currently.
    std::vector<char> commandLineString;
    for (const char *arg : args)
    {
        if (arg)
        {
            if (!commandLineString.empty())
            {
                commandLineString.push_back(' ');
            }
            commandLineString.insert(commandLineString.end(), arg, arg + strlen(arg));
        }
    }
    commandLineString.push_back('\0');

    STARTUPINFOA startInfo = {};

    startInfo.cb        = sizeof(STARTUPINFOA);
    startInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    if (stdoutOut)
    {
        startInfo.hStdOutput = stdoutPipe.writeHandle;
    }
    else
    {
        startInfo.hStdError = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    if (stderrOut)
    {
        startInfo.hStdError = stderrPipe.writeHandle;
    }
    else
    {
        startInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    }

    if (stderrOut || stdoutOut)
    {
        startInfo.dwFlags |= STARTF_USESTDHANDLES;
    }

    // Create the child process.
    PROCESS_INFORMATION processInfo = {};
    if (!CreateProcessA(nullptr, commandLineString.data(), nullptr, nullptr,
                        TRUE,  // Handles are inherited.
                        0, nullptr, nullptr, &startInfo, &processInfo))
    {
        return false;
    }

    // Close the write end of the pipes, so EOF can be generated when child exits.
    stdoutPipe.closeWriteHandle();
    stderrPipe.closeWriteHandle();

    // Read back the output of the child.
    if (stdoutOut)
    {
        ReadEntireFile(stdoutPipe.readHandle, stdoutOut);
    }
    if (stderrOut)
    {
        ReadEntireFile(stderrPipe.readHandle, stderrOut);
    }

    // Cleanup the child.
    bool success = WaitForSingleObject(processInfo.hProcess, INFINITE) == WAIT_OBJECT_0;

    if (success)
    {
        DWORD exitCode = 0;
        success        = GetExitCodeProcess(processInfo.hProcess, &exitCode);

        if (success)
        {
            *exitCodeOut = static_cast<int>(exitCode);
        }
    }

    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    return success;
}
}  // namespace angle
