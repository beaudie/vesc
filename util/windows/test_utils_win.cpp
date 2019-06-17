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
#include <iostream>
#include <vector>

#include <aclapi.h>

#include "common/angleutils.h"

#include "anglebase/no_destructor.h"
#include "util/Timer.h"
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

CrashCallback *gCrashHandlerCallback;

LONG WINAPI CrashHandler(EXCEPTION_POINTERS *e)
{
    if (gCrashHandlerCallback)
    {
        (*gCrashHandlerCallback)();
    }
    return StackTraceCrashHandler(e);
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
            ::CloseHandle(readHandle);
            readHandle = nullptr;
        }
    }
    void closeWriteHandle()
    {
        if (writeHandle)
        {
            ::CloseHandle(writeHandle);
            writeHandle = nullptr;
        }
    }

    bool initPipe(SECURITY_ATTRIBUTES *securityAttribs)
    {
        if (::CreatePipe(&readHandle, &writeHandle, securityAttribs, 0) == FALSE)
        {
            std::cerr << "Error creating pipe: " << GetLastError() << "\n";
            return false;
        }

        // Ensure the read handles to the pipes are not inherited.
        return ::SetHandleInformation(readHandle, HANDLE_FLAG_INHERIT, 0) == TRUE;
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

// Returns the Win32 last error code or ERROR_SUCCESS if the last error code is
// ERROR_FILE_NOT_FOUND or ERROR_PATH_NOT_FOUND. This is useful in cases where
// the absence of a file or path is a success condition (e.g., when attempting
// to delete an item in the filesystem).
bool ReturnSuccessOnNotFound()
{
    const DWORD error_code = ::GetLastError();
    return (error_code == ERROR_FILE_NOT_FOUND || error_code == ERROR_PATH_NOT_FOUND);
}

class WindowsProcess : public Process
{
  public:
    WindowsProcess(const std::vector<const char *> &commandLineArgs,
                   bool captureStdOut,
                   bool captureStdErr)
    {
        mProcessInfo.hProcess = INVALID_HANDLE_VALUE;
        mProcessInfo.hThread  = INVALID_HANDLE_VALUE;

        std::vector<char> commandLineString;
        for (const char *arg : commandLineArgs)
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

        // Set the bInheritHandle flag so pipe handles are inherited.
        SECURITY_ATTRIBUTES securityAttribs;
        securityAttribs.nLength              = sizeof(SECURITY_ATTRIBUTES);
        securityAttribs.bInheritHandle       = TRUE;
        securityAttribs.lpSecurityDescriptor = nullptr;

        STARTUPINFOA startInfo = {};

        // Create pipes for stdout and stderr.
        startInfo.cb        = sizeof(STARTUPINFOA);
        startInfo.hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);
        if (captureStdOut)
        {
            if (!mStdoutPipe.initPipe(&securityAttribs))
            {
                return;
            }
            startInfo.hStdOutput = mStdoutPipe.writeHandle;
        }
        else
        {
            startInfo.hStdOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
        }

        if (captureStdErr)
        {
            if (!mStderrPipe.initPipe(&securityAttribs))
            {
                return;
            }
            startInfo.hStdError = mStderrPipe.writeHandle;
        }
        else
        {
            startInfo.hStdError = ::GetStdHandle(STD_ERROR_HANDLE);
        }

        if (captureStdOut || captureStdErr)
        {
            startInfo.dwFlags |= STARTF_USESTDHANDLES;
        }

        // Create the child process.
        if (::CreateProcessA(nullptr, commandLineString.data(), nullptr, nullptr,
                             TRUE,  // Handles are inherited.
                             0, nullptr, nullptr, &startInfo, &mProcessInfo) == FALSE)
        {
            std::cerr << "CreateProcessA Error code: " << GetLastError() << "\n";
            return;
        }

        mStarted = true;
        mTimer.start();

        // Close the write end of the pipes, so EOF can be generated when child exits.
        mStdoutPipe.closeWriteHandle();
        mStderrPipe.closeWriteHandle();
    }

    ~WindowsProcess() override
    {
        if (mProcessInfo.hProcess != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(mProcessInfo.hProcess);
        }
        if (mProcessInfo.hThread != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(mProcessInfo.hThread);
        }
    }

    bool started() override { return mStarted; }

    void getStdout(std::string *stdoutOut) override
    {
        ReadEntireFile(mStdoutPipe.readHandle, stdoutOut);
    }

    void getStderr(std::string *stderrOut) override
    {
        ReadEntireFile(mStderrPipe.readHandle, stderrOut);
    }

    bool finish() override
    {
        DWORD result = ::WaitForSingleObject(mProcessInfo.hProcess, INFINITE);
        mTimer.stop();
        return result == WAIT_OBJECT_0;
    }

    bool finished() override
    {
        if (!mStarted)
            return false;

        DWORD result = ::WaitForSingleObject(mProcessInfo.hProcess, 0);
        if (result == WAIT_OBJECT_0)
        {
            mTimer.stop();
            return true;
        }
        if (result == WAIT_TIMEOUT)
            return false;

        mTimer.stop();
        std::cerr << "Unexpected result from WaitForSingleObject: " << result
                  << ". Last error: " << ::GetLastError() << "\n";
        return false;
    }

    int getExitCode() override
    {
        if (!mStarted)
            return -1;

        if (mProcessInfo.hProcess == INVALID_HANDLE_VALUE)
            return -1;

        DWORD exitCode = 0;
        if (::GetExitCodeProcess(mProcessInfo.hProcess, &exitCode) == FALSE)
            return false;

        return static_cast<int>(exitCode);
    }

    double getElapsedTimeSeconds() override { return mTimer.getElapsedTime(); }

    bool kill() override
    {
        if (!mStarted)
            return true;

        HANDLE newHandle;
        if (::DuplicateHandle(::GetCurrentProcess(), mProcessInfo.hProcess, ::GetCurrentProcess(),
                              &newHandle, PROCESS_ALL_ACCESS, false,
                              DUPLICATE_CLOSE_SOURCE) == FALSE)
        {
            std::cerr << "Error getting permission to terminate process: " << ::GetLastError()
                      << "\n";
            return false;
        }
        mProcessInfo.hProcess = newHandle;

        if (::TerminateThread(mProcessInfo.hThread, 1) == FALSE)
        {
            std::cerr << "TerminateThread failed: " << GetLastError() << "\n";
            return false;
        }

        if (::TerminateProcess(mProcessInfo.hProcess, 1) == FALSE)
        {
            std::cerr << "TerminateProcess failed: " << GetLastError() << "\n";
            return false;
        }

        mStarted = false;
        mTimer.stop();
        return true;
    }

  private:
    bool mStarted = false;
    ScopedPipe mStdoutPipe;
    ScopedPipe mStderrPipe;
    PROCESS_INFORMATION mProcessInfo = {};
    Timer mTimer;
};
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

