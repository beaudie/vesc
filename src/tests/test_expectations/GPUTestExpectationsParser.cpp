//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "GPUTestExpectationsParser.h"

#include <stddef.h>
#include <stdint.h>

#include "common/angleutils.h"
#include "common/debug.h"
#include "common/string_utils.h"

#include "GPUTestConfig.h"

namespace base
{

namespace
{

bool StartsWithASCII(const std::string &str, const std::string &search, bool case_sensitive)
{
    ASSERT(!case_sensitive);
    return str.compare(0, search.length(), search) == 0;
}

template <class Char>
inline Char ToLowerASCII(Char c)
{
    return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
}

template <typename Iter>
static inline bool DoLowerCaseEqualsASCII(Iter a_begin, Iter a_end, const char *b)
{
    for (Iter it = a_begin; it != a_end; ++it, ++b)
    {
        if (!*b || base::ToLowerASCII(*it) != *b)
            return false;
    }
    return *b == 0;
}

bool LowerCaseEqualsASCII(const std::string &a, const char *b)
{
    return DoLowerCaseEqualsASCII(a.begin(), a.end(), b);
}

}  // anonymous namespace

}  // namespace base

namespace angle
{

namespace
{

enum LineParserStage
{
    kLineParserBegin = 0,
    kLineParserBugID,
    kLineParserConfigs,
    kLineParserColon,
    kLineParserTestName,
    kLineParserEqual,
    kLineParserExpectations,
};

enum Token
{
    // os
    kConfigWinXP = 0,
    kConfigWinVista,
    kConfigWin7,
    kConfigWin8,
    kConfigWin10,
    kConfigWin,
    kConfigMacLeopard,
    kConfigMacSnowLeopard,
    kConfigMacLion,
    kConfigMacMountainLion,
    kConfigMacMavericks,
    kConfigMacYosemite,
    kConfigMacElCapitan,
    kConfigMacSierra,
    kConfigMacHighSierra,
    kConfigMacMojave,
    kConfigMac,
    kConfigLinux,
    kConfigChromeOS,
    kConfigAndroid,
    // gpu vendor
    kConfigNVIDIA,
    kConfigAMD,
    kConfigIntel,
    kConfigVMWare,
    // build type
    kConfigRelease,
    kConfigDebug,
    // ANGLE renderer
    kConfigD3D9,
    kConfigD3D11,
    kConfigGLDesktop,
    kConfigGLES,
    kConfigVulkan,
    // Android devices
    kConfigNexus5X,
    kConfigPixel2,
    // GPU devices
    kConfigNVIDIAQuadroP400,
    // expectation
    kExpectationPass,
    kExpectationFail,
    kExpectationFlaky,
    kExpectationTimeout,
    kExpectationSkip,
    // separator
    kSeparatorColon,
    kSeparatorEqual,

    kNumberOfExactMatchTokens,

    // others
    kTokenComment,
    kTokenWord,

