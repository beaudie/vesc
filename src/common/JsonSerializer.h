//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_JSONSERIALIZER_H_
#define LIBANGLE_JSONSERIALIZER_H_

#include "common/angleutils.h"

#include "rapidjson/document.h"

#include <memory>
#include <stack>

namespace angle
{

class JsonSerializer : public angle::NonCopyable
{
  public:
    JsonSerializer();
    ~JsonSerializer();

    void startDocument(const std::string &name);
    void endDocument();

    template <typename T>
    void addEntry(const std::string &name, T value)
    {
        rapidjson::Value tag(name.c_str(), allocator);
        groupValueStack.top()->AddMember(tag, value, allocator);
    }

    template <typename T>
    void addEntry(const std::string &name, const std::vector<T> &value)
    {
        rapidjson::Value tag(name.c_str(), allocator);
        rapidjson::Value array(rapidjson::kArrayType);
        array.SetArray();

        for (auto &&v : value)
            array.PushBack(v, allocator);

        groupValueStack.top()->AddMember(tag, array, allocator);
    }

    void addEntry(const std::string &name, const char *value);

    void addEntry(const std::string &name, const std::string &value);

    void addEntry(const std::string &name, const unsigned char *value, size_t length);

    void startGroup(const std::string &name);
    void endGroup();

    const char *data() const;

    std::vector<uint8_t> getData() const;

    size_t length() const;

  private:
    rapidjson::Document doc;
    rapidjson::Document::AllocatorType &allocator;
    using ValuePointer = std::unique_ptr<rapidjson::Value>;
    std::stack<ValuePointer> groupValueStack;
    std::stack<std::string> groupNameStack;
    std::string result;
};

}  // namespace angle

#endif  // JSONSERIALIZER_H
