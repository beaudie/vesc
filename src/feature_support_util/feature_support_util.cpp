//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// feature_support_util.cpp: Implementation of the code that helps the Android EGL loader
// determine whether to use ANGLE or a native GLES driver.

#include "feature_support_util.h"
#include <string.h>
#include <fstream>
#ifdef USE_WHEN_JSON_AVAILABLE
#include <json/json.h>
#endif  // USE_WHEN_JSON_AVAILABLE
#include <list>

//#if defined(ANGLE_PLATFORM_ANDROID)
#include <android/log.h>

#ifndef ALOGD
// FIXME/TODO: WRITE THE REAL CODE!!!
#define ALOGD(...) printf(__VA_ARGS__);
//#define ALOGD(...) ((void)ALOG(LOG_DEBUG, LOG_TAG, __VA_ARGS__))
//   From src/common/debug.cpp ...
//__android_log_print((severity == LOG_ERR) ? ANDROID_LOG_ERROR : ANDROID_LOG_WARN, "ANGLE",
//                            "%s: %s\n", LogSeverityName(severity), str.c_str());
#endif  // ALOGD
//#endif // defined(ANGLE_PLATFORM_ANDROID)

#ifdef USE_WHEN_JSON_AVAILABLE
// JSON values are generally composed of either:
//  - Objects, which are a set of comma-separated string:value pairs (note the recursive nature)
//  - Arrays, which are a set of comma-separated values.
// We'll call the string in a string:value pair the "identifier".  These identifiers are defined
// below, as follows:

// The JSON identifier for the top-level set of rules.  This is an object, the value of which is an
// array of rules.  The rules will be processed in order.  For any given type of answer, if a rule
// matches, the rule's version of the answer (true or false) becomes the new answer.  After all
// rules are processed, the most-recent answer is the final answer.
constexpr char kJson_Rules[] = "Rules";
// The JSON identifier for a given rule.  A rule is an object, the first string:value pair is this
// identifier (i.e. "Rule") as the string and a user-firendly description of the rule:
constexpr char kJson_Rule[] = "Rule";
// Within a rule, the JSON identifier for one type of answer--whether to allow an application to
// specify whether to use ANGLE.  The value is a boolean (i.e. true or false), with true allowing
// the application to specify whether or not to use ANGLE.
constexpr char kJson_AppChoice[] = "AppChoice";
// Within a rule, the JSON identifier for one type of answer--whether or not to use ANGLE when an
// application doesn't specify (or isn't allowed to specify) whether or not to use ANGLE.  The
// value is a boolean (i.e. true or false).
constexpr char kJson_NonChoice[] = "NonChoice";

// Within a rule, the JSON identifier for describing one or more applications.  The value is an
// array of objects, each object of which can specify attributes of an application.
constexpr char kJson_Applications[] = "Applications";
// Within an object that describes the attributes of an application, the JSON identifier for the
// name of the application (e.g. "com.google.maps").  The value is a string.  If any other
// attributes will be specified, this must be the first attribute specified in the object.
constexpr char kJson_AppName[] = "AppName";
// Within an object that describes the attributes of an application, the JSON identifier for the
// intent of the application to run.  The value is a string.
constexpr char kJson_Intent[] = "Intent";

// Within a rule, the JSON identifier for describing one or more devices.  The value is an
// array of objects, each object of which can specify attributes of a device.
constexpr char kJson_Devices[] = "Devices";
// Within an object that describes the attributes of a device, the JSON identifier for the
// manufacturer of the device.  The value is a string.  If any other attributes will be specified,
// this must be the first attribute specified in the object.
constexpr char kJson_Manufacturer[] = "Manufacturer";
// Within an object that describes the attributes of a device, the JSON identifier for the
// model of the device.  The value is a string.
constexpr char kJson_Model[] = "Model";

