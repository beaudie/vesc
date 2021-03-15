//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// JsonSerializer.h: Implementation of a JSON based serializer
//

#ifndef LIBANGLE_JSONSERIALIZER_H_
#define LIBANGLE_JSONSERIALIZER_H_

#include "common/angleutils.h"

#include <rapidjson/document.h>

#include <memory>
#include <sstream>
#include <stack>
#include <type_traits>

namespace angle
{

/* Rapidjson has problems picking the right AddMember template for some
 * integer types, so let's just make all integral values use 64 bit variants
 */
template <typename T, bool is_integer, bool is_signed>
struct JsonSerializerAddEntry
{
    static void apply(rapidjson::Value *node,
                      const std::string &name,
                      T value,
                      rapidjson::Document::AllocatorType &allocator)
    {
        rapidjson::Value tag(name.c_str(), allocator);
        node->AddMember(tag, value, allocator);
    }
};

template <typename T>
struct JsonSerializerAddEntry<T, true, true>
{
    static void apply(rapidjson::Value *node,
                      const std::string &name,
                      int64_t value,
                      rapidjson::Document::AllocatorType &allocator)
    {
        rapidjson::Value tag(name.c_str(), allocator);
        node->AddMember(tag, value, allocator);
    }
};

template <typename T>
struct JsonSerializerAddEntry<T, true, false>
{
    static void apply(rapidjson::Value *node,
                      const std::string &name,
                      uint64_t value,
                      rapidjson::Document::AllocatorType &allocator)
    {
        rapidjson::Value tag(name.c_str(), allocator);
        node->AddMember(tag, value, allocator);
    }
};

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
        using AddEntry =
            JsonSerializerAddEntry<T, std::is_integral<T>::value, std::is_signed<T>::value>;
        AddEntry::apply(mGroupValueStack.top().get(), name, value, mAllocator);
    }

    template <typename T>
    void addEntry(const std::string &name, const std::vector<T> &value)
    {
        rapidjson::Value tag(name.c_str(), mAllocator);
        rapidjson::Value array(rapidjson::kArrayType);
        array.SetArray();

        for (auto &&v : value)
            array.PushBack(v, mAllocator);

        mGroupValueStack.top()->AddMember(tag, array, mAllocator);
    }

    void addEntry(const std::string &name, bool value);

    void addEntry(const std::string &name, const char *value);

    void addEntry(const std::string &name, const std::string &value);

    void addEntry(const std::string &name, const uint8_t *value, size_t length);

    void startGroup(const std::string &name);
    void endGroup();

    const char *data() const;

    std::vector<uint8_t> getData() const;

    size_t length() const;

  private:
    using ValuePointer = std::unique_ptr<rapidjson::Value>;

    rapidjson::Document mDoc;
    rapidjson::Document::AllocatorType &mAllocator;
    std::stack<std::string> mGroupNameStack;
    std::stack<ValuePointer> mGroupValueStack;
    std::string mResult;
};

}  // namespace angle

#endif  // JSONSERIALIZER_H
