// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Debug11.cpp: Implements the Debug11 class for handling debug events and markers

#include "libANGLE/renderer/d3d/d3d11/Debug11.h"

#include "common/debug.h"

namespace rx
{

Debug11::Debug11(ID3DUserDefinedAnnotation *userDefinedAnnotation)
    : mUserDefinedAnnotation(userDefinedAnnotation)
{
    ASSERT(mUserDefinedAnnotation);
    mUserDefinedAnnotation->AddRef();
}

Debug11::~Debug11()
{
    SafeRelease(mUserDefinedAnnotation);
}

void Debug11::beginEvent(const std::string &msg)
{
#if defined(ANGLE_ENABLE_DEBUG_ANNOTATIONS)
    std::wstring wideMessage(msg.begin(), msg.end());
    mUserDefinedAnnotation->BeginEvent(wideMessage.c_str());
#endif
}

void Debug11::endEvent()
{
#if defined(ANGLE_ENABLE_DEBUG_ANNOTATIONS)
    mUserDefinedAnnotation->EndEvent();
#endif
}

void Debug11::setMarker(const std::string &msg)
{
#if defined(ANGLE_ENABLE_DEBUG_ANNOTATIONS)
    std::wstring wideMessage(msg.begin(), msg.end());
    mUserDefinedAnnotation->SetMarker(wideMessage.c_str());
#endif
}

}
