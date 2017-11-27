//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ScalarizeVecAndMatConstructorArgs_test.cpp:
//   Tests for scalarizing vector and matrix constructor args.
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

namespace
{

class ScalarizeVecAndMatConstructorArgsTest : public MatchOutputCodeTest
{
  public:
    ScalarizeVecAndMatConstructorArgsTest()
        : MatchOutputCodeTest(GL_FRAGMENT_SHADER,
                              SH_SCALARIZE_VEC_AND_MAT_CONSTRUCTOR_ARGS,
                              SH_ESSL_OUTPUT)
    {
    }
};

// Verifies that scalarizing vector and matrix constructor args inside a sequence operator preserves
// correct order of operations.
TEST_F(ScalarizeVecAndMatConstructorArgsTest, SequenceOperator)
{
    const std::string shaderString =
        R"(
        precision mediump float;

        uniform vec2 u;

        void main()
        {
            vec2 v = u;
            mat2 m = (v[0] += 1.0, mat2(v, v[1], -v[0]));
            gl_FragColor = vec4(m[0], m[1]);
        })";
    compile(shaderString);

    std::vector<const char *> expectedStrings = {"_uv[0] += 1.0", "-_uv[0]"};

    EXPECT_TRUE(foundInCodeInOrder(expectedStrings));
}

// Verifies that scalarizing vector and matrix constructor args inside multiple declarations
// preserves the correct order of operations.
TEST_F(ScalarizeVecAndMatConstructorArgsTest, MultiDeclaration)
{
    const std::string shaderString =
        R"(
        precision mediump float;

        uniform vec2 u;

        void main()
        {
            vec2 v = vec2(u[0]),
                 w = mat2(v, v) * u;
            gl_FragColor = vec4(v, w);
        })";
    compile(shaderString);

    std::vector<const char *> expectedStrings = {"vec2(_uu[0])", "mat2("};

    EXPECT_TRUE(foundInCodeInOrder(expectedStrings));
}

}  // anonymous namespace
