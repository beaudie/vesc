//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/MemoryUsageStats.h"

#include "angle_mem_stats.h"

// API defined in include/angle_mem_stats.h
void ANGLECollectMemoryUsageStats(ANGLECollectMemoryUsageStatsCallbackFunc callback, void *context)
{
    gl::MemoryUsageStats::GetInstance()->visitMemoryCategory(
        [context, callback](const std::string &category, size_t totalBytes) {
            callback(context, category.c_str(), totalBytes);
        });
}
void ANGLECollectBackendMemoryUsageStats(ANGLECollectMemoryUsageStatsCallbackFunc callback,
                                         void *context)
{
    gl::MemoryUsageStats::GetBackendInstance()->visitMemoryCategory(
        [context, callback](const std::string &category, size_t totalBytes) {
            callback(context, category.c_str(), totalBytes);
        });
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

        visitor(reporterCategory.first, totalBytes);
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
