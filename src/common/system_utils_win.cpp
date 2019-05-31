//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// system_utils_win.cpp: Implementation of OS-specific functions for Windows

#include "common/system_utils.h"

#include "common/third_party/StackWalker/src/StackWalker.h"

#include <DbgHelp.h>
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include <array>
#include <vector>

namespace angle
{
namespace
{
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
}  // anonymous namespace

std::string GetExecutablePath()
{
    std::array<char, MAX_PATH> executableFileBuf;
    DWORD executablePathLen = GetModuleFileNameA(nullptr, executableFileBuf.data(),
                                                 static_cast<DWORD>(executableFileBuf.size()));
    return (executablePathLen > 0 ? std::string(executableFileBuf.data()) : "");
}

std::string GetExecutableDirectory()
{
    std::string executablePath = GetExecutablePath();
    size_t lastPathSepLoc      = executablePath.find_last_of("\\/");
    return (lastPathSepLoc != std::string::npos) ? executablePath.substr(0, lastPathSepLoc) : "";
}

const char *GetSharedLibraryExtension()
{
    return "dll";
}

Optional<std::string> GetCWD()
{
    std::array<char, MAX_PATH> pathBuf;
    DWORD result = GetCurrentDirectoryA(static_cast<DWORD>(pathBuf.size()), pathBuf.data());
    if (result == 0)
    {
        return Optional<std::string>::Invalid();
    }
    return std::string(pathBuf.data());
}

bool SetCWD(const char *dirName)
{
    return (SetCurrentDirectoryA(dirName) == TRUE);
}

bool UnsetEnvironmentVar(const char *variableName)
{
    return (SetEnvironmentVariableA(variableName, nullptr) == TRUE);
}

bool SetEnvironmentVar(const char *variableName, const char *value)
{
    return (SetEnvironmentVariableA(variableName, value) == TRUE);
}

std::string GetEnvironmentVar(const char *variableName)
{
    std::array<char, MAX_PATH> oldValue;
    DWORD result =
        GetEnvironmentVariableA(variableName, oldValue.data(), static_cast<DWORD>(oldValue.size()));
    if (result == 0)
    {
        return std::string();
    }
    else
    {
        return std::string(oldValue.data());
    }
}

const char *GetPathSeparator()
{
    return ";";
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

class Win32Library : public Library
{
  public:
    Win32Library(const char *libraryName)
    {
        char buffer[MAX_PATH];
        int ret = snprintf(buffer, MAX_PATH, "%s.%s", libraryName, GetSharedLibraryExtension());
        if (ret > 0 && ret < MAX_PATH)
        {
            mModule = LoadLibraryA(buffer);
        }
    }

    ~Win32Library() override
    {
        if (mModule)
        {
            FreeLibrary(mModule);
        }
    }

    void *getSymbol(const char *symbolName) override
    {
        if (!mModule)
        {
            return nullptr;
        }

        return reinterpret_cast<void *>(GetProcAddress(mModule, symbolName));
    }

    void *getNative() const override { return reinterpret_cast<void *>(mModule); }

  private:
    HMODULE mModule = nullptr;
};

Library *OpenSharedLibrary(const char *libraryName)
{
    return new Win32Library(libraryName);
}

bool IsDirectory(const char *filename)
{
    WIN32_FILE_ATTRIBUTE_DATA fileInformation;

    BOOL result = GetFileAttributesExA(filename, GetFileExInfoStandard, &fileInformation);
    if (result)
    {
        DWORD attribs = fileInformation.dwFileAttributes;
        return (attribs != INVALID_FILE_ATTRIBUTES) && ((attribs & FILE_ATTRIBUTE_DIRECTORY) > 0);
    }

    return false;
}

bool IsDebuggerAttached()
{
    return !!::IsDebuggerPresent();
}

void BreakDebugger()
{
    __debugbreak();
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
}  // namespace angle
