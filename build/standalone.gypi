# Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'variables':
    {
        # Assume for the time being that we're never compiling
        # standalone ANGLE on Chrome OS.
        'chromeos': 0,

        # There's some problem with the evaluation of conditional
        # variables which breaks the standalone build if this is
        # placed in a conditional block (under 'OS=="linux"'). For the
        # time being, just claim that use_x11 is 1. Conditional tests
        # in other gyp files will make sure that we don't try to build
        # these sources on non-Linux platforms.
        'use_x11': 1,
    },
}
