//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// trace_interface:
//   Interface shared between trace libraries and ANGLE test suite.
//

#ifndef UTIL_CAPTURE_TRACE_INTERFACE_H_
#define UTIL_CAPTURE_TRACE_INTERFACE_H_

namespace angle
{

struct TraceCallbacks
{
    virtual uint8_t *LoadBinaryData(const char *fileName) = 0;
    virtual ~TraceCallbacks() {}
};

}  // namespace angle
#endif  // UTIL_CAPTURE_TRACE_INTERFACE_H_
