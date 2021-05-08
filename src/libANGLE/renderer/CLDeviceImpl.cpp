//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLDeviceImpl.cpp: Implements the class methods for CLDeviceImpl.

#include "libANGLE/renderer/CLDeviceImpl.h"

namespace rx
{

CLDeviceImpl::Info::Info() = default;

CLDeviceImpl::Info::~Info() = default;

CLDeviceImpl::Info::Info(Info &&) = default;

CLDeviceImpl::Info &CLDeviceImpl::Info::operator=(Info &&) = default;

bool CLDeviceImpl::Info::isValid() const
{
    return mMaxWorkItemSizes.size() >= 3u && mMaxWorkItemSizes[0] >= 1u &&
           mMaxWorkItemSizes[1] >= 1u && mMaxWorkItemSizes[2] >= 1u;
}

}  // namespace rx
