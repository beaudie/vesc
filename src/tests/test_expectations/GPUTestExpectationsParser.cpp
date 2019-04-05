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

#include "GPUTestInfo.h"

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
    kConfigNVidia,
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
    kConfigNVidiaQuadroP400,
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

enum TokenFlag
{
    kFlagOther = 0,
    kFlagConfig,
    kFlagExpectation,
    kFlagSeparator,
};

struct TokenInfo
{
    const char *name;
    TokenFlag flag;
    bool (*checkConfigCondition)(void);
    GPUTestExpectationsParser::GPUTestExpectation expectation;
};

const TokenInfo kTokenData[kNumberOfTokens] = {
    [kConfigWinXP]    = {.name = "xp", .flag = kFlagConfig, .checkConfigCondition = IsWinXP},
    [kConfigWinVista] = {.name = "vista", .flag = kFlagConfig, .checkConfigCondition = IsWinVista},
    [kConfigWin7]     = {.name = "win7", .flag = kFlagConfig, .checkConfigCondition = IsWin7},
    [kConfigWin8]     = {.name = "win8", .flag = kFlagConfig, .checkConfigCondition = IsWin8},
    [kConfigWin10]    = {.name = "win10", .flag = kFlagConfig, .checkConfigCondition = IsWin10},
    [kConfigWin]      = {.name = "win", .flag = kFlagConfig, .checkConfigCondition = IsWin},
    [kConfigMacLeopard]     = {.name                 = "leopard",
                           .flag                 = kFlagConfig,
                           .checkConfigCondition = IsMacLeopard},
    [kConfigMacSnowLeopard] = {.name                 = "snowleopard",
                               .flag                 = kFlagConfig,
                               .checkConfigCondition = IsMacSnowLeopard},
    [kConfigMacLion] = {.name = "lion", .flag = kFlagConfig, .checkConfigCondition = IsMacLion},
    [kConfigMacMountainLion] = {.name                 = "mountainlion",
                                .flag                 = kFlagConfig,
                                .checkConfigCondition = IsMacMountainLion},
    [kConfigMacMavericks]    = {.name                 = "mavericks",
                             .flag                 = kFlagConfig,
                             .checkConfigCondition = IsMacMavericks},
    [kConfigMacYosemite]     = {.name                 = "yosemite",
                            .flag                 = kFlagConfig,
                            .checkConfigCondition = IsMacYosemite},
    [kConfigMacElCapitan]    = {.name                 = "elcapitan",
                             .flag                 = kFlagConfig,
                             .checkConfigCondition = IsMacElCapitan},
    [kConfigMacSierra]       = {.name                 = "sierra",
                          .flag                 = kFlagConfig,
                          .checkConfigCondition = IsMacSierra},
    [kConfigMacHighSierra]   = {.name                 = "highsierra",
                              .flag                 = kFlagConfig,
                              .checkConfigCondition = IsMacHighSierra},
    [kConfigMacMojave]       = {.name                 = "mojave",
                          .flag                 = kFlagConfig,
                          .checkConfigCondition = IsMacMojave},
    [kConfigMac]             = {.name = "mac", .flag = kFlagConfig, .checkConfigCondition = IsMac},
    [kConfigLinux] = {.name = "linux", .flag = kFlagConfig, .checkConfigCondition = IsLinux},
    // ChromeOS not supported yet
    [kConfigChromeOS] = {.name = "chromeos", .flag = kFlagConfig, .checkConfigCondition = nullptr},
    [kConfigAndroid]  = {.name = "android", .flag = kFlagConfig, .checkConfigCondition = IsAndroid},
    [kConfigNVidia]   = {.name = "nvidia", .flag = kFlagConfig, .checkConfigCondition = IsNVidia},
    [kConfigAMD]      = {.name = "amd", .flag = kFlagConfig, .checkConfigCondition = IsAMD},
    [kConfigIntel]    = {.name = "intel", .flag = kFlagConfig, .checkConfigCondition = IsIntel},
    [kConfigVMWare]   = {.name = "vmware", .flag = kFlagConfig, .checkConfigCondition = IsVMWare},
    [kConfigRelease]  = {.name = "release", .flag = kFlagConfig, .checkConfigCondition = IsRelease},
    [kConfigDebug]    = {.name = "debug", .flag = kFlagConfig, .checkConfigCondition = IsDebug},
    [kConfigD3D9]     = {.name = "d3d9", .flag = kFlagConfig, .checkConfigCondition = IsD3D9},
    [kConfigD3D11]    = {.name = "d3d11", .flag = kFlagConfig, .checkConfigCondition = IsD3D11},
    [kConfigGLDesktop] = {.name                 = "opengl",
                          .flag                 = kFlagConfig,
                          .checkConfigCondition = IsGLDesktop},
    [kConfigGLES]      = {.name = "gles", .flag = kFlagConfig, .checkConfigCondition = IsGLES},
    [kConfigVulkan]    = {.name = "vulkan", .flag = kFlagConfig, .checkConfigCondition = IsVulkan},
    [kConfigNexus5X] = {.name = "nexus5x", .flag = kFlagConfig, .checkConfigCondition = IsNexus5X},
    [kConfigPixel2]  = {.name = "pixel2", .flag = kFlagConfig, .checkConfigCondition = IsPixel2},
    [kConfigNVidiaQuadroP400]   = {.name                 = "quadrop400",
                                 .flag                 = kFlagConfig,
                                 .checkConfigCondition = IsNVidiaQuadroP400},
    [kExpectationPass]          = {.name        = "pass",
                          .flag        = kFlagExpectation,
                          .expectation = GPUTestExpectationsParser::kGpuTestPass},
    [kExpectationFail]          = {.name        = "fail",
                          .flag        = kFlagExpectation,
                          .expectation = GPUTestExpectationsParser::kGpuTestFail},
    [kExpectationFlaky]         = {.name        = "flaky",
                           .flag        = kFlagExpectation,
                           .expectation = GPUTestExpectationsParser::kGpuTestFlaky},
    [kExpectationTimeout]       = {.name        = "timeout",
                             .flag        = kFlagExpectation,
                             .expectation = GPUTestExpectationsParser::kGpuTestTimeout},
    [kExpectationSkip]          = {.name        = "skip",
                          .flag        = kFlagExpectation,
                          .expectation = GPUTestExpectationsParser::kGpuTestSkip},
    [kSeparatorColon]           = {.name = ":", .flag = kFlagSeparator},
    [kSeparatorEqual]           = {.name = "=", .flag = kFlagSeparator},
    [kNumberOfExactMatchTokens] = {.flag = kFlagOther},
    [kTokenComment]             = {.flag = kFlagOther},
    [kTokenWord]                = {.flag = kFlagOther},
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

bool GPUTestExpectationsParser::LoadTestExpectations(const std::string &data)
{
    entries_.clear();
    error_messages_.clear();

    std::vector<std::string> lines = SplitString(data, "\n", TRIM_WHITESPACE, SPLIT_WANT_ALL);
    bool rt                        = true;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (!ParseLine(lines[i], i + 1))
            rt = false;
    }
    if (DetectConflictsBetweenEntries())
    {
        entries_.clear();
        rt = false;
    }

    return rt;
}

