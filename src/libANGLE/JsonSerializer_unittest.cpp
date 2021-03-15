//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "JsonSerializer.h"

#include <gtest/gtest.h>

class JsonSerializerTest : public ::testing::Test
{
  protected:
    void SetUp() override;
    void check(const std::string &expect);

    gl::JsonSerializer js;
};

TEST_F(JsonSerializerTest, NamedIntValue1)
{
    js.addEntry("test1", 1);

    const std::string expect =
        R"({
    "context": {
        "test1": 1
    }
})";
    check(expect);
}

TEST_F(JsonSerializerTest, NamedIntValue2)
{
    js.startDocument("context");

    js.addEntry("test2", 2);

    const std::string expect =
        R"({
    "context": {
        "test2": 2
    }
})";

    check(expect);
}

TEST_F(JsonSerializerTest, NamedStringValue)
{
    js.addEntry("test2", "value");

    const std::string expect =
        R"({
    "context": {
        "test2": "value"
    }
})";

    check(expect);
}

TEST_F(JsonSerializerTest, ByteArrayValue)
{
    const uint32_t value[2] = {10, 0xffccddaa};
    js.addEntry("test2", reinterpret_cast<const unsigned char *>(value), 10);

    const std::string expect =
        R"({
    "context": {
        "test2": "SHA1:4D18FA7780A1D377D7CBB14309953B139BBF2855"
    }
})";

    check(expect);
}

TEST_F(JsonSerializerTest, IntVectorValue)
{
    std::vector<int> v = {0, 1, -1};

    js.addEntry("test2", v);

    const std::string expect =
        R"({
    "context": {
        "test2": [
            0,
            1,
            -1
        ]
    }
})";

    check(expect);
}

TEST_F(JsonSerializerTest, NamedBoolValues)
{
    js.addEntry("test_false", false);
    js.addEntry("test_true", true);

    const std::string expect =
        R"({
    "context": {
        "test_false": false,
        "test_true": true
    }
})";

    check(expect);
}

TEST_F(JsonSerializerTest, GroupedIntValue)
{
    js.startGroup("group");
    js.addEntry("test1", 1);
    js.addEntry("test2", 2);
    js.endGroup();

    const std::string expect =
        R"({
    "context": {
        "group": {
            "test1": 1,
            "test2": 2
        }
    }
})";

    check(expect);
}

void JsonSerializerTest::SetUp()
{
    js.startDocument("context");
}

void JsonSerializerTest::check(const std::string &expect)
{
    js.endDocument();
    EXPECT_EQ(js.data(), expect);
    EXPECT_EQ(js.length(), expect.length());
    std::vector<uint8_t> expect_as_ubyte(expect.begin(), expect.end());
    EXPECT_EQ(js.getData(), expect_as_ubyte);
}
