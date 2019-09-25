//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// test_utils_posix.cpp: Implementation of OS-specific functions for Posix systems

#include "util/test_utils.h"

#include <errno.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>
#include <cstdarg>

#include "common/platform.h"

#if !defined(ANGLE_PLATFORM_FUCHSIA)
#    include <dlfcn.h>
#    include <sys/resource.h>
#    include <sys/stat.h>
#    include <sys/types.h>
#    include <sys/wait.h>
#endif

namespace angle
{
namespace
{
struct ScopedPipe
{
    ~ScopedPipe()
    {
        closeEndPoint(0);
        closeEndPoint(1);
    }
    void closeEndPoint(int index)
    {
        if (fds[index] >= 0)
        {
            close(fds[index]);
            fds[index] = -1;
        }
    }
    int fds[2] = {
        -1,
        -1,
    };
};

void ReadEntireFile(int fd, std::string *out)
{
    out->clear();

    while (true)
    {
        char buffer[256];
        ssize_t bytesRead = read(fd, buffer, sizeof(buffer));

        // If interrupted, retry.
        if (bytesRead < 0 && errno == EINTR)
        {
            continue;
        }

        // If failed, or nothing to read, we are done.
        if (bytesRead <= 0)
        {
            break;
        }

        out->append(buffer, bytesRead);
    }
}
}  // anonymous namespace

void Sleep(unsigned int milliseconds)
{
    // On Windows Sleep(0) yields while it isn't guaranteed by Posix's sleep
    // so we replicate Windows' behavior with an explicit yield.
    if (milliseconds == 0)
    {
        sched_yield();
    }
    else
    {
        timespec sleepTime = {
            .tv_sec  = milliseconds / 1000,
            .tv_nsec = (milliseconds % 1000) * 1000000,
        };

        nanosleep(&sleepTime, nullptr);
    }
}

void SetLowPriorityProcess()
{
#if !defined(ANGLE_PLATFORM_FUCHSIA)
    setpriority(PRIO_PROCESS, getpid(), 10);
#endif
}

void WriteDebugMessage(const char *format, ...)
{
    va_list vararg;
    va_start(vararg, format);
    vfprintf(stderr, format, vararg);
    va_end(vararg);
}

bool StabilizeCPUForBenchmarking()
{
#if !defined(ANGLE_PLATFORM_FUCHSIA)
    bool success = true;
    errno        = 0;
    setpriority(PRIO_PROCESS, getpid(), -20);
    if (errno)
    {
        // A friendly warning in case the test was run without appropriate permission.
        perror(
            "Warning: setpriority failed in StabilizeCPUForBenchmarking. Process will retain "
            "default priority");
        success = false;
    }
#    if ANGLE_PLATFORM_LINUX
    cpu_set_t affinity;
    CPU_SET(0, &affinity);
    errno = 0;
    if (sched_setaffinity(getpid(), sizeof(affinity), &affinity))
    {
        perror(
            "Warning: sched_setaffinity failed in StabilizeCPUForBenchmarking. Process will retain "
            "default affinity");
        success = false;
    }
#    else
    // TODO(jmadill): Implement for non-linux. http://anglebug.com/2923
#    endif

    return success;
#else  // defined(ANGLE_PLATFORM_FUCHSIA)
    return false;
#endif
}

bool RunApp(const std::vector<const char *> &args,
            std::string *stdoutOut,
            std::string *stderrOut,
            int *exitCodeOut)
{
#if defined(ANGLE_PLATFORM_FUCHSIA)
    return false
#else
    if (args.size() == 0 || args.back() != nullptr)
    {
        return false;
    }

    ScopedPipe stdoutPipe;
    ScopedPipe stderrPipe;

    // Create pipes for stdout and stderr.
    if (stdoutOut && pipe(stdoutPipe.fds) != 0)
    {
        return false;
    }
    if (stderrOut && pipe(stderrPipe.fds) != 0)
    {
        return false;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        return false;
    }

    if (pid == 0)
    {
        // Child.  Execute the application.

        // Redirect stdout and stderr to the pipe fds.
        if (stdoutOut)
        {
            if (dup2(stdoutPipe.fds[1], STDOUT_FILENO) < 0)
            {
                _exit(errno);
            }
        }
        if (stderrOut)
        {
            if (dup2(stderrPipe.fds[1], STDERR_FILENO) < 0)
            {
                _exit(errno);
            }
        }

        // Execute the application, which doesn't return unless failed.  Note: execv takes argv as
        // `char * const *` for historical reasons.  It is safe to const_cast it:
        //
        // http://pubs.opengroup.org/onlinepubs/9699919799/functions/exec.html
        //
        // > The statement about argv[] and envp[] being constants is included to make explicit to
        // future writers of language bindings that these objects are completely constant. Due to a
        // limitation of the ISO C standard, it is not possible to state that idea in standard C.
        // Specifying two levels of const- qualification for the argv[] and envp[] parameters for
        // the exec functions may seem to be the natural choice, given that these functions do not
        // modify either the array of pointers or the characters to which the function points, but
        // this would disallow existing correct code. Instead, only the array of pointers is noted
        // as constant.
        execv(args[0], const_cast<char *const *>(args.data()));
        _exit(errno);
    }

    // Parent.  Read child output from the pipes and clean it up.

    // Close the write end of the pipes, so EOF can be generated when child exits.
    stdoutPipe.closeEndPoint(1);
    stderrPipe.closeEndPoint(1);

    // Read back the output of the child.
    if (stdoutOut)
    {
        ReadEntireFile(stdoutPipe.fds[0], stdoutOut);
    }
    if (stderrOut)
    {
        ReadEntireFile(stderrPipe.fds[0], stderrOut);
    }

    // Cleanup the child.
    int status = 0;
    do
    {
        pid_t changedPid = waitpid(pid, &status, 0);
        if (changedPid < 0 && errno == EINTR)
        {
            continue;
        }
        if (changedPid < 0)
        {
            return false;
        }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));

    // Retrieve the error code.
    if (exitCodeOut)
    {
        *exitCodeOut = WEXITSTATUS(status);
    }

    return true;
#endif  // defined(ANGLE_PLATFORM_FUCHSIA)
}
}  // namespace angle
