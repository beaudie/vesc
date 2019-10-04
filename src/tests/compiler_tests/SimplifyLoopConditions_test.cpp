//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SimplifyLoopConditions_test.cpp:
//   Tests for the SimplifyLoopConditions transform.
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

namespace
{

class SimplifyLoopConditionsTest : public MatchOutputCodeTest
{
  public:
    // The SimplifyLoopConditions pass is used when outputting ESSL
    SimplifyLoopConditionsTest() : MatchOutputCodeTest(GL_FRAGMENT_SHADER, 0, SH_ESSL_OUTPUT) {}
};

// For-loop with "continue" statement converts correctly to while-loop.
TEST_F(SimplifyLoopConditionsTest, ForWithContinueToWhile)
{
    const std::string shaderString =
        R"(precision mediump float;
        void main()
        {
            for (int k, p=0; p<9; p++) {
                k = p;
                if (p == 4) continue;
                gl_FragColor++;
            }
        })";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("for"));
    ASSERT_TRUE(notFoundInCode("while"));
}

}  // namespace