bool GPUTestExpectationsParser::LoadTestExpectationsFromFile(const std::string &path)
{
    entries_.clear();
    error_messages_.clear();

    std::string data;
    if (!ReadFileToString(path, &data))
    {
        error_messages_.push_back(kErrorMessage[kErrorFileIO]);
        return false;
    }
    return LoadTestExpectations(data);
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

bool GPUTestExpectationsParser::ParseLine(const std::string &line_data, size_t line_number)
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
            case kConfigNVidia:
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
                // MODIFIERS, check each condition and add accordingly.
                if (stage != kLineParserConfigs && stage != kLineParserBugID)
                {
                    PushErrorMessage(kErrorMessage[kErrorIllegalEntry], line_number);
                    return false;
                }
                if (!CheckTokenCondition(token, line_number))
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
                if ((kTokenData[token].flag & entry.test_expectation) != 0)
                {
                    PushErrorMessage(kErrorMessage[kErrorEntryWithExpectationConflicts],
                                     line_number);
                    return false;
                }
                entry.test_expectation = (kTokenData[token].expectation | entry.test_expectation);
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

bool GPUTestExpectationsParser::CheckTokenCondition(int32_t token, size_t line_number)
{
    if (token >= kNumberOfTokens)
    {
        PushErrorMessage(kErrorMessage[kErrorIllegalEntry], line_number);
        return false;
    }

    switch (kTokenData[token].flag)
    {
        case kFlagConfig:
            if (kTokenData[token].checkConfigCondition != nullptr)
            {
                return kTokenData[token].checkConfigCondition();
            }
            else
            {
                // This condition does not have an implementation
                PushErrorMessage(kErrorMessage[kErrorInvalidEntry], line_number);
                return false;
            }
            break;

        case kFlagOther:
        case kFlagExpectation:
        case kFlagSeparator:
        default:
            break;
    }
    return false;
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
