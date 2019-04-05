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

bool StartsWithASCII(const std::string &str, const std::string &search, bool caseSensitive)
{
    ASSERT(!caseSensitive);
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
bool NamesMatching(const std::string &ref, const std::string &testName)
{
    size_t len = ref.length();
    if (len == 0)
        return false;
    if (ref[len - 1] == '*')
    {
        if (testName.length() > len - 1 && ref.compare(0, len - 1, testName, 0, len - 1) == 0)
            return true;
        return false;
    }
    return (ref == testName);
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

bool GPUTestExpectationsParser::loadTestExpectations(const GPUTestConfig &config,
                                                     const std::string &data)
{
    mEntries.clear();
    mErrorMessages.clear();

    std::vector<std::string> lines = SplitString(data, "\n", TRIM_WHITESPACE, SPLIT_WANT_ALL);
    bool rt                        = true;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (!parseLine(config, lines[i], i + 1))
            rt = false;
    }
    if (detectConflictsBetweenEntries())
    {
        mEntries.clear();
        rt = false;
    }

    return rt;
}

bool GPUTestExpectationsParser::loadTestExpectationsFromFile(const GPUTestConfig &config,
                                                             const std::string &path)
{
    mEntries.clear();
    mErrorMessages.clear();

    std::string data;
    if (!ReadFileToString(path, &data))
    {
        mErrorMessages.push_back(kErrorMessage[kErrorFileIO]);
        return false;
    }
    return loadTestExpectations(config, data);
}

int32_t GPUTestExpectationsParser::getTestExpectation(const std::string &testName) const
{
    for (size_t i = 0; i < mEntries.size(); ++i)
    {
        if (NamesMatching(mEntries[i].mTestName, testName))
            return mEntries[i].mTestExpectation;
    }
    return kGpuTestPass;
}

const std::vector<std::string> &GPUTestExpectationsParser::getErrorMessages() const
{
    return mErrorMessages;
}

bool GPUTestExpectationsParser::parseLine(const GPUTestConfig &config,
                                          const std::string &lineData,
                                          size_t lineNumber)
{
    std::vector<std::string> tokens =
        SplitString(lineData, kWhitespaceASCII, KEEP_WHITESPACE, SPLIT_WANT_NONEMPTY);
    int32_t stage = kLineParserBegin;
    GPUTestExpectationEntry entry;
    entry.mLineNumber = lineNumber;
    bool skipLine     = false;
    for (size_t i = 0; i < tokens.size() && !skipLine; ++i)
    {
        Token token = ParseToken(tokens[i]);
        switch (token)
        {
            case kTokenComment:
                skipLine = true;
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
                    pushErrorMessage(kErrorMessage[kErrorIllegalEntry], lineNumber);
                    return false;
                }
                if (!checkTokenCondition(config, token, lineNumber))
                {
                    skipLine = true;  // Move to the next line without adding this one.
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
                    pushErrorMessage(kErrorMessage[kErrorIllegalEntry], lineNumber);
                    return false;
                }
                stage++;
                break;
            case kSeparatorEqual:
                // =
                if (stage != kLineParserTestName)
                {
                    pushErrorMessage(kErrorMessage[kErrorIllegalEntry], lineNumber);
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
                    entry.mTestName = tokens[i];
                }
                else
                {
                    pushErrorMessage(kErrorMessage[kErrorIllegalEntry], lineNumber);
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
                    pushErrorMessage(kErrorMessage[kErrorIllegalEntry], lineNumber);
                    return false;
                }
                if (entry.mTestExpectation != 0)
                {
                    pushErrorMessage(kErrorMessage[kErrorEntryWithExpectationConflicts],
                                     lineNumber);
                    return false;
                }
                entry.mTestExpectation = kTokenData[token].expectation;
                if (stage == kLineParserEqual)
                    stage++;
                break;
            default:
                ASSERT(false);
                break;
        }
    }
    if (stage == kLineParserBegin || skipLine)
    {
        // The whole line is empty or all comments
        return true;
    }
    if (stage == kLineParserExpectations)
    {
        mEntries.push_back(entry);
        return true;
    }
    pushErrorMessage(kErrorMessage[kErrorIllegalEntry], lineNumber);
    return false;
}

bool GPUTestExpectationsParser::checkTokenCondition(const GPUTestConfig &config,
                                                    int32_t token,
                                                    size_t lineNumber)
{
    if (token >= kNumberOfTokens)
    {
        pushErrorMessage(kErrorMessage[kErrorIllegalEntry], lineNumber);
        return false;
    }

    if (kTokenData[token].condition == GPUTestConfig::kConditionNone ||
        kTokenData[token].condition >= GPUTestConfig::kNumberOfConditions)
    {
        pushErrorMessage(kErrorMessage[kErrorInvalidEntry], lineNumber);
        // pass any unsupported conditions by default
        return true;
    }

    return config.getConditions()[kTokenData[token].condition];
}

bool GPUTestExpectationsParser::detectConflictsBetweenEntries()
{
    bool rt = false;
    for (size_t i = 0; i < mEntries.size(); ++i)
    {
        for (size_t j = i + 1; j < mEntries.size(); ++j)
        {
            if (mEntries[i].mTestName == mEntries[j].mTestName)
            {
                pushErrorMessage(kErrorMessage[kErrorEntriesOverlap], mEntries[i].mLineNumber,
                                 mEntries[j].mLineNumber);
                rt = true;
            }
        }
    }
    return rt;
}

void GPUTestExpectationsParser::pushErrorMessage(const std::string &message, size_t lineNumber)
{
    mErrorMessages.push_back("Line " + ToString(lineNumber) + " : " + message.c_str());
}

void GPUTestExpectationsParser::pushErrorMessage(const std::string &message,
                                                 size_t entry1LineNumber,
                                                 size_t entry2LineNumber)
{
    mErrorMessages.push_back("Line " + ToString(entry1LineNumber) + " and " +
                             ToString(entry2LineNumber) + " : " + message.c_str());
}

GPUTestExpectationsParser::GPUTestExpectationEntry::GPUTestExpectationEntry()
    : mTestExpectation(0), mLineNumber(0)
{}

}  // namespace angle
