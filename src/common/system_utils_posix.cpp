//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// system_utils_posix.cpp: Implementation of POSIX OS-specific functions.

#include "system_utils.h"

#include <array>
#include <iostream>

#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <signal.h>
#include <sys/mman.h>

namespace angle
{

namespace
{
std::string GetModulePath(void *moduleOrSymbol)
{
    Dl_info dlInfo;
    if (dladdr(moduleOrSymbol, &dlInfo) == 0)
    {
        return "";
    }

    return dlInfo.dli_fname;
}
}  // namespace

Optional<std::string> GetCWD()
{
    std::array<char, 4096> pathBuf;
    char *result = getcwd(pathBuf.data(), pathBuf.size());
    if (result == nullptr)
    {
        return Optional<std::string>::Invalid();
    }
    return std::string(pathBuf.data());
}

bool SetCWD(const char *dirName)
{
    return (chdir(dirName) == 0);
}

bool UnsetEnvironmentVar(const char *variableName)
{
    return (unsetenv(variableName) == 0);
}

bool SetEnvironmentVar(const char *variableName, const char *value)
{
    return (setenv(variableName, value, 1) == 0);
}

std::string GetEnvironmentVar(const char *variableName)
{
    const char *value = getenv(variableName);
    return (value == nullptr ? std::string() : std::string(value));
}

const char *GetPathSeparatorForEnvironmentVar()
{
    return ":";
}

std::string GetModuleDirectory()
{
    std::string directory;
    static int placeholderSymbol = 0;
    std::string moduleName       = GetModulePath(&placeholderSymbol);
    if (!moduleName.empty())
    {
        directory = moduleName.substr(0, moduleName.find_last_of('/') + 1);
    }

    // Ensure we return the full path to the module, not the relative path
    Optional<std::string> cwd = GetCWD();
    if (cwd.valid() && !IsFullPath(directory))
    {
        directory = ConcatenatePath(cwd.value(), directory);
    }
    return directory;
}

class PosixLibrary : public Library
{
  public:
    PosixLibrary(const std::string &fullPath, int extraFlags)
        : mModule(dlopen(fullPath.c_str(), RTLD_NOW | extraFlags))
    {}

    ~PosixLibrary() override
    {
        if (mModule)
        {
            dlclose(mModule);
        }
    }

    void *getSymbol(const char *symbolName) override
    {
        if (!mModule)
        {
            return nullptr;
        }

        return dlsym(mModule, symbolName);
    }

    void *getNative() const override { return mModule; }

    std::string getPath() const override
    {
        if (!mModule)
        {
            return "";
        }

        return GetModulePath(mModule);
    }

  private:
    void *mModule = nullptr;
};

Library *OpenSharedLibrary(const char *libraryName, SearchType searchType)
{
    std::string libraryWithExtension = std::string(libraryName) + "." + GetSharedLibraryExtension();
    return OpenSharedLibraryWithExtension(libraryWithExtension.c_str(), searchType);
}

Library *OpenSharedLibraryWithExtension(const char *libraryName, SearchType searchType)
{
    std::string directory;
    if (searchType == SearchType::ModuleDir)
    {
#if ANGLE_PLATFORM_IOS
        // On iOS, shared libraries must be loaded from within the app bundle.
        directory = GetExecutableDirectory() + "/Frameworks/";
#else
        directory = GetModuleDirectory();
#endif
    }

    int extraFlags = 0;
    if (searchType == SearchType::AlreadyLoaded)
    {
        extraFlags = RTLD_NOLOAD;
    }

    std::string fullPath = directory + libraryName;
#if ANGLE_PLATFORM_IOS
    // On iOS, dlopen needs a suffix on the framework name to work.
    fullPath = fullPath + "/" + libraryName;
#endif
    return new PosixLibrary(fullPath, extraFlags);
}

bool IsDirectory(const char *filename)
{
    struct stat st;
    int result = stat(filename, &st);
    return result == 0 && ((st.st_mode & S_IFDIR) == S_IFDIR);
}

bool IsDebuggerAttached()
{
    // This could have a fuller implementation.
    // See https://cs.chromium.org/chromium/src/base/debug/debugger_posix.cc
    return false;
}

void BreakDebugger()
{
    // This could have a fuller implementation.
    // See https://cs.chromium.org/chromium/src/base/debug/debugger_posix.cc
    abort();
}

const char *GetExecutableExtension()
{
    return "";
}

char GetPathSeparator()
{
    return '/';
}

std::string GetRootDirectory()
{
    return "/";
}

class PosixPageFaultHandler : public PageFaultHandler
{
  public:
    PosixPageFaultHandler(PageFaultCallback callback) : PageFaultHandler(callback) {}
    ~PosixPageFaultHandler() override {}

    bool enable() override;
    bool disable() override;
    void handle(int sig, siginfo_t *info, void *unused);

  private:
    struct sigaction mDefaultAction;
};

PosixPageFaultHandler *gPosixPageFaultHandler = nullptr;
static void SegfaultHandlerFunction(int sig, siginfo_t *info, void *unused)
{
    gPosixPageFaultHandler->handle(sig, info, unused);
}

void PosixPageFaultHandler::handle(int sig, siginfo_t *info, void *unused)
{
    bool found = false;
    if (sig == SIGSEGV && info->si_code == SEGV_ACCERR)
    {
        found = mCallback(reinterpret_cast<uintptr_t>(info->si_addr));
    }

    // Fall back to default signal handler
    if (!found)
    {
        fprintf(stderr, "Falling back to default signal handler.\n");
        mDefaultAction.sa_sigaction(sig, info, unused);
    }
}

bool PosixPageFaultHandler::disable()
{
    return sigaction(SIGSEGV, &mDefaultAction, nullptr) != 1;
}

bool PosixPageFaultHandler::enable()
{
    struct sigaction sigAction;
    sigAction.sa_flags     = SA_SIGINFO;
    sigAction.sa_sigaction = &SegfaultHandlerFunction;
    sigemptyset(&sigAction.sa_mask);

    return sigaction(SIGSEGV, &sigAction, &mDefaultAction) != 1;
}

static bool SetProtection(uintptr_t p, size_t size, uint32_t protections)
{
    int ret = mprotect(reinterpret_cast<void *>(p), size, protections);
    if (ret < 0)
    {
        fprintf(stderr, "mprotect failed: %s.\n", strerror(errno));
    }
    return ret == 0;
}

bool Protect(uintptr_t start, size_t size)
{
    return SetProtection(start, size, PROT_READ);
}

bool UnProtect(uintptr_t start, size_t size)
{
    return SetProtection(start, size, PROT_READ | PROT_WRITE);
}

size_t GetPageSize()
{
    long pageSize = sysconf(_SC_PAGE_SIZE);
    if (pageSize < 0)
    {
        fprintf(stderr, "Could not get sysconf page size: %s.\n", strerror(errno));
        return 0;
    }
    return static_cast<size_t>(pageSize);
}

PageFaultHandler *CreatePageFaultHandler(PageFaultCallback callback)
{
    gPosixPageFaultHandler = new PosixPageFaultHandler(callback);
    return gPosixPageFaultHandler;
}
}  // namespace angle
