//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// feature_support_util.h: Internal-to-ANGLE header file for feature-support utilities.

#ifndef FEATURE_SUPPORT_UTIL_H_
#define FEATURE_SUPPORT_UTIL_H_

#include <string>

#include "export.h"
#include <json/json.h>

/**************************************************************************************************
 *
 * The following are the declarations of the internal classes:
 *
 **************************************************************************************************/

namespace angle
{

// This encapsulates a std::string.  The default constructor (not given a string) assumes that this
// is a wildcard (i.e. will match all other StringPart objects).
class StringPart
{
  public:
    StringPart() : mPart(""), mWildcard(true) {}
    StringPart(const std::string part) : mPart(part), mWildcard(false) {}
    ~StringPart() {}
    bool match(const StringPart &toCheck) const
    {
        return (mWildcard || toCheck.mWildcard || (toCheck.mPart == mPart));
    }
    std::string getPart() { return mPart; }
    bool getWildcard() { return mWildcard; }

  public:
    std::string mPart;
    bool mWildcard;
};

// This encapsulates a 32-bit unsigned integer.  The default constructor (not given a number)
// assumes that this is a wildcard (i.e. will match all other IntegerPart objects).
class IntegerPart
{
  public:
    IntegerPart() : mPart(0), mWildcard(true) {}
    IntegerPart(uint32_t part) : mPart(part), mWildcard(false) {}
    ~IntegerPart() {}
    bool match(const IntegerPart &toCheck) const
    {
        return (mWildcard || toCheck.mWildcard || (toCheck.mPart == mPart));
    }
    uint32_t getPart() { return mPart; }
    bool getWildcard() { return mWildcard; }

  public:
    uint32_t mPart;
    bool mWildcard;
};

// This encapsulates up-to four 32-bit unsigned integers, that represent a potentially-complex
// version number.  The default constructor (not given any numbers) assumes that this is a wildcard
// (i.e. will match all other Version objects).  Each part of a Version is stored in an IntegerPart
// class, and so may be wildcarded as well.
class Version
{
  public:
    Version(uint32_t major, uint32_t minor, uint32_t subminor, uint32_t patch)
        : mMajor(major), mMinor(minor), mSubminor(subminor), mPatch(patch), mWildcard(false)
    {}
    Version(uint32_t major, uint32_t minor, uint32_t subminor)
        : mMajor(major), mMinor(minor), mSubminor(subminor), mWildcard(false)
    {}
    Version(uint32_t major, uint32_t minor) : mMajor(major), mMinor(minor), mWildcard(false) {}
    Version(uint32_t major) : mMajor(major), mWildcard(false) {}
    Version() : mWildcard(true) {}
    Version(const Version &toCopy)
        : mMajor(toCopy.mMajor),
          mMinor(toCopy.mMinor),
          mSubminor(toCopy.mSubminor),
          mPatch(toCopy.mPatch),
          mWildcard(toCopy.mWildcard)
    {}
    ~Version() {}

    static Version *CreateVersionFromJson(const Json::Value &jObject);

    bool match(const Version &toCheck) const;
    std::string getString() const;

  public:
    IntegerPart mMajor;
    IntegerPart mMinor;
    IntegerPart mSubminor;
    IntegerPart mPatch;
    bool mWildcard;
};

// This encapsulates a list of other classes, each of which will have a match() and logItem()
// method.  The common constructor (given a type, but not any list items) assumes that this is
// a wildcard (i.e. will match all other ListOf<t> objects).
template <class T>
class ListOf
{
  public:
    ListOf(const std::string listType) : mWildcard(true), mListType(listType) {}
    ~ListOf() { mList.clear(); }
    void addItem(const T &toAdd);
    bool match(const T &toCheck) const;
    const T &front() const { return (mList.front()); }
    void logListOf(const std::string prefix, const std::string name) const;

    bool mWildcard;

  private:
    std::string mListType;
    std::vector<T> mList;
};

}  // namespace angle