    kNumberOfTokens,
};

struct TokenInfo
{
    const char *name;
    GPUTestConfig::Condition condition;
    GPUTestExpectationsParser::GPUTestExpectation expectation;
};

const TokenInfo kTokenData[kNumberOfTokens] = {
    [kConfigWinXP]          = {.name = "xp", .condition = GPUTestConfig::kConditionWinXP},
    [kConfigWinVista]       = {.name = "vista", .condition = GPUTestConfig::kConditionWinVista},
    [kConfigWin7]           = {.name = "win7", .condition = GPUTestConfig::kConditionWin7},
    [kConfigWin8]           = {.name = "win8", .condition = GPUTestConfig::kConditionWin8},
    [kConfigWin10]          = {.name = "win10", .condition = GPUTestConfig::kConditionWin10},
    [kConfigWin]            = {.name = "win", .condition = GPUTestConfig::kConditionWin},
    [kConfigMacLeopard]     = {.name = "leopard", .condition = GPUTestConfig::kConditionMacLeopard},
    [kConfigMacSnowLeopard] = {.name      = "snowleopard",
                               .condition = GPUTestConfig::kConditionMacSnowLeopard},
    [kConfigMacLion]        = {.name = "lion", .condition = GPUTestConfig::kConditionMacLion},
    [kConfigMacMountainLion] = {.name      = "mountainlion",
                                .condition = GPUTestConfig::kConditionMacMountainLion},
    [kConfigMacMavericks]    = {.name      = "mavericks",
                             .condition = GPUTestConfig::kConditionMacMavericks},
    [kConfigMacYosemite]  = {.name = "yosemite", .condition = GPUTestConfig::kConditionMacYosemite},
    [kConfigMacElCapitan] = {.name      = "elcapitan",
                             .condition = GPUTestConfig::kConditionMacElCapitan},
    [kConfigMacSierra]    = {.name = "sierra", .condition = GPUTestConfig::kConditionMacSierra},
    [kConfigMacHighSierra] = {.name      = "highsierra",
                              .condition = GPUTestConfig::kConditionMacHighSierra},
    [kConfigMacMojave]     = {.name = "mojave", .condition = GPUTestConfig::kConditionMacMojave},
    [kConfigMac]           = {.name = "mac", .condition = GPUTestConfig::kConditionMac},
    [kConfigLinux]         = {.name = "linux", .condition = GPUTestConfig::kConditionLinux},
    // ChromeOS not supported yet
    [kConfigChromeOS]         = {.name = "chromeos"},
    [kConfigAndroid]          = {.name = "android", .condition = GPUTestConfig::kConditionAndroid},
    [kConfigNVIDIA]           = {.name = "nvidia", .condition = GPUTestConfig::kConditionNVIDIA},
    [kConfigAMD]              = {.name = "amd", .condition = GPUTestConfig::kConditionAMD},
    [kConfigIntel]            = {.name = "intel", .condition = GPUTestConfig::kConditionIntel},
    [kConfigVMWare]           = {.name = "vmware", .condition = GPUTestConfig::kConditionVMWare},
    [kConfigRelease]          = {.name = "release", .condition = GPUTestConfig::kConditionRelease},
    [kConfigDebug]            = {.name = "debug", .condition = GPUTestConfig::kConditionDebug},
    [kConfigD3D9]             = {.name = "d3d9", .condition = GPUTestConfig::kConditionD3D9},
    [kConfigD3D11]            = {.name = "d3d11", .condition = GPUTestConfig::kConditionD3D11},
    [kConfigGLDesktop]        = {.name = "opengl", .condition = GPUTestConfig::kConditionGLDesktop},
    [kConfigGLES]             = {.name = "gles", .condition = GPUTestConfig::kConditionGLES},
    [kConfigVulkan]           = {.name = "vulkan", .condition = GPUTestConfig::kConditionVulkan},
    [kConfigNexus5X]          = {.name = "nexus5x", .condition = GPUTestConfig::kConditionNexus5X},
    [kConfigPixel2]           = {.name = "pixel2", .condition = GPUTestConfig::kConditionPixel2},
    [kConfigNVIDIAQuadroP400] = {.name      = "quadrop400",
                                 .condition = GPUTestConfig::kConditionNVIDIAQuadroP400},
    [kExpectationPass]  = {.name = "pass", .expectation = GPUTestExpectationsParser::kGpuTestPass},
    [kExpectationFail]  = {.name = "fail", .expectation = GPUTestExpectationsParser::kGpuTestFail},
    [kExpectationFlaky] = {.name        = "flaky",
                           .expectation = GPUTestExpectationsParser::kGpuTestFlaky},
    [kExpectationTimeout] = {.name        = "timeout",
                             .expectation = GPUTestExpectationsParser::kGpuTestTimeout},
    [kExpectationSkip] = {.name = "skip", .expectation = GPUTestExpectationsParser::kGpuTestSkip},
    [kSeparatorColon]  = {.name = ":"},
    [kSeparatorEqual]  = {.name = "="},
    [kNumberOfExactMatchTokens] = {},
    [kTokenComment]             = {},
    [kTokenWord]                = {},
};

enum ErrorType
{
    kErrorFileIO = 0,
    kErrorIllegalEntry,
    kErrorInvalidEntry,
    kErrorEntryWithExpectationConflicts,
    kErrorEntriesOverlap,

