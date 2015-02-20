//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_FEATURES_H_
#define LIBANGLE_FEATURES_H_

#define ANGLE_DISABLED 0
#define ANGLE_ENABLED 1

// Feature defaults

// Direct3D9EX
// The "Debug This Pixel..." feature in PIX often fails when using the
// D3D9Ex interfaces.  In order to get debug pixel to work on a Vista/Win 7
// machine, define "ANGLE_D3D9EX=0" in your project file.
#if !defined(ANGLE_D3D9EX)
#define ANGLE_D3D9EX ANGLE_ENABLED
#endif

// Vsync
// ENABLED allows Vsync to be configured at runtime
// DISABLED disallows Vsync
#if !defined(ANGLE_VSYNC)
#define ANGLE_VSYNC ANGLE_ENABLED
#endif

// Program binary loading
#if !defined(ANGLE_PROGRAM_BINARY_LOAD)
#define ANGLE_PROGRAM_BINARY_LOAD ANGLE_ENABLED
#endif

// Shader debug info
#if !defined(ANGLE_SHADER_DEBUG_INFO)
#define ANGLE_SHADER_DEBUG_INFO ANGLE_DISABLED
#endif

// MULTITHREADED_D3D_SHADER_COMPILE_WIN8_PLUS
// ENABLED - on machines running Win8+, compile the D3D VS and PS on separate threads simultaneously
//         - on machines running Win7, compile the D3D VS and PS on the same thread back-to-back
// DISABLED - compile the D3D VS and PS shaders on the same thread back-to-back
#if !defined(ANGLE_MULTITHREADED_D3D_SHADER_COMPILE_WIN8_PLUS)
#define ANGLE_MULTITHREADED_D3D_SHADER_COMPILE_WIN8_PLUS ANGLE_ENABLED
#endif

#endif // LIBANGLE_FEATURES_H_
