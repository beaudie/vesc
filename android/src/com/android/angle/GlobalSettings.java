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

import android.content.ContentResolver;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.provider.Settings;
import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

class GlobalSettings
{

    private final String TAG = this.getClass().getSimpleName();

    private static final String GLOBAL_SETTINGS_ALL_ANGLE = "angle_gl_driver_all_angle";
    // Global.Settings value to list PKGs that have an OpenGL driver selected.
    private static final String GLOBAL_SETTINGS_DRIVER_PKGS = "angle_gl_driver_selection_pkgs";
    // Global.Settings vlaue to list which OpenGL driver is selected for the corresponding PKG.
    private static final String GLOBAL_SETTINGS_DRIVER_VALUES = "angle_gl_driver_selection_values";

    private Context mContext;
    private List<PackageInfo> mInstalledPkgs         = new ArrayList<>();
    private List<String> mGlobalSettingsDriverPkgs   = new ArrayList<>();
    private List<String> mGlobalSettingsDriverValues = new ArrayList<>();

    GlobalSettings(Context context, List<PackageInfo> installedPkgs)
    {
        mContext = context;

        setInstalledPkgs(installedPkgs);
    }

    Boolean getAllUseAngle()
    {
        ContentResolver contentResolver = mContext.getContentResolver();
        try
        {
            int allUseAngle = Settings.Global.getInt(contentResolver, GLOBAL_SETTINGS_ALL_ANGLE);
            return (allUseAngle == 1);
        }
        catch (Settings.SettingNotFoundException e)
        {
            return false;
        }
    }

    static void updateAllUseAngle(Context context, Boolean allUseAngle)
    {
        ContentResolver contentResolver = context.getContentResolver();
        Settings.Global.putInt(contentResolver, GLOBAL_SETTINGS_ALL_ANGLE, allUseAngle ? 1 : 0);
    }

    void updatePkg(String pkgName, String driver)
    {
        int pkgIndex = getGlobalSettingsPkgIndex(pkgName);

        if (!isValidDiverValue(driver))
        {
            Log.e(TAG, "Attempting to update a PKG with an invalid driver: '" + driver + "'");
            return;
        }

        String defaultDriver = mContext.getString(R.string.default_driver);
        if (driver.equals(defaultDriver))
        {
            if (pkgIndex >= 0)
            {
                // We only store global settings for driver values other than the default
                mGlobalSettingsDriverPkgs.remove(pkgIndex);
                mGlobalSettingsDriverValues.remove(pkgIndex);
            }
        }
        else
        {
            if (pkgIndex >= 0)
            {
                mGlobalSettingsDriverValues.set(pkgIndex, driver);
            }
            else
            {
                mGlobalSettingsDriverPkgs.add(pkgName);
                mGlobalSettingsDriverValues.add(driver);
            }
        }

        writeGlobalSettings();
    }

    String getDriverForPkg(String pkgName)
    {
        int pkgIndex = getGlobalSettingsPkgIndex(pkgName);

        if (pkgIndex >= 0)
        {
            return mGlobalSettingsDriverValues.get(pkgIndex);
        }

        return null;
    }

    private void setInstalledPkgs(List<PackageInfo> installedPkgs)
    {
        mInstalledPkgs = installedPkgs;

        updateGlobalSettings();
    }

    private void updateGlobalSettings()
    {
        readGlobalSettings();

        validateGlobalSettings();

        writeGlobalSettings();
    }

    private void readGlobalSettings()
    {
        mGlobalSettingsDriverPkgs   = getGlobalSettingsString(GLOBAL_SETTINGS_DRIVER_PKGS);
        mGlobalSettingsDriverValues = getGlobalSettingsString(GLOBAL_SETTINGS_DRIVER_VALUES);
    }

    private List<String> getGlobalSettingsString(String globalSetting)
    {
        List<String> valueList;
        ContentResolver contentResolver = mContext.getContentResolver();
        String settingsValue            = Settings.Global.getString(contentResolver, globalSetting);

        if (settingsValue != null)
        {
            valueList = new ArrayList<>(Arrays.asList(settingsValue.split(",")));
        }
        else
        {
            valueList = new ArrayList<>();
        }

        return valueList;
    }

    private void writeGlobalSettings()
    {
        String driverPkgs   = String.join(",", mGlobalSettingsDriverPkgs);
        String driverValues = String.join(",", mGlobalSettingsDriverValues);

        ContentResolver contentResolver = mContext.getContentResolver();
        Settings.Global.putString(contentResolver, GLOBAL_SETTINGS_DRIVER_PKGS, driverPkgs);
        Settings.Global.putString(contentResolver, GLOBAL_SETTINGS_DRIVER_VALUES, driverValues);
    }

    private void validateGlobalSettings()
    {
        // Verify lengths
        if (mGlobalSettingsDriverPkgs.size() != mGlobalSettingsDriverValues.size())
        {
            // The lengths don't match, so clear the values out and rebuild later.
            mGlobalSettingsDriverPkgs.clear();
            mGlobalSettingsDriverValues.clear();
            return;
        }

        // Remove any uninstalled packages
        for (String pkgName : mGlobalSettingsDriverPkgs)
        {
            if (!isPkgInstalled(pkgName))
            {
                removePkgFromGlobalSettings(pkgName);
            }
        }

        String defaultDriver = mContext.getString(R.string.default_driver);
        for (int i = 0; i < mGlobalSettingsDriverValues.size(); i++)
        {
            // Remove any packages with invalid driver values
            if (!isValidDiverValue(mGlobalSettingsDriverValues.get(i)))
            {
                removePkgFromGlobalSettings(mGlobalSettingsDriverPkgs.get(i));
            }
            // Remove any packages with the 'default' driver selected
            if (defaultDriver.equals(mGlobalSettingsDriverValues.get(i)))
            {
                removePkgFromGlobalSettings(mGlobalSettingsDriverPkgs.get(i));
            }
        }
    }

    private void removePkgFromGlobalSettings(String pkgName)
    {
        int pkgIndex = getGlobalSettingsPkgIndex(pkgName);

        mGlobalSettingsDriverPkgs.remove(pkgIndex);
        mGlobalSettingsDriverValues.remove(pkgIndex);
    }

    private int getGlobalSettingsPkgIndex(String pkgName)
    {
        for (int pkgIndex = 0; pkgIndex < mGlobalSettingsDriverPkgs.size(); pkgIndex++)
        {
            if (mGlobalSettingsDriverPkgs.get(pkgIndex).equals(pkgName))
            {
                return pkgIndex;
            }
        }

        return -1;
    }

    private Boolean isPkgInstalled(String pkgName)
    {
        for (PackageInfo pkg : mInstalledPkgs)
        {
            if (pkg.packageName.equals(pkgName))
            {
                return true;
            }
        }

        return false;
    }

    private Boolean isValidDiverValue(String driverValue)
    {
        CharSequence[] drivers = mContext.getResources().getStringArray(R.array.driver_values);

        for (CharSequence driver : drivers)
        {
            if (driverValue.equals(driver.toString()))
            {
                return true;
            }
        }

        return false;
    }
}
