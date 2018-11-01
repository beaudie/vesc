/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.android.angle;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;

public class Receiver extends BroadcastReceiver
{

    private final static String TAG = Receiver.class.getSimpleName();

    @Override
    public void onReceive(Context context, Intent intent)
    {
        // Nothing to do yet...
    }

    static void updateAllUseAngle(Context context)
    {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        String allUseAngleKey   = context.getString(R.string.pref_key_all_angle);
        boolean allUseAngle     = prefs.getBoolean(allUseAngleKey, false);

        GlobalSettings.updateAllUseAngle(context, allUseAngle);

        Log.v(TAG, "Use Rules File set to: " + allUseAngle);
    }
}
