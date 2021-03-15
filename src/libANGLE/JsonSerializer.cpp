#include "JsonSerializer.h"

#include <rapidjson/filewritestream.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include "rapidjson/document.h"

#include <anglebase/sha1.h>

namespace gl
{

namespace js = rapidjson;

JsonSerializer::JsonSerializer() : doc(js::kObjectType), allocator(doc.GetAllocator()) {}

JsonSerializer::~JsonSerializer() {}

void JsonSerializer::startDocument(const std::string &name)
{
    auto group = std::make_shared<js::Value>(js::kObjectType);
    groupStack.push(std::make_pair(name, group));
}

void JsonSerializer::startGroup(const std::string &name)
{
    auto group = std::make_shared<js::Value>(js::kObjectType);
    groupStack.push(std::make_pair(name, group));
}

void JsonSerializer::endGroup()
{
    auto group = std::move(groupStack.top());
    groupStack.pop();
    rapidjson::Value name_value(group.first.c_str(), allocator);
    groupStack.top().second->AddMember(name_value, *group.second, allocator);
}

void JsonSerializer::addEntry(const std::string &name, const char *value)
{
    rapidjson::Value tag(name.c_str(), allocator);
    rapidjson::Value val(value, allocator);
    groupStack.top().second->AddMember(tag, val, allocator);
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
    const GroupEntry &entry = groupStack.top();
    rapidjson::Value name_value(entry.first.c_str(), allocator);
    doc.AddMember(name_value, *entry.second, allocator);

    groupStack.pop();
    assert(groupStack.empty());

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

}  // namespace gl
