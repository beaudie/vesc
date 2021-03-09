// Copyright (c) 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "native_test_util.h"

#include <fstream>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/strings/string_tokenizer.h"
#include "base/strings/string_util.h"

namespace angle
{
const char kWhitespaceASCII[] = " \f\n\r\t\v";

namespace
{

bool ReadFileToString(const std::string &path, std::string *stringOut)
{
    std::ifstream inFile(path.c_str());
    if (inFile.fail())
    {
        return false;
    }

    inFile.seekg(0, std::ios::end);
    stringOut->reserve(static_cast<std::string::size_type>(inFile.tellg()));
    inFile.seekg(0, std::ios::beg);

    stringOut->assign(std::istreambuf_iterator<char>(inFile), std::istreambuf_iterator<char>());
    return !inFile.fail();
}

std::string RemoveDoubleQuotes(const std::string &input)
{
    if (input.find('"') == std::string::npos)
        return input;

    std::string out  = input;
    size_t out_index = 0;
    for (size_t in_index = 0; in_index < out.length(); ++in_index)
    {
        if (out[in_index] == '"')
            continue;

        out[out_index] = out[in_index];
        out_index++;
    }
    out.resize(out_index);
    return out;
}

}  // anonymous namespace

void ParseArgsFromString(const std::string &command_line, std::vector<std::string> *args)
{
    size_t start_token_index = 0;
    while (start_token_index < command_line.length())
    {
        size_t end_token_index    = command_line.find_first_of(kWhitespaceASCII, start_token_index);
        size_t double_quote_index = command_line.find('"', start_token_index);
        if (double_quote_index < end_token_index)
        {
            double_quote_index = command_line.find('"', double_quote_index + 1);
            end_token_index =
                (double_quote_index != std::string::npos)
                    ? command_line.find_first_of(kWhitespaceASCII, double_quote_index + 1)
                    : std::string::npos;
        }
        if (end_token_index == std::string::npos)
        {
            end_token_index = command_line.length();
        }
        std::string token =
            command_line.substr(start_token_index, end_token_index - start_token_index);
        if (!token.empty())
        {
            args->push_back(RemoveDoubleQuotes(token));
        }
        start_token_index = end_token_index + 1;
    }
}

void ParseArgsFromCommandLineFile(const std::string &path, std::vector<std::string> *args)
{
    std::string command_line_string;
    if (ReadFileToString(path, &command_line_string))
    {
        ParseArgsFromString(command_line_string, args);
    }
}

int ArgsToArgv(const std::vector<std::string> &args, std::vector<char *> *argv)
{
    // We need to pass in a non-const char**.
    int argc = args.size();

    argv->resize(argc + 1);
    for (int i = 0; i < argc; ++i)
    {
        (*argv)[i] = const_cast<char *>(args[i].c_str());
    }
    (*argv)[argc] = NULL;  // argv must be NULL terminated.

    return argc;
}

}  // namespace angle
