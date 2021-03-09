// Copyright (c) 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <jni.h>
#include <vector>

#include "base/android/scoped_java_ref.h"
#include "third_party/angle/src/tests/native_test_jni_headers/AngleNativeTestDelegate_jni.h"

#include <android/log.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#include "third_party/angle/src/tests/test_utils/runner/android/native_test_util.h"

using base::android::JavaParamRef;

// The main function of the program to be wrapped as a test apk.
extern int main(int argc, char **argv);

namespace
{

const char kLogTag[]        = "chromium";
const char kCrashedMarker[] = "[ CRASHED      ]\n";

// The list of signals which are considered to be crashes.
const int kExceptionSignals[] = {SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGBUS, -1};

struct sigaction g_old_sa[NSIG];

// This function runs in a compromised context. It should not allocate memory.
void SignalHandler(int sig, siginfo_t *info, void *reserved)
{
    // Output the crash marker.
    write(STDOUT_FILENO, kCrashedMarker, sizeof(kCrashedMarker) - 1);
    g_old_sa[sig].sa_sigaction(sig, info, reserved);
}

std::string ASCIIJavaStringToUTF8(JNIEnv *env, jstring str)
{
    if (!str)
    {
        return "";
    }

    const jsize length = env->GetStringLength(str);
    if (!length)
    {
        return "";
    }

    // JNI's GetStringUTFChars() returns strings in Java "modified" UTF8, so
    // instead get the String in UTF16. As the input is ASCII, drop the higher
    // bytes.
    const jchar *jchars   = env->GetStringChars(str, NULL);
    const char16_t *chars = reinterpret_cast<const char16_t *>(jchars);
    std::string out(chars, chars + length);
    env->ReleaseStringChars(str, jchars);
    return out;
}

void InstallExceptionHandlers()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_sigaction = SignalHandler;
    sa.sa_flags     = SA_SIGINFO;

    for (unsigned int i = 0; kExceptionSignals[i] != -1; ++i)
    {
        sigaction(kExceptionSignals[i], &sa, &g_old_sa[kExceptionSignals[i]]);
    }
}

void AndroidLog(int priority, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    __android_log_vprint(priority, kLogTag, format, args);
    va_end(args);
}

}  // anonymous namespace

static void JNI_AngleNativeTestDelegate_RunTests(
    JNIEnv *env,
    const base::android::JavaParamRef<jstring> &jcommand_line_flags,
    const base::android::JavaParamRef<jstring> &jcommand_line_file_path,
    const base::android::JavaParamRef<jstring> &jstdout_file_path)
{
    InstallExceptionHandlers();

    const std::string command_line_flags(ASCIIJavaStringToUTF8(env, jcommand_line_flags.obj()));
    const std::string command_line_file_path(
        ASCIIJavaStringToUTF8(env, jcommand_line_file_path.obj()));
    const std::string stdout_file_path(ASCIIJavaStringToUTF8(env, jstdout_file_path.obj()));

    std::vector<std::string> args;
    if (command_line_file_path.empty())
        args.push_back("_");
    else
        angle::ParseArgsFromCommandLineFile(command_line_file_path.c_str(), &args);
    angle::ParseArgsFromString(command_line_flags, &args);

    // A few options, such "--gtest_list_tests", will just use printf directly
    // Always redirect stdout to a known file.
    if (freopen(stdout_file_path.c_str(), "a+", stdout) == NULL)
    {
        AndroidLog(ANDROID_LOG_ERROR, "Failed to redirect stream to file: %s: %s\n",
                   stdout_file_path.c_str(), strerror(errno));
        exit(EXIT_FAILURE);
    }

    dup2(STDOUT_FILENO, STDERR_FILENO);

    std::vector<char *> argv;
    int argc = angle::ArgsToArgv(args, &argv);

    angle::ScopedMainEntryLogger scoped_main_entry_logger;
    main(argc, &argv[0]);
}
