//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShCompile_test.cpp
//   Test the ShCompile interface with different parameters.
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"

#include <thread>

class ShCompileTest : public testing::Test
{
  public:
    ShCompileTest() {}

  protected:
    void SetUp() override
    {
        ShInitBuiltInResources(&mResources);
        mCompiler = ShConstructCompiler(GL_FRAGMENT_SHADER, SH_WEBGL_SPEC,
                                        SH_GLSL_COMPATIBILITY_OUTPUT, &mResources);
        ASSERT_TRUE(mCompiler != nullptr) << "Compiler could not be constructed.";
    }

    void TearDown() override
    {
        if (mCompiler)
        {
            ShDestruct(mCompiler);
            mCompiler = nullptr;
        }
    }

    bool testCompiler(ShHandle compiler,
                      const char **shaderStrings,
                      int stringCount,
                      bool expectation)
    {
        bool success                  = ShCompile(compiler, shaderStrings, stringCount, 0);
        const std::string &compileLog = ShGetInfoLog(compiler);
        EXPECT_EQ(expectation, success) << compileLog;
        return (success == expectation);
    }

    void testCompile(const char **shaderStrings, int stringCount, bool expectation)
    {
        testCompiler(mCompiler, shaderStrings, stringCount, expectation);
    }

    ShBuiltInResources mResources;
    ShHandle mCompiler = nullptr;
};

// Test calling ShCompile with more than one shader source string.
TEST_F(ShCompileTest, MultipleShaderStrings)
{
    const std::string &shaderString1 =
        "precision mediump float;\n"
        "void main() {\n";
    const std::string &shaderString2 =
        "    gl_FragColor = vec4(0.0);\n"
        "}";

    const char *shaderStrings[] = {shaderString1.c_str(), shaderString2.c_str()};

    testCompile(shaderStrings, 2, true);
}

// Test calling ShCompile with a tokens split into different shader source strings.
TEST_F(ShCompileTest, TokensSplitInShaderStrings)
{
    const std::string &shaderString1 =
        "precision mediump float;\n"
        "void ma";
    const std::string &shaderString2 =
        "in() {\n"
        "#i";
    const std::string &shaderString3 =
        "f 1\n"
        "    gl_FragColor = vec4(0.0);\n"
        "#endif\n"
        "}";

    const char *shaderStrings[] = {shaderString1.c_str(), shaderString2.c_str(),
                                   shaderString3.c_str()};

    testCompile(shaderStrings, 3, true);
}

class ShCompileMultithread : public ShCompileTest
{
  public:
    ShCompileMultithread() {}

    bool testNewCompiler(const ShBuiltInResources &resources,
                         const char **shaderStrings,
                         int stringCount,
                         bool expectation)
    {
        ShHandle compiler = ShConstructCompiler(GL_FRAGMENT_SHADER, SH_WEBGL_SPEC,
                                                SH_GLSL_COMPATIBILITY_OUTPUT, &resources);
        if (!compiler)
            return false;

        bool result =
            ShCompileTest::testCompiler(compiler, shaderStrings, stringCount, expectation);
        ShDestruct(compiler);
        return result;
    }

    std::thread spawnThread(const char **shaderStrings, int stringCount, bool expectation)
    {
        return std::thread(&ShCompileMultithread::testNewCompiler, this, mResources, shaderStrings,
                           stringCount, expectation);
    }

    static const int kThreads = 2000;
};

// Test compilation on multiple threads.
TEST_F(ShCompileMultithread, TestManyThreads)
{
    std::vector<std::thread> manyThreads;

    const std::string &shaderString =
        "precision mediump float;\n"
        "void main() {\n"
        "    gl_FragColor = vec4(0.0);\n"
        "}";

    const char *shaderStrings[] = {shaderString.c_str()};

    for (int cnt = 0; cnt < kThreads; ++cnt)
    {
        manyThreads.push_back(spawnThread(shaderStrings, 1, true));
    }

    for (auto &thread : manyThreads)
    {
        thread.join();
    }
}
