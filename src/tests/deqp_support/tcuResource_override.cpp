/*-------------------------------------------------------------------------
 * drawElements Quality Program Tester Core
 * ----------------------------------------
 *
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *//*!
 * \file
 * \brief Resource system.
 *//*--------------------------------------------------------------------*/

#include "tcuResource.hpp"

#include <stdio.h>
#include <string>
#include <unordered_map>

namespace tcu
{

DirArchive::DirArchive(const char *path) : m_path(path)
{
    // Append leading / if necessary
    if (m_path.length() > 0 && m_path[m_path.length() - 1] != '/')
        m_path += "/";
}

DirArchive::~DirArchive() {}

Resource *DirArchive::getResource(const char *name) const
{
    return static_cast<Resource *>(new FileResource((m_path + name).c_str()));
}

FileResource::FileResource(const char *filename) : Resource(std::string(filename))
{
    const std::string baseDataDir = "deqp/src/data";
    std::unordered_map<std::string, std::string> replacementMap;
    replacementMap["graphicsfuzz"] = "deqp/src/external/graphicsfuzz/data";

    std::string path(filename);

    for (const auto &it : replacementMap)
    {
        size_t index = path.find(it.first);
        if (index == std::string::npos)
        {
            continue;
        }

        // Found a keyword, so replace the base data directory
        std::string originalPath(filename);
        index = originalPath.find(baseDataDir);
        path  = originalPath.substr(0, index);
        path += it.second;
        path += originalPath.substr(index + baseDataDir.length());
        break;
    }

    m_file = fopen(path.c_str(), "rb");
    if (!m_file)
        throw ResourceError("Failed to open file", filename, __FILE__, __LINE__);
}

FileResource::~FileResource()
{
    fclose(m_file);
}

void FileResource::read(deUint8 *dst, int numBytes)
{
    int numRead = (int)fread(dst, 1, numBytes, m_file);
    TCU_CHECK(numRead == numBytes);
}

int FileResource::getSize(void) const
{
    long curPos = ftell(m_file);
    fseek(m_file, 0, SEEK_END);
    int size = (int)ftell(m_file);
    fseek(m_file, curPos, SEEK_SET);
    return size;
}

int FileResource::getPosition(void) const
{
    return (int)ftell(m_file);
}

void FileResource::setPosition(int position)
{
    fseek(m_file, (size_t)position, SEEK_SET);
}

ResourcePrefix::ResourcePrefix(const Archive &archive, const char *prefix)
    : m_archive(archive), m_prefix(prefix)
{}

Resource *ResourcePrefix::getResource(const char *name) const
{
    return m_archive.getResource((m_prefix + name).c_str());
}

}  // namespace tcu