void InitCrashHandler(CrashCallback *callback)
{
    if (callback)
    {
        gCrashHandlerCallback = callback;
    }
    SetUnhandledExceptionFilter(CrashHandler);
}

void TerminateCrashHandler()
{
    gCrashHandlerCallback = nullptr;
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
    WindowsProcess process(args, stdoutOut != nullptr, stderrOut != nullptr);

    if (!process.started())
        return false;

    // Read back the output of the child.
    if (stdoutOut)
    {
        process.getStdout(stdoutOut);
    }
    if (stderrOut)
    {
        process.getStderr(stderrOut);
    }

    // Cleanup the child.
    if (!process.finish())
        return false;

    *exitCodeOut = process.getExitCode();
    return true;
}

Process *RunProcessAsync(const std::vector<const char *> &args,
                         bool captureStdout,
                         bool captureStderr)
{
    return new WindowsProcess(args, captureStdout, captureStderr);
}

bool GetTempDir(std::string *tempDirOut)
{
    char tempPath[MAX_PATH + 1];
    DWORD pathLen = ::GetTempPathA(MAX_PATH, tempPath);
    if (pathLen >= MAX_PATH || pathLen <= 0)
        return false;
    *tempDirOut = tempPath;
    return true;
}

bool CreateTemporaryFileInDir(const std::string &dir, std::string *tempFileNameOut)
{
    char fileName[MAX_PATH + 1];
    if (::GetTempFileNameA(dir.c_str(), "ANGLE", 0, fileName) == 0)
        return false;

    *tempFileNameOut = fileName;
    return true;
}

bool DeleteFile(const char *path)
{
    if (strlen(path) >= MAX_PATH)
        return false;

    const DWORD attr = ::GetFileAttributesA(path);
    // Report success if the file or path does not exist.
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        return ReturnSuccessOnNotFound();
    }

    // Clear the read-only bit if it is set.
    if ((attr & FILE_ATTRIBUTE_READONLY) &&
        !::SetFileAttributesA(path, attr & ~FILE_ATTRIBUTE_READONLY))
    {
        // It's possible for |path| to be gone now under a race with other deleters.
        return ReturnSuccessOnNotFound();
    }

    // We don't handle directories right now.
    if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
    {
        return false;
    }

    return !!::DeleteFileA(path) ? true : ReturnSuccessOnNotFound();
}

int NumberOfProcessors()
{
    return ::GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
}
}  // namespace angle
