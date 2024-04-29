//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_RENDERER_WGPU_PIPELINE_STATE_H_
#define LIBANGLE_RENDERER_WGPU_PIPELINE_STATE_H_

#include <dawn/webgpu_cpp.h>
#include <stdint.h>
#include <limits>

#include "libANGLE/Constants.h"
#include "libANGLE/Error.h"

#include "common/PackedEnums.h"

namespace rx
{
class ContextWgpu;

namespace webgpu
{
template <typename T>
constexpr uint32_t BitCountForEnumValue(T value)
{
    return static_cast<uint32_t>(value) < 2
               ? static_cast<uint32_t>(value)
               : 1 + BitCountForEnumValue(static_cast<uint32_t>(value) >> 1);
}

constexpr uint32_t kTextureFormatBitCount =
    BitCountForEnumValue(wgpu::TextureFormat::R8BG8A8Triplanar420Unorm);
constexpr uint32_t kColorWriteMaskBitCount = 4;

constexpr uint32_t kBlendFactorBitCount =
    BitCountForEnumValue(wgpu::BlendFactor::OneMinusSrc1Alpha);
constexpr uint32_t kBlendOperationBitCount = BitCountForEnumValue(wgpu::BlendOperation::Max);

struct PackedColorTargetState final
{
    uint8_t format : kTextureFormatBitCount;
    uint8_t blendEnabled : 1;
    uint8_t colorBlendSrcFactor : kBlendFactorBitCount;
    uint8_t colorBlendDstFactor : kBlendFactorBitCount;
    uint8_t colorBlendOp : kBlendOperationBitCount;
    uint8_t alphaBlendSrcFactor : kBlendFactorBitCount;
    uint8_t alphaBlendDstFactor : kBlendFactorBitCount;
    uint8_t alphaBlendOp : kBlendOperationBitCount;
    uint8_t writeMask : kColorWriteMaskBitCount;
};

constexpr size_t kPackedColorTargetStateSize = sizeof(PackedColorTargetState);
static_assert(kPackedColorTargetStateSize == 6, "Size mismatch");

constexpr uint32_t kCompareFunctionBitCount = BitCountForEnumValue(wgpu::CompareFunction::Always);
constexpr uint32_t kStencilOperationBitCount =
    BitCountForEnumValue(wgpu::StencilOperation::DecrementWrap);

struct PackedDepthStencilState final
{
    uint8_t depthCompare : kCompareFunctionBitCount;

    uint8_t stencilFrontCompare : kCompareFunctionBitCount;
    uint8_t stencilFrontFailOp : kStencilOperationBitCount;
    uint8_t stencilFrontDepthFailOp : kStencilOperationBitCount;
    uint8_t stencilFrontPassOp : kStencilOperationBitCount;

    uint8_t stencilBackCompare : kCompareFunctionBitCount;
    uint8_t stencilBackFailOp : kStencilOperationBitCount;
    uint8_t stencilBackDepthFailOp : kStencilOperationBitCount;
    uint8_t stencilBackPassOp : kStencilOperationBitCount;

    uint8_t depthWriteEnabled : 1;

    uint32_t stencilReadMask;
    uint32_t stencilWriteMask;
    int32_t depthBias;
    float depthBiasSlopeScalef;
    float depthBiasClamp;
};

constexpr size_t kPackedDepthStencilStateSize = sizeof(PackedDepthStencilState);
static_assert(kPackedDepthStencilStateSize == 28, "Size mismatch");

constexpr uint32_t kVertexFormatBitCount =
    BitCountForEnumValue(wgpu::VertexFormat::Unorm10_10_10_2);

// A maximum offset of 4096 covers almost every Vulkan driver on desktop (80%) and mobile (99%). The
// next highest values to meet native drivers are 16 bits or 32 bits.
constexpr uint32_t kAttributeOffsetMaxBits = 15;

struct PackedVertexAttribute final
{
    uint16_t offset : kAttributeOffsetMaxBits;
    uint8_t format : kVertexFormatBitCount;
    uint8_t shaderLocation;
};

constexpr size_t kPackedVertexAttributeSize = sizeof(PackedVertexAttribute);
static_assert(kPackedVertexAttributeSize == 4, "Size mismatch");

class RenderPipelineDesc final
{
  public:
    // Use aligned allocation and free so we can use the alignas keyword.
    void *operator new(std::size_t size);
    void operator delete(void *ptr);

    RenderPipelineDesc();
    ~RenderPipelineDesc();
    RenderPipelineDesc(const RenderPipelineDesc &other);
    RenderPipelineDesc &operator=(const RenderPipelineDesc &other);

    size_t hash() const;

    angle::Result createPipeline(ContextWgpu *context,
                                 const wgpu::PipelineLayout &pipelineLayout,
                                 const gl::ShaderMap<wgpu::ShaderModule> &shaders,
                                 wgpu::RenderPipeline *pipelineOut) const;

  private:
    PackedVertexAttribute mVertexAttributes[gl::MAX_VERTEX_ATTRIBS];

    PackedColorTargetState mColorTargetStates[gl::IMPLEMENTATION_MAX_DRAW_BUFFERS];
    PackedDepthStencilState mDepthStencilState;
};

bool operator==(const RenderPipelineDesc &lhs, const RenderPipelineDesc &rhs);

}  // namespace webgpu
}  // namespace rx

// Introduce std::hash for the above classes.
namespace std
{
template <>
struct hash<rx::webgpu::RenderPipelineDesc>
{
    size_t operator()(const rx::webgpu::RenderPipelineDesc &key) const { return key.hash(); }
};
}  // namespace std

namespace rx
{
namespace webgpu
{

class PipelineCache final
{
  public:
    PipelineCache();
    ~PipelineCache();

    angle::Result getRenderPipeline(ContextWgpu *context,
                                    const RenderPipelineDesc &desc,
                                    wgpu::RenderPipeline *pipelineOut);

  private:
    std::unordered_map<RenderPipelineDesc, wgpu::RenderPipeline> mRenderPipelines;
};

}  // namespace webgpu

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_PIPELINE_STATE_H_
