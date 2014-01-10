# Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'conditions':
    [
        ['OS=="win"',
        {
            'targets':
            [
                {
                    'target_name': 'dds_to_header',
                    'type': 'executable',
                    'sources':
                    [
                        '<!@(python enumerate_files.py dds_to_header -types *.cpp)',
                    ],
                },
            ]
        }],
    ],
}
