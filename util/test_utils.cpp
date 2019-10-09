//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// system_utils: Defines common utility functions

#include "util/test_utils.h"

#include <fstream>

namespace angle
{
bool CreateTemporaryFile(char *tempFileNameOut, uint32_t maxFileNameLen)
{
    constexpr uint32_t kMaxPath = 1000u;
    char tempPath[kMaxPath];

    if (!GetTempDir(tempPath, kMaxPath))
        return false;

    return CreateTemporaryFileInDir(tempPath, tempFileNameOut, maxFileNameLen);
}

bool ReadEntireFileToString(const char *filePath, char *contentsOut, uint32_t maxLen)
{
    std::ifstream stream(filePath);
    if (!stream)
    {
        return false;
    }

    std::string contents;

    stream.seekg(0, std::ios::end);
    contents.reserve(static_cast<unsigned int>(stream.tellg()));
    stream.seekg(0, std::ios::beg);

    contents.assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

    strncpy(contentsOut, contents.c_str(), maxLen);
    return true;
}

// static
Process::~Process() = default;

void Process::Delete(Process **process)
{
    delete *process;
    *process = nullptr;
}
}  // namespace angle
