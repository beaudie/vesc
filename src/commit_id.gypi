# Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
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
                    'target_name': 'libGLESv2',
                    'type': 'none',
                    'actions':
                    [
                        {
                            'action_name': 'Generate Commit ID Header',
                            'message': 'Generating commit ID header...',
                            'inputs': [],
                            'outputs': ['common/commit.h'],
                            'action': ['gyp', 'rev-parse', '--short=12', 'HEAD', '>', 'common/commit.h'],
                        }
                    ] #actions
                }
            ] # targets
        }
    }
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
# Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
