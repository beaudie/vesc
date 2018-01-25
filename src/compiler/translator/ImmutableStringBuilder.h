//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImmutableStringBuilder.h: Stringstream-like utility for building pool allocated strings where the
// maximum length is known in advance.
//

#ifndef COMPILER_TRANSLATOR_IMMUTABLESTRINGBUILDER_H_
#define COMPILER_TRANSLATOR_IMMUTABLESTRINGBUILDER_H_

#include "compiler/translator/ImmutableString.h"

namespace sh
{

class ImmutableStringBuilder
{
  public:
    ImmutableStringBuilder(size_t maxLength)
        : mPos(0u), mMaxLength(maxLength), mData(AllocateEmptyPoolCharArray(maxLength))
    {
    }

    ImmutableStringBuilder &operator<<(const ImmutableString &str)
    {
        ASSERT(mData != nullptr);
        ASSERT(mPos + str.length() <= mMaxLength);
        memcpy(mData + mPos, str.data(), str.length());
        mPos += str.length();
        return *this;
    }

    ImmutableStringBuilder &operator<<(const char *str)
    {
        ASSERT(mData != nullptr);
        size_t len = strlen(str);
        ASSERT(mPos + len <= mMaxLength);
        memcpy(mData + mPos, str, len);
        mPos += len;
        return *this;
    }

    ImmutableStringBuilder &operator<<(const char &c)
    {
        ASSERT(mData != nullptr);
        ASSERT(mPos + 1 <= mMaxLength);
        mData[mPos++] = c;
        return *this;
    }

    operator ImmutableString()
    {
        mData[mPos] = '\0';
        ImmutableString str(static_cast<const char *>(mData), mPos);
#if defined(ANGLE_ENABLE_ASSERTS)
        // Make sure that nothing is added to the string after it is finalized.
        mData = nullptr;
#endif
        return str;
    }

  private:
    inline static char *AllocateEmptyPoolCharArray(size_t strLength)
    {
        size_t requiredSize = strLength + 1u;
        return reinterpret_cast<char *>(GetGlobalPoolAllocator()->allocate(requiredSize));
    }

    size_t mPos;
    size_t mMaxLength;
    char *mData;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_IMMUTABLESTRINGBUILDER_H_
