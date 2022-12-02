//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ANGLEShaderProgramVersion.cpp:
//      Defines the values for the static const ANGLE_PROGRAM_VERSION.
//      Whenever we touch files that affect program serialize/deserialize,
//      we should increment the ANGLE_PROGRAM_VERSION number here as well.
//
#include "Context.h"
// test presubmit script
// def angleProgramVersionFile(f):
//        return input_api.FilterSourceFile(
//            f, files_to_check=(r'^src/libANGLE/ANGLEShaderProgramVersion.cpp$'))
namespace gl
{
const int Context::ANGLE_PROGRAM_VERSION = 1;
}
