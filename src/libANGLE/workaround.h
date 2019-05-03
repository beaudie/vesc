//
// Copyright (c) 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_WORKAROUND_H_
#define LIBANGLE_WORKAROUND_H_

#include <map>
#include <string>

namespace angle
{

struct Workaround
{
    Workaround(const Workaround &other);
    Workaround(const std::string &name,
               const std::string &set,
               const std::string &description,
               std::map<std::string, Workaround *> *const mapPtr,
               const std::string &bug);
    ~Workaround();

    // The name of the workaround, lowercase, camel_case
    const std::string name;

    // The set that the workaround belongs to. Eg. "Vulkan workarounds"
    const std::string set;

    // A short description to be read by the user.
    const std::string description;

    // A link to the bug, if any
    const std::string bug;

    // Whether the workaround is applied or not. Determined by heuristics like vendor ID and
    // version, but may be overriden to any value.
    bool applied = false;
};

inline Workaround::Workaround(const Workaround &other) = default;
inline Workaround::Workaround(const std::string &name,
                              const std::string &set,
                              const std::string &description,
                              std::map<std::string, Workaround *> *const mapPtr,
                              const std::string &bug = "")
    : name(name), set(set), description(description), bug(bug), applied(false)
{
    if (mapPtr != nullptr)
    {
        (*mapPtr)[name] = this;
    }
}

inline Workaround::~Workaround() = default;

}  // namespace angle

#endif  // LIBANGLE_WORKAROUND_H_
