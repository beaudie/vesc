//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_MEMORYUSAGESTATS_H_
#define LIBANGLE_MEMORYUSAGESTATS_H_

#include <functional>
#include <map>
#include <mutex>

#include "libANGLE/angletypes.h"

namespace gl
{

using MemoryCategoryVisitFunc =
    std::function<void(const std::string &, size_t, const std::map<std::string, size_t> &)>;

class MemoryUsageReporter
{
  public:
    virtual ~MemoryUsageReporter()      = default;
    virtual size_t getTotalMemorySize() = 0;
    virtual void dumpMemory(MemoryCategoryVisitFunc callback) {}
};

class GL_API MemoryUsageStats
{
  public:
    // Instance to collect memory usage related to OpenGL objects
    static MemoryUsageStats *GetInstance();
    // Instance to collect memory usage related to backend specific
    static MemoryUsageStats *GetBackendInstance();

    MemoryUsageStats(const MemoryUsageStats &)            = delete;
    MemoryUsageStats &operator=(const MemoryUsageStats &) = delete;

    void visitMemoryCategory(MemoryCategoryVisitFunc visitor);

    void registerMemoryUsageReporter(const std::string &category, MemoryUsageReporter *reporter);
    void unregisterMemoryUsageReporter(const std::string &category, MemoryUsageReporter *reporter);

  private:
    MemoryUsageStats();
    ~MemoryUsageStats();

    std::mutex mLock;
    using ReporterMap = angle::HashMap<std::string, angle::HashSet<MemoryUsageReporter *>>;
    ReporterMap mReporterMap;
};

}  // namespace gl

#endif