#ifdef __cplusplus
extern "C" {
#endif


/**************************************************************************************************
 *
 * The following is the "version 1" private API for the Android EGL loader:
 *
 **************************************************************************************************/

// Callers of the ANGLE feature-support-utility API (e.g. the Android EGL loader) will call this
// function in order to determine what version of the API it can use (if any).
//
// The caller supplies the highest version of the API that it knows about.  If that version is
// supported by the feature-support-utility, true is returned and the version isn't changed.  If
// the supplied version is higher than supported by the feature-support-utility, true is
// returned and the version is changed to the highest supported by the feature-support-utility.
// If the supplied version is lower than supported by the feature-support-utility, false is
// returned.
//
// Parameters:
//
// - versionToUse (IN/OUT) - The application supplies the highest version of the interface that
//   it knows about.  If successful, the output value is either unchanged or is the highest
//   supported by the interface.
//
ANGLE_EXPORT bool ANGLEGetUtilityAPI(unsigned int *versionToUse);

// The Android EGL loader will call this function in order to determine whether
// to use ANGLE instead of a native OpenGL-ES (GLES) driver.
//
// Parameters:
// - rules_fd - File descriptor of the rules file to use
// - rules_offset - Offset into the fd before finding the contents of the rules file
// - rules_length - length of the rules file content
// - appName - Java name of the application (e.g. "com.google.android.apps.maps")
// - deviceMfr - Device manufacturer, from the "ro.product.manufacturer"com.google.android"
//   property
// - deviceModel - Device model, from the "ro.product.model"com.google.android" property
//
ANGLE_EXPORT bool AndroidUseANGLEForApplication(int rules_fd,
                                                long rules_offset,
                                                long rules_length,
                                                const char *appName,
                                                const char *deviceMfr,
                                                const char *deviceModel);

/**************************************************************************************************
 *
 * The following is the "version 2" private API for the Android EGL loader:
 *
 **************************************************************************************************/

// Typedefs for handles:
typedef void *RulesHandle;
typedef void *SystemInfoHandle;

// Callers of the ANGLE feature-support-utility API (e.g. the Android EGL loader) will call this
// function in order to determine what version of the API it can use (if any).
//
// The caller supplies the highest version of the API that it knows about.  If that version is
// supported by the feature-support-utility, true is returned and the version isn't changed.  If
// the supplied version is higher than supported by the feature-support-utility, true is
// returned and the version is changed to the highest supported by the feature-support-utility.
// If the supplied version is lower than supported by the feature-support-utility, false is
// returned.
//
// Parameters:
//
// - versionToUse (IN/OUT) - The application supplies the highest version of the interface that
//                           it knows about.  If successful, the output value is either
//                           unchanged or is the highest supported by the interface.
//
ANGLE_EXPORT bool ANGLEGetFeatureSupportUtilAPIVersion(unsigned int *versionToUse);

// The Android EGL loader will call this function in order to parse a rules file
// and create a set of rules, for which a handle is returned.
//
// Parameters:
// - rulesString   (IN) - Rules-file contents, as a non-zero length, null-terminated char*
//                        string
// - rulesHandle  (OUT) - Handle to the rules data structure
// - rulesVersion (OUT) - Version of the rules data structure (potentially because of schema
//                        changes) that should be passed to ANGLEShouldBeUsedForApplication()
//
// Return value:
// - bool - true if no errors, otherwise false
//
ANGLE_EXPORT bool ANGLEAndroidParseRulesString(const char *rulesString,
                                               RulesHandle *rulesHandle,
                                               int *rulesVersion);

// The Android EGL loader will call this function in order to obtain a handle to
// the SystemInfo struct.
//
// Parameters:
// - systemInfoHandle (OUT) - handle to the SystemInfo structure
//
// Return value:
// - bool - true if no errors, otherwise false
//
ANGLE_EXPORT bool ANGLEGetSystemInfo(SystemInfoHandle *systemInfoHandle);

// The Android EGL loader will call this function in order to add the device's manufacturer and
// model to the SystemInfo struct associated with the handle returned by ANGLEGetSystemInfo.
//
// Parameters:
// - deviceMfr   (IN) - Device manufacturer, from the
//                      "ro.product.manufacturer"com.google.android" property
// - deviceModel (IN) - Device model, from the "ro.product.model"com.google.android" property
// - systemInfoHandle (IN) - handle to the SystemInfo structure
//
// Return value:
// - bool - true if no errors, otherwise false
//
ANGLE_EXPORT bool ANGLEAddDeviceInfoToSystemInfo(const char *deviceMfr,
                                                 const char *deviceModel,
                                                 SystemInfoHandle systemInfoHandle);

// The Android EGL loader will call this function in order to determine whether
// to use ANGLE instead of a native OpenGL-ES (GLES) driver.
//
// Parameters:
// - rulesHandle      (IN) - Handle to the rules data structure
// - rulesVersion     (IN) - Version of the rules data structure (potentially because of schema
//                           changes) that was passed from AndroidParseRulesFile()
// - systemInfoHandle (IN) - Handle to the SystemInfo structure
// - appName          (IN) - Java name of the application (e.g. "com.google.android.apps.maps")
//
// Return value:
// - bool - true if Android should use ANGLE for appName, otherwise false (i.e. use the native
//          GLES driver)
//
ANGLE_EXPORT bool ANGLEShouldBeUsedForApplication(const RulesHandle rulesHandle,
                                                  int rulesVersion,
                                                  const SystemInfoHandle systemInfoHandle,
                                                  const char *appName);

// The Android EGL loader will call this function in order to free a rules handle.
//
// Parameters:
// - rulesHandle (IN) - Handle to the rules data structure
//
ANGLE_EXPORT void ANGLEFreeRulesHandle(const RulesHandle rulesHandle);

// The Android EGL loader will call this function in order to free a SystemInfo handle.
//
// Parameters:
// - systemInfoHandle (IN) - Handle to the SystemInfo structure
//
ANGLE_EXPORT void ANGLEFreeSystemInfoHandle(const SystemInfoHandle systemInfoHandle);

// The following are internal versions supported by the current  feature-support-utility API.

constexpr unsigned int kFeatureVersion_LowestSupported  = 1;
constexpr unsigned int kFeatureVersion_HighestSupported = 2;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // FEATURE_SUPPORT_UTIL_H_
