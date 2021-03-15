//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// frame_capture_utils.cpp:
//   ANGLE frame capture util implementation.
//

#include "libANGLE/capture/frame_capture_utils.h"

#include <vector>

#include "common/Color.h"
#include "common/MemoryBuffer.h"
#include "common/angleutils.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/Caps.h"
#include "libANGLE/Context.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/Query.h"
#include "libANGLE/RefCountObject.h"
#include "libANGLE/ResourceMap.h"
#include "libANGLE/Sampler.h"
#include "libANGLE/State.h"

#include "libANGLE/TransformFeedback.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/FramebufferImpl.h"
#include "libANGLE/renderer/RenderbufferImpl.h"
#include "libANGLE/serializer/JsonSerializer.h"

// Note: when diagnosing serialization comparison failures, you can disable the unused function
// compiler warning to allow bisecting the comparison function. One first check is to disable
// Framebuffer Attachment pixel comparison which includes the pixel contents of the default FBO.
// ANGLE_DISABLE_UNUSED_FUNCTION_WARNING

namespace angle
{

namespace
{

using angle::JsonSerializer;

using gl::BufferUsage;
using gl::CompileStatus;
using gl::CullFaceMode;
using gl::InitState;
using gl::PrimitiveMode;
using gl::ProvokingVertexConvention;
using gl::SrgbOverride;
using gl::TextureTarget;
using gl::TextureType;

using sh::BlockLayoutType;
using sh::BlockType;
using sh::InterpolationType;

static const char *TextureTypeToString(TextureType type)
{
    switch (type)
    {
        case TextureType::_2D:
            return "TEXTURE_2D";
        case TextureType::_2DArray:
            return "TEXTURE_2D_ARRAY";
        case TextureType::_2DMultisample:
            return "TEXTURE_2DMS";
        case TextureType::_2DMultisampleArray:
            return "TEXTURE_2DMS_ARRAY";
        case TextureType::_3D:
            return "TEXTURE_3D";
        case TextureType::External:
            return "TEXTURE_EXTERNAL";
        case TextureType::Rectangle:
            return "TEXTURE_RECT";
        case TextureType::CubeMap:
            return "TEXTURE_CUBE_MAP";
        case TextureType::CubeMapArray:
            return "TEXTURE_CUBE_MAP_ARRAY";
        case TextureType::VideoImage:
            return "TEXTURE_VIDEO_IMAGE";
        case TextureType::Buffer:
            return "TEXTURE_BUFFER";
        default:
            return "invalid";
    }
}

static const char *CullFaceModeToString(CullFaceMode mode)
{
    switch (mode)
    {
        case CullFaceMode::Back:
            return "CULL_BACK";
        case CullFaceMode::Front:
            return "CULL_FRONT";
        case CullFaceMode::FrontAndBack:
            return "CULL_FRONT_AND_BACK";
        default:
            return "invalid";
    }
}

static const char *ProvokingVertexConventionToString(ProvokingVertexConvention mode)
{
    switch (mode)
    {
        case ProvokingVertexConvention::FirstVertexConvention:
            return "First";
        case ProvokingVertexConvention::LastVertexConvention:
            return "Last";
        default:
            return "invalid";
    }
}

static const char *InitStateToString(InitState state)
{
    return state == InitState::Initialized ? "Initialized" : "MayNeedInit";
}

static const char *BlockLayoutTypeToString(BlockLayoutType type)
{
    switch (type)
    {
        case BlockLayoutType::BLOCKLAYOUT_STD140:
            return "std140";
        case BlockLayoutType::BLOCKLAYOUT_STD430:
            return "std430";
        case BlockLayoutType::BLOCKLAYOUT_PACKED:
            return "packed";
        case BlockLayoutType::BLOCKLAYOUT_SHARED:
            return "shared";
        default:
            return "invalid";
    }
}

static const char *BlockTypeToString(BlockType type)
{
    return type == BlockType::BLOCK_BUFFER ? "buffer" : "uniform";
}

static const char *InterpolationTypeToString(InterpolationType type)
{
    switch (type)
    {
        case InterpolationType::INTERPOLATION_SMOOTH:
            return "smooth";
        case InterpolationType::INTERPOLATION_CENTROID:
            return "centroid";
        case InterpolationType::INTERPOLATION_SAMPLE:
            return "sampled";
        case InterpolationType::INTERPOLATION_FLAT:
            return "flat";
        case InterpolationType::INTERPOLATION_NOPERSPECTIVE:
            return "nonperspective";
        default:
            return "invalid";
    }
}

#define ENUM_TO_STRING(C, M) \
    case C ::M:              \
        return #M

static const char *PrimitiveModeToString(PrimitiveMode mode)
{
    switch (mode)
    {
        ENUM_TO_STRING(PrimitiveMode, Points);
        ENUM_TO_STRING(PrimitiveMode, Lines);
        ENUM_TO_STRING(PrimitiveMode, LineLoop);
        ENUM_TO_STRING(PrimitiveMode, LineStrip);
        ENUM_TO_STRING(PrimitiveMode, Triangles);
        ENUM_TO_STRING(PrimitiveMode, TriangleStrip);
        ENUM_TO_STRING(PrimitiveMode, TriangleFan);
        ENUM_TO_STRING(PrimitiveMode, Unused1);
        ENUM_TO_STRING(PrimitiveMode, Unused2);
        ENUM_TO_STRING(PrimitiveMode, Unused3);
        ENUM_TO_STRING(PrimitiveMode, LinesAdjacency);
        ENUM_TO_STRING(PrimitiveMode, LineStripAdjacency);
        ENUM_TO_STRING(PrimitiveMode, TrianglesAdjacency);
        ENUM_TO_STRING(PrimitiveMode, TriangleStripAdjacency);
        ENUM_TO_STRING(PrimitiveMode, Patches);
        default:
            return "invalid";
    }
}

static const char *BufferUsageToString(BufferUsage usage)
{
    switch (usage)
    {
        ENUM_TO_STRING(BufferUsage, DynamicCopy);
        ENUM_TO_STRING(BufferUsage, DynamicDraw);
        ENUM_TO_STRING(BufferUsage, DynamicRead);
        ENUM_TO_STRING(BufferUsage, StaticCopy);
        ENUM_TO_STRING(BufferUsage, StaticDraw);
        ENUM_TO_STRING(BufferUsage, StaticRead);
        ENUM_TO_STRING(BufferUsage, StreamCopy);
        ENUM_TO_STRING(BufferUsage, StreamDraw);
        ENUM_TO_STRING(BufferUsage, StreamRead);
        default:
            return "invalid";
    }
}

static const char *SrgbOverrideToString(SrgbOverride value)
{
    switch (value)
    {
        ENUM_TO_STRING(SrgbOverride, Default);
        ENUM_TO_STRING(SrgbOverride, SRGB);
        ENUM_TO_STRING(SrgbOverride, Linear);
        default:
            return "invalid";
    }
}

static const char *ColorGenericTypeToString(ColorGeneric::Type type)
{
    switch (type)
    {
        ENUM_TO_STRING(ColorGeneric::Type, Float);
        ENUM_TO_STRING(ColorGeneric::Type, Int);
        ENUM_TO_STRING(ColorGeneric::Type, UInt);
        default:
            return "invalid";
    }
}

static const char *CompileStatusToString(CompileStatus status)
{
    switch (status)
    {
        ENUM_TO_STRING(CompileStatus, NOT_COMPILED);
        ENUM_TO_STRING(CompileStatus, COMPILE_REQUESTED);
        ENUM_TO_STRING(CompileStatus, COMPILED);
        default:
            return "invalid";
    }
}

#undef ENUM_TO_STRING

class GroupScope
{
  public:
    GroupScope(JsonSerializer *json_, const std::string &name) : json(json_)
    {
        json->startGroup(name);
    }

