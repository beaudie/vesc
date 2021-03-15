//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "JsonSerializer.h"

#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>

#include <anglebase/sha1.h>

namespace angle
{

namespace js = rapidjson;

JsonSerializer::JsonSerializer() : doc(js::kObjectType), allocator(doc.GetAllocator()) {}

JsonSerializer::~JsonSerializer() {}

void JsonSerializer::startDocument(const std::string &name)
{
    startGroup(name);
}

void JsonSerializer::startGroup(const std::string &name)
{
    auto group = std::make_unique<js::Value>(js::kObjectType);
    groupValueStack.push(std::move(group));
    groupNameStack.push(name);
}

void JsonSerializer::endGroup()
{
    auto group = std::move(groupValueStack.top());
    auto name  = std::move(groupNameStack.top());
    groupValueStack.pop();
    groupNameStack.pop();
    rapidjson::Value name_value(name.c_str(), allocator);
    groupValueStack.top()->AddMember(name_value, *group, allocator);
}

void JsonSerializer::addEntry(const std::string &name, const char *value)
{
    rapidjson::Value tag(name.c_str(), allocator);
    rapidjson::Value val(value, allocator);
    groupValueStack.top()->AddMember(tag, val, allocator);
}

void JsonSerializer::addEntry(const std::string &name, const unsigned char *blob, size_t length)
{
    rapidjson::Value tag(name.c_str(), allocator);
    unsigned char hash[angle::base::kSHA1Length];
    angle::base::SHA1HashBytes(blob, length, hash);
    std::ostringstream os;

    /* Since we don't want to de-serialize the data we just store a checksume
     * of the blob */
    os << "SHA1:";
    const char *ascii = "0123456789ABCDEF";
    for (unsigned i = 0; i < angle::base::kSHA1Length; ++i)
    {
        os << ascii[hash[i] & 0xf] << ascii[hash[i] >> 4];
    }
    addEntry(name, os.str().c_str());
}

void JsonSerializer::addEntry(const std::string &name, const std::string &value)
{
    addEntry(name, value.c_str());
}

const char *JsonSerializer::data() const
{
    return result.c_str();
}

std::vector<uint8_t> JsonSerializer::getData() const
{
    return std::vector<uint8_t>(result.begin(), result.end());
}

void JsonSerializer::endDocument()
{
    /* finalize last group */
    auto group = std::move(groupValueStack.top());
    auto name  = std::move(groupNameStack.top());

    assert(group);
    rapidjson::Value name_value(name.c_str(), allocator);
    doc.AddMember(name_value, *group, allocator);

    groupValueStack.pop();
    groupNameStack.pop();
    assert(groupValueStack.empty());
    assert(groupNameStack.empty());

    std::stringstream os;
    js::OStreamWrapper osw(os);
    js::PrettyWriter<js::OStreamWrapper> pretty_os(osw);
    doc.Accept(pretty_os);
    result = os.str();
}

size_t JsonSerializer::length() const
{
    return result.length();
}

}  // namespace angle
