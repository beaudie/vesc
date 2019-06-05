//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// InvalidVariables_test.cpp:
//   Tests that certain internally-used variables are not defined in
//   input shaders.
//

#include <sstream>
#include "../test_utils/compiler_test.h"

using namespace sh;

class InvalidVariablesTest : public MatchOutputCodeTest
{
  public:
    InvalidVariablesTest()
        : MatchOutputCodeTest(GL_VERTEX_SHADER, SH_VALIDATE, SH_GLSL_VULKAN_OUTPUT)
    {}

  protected:
    void CheckVariableNotDefined(const std::string &variableName, const std::string &variableType)
    {
        std::ostringstream shaderStream;
        shaderStream << "#version 310 es\n"
                     << "out " << variableType << " myVar;\n"
                     << "void main() {\n"
                     << "    myVar = " << variableName << ";\n"
                     << "}\n";
        std::string translatedCode;
        std::string infoLog;
        bool success = compileTestShader(GL_VERTEX_SHADER, SH_GLES3_1_SPEC, SH_GLSL_VULKAN_OUTPUT,
                                         shaderStream.str(), getResources(), SH_VALIDATE,
                                         &translatedCode, &infoLog);
        EXPECT_FALSE(success);
        EXPECT_TRUE(infoLog.find(variableName) != std::string::npos);
        EXPECT_TRUE(infoLog.find("undeclared identifier") != std::string::npos);
    }
};

// Test that gl_InstanceIndex is not defined.
TEST_F(InvalidVariablesTest, InstanceIndexInvalid)
{
    CheckVariableNotDefined("gl_InstanceIndex", "uint");
}

// Test that gl_VertexIndex is not defined.
TEST_F(InvalidVariablesTest, VertexIndexInvalid)
{
    CheckVariableNotDefined("gl_VertexIndex", "uint");
}