// Within an object that describes the attributes of a device, the JSON identifier for describing
// one or more GPUs/drivers used in the device.  The value is an
// array of objects, each object of which can specify attributes of a GPU and its driver.
constexpr char kJson_GPUs[] = "GPUs";
// Within an object that describes the attributes of a GPU and driver, the JSON identifier for the
// vendor of the device/driver.  The value is a string.  If any other attributes will be specified,
// this must be the first attribute specified in the object.
constexpr char kJson_vendor[] = "vendor";
// Within an object that describes the attributes of a GPU and driver, the JSON identifier for the
// deviceId of the device.  The value is an unsigned integer.  If the driver version will be
// specified, this must preceded the version attributes specified in the object.
constexpr char kJson_deviceId[] = "deviceId";

// Within an object that describes the attributes of either an application or a GPU, the JSON
// identifier for the major version of that application or GPU driver.  The value is a positive
// integer number.  Not specifying a major version implies a wildcard for all values of a version.
constexpr char kJson_VerMajor[] = "VerMajor";
// Within an object that describes the attributes of either an application or a GPU, the JSON
// identifier for the minor version of that application or GPU driver.  The value is a positive
// integer number.  In order to specify a minor version, it must be specified immediately after the
// major number associated with it.  Not specifying a minor version implies a wildcard for the
// minor, subminor, and patch values of a version.
constexpr char kJson_VerMinor[] = "VerMinor";
// Within an object that describes the attributes of either an application or a GPU, the JSON
// identifier for the subminor version of that application or GPU driver.  The value is a positive
// integer number.  In order to specify a subminor version, it must be specified immediately after
// the minor number associated with it.  Not specifying a subminor version implies a wildcard for
// the subminor and patch values of a version.
constexpr char kJson_VerSubMinor[] = "VerSubMinor";
// Within an object that describes the attributes of either an application or a GPU, the JSON
// identifier for the patch version of that application or GPU driver.  The value is a positive
// integer number.  In order to specify a patch version, it must be specified immediately after the
// subminor number associated with it.  Not specifying a patch version implies a wildcard for the
// patch value of a version.
constexpr char kJson_VerPatch[] = "VerPatch";
#endif  // USE_WHEN_JSON_AVAILABLE

// This encapsulates a std::string.  The default constructor (not given a string) assumes that this
// is a wildcard (i.e. will match all other StringPart objects).
class StringPart
{
  public:
    StringPart() : mPart(""), mWildcard(true) {}
    StringPart(std::string part) : mPart(part), mWildcard(false) {}
    ~StringPart() {}
    bool match(StringPart &toCheck)
    {
        return (mWildcard || toCheck.mWildcard || (toCheck.mPart == mPart));
    }

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
    bool match(IntegerPart &toCheck)
    {
        return (mWildcard || toCheck.mWildcard || (toCheck.mPart == mPart));
    }

  public:
    uint32_t mPart;
    bool mWildcard;
};

// This encapsulates a list of other classes, which of which will have a match() method.  The
// common constructor (given a type, but not any list items) assumes that this is a wildcard
// (i.e. will match all other ListOf<t> objects).
template <class T>
class ListOf
{
  public:
    ListOf(std::string listType) : mListType(listType), mWildcard(true) {}
    void addItem(T &toAdd)
    {
        mList.push_back(toAdd);
        mWildcard = false;
    }
    bool match(T &toCheck)
    {
        ALOGD("\t\t Within ListOf<%s> match: wildcards are %s and %s,\n", mListType.c_str(),
              mWildcard ? "true" : "false", toCheck.mWildcard ? "true" : "false");
        if (mWildcard || toCheck.mWildcard)
        {
            return true;
        }
        for (auto &it : mList)
        {
            ALOGD("\t\t   Within ListOf<%s> match: calling match on sub-item is %s,\n",
                  mListType.c_str(), it.match(toCheck) ? "true" : "false");
            if (it.match(toCheck))
            {
                return true;
            }
        }
        return false;
    }
    T &front() { return (mList.front()); }
    void logListOf(std::string prefix, std::string name)
    {
        if (mWildcard)
        {
            ALOGD("%sListOf%s is wildcarded to always match", prefix.c_str(), name.c_str());
        }
        else
        {
            int nItems = mList.size();
            ALOGD("%sListOf%s is has %d item(s):", prefix.c_str(), name.c_str(), nItems);
            for (auto &it : mList)
            {
                it.logItem();
            }
        }
    }

