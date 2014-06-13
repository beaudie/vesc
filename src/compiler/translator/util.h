//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_UTIL_H
#define COMPILER_UTIL_H

#include "compiler/translator/Types.h"
#include <GLES3/gl3.h>
#include <GLES2/gl2.h>
#include "common/shadervars.h"

// atof_clamp is like atof but
//   1. it forces C locale, i.e. forcing '.' as decimal point.
//   2. it clamps the value to -FLT_MAX or FLT_MAX if overflow happens.
// Return false if overflow happens.
extern bool atof_clamp(const char *str, float *value);

// If overflow happens, clamp the value to INT_MIN or INT_MAX.
// Return false if overflow happens.
extern bool atoi_clamp(const char *str, int *value);

namespace sh
{

GLenum GLVariableType(const TType &type);
GLenum GLVariablePrecision(const TType &type);
bool IsVaryingIn(TQualifier qualifier);
bool IsVaryingOut(TQualifier qualifier);
bool IsVarying(TQualifier qualifier);
gl::InterpolationType GetInterpolationType(TQualifier qualifier);
TString ArrayString(const TType &type);

}

#endif // COMPILER_UTIL_H
