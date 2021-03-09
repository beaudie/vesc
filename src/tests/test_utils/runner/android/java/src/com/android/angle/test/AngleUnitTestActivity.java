// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package com.android.angle.test;

import android.app.NativeActivity;
import android.os.Bundle;

import org.chromium.build.native_test.NativeTest;

/**
 * A {@link android.app.NativeActivity} for running angle gtests.
 */
public class AngleUnitTestActivity extends NativeActivity
{
    private NativeTest mTest = new NativeTest(new AngleNativeTestDelegate());

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        mTest.preCreate(this);
        super.onCreate(savedInstanceState);
        mTest.postCreate(this);
    }

    @Override
    public void onStart()
    {
        super.onStart();
        // Force running in sub thread,
        // since NativeActivity processes Looper messages in native code,
        // which makes invoking the test runner Handler problematic.
        mTest.postStart(this, true);
    }
}
