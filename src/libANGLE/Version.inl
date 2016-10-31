//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Version.inl: Encapsulation of a GL version.

#include <tuple>

namespace gl
{

inline Version::Version()
    : Version(0, 0)
{
}

inline Version::Version(GLuint major_, GLuint minor_)
{
    major = major_;
    minor = minor_;
}

inline bool operator==(const Version &a, const Version &b)
{
    return std::tie(a.major, a.minor) == std::tie(b.major, b.minor);
}

inline bool operator!=(const Version &a, const Version &b)
{
    return std::tie(a.major, a.minor) != std::tie(b.major, b.minor);
}

inline bool operator>=(const Version &a, const Version &b)
{
    return std::tie(a.major, a.minor) >= std::tie(b.major, b.minor);
}

inline bool operator<=(const Version &a, const Version &b)
{
    return std::tie(a.major, a.minor) <= std::tie(b.major, b.minor);
}

inline bool operator<(const Version &a, const Version &b)
{
    return std::tie(a.major, a.minor) < std::tie(b.major, b.minor);
}

inline bool operator>(const Version &a, const Version &b)
{
    return std::tie(a.major, a.minor) > std::tie(b.major, b.minor);
}

}
