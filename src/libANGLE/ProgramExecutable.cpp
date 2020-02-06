//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/ProgramExecutable.h"

namespace gl
{

ProgramExecutable::ProgramExecutable() {}

ProgramExecutable::~ProgramExecutable() {}

int ProgramExecutable::getInfoLogLength() const
{
    return static_cast<int>(mInfoLog.getLength());
}

void ProgramExecutable::getInfoLog(GLsizei bufSize, GLsizei *length, char *infoLog) const
{
    return mInfoLog.getLog(bufSize, length, infoLog);
}

std::string ProgramExecutable::getInfoLogString() const
{
    return mInfoLog.str();
}

}  // namespace gl
