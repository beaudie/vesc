//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef PREPROCESSOR_TESTS_MOCK_DIRECTIVE_HANDLER_H_
#define PREPROCESSOR_TESTS_MOCK_DIRECTIVE_HANDLER_H_

#include "compiler/preprocessor/DirectiveHandlerBase.h"
#include "gmock/gmock.h"

namespace angle
{

class MockDirectiveHandler : public pp::DirectiveHandler
{
  public:
    MOCK_METHOD(void,
                handleError,
                (const pp::SourceLocation &loc, const std::string &msg),
                (override));

    MOCK_METHOD(void,
                handlePragma,
                (const pp::SourceLocation &loc,
                 const std::string &name,
                 const std::string &value,
                 bool stdgl),
                (override));

    MOCK_METHOD(void,
                handleExtension,
                (const pp::SourceLocation &loc,
                 const std::string &name,
                 const std::string &behavior),
                (override));

    MOCK_METHOD(void,
                handleVersion,
                (const pp::SourceLocation &loc, int version, ShShaderSpec spec),
                (override));
};

}  // namespace angle

#endif  // PREPROCESSOR_TESTS_MOCK_DIRECTIVE_HANDLER_H_
