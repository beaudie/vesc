# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os


def _CommonChecks(input_api, output_api):
    results = []

    # Validate the format of the mb_config.pyl file.
    d = os.path.dirname
    angle_root_dir = d(d(input_api.PresubmitLocalPath()))
    mb_path = os.path.join(angle_root_dir, 'tools', 'mb', 'mb.py')
    config_path = os.path.join(input_api.PresubmitLocalPath(), 'angle_mb_config.pyl')

    cmd = [input_api.python_executable, mb_path, 'validate', '-f', config_path]
    kwargs = {'cwd': input_api.PresubmitLocalPath()}
    results.extend(
        input_api.RunTests([
            input_api.Command(
                name='mb_validate', cmd=cmd, kwargs=kwargs, message=output_api.PresubmitError)
        ]))

    return results


def CheckChangeOnUpload(input_api, output_api):
    return _CommonChecks(input_api, output_api)


def CheckChangeOnCommit(input_api, output_api):
    return _CommonChecks(input_api, output_api)
