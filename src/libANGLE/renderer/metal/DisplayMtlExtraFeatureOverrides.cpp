//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayMtlExtraFeatureOverrides.cpp:
//   Empty implementation for ANGLE compiles that do not have extra overrides.
//   Other compiles, for example downstream projects, may compile in a similar
//   file that overrides some upstream defaults.
//

#include "libANGLE/Display.h"
#include "platform/FeaturesMtl_autogen.h"

namespace rx
{

void ApplyExtraFeatureOverrides(angle::FeaturesMtl *features, const egl::DisplayState &state) {}

}  // namespace rx
