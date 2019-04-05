//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// GPUTestExpectationsTest.cpp : Tests of the test_expectations library.

#include "test_expectations/GPUTestConfig.h"
#include "test_expectations/GPUTestExpectationsParser.h"
#include "test_utils/ANGLETest.h"

namespace angle
{

class GPUTestExpectationsTest : public ANGLETest
{
  protected:
    GPUTestExpectationsTest() {}

    // todo(jonahr): Eventually could add support for all conditions/operating
    // systems, but these are the ones in use for now
    void validateConfigBase(const GPUTestConfig &config)
    {
        if (IsWindows())
        {
            EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionWin]);
        }
        else
        {
            EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionWin]);
        }
        if (IsOSX())
        {
            EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionMac]);
        }
        else
        {
            EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionMac]);
        }
        if (IsLinux())
        {
            EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionLinux]);
        }
        else
        {
            EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionLinux]);
        }
        if (IsAndroid())
        {
            EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionAndroid]);
        }
        else
        {
            EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionAndroid]);
        }
        if (IsNexus5X())
        {
            EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionNexus5X]);
        }
        else
        {
            EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionNexus5X]);
        }
        if (IsPixel2())
        {
            EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionPixel2]);
        }
        else
        {
            EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionPixel2]);
        }
        if (IsIntel())
        {
            EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionIntel]);
        }
        else
        {
            EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionIntel]);
        }
        if (IsAMD())
        {
            EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionAMD]);
        }
        else
        {
            EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionAMD]);
        }
        if (IsNVIDIA())
        {
            EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionNVIDIA]);
        }
        else
        {
            EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionNVIDIA]);
        }
        if (IsDebug())
        {
            EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionDebug]);
        }
        else
        {
            EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionDebug]);
        }
        if (IsRelease())
        {
            EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionRelease]);
        }
        else
        {
            EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionRelease]);
        }
    }

    void validateConfigAPI(const GPUTestConfig &config, const GPUTestConfig::API &api)
    {
        switch (api)
        {
            case GPUTestConfig::kAPID3D9:
                EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionD3D9]);
                break;
            case GPUTestConfig::kAPID3D11:
                EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionD3D11]);
                break;
            case GPUTestConfig::kAPIGLDesktop:
                EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionGLDesktop]);
                break;
            case GPUTestConfig::kAPIGLES:
                EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionGLES]);
                break;
            case GPUTestConfig::kAPIVulkan:
                EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionVulkan]);
                break;
            case GPUTestConfig::kAPIUnknown:
            default:
                break;
        }
    }

    GPUTestConfig::API getAPI()
    {
        if (IsD3D9())
        {
            return GPUTestConfig::kAPID3D9;
        }
        if (IsD3D11())
        {
            return GPUTestConfig::kAPID3D11;
        }
        if (IsDesktopOpenGL())
        {
            return GPUTestConfig::kAPIGLDesktop;
        }
        if (IsOpenGLES())
        {
            return GPUTestConfig::kAPIGLES;
        }
        if (IsVulkan())
        {
            return GPUTestConfig::kAPIVulkan;
        }
        return GPUTestConfig::kAPIUnknown;
    }
};

TEST_P(GPUTestExpectationsTest, GPUTestConfigConditions)
{
    GPUTestConfig config;
    validateConfigBase(config);
}

TEST_P(GPUTestExpectationsTest, GPUTestConfigConditions_D3D9)
{
    GPUTestConfig config(GPUTestConfig::kAPID3D9);
    validateConfigAPI(config, GPUTestConfig::kAPID3D9);
}

TEST_P(GPUTestExpectationsTest, GPUTestConfigConditions_D3D11)
{
    GPUTestConfig config(GPUTestConfig::kAPID3D11);
    validateConfigAPI(config, GPUTestConfig::kAPID3D11);
}

TEST_P(GPUTestExpectationsTest, GPUTestConfigConditions_GLDesktop)
{
    GPUTestConfig config(GPUTestConfig::kAPIGLDesktop);
    validateConfigAPI(config, GPUTestConfig::kAPIGLDesktop);
}

