//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "deqp_test.h"
#include <vector>
#include <sstream>
#include <windows.h>

static bool ExecuteCommand(const std::string &cmd, std::string &output)
{
    HANDLE outputReadHandle = NULL;
    HANDLE outputWriteHandle = NULL;

    SECURITY_ATTRIBUTES sa_attr = { 0 };
    // Set the bInheritHandle flag so pipe handles are inherited.
    sa_attr.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa_attr.bInheritHandle = TRUE;
    sa_attr.lpSecurityDescriptor = NULL;

    // Create the pipe for the child process's STDOUT.
    if (!CreatePipe(&outputReadHandle, &outputWriteHandle, &sa_attr, 0))
    {
        return false;
    }

    // Ensure the read handle to the pipe for STDOUT is not inherited.
    if (!SetHandleInformation(outputReadHandle, HANDLE_FLAG_INHERIT, 0))
    {
        CloseHandle(outputReadHandle);
        CloseHandle(outputWriteHandle);
        return false;
    }

    STARTUPINFOA start_info = { 0 };
    start_info.cb = sizeof(STARTUPINFO);
    start_info.hStdOutput = outputWriteHandle;
    start_info.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    start_info.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    start_info.dwFlags = STARTF_USESTDHANDLES;

    // Create the child process.
    PROCESS_INFORMATION process_info = { 0 };
    if (!CreateProcessA(NULL, const_cast<char*>(cmd.c_str()), NULL, NULL, TRUE, 0, NULL, NULL,
        &start_info, &process_info))
    {
        CloseHandle(outputReadHandle);
        CloseHandle(outputWriteHandle);
        return false;
    }

    // Close our writing end of pipe now. Otherwise later read would not be able
    // to detect end of child's output.
    CloseHandle(outputWriteHandle);

    // Read output from the child process's pipe for STDOUT
    std::vector<char> buffer(1024);
    while (true)
    {
        DWORD bytes_read = 0;
        BOOL success = ReadFile(outputReadHandle, buffer.data(), buffer.size(), &bytes_read, NULL);
        if (!success || bytes_read == 0)
        {
            break;
        }
        output.append(buffer.data(), bytes_read);
    }

    CloseHandle(outputReadHandle);

    // Let's wait for the process to finish.
    WaitForSingleObject(process_info.hProcess, INFINITE);

    return true;
}

bool RunDEQPTest(const std::string &name, std::string &output)
{
    // The DEQP binary is in the same directory as the test executable, get the test executable's
    // directory
    std::vector<char> executableFileBuf(MAX_PATH);
    DWORD executablePathLen = GetModuleFileNameA(NULL, executableFileBuf.data(), executableFileBuf.size());
    if (executablePathLen == 0)
    {
        return false;
    }

    std::string executableLocation = executableFileBuf.data();
    size_t lastPathSepLoc = executableLocation.find_last_of("\\/");
    if (lastPathSepLoc != std::string::npos)
    {
        executableLocation = executableLocation.substr(0, lastPathSepLoc);
    }
    else
    {
        executableLocation = "";
    }

    std::ostringstream commandline;
    commandline << executableLocation << "/" << DEQP_BINARY_NAME;
    commandline << " --deqp-visibility=hidden";
    commandline << " --deqp-case=" << name;

    return (ExecuteCommand(commandline.str(), output) && output.find("Fail (") == std::string::npos);
}
