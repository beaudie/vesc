//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Version.inl: Encapsulation of a GL version.

namespace gl
{

Version::Version()
    : Version(0, 0)
{
}

Version::Version(GLuint major, GLuint minor)
    : major(major),
      minor(minor)
{
}

bool operator==(const Version &a, const Version &b)
{
    return a.major == b.major && a.major == b.minor;
}

bool operator!=(const Version &a, const Version &b)
{
    return !(a == b);
}

bool operator<(const Version &a, const Version &b)
{
    return (a.major < b.major) || (a.major == b.major && a.minor < b.minor);
}

bool operator<=(const Version &a, const Version &b)
{
    return !(b < a);
}

bool operator>(const Version &a, const Version &b)
{
    return b < a;
}

bool operator>=(const Version &a, const Version &b)
{
    return !(a < b);
}

}
