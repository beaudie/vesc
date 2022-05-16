// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// spirv_types.cpp:
//   Helper SPIR-V functions.

#include "spirv_types.h"

// SPIR-V tools include for AST validation.
#include <spirv-tools/libspirv.hpp>

namespace angle
{
namespace spirv
{

#if defined(ANGLE_ENABLE_ASSERTS)
namespace
{
void ValidateSpirvMessage(spv_message_level_t level,
                          const char *source,
                          const spv_position_t &position,
                          const char *message)
{
    WARN() << "Level" << level << ": " << message;
}
}  // anonymous namespace

bool Validate(const Blob &blob)
{
    spvtools::SpirvTools spirvTools(SPV_ENV_VULKAN_1_1);

    spirvTools.SetMessageConsumer(ValidateSpirvMessage);
    bool result = spirvTools.Validate(blob);

    if (!result)
    {
        std::string readableSpirv;
        spirvTools.Disassemble(blob, &readableSpirv, 0);
        WARN() << "Invalid SPIR-V:\n" << readableSpirv;
    }

    return result;
}
#else   // ANGLE_ENABLE_ASSERTS
bool Validate(const Blob &blob)
{
    // Placeholder implementation since this is only used inside an ASSERT().
    // Return false to indicate an error in case this is ever accidentally used somewhere else.
    return false;
}
#endif  // ANGLE_ENABLE_ASSERTS

namespace
{

uint32_t MakeLengthOp(size_t length, uint32_t op)
{
    ASSERT(length <= 0xFFFFu);
    ASSERT(op <= 0xFFFFu);

    return static_cast<uint32_t>(length) << 16 | op;
}

}  // namespace
void WriteDecorateUint32(Blob *blob,
                         IdRef target,
                         uint32_t decoration,
                         const LiteralIntegerList &valuesList)
{
    const size_t startSize = blob->size();
    blob->push_back(0);
    blob->push_back(target);
    blob->push_back(decoration);
    for (const auto &operand : valuesList)
    {
        blob->push_back(operand);
    }
    const uint32_t kOpDecorate = 71;
    (*blob)[startSize]         = MakeLengthOp(blob->size() - startSize, kOpDecorate);
}

}  // namespace spirv
}  // namespace angle
