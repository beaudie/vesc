//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/HashNames.h"

#include "compiler/translator/ImmutableString.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/Symbol.h"

namespace sh
{

namespace
{

// GLSL ES 3.00.6 section 3.9: the maximum length of an identifier is 1024 characters.
static const unsigned int kESSLMaxIdentifierLength = 1024u;

static const char *kHashedNamePrefix = "webgl_";
static const unsigned int kHashedNamePrefixLength = 6u;

// Can't prefix with just _ because then we might introduce a double underscore, which is not safe
// in GLSL (ESSL 3.00.6 section 3.8: All identifiers containing a double underscore are reserved for
// use by the underlying implementation). u is short for user-defined.
static const char *kUnhashedNamePrefix              = "_u";
static const unsigned int kUnhashedNamePrefixLength = 2u;

ImmutableString HashName(const ImmutableString &name, ShHashFunction64 hashFunction)
{
    ASSERT(!name.empty());
    ASSERT(hashFunction);
    khronos_uint64_t number = (*hashFunction)(name, name.length());

    // Build the hashed name in place.
    static const unsigned int kHexStrMaxLength = 16;
    static const size_t kHashedNameMaxLength   = kHashedNamePrefixLength + kHexStrMaxLength;

    char *hashedName = AllocateEmptyPoolCharArray(kHashedNameMaxLength);
    memcpy(hashedName, kHashedNamePrefix, kHashedNamePrefixLength);

    char *digitWritePtr = hashedName + kHashedNamePrefixLength;
    while (number != 0)
    {
        khronos_uint64_t digit = number & 0xf;
        char digitChar         = digit < 10 ? digit + '0' : digit + 'a';
        *(digitWritePtr)       = digitChar;
        number                 = number >> 4;
        ++digitWritePtr;
    }
    ASSERT(digitWritePtr <= hashedName + kHashedNameMaxLength);
    *(digitWritePtr) = '\0';
    return ImmutableString(hashedName, digitWritePtr - hashedName);
}

}  // anonymous namespace

ImmutableString HashName(const ImmutableString &name,
                         ShHashFunction64 hashFunction,
                         NameMap *nameMap)
{
    if (hashFunction == nullptr)
    {
        if (name.length() + kUnhashedNamePrefixLength > kESSLMaxIdentifierLength)
        {
            // If the identifier length is already close to the limit, we can't prefix it. This is
            // not a problem since there are no builtins or ANGLE's internal variables that would
            // have as long names and could conflict.
            return name;
        }
        char *prefixedName = AllocateEmptyPoolCharArray(kUnhashedNamePrefixLength + name.length());
        memcpy(prefixedName, kUnhashedNamePrefix, kUnhashedNamePrefixLength);
        memcpy(prefixedName + kUnhashedNamePrefixLength, name.data(), name.length());
        return ImmutableString(prefixedName, kUnhashedNamePrefixLength + name.length());
    }
    if (nameMap)
    {
        NameMap::const_iterator it = nameMap->find(name.data());
        if (it != nameMap->end())
        {
            // TODO: Don't allocate string here.
            return ImmutableString(it->second);
        }
    }
    ImmutableString hashedName = HashName(name, hashFunction);
    if (nameMap)
    {
        (*nameMap)[name.data()] = hashedName.data();
    }
    return hashedName;
}

ImmutableString HashName(const TSymbol *symbol, ShHashFunction64 hashFunction, NameMap *nameMap)
{
    if (symbol->symbolType() == SymbolType::Empty)
    {
        return ImmutableString("");
    }
    if (symbol->symbolType() == SymbolType::AngleInternal ||
        symbol->symbolType() == SymbolType::BuiltIn)
    {
        return symbol->name();
    }
    return HashName(symbol->name(), hashFunction, nameMap);
}

}  // namespace sh
