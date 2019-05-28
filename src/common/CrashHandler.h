//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CrashHandler:
//    ANGLE's crash handling and stack walking code. Modified from Skia's:
//     https://github.com/google/skia/blob/master/tools/CrashHandler.h
//

#ifndef COMMON_CRASHHANDLER_H_
#define COMMON_CRASHHANDLER_H_

// If possible (and not already done), and SK_CRASH_HANDLER is defined,
// register a handler for a stack trace when we crash.
//
// Currently this works on Linux and Mac and Windows.
// On Linux, our output is garbage compared to catchsegv.  Use catchsegv if possible.
void SetupCrashHandler();

void PrintStackTrace();

#endif  // COMMON_CRASHHANDLER_H_
