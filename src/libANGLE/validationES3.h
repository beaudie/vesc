//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// validationES3.h:
//  Inlined validation functions for OpenGL ES 3.0 entry points.

#ifndef LIBANGLE_VALIDATION_ES3_H_
#define LIBANGLE_VALIDATION_ES3_H_

#include "libANGLE/ErrorStrings.h"
#include "libANGLE/validationES3_autogen.h"

namespace gl
{
bool ValidateES3TexStorageParametersBase(Context *context,
                                         TextureType target,
                                         GLsizei levels,
                                         GLenum internalformat,
                                         GLsizei width,
                                         GLsizei height,
                                         GLsizei depth);
}  // namespace gl

#endif  // LIBANGLE_VALIDATION_ES3_H_