    kNumberOfErrors,
};

const char *kErrorMessage[kNumberOfErrors] = {
    [kErrorFileIO]                        = "file IO failed",
    [kErrorIllegalEntry]                  = "entry with wrong format",
    [kErrorInvalidEntry]                  = "entry invalid, likely unimplemented modifiers",
    [kErrorEntryWithExpectationConflicts] = "entry with expectation modifier conflicts",
    [kErrorEntriesOverlap]                = "two entries' configs overlap",
};

Token ParseToken(const std::string &word)
{
    if (base::StartsWithASCII(word, "//", false))
        return kTokenComment;

    for (int32_t i = 0; i < kNumberOfExactMatchTokens; ++i)
    {
        if (base::LowerCaseEqualsASCII(word, kTokenData[i].name))
            return static_cast<Token>(i);
    }
    return kTokenWord;
}

// reference name can have the last character as *.
bool NamesMatching(const std::string &ref, const std::string &test_name)
{
    size_t len = ref.length();
    if (len == 0)
        return false;
    if (ref[len - 1] == '*')
    {
        if (test_name.length() > len - 1 && ref.compare(0, len - 1, test_name, 0, len - 1) == 0)
            return true;
        return false;
    }
    return (ref == test_name);
}

}  // namespace

GPUTestExpectationsParser::GPUTestExpectationsParser()
{
    // Some sanity check.
    ASSERT((static_cast<unsigned int>(kNumberOfTokens)) ==
           (sizeof(kTokenData) / sizeof(kTokenData[0])));
    ASSERT((static_cast<unsigned int>(kNumberOfErrors)) ==
           (sizeof(kErrorMessage) / sizeof(kErrorMessage[0])));
}

GPUTestExpectationsParser::~GPUTestExpectationsParser() = default;

bool GPUTestExpectationsParser::LoadTestExpectations(const GPUTestConfig &config,
                                                     const std::string &data)
{
    entries_.clear();
    error_messages_.clear();

    std::vector<std::string> lines = SplitString(data, "\n", TRIM_WHITESPACE, SPLIT_WANT_ALL);
    bool rt                        = true;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (!ParseLine(config, lines[i], i + 1))
            rt = false;
    }
    if (DetectConflictsBetweenEntries())
    {
        entries_.clear();
        rt = false;
    }

    return rt;
}

bool GPUTestExpectationsParser::LoadTestExpectationsFromFile(const GPUTestConfig &config,
                                                             const std::string &path)
{
    entries_.clear();
    error_messages_.clear();

    std::string data;
    if (!ReadFileToString(path, &data))
    {
        error_messages_.push_back(kErrorMessage[kErrorFileIO]);
        return false;
    }
    return LoadTestExpectations(config, data);
}

int32_t GPUTestExpectationsParser::GetTestExpectation(const std::string &test_name) const
{
    for (size_t i = 0; i < entries_.size(); ++i)
    {
        if (NamesMatching(entries_[i].test_name, test_name))
            return entries_[i].test_expectation;
    }
    return kGpuTestPass;
}

const std::vector<std::string> &GPUTestExpectationsParser::GetErrorMessages() const
{
    return error_messages_;
}