TEST_P(GPUTestExpectationsTest, GPUTestConfigConditions_GLES)
{
    GPUTestConfig config(GPUTestConfig::kAPIGLES);
    validateConfigAPI(config, GPUTestConfig::kAPIGLES);
}

TEST_P(GPUTestExpectationsTest, GPUTestConfigConditions_Vulkan)
{
    GPUTestConfig config(GPUTestConfig::kAPIVulkan);
    validateConfigAPI(config, GPUTestConfig::kAPIVulkan);
}

TEST_P(GPUTestExpectationsTest, GPUTestExpectationsParserSkip)
{
    GPUTestConfig config(getAPI());
    std::string line =
        R"(100 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    GPUTestExpectationsParser parser;
    EXPECT_TRUE(parser.loadTestExpectations(config, line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(
        parser.getTestExpectation("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
        GPUTestExpectationsParser::kGpuTestSkip);
}

TEST_P(GPUTestExpectationsTest, GPUTestExpectationsParserFail)
{
    GPUTestConfig config(getAPI());
    std::string line =
        R"(100 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = FAIL)";
    GPUTestExpectationsParser parser;
    EXPECT_TRUE(parser.loadTestExpectations(config, line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(
        parser.getTestExpectation("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
        GPUTestExpectationsParser::kGpuTestFail);
}

TEST_P(GPUTestExpectationsTest, GPUTestExpectationsParserPass)
{
    GPUTestConfig config(getAPI());
    std::string line =
        R"(100 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = PASS)";
    GPUTestExpectationsParser parser;
    EXPECT_TRUE(parser.loadTestExpectations(config, line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(
        parser.getTestExpectation("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
        GPUTestExpectationsParser::kGpuTestPass);
}

TEST_P(GPUTestExpectationsTest, GPUTestExpectationsParserTimeout)
{
    GPUTestConfig config(getAPI());
    std::string line =
        R"(100 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = TIMEOUT)";
    GPUTestExpectationsParser parser;
    EXPECT_TRUE(parser.loadTestExpectations(config, line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(
        parser.getTestExpectation("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
        GPUTestExpectationsParser::kGpuTestTimeout);
}

TEST_P(GPUTestExpectationsTest, GPUTestExpectationsParserFlaky)
{
    GPUTestConfig config(getAPI());
    std::string line =
        R"(100 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = FLAKY)";
    GPUTestExpectationsParser parser;
    EXPECT_TRUE(parser.loadTestExpectations(config, line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(
        parser.getTestExpectation("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
        GPUTestExpectationsParser::kGpuTestFlaky);
}

TEST_P(GPUTestExpectationsTest, GPUTestExpectationsParserSingleLineWin)
{
    GPUTestConfig config(getAPI());
    std::string line =
        R"(100 WIN : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    GPUTestExpectationsParser parser;
    EXPECT_TRUE(parser.loadTestExpectations(config, line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    if (IsWindows())
    {
        EXPECT_EQ(parser.getTestExpectation(
                      "dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
                  GPUTestExpectationsParser::kGpuTestSkip);
    }
    else
    {
        EXPECT_EQ(parser.getTestExpectation(
                      "dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
                  GPUTestExpectationsParser::kGpuTestPass);
    }
}

TEST_P(GPUTestExpectationsTest, GPUTestExpectationsParserSingleLineWinNVIDIA)
{
    GPUTestConfig config(getAPI());
    std::string line =
        R"(100 WIN NVIDIA : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    GPUTestExpectationsParser parser;
    EXPECT_TRUE(parser.loadTestExpectations(config, line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    if (IsWindows() && IsNVIDIA())
    {
        EXPECT_EQ(parser.getTestExpectation(
                      "dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
                  GPUTestExpectationsParser::kGpuTestSkip);
    }
    else
    {
        EXPECT_EQ(parser.getTestExpectation(
                      "dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
                  GPUTestExpectationsParser::kGpuTestPass);
    }
}

TEST_P(GPUTestExpectationsTest, GPUTestExpectationsParserSingleLineWinNVIDIAD3D11)
{
    GPUTestConfig config(getAPI());
    std::string line =
        R"(100 WIN NVIDIA D3D11 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    GPUTestExpectationsParser parser;
    EXPECT_TRUE(parser.loadTestExpectations(config, line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    if (IsWindows() && IsNVIDIA() && IsD3D11())
    {
        EXPECT_EQ(parser.getTestExpectation(
                      "dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
                  GPUTestExpectationsParser::kGpuTestSkip);
    }
    else
    {
        EXPECT_EQ(parser.getTestExpectation(
                      "dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
                  GPUTestExpectationsParser::kGpuTestPass);
    }
}

TEST_P(GPUTestExpectationsTest, GPUTestExpectationsParserSingleLineWinNVIDIAD3D11OtherOrder)
{
    GPUTestConfig config(getAPI());
    std::string line =
        R"(100 D3D11 NVIDIA WIN : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    GPUTestExpectationsParser parser;
    EXPECT_TRUE(parser.loadTestExpectations(config, line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    if (IsWindows() && IsNVIDIA() && IsD3D11())
    {
        EXPECT_EQ(parser.getTestExpectation(
                      "dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
                  GPUTestExpectationsParser::kGpuTestSkip);
    }
    else
    {
        EXPECT_EQ(parser.getTestExpectation(
                      "dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
                  GPUTestExpectationsParser::kGpuTestPass);
    }
}

TEST_P(GPUTestExpectationsTest, GPUTestExpectationsParserMissingBugId)
{
    GPUTestConfig config(getAPI());
    std::string line = R"( : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    GPUTestExpectationsParser parser;
    EXPECT_FALSE(parser.loadTestExpectations(config, line));
    EXPECT_EQ(parser.getErrorMessages().size(), 1u);
    if (parser.getErrorMessages().size() >= 1)
        EXPECT_EQ(parser.getErrorMessages()[0], "Line 1 : entry with wrong format");
    // Default behavior is to let missing tests pass
    EXPECT_EQ(
        parser.getTestExpectation("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
        GPUTestExpectationsParser::kGpuTestPass);
}

TEST_P(GPUTestExpectationsTest, GPUTestExpectationsParserMissingBugIdWithConditions)
{
    GPUTestConfig config(getAPI());
    std::string line =
        R"(WIN D3D11 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    GPUTestExpectationsParser parser;
    EXPECT_FALSE(parser.loadTestExpectations(config, line));
    EXPECT_EQ(parser.getErrorMessages().size(), 1u);
    if (parser.getErrorMessages().size() >= 1)
        EXPECT_EQ(parser.getErrorMessages()[0], "Line 1 : entry with wrong format");
    // Default behavior is to let missing tests pass
    EXPECT_EQ(
        parser.getTestExpectation("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
        GPUTestExpectationsParser::kGpuTestPass);
}

TEST_P(GPUTestExpectationsTest, GPUTestExpectationsParserMissingColon)
{
    GPUTestConfig config(getAPI());
    std::string line = R"(100 dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    GPUTestExpectationsParser parser;
    EXPECT_FALSE(parser.loadTestExpectations(config, line));
    EXPECT_EQ(parser.getErrorMessages().size(), 1u);
    if (parser.getErrorMessages().size() >= 1)
        EXPECT_EQ(parser.getErrorMessages()[0], "Line 1 : entry with wrong format");
    // Default behavior is to let missing tests pass
    EXPECT_EQ(
        parser.getTestExpectation("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
        GPUTestExpectationsParser::kGpuTestPass);
}

TEST_P(GPUTestExpectationsTest, GPUTestExpectationsParserWildChar)
{
    GPUTestConfig config(getAPI());
    std::string line = R"(100 : dEQP-GLES31.functional.layout_binding.ubo.* = SKIP)";
    GPUTestExpectationsParser parser;
    EXPECT_TRUE(parser.loadTestExpectations(config, line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(
        parser.getTestExpectation("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
        GPUTestExpectationsParser::kGpuTestSkip);
    // Also ensure the wild char is not too wild, only covers tests that are more specific
    EXPECT_EQ(parser.getTestExpectation(
                  "dEQP-GLES31.functional.program_interface_query.transform_feedback_varying."
                  "resource_list.vertex_fragment.builtin_gl_position"),
              GPUTestExpectationsParser::kGpuTestPass);
}

TEST_P(GPUTestExpectationsTest, GPUTestExpectationsParserMissingEquals)
{
    GPUTestConfig config(getAPI());
    std::string line = R"(100 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max SKIP)";
    GPUTestExpectationsParser parser;
    EXPECT_FALSE(parser.loadTestExpectations(config, line));
    EXPECT_EQ(parser.getErrorMessages().size(), 1u);
    if (parser.getErrorMessages().size() >= 1)
        EXPECT_EQ(parser.getErrorMessages()[0], "Line 1 : entry with wrong format");
    // Default behavior is to let missing tests pass
    EXPECT_EQ(
        parser.getTestExpectation("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
        GPUTestExpectationsParser::kGpuTestPass);
}

TEST_P(GPUTestExpectationsTest, GPUTestExpectationsParserMissingExpectation)
{
    GPUTestConfig config(getAPI());
    std::string line = R"(100 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max =)";
    GPUTestExpectationsParser parser;
    EXPECT_FALSE(parser.loadTestExpectations(config, line));
    EXPECT_EQ(parser.getErrorMessages().size(), 1u);
    if (parser.getErrorMessages().size() >= 1)
        EXPECT_EQ(parser.getErrorMessages()[0], "Line 1 : entry with wrong format");
    // Default behavior is to let missing tests pass
    EXPECT_EQ(
        parser.getTestExpectation("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
        GPUTestExpectationsParser::kGpuTestPass);
}

TEST_P(GPUTestExpectationsTest, GPUTestExpectationsParserInvalidExpectation)
{
    GPUTestConfig config(getAPI());
    std::string line =
        R"(100 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = WRONG)";
    GPUTestExpectationsParser parser;
    EXPECT_FALSE(parser.loadTestExpectations(config, line));
    EXPECT_EQ(parser.getErrorMessages().size(), 1u);
    if (parser.getErrorMessages().size() >= 1)
        EXPECT_EQ(parser.getErrorMessages()[0], "Line 1 : entry with wrong format");
    // Default behavior is to let missing tests pass
    EXPECT_EQ(
        parser.getTestExpectation("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
        GPUTestExpectationsParser::kGpuTestPass);
}

// (https://anglebug.com/3363) Remove/update this test when ChromeOS is supported
TEST_P(GPUTestExpectationsTest, GPUTestExpectationsParserUnimplementedCondition)
{
    GPUTestConfig config(getAPI());
    std::string line =
        R"(100 CHROMEOS : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    GPUTestExpectationsParser parser;
    EXPECT_FALSE(parser.loadTestExpectations(config, line));
    EXPECT_EQ(parser.getErrorMessages().size(), 1u);
    if (parser.getErrorMessages().size() >= 1)
        EXPECT_EQ(parser.getErrorMessages()[0],
                  "Line 1 : entry invalid, likely unimplemented modifiers");
    // Default behavior is to let missing tests pass
    EXPECT_EQ(
        parser.getTestExpectation("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
        GPUTestExpectationsParser::kGpuTestPass);
}

TEST_P(GPUTestExpectationsTest, GPUTestExpectationsParserComment)
{
    GPUTestConfig config(getAPI());
    std::string line =
        R"(//100 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    GPUTestExpectationsParser parser;
    EXPECT_TRUE(parser.loadTestExpectations(config, line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(
        parser.getTestExpectation("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
        GPUTestExpectationsParser::kGpuTestPass);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(GPUTestExpectationsTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_D3D11_FL9_3(),
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES2_OPENGLES(),
                       ES3_OPENGLES(),
                       ES2_VULKAN());

}  // namespace angle
