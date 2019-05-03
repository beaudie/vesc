//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Feature.h: Definition of structs to hold feature/workaround information.
//

#ifndef ANGLE_PLATFORM_FEATURE_H_
#define ANGLE_PLATFORM_FEATURE_H_

#include <map>
#include <string>
#include <vector>

namespace angle
{

enum FeatureCategory
{
    FRONTEND_WORKAROUNDS,
    OPENGL_WORKAROUNDS,
    D3D_WORKAROUNDS,
    D3D_COMPILER_WORKAROUNDS,
    VULKAN_WORKAROUNDS,
    VULKAN_FEATURES,
};

constexpr char kFeatureCategoryFrontendWorkarounds[]    = "Frontend workarounds";
constexpr char kFeatureCategoryOpenGLWorkarounds[]      = "OpenGL workarounds";
constexpr char kFeatureCategoryD3DWorkarounds[]         = "D3D workarounds";
constexpr char kFeatureCategoryD3DCompilerWorkarounds[] = "D3D compiler workarounds";
constexpr char kFeatureCategoryVulkanWorkarounds[]      = "Vulkan workarounds";
constexpr char kFeatureCategoryVulkanFeatures[]         = "Vulkan features";

inline const char *FeatureCategoryToString(const FeatureCategory &fc)
{
    switch (fc)
    {
        case FRONTEND_WORKAROUNDS:
            return kFeatureCategoryFrontendWorkarounds;
            break;

        case OPENGL_WORKAROUNDS:
            return kFeatureCategoryOpenGLWorkarounds;
            break;

        case D3D_WORKAROUNDS:
            return kFeatureCategoryD3DWorkarounds;
            break;

        case D3D_COMPILER_WORKAROUNDS:
            return kFeatureCategoryD3DCompilerWorkarounds;
            break;

        case VULKAN_WORKAROUNDS:
            return kFeatureCategoryVulkanWorkarounds;
            break;

        case VULKAN_FEATURES:
            return kFeatureCategoryVulkanFeatures;
            break;

        default:
            return "Unknown";
            break;
    }
}

struct Feature;

typedef std::map<std::string, Feature *> FeatureMap;
typedef std::vector<const Feature *> FeatureList;

struct Feature
{
    Feature(const Feature &other);
    Feature(const char *name,
            const FeatureCategory &category,
            const char *description,
            FeatureMap *const mapPtr,
            const char *bug);
    ~Feature();

    // The name of the workaround, lowercase, camel_case
    const char *name;

    // The category that the workaround belongs to. Eg. "Vulkan workarounds"
    const FeatureCategory category;

    // A short description to be read by the user.
    const char *description;

    // A link to the bug, if any
    const char *bug;

    // Whether the workaround is applied or not. Determined by heuristics like vendor ID and
    // version, but may be overriden to any value.
    bool applied = false;
};

inline Feature::Feature(const Feature &other) = default;
inline Feature::Feature(const char *name,
                        const FeatureCategory &category,
                        const char *description,
                        FeatureMap *const mapPtr,
                        const char *bug = "")
    : name(name), category(category), description(description), bug(bug), applied(false)
{
    if (mapPtr != nullptr)
    {
        (*mapPtr)[std::string(name)] = this;
    }
}

inline Feature::~Feature() = default;

struct FeaturesStruct
{
  public:
    FeaturesStruct();
    ~FeaturesStruct();
    // non-copyable
    FeaturesStruct(const FeaturesStruct &) = delete;
    void operator=(const FeaturesStruct &) = delete;

  protected:
    FeatureMap members = FeatureMap();

  public:
    void forceFeatureApplied(const std::string &name, const bool applied)
    {
        if (members.find(name) != members.end())
        {
            members[name]->applied = applied;
        }
    }

    void getFeatures(FeatureList &features)
    {
        for (FeatureMap::iterator it = members.begin(); it != members.end(); it++)
        {
            features.push_back(it->second);
        }
    }
};

inline FeaturesStruct::FeaturesStruct()  = default;
inline FeaturesStruct::~FeaturesStruct() = default;

}  // namespace angle

#endif  // ANGLE_PLATFORM_WORKAROUND_H_
