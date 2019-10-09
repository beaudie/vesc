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
bool CreateTemporaryFile(std::string *tempFileNameOut)
{
    std::string tempPath;

    if (!GetTempDir(&tempPath))
        return false;

    return CreateTemporaryFileInDir(tempPath, tempFileNameOut);
}

bool ReadEntireFileToString(const std::string &filePath, std::string *contentsOut)
{
    std::ifstream stream(filePath.c_str());
    if (!stream)
    {
        return false;
    }

    stream.seekg(0, std::ios::end);
    contentsOut->reserve(static_cast<unsigned int>(stream.tellg()));
    stream.seekg(0, std::ios::beg);

    contentsOut->assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

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
