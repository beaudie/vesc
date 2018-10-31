//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WorkaroundsVulkan.h: Workarounds for the Vulkan driver.
//

namespace angle
{

struct WorkaroundsVulkan
{
    // This workaround flag is added for the sole purpose of end2end tests, to test the correctness
    // of various algorithms when a packed format is used to emulate a depth- or stencil-only
    // format.
    bool forceEmulateWithPackedDepthStencil = false;
};

}  // namespace angle