    template <typename Int>
    GroupScope(JsonSerializer *json_, const std::string &name, Int index) : json(json_)
    {
        std::ostringstream os;
        os << name << index;
        json->startGroup(os.str());
    }
    template <typename Int>
    GroupScope(JsonSerializer *json_, Int index) : GroupScope(json_, "", index)
    {}

    ~GroupScope() { json->endGroup(); }

  private:
    JsonSerializer *json;
};

void SerializeColorF(JsonSerializer *json, const ColorF &color)
{
    json->addEntry("red", color.red);
    json->addEntry("green", color.green);
    json->addEntry("blue", color.blue);
    json->addEntry("alpha", color.alpha);
}

void SerializeColorI(JsonSerializer *json, const ColorI &color)
{
    json->addEntry("Red", color.red);
    json->addEntry("Green", color.green);
    json->addEntry("Blue", color.blue);
    json->addEntry("Alpha", color.alpha);
}

void SerializeColorUI(JsonSerializer *json, const ColorUI &color)
{
    json->addEntry("Red", color.red);
    json->addEntry("Green", color.green);
    json->addEntry("Blue", color.blue);
    json->addEntry("Alpha", color.alpha);
}

template <class ObjectType>
void SerializeOffsetBindingPointerVector(
    JsonSerializer *json,
    const std::vector<gl::OffsetBindingPointer<ObjectType>> &offsetBindingPointerVector)
{
    for (size_t i = 0; i < offsetBindingPointerVector.size(); i++)
    {
        GroupScope group(json, i);
        json->addEntry("Value", offsetBindingPointerVector[i].id().value);
        json->addEntry("Offset", offsetBindingPointerVector[i].getOffset());
        json->addEntry("Size", offsetBindingPointerVector[i].getSize());
    }
}

template <class ObjectType>
void SerializeBindingPointerVector(
    JsonSerializer *json,
    const std::vector<gl::BindingPointer<ObjectType>> &bindingPointerVector)
{
    for (size_t i = 0; i < bindingPointerVector.size(); i++)
    {
        // Do not serialize zero bindings, as this will create unwanted diffs
        if (bindingPointerVector[i].id().value != 0)
        {
            std::ostringstream s;
            s << i;
            json->addEntry(s.str().c_str(), bindingPointerVector[i].id().value);
        }
    }
}

template <class T>
void SerializeRange(JsonSerializer *json, const gl::Range<T> &range)
{
    GroupScope group(json, "Range");
    json->addEntry("Low", range.low());
    json->addEntry("High", range.high());
}

bool IsValidColorAttachmentBinding(GLenum binding, size_t colorAttachmentsCount)
{
    return binding == GL_BACK || (binding >= GL_COLOR_ATTACHMENT0 &&
                                  (binding - GL_COLOR_ATTACHMENT0) < colorAttachmentsCount);
}

Result ReadPixelsFromAttachment(const gl::Context *context,
                                gl::Framebuffer *framebuffer,
                                const gl::FramebufferAttachment &framebufferAttachment,
                                ScratchBuffer *scratchBuffer,
                                MemoryBuffer **pixels)
{
    gl::Extents extents       = framebufferAttachment.getSize();
    GLenum binding            = framebufferAttachment.getBinding();
    gl::InternalFormat format = *framebufferAttachment.getFormat().info;
    if (IsValidColorAttachmentBinding(binding,
                                      framebuffer->getState().getColorAttachments().size()))
    {
        format = framebuffer->getImplementation()->getImplementationColorReadFormat(context);
    }
    ANGLE_CHECK_GL_ALLOC(const_cast<gl::Context *>(context),
                         scratchBuffer->getInitialized(
                             format.pixelBytes * extents.width * extents.height, pixels, 0));
    ANGLE_TRY(framebuffer->readPixels(context, gl::Rectangle{0, 0, extents.width, extents.height},
                                      format.format, format.type, gl::PixelPackState{}, nullptr,
                                      (*pixels)->data()));
    return Result::Continue;
}
void SerializeImageIndex(JsonSerializer *json, const gl::ImageIndex &imageIndex)
{
    GroupScope group(json, "Image");
    json->addEntry("ImageType", TextureTypeToString(imageIndex.getType()));
    json->addEntry("LevelIndex", imageIndex.getLevelIndex());
    json->addEntry("LayerIndex", imageIndex.getLayerIndex());
    json->addEntry("LayerCount", imageIndex.getLayerCount());
}

Result SerializeFramebufferAttachment(const gl::Context *context,
                                      JsonSerializer *json,
                                      ScratchBuffer *scratchBuffer,
                                      gl::Framebuffer *framebuffer,
                                      const gl::FramebufferAttachment &framebufferAttachment)
{
    if (framebufferAttachment.type() == GL_TEXTURE ||
        framebufferAttachment.type() == GL_RENDERBUFFER)
    {
        json->addEntry("ID", framebufferAttachment.id());
    }
    json->addEntry("Type", framebufferAttachment.type());
    // serialize target variable
    json->addEntry("Binding", framebufferAttachment.getBinding());
    if (framebufferAttachment.type() == GL_TEXTURE)
    {
        SerializeImageIndex(json, framebufferAttachment.getTextureImageIndex());
    }
    json->addEntry("NumViews", framebufferAttachment.getNumViews());
    json->addEntry("Multiview", framebufferAttachment.isMultiview());
    json->addEntry("ViewIndex", framebufferAttachment.getBaseViewIndex());
    json->addEntry("Samples", framebufferAttachment.getRenderToTextureSamples());

    if (framebufferAttachment.type() != GL_TEXTURE &&
        framebufferAttachment.type() != GL_RENDERBUFFER)
    {
        GLenum prevReadBufferState = framebuffer->getReadBufferState();
        GLenum binding             = framebufferAttachment.getBinding();
        if (IsValidColorAttachmentBinding(binding,
                                          framebuffer->getState().getColorAttachments().size()))
        {
            framebuffer->setReadBuffer(framebufferAttachment.getBinding());
            ANGLE_TRY(framebuffer->syncState(context, GL_FRAMEBUFFER, gl::Command::Other));
        }
        MemoryBuffer *pixelsPtr = nullptr;
        ANGLE_TRY(ReadPixelsFromAttachment(context, framebuffer, framebufferAttachment,
                                           scratchBuffer, &pixelsPtr));
        json->addEntry("Data", pixelsPtr->data(), pixelsPtr->size());
        // Reset framebuffer state
        framebuffer->setReadBuffer(prevReadBufferState);
    }
    return Result::Continue;
}

Result SerializeFramebufferState(const gl::Context *context,
                                 JsonSerializer *json,
                                 ScratchBuffer *scratchBuffer,
                                 gl::Framebuffer *framebuffer,
                                 const gl::FramebufferState &framebufferState)
{
    GroupScope group(json, "Framebuffer");
    json->addEntry("ID", framebufferState.id().value);
    json->addEntry("Label", framebufferState.getLabel());
    json->addEntry("DrawStates", framebufferState.getDrawBufferStates());
    json->addEntry("ReadBufferState", framebufferState.getReadBufferState());
    json->addEntry("DefaultWidth", framebufferState.getDefaultWidth());
    json->addEntry("DefaultHeight", framebufferState.getDefaultHeight());
    json->addEntry("DefaultSamples", framebufferState.getDefaultSamples());
    json->addEntry("DefaultFixedSampleLocation", framebufferState.getDefaultFixedSampleLocations());
    json->addEntry("DefaultLayers", framebufferState.getDefaultLayers());

    const std::vector<gl::FramebufferAttachment> &colorAttachments =
        framebufferState.getColorAttachments();
    for (const gl::FramebufferAttachment &colorAttachment : colorAttachments)
    {
        if (colorAttachment.isAttached())
        {
            GroupScope colorAttachmentgroup(json, "ColorAttachment");
            ANGLE_TRY(SerializeFramebufferAttachment(context, json, scratchBuffer, framebuffer,
                                                     colorAttachment));
        }
    }
    if (framebuffer->getDepthStencilAttachment())
    {
        GroupScope dsAttachmentgroup(json, "DepthStencilAttachment");
        ANGLE_TRY(SerializeFramebufferAttachment(context, json, scratchBuffer, framebuffer,
                                                 *framebuffer->getDepthStencilAttachment()));
    }
    else
    {
        if (framebuffer->getDepthAttachment())
        {
            GroupScope depthAttachmentgroup(json, "DepthAttachment");
            ANGLE_TRY(SerializeFramebufferAttachment(context, json, scratchBuffer, framebuffer,
                                                     *framebuffer->getDepthAttachment()));
        }
        if (framebuffer->getStencilAttachment())
        {
            GroupScope stencilAttachmengroup(json, "StencilAttachment");
            ANGLE_TRY(SerializeFramebufferAttachment(context, json, scratchBuffer, framebuffer,
                                                     *framebuffer->getStencilAttachment()));
        }
    }
    return Result::Continue;
}

Result SerializeFramebuffer(const gl::Context *context,
                            JsonSerializer *json,
                            ScratchBuffer *scratchBuffer,
                            gl::Framebuffer *framebuffer)
{
    return SerializeFramebufferState(context, json, scratchBuffer, framebuffer,
                                     framebuffer->getState());
}

void SerializeRasterizerState(JsonSerializer *json, const gl::RasterizerState &rasterizerState)
{
    GroupScope group(json, "Rasterizer");
    json->addEntry("CullFace", rasterizerState.cullFace);
    json->addEntry("CullMode", CullFaceModeToString(rasterizerState.cullMode));
    json->addEntry("FrontFace", rasterizerState.frontFace);
    json->addEntry("PolygonOffsetFill", rasterizerState.polygonOffsetFill);
    json->addEntry("PolygonOffsetFactor", rasterizerState.polygonOffsetFactor);
    json->addEntry("PolygonOffsetUnits", rasterizerState.polygonOffsetUnits);
    json->addEntry("PointDrawMode", rasterizerState.pointDrawMode);
    json->addEntry("MultiSample", rasterizerState.multiSample);
    json->addEntry("RasterizerDiscard", rasterizerState.rasterizerDiscard);
    json->addEntry("Dither", rasterizerState.dither);
}

void SerializeRectangle(JsonSerializer *json,
                        const std::string &name,
                        const gl::Rectangle &rectangle)
{
    GroupScope group(json, name);
    json->addEntry("x", rectangle.x);
    json->addEntry("y", rectangle.y);
    json->addEntry("w", rectangle.width);
    json->addEntry("h", rectangle.height);
}

void SerializeBlendStateExt(JsonSerializer *json, const gl::BlendStateExt &blendStateExt)
{
    GroupScope group(json, "BlendStateExt");
    uint64_t maxDrawBuffers = blendStateExt.mMaxDrawBuffers;
    json->addEntry("MaxDrawBuffers", maxDrawBuffers);
    json->addEntry("enableMask", blendStateExt.mEnabledMask.bits());
    json->addEntry("DstColor", blendStateExt.mDstColor);
    json->addEntry("DstAlpha", blendStateExt.mDstAlpha);
    json->addEntry("SrcColor", blendStateExt.mSrcColor);
    json->addEntry("SrcAlpha", blendStateExt.mSrcAlpha);
    json->addEntry("EquationColor", blendStateExt.mEquationColor);
    json->addEntry("EquationAlpha", blendStateExt.mEquationAlpha);
    json->addEntry("ColorMask", blendStateExt.mColorMask);
}

void SerializeDepthStencilState(JsonSerializer *json,
                                const gl::DepthStencilState &depthStencilState)
{
    GroupScope group(json, "DepthStencilState");
    json->addEntry("DepthTest", depthStencilState.depthTest);
    json->addEntry("DepthFunc", depthStencilState.depthFunc);
    json->addEntry("DepthMask", depthStencilState.depthMask);
    json->addEntry("StencilTest", depthStencilState.stencilTest);
    json->addEntry("StencilFunc", depthStencilState.stencilFunc);
    json->addEntry("StencilMask", depthStencilState.stencilMask);
    json->addEntry("StencilFail", depthStencilState.stencilFail);
    json->addEntry("StencilPassDepthFail", depthStencilState.stencilPassDepthFail);
    json->addEntry("StencilPassDepthPass", depthStencilState.stencilPassDepthPass);
    json->addEntry("StencilWritemask", depthStencilState.stencilWritemask);
    json->addEntry("StencilBackFunc", depthStencilState.stencilBackFunc);
    json->addEntry("StencilBackMask", depthStencilState.stencilBackMask);
    json->addEntry("StencilBackFail", depthStencilState.stencilBackFail);
    json->addEntry("StencilBackPassDepthFail", depthStencilState.stencilBackPassDepthFail);
    json->addEntry("StencilBackPassDepthPass", depthStencilState.stencilBackPassDepthPass);
    json->addEntry("StencilBackWritemask", depthStencilState.stencilBackWritemask);
}

void SerializeVertexAttribCurrentValueData(
    JsonSerializer *json,
    const gl::VertexAttribCurrentValueData &vertexAttribCurrentValueData)
{
    ASSERT(vertexAttribCurrentValueData.Type == gl::VertexAttribType::Float ||
           vertexAttribCurrentValueData.Type == gl::VertexAttribType::Int ||
           vertexAttribCurrentValueData.Type == gl::VertexAttribType::UnsignedInt);
    if (vertexAttribCurrentValueData.Type == gl::VertexAttribType::Float)
    {
        json->addEntry("0", vertexAttribCurrentValueData.Values.FloatValues[0]);
        json->addEntry("1", vertexAttribCurrentValueData.Values.FloatValues[1]);
        json->addEntry("2", vertexAttribCurrentValueData.Values.FloatValues[2]);
        json->addEntry("3", vertexAttribCurrentValueData.Values.FloatValues[3]);
    }
    else if (vertexAttribCurrentValueData.Type == gl::VertexAttribType::Int)
    {
        json->addEntry("0", vertexAttribCurrentValueData.Values.IntValues[0]);
        json->addEntry("1", vertexAttribCurrentValueData.Values.IntValues[1]);
        json->addEntry("2", vertexAttribCurrentValueData.Values.IntValues[2]);
        json->addEntry("3", vertexAttribCurrentValueData.Values.IntValues[3]);
    }
    else
    {
        json->addEntry("0", vertexAttribCurrentValueData.Values.UnsignedIntValues[0]);
        json->addEntry("1", vertexAttribCurrentValueData.Values.UnsignedIntValues[1]);
        json->addEntry("2", vertexAttribCurrentValueData.Values.UnsignedIntValues[2]);
        json->addEntry("3", vertexAttribCurrentValueData.Values.UnsignedIntValues[3]);
    }
}

void SerializePixelPackState(JsonSerializer *json, const gl::PixelPackState &pixelPackState)
{
    GroupScope group(json, "PixelPackState");
    json->addEntry("Alignment", pixelPackState.alignment);
    json->addEntry("RowLength", pixelPackState.rowLength);
    json->addEntry("SkipRows", pixelPackState.skipRows);
    json->addEntry("SkipPixels", pixelPackState.skipPixels);
    json->addEntry("ImageHeight", pixelPackState.imageHeight);
    json->addEntry("SkipImages", pixelPackState.skipImages);
    json->addEntry("ReverseRowOrder", pixelPackState.reverseRowOrder);
}

void SerializePixelUnpackState(JsonSerializer *json, const gl::PixelUnpackState &pixelUnpackState)
{
    GroupScope group(json, "PixelUnpackState");
    json->addEntry("Alignment", pixelUnpackState.alignment);
    json->addEntry("RowLength", pixelUnpackState.rowLength);
    json->addEntry("SkipRows", pixelUnpackState.skipRows);
    json->addEntry("SkipPixels", pixelUnpackState.skipPixels);
    json->addEntry("ImageHeight", pixelUnpackState.imageHeight);
    json->addEntry("SkipImages", pixelUnpackState.skipImages);
}

void SerializeImageUnit(JsonSerializer *json, const gl::ImageUnit &imageUnit)
{
    GroupScope group(json, "ImageUnit");
    json->addEntry("Level", imageUnit.level);
    json->addEntry("Layered", imageUnit.layered);
    json->addEntry("Layer", imageUnit.layer);
    json->addEntry("Access", imageUnit.access);
    json->addEntry("Format", imageUnit.format);
    json->addEntry("Texid", imageUnit.texture.id().value);
}

void SerializeGLContextStates(JsonSerializer *json, const gl::State &state)
{
    GroupScope group(json, "ContextStates");
    json->addEntry("ClientType", state.getClientType());
    json->addEntry("Priority", state.getContextPriority());
    json->addEntry("Major", state.getClientMajorVersion());
    json->addEntry("Minor", state.getClientMinorVersion());

    SerializeColorF(json, state.getColorClearValue());
    json->addEntry("DepthClearValue", state.getDepthClearValue());
    json->addEntry("StencilClearValue", state.getStencilClearValue());
    SerializeRasterizerState(json, state.getRasterizerState());
    json->addEntry("ScissorTestEnabled", state.isScissorTestEnabled());
    SerializeRectangle(json, "Scissors", state.getScissor());
    SerializeBlendStateExt(json, state.getBlendStateExt());
    SerializeColorF(json, state.getBlendColor());
    json->addEntry("SampleAlphaToCoverageEnabled", state.isSampleAlphaToCoverageEnabled());
    json->addEntry("SampleCoverageEnabled", state.isSampleCoverageEnabled());
    json->addEntry("SampleCoverageValue", state.getSampleCoverageValue());
    json->addEntry("SampleCoverageInvert", state.getSampleCoverageInvert());
    json->addEntry("SampleMaskEnabled", state.isSampleMaskEnabled());
    json->addEntry("MaxSampleMaskWords", state.getMaxSampleMaskWords());
    const auto &sampleMaskValues = state.getSampleMaskValues();
    for (size_t i = 0; i < sampleMaskValues.size(); i++)
    {
        std::ostringstream os;
        os << i;
        json->addEntry(os.str(), sampleMaskValues[i]);
    }
    SerializeDepthStencilState(json, state.getDepthStencilState());
    json->addEntry("StencilRef", state.getStencilRef());
    json->addEntry("StencilBackRef", state.getStencilBackRef());
    json->addEntry("LineWidth", state.getLineWidth());
    json->addEntry("GenerateMipmapHint", state.getGenerateMipmapHint());
    json->addEntry("TextureFilteringHint", state.getTextureFilteringHint());
    json->addEntry("FragmentShaderDerivativeHint", state.getFragmentShaderDerivativeHint());
    json->addEntry("BindGeneratesResourceEnabled", state.isBindGeneratesResourceEnabled());
    json->addEntry("ClientArraysEnabled", state.areClientArraysEnabled());
    SerializeRectangle(json, "Viewport", state.getViewport());
    json->addEntry("Near", state.getNearPlane());
    json->addEntry("Far", state.getFarPlane());
    if (state.getReadFramebuffer())
    {
        json->addEntry("Framebuffer ID", state.getReadFramebuffer()->id().value);
    }
    if (state.getDrawFramebuffer())
    {
        json->addEntry("Draw Framebuffer ID", state.getDrawFramebuffer()->id().value);
    }
    json->addEntry("Renderbuffer ID", state.getRenderbufferId().value);
    if (state.getProgram())
    {
        json->addEntry("ProgramID", state.getProgram()->id().value);
    }
    if (state.getProgramPipeline())
    {
        json->addEntry("ProgramPipelineID", state.getProgramPipeline()->id().value);
    }
    json->addEntry("ProvokingVertex",
                   ProvokingVertexConventionToString(state.getProvokingVertex()));
    const std::vector<gl::VertexAttribCurrentValueData> &vertexAttribCurrentValues =
        state.getVertexAttribCurrentValues();
    for (size_t i = 0; i < vertexAttribCurrentValues.size(); i++)
    {
        GroupScope vagroup(json, "VertexAttribCurrentValues", i);
        SerializeVertexAttribCurrentValueData(json, vertexAttribCurrentValues[i]);
    }
    if (state.getVertexArray())
    {
        json->addEntry("VertexArrayID", state.getVertexArray()->id().value);
    }
    json->addEntry("CurrentValuesTypeMask", state.getCurrentValuesTypeMask().to_ulong());
    json->addEntry("ActiveSampler", state.getActiveSampler());
    for (const auto &textures : state.getBoundTexturesForCapture())
    {
        SerializeBindingPointerVector<gl::Texture>(json, textures);
    }
    json->addEntry("texturesIncompatibleWithSamplers",
                   state.getTexturesIncompatibleWithSamplers().to_ulong());
    SerializeBindingPointerVector<gl::Sampler>(json, state.getSamplers());
    for (const gl::ImageUnit &imageUnit : state.getImageUnits())
    {
        SerializeImageUnit(json, imageUnit);
    }
    for (const auto &query : state.getActiveQueriesForCapture())
    {
        json->addEntry("Query", query.id().value);
    }
    for (const auto &boundBuffer : state.getBoundBuffersForCapture())
    {
        json->addEntry("Bound", boundBuffer.id().value);
    }
    SerializeOffsetBindingPointerVector<gl::Buffer>(json,
                                                    state.getOffsetBindingPointerUniformBuffers());
    SerializeOffsetBindingPointerVector<gl::Buffer>(
        json, state.getOffsetBindingPointerAtomicCounterBuffers());
    SerializeOffsetBindingPointerVector<gl::Buffer>(
        json, state.getOffsetBindingPointerShaderStorageBuffers());
    if (state.getCurrentTransformFeedback())
    {
        json->addEntry("CurrentTransformFeedback", state.getCurrentTransformFeedback()->id().value);
    }
    SerializePixelUnpackState(json, state.getUnpackState());
    SerializePixelPackState(json, state.getPackState());
    json->addEntry("PrimitiveRestartEnabled", state.isPrimitiveRestartEnabled());
    json->addEntry("MultisamplingEnabled", state.isMultisamplingEnabled());
    json->addEntry("SampleAlphaToOneEnabled", state.isSampleAlphaToOneEnabled());
    json->addEntry("CoverageModulation", state.getCoverageModulation());
    json->addEntry("FramebufferSRGB", state.getFramebufferSRGB());
    json->addEntry("RobustResourceInitEnabled", state.isRobustResourceInitEnabled());
    json->addEntry("ProgramBinaryCacheEnabled", state.isProgramBinaryCacheEnabled());
    json->addEntry("TextureRectangleEnabled", state.isTextureRectangleEnabled());
    json->addEntry("MaxShaderCompilerThreads", state.getMaxShaderCompilerThreads());
    json->addEntry("EnabledClipDistances", state.getEnabledClipDistances().to_ulong());
    json->addEntry("BlendFuncConstantAlphaDrawBuffers",
                   state.getBlendFuncConstantAlphaDrawBuffers().to_ulong());
    json->addEntry("BlendFuncConstantColorDrawBuffers",
                   state.getBlendFuncConstantColorDrawBuffers().to_ulong());
    json->addEntry("SimultaneousConstantColorAndAlphaBlendFunc",
                   state.noSimultaneousConstantColorAndAlphaBlendFunc());
}

void SerializeBufferState(JsonSerializer *json, const gl::BufferState &bufferState)
{
    json->addEntry("Label", bufferState.getLabel());
    json->addEntry("Usage", BufferUsageToString(bufferState.getUsage()));
    json->addEntry("Size", bufferState.getSize());
    json->addEntry("AccessFlags", bufferState.getAccessFlags());
    json->addEntry("Access", bufferState.getAccess());
    json->addEntry("Mapped", bufferState.isMapped());
    json->addEntry("MapOffset", bufferState.getMapOffset());
    json->addEntry("MapLength", bufferState.getMapLength());
}

Result SerializeBuffer(const gl::Context *context,
                       JsonSerializer *json,
                       ScratchBuffer *scratchBuffer,
                       gl::Buffer *buffer)
{
    GroupScope group(json, "Buffer");
    SerializeBufferState(json, buffer->getState());
    MemoryBuffer *dataPtr = nullptr;
    ANGLE_CHECK_GL_ALLOC(
        const_cast<gl::Context *>(context),
        scratchBuffer->getInitialized(static_cast<size_t>(buffer->getSize()), &dataPtr, 0));
    ANGLE_TRY(buffer->getSubData(context, 0, dataPtr->size(), dataPtr->data()));
    json->addEntry("data", dataPtr->data(), dataPtr->size());
    return Result::Continue;
}
void SerializeColorGeneric(JsonSerializer *json,
                           const std::string &name,
                           const ColorGeneric &colorGeneric)
{
    GroupScope group(json, name);
    ASSERT(colorGeneric.type == ColorGeneric::Type::Float ||
           colorGeneric.type == ColorGeneric::Type::Int ||
           colorGeneric.type == ColorGeneric::Type::UInt);
    json->addEntry("Type", ColorGenericTypeToString(colorGeneric.type));
    if (colorGeneric.type == ColorGeneric::Type::Float)
    {
        SerializeColorF(json, colorGeneric.colorF);
    }
    else if (colorGeneric.type == ColorGeneric::Type::Int)
    {
        SerializeColorI(json, colorGeneric.colorI);
    }
    else
    {
        SerializeColorUI(json, colorGeneric.colorUI);
    }
}

void SerializeSamplerState(JsonSerializer *json,
                           const std::string &name,
                           const gl::SamplerState &samplerState)
{
    GroupScope group(json, name);
    json->addEntry("MinFilter", samplerState.getMinFilter());
    json->addEntry("MagFilter", samplerState.getMagFilter());
    json->addEntry("WrapS", samplerState.getWrapS());
    json->addEntry("WrapT", samplerState.getWrapT());
    json->addEntry("WrapR", samplerState.getWrapR());
    json->addEntry("MaxAnisotropy", samplerState.getMaxAnisotropy());
    json->addEntry("MinLod", samplerState.getMinLod());
    json->addEntry("MaxLod", samplerState.getMaxLod());
    json->addEntry("CompareMode", samplerState.getCompareMode());
    json->addEntry("CompareFunc", samplerState.getCompareFunc());
    json->addEntry("SRGBDecode", samplerState.getSRGBDecode());
    SerializeColorGeneric(json, "BorderColor", samplerState.getBorderColor());
}

void SerializeSampler(JsonSerializer *json, gl::Sampler *sampler)
{
    json->addEntry("Label", sampler->getLabel());
    SerializeSamplerState(json, "Sampler", sampler->getSamplerState());
}

void SerializeSwizzleState(JsonSerializer *json, const gl::SwizzleState &swizzleState)
{
    json->addEntry("SwizzleRed", swizzleState.swizzleRed);
    json->addEntry("SwizzleGreen", swizzleState.swizzleGreen);
    json->addEntry("SwizzleBlue", swizzleState.swizzleBlue);
    json->addEntry("SwizzleAlpha", swizzleState.swizzleAlpha);
}

void SerializeExtents(JsonSerializer *json, const gl::Extents &extents)
{
    json->addEntry("Width", extents.width);
    json->addEntry("Height", extents.height);
    json->addEntry("Depth", extents.depth);
}

void SerializeInternalFormat(JsonSerializer *json, const gl::InternalFormat *internalFormat)
{
    json->addEntry("InternalFormat", internalFormat->internalFormat);
}

void SerializeFormat(JsonSerializer *json, const gl::Format &format)
{
    SerializeInternalFormat(json, format.info);
}

void SerializeRenderbufferState(JsonSerializer *json,
                                const gl::RenderbufferState &renderbufferState)
{
    GroupScope wg(json, "State");
    json->addEntry("Width", renderbufferState.getWidth());
    json->addEntry("Height", renderbufferState.getHeight());
    SerializeFormat(json, renderbufferState.getFormat());
    json->addEntry("Samples", renderbufferState.getSamples());
    json->addEntry("InitState", InitStateToString(renderbufferState.getInitState()));
}

Result SerializeRenderbuffer(const gl::Context *context,
                             JsonSerializer *json,
                             ScratchBuffer *scratchBuffer,
                             gl::Renderbuffer *renderbuffer)
{
    GroupScope wg(json, "Renderbuffer");
    SerializeRenderbufferState(json, renderbuffer->getState());
    json->addEntry("Label", renderbuffer->getLabel());
    MemoryBuffer *pixelsPtr = nullptr;
    ANGLE_CHECK_GL_ALLOC(
        const_cast<gl::Context *>(context),
        scratchBuffer->getInitialized(renderbuffer->getMemorySize(), &pixelsPtr, 0));
    gl::PixelPackState packState;
    packState.alignment = 1;
    ANGLE_TRY(renderbuffer->getImplementation()->getRenderbufferImage(
        context, packState, nullptr, renderbuffer->getImplementationColorReadFormat(context),
        renderbuffer->getImplementationColorReadType(context), pixelsPtr->data()));
    json->addEntry("pixel", pixelsPtr->data(), pixelsPtr->size());
    return Result::Continue;
}

void SerializeWorkGroupSize(JsonSerializer *json, const sh::WorkGroupSize &workGroupSize)
{
    GroupScope wg(json, "workGroupSize");
    json->addEntry("x", workGroupSize[0]);
    json->addEntry("y", workGroupSize[1]);
    json->addEntry("z", workGroupSize[2]);
}

void SerializeShaderVariable(JsonSerializer *json, const sh::ShaderVariable &shaderVariable)
{
    GroupScope wg(json, "ShaderVariable");
    json->addEntry("Type", shaderVariable.type);
    json->addEntry("Precision", shaderVariable.precision);
    json->addEntry("Name", shaderVariable.name);
    json->addEntry("MappedName", shaderVariable.mappedName);
    json->addEntry("ArraySizes", shaderVariable.arraySizes);
    json->addEntry("StaticUse", shaderVariable.staticUse);
    json->addEntry("Active", shaderVariable.active);
    for (const sh::ShaderVariable &field : shaderVariable.fields)
    {
        SerializeShaderVariable(json, field);
    }
    json->addEntry("StructOrBlockName", shaderVariable.structOrBlockName);
    json->addEntry("MappedStructOrBlockName", shaderVariable.mappedStructOrBlockName);
    json->addEntry("RowMajorLayout", shaderVariable.isRowMajorLayout);
    json->addEntry("Location", shaderVariable.location);
    json->addEntry("Binding", shaderVariable.binding);
    json->addEntry("ImageUnitFormat", shaderVariable.imageUnitFormat);
    json->addEntry("Offset", shaderVariable.offset);
    json->addEntry("Readonly", shaderVariable.readonly);
    json->addEntry("Writeonly", shaderVariable.writeonly);
    json->addEntry("Index", shaderVariable.index);
    json->addEntry("YUV", shaderVariable.yuv);
    json->addEntry("Interpolation", InterpolationTypeToString(shaderVariable.interpolation));
    json->addEntry("Invariant", shaderVariable.isInvariant);
    json->addEntry("TexelFetchStaticUse", shaderVariable.texelFetchStaticUse);
}

void SerializeShaderVariablesVector(JsonSerializer *json,
                                    const std::vector<sh::ShaderVariable> &shaderVariables)
{
    for (const sh::ShaderVariable &shaderVariable : shaderVariables)
    {
        SerializeShaderVariable(json, shaderVariable);
    }
}

void SerializeInterfaceBlocksVector(JsonSerializer *json,
                                    const std::vector<sh::InterfaceBlock> &interfaceBlocks)
{
    for (const sh::InterfaceBlock &interfaceBlock : interfaceBlocks)
    {
        GroupScope group(json, "Interface Block");
        json->addEntry("Name", interfaceBlock.name);
        json->addEntry("MappedName", interfaceBlock.mappedName);
        json->addEntry("InstanceName", interfaceBlock.instanceName);
        json->addEntry("ArraySize", interfaceBlock.arraySize);
        json->addEntry("Layout", BlockLayoutTypeToString(interfaceBlock.layout));
        json->addEntry("Binding", interfaceBlock.binding);
        json->addEntry("StaticUse", interfaceBlock.staticUse);
        json->addEntry("Active", interfaceBlock.active);
        json->addEntry("BlockType", BlockTypeToString(interfaceBlock.blockType));
        SerializeShaderVariablesVector(json, interfaceBlock.fields);
    }
}

void SerializeShaderState(JsonSerializer *json, const gl::ShaderState &shaderState)
{
    GroupScope group(json, "ShaderState");
    json->addEntry("Label", shaderState.getLabel());
    json->addEntry("Type", gl::ShaderTypeToString(shaderState.getShaderType()));
    json->addEntry("Version", shaderState.getShaderVersion());
    json->addEntry("TranslatedSource", shaderState.getTranslatedSource());
    json->addEntry("Source", shaderState.getSource());
    SerializeWorkGroupSize(json, shaderState.getLocalSize());
    SerializeShaderVariablesVector(json, shaderState.getInputVaryings());
    SerializeShaderVariablesVector(json, shaderState.getOutputVaryings());
    SerializeShaderVariablesVector(json, shaderState.getUniforms());
    SerializeInterfaceBlocksVector(json, shaderState.getUniformBlocks());
    SerializeInterfaceBlocksVector(json, shaderState.getShaderStorageBlocks());
    SerializeShaderVariablesVector(json, shaderState.getAllAttributes());
    SerializeShaderVariablesVector(json, shaderState.getActiveAttributes());
    SerializeShaderVariablesVector(json, shaderState.getActiveOutputVariables());
    json->addEntry("EarlyFragmentTestsOptimization",
                   shaderState.getEarlyFragmentTestsOptimization());
    json->addEntry("NumViews", shaderState.getNumViews());
    json->addEntry("SpecConstUsageBits", shaderState.getSpecConstUsageBits().bits());
    if (shaderState.getGeometryShaderInputPrimitiveType().valid())
    {
        json->addEntry(
            "GeometryShaderInputPrimitiveType",
            PrimitiveModeToString(shaderState.getGeometryShaderInputPrimitiveType().value()));
    }
    if (shaderState.getGeometryShaderOutputPrimitiveType().valid())
    {
        json->addEntry(
            "GeometryShaderOutputPrimitiveType",
            PrimitiveModeToString(shaderState.getGeometryShaderOutputPrimitiveType().value()));
    }
    if (shaderState.getGeometryShaderInvocations().valid())
    {
        json->addEntry("GeometryShaderInvocations",
                       shaderState.getGeometryShaderInvocations().value());
    }
    json->addEntry("CompileStatus", CompileStatusToString(shaderState.getCompileStatus()));
}

void SerializeShader(JsonSerializer *json, gl::Shader *shader)
{
    GroupScope group(json, "Shader");
    SerializeShaderState(json, shader->getState());
    json->addEntry("Handle", shader->getHandle().value);
    json->addEntry("RefCount", shader->getRefCount());
    json->addEntry("FlaggedForDeletion", shader->isFlaggedForDeletion());
    // Do not serialize mType because it is already serialized in SerializeShaderState.
    json->addEntry("InfoLogString", shader->getInfoLogString());
    // Do not serialize compiler resources string because it can vary between test modes.
    json->addEntry("CurrentMaxComputeWorkGroupInvocations",
                   shader->getCurrentMaxComputeWorkGroupInvocations());
    json->addEntry("MaxComputeSharedMemory", shader->getMaxComputeSharedMemory());
}

void SerializeVariableLocationsVector(JsonSerializer *json,
                                      const std::string &group_name,
                                      const std::vector<gl::VariableLocation> &variableLocations)
{
    GroupScope group(json, group_name);
    for (const gl::VariableLocation &variableLocation : variableLocations)
    {
        GroupScope vargroup(json, "Variable");
        json->addEntry("ArrayIndex", variableLocation.arrayIndex);
        json->addEntry("Index", variableLocation.index);
        json->addEntry("Ignored", variableLocation.ignored);
    }
}

void SerializeBlockMemberInfo(JsonSerializer *json, const sh::BlockMemberInfo &blockMemberInfo)
{
    GroupScope(json, "BlockMemberInfo");
    json->addEntry("Offset", blockMemberInfo.offset);
    json->addEntry("Stride", blockMemberInfo.arrayStride);
    json->addEntry("MatrixStride", blockMemberInfo.matrixStride);
    json->addEntry("IsRowMajorMatrix", blockMemberInfo.isRowMajorMatrix);
    json->addEntry("TopLevelArrayStride", blockMemberInfo.topLevelArrayStride);
}

void SerializeActiveVariable(JsonSerializer *json, const gl::ActiveVariable &activeVariable)
{
    json->addEntry("ActiveShaders", activeVariable.activeShaders().to_ulong());
}

void SerializeBufferVariablesVector(JsonSerializer *json,
                                    const std::vector<gl::BufferVariable> &bufferVariables)
{
    for (const gl::BufferVariable &bufferVariable : bufferVariables)
    {
        GroupScope group(json, "BufferVariable");
        json->addEntry("BufferIndex", bufferVariable.bufferIndex);
        SerializeBlockMemberInfo(json, bufferVariable.blockInfo);
        json->addEntry("TopLevelArraySize", bufferVariable.topLevelArraySize);
        SerializeActiveVariable(json, bufferVariable);
        SerializeShaderVariable(json, bufferVariable);
    }
}

void SerializeProgramAliasedBindings(JsonSerializer *json,
                                     const gl::ProgramAliasedBindings &programAliasedBindings)
{
    for (const auto &programAliasedBinding : programAliasedBindings)
    {
        GroupScope group(json, programAliasedBinding.first);
        json->addEntry("Location", programAliasedBinding.second.location);
        json->addEntry("Aliased", programAliasedBinding.second.aliased);
    }
}

void SerializeProgramState(JsonSerializer *json, const gl::ProgramState &programState)
{
    json->addEntry("Label", programState.getLabel());
    SerializeWorkGroupSize(json, programState.getComputeShaderLocalSize());
    for (gl::Shader *shader : programState.getAttachedShaders())
    {
        if (shader)
        {
            json->addEntry("Handle", shader->getHandle().value);
        }
        else
        {
            json->addEntry("Handle", 0);
        }
    }
    const gl::ShaderMap<bool> sm = programState.getAttachedShadersMarkedForDetach();
    for (auto i = 0; i < static_cast<int>(gl::ShaderType::EnumCount); ++i)
    {
        auto shaderType = static_cast<gl::ShaderType>(i);
        if (sm[shaderType])
            json->addEntry("Attached", gl::ShaderTypeToString(shaderType));
    }
    json->addEntry("LocationsUsedForXfbExtension", programState.getLocationsUsedForXfbExtension());
    for (const std::string &transformFeedbackVaryingName :
         programState.getTransformFeedbackVaryingNames())
    {
        json->addEntry("TransformFeedbackVaryingName", transformFeedbackVaryingName);
    }
    json->addEntry("ActiveUniformBlockBindingsMask",
                   programState.getActiveUniformBlockBindingsMask().to_ulong());
    SerializeVariableLocationsVector(json, "UniformLocations", programState.getUniformLocations());
    SerializeBufferVariablesVector(json, programState.getBufferVariables());
    SerializeRange(json, programState.getAtomicCounterUniformRange());
    SerializeVariableLocationsVector(json, "SecondaryOutputLocations",
                                     programState.getSecondaryOutputLocations());
    json->addEntry("ActiveOutputVariables", programState.getActiveOutputVariables().to_ulong());
    for (GLenum outputVariableType : programState.getOutputVariableTypes())
    {
        json->addEntry("OutputVariableType", outputVariableType);
    }
    json->addEntry("DrawBufferTypeMask", programState.getDrawBufferTypeMask().to_ulong());
    json->addEntry("BinaryRetrieveableHint", programState.hasBinaryRetrieveableHint());
    json->addEntry("Separable", programState.isSeparable());
    json->addEntry("EarlyFragmentTestsOptimization",
                   programState.hasEarlyFragmentTestsOptimization());
    json->addEntry("NumViews", programState.getNumViews());
    json->addEntry("DrawIDLocation", programState.getDrawIDLocation());
    json->addEntry("BaseVertexLocation", programState.getBaseVertexLocation());
    json->addEntry("BaseInstanceLocation", programState.getBaseInstanceLocation());
    SerializeProgramAliasedBindings(json, programState.getUniformLocationBindings());
}

void SerializeProgramBindings(JsonSerializer *json, const gl::ProgramBindings &programBindings)
{
    for (const auto &programBinding : programBindings)
    {
        json->addEntry(programBinding.first, programBinding.second);
    }
}

void SerializeProgram(JsonSerializer *json, gl::Program *program)
{
    GroupScope group(json, "Program");
    SerializeProgramState(json, program->getState());
    json->addEntry("IsValidated", program->isValidated());
    SerializeProgramBindings(json, program->getAttributeBindings());
    SerializeProgramAliasedBindings(json, program->getFragmentOutputLocations());
    SerializeProgramAliasedBindings(json, program->getFragmentOutputIndexes());
    json->addEntry("IsLinked", program->isLinked());
    json->addEntry("IsFlaggedForDeletion", program->isFlaggedForDeletion());
    json->addEntry("RefCount", program->getRefCount());
    json->addEntry("ID", program->id().value);
}

void SerializeImageDesc(JsonSerializer *json, const gl::ImageDesc &imageDesc)
{
    GroupScope group(json, "ImageDesc");
    SerializeExtents(json, imageDesc.size);
    SerializeFormat(json, imageDesc.format);
    json->addEntry("Samples", imageDesc.samples);
    json->addEntry("FixesSampleLocations", imageDesc.fixedSampleLocations);
    json->addEntry("InitState", InitStateToString(imageDesc.initState));
}

void SerializeTextureState(JsonSerializer *json, const gl::TextureState &textureState)
{
    json->addEntry("Type", TextureTypeToString(textureState.getType()));
    SerializeSwizzleState(json, textureState.getSwizzleState());
    SerializeSamplerState(json, "TextureState", textureState.getSamplerState());
    json->addEntry("SRGB", SrgbOverrideToString(textureState.getSRGBOverride()));
    json->addEntry("BaseLevel", textureState.getBaseLevel());
    json->addEntry("MaxLevel", textureState.getMaxLevel());
    json->addEntry("DepthStencilTextureMode", textureState.getDepthStencilTextureMode());
    json->addEntry("BeenBoundAsImage", textureState.hasBeenBoundAsImage());
    json->addEntry("ImmutableFormat", textureState.getImmutableFormat());
    json->addEntry("ImmutableLevels", textureState.getImmutableLevels());
    json->addEntry("Usage", textureState.getUsage());
    const std::vector<gl::ImageDesc> &imageDescs = textureState.getImageDescs();
    for (const gl::ImageDesc &imageDesc : imageDescs)
    {
        SerializeImageDesc(json, imageDesc);
    }
    SerializeRectangle(json, "Crop", textureState.getCrop());

    json->addEntry("GenerateMipmapHint", textureState.getGenerateMipmapHint());
    json->addEntry("InitState", InitStateToString(textureState.getInitState()));
}

Result SerializeTextureData(JsonSerializer *json,
                            const gl::Context *context,
                            gl::Texture *texture,
                            ScratchBuffer *scratchBuffer)
{
    gl::ImageIndexIterator imageIter = gl::ImageIndexIterator::MakeGeneric(
        texture->getType(), 0, texture->getMipmapMaxLevel() + 1, gl::ImageIndex::kEntireLevel,
        gl::ImageIndex::kEntireLevel);
    while (imageIter.hasNext())
    {
        gl::ImageIndex index = imageIter.next();

        const gl::ImageDesc &desc = texture->getTextureState().getImageDesc(index);

        if (desc.size.empty())
            continue;

        const gl::InternalFormat &format = *desc.format.info;

        // Check for supported textures
        ASSERT(index.getType() == gl::TextureType::_2D || index.getType() == gl::TextureType::_3D ||
               index.getType() == gl::TextureType::_2DArray ||
               index.getType() == gl::TextureType::CubeMap);

        GLenum getFormat = format.format;
        GLenum getType   = format.type;

        const gl::Extents size(desc.size.width, desc.size.height, desc.size.depth);
        const gl::PixelUnpackState &unpack = context->getState().getUnpackState();

        GLuint endByte  = 0;
        bool unpackSize = format.computePackUnpackEndByte(getType, size, unpack, true, &endByte);
        ASSERT(unpackSize);
        MemoryBuffer *texelsPtr = nullptr;
        ANGLE_CHECK_GL_ALLOC(const_cast<gl::Context *>(context),
                             scratchBuffer->getInitialized(endByte, &texelsPtr, 0));

        gl::PixelPackState packState;
        packState.alignment = 1;

        ANGLE_TRY(texture->getTexImage(context, packState, nullptr, index.getTarget(),
                                       index.getLevelIndex(), getFormat, getType,
                                       texelsPtr->data()));
        json->addEntry("Texels", texelsPtr->data(), texelsPtr->size());
    }
    return Result::Continue;
}

Result SerializeTexture(const gl::Context *context,
                        JsonSerializer *json,
                        ScratchBuffer *scratchBuffer,
                        gl::Texture *texture)
{
    GroupScope group(json, "Texture ", texture->getId());
    SerializeTextureState(json, texture->getState());
    json->addEntry("Label", texture->getLabel());
    // FrameCapture can not serialize mBoundSurface and mBoundStream
    // because they are likely to change with each run
    ANGLE_TRY(SerializeTextureData(json, context, texture, scratchBuffer));
    return Result::Continue;
}

void SerializeFormat(JsonSerializer *json, const angle::Format *format)
{
    json->addEntry("InternalFormat", format->glInternalFormat);
}

void SerializeVertexAttributeVector(JsonSerializer *json,
                                    const std::vector<gl::VertexAttribute> &vertexAttributes)
{
    for (const gl::VertexAttribute &vertexAttribute : vertexAttributes)
    {
        GroupScope group(json, "VertexAttribute@BindingIndex", vertexAttribute.bindingIndex);
        json->addEntry("Enabled", vertexAttribute.enabled);
        ASSERT(vertexAttribute.format);
        SerializeFormat(json, vertexAttribute.format);
        json->addEntry("RelativeOffset", vertexAttribute.relativeOffset);
        json->addEntry("VertexAttribArrayStride", vertexAttribute.vertexAttribArrayStride);
    }
}

void SerializeVertexBindingsVector(JsonSerializer *json,
                                   const std::vector<gl::VertexBinding> &vertexBindings)
{
    for (const gl::VertexBinding &vertexBinding : vertexBindings)
    {
        GroupScope group(json, "VertexBinding");
        json->addEntry("Stride", vertexBinding.getStride());
        json->addEntry("Divisor", vertexBinding.getDivisor());

        /* the returntype is a GLintptr that translates to long int, and the conversion to
         * a rapidjson::GenericValue<rapidjson::UTF8<>>' is ambiguous on some architectures */
        int64_t offset = vertexBinding.getOffset();
        json->addEntry("Offset", offset);
        json->addEntry("BufferID", vertexBinding.getBuffer().id().value);
        json->addEntry("BoundAttributesMask", vertexBinding.getBoundAttributesMask().to_ulong());
    }
}

void SerializeVertexArrayState(JsonSerializer *json, const gl::VertexArrayState &vertexArrayState)
{
    json->addEntry("Label", vertexArrayState.getLabel());
    SerializeVertexAttributeVector(json, vertexArrayState.getVertexAttributes());
    if (vertexArrayState.getElementArrayBuffer())
    {
        json->addEntry("ElementArrayBufferID",
                       vertexArrayState.getElementArrayBuffer()->id().value);
    }
    else
    {
        json->addEntry("ElementArrayBufferID", 0);
    }
    SerializeVertexBindingsVector(json, vertexArrayState.getVertexBindings());
    json->addEntry("EnabledAttributesMask", vertexArrayState.getEnabledAttributesMask().to_ulong());
    json->addEntry("VertexAttributesTypeMask",
                   vertexArrayState.getVertexAttributesTypeMask().to_ulong());
    json->addEntry("ClientMemoryAttribsMask",
                   vertexArrayState.getClientMemoryAttribsMask().to_ulong());
    json->addEntry("NullPointerClientMemoryAttribsMask",
                   vertexArrayState.getNullPointerClientMemoryAttribsMask().to_ulong());
}

void SerializeVertexArray(JsonSerializer *json, gl::VertexArray *vertexArray)
{
    GroupScope group(json, "VertexArray", vertexArray->id().value);
    SerializeVertexArrayState(json, vertexArray->getState());
    json->addEntry("BufferAccessValidationEnabled", vertexArray->isBufferAccessValidationEnabled());
}

}  // namespace

Result SerializeContext(JsonSerializer *json, const gl::Context *context)
{
    json->startDocument("Context");

    SerializeGLContextStates(json, context->getState());
    ScratchBuffer scratchBuffer(1);
    const gl::FramebufferManager &framebufferManager =
        context->getState().getFramebufferManagerForCapture();
    for (const auto &framebuffer : framebufferManager)
    {
        gl::Framebuffer *framebufferPtr = framebuffer.second;
        ANGLE_TRY(SerializeFramebuffer(context, json, &scratchBuffer, framebufferPtr));
    }
    const gl::BufferManager &bufferManager = context->getState().getBufferManagerForCapture();
    for (const auto &buffer : bufferManager)
    {
        gl::Buffer *bufferPtr = buffer.second;
        ANGLE_TRY(SerializeBuffer(context, json, &scratchBuffer, bufferPtr));
    }
    const gl::SamplerManager &samplerManager = context->getState().getSamplerManagerForCapture();
    for (const auto &sampler : samplerManager)
    {
        gl::Sampler *samplerPtr = sampler.second;
        SerializeSampler(json, samplerPtr);
    }
    const gl::RenderbufferManager &renderbufferManager =
        context->getState().getRenderbufferManagerForCapture();
    for (const auto &renderbuffer : renderbufferManager)
    {
        gl::Renderbuffer *renderbufferPtr = renderbuffer.second;
        ANGLE_TRY(SerializeRenderbuffer(context, json, &scratchBuffer, renderbufferPtr));
    }
    const gl::ShaderProgramManager &shaderProgramManager =
        context->getState().getShaderProgramManagerForCapture();
    const gl::ResourceMap<gl::Shader, gl::ShaderProgramID> &shaderManager =
        shaderProgramManager.getShadersForCapture();
    for (const auto &shader : shaderManager)
    {
        gl::Shader *shaderPtr = shader.second;
        SerializeShader(json, shaderPtr);
    }
    const gl::ResourceMap<gl::Program, gl::ShaderProgramID> &programManager =
        shaderProgramManager.getProgramsForCaptureAndPerf();
    for (const auto &program : programManager)
    {
        gl::Program *programPtr = program.second;
        SerializeProgram(json, programPtr);
    }
    const gl::TextureManager &textureManager = context->getState().getTextureManagerForCapture();
    for (const auto &texture : textureManager)
    {
        gl::Texture *texturePtr = texture.second;
        ANGLE_TRY(SerializeTexture(context, json, &scratchBuffer, texturePtr));
    }
    const gl::VertexArrayMap &vertexArrayMap = context->getVertexArraysForCapture();
    for (auto &vertexArray : vertexArrayMap)
    {
        gl::VertexArray *vertexArrayPtr = vertexArray.second;
        SerializeVertexArray(json, vertexArrayPtr);
    }
    json->endDocument();

    scratchBuffer.clear();
    return Result::Continue;
}

}  // namespace angle
