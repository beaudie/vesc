//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

package org.angle.util;

import android.os.Environment;

/**
 * This class provides the path related methods for the native library.
 */
public abstract class Utils
{

    /**
     * @return the external storage directory.
     */
    @SuppressWarnings("unused")
    @CalledByNative
    public static String getExternalStorageDirectory()
    {
        return Environment.getExternalStorageDirectory().getAbsolutePath();
    }
}
