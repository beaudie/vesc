//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Version.inl: Encapsulation of a GL version.

#include <tuple>

namespace gl
{

constexpr Version::Version()
    : Version(0, 0)
{
}

// Avoid conflicts with linux system defines
#undef major
#undef minor

constexpr Version::Version(GLuint major_, GLuint minor_)
    : major(major_),
      minor(minor_)
{
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

constexpr VersionRange::VersionRange()
    : VersionRange(Version(), Version())
{
}

constexpr VersionRange::VersionRange(const Version &minimum_, const Version &maximum_)
    : minimum(minimum_),
      maximum(maximum_)
{
}

inline bool VersionInRange(const VersionRange &range, const Version &version)
{
    return version >= range.minimum && version <= range.maximum;
}

}
