//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// cl_types.h: Defines common types for the OpenCL support in ANGLE.

#ifndef LIBANGLE_CLTYPES_H_
#define LIBANGLE_CLTYPES_H_

#include "libANGLE/CLBitField.h"
#include "libANGLE/CLRefPointer.h"
#include "libANGLE/Debug.h"

#include "common/PackedCLEnums_autogen.h"
#include "common/angleutils.h"

// Include frequently used standard headers
#include <algorithm>
#include <array>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace cl
{

class Buffer;
class CommandQueue;
class Context;
class Device;
class Event;
class Image;
class Kernel;
class Memory;
class Object;
class Platform;
class Program;
class Sampler;

using BufferPtr       = RefPointer<Buffer>;
using CommandQueuePtr = RefPointer<CommandQueue>;
using ContextPtr      = RefPointer<Context>;
using DevicePtr       = RefPointer<Device>;
using EventPtr        = RefPointer<Event>;
using KernelPtr       = RefPointer<Kernel>;
using MemoryPtr       = RefPointer<Memory>;
using PlatformPtr     = RefPointer<Platform>;
using ProgramPtr      = RefPointer<Program>;
using SamplerPtr      = RefPointer<Sampler>;

using BufferPtrs   = std::vector<BufferPtr>;
using DevicePtrs   = std::vector<DevicePtr>;
using EventPtrs    = std::vector<EventPtr>;
using KernelPtrs   = std::vector<KernelPtr>;
using MemoryPtrs   = std::vector<MemoryPtr>;
using PlatformPtrs = std::vector<PlatformPtr>;
using ProgramPtrs  = std::vector<ProgramPtr>;

using CompiledWorkgroupSize = std::array<uint32_t, 3>;

struct ImageDescriptor
{
    MemObjectType type;
    size_t width;
    size_t height;
    size_t depth;
    size_t arraySize;
    size_t rowPitch;
    size_t slicePitch;
    cl_uint numMipLevels;
    cl_uint numSamples;
};

struct MemOffsets
{
    size_t x, y, z;
};

struct Coordinate
{
    size_t x, y, z;
};

struct NDRange
{
    NDRange(cl_uint workDim,
            const size_t *globalWorkOffset,
            const size_t *globalWorkSize,
            const size_t *localWorkSize)
        : workDimensions(workDim),
          nullLocalWorkSize(localWorkSize == nullptr),
          offset({0, 0, 0}),
          gws({1, 1, 1}),
          lws({1, 1, 1})
    {
        for (cl_uint dim = 0; dim < workDim; dim++)
        {
            if (globalWorkOffset != nullptr)
            {
                offset[dim] = static_cast<uint32_t>(globalWorkOffset[dim]);
            }
            if (globalWorkSize != nullptr)
            {
                gws[dim] = static_cast<uint32_t>(globalWorkSize[dim]);
            }
            if (localWorkSize != nullptr)
            {
                lws[dim] = static_cast<uint32_t>(localWorkSize[dim]);
            }
            ASSERT(gws[dim] != 0);
            ASSERT(lws[dim] != 0);
        }
    }

    cl_uint workDimensions;
    bool nullLocalWorkSize{false};
    std::array<uint32_t, 3> offset;
    std::array<uint32_t, 3> gws;
    std::array<uint32_t, 3> lws;
};

}  // namespace cl

#endif  // LIBANGLE_CLTYPES_H_
