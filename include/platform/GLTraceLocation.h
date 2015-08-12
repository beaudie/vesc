//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GLTraceLocation:
//   Platform API for a trace location (file/function pair).
//

#ifndef ANGLE_PLATFORM_GLTRACELOCATION_H_
#define ANGLE_PLATFORM_GLTRACELOCATION_H_

#include "../export.h"

namespace angle
{

// This class is used to keep track of where posted tasks originate. See base/location.h in
// Chromium.
// Currently only store the bits used in Blink, base::Location stores more.
class ANGLE_EXPORT GLTraceLocation
{
  public:
    // These char*s are not copied and must live for the duration of the program.
    GLTraceLocation(const char *function, const char *file);
    GLTraceLocation();

    const char *functionName() const { return mFunctionName; }
    const char *fileName() const { return mFileName; }

  private:
    const char *mFunctionName;
    const char *mFileName;
};

inline GLTraceLocation::GLTraceLocation(const char *function, const char *file)
    : mFunctionName(function), mFileName(file)
{
}

inline GLTraceLocation::GLTraceLocation() : mFunctionName("unknown"), mFileName("unknown")
{
}

#if LIBANGLE_IMPLEMENTATION
#define FROM_HERE ::angle::GLTraceLocation(__FUNCTION__, __FILE__)
#endif  // LIBANGLE_IMPLEMENTATION

}  // angle

#endif  // ANGLE_PLATFORM_GLTRACELOCATION_H_
