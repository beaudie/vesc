//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// serial_utils:
//   Implementation of Utilities for generating unique IDs for resources in ANGLE.
//

#include "serial_utils.h"

namespace rx
{
// AtomicSerial implementation
AtomicSerial::AtomicSerial() : mValue(Serial::kInvalid) {}
}  // namespace rx
