//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// system_utils_linux.cpp: Implementation of OS-specific functions for Linux

#include "system_utils.h"

#include "string_utils.h"
#include "utilities.h"

#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>

namespace angle
{
std::string GetExecutablePath()
{
    // We cannot use lstat to get the size of /proc/self/exe as it always returns 0
    // so we just use a big buffer and hope the path fits in it.
    char path[4096];

    ssize_t result = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (result < 0 || static_cast<size_t>(result) >= sizeof(path) - 1)
    {
        return "";
    }

    path[result] = '\0';
    return path;
}

std::string GetExecutableDirectory()
{
    std::string executablePath = GetExecutablePath();
    size_t lastPathSepLoc      = executablePath.find_last_of("/");
    return (lastPathSepLoc != std::string::npos) ? executablePath.substr(0, lastPathSepLoc) : "";
}

const char *GetSharedLibraryExtension()
{
    return "so";
}

double GetCurrentSystemTime()
{
    struct timespec currentTime;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    return currentTime.tv_sec + currentTime.tv_nsec * 1e-9;
}

// Adapted from: https://github.com/fcarucci/peuck
void SetThreadAffinity()
{
    struct Cpu
    {
        struct Core
        {
            int id;
            int frequency;
            int package_id;
        };

        std::vector<Core> cores;
        std::string topology;
    };

    constexpr size_t kBufferSize = 10240;  // 10KB
    Cpu cpu;

    char buf[kBufferSize];
    FILE *fp = fopen("/proc/cpuinfo", "r");

    if (fp)
    {
        std::stringstream ss;
        while (fgets(buf, kBufferSize, fp) != nullptr)
        {
            buf[strlen(buf) - 1] = '\0';  // eat the newline fgets() stores
            std::string line     = buf;

            if (BeginsWith(line, "processor"))
            {
                ss.str("");
                ss << "/sys/devices/system/cpu/cpu" << cpu.cores.size()
                   << "/cpufreq/cpuinfo_max_freq";
                char frequency[kBufferSize] = {'\0'};
                readFile(ss.str().c_str(), frequency, kBufferSize);

                ss.str("");
                char package_id[kBufferSize] = {'\0'};
                ss << "/sys/devices/system/cpu/cpu" << cpu.cores.size()
                   << "/topology/physical_package_id";
                readFile(ss.str().c_str(), package_id, kBufferSize);

                Cpu::Core core;
                core.id         = static_cast<int>(cpu.cores.size());
                core.frequency  = std::atoi(frequency);
                core.package_id = std::atoi(package_id);

                cpu.cores.push_back(core);
            }
        }
        fclose(fp);

        ss.str("");
        for (const auto &core : cpu.cores)
        {
            ss << core.id << " " << core.frequency << " " << core.package_id << std::endl;
        }
        cpu.topology = ss.str();
    }

    // Sort the CPUs by frequency
    std::sort(cpu.cores.begin(), cpu.cores.end(),
              [](Cpu::Core a, Cpu::Core b) -> bool { return a.frequency < b.frequency; });

    // There's no need to set CPU affinity if every core is the same.
    if (cpu.cores.front().frequency == cpu.cores.back().frequency)
    {
        return;
    }

    // Set the thread affinity to all the CPUs with the highest frequency (mid+big cores).
    pid_t tid = gettid();
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);

    int highestFreq = cpu.cores.back().frequency;
    for (const Cpu::Core core : cpu.cores)
    {
        if (core.frequency == highestFreq)
        {
            CPU_SET(core.id, &cpu_set);
        }
    }

    sched_setaffinity(tid, sizeof(cpu_set_t), &cpu_set);
}

}  // namespace angle
