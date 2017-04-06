//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PrunePureLiteralStatements_test.cpp:
//   Tests for pruning literal statements.
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

namespace
{

class PrunePureLiteralStatementsTest : public MatchOutputCodeTest
{
  public:
    // The PrunePureLiteralStatements pass is used when outputting ESSL
    PrunePureLiteralStatementsTest() : MatchOutputCodeTest(GL_FRAGMENT_SHADER, 0, SH_ESSL_OUTPUT) {}
};

// Most basic test for the pruning
TEST_F(PrunePureLiteralStatementsTest, FloatLiteralStatement)
{
    const std::string shaderString =
        "precision mediump float;\n"
        "void main()\n"
        "{\n"
        "   float f = 41.0;\n"
        "   42.0;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("41"));
    ASSERT_TRUE(notFoundInCode("42"));
}

// Test the pruning works for constructed types too
TEST_F(PrunePureLiteralStatementsTest, ConstructorLiteralStatement)
{
    const std::string shaderString =
        "precision mediump float;\n"
        "void main()\n"
        "{\n"
        "   vec2 f = vec2(41.0, 41.0);\n"
        "   vec2(42.0, 42.0);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("41"));
    ASSERT_TRUE(notFoundInCode("42"));
}

// Test the pruning works when the literal is a (non-tribial) expression
TEST_F(PrunePureLiteralStatementsTest, ExpressionLiteralStatement)
{
    const std::string shaderString =
        "precision mediump float;\n"
        "void main()\n"
        "{\n"
        "   vec2(21.0, 21.0) + vec2(21.0, 21.0);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(notFoundInCode("21"));
    ASSERT_TRUE(notFoundInCode("42"));
}

// Test that the pruning happens in the for-loop expression too
TEST_F(PrunePureLiteralStatementsTest, ForLoopLiteralExpression)
{
    const std::string shaderString =
        "precision mediump float;\n"
        "void main()\n"
        "{\n"
        "    for (;; vec2(42.0, 42.0)) {}\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(notFoundInCode("42"));
}

}  // namespace
