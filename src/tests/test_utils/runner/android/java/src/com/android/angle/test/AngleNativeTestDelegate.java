// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package com.android.angle.test;

import android.app.Activity;
import android.util.Log;

import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.base.library_loader.NativeLibraries;
import org.chromium.build.native_test.NativeTestDelegate;

/**
 * {@link NativeTestDelegate} for angle gtests.
 */
public class AngleNativeTestDelegate implements NativeTestDelegate
{
    private static String TAG = "AngleNativeTest";

    @Override
    public void preCreate(Activity activity)
    {
        LibraryLoader.setEnvForNative();
        for (String library : NativeLibraries.LIBRARIES)
        {
            Log.i(TAG, "loading: " + library);
            System.loadLibrary(library);
            Log.i(TAG, "loaded: " + library);
        }
    }

    public void runTests(String commandLineFlags, String commandLineFilePath, String stdoutFilePath)
    {
        nativeRunTests(commandLineFlags, commandLineFilePath, stdoutFilePath);
    }

    private static native void nativeRunTests(
            String commandLineFlags, String commandLineFilePath, String stdoutFilePath);
}
