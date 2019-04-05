//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef TEST_EXPECTATIONS_GPU_TEST_INFO_H_
#define TEST_EXPECTATIONS_GPU_TEST_INFO_H_

#include <string>

namespace angle
{

enum API
{
    kAPIUnknown   = 0,
    kAPID3D9      = 1 << 0,
    kAPID3D11     = 1 << 1,
    kAPIGLDesktop = 1 << 2,
    kAPIGLES      = 1 << 3,
    kAPIVulkan    = 1 << 4,
};

bool IsWinXP();
bool IsWinVista();
bool IsWin7();
bool IsWin8();
bool IsWin10();
bool IsWin();

bool IsMacLeopard();
bool IsMacSnowLeopard();
bool IsMacLion();
bool IsMacMountainLion();
bool IsMacMavericks();
bool IsMacYosemite();
bool IsMacElCapitan();
bool IsMacSierra();
bool IsMacHighSierra();
bool IsMacMojave();
bool IsMac();

bool IsLinux();
bool IsAndroid();

bool IsNVidia();
bool IsAMD();
bool IsIntel();
bool IsVMWare();

bool IsRelease();
bool IsDebug();

bool IsNexus5X();
bool IsPixel2();

bool IsNVidiaQuadroP400();

bool IsDeviceIdGPU(const std::string &gpu_device_id);

// Setup the parser with the current gpu testing environment.
bool LoadCurrentConfig(API api);

bool IsD3D9();
bool IsD3D11();
bool IsGLDesktop();
bool IsGLES();
bool IsVulkan();

}  // namespace angle

#endif  // TEST_EXPECTATIONS_GPU_TEST_INFO_H_
