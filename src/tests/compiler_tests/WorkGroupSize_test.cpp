//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WorkGroupSize_test.cpp:
// tests for local group size in a compute shader
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "compiler/translator/TranslatorESSL.h"
#include "tests/test_utils/compiler_test.h"

class WorkGroupSizeTest : public testing::Test
{
  public:
    WorkGroupSizeTest() {}

  protected:
    virtual void SetUp()
    {
        ShBuiltInResources resources;
        ShInitBuiltInResources(&resources);

        mTranslator = new TranslatorESSL(GL_COMPUTE_SHADER, SH_GLES3_1_SPEC);
        ASSERT_TRUE(mTranslator->Init(resources));
    }

    virtual void TearDown() { delete mTranslator; }

    // Return true when compilation succeeds
    bool compile(const std::string &shaderString)
    {
        const char *shaderStrings[] = {shaderString.c_str()};
        bool status = mTranslator->compile(shaderStrings, 1, SH_INTERMEDIATE_TREE | SH_VARIABLES);
        TInfoSink &infoSink = mTranslator->getInfoSink();
        mInfoLog            = infoSink.info.c_str();
        return status;
    }

  protected:
    std::string mInfoLog;
    TranslatorESSL *mTranslator;
};

TEST_F(WorkGroupSizeTest, OnlyLocalSizeXSpecified)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x=5) in;\n"
        "void main() {\n"
        "}\n";

    compile(shaderString);

    const std::array<int, 3> &localSize = mTranslator->getLocalSize();
    ASSERT_EQ(localSize[0], 5);
    ASSERT_EQ(localSize[1], 1);
    ASSERT_EQ(localSize[2], 1);
}

TEST_F(WorkGroupSizeTest, LocalSizeXandZ)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x=5, local_size_z=10) in;\n"
        "void main() {\n"
        "}\n";

    compile(shaderString);

    const std::array<int, 3> &localSize = mTranslator->getLocalSize();
    ASSERT_EQ(localSize[0], 5);
    ASSERT_EQ(localSize[1], 1);
    ASSERT_EQ(localSize[2], 10);
}

TEST_F(WorkGroupSizeTest, LocalSizeAll)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x=5, local_size_z=10, local_size_y=15) in;\n"
        "void main() {\n"
        "}\n";

    compile(shaderString);

    const std::array<int, 3> &localSize = mTranslator->getLocalSize();
    ASSERT_EQ(localSize[0], 5);
    ASSERT_EQ(localSize[1], 15);
    ASSERT_EQ(localSize[2], 10);
}
