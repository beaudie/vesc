//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// angle_deqp_tests_main.cpp: Entry point for ANGLE's dEQP tests.

__declspec(dllimport) int deqp_libtester_main(int argc, const char* argv[]);

int main(int argc, const char* argv[])
{
    deqp_libtester_main(argc, argv);
}
