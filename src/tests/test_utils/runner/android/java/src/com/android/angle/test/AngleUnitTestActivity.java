// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AngleUnitTestActivity:
//   A {@link android.app.NativeActivity} for running angle gtests.

package com.android.angle.test;

import android.app.NativeActivity;
import android.os.Bundle;
import android.os.SystemClock;
import android.util.Log;

import org.chromium.build.NativeLibraries;

public class AngleUnitTestActivity extends NativeActivity
{
    private static final String TAG = "NativeTest";

    private AngleNativeTest mTest = new AngleNativeTest();

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        Log.e(TAG, "ASDF onCreate");
        // For NativeActivity based tests,
        // dependency libraries must be loaded before NativeActivity::OnCreate,
        // otherwise loading android.app.lib_name will fail
        for (String library : NativeLibraries.LIBRARIES)
        {
            Log.i(TAG, "loading: " + library);
            Log.e(TAG, "ASDF mapped name is " + System.mapLibraryName(library));
            System.loadLibrary(library);
            Log.i(TAG, "loaded: " + library);
        }

        Log.e(TAG, "ASDF done loading libraries");
        if (savedInstanceState != null) {
            Log.e(TAG, "ASDF bundle: " + savedInstanceState.toString());    
        } else {
            Log.e(TAG, "ASDF bundle was null");
        }
        Log.e(TAG, "ASDF starting sleep");
        SystemClock.sleep(5000);
        Log.e(TAG, "ASDF done sleeping");
        super.onCreate(savedInstanceState);
        Log.e(TAG, "ASDF calling postCreate");
        mTest.postCreate(this);
        Log.e(TAG, "ASDF done calling postCreate");
    }

    @Override
    public void onStart()
    {
        super.onStart();
        Log.e(TAG, "ASDF onStart");
        mTest.postStart(this);
    }
}