  private:
    std::string mListType;
    std::list<T> mList;

  public:
    bool mWildcard = true;
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
    {
    }
    Version(uint32_t major, uint32_t minor, uint32_t subminor)
        : mMajor(major), mMinor(minor), mSubminor(subminor), mWildcard(false)
    {
    }
    Version(uint32_t major, uint32_t minor) : mMajor(major), mMinor(minor), mWildcard(false) {}
    Version(uint32_t major) : mMajor(major), mWildcard(false) {}
    Version() : mWildcard(true) {}
    Version(const Version &toCopy)
        : mMajor(toCopy.mMajor),
          mMinor(toCopy.mMinor),
          mSubminor(toCopy.mSubminor),
          mPatch(toCopy.mPatch),
          mWildcard(toCopy.mWildcard)
    {
    }
    bool match(Version &toCheck)
    {
        ALOGD("\t\t\t Within Version %d,%d,%d,%d match(%d,%d,%d,%d): wildcards are %s and %s,\n",
              mMajor.mPart, mMinor.mPart, mSubminor.mPart, mPatch.mPart, toCheck.mMajor.mPart,
              toCheck.mMinor.mPart, toCheck.mSubminor.mPart, toCheck.mPatch.mPart,
              mWildcard ? "true" : "false", toCheck.mWildcard ? "true" : "false");
        if (!(mWildcard || toCheck.mWildcard))
        {
            ALOGD("\t\t\t   mMajor match is %s, mMinor is %s, mSubminor is %s, mPatch is %s\n",
                  mMajor.match(toCheck.mMajor) ? "true" : "false",
                  mMinor.match(toCheck.mMinor) ? "true" : "false",
                  mSubminor.match(toCheck.mSubminor) ? "true" : "false",
                  mPatch.match(toCheck.mPatch) ? "true" : "false");
        }
        return (mWildcard || toCheck.mWildcard ||
                (mMajor.match(toCheck.mMajor) && mMinor.match(toCheck.mMinor) &&
                 mSubminor.match(toCheck.mSubminor) && mPatch.match(toCheck.mPatch)));
    }
#ifdef USE_WHEN_JSON_AVAILABLE
    static Version *createVersionFromJson(Json::Value &jObject)
    {
        Version *version = nullptr;
        // A major version must be provided before a minor, and so on:
        if (jObject.isMember(kJson_VerMajor) && jObject[kJson_VerMajor].isInt())
        {
            int major = jObject[kJson_VerMajor].asInt();
            if (jObject.isMember(kJson_VerMinor) && jObject[kJson_VerMinor].isInt())
            {
                int minor = jObject[kJson_VerMinor].asInt();
                if (jObject.isMember(kJson_VerSubMinor) && jObject[kJson_VerSubMinor].isInt())
                {
                    int subMinor = jObject[kJson_VerSubMinor].asInt();
                    if (jObject.isMember(kJson_VerPatch) && jObject[kJson_VerPatch].isInt())
                    {
                        int patch = jObject[kJson_VerPatch].asInt();
                        version   = new Version(major, minor, subMinor, patch);
                    }
                    else
                    {
                        version = new Version(major, minor, subMinor);
                    }
                }
                else
                {
                    version = new Version(major, minor);
                }
            }
            else
            {
                version = new Version(major);
            }
        }
        // TODO (ianelliott@) (b/113346561) appropriately destruct lists and
        // other items that get created from json parsing
        return version;
    }
#endif  // USE_WHEN_JSON_AVAILABLE
    std::string getString()
    {
        std::string ret;
        if (mWildcard)
        {
            return "*";
        }
        else
        {
            char ret[100];
            // Must at least have a major version:
            if (!mMinor.mWildcard)
            {
                if (!mSubminor.mWildcard)
                {
                    if (!mPatch.mWildcard)
                    {
                        snprintf(ret, 100, "%d.%d.%d.%d", mMajor.mPart, mMinor.mPart,
                                 mSubminor.mPart, mPatch.mPart);
                    }
                    else
                    {
                        snprintf(ret, 100, "%d.%d.%d.*", mMajor.mPart, mMinor.mPart,
                                 mSubminor.mPart);
                    }
                }
                else
                {
                    snprintf(ret, 100, "%d.%d.*", mMajor.mPart, mMinor.mPart);
                }
            }
            else
            {
                snprintf(ret, 100, "%d.*", mMajor.mPart);
            }
            std::string retString = ret;
            return retString;
        }
    }

