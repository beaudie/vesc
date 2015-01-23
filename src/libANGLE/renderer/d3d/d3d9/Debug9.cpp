// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Debug9.cpp: Implements the Debug9 class for handling debug events and markers

#include "libANGLE/renderer/d3d/d3d9/Debug9.h"

#include "common/debug.h"

namespace rx
{

Debug9::Debug9()
{
}

Debug9::~Debug9()
{
}

void Debug9::beginEvent(const std::string &msg)
{
#if defined(ANGLE_ENABLE_DEBUG_ANNOTATIONS)
    std::wstring wideMessage(msg.begin(), msg.end());
    D3DPERF_BeginEvent(0, wideMessage.c_str());
#endif
}

void Debug9::endEvent()
{
#if defined(ANGLE_ENABLE_DEBUG_ANNOTATIONS)
    D3DPERF_EndEvent();
#endif
}

void Debug9::setMarker(const std::string &msg)
{
#if defined(ANGLE_ENABLE_DEBUG_ANNOTATIONS)
    std::wstring wideMessage(msg.begin(), msg.end());
    D3DPERF_SetMarker(0, wideMessage.c_str());
#endif
}

}
