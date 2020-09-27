//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file includes APIs to detect whether certain Apple renderer is availabe for testing.
//

#ifndef ANGLE_TEST_INSTANTIATE_APPLE_H_
#define ANGLE_TEST_INSTANTIATE_APPLE_H_

namespace angle
{

bool IsMetalRendererAvailable();
// Check whether texture swizzle is natively supported on Metal device.
bool IsMetalTextureSwizzleAvailable();

}  // namespace angle

#endif  // ANGLE_TEST_INSTANTIATE_APPLE_H_