  public:
    IntegerPart mMajor;
    IntegerPart mMinor;
    IntegerPart mSubminor;
    IntegerPart mPatch;
    bool mWildcard;
};

// This encapsulates an application, and potentially the application's Version and/or the intent
// that it is launched with.  The default constructor (not given any values) assumes that this is a
// wildcard (i.e. will match all other Application objects).  Each part of an Application is stored
// in a class that may also be wildcarded.
class Application
{
  public:
    Application(std::string name, Version &version, std::string intent)
        : mName(name), mVersion(version), mIntent(intent), mWildcard(false)
    {
    }
    Application(std::string name, std::string intent)
        : mName(name), mVersion(), mIntent(intent), mWildcard(false)
    {
    }
    Application(std::string name, Version &version)
        : mName(name), mVersion(version), mIntent(), mWildcard(false)
    {
    }
    Application(std::string name) : mName(name), mVersion(), mIntent(), mWildcard(false) {}
    Application() : mName(), mVersion(), mIntent(), mWildcard(true) {}
    bool match(Application &toCheck)
    {
        return (mWildcard || toCheck.mWildcard ||
                (toCheck.mName.match(mName) && toCheck.mVersion.match(mVersion) &&
                 toCheck.mIntent.match(mIntent)));
    }
#ifdef USE_WHEN_JSON_AVAILABLE
    static Application *createApplicationFromJson(Json::Value &jObject)
    {
        Application *application = nullptr;

        // If an application is listed, the application's name is required:
        std::string appName = jObject[kJson_AppName].asString();

        // The application's version and intent are optional:
        Version *version = Version::createVersionFromJson(jObject);
        if (version)
        {
            if (jObject.isMember(kJson_Intent) && jObject[kJson_Intent].isString())
            {
                application = new Application(appName, *version, jObject[kJson_Intent].asString());
            }
            else
            {
                application = new Application(appName, *version);
            }
        }
        else
        {
            if (jObject.isMember(kJson_Intent) && jObject[kJson_Intent].isString())
            {
                application = new Application(appName, jObject[kJson_Intent].asString());
            }
            else
            {
                application = new Application(appName);
            }
        }
        // TODO (ianelliott@) (b/113346561) appropriately destruct lists and
        // other items that get created from json parsing
        return application;
    }
#endif  // USE_WHEN_JSON_AVAILABLE
    void logItem()
    {
        if (mWildcard)
        {
            ALOGD("      Wildcard (i.e. will match all applications)");
        }
        else if (!mVersion.mWildcard)
        {
            if (!mIntent.mWildcard)
            {
                ALOGD("      Application \"%s\" (version: %s; intent: \"%s\")", mName.mPart.c_str(),
                      mVersion.getString().c_str(), mIntent.mPart.c_str());
            }
            else
            {
                ALOGD("      Application \"%s\" (version: %s)", mName.mPart.c_str(),
                      mVersion.getString().c_str());
            }
        }
        else if (!mIntent.mWildcard)
        {
            ALOGD("      Application \"%s\" (intent: \"%s\")", mName.mPart.c_str(),
                  mIntent.mPart.c_str());
        }
        else
        {
            ALOGD("      Application \"%s\"", mName.mPart.c_str());
        }
    }

  public:
    StringPart mName;
    Version mVersion;
    StringPart mIntent;
    bool mWildcard;
};

