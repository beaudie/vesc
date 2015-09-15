// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// gpu_test_config_mac.mm:
//   Helper functions for gpu_test_config that have to be compiled in ObjectiveC++

#include "gpu_test_config_mac.h"

#import <Cocoa/Cocoa.h>

namespace base {

void SysInfo::OperatingSystemVersionNumbers(
    int32 *major_version, int32 *minor_version, int32 *bugfix_version)
{
#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_8
  Gestalt(gestaltSystemVersionMajor, reinterpret_cast<SInt32*>(major_version));
  Gestalt(gestaltSystemVersionMinor, reinterpret_cast<SInt32*>(minor_version));
  Gestalt(gestaltSystemVersionBugFix, reinterpret_cast<SInt32*>(bugfix_version));
#else
  NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
  *major_version = version.majorVersion;
  *minor_version = version.minorVersion;
  *bugfix_version = version.patchVersion;
#endif
}

} // namespace base

gpu::GPUInfo::GPUDevice GetActiveGPUMac() {
  gpu::GPUInfo::GPUDevice gpu;
#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_9
  io_registry_entry_t dsp_port = CGDisplayIOServicePort(kCGDirectMainDisplay);
  gpu.vendor_id = GetEntryProperty(dsp_port, CFSTR("vendor-id"));
  gpu.device_id = GetEntryProperty(dsp_port, CFSTR("device-id"));
#else
  // TODO(cwallez) make sure this is correct
  gpu.vendor_id = CGDisplayVendorNumber(CGMainDisplayID());
  gpu.device_id = CGDisplaySerialNumber(CGMainDisplayID());
#endif
  return gpu;
}

