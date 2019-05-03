//
// Copyright (c) 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_WORKAROUND_H_
#define LIBANGLE_WORKAROUND_H_

#include <string>

namespace angle
{

struct Workaround
{
    Workaround();
    Workaround(const Workaround &other);
    Workaround(const std::string &name,
               const std::string &set,
               const std::string &description,
               const int chromiumBugId,
               const int angleBugId);

    std::string name;
    std::string set;
    std::string description;
    uint chromiumBugId;
    uint angleBugId;

    bool applied = false;
};

inline Workaround::Workaround()                        = default;
inline Workaround::Workaround(const Workaround &other) = default;
inline Workaround::Workaround(const std::string &name,
                              const std::string &set,
                              const std::string &description,
                              const int chromiumBugId,
                              const int angleBugId)
    : name(name),
      set(set),
      description(description),
      chromiumBugId(chromiumBugId),
      angleBugId(angleBugId),
      applied(false)
{}

}  // namespace angle

#endif  // LIBANGLE_WORKAROUND_H_