// This encapsulates a GPU and its driver.  The default constructor (not given any values) assumes
// that this is a wildcard (i.e. will match all other GPU objects).  Each part of a GPU is stored
// in a class that may also be wildcarded.
class GPU
{
  public:
    GPU(std::string vendor, uint32_t deviceId, Version &version)
        : mVendor(vendor), mDeviceId(IntegerPart(deviceId)), mVersion(version), mWildcard(false)
    {
    }
    GPU(uint32_t deviceId, Version &version)
        : mVendor(), mDeviceId(IntegerPart(deviceId)), mVersion(version), mWildcard(false)
    {
    }
    GPU(std::string vendor, uint32_t deviceId)
        : mVendor(vendor), mDeviceId(IntegerPart(deviceId)), mVersion(), mWildcard(false)
    {
    }
    GPU(std::string vendor) : mVendor(vendor), mDeviceId(), mVersion(), mWildcard(false) {}
    GPU(uint32_t deviceId)
        : mVendor(), mDeviceId(IntegerPart(deviceId)), mVersion(), mWildcard(false)
    {
    }
    GPU() : mVendor(), mDeviceId(), mVersion(), mWildcard(true) {}
    bool match(GPU &toCheck)
    {
        ALOGD("\t\t Within GPU match: wildcards are %s and %s,\n", mWildcard ? "true" : "false",
              toCheck.mWildcard ? "true" : "false");
        ALOGD("\t\t   mVendor = \"%s\" and toCheck.mVendor = \"%s\"\n", mVendor.mPart.c_str(),
              toCheck.mVendor.mPart.c_str());
        ALOGD("\t\t   mDeviceId = %d and toCheck.mDeviceId = %d\n", mDeviceId.mPart,
              toCheck.mDeviceId.mPart);
        ALOGD("\t\t   mVendor match is %s, mDeviceId is %s, mVersion is %s\n",
              toCheck.mVendor.match(mVendor) ? "true" : "false",
              toCheck.mDeviceId.match(mDeviceId) ? "true" : "false",
              toCheck.mVersion.match(mVersion) ? "true" : "false");
        return (mWildcard || toCheck.mWildcard ||
                (toCheck.mVendor.match(mVendor) && toCheck.mDeviceId.match(mDeviceId) &&
                 toCheck.mVersion.match(mVersion)));
    }
#ifdef USE_WHEN_JSON_AVAILABLE
    static GPU *createGpuFromJson(Json::Value &jObject)
    {
        GPU *gpu = nullptr;

        // If a GPU is listed, the vendor name is required:
        if (jObject.isMember(kJson_vendor) && jObject[kJson_vendor].isString())
        {
            std::string vendor = jObject[kJson_vendor].asString();
            // If a version is given, the deviceId is required:
            if (jObject.isMember(kJson_deviceId) && jObject[kJson_deviceId].isUInt())
            {
                uint32_t deviceId = jObject[kJson_deviceId].asUInt();
                Version *version  = Version::createVersionFromJson(jObject);
                if (version)
                {
                    gpu = new GPU(vendor, deviceId, *version);
                }
                else
                {
                    gpu = new GPU(vendor, deviceId);
                }
            }
            else
            {
                gpu = new GPU(vendor);
            }
        }
        else
        {
            ALOGD("Asked to parse a GPU, but no GPU found");
        }

        // TODO (ianelliott@) (b/113346561) appropriately destruct lists and
        // other items that get created from json parsing
        return gpu;
    }
#endif  // USE_WHEN_JSON_AVAILABLE
    void logItem()
    {
        if (mWildcard)
        {
            ALOGD("          Wildcard (i.e. will match all GPUs)");
        }
        else
        {
            if (!mDeviceId.mWildcard)
            {
                if (!mVersion.mWildcard)
                {
                    ALOGD("\t     GPU vendor: %s, deviceId: 0x%x, version: %s",
                          mVendor.mPart.c_str(), mDeviceId.mPart, mVersion.getString().c_str());
                }
                else
                {
                    ALOGD("\t     GPU vendor: %s, deviceId: 0x%x", mVendor.mPart.c_str(),
                          mDeviceId.mPart);
                }
            }
            else
            {
                ALOGD("\t     GPU vendor: %s", mVendor.mPart.c_str());
            }
        }
    }