bool GPUTestExpectationsParser::ParseLine(const GPUTestConfig &config,
                                          const std::string &line_data,
                                          size_t line_number)
{
    std::vector<std::string> tokens =
        SplitString(line_data, kWhitespaceASCII, KEEP_WHITESPACE, SPLIT_WANT_NONEMPTY);
    int32_t stage = kLineParserBegin;
    GPUTestExpectationEntry entry;
    entry.line_number         = line_number;
    bool skip_line            = false;
    for (size_t i = 0; i < tokens.size() && !skip_line; ++i)
    {
        Token token = ParseToken(tokens[i]);
        switch (token)
        {
            case kTokenComment:
                skip_line = true;
                break;
            case kConfigWinXP:
            case kConfigWinVista:
            case kConfigWin7:
            case kConfigWin8:
            case kConfigWin10:
            case kConfigWin:
            case kConfigMacLeopard:
            case kConfigMacSnowLeopard:
            case kConfigMacLion:
            case kConfigMacMountainLion:
            case kConfigMacMavericks:
            case kConfigMacYosemite:
            case kConfigMacElCapitan:
            case kConfigMacSierra:
            case kConfigMacHighSierra:
            case kConfigMacMojave:
            case kConfigMac:
            case kConfigLinux:
            case kConfigChromeOS:
            case kConfigAndroid:
            case kConfigNVIDIA:
            case kConfigAMD:
            case kConfigIntel:
            case kConfigVMWare:
            case kConfigRelease:
            case kConfigDebug:
            case kConfigD3D9:
            case kConfigD3D11:
            case kConfigGLDesktop:
            case kConfigGLES:
            case kConfigVulkan:
            case kConfigNexus5X:
            case kConfigPixel2:
            case kConfigNVIDIAQuadroP400:
                // MODIFIERS, check each condition and add accordingly.
                if (stage != kLineParserConfigs && stage != kLineParserBugID)
                {
                    PushErrorMessage(kErrorMessage[kErrorIllegalEntry], line_number);
                    return false;
                }
                if (!CheckTokenCondition(config, token, line_number))
                {
                    skip_line = true;  // Move to the next line without adding this one.
                }

                if (stage == kLineParserBugID)
                {
                    stage++;
                }
                break;
            case kSeparatorColon:
                // :
                // If there are no modifiers, move straight to separator colon
                if (stage == kLineParserBugID)
                {
                    stage++;
                }
                if (stage != kLineParserConfigs)
                {
                    PushErrorMessage(kErrorMessage[kErrorIllegalEntry], line_number);
                    return false;
                }
                stage++;
                break;
            case kSeparatorEqual:
                // =
                if (stage != kLineParserTestName)
                {
                    PushErrorMessage(kErrorMessage[kErrorIllegalEntry], line_number);
                    return false;
                }
                stage++;
                break;
            case kTokenWord:
                // BUG_ID or TEST_NAME
                if (stage == kLineParserBegin)
                {
                    // Bug ID is not used for anything; ignore it.
                }
                else if (stage == kLineParserColon)
                {
                    entry.test_name = tokens[i];
                }
                else
                {
                    PushErrorMessage(kErrorMessage[kErrorIllegalEntry], line_number);
                    return false;
                }
                stage++;
                break;
            case kExpectationPass:
            case kExpectationFail:
            case kExpectationFlaky:
            case kExpectationTimeout:
            case kExpectationSkip:
                // TEST_EXPECTATIONS
                if (stage != kLineParserEqual && stage != kLineParserExpectations)
                {
                    PushErrorMessage(kErrorMessage[kErrorIllegalEntry], line_number);
                    return false;
                }
                if (entry.test_expectation != 0)
                {
                    PushErrorMessage(kErrorMessage[kErrorEntryWithExpectationConflicts],
                                     line_number);
                    return false;
                }
                entry.test_expectation = kTokenData[token].expectation;
                if (stage == kLineParserEqual)
                    stage++;
                break;
            default:
                ASSERT(false);
                break;
        }
    }
    if (stage == kLineParserBegin || skip_line)
    {
        // The whole line is empty or all comments
        return true;
    }
    if (stage == kLineParserExpectations)
    {
        entries_.push_back(entry);
        return true;
    }
    PushErrorMessage(kErrorMessage[kErrorIllegalEntry], line_number);
    return false;
}

bool GPUTestExpectationsParser::CheckTokenCondition(const GPUTestConfig &config,
                                                    int32_t token,
                                                    size_t line_number)
{
    if (token >= kNumberOfTokens)
    {
        PushErrorMessage(kErrorMessage[kErrorIllegalEntry], line_number);
        return false;
    }

    if (kTokenData[token].condition == GPUTestConfig::kConditionNone ||
        kTokenData[token].condition >= GPUTestConfig::kNumberOfConditions)
    {
        PushErrorMessage(kErrorMessage[kErrorInvalidEntry], line_number);
        // pass any unsupported conditions by default
        return true;
    }

    return config.GetConditions()[kTokenData[token].condition];
}

bool GPUTestExpectationsParser::DetectConflictsBetweenEntries()
{
    bool rt = false;
    for (size_t i = 0; i < entries_.size(); ++i)
    {
        for (size_t j = i + 1; j < entries_.size(); ++j)
        {
            if (entries_[i].test_name == entries_[j].test_name)
            {
                PushErrorMessage(kErrorMessage[kErrorEntriesOverlap], entries_[i].line_number,
                                 entries_[j].line_number);
                rt = true;
            }
        }
    }
    return rt;
}

void GPUTestExpectationsParser::PushErrorMessage(const std::string &message, size_t line_number)
{
    error_messages_.push_back("Line " + ToString(line_number) + " : " + message.c_str());
}

void GPUTestExpectationsParser::PushErrorMessage(const std::string &message,
                                                 size_t entry1_line_number,
                                                 size_t entry2_line_number)
{
    error_messages_.push_back("Line " + ToString(entry1_line_number) + " and " +
                              ToString(entry2_line_number) + " : " + message.c_str());
}

GPUTestExpectationsParser::GPUTestExpectationEntry::GPUTestExpectationEntry()
    : test_expectation(0), line_number(0)
{}

}  // namespace angle
