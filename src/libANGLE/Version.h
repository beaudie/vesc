//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Version.h: Encapsulation of a GL version.

#ifndef LIBANGLE_VERSION_H_
#define LIBANGLE_VERSION_H_

#include <angle_gl.h>

namespace gl
{

struct Version
{
    constexpr Version();
    constexpr Version(GLuint major, GLuint minor);

    GLuint major;
    GLuint minor;
};

bool operator==(const Version &a, const Version &b);
bool operator!=(const Version &a, const Version &b);
bool operator>=(const Version &a, const Version &b);
bool operator<=(const Version &a, const Version &b);
bool operator<(const Version &a, const Version &b);
bool operator>(const Version &a, const Version &b);

struct VersionRange
{
    constexpr VersionRange();
    constexpr VersionRange(const Version &min, const Version &max);

    Version min;
    Version max;
};

bool VersionInRange(const VersionRange &range, const Version &version);
}

#include "Version.inl"

namespace gl
{
static constexpr Version MinimumVersion = Version(0, 0);
static constexpr Version MaximumVersion =
    Version(std::numeric_limits<GLuint>::max(), std::numeric_limits<GLuint>::max());
static constexpr VersionRange AllVersions = VersionRange(MinimumVersion, MinimumVersion);
static constexpr VersionRange NoVersions  = VersionRange(MinimumVersion, MaximumVersion);
}

#endif // LIBANGLE_VERSION_H_