  public:
    StringPart mVendor;
    IntegerPart mDeviceId;
    Version mVersion;
    bool mWildcard;
};

// This encapsulates a device, and potentially the device's model and/or a list of GPUs/drivers
// associated with the Device.  The default constructor (not given any values) assumes that this is
// a wildcard (i.e. will match all other Device objects).  Each part of a Device is stored in a
// class that may also be wildcarded.
class Device
{
  public:
    Device(std::string manufacturer, std::string model)
        : mManufacturer(manufacturer), mModel(model), mGpuList("GPU"), mWildcard(false)
    {
    }
    Device(std::string manufacturer)
        : mManufacturer(manufacturer), mModel(), mGpuList("GPU"), mWildcard(false)
    {
    }
    Device() : mManufacturer(), mModel(), mGpuList("GPU"), mWildcard(true) {}
    void AddGPU(GPU &gpu) { mGpuList.addItem(gpu); }
    bool match(Device &toCheck)
    {
        ALOGD("\t Within Device match: wildcards are %s and %s,\n", mWildcard ? "true" : "false",
              toCheck.mWildcard ? "true" : "false");
        if (!(mWildcard || toCheck.mWildcard))
        {
            ALOGD("\t   Manufacturer match is %s, model is %s\n",
                  toCheck.mManufacturer.match(mManufacturer) ? "true" : "false",
                  toCheck.mModel.match(mModel) ? "true" : "false");
        }
        ALOGD("\t   Need to check ListOf<GPU>\n");
        return ((mWildcard || toCheck.mWildcard ||
                 // The wildcards can override the Manufacturer/Model check, but not the GPU check
                 (toCheck.mManufacturer.match(mManufacturer) && toCheck.mModel.match(mModel))) &&
                // Note: toCheck.mGpuList is for the device and must contain exactly one item,
                // where mGpuList may contain zero or more items:
                mGpuList.match(toCheck.mGpuList.front()));
    }
#ifdef USE_WHEN_JSON_AVAILABLE
    static Device *createDeviceFromJson(Json::Value &jObject)
    {
        Device *device = nullptr;
        if (jObject.isMember(kJson_Manufacturer) && jObject[kJson_Manufacturer].isString())
        {
            std::string manufacturerName = jObject[kJson_Manufacturer].asString();
            // We don't let a model be specified without also specifying an Manufacturer:
            if (jObject.isMember(kJson_Model) && jObject[kJson_Model].isString())
            {
                std::string model = jObject[kJson_Model].asString();
                device            = new Device(manufacturerName, model);
            }
            else
            {
                device = new Device(manufacturerName);
            }
        }
        else
        {
            // This case is not treated as an error because a rule may wish to only call out one or
            // more GPUs, and not any specific Manufacturer devices:
            device = new Device();
        }
        // TODO (ianelliott@) (b/113346561) appropriately destruct lists and
        // other items that get created from json parsing
        return device;
    }
#endif  // USE_WHEN_JSON_AVAILABLE
    void logItem()
    {
        if (mWildcard)
        {
            if (mGpuList.mWildcard)
            {
                ALOGD("      Wildcard (i.e. will match all devices)");
                return;
            }
            else
            {
                ALOGD(
                    "      Device with any manufacturer and model"
                    ", and with the following GPUs:");
            }
        }
        else
        {
            if (!mModel.mWildcard)
            {
                ALOGD(
                    "      Device manufacturer: \"%s\" and model \"%s\""
                    ", and with the following GPUs:",
                    mManufacturer.mPart.c_str(), mModel.mPart.c_str());
            }
            else
            {
                ALOGD(
                    "      Device manufacturer: \"%s\""
                    ", and with the following GPUs:",
                    mManufacturer.mPart.c_str());
            }
        }
        mGpuList.logListOf("        ", "GPUs");
    }

  public:
    StringPart mManufacturer;
    StringPart mModel;
    ListOf<GPU> mGpuList;
    bool mWildcard;
};

