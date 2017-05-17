//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/HashNames.h"

#include "compiler/translator/IntermNode.h"

namespace sh
{

namespace
{

TString HashName(const TString &name, ShHashFunction64 hashFunction)
{
    ASSERT(!name.empty());
    ASSERT(hashFunction);
    khronos_uint64_t number = (*hashFunction)(name.c_str(), name.length());
    TStringStream stream;
    stream << HASHED_NAME_PREFIX << std::hex << number;
    TString hashedName = stream.str();
    return hashedName;
}

}  // anonymous namespace

TString HashName(const TName &name, ShHashFunction64 hashFunction, NameMap *nameMap)
{
    if (name.getString().empty() || name.isInternal())
    {
        return name.getString();
    }
    if (hashFunction == nullptr)
    {
        // Can't prefix with just _ because then we might introduce a double underscore, which is
        // not allowed in GLSL. u is short for user-defined.
        return "_u" + name.getString();
    }
    if (nameMap)
    {
        NameMap::const_iterator it = nameMap->find(name.getString().c_str());
        if (it != nameMap->end())
            return it->second.c_str();
    }
    TString hashedName = HashName(name.getString(), hashFunction);
    if (nameMap)
    {
        (*nameMap)[name.getString().c_str()] = hashedName.c_str();
    }
    return hashedName;
}

}  // namespace sh
