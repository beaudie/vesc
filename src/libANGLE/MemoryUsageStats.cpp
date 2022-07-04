//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/MemoryUsageStats.h"

#include "angle_mem_stats.h"

namespace
{
void CollectMemoryUsageStatsCommon(gl::MemoryUsageStats &memStats,
                                   ANGLECollectMemoryUsageStatsCallbackFunc callback,
                                   void *context)
{
    memStats.visitMemoryCategory(
        [context, callback](const std::string &category, size_t totalBytes,
                            const std::map<std::string, size_t> &extraProps) {
            std::vector<const char *> extraPropsKeys;
            std::vector<size_t> extraPropsValues;
            std::for_each(extraProps.begin(), extraProps.end(), [&](const auto &extraProp) {
                extraPropsKeys.push_back(extraProp.first.c_str());
                extraPropsValues.push_back(extraProp.second);
            });
            callback(context, category.c_str(), totalBytes, extraProps.size(),
                     extraPropsKeys.data(), extraPropsValues.data());
        });
}
}  // namespace

// API defined in include/angle_mem_stats.h
void ANGLECollectMemoryUsageStats(ANGLECollectMemoryUsageStatsCallbackFunc callback, void *context)
{
    CollectMemoryUsageStatsCommon(*gl::MemoryUsageStats::GetInstance(), callback, context);
}
void ANGLECollectBackendMemoryUsageStats(ANGLECollectMemoryUsageStatsCallbackFunc callback,
                                         void *context)
{
    CollectMemoryUsageStatsCommon(*gl::MemoryUsageStats::GetBackendInstance(), callback, context);
}

namespace gl
{
// static
MemoryUsageStats *MemoryUsageStats::GetInstance()
{
    static MemoryUsageStats *instance = new MemoryUsageStats();
    return instance;
}
MemoryUsageStats *MemoryUsageStats::GetBackendInstance()
{
    static MemoryUsageStats *instance = new MemoryUsageStats();
    return instance;
}

MemoryUsageStats::MemoryUsageStats() = default;

MemoryUsageStats::~MemoryUsageStats() = default;

void MemoryUsageStats::visitMemoryCategory(MemoryCategoryVisitFunc visitor)
{
    std::lock_guard<std::mutex> lg(mLock);

    for (const ReporterMap::value_type &reporterCategory : mReporterMap)
    {
        size_t totalBytes = 0;
        for (MemoryUsageReporter *reporter : reporterCategory.second)
        {
            totalBytes += reporter->getTotalMemorySize();
            reporter->dumpMemory(visitor);
        }

        visitor(reporterCategory.first, totalBytes, {});
    }
}

void MemoryUsageStats::registerMemoryUsageReporter(const std::string &category,
                                                   MemoryUsageReporter *reporter)
{
    std::lock_guard<std::mutex> lg(mLock);
    mReporterMap[category].insert(reporter);
}

void MemoryUsageStats::unregisterMemoryUsageReporter(const std::string &category,
                                                     MemoryUsageReporter *reporter)
{
    std::lock_guard<std::mutex> lg(mLock);
    mReporterMap[category].erase(reporter);
}
}  // namespace gl