// This encapsulates a particular scenario to check against the rules.  A Scenario is similar to a
// Rule, except that a Rule has answers and potentially many wildcards, and a Scenario is the
// fully-specified combination of an Application and a Device that is proposed to be run with
// ANGLE.  It is compared with the list of Rules.
class Scenario
{
  public:
    Scenario(const char *appName, const char *deviceMfr, const char *deviceModel)
        : mApplication(Application(appName)), mDevice(Device(deviceMfr, deviceModel))
    {
    }
    void logScenario()
    {
        ALOGD("  Scenario to compare against the rules");
        ALOGD("    Application:");
        mApplication.logItem();
        ALOGD("    Device:");
        mDevice.logItem();
    }

  public:
    Application mApplication;
    Device mDevice;

  private:
    Scenario(Application &app, Device &dev) : mApplication(app), mDevice(dev) {}
    Scenario() : mApplication(), mDevice() {}
};

// This encapsulates a Rule that provides answers based on whether a particular Scenario matches
// the Rule.  A Rule always has answers, but can potentially wildcard every item in it (i.e. match
// every scenario).
class Rule
{
  public:
    Rule(std::string description, bool appChoice, bool answer)
        : mDescription(description),
          mAppList("Application"),
          mDevList("Device"),
          mAppChoice(appChoice),
          mAnswer(answer)
    {
    }
    void AddApp(Application &app) { mAppList.addItem(app); }
    void AddDev(Device &dev) { mDevList.addItem(dev); }
    bool match(Scenario &toCheck)
    {
        ALOGD("    Within \"%s\" Rule: application match is %s and device match is %s\n",
              mDescription.c_str(), mAppList.match(toCheck.mApplication) ? "true" : "false",
              mDevList.match(toCheck.mDevice) ? "true" : "false");
        return (mAppList.match(toCheck.mApplication) && mDevList.match(toCheck.mDevice));
    }
    bool getAppChoice() { return mAppChoice; }
    bool getAnswer() { return mAnswer; }
    void logRule()
    {
        ALOGD("  Rule: \"%s\" %s ANGLE, and %s the app a choice if matched", mDescription.c_str(),
              mAnswer ? "enables" : "disables", mAppChoice ? "does give" : "does NOT give");
        mAppList.logListOf("    ", "Applications");
        mDevList.logListOf("    ", "Devices");
    }

  public:
    std::string mDescription;
    ListOf<Application> mAppList;
    ListOf<Device> mDevList;
    bool mAppChoice;
    bool mAnswer;

  private:
    Rule()
        : mDescription(),
          mAppList("Application"),
          mDevList("Device"),
          mAppChoice(false),
          mAnswer(false)
    {
    }
};

// This encapsulates a list of Rules that Scenarios are matched against.  A Scenario is compared
// with each Rule, in order.  Any time a Scenario matches a Rule, the current answer is overridden
// with the answer of the matched Rule.
class RuleList
{
  public:
    RuleList() {}
    void AddRule(Rule &rule) { mRuleList.push_back(rule); }
    bool getAppChoice(Scenario &toCheck)
    {
        bool appChoice = false;
        int nRules     = mRuleList.size();
        ALOGD("Checking scenario against %d ANGLE-for-Android rules:", nRules);

        for (auto &it : mRuleList)
        {
            ALOGD("  Checking Rule: \"%s\" (to see whether there's a match)",
                  it.mDescription.c_str());
            if (it.match(toCheck))
            {
                ALOGD("  -> Rule matches.  Setting the app choice to %s",
                      it.getAppChoice() ? "true" : "false");
                appChoice = it.getAppChoice();
            }
            else
            {
                ALOGD("  -> Rule doesn't match.");
            }
        }

        return appChoice;
    }
    bool getAnswer(Scenario &toCheck)
    {
        bool answer = false;
        int nRules  = mRuleList.size();
        ALOGD("Checking scenario against %d ANGLE-for-Android rules:", nRules);

        for (auto &it : mRuleList)
        {
            ALOGD("  Checking Rule: \"%s\" (to see whether there's a match)",
                  it.mDescription.c_str());
            if (it.match(toCheck))
            {
                ALOGD("  -> Rule matches.  Setting the answer to %s",
                      it.getAnswer() ? "true" : "false");
                answer = it.getAnswer();
            }
            else
            {
                ALOGD("  -> Rule doesn't match.");
            }
        }

        return answer;
    }
    void logRules()
    {
        int nRules = mRuleList.size();
        ALOGD("Showing %d ANGLE-for-Android rules:", nRules);
        for (auto &it : mRuleList)
        {
            it.logRule();
        }
    }

  public:
    std::list<Rule> mRuleList;
};

#ifdef USE_WHEN_JSON_AVAILABLE
RuleList *read_rules_from_json_file()
{
    RuleList *rules = new RuleList;

    // Open the file and start parsing it:
    using namespace std;
    // FIXME/TODO: NEED TO GET THE FINAL LOCATION AND ENSURE THAT ANY APPLICATION CAN READ FROM
    // THAT LOCATION.
    ifstream ifs("/storage/self/primary/angle_opt_in_out.json");
    Json::Reader jReader;
    Json::Value jTopLevelObject;
    jReader.parse(ifs, jTopLevelObject);
    Json::Value jRules = jTopLevelObject[kJson_Rules];
    for (unsigned int i = 0; i < jRules.size(); i++)
    {
        Json::Value jRule           = jRules[i];
        std::string ruleDescription = jRule[kJson_Rule].asString();
        bool ruleAppChoice          = jRule[kJson_AppChoice].asBool();
        bool ruleAnswer             = jRule[kJson_NonChoice].asBool();
        // TODO (ianelliott@) (b/113346561) appropriately destruct lists and
        // other items that get created from json parsing
        Rule *newRule = new Rule(ruleDescription, ruleAppChoice, ruleAnswer);

        Json::Value jApps = jRule[kJson_Applications];
        for (unsigned int i = 0; i < jApps.size(); i++)
        {
            Json::Value jApp    = jApps[i];
            Application *newApp = Application::createApplicationFromJson(jApp);
            // TODO (ianelliott@) (b/113346561) appropriately destruct lists and
            // other items that get created from json parsing
            newRule->AddApp(*newApp);
        }

        Json::Value jDevs = jRule[kJson_Devices];
        for (unsigned int i = 0; i < jDevs.size(); i++)
        {
            Json::Value jDev = jDevs[i];
            Device *newDev   = Device::createDeviceFromJson(jDev);

            Json::Value jGPUs = jDev["GPUs"];
            for (unsigned int i = 0; i < jGPUs.size(); i++)
            {
                Json::Value jGPU = jGPUs[i];
                GPU *newGPU      = GPU::createGpuFromJson(jGPU);
                if (newGPU)
                {
                    newDev->AddGPU(*newGPU);
                }
            }
            newRule->AddDev(*newDev);
        }

        // TODO: Need to manage memory
        rules->AddRule(*newRule);
    }
    return rules;
}
#endif  // USE_WHEN_JSON_AVAILABLE

void destroy_rules(RuleList *rules)
{
    delete rules;
}

extern "C" {

ANGLE_EXPORT bool ANGLEUseForApplication(const char *appName,
                                         const char *deviceMfr,
                                         const char *deviceModel,
                                         ANGLEPreference developerOption,
                                         ANGLEPreference appPreference)
{
    Scenario scenario(appName, deviceMfr, deviceModel);
    RuleList rules;
    // TODO: Replace these in-code rules with one that come from JSON:
    Rule defaultRule("Default Rule", true, false);
    rules.AddRule(defaultRule);
    Rule mapsRule("Allow Google Maps to run", true, true);
    Application mapsApp("com.google.android.apps.maps");
    mapsRule.AddApp(mapsApp);
    rules.AddRule(mapsRule);

    if (developerOption != ANGLE_NO_PREFERENCE)
    {
        return (developerOption == ANGLE_PREFER_ANGLE);
    }
    else if ((appPreference != ANGLE_NO_PREFERENCE) && rules.getAppChoice(scenario))
    {
        return (appPreference == ANGLE_PREFER_ANGLE);
    }
    else
    {
        return rules.getAnswer(scenario);
    }
}

}  // extern "C"
