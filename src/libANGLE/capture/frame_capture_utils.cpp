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
#include "common/JsonSerializer.h"
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
    GroupScope(JsonSerializer *bos_, const std::string &name) : bos(bos_) { bos->startGroup(name); }

    template <typename Int>
    GroupScope(JsonSerializer *bos_, const std::string &name, Int index) : bos(bos_)
    {
        std::ostringstream os;
        os << name << index;
        bos->startGroup(os.str());
    }
    template <typename Int>
    GroupScope(JsonSerializer *bos_, Int index) : GroupScope(bos_, "", index)
    {}

    ~GroupScope() { bos->endGroup(); }

  private:
    JsonSerializer *bos;
};

void SerializeColorF(JsonSerializer *bos, const ColorF &color)
{
    bos->addEntry("red", color.red);
    bos->addEntry("green", color.green);
    bos->addEntry("blue", color.blue);
    bos->addEntry("alpha", color.alpha);
}

void SerializeColorI(JsonSerializer *bos, const ColorI &color)
{
    bos->addEntry("Red", color.red);
    bos->addEntry("Green", color.green);
    bos->addEntry("Blue", color.blue);
    bos->addEntry("Alpha", color.alpha);
}

void SerializeColorUI(JsonSerializer *bos, const ColorUI &color)
{
    bos->addEntry("Red", color.red);
    bos->addEntry("Green", color.green);
    bos->addEntry("Blue", color.blue);
    bos->addEntry("Alpha", color.alpha);
}

template <class ObjectType>
void SerializeOffsetBindingPointerVector(
    JsonSerializer *bos,
    const std::vector<gl::OffsetBindingPointer<ObjectType>> &offsetBindingPointerVector)
{
    for (size_t i = 0; i < offsetBindingPointerVector.size(); i++)
    {
        GroupScope group(bos, i);
        bos->addEntry("Value", offsetBindingPointerVector[i].id().value);
        bos->addEntry("Offset", offsetBindingPointerVector[i].getOffset());
        bos->addEntry("Size", offsetBindingPointerVector[i].getSize());
    }
}

template <class ObjectType>
void SerializeBindingPointerVector(
    JsonSerializer *bos,
    const std::vector<gl::BindingPointer<ObjectType>> &bindingPointerVector)
{
    for (size_t i = 0; i < bindingPointerVector.size(); i++)
    {
        std::ostringstream s;
        s << i;
        bos->addEntry(s.str().c_str(), bindingPointerVector[i].id().value);
    }
}

template <class T>
void SerializeRange(JsonSerializer *bos, const gl::Range<T> &range)
{
    GroupScope group(bos, "Range");
    bos->addEntry("Low", range.low());
    bos->addEntry("High", range.high());
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
void SerializeImageIndex(JsonSerializer *bos, const gl::ImageIndex &imageIndex)
{
    GroupScope group(bos, "Image");
    bos->addEntry("ImageType", TextureTypeToString(imageIndex.getType()));
    bos->addEntry("LevelIndex", imageIndex.getLevelIndex());
    bos->addEntry("LayerIndex", imageIndex.getLayerIndex());
    bos->addEntry("LayerCount", imageIndex.getLayerCount());
}

Result SerializeFramebufferAttachment(const gl::Context *context,
                                      JsonSerializer *bos,
                                      ScratchBuffer *scratchBuffer,
                                      gl::Framebuffer *framebuffer,
                                      const gl::FramebufferAttachment &framebufferAttachment)
{
    if (framebufferAttachment.type() == GL_TEXTURE ||
        framebufferAttachment.type() == GL_RENDERBUFFER)
    {
        bos->addEntry("ID", framebufferAttachment.id());
    }
    bos->addEntry("Type", framebufferAttachment.type());
    // serialize target variable
    bos->addEntry("Binding", framebufferAttachment.getBinding());
    if (framebufferAttachment.type() == GL_TEXTURE)
    {
        SerializeImageIndex(bos, framebufferAttachment.getTextureImageIndex());
    }
    bos->addEntry("NumViews", framebufferAttachment.getNumViews());
    bos->addEntry("Multiview", framebufferAttachment.isMultiview());
    bos->addEntry("ViewIndex", framebufferAttachment.getBaseViewIndex());
    bos->addEntry("Samples", framebufferAttachment.getRenderToTextureSamples());

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
        bos->addEntry("Data", pixelsPtr->data(), pixelsPtr->size());
        // Reset framebuffer state
        framebuffer->setReadBuffer(prevReadBufferState);
    }
    return Result::Continue;
}

Result SerializeFramebufferState(const gl::Context *context,
                                 JsonSerializer *bos,
                                 ScratchBuffer *scratchBuffer,
                                 gl::Framebuffer *framebuffer,
                                 const gl::FramebufferState &framebufferState)
{
    GroupScope group(bos, "Framebuffer");
    bos->addEntry("ID", framebufferState.id().value);
    bos->addEntry("Label", framebufferState.getLabel());
    bos->addEntry("DrawStates", framebufferState.getDrawBufferStates());
    bos->addEntry("ReadBufferState", framebufferState.getReadBufferState());
    bos->addEntry("DefaultWidth", framebufferState.getDefaultWidth());
    bos->addEntry("DefaultHeight", framebufferState.getDefaultHeight());
    bos->addEntry("DefaultSamples", framebufferState.getDefaultSamples());
    bos->addEntry("DefaultFixedSampleLocation", framebufferState.getDefaultFixedSampleLocations());
    bos->addEntry("DefaultLayers", framebufferState.getDefaultLayers());

    const std::vector<gl::FramebufferAttachment> &colorAttachments =
        framebufferState.getColorAttachments();
    for (const gl::FramebufferAttachment &colorAttachment : colorAttachments)
    {
        if (colorAttachment.isAttached())
        {
            GroupScope group(bos, "ColorAttachment");
            ANGLE_TRY(SerializeFramebufferAttachment(context, bos, scratchBuffer, framebuffer,
                                                     colorAttachment));
        }
    }
    if (framebuffer->getDepthStencilAttachment())
    {
        GroupScope group(bos, "DepthStencilAttachment");
        ANGLE_TRY(SerializeFramebufferAttachment(context, bos, scratchBuffer, framebuffer,
                                                 *framebuffer->getDepthStencilAttachment()));
    }
    else
    {
        if (framebuffer->getDepthAttachment())
        {
            GroupScope group(bos, "DepthAttachment");
            ANGLE_TRY(SerializeFramebufferAttachment(context, bos, scratchBuffer, framebuffer,
                                                     *framebuffer->getDepthAttachment()));
        }
        if (framebuffer->getStencilAttachment())
        {
            GroupScope group(bos, "StencilAttachment");
            ANGLE_TRY(SerializeFramebufferAttachment(context, bos, scratchBuffer, framebuffer,
                                                     *framebuffer->getStencilAttachment()));
        }
    }
    return Result::Continue;
}

Result SerializeFramebuffer(const gl::Context *context,
                            JsonSerializer *bos,
                            ScratchBuffer *scratchBuffer,
                            gl::Framebuffer *framebuffer)
{
    return SerializeFramebufferState(context, bos, scratchBuffer, framebuffer,
                                     framebuffer->getState());
}

void SerializeRasterizerState(JsonSerializer *bos, const gl::RasterizerState &rasterizerState)
{
    GroupScope group(bos, "Rasterizer");
    bos->addEntry("CullFace", rasterizerState.cullFace);
    bos->addEntry("CullMode", CullFaceModeToString(rasterizerState.cullMode));
    bos->addEntry("FrontFace", rasterizerState.frontFace);
    bos->addEntry("PolygonOffsetFill", rasterizerState.polygonOffsetFill);
    bos->addEntry("PolygonOffsetFactor", rasterizerState.polygonOffsetFactor);
    bos->addEntry("PolygonOffsetUnits", rasterizerState.polygonOffsetUnits);
    bos->addEntry("PointDrawMode", rasterizerState.pointDrawMode);
    bos->addEntry("MultiSample", rasterizerState.multiSample);
    bos->addEntry("RasterizerDiscard", rasterizerState.rasterizerDiscard);
    bos->addEntry("Dither", rasterizerState.dither);
}

void SerializeRectangle(JsonSerializer *bos,
                        const std::string &name,
                        const gl::Rectangle &rectangle)
{
    GroupScope group(bos, name);
    bos->addEntry("x", rectangle.x);
    bos->addEntry("y", rectangle.y);
    bos->addEntry("w", rectangle.width);
    bos->addEntry("h", rectangle.height);
}

void SerializeBlendStateExt(JsonSerializer *bos, const gl::BlendStateExt &blendStateExt)
{
    GroupScope group(bos, "BlendStateExt");
    bos->addEntry("MaxDrawBuffers", blendStateExt.mMaxDrawBuffers);
    bos->addEntry("enableMask", blendStateExt.mEnabledMask.bits());
    bos->addEntry("DstColor", blendStateExt.mDstColor);
    bos->addEntry("DstAlpha", blendStateExt.mDstAlpha);
    bos->addEntry("SrcColor", blendStateExt.mSrcColor);
    bos->addEntry("SrcAlpha", blendStateExt.mSrcAlpha);
    bos->addEntry("EquationColor", blendStateExt.mEquationColor);
    bos->addEntry("EquationAlpha", blendStateExt.mEquationAlpha);
    bos->addEntry("ColorMask", blendStateExt.mColorMask);
}

void SerializeDepthStencilState(JsonSerializer *bos, const gl::DepthStencilState &depthStencilState)
{
    GroupScope group(bos, "DepthStencilState");
    bos->addEntry("DepthTest", depthStencilState.depthTest);
    bos->addEntry("DepthFunc", depthStencilState.depthFunc);
    bos->addEntry("DepthMask", depthStencilState.depthMask);
    bos->addEntry("StencilTest", depthStencilState.stencilTest);
    bos->addEntry("StencilFunc", depthStencilState.stencilFunc);
    bos->addEntry("StencilMask", depthStencilState.stencilMask);
    bos->addEntry("StencilFail", depthStencilState.stencilFail);
    bos->addEntry("StencilPassDepthFail", depthStencilState.stencilPassDepthFail);
    bos->addEntry("StencilPassDepthPass", depthStencilState.stencilPassDepthPass);
    bos->addEntry("StencilWritemask", depthStencilState.stencilWritemask);
    bos->addEntry("StencilBackFunc", depthStencilState.stencilBackFunc);
    bos->addEntry("StencilBackMask", depthStencilState.stencilBackMask);
    bos->addEntry("StencilBackFail", depthStencilState.stencilBackFail);
    bos->addEntry("StencilBackPassDepthFail", depthStencilState.stencilBackPassDepthFail);
    bos->addEntry("StencilBackPassDepthPass", depthStencilState.stencilBackPassDepthPass);
    bos->addEntry("StencilBackWritemask", depthStencilState.stencilBackWritemask);
}

void SerializeVertexAttribCurrentValueData(
    JsonSerializer *bos,
    const gl::VertexAttribCurrentValueData &vertexAttribCurrentValueData)
{
    ASSERT(vertexAttribCurrentValueData.Type == gl::VertexAttribType::Float ||
           vertexAttribCurrentValueData.Type == gl::VertexAttribType::Int ||
           vertexAttribCurrentValueData.Type == gl::VertexAttribType::UnsignedInt);
    if (vertexAttribCurrentValueData.Type == gl::VertexAttribType::Float)
    {
        bos->addEntry("0", vertexAttribCurrentValueData.Values.FloatValues[0]);
        bos->addEntry("1", vertexAttribCurrentValueData.Values.FloatValues[1]);
        bos->addEntry("2", vertexAttribCurrentValueData.Values.FloatValues[2]);
        bos->addEntry("3", vertexAttribCurrentValueData.Values.FloatValues[3]);
    }
    else if (vertexAttribCurrentValueData.Type == gl::VertexAttribType::Int)
    {
        bos->addEntry("0", vertexAttribCurrentValueData.Values.IntValues[0]);
        bos->addEntry("1", vertexAttribCurrentValueData.Values.IntValues[1]);
        bos->addEntry("2", vertexAttribCurrentValueData.Values.IntValues[2]);
        bos->addEntry("3", vertexAttribCurrentValueData.Values.IntValues[3]);
    }
    else
    {
        bos->addEntry("0", vertexAttribCurrentValueData.Values.UnsignedIntValues[0]);
        bos->addEntry("1", vertexAttribCurrentValueData.Values.UnsignedIntValues[1]);
        bos->addEntry("2", vertexAttribCurrentValueData.Values.UnsignedIntValues[2]);
        bos->addEntry("3", vertexAttribCurrentValueData.Values.UnsignedIntValues[3]);
    }
}

void SerializePixelPackState(JsonSerializer *bos, const gl::PixelPackState &pixelPackState)
{
    GroupScope(bos, "PixelPackState");
    bos->addEntry("Alignment", pixelPackState.alignment);
    bos->addEntry("RowLength", pixelPackState.rowLength);
    bos->addEntry("SkipRows", pixelPackState.skipRows);
    bos->addEntry("SkipPixels", pixelPackState.skipPixels);
    bos->addEntry("ImageHeight", pixelPackState.imageHeight);
    bos->addEntry("SkipImages", pixelPackState.skipImages);
    bos->addEntry("ReverseRowOrder", pixelPackState.reverseRowOrder);
}

void SerializePixelUnpackState(JsonSerializer *bos, const gl::PixelUnpackState &pixelUnpackState)
{
    GroupScope(bos, "PixelUnpackState");
    bos->addEntry("Alignment", pixelUnpackState.alignment);
    bos->addEntry("RowLength", pixelUnpackState.rowLength);
    bos->addEntry("SkipRows", pixelUnpackState.skipRows);
    bos->addEntry("SkipPixels", pixelUnpackState.skipPixels);
    bos->addEntry("ImageHeight", pixelUnpackState.imageHeight);
    bos->addEntry("SkipImages", pixelUnpackState.skipImages);
}

void SerializeImageUnit(JsonSerializer *bos, const gl::ImageUnit &imageUnit)
{
    GroupScope(bos, "ImageUnit");
    bos->addEntry("Level", imageUnit.level);
    bos->addEntry("Layered", imageUnit.layered);
    bos->addEntry("Layer", imageUnit.layer);
    bos->addEntry("Access", imageUnit.access);
    bos->addEntry("Format", imageUnit.format);
    bos->addEntry("Texid", imageUnit.texture.id().value);
}

void SerializeGLContextStates(JsonSerializer *bos, const gl::State &state)
{
    GroupScope(bos, "ContextStates");
    bos->addEntry("ClientType", state.getClientType());
    bos->addEntry("Priority", state.getContextPriority());
    bos->addEntry("Major", state.getClientMajorVersion());
    bos->addEntry("Minor", state.getClientMinorVersion());

    SerializeColorF(bos, state.getColorClearValue());
    bos->addEntry("DepthClearValue", state.getDepthClearValue());
    bos->addEntry("StencilClearValue", state.getStencilClearValue());
    SerializeRasterizerState(bos, state.getRasterizerState());
    bos->addEntry("ScissorTestEnabled", state.isScissorTestEnabled());
    SerializeRectangle(bos, "Scissors", state.getScissor());
    SerializeBlendStateExt(bos, state.getBlendStateExt());
    SerializeColorF(bos, state.getBlendColor());
    bos->addEntry("SampleAlphaToCoverageEnabled", state.isSampleAlphaToCoverageEnabled());
    bos->addEntry("SampleCoverageEnabled", state.isSampleCoverageEnabled());
    bos->addEntry("SampleCoverageValue", state.getSampleCoverageValue());
    bos->addEntry("SampleCoverageInvert", state.getSampleCoverageInvert());
    bos->addEntry("SampleMaskEnabled", state.isSampleMaskEnabled());
    bos->addEntry("MaxSampleMaskWords", state.getMaxSampleMaskWords());
    const auto &sampleMaskValues = state.getSampleMaskValues();
    for (size_t i = 0; i < sampleMaskValues.size(); i++)
    {
        std::ostringstream os;
        os << i;
        bos->addEntry(os.str(), sampleMaskValues[i]);
    }
    SerializeDepthStencilState(bos, state.getDepthStencilState());
    bos->addEntry("StencilRef", state.getStencilRef());
    bos->addEntry("StencilBackRef", state.getStencilBackRef());
    bos->addEntry("LineWidth", state.getLineWidth());
    bos->addEntry("GenerateMipmapHint", state.getGenerateMipmapHint());
    bos->addEntry("TextureFilteringHint", state.getTextureFilteringHint());
    bos->addEntry("FragmentShaderDerivativeHint", state.getFragmentShaderDerivativeHint());
    bos->addEntry("BindGeneratesResourceEnabled", state.isBindGeneratesResourceEnabled());
    bos->addEntry("ClientArraysEnabled", state.areClientArraysEnabled());
    SerializeRectangle(bos, "Viewport", state.getViewport());
    bos->addEntry("Near", state.getNearPlane());
    bos->addEntry("Far", state.getFarPlane());
    if (state.getReadFramebuffer())
    {
        bos->addEntry("Framebuffer ID", state.getReadFramebuffer()->id().value);
    }
    if (state.getDrawFramebuffer())
    {
        bos->addEntry("Draw Framebuffer ID", state.getDrawFramebuffer()->id().value);
    }
    bos->addEntry("Renderbuffer ID", state.getRenderbufferId().value);
    if (state.getProgram())
    {
        bos->addEntry("ProgramID", state.getProgram()->id().value);
    }
    if (state.getProgramPipeline())
    {
        bos->addEntry("ProgramPipelineID", state.getProgramPipeline()->id().value);
    }
    bos->addEntry("ProvokingVertex", ProvokingVertexConventionToString(state.getProvokingVertex()));
    const std::vector<gl::VertexAttribCurrentValueData> &vertexAttribCurrentValues =
        state.getVertexAttribCurrentValues();
    for (size_t i = 0; i < vertexAttribCurrentValues.size(); i++)
    {
        GroupScope group(bos, "VertexAttribCurrentValues", i);
        SerializeVertexAttribCurrentValueData(bos, vertexAttribCurrentValues[i]);
    }
    if (state.getVertexArray())
    {
        bos->addEntry("VertexArrayID", state.getVertexArray()->id().value);
    }
    bos->addEntry("CurrentValuesTypeMask", state.getCurrentValuesTypeMask().to_ulong());
    bos->addEntry("ActiveSampler", state.getActiveSampler());
    for (const auto &textures : state.getBoundTexturesForCapture())
    {
        SerializeBindingPointerVector<gl::Texture>(bos, textures);
    }
    bos->addEntry("texturesIncompatibleWithSamplers",
                  state.getTexturesIncompatibleWithSamplers().to_ulong());
    SerializeBindingPointerVector<gl::Sampler>(bos, state.getSamplers());
    for (const gl::ImageUnit &imageUnit : state.getImageUnits())
    {
        SerializeImageUnit(bos, imageUnit);
    }
    for (const auto &query : state.getActiveQueriesForCapture())
    {
        bos->addEntry("Query", query.id().value);
    }
    for (const auto &boundBuffer : state.getBoundBuffersForCapture())
    {
        bos->addEntry("Bound", boundBuffer.id().value);
    }
    SerializeOffsetBindingPointerVector<gl::Buffer>(bos,
                                                    state.getOffsetBindingPointerUniformBuffers());
    SerializeOffsetBindingPointerVector<gl::Buffer>(
        bos, state.getOffsetBindingPointerAtomicCounterBuffers());
    SerializeOffsetBindingPointerVector<gl::Buffer>(
        bos, state.getOffsetBindingPointerShaderStorageBuffers());
    if (state.getCurrentTransformFeedback())
    {
        bos->addEntry("CurrentTransformFeedback", state.getCurrentTransformFeedback()->id().value);
    }
    SerializePixelUnpackState(bos, state.getUnpackState());
    SerializePixelPackState(bos, state.getPackState());
    bos->addEntry("PrimitiveRestartEnabled", state.isPrimitiveRestartEnabled());
    bos->addEntry("MultisamplingEnabled", state.isMultisamplingEnabled());
    bos->addEntry("SampleAlphaToOneEnabled", state.isSampleAlphaToOneEnabled());
    bos->addEntry("CoverageModulation", state.getCoverageModulation());
    bos->addEntry("FramebufferSRGB", state.getFramebufferSRGB());
    bos->addEntry("RobustResourceInitEnabled", state.isRobustResourceInitEnabled());
    bos->addEntry("ProgramBinaryCacheEnabled", state.isProgramBinaryCacheEnabled());
    bos->addEntry("TextureRectangleEnabled", state.isTextureRectangleEnabled());
    bos->addEntry("MaxShaderCompilerThreads", state.getMaxShaderCompilerThreads());
    bos->addEntry("EnabledClipDistances", state.getEnabledClipDistances().to_ulong());
    bos->addEntry("BlendFuncConstantAlphaDrawBuffers",
                  state.getBlendFuncConstantAlphaDrawBuffers().to_ulong());
    bos->addEntry("BlendFuncConstantColorDrawBuffers",
                  state.getBlendFuncConstantColorDrawBuffers().to_ulong());
    bos->addEntry("SimultaneousConstantColorAndAlphaBlendFunc",
                  state.noSimultaneousConstantColorAndAlphaBlendFunc());
}

void SerializeBufferState(JsonSerializer *bos, const gl::BufferState &bufferState)
{
    bos->addEntry("Label", bufferState.getLabel());
    bos->addEntry("Usage", BufferUsageToString(bufferState.getUsage()));
    bos->addEntry("Size", bufferState.getSize());
    bos->addEntry("AccessFlags", bufferState.getAccessFlags());
    bos->addEntry("Access", bufferState.getAccess());
    bos->addEntry("Mapped", bufferState.isMapped());
    bos->addEntry("MapOffset", bufferState.getMapOffset());
    bos->addEntry("MapLength", bufferState.getMapLength());
}

Result SerializeBuffer(const gl::Context *context,
                       JsonSerializer *bos,
                       ScratchBuffer *scratchBuffer,
                       gl::Buffer *buffer)
{
    GroupScope group(bos, "Buffer");
    SerializeBufferState(bos, buffer->getState());
    MemoryBuffer *dataPtr = nullptr;
    ANGLE_CHECK_GL_ALLOC(
        const_cast<gl::Context *>(context),
        scratchBuffer->getInitialized(static_cast<size_t>(buffer->getSize()), &dataPtr, 0));
    ANGLE_TRY(buffer->getSubData(context, 0, dataPtr->size(), dataPtr->data()));
    bos->addEntry("data", dataPtr->data(), dataPtr->size());
    return Result::Continue;
}
void SerializeColorGeneric(JsonSerializer *bos,
                           const std::string &name,
                           const ColorGeneric &colorGeneric)
{
    GroupScope group(bos, name);
    ASSERT(colorGeneric.type == ColorGeneric::Type::Float ||
           colorGeneric.type == ColorGeneric::Type::Int ||
           colorGeneric.type == ColorGeneric::Type::UInt);
    bos->addEntry("Type", ColorGenericTypeToString(colorGeneric.type));
    if (colorGeneric.type == ColorGeneric::Type::Float)
    {
        SerializeColorF(bos, colorGeneric.colorF);
    }
    else if (colorGeneric.type == ColorGeneric::Type::Int)
    {
        SerializeColorI(bos, colorGeneric.colorI);
    }
    else
    {
        SerializeColorUI(bos, colorGeneric.colorUI);
    }
}

void SerializeSamplerState(JsonSerializer *bos,
                           const std::string &name,
                           const gl::SamplerState &samplerState)
{
    GroupScope group(bos, name);
    bos->addEntry("MinFilter", samplerState.getMinFilter());
    bos->addEntry("MagFilter", samplerState.getMagFilter());
    bos->addEntry("WrapS", samplerState.getWrapS());
    bos->addEntry("WrapT", samplerState.getWrapT());
    bos->addEntry("WrapR", samplerState.getWrapR());
    bos->addEntry("MaxAnisotropy", samplerState.getMaxAnisotropy());
    bos->addEntry("MinLod", samplerState.getMinLod());
    bos->addEntry("MaxLod", samplerState.getMaxLod());
    bos->addEntry("CompareMode", samplerState.getCompareMode());
    bos->addEntry("CompareFunc", samplerState.getCompareFunc());
    bos->addEntry("SRGBDecode", samplerState.getSRGBDecode());
    SerializeColorGeneric(bos, "BorderColor", samplerState.getBorderColor());
}

void SerializeSampler(JsonSerializer *bos, gl::Sampler *sampler)
{
    bos->addEntry("Label", sampler->getLabel());
    SerializeSamplerState(bos, "Sampler", sampler->getSamplerState());
}

void SerializeSwizzleState(JsonSerializer *bos, const gl::SwizzleState &swizzleState)
{
    bos->addEntry("SwizzleRed", swizzleState.swizzleRed);
    bos->addEntry("SwizzleGreen", swizzleState.swizzleGreen);
    bos->addEntry("SwizzleBlue", swizzleState.swizzleBlue);
    bos->addEntry("SwizzleAlpha", swizzleState.swizzleAlpha);
}

void SerializeExtents(JsonSerializer *bos, const gl::Extents &extents)
{
    bos->addEntry("Width", extents.width);
    bos->addEntry("Height", extents.height);
    bos->addEntry("Depth", extents.depth);
}

void SerializeInternalFormat(JsonSerializer *bos, const gl::InternalFormat *internalFormat)
{
    bos->addEntry("InternalFormat", internalFormat->internalFormat);
}

void SerializeFormat(JsonSerializer *bos, const gl::Format &format)
{
    SerializeInternalFormat(bos, format.info);
}

void SerializeRenderbufferState(JsonSerializer *bos, const gl::RenderbufferState &renderbufferState)
{
    GroupScope wg(bos, "State");
    bos->addEntry("Width", renderbufferState.getWidth());
    bos->addEntry("Height", renderbufferState.getHeight());
    SerializeFormat(bos, renderbufferState.getFormat());
    bos->addEntry("Samples", renderbufferState.getSamples());
    bos->addEntry("InitState", InitStateToString(renderbufferState.getInitState()));
}

Result SerializeRenderbuffer(const gl::Context *context,
                             JsonSerializer *bos,
                             ScratchBuffer *scratchBuffer,
                             gl::Renderbuffer *renderbuffer)
{
    GroupScope wg(bos, "Renderbuffer");
    SerializeRenderbufferState(bos, renderbuffer->getState());
    bos->addEntry("Label", renderbuffer->getLabel());
    MemoryBuffer *pixelsPtr = nullptr;
    ANGLE_CHECK_GL_ALLOC(
        const_cast<gl::Context *>(context),
        scratchBuffer->getInitialized(renderbuffer->getMemorySize(), &pixelsPtr, 0));
    gl::PixelPackState packState;
    packState.alignment = 1;
    ANGLE_TRY(renderbuffer->getImplementation()->getRenderbufferImage(
        context, packState, nullptr, renderbuffer->getImplementationColorReadFormat(context),
        renderbuffer->getImplementationColorReadType(context), pixelsPtr->data()));
    bos->addEntry("pixel", pixelsPtr->data(), pixelsPtr->size());
    return Result::Continue;
}

void SerializeWorkGroupSize(JsonSerializer *bos, const sh::WorkGroupSize &workGroupSize)
{
    GroupScope wg(bos, "workGroupSize");
    bos->addEntry("x", workGroupSize[0]);
    bos->addEntry("y", workGroupSize[1]);
    bos->addEntry("z", workGroupSize[2]);
}

void SerializeShaderVariable(JsonSerializer *bos, const sh::ShaderVariable &shaderVariable)
{
    GroupScope wg(bos, "ShaderVariable");
    bos->addEntry("Type", shaderVariable.type);
    bos->addEntry("Precision", shaderVariable.precision);
    bos->addEntry("Name", shaderVariable.name);
    bos->addEntry("MappedName", shaderVariable.mappedName);
    bos->addEntry("ArraySizes", shaderVariable.arraySizes);
    bos->addEntry("StaticUse", shaderVariable.staticUse);
    bos->addEntry("Active", shaderVariable.active);
    for (const sh::ShaderVariable &field : shaderVariable.fields)
    {
        SerializeShaderVariable(bos, field);
    }
    bos->addEntry("StructOrBlockName", shaderVariable.structOrBlockName);
    bos->addEntry("MappedStructOrBlockName", shaderVariable.mappedStructOrBlockName);
    bos->addEntry("RowMajorLayout", shaderVariable.isRowMajorLayout);
    bos->addEntry("Location", shaderVariable.location);
    bos->addEntry("Binding", shaderVariable.binding);
    bos->addEntry("ImageUnitFormat", shaderVariable.imageUnitFormat);
    bos->addEntry("Offset", shaderVariable.offset);
    bos->addEntry("Readonly", shaderVariable.readonly);
    bos->addEntry("Writeonly", shaderVariable.writeonly);
    bos->addEntry("Index", shaderVariable.index);
    bos->addEntry("YUV", shaderVariable.yuv);
    bos->addEntry("Interpolation", InterpolationTypeToString(shaderVariable.interpolation));
    bos->addEntry("Invariant", shaderVariable.isInvariant);
    bos->addEntry("TexelFetchStaticUse", shaderVariable.texelFetchStaticUse);
}

void SerializeShaderVariablesVector(JsonSerializer *bos,
                                    const std::vector<sh::ShaderVariable> &shaderVariables)
{
    for (const sh::ShaderVariable &shaderVariable : shaderVariables)
    {
        SerializeShaderVariable(bos, shaderVariable);
    }
}

void SerializeInterfaceBlocksVector(JsonSerializer *bos,
                                    const std::vector<sh::InterfaceBlock> &interfaceBlocks)
{
    for (const sh::InterfaceBlock &interfaceBlock : interfaceBlocks)
    {
        GroupScope group(bos, "Interface Block");
        bos->addEntry("Name", interfaceBlock.name);
        bos->addEntry("MappedName", interfaceBlock.mappedName);
        bos->addEntry("InstanceName", interfaceBlock.instanceName);
        bos->addEntry("ArraySize", interfaceBlock.arraySize);
        bos->addEntry("Layout", BlockLayoutTypeToString(interfaceBlock.layout));
        bos->addEntry("Binding", interfaceBlock.binding);
        bos->addEntry("StaticUse", interfaceBlock.staticUse);
        bos->addEntry("Active", interfaceBlock.active);
        bos->addEntry("BlockType", BlockTypeToString(interfaceBlock.blockType));
        SerializeShaderVariablesVector(bos, interfaceBlock.fields);
    }
}

void SerializeShaderState(JsonSerializer *bos, const gl::ShaderState &shaderState)
{
    GroupScope group(bos, "ShaderState");
    bos->addEntry("Label", shaderState.getLabel());
    bos->addEntry("Type", gl::ShaderTypeToString(shaderState.getShaderType()));
    bos->addEntry("Version", shaderState.getShaderVersion());
    bos->addEntry("TranslatedSource", shaderState.getTranslatedSource());
    bos->addEntry("Source", shaderState.getSource());
    SerializeWorkGroupSize(bos, shaderState.getLocalSize());
    SerializeShaderVariablesVector(bos, shaderState.getInputVaryings());
    SerializeShaderVariablesVector(bos, shaderState.getOutputVaryings());
    SerializeShaderVariablesVector(bos, shaderState.getUniforms());
    SerializeInterfaceBlocksVector(bos, shaderState.getUniformBlocks());
    SerializeInterfaceBlocksVector(bos, shaderState.getShaderStorageBlocks());
    SerializeShaderVariablesVector(bos, shaderState.getAllAttributes());
    SerializeShaderVariablesVector(bos, shaderState.getActiveAttributes());
    SerializeShaderVariablesVector(bos, shaderState.getActiveOutputVariables());
    bos->addEntry("EarlyFragmentTestsOptimization",
                  shaderState.getEarlyFragmentTestsOptimization());
    bos->addEntry("NumViews", shaderState.getNumViews());
    bos->addEntry("SpecConstUsageBits", shaderState.getSpecConstUsageBits().bits());
    if (shaderState.getGeometryShaderInputPrimitiveType().valid())
    {
        bos->addEntry(
            "GeometryShaderInputPrimitiveType",
            PrimitiveModeToString(shaderState.getGeometryShaderInputPrimitiveType().value()));
    }
    if (shaderState.getGeometryShaderOutputPrimitiveType().valid())
    {
        bos->addEntry(
            "GeometryShaderOutputPrimitiveType",
            PrimitiveModeToString(shaderState.getGeometryShaderOutputPrimitiveType().value()));
    }
    if (shaderState.getGeometryShaderInvocations().valid())
    {
        bos->addEntry("GeometryShaderInvocations",
                      shaderState.getGeometryShaderInvocations().value());
    }
    bos->addEntry("CompileStatus", CompileStatusToString(shaderState.getCompileStatus()));
}

void SerializeShader(JsonSerializer *bos, gl::Shader *shader)
{
    GroupScope group(bos, "Shader");
    SerializeShaderState(bos, shader->getState());
    bos->addEntry("Handle", shader->getHandle().value);
    bos->addEntry("RefCount", shader->getRefCount());
    bos->addEntry("FlaggedForDeletion", shader->isFlaggedForDeletion());
    // Do not serialize mType because it is already serialized in SerializeShaderState.
    bos->addEntry("InfoLogString", shader->getInfoLogString());
    // Do not serialize compiler resources string because it can vary between test modes.
    bos->addEntry("CurrentMaxComputeWorkGroupInvocations",
                  shader->getCurrentMaxComputeWorkGroupInvocations());
    bos->addEntry("MaxComputeSharedMemory", shader->getMaxComputeSharedMemory());
}

void SerializeVariableLocationsVector(JsonSerializer *bos,
                                      const std::string &group_name,
                                      const std::vector<gl::VariableLocation> &variableLocations)
{
    GroupScope group(bos, group_name);
    for (const gl::VariableLocation &variableLocation : variableLocations)
    {
        GroupScope group(bos, "Variable");
        bos->addEntry("ArrayIndex", variableLocation.arrayIndex);
        bos->addEntry("Index", variableLocation.index);
        bos->addEntry("Ignored", variableLocation.ignored);
    }
}

void SerializeBlockMemberInfo(JsonSerializer *bos, const sh::BlockMemberInfo &blockMemberInfo)
{
    GroupScope(bos, "BlockMemberInfo");
    bos->addEntry("Offset", blockMemberInfo.offset);
    bos->addEntry("Stride", blockMemberInfo.arrayStride);
    bos->addEntry("MatrixStride", blockMemberInfo.matrixStride);
    bos->addEntry("IsRowMajorMatrix", blockMemberInfo.isRowMajorMatrix);
    bos->addEntry("TopLevelArrayStride", blockMemberInfo.topLevelArrayStride);
}

void SerializeActiveVariable(JsonSerializer *bos, const gl::ActiveVariable &activeVariable)
{
    bos->addEntry("ActiveShaders", activeVariable.activeShaders().to_ulong());
}

void SerializeBufferVariablesVector(JsonSerializer *bos,
                                    const std::vector<gl::BufferVariable> &bufferVariables)
{
    for (const gl::BufferVariable &bufferVariable : bufferVariables)
    {
        GroupScope(bos, "BufferVariable");
        bos->addEntry("BufferIndex", bufferVariable.bufferIndex);
        SerializeBlockMemberInfo(bos, bufferVariable.blockInfo);
        bos->addEntry("TopLevelArraySize", bufferVariable.topLevelArraySize);
        SerializeActiveVariable(bos, bufferVariable);
        SerializeShaderVariable(bos, bufferVariable);
    }
}

void SerializeProgramAliasedBindings(JsonSerializer *bos,
                                     const gl::ProgramAliasedBindings &programAliasedBindings)
{
    for (const auto &programAliasedBinding : programAliasedBindings)
    {
        GroupScope group(bos, programAliasedBinding.first);
        bos->addEntry("Location", programAliasedBinding.second.location);
        bos->addEntry("Aliased", programAliasedBinding.second.aliased);
    }
}

void SerializeProgramState(JsonSerializer *bos, const gl::ProgramState &programState)
{
    bos->addEntry("Label", programState.getLabel());
    SerializeWorkGroupSize(bos, programState.getComputeShaderLocalSize());
    for (gl::Shader *shader : programState.getAttachedShaders())
    {
        if (shader)
        {
            bos->addEntry("Handle", shader->getHandle().value);
        }
        else
        {
            bos->addEntry("Handle", 0);
        }
    }
    const gl::ShaderMap<bool> sm = programState.getAttachedShadersMarkedForDetach();
    for (auto i = 0; i < static_cast<int>(gl::ShaderType::EnumCount); ++i)
    {
        auto shaderType = static_cast<gl::ShaderType>(i);
        if (sm[shaderType])
            bos->addEntry("Attached", gl::ShaderTypeToString(shaderType));
    }
    bos->addEntry("LocationsUsedForXfbExtension", programState.getLocationsUsedForXfbExtension());
    for (const std::string &transformFeedbackVaryingName :
         programState.getTransformFeedbackVaryingNames())
    {
        bos->addEntry("TransformFeedbackVaryingName", transformFeedbackVaryingName);
    }
    bos->addEntry("ActiveUniformBlockBindingsMask",
                  programState.getActiveUniformBlockBindingsMask().to_ulong());
    SerializeVariableLocationsVector(bos, "UniformLocations", programState.getUniformLocations());
    SerializeBufferVariablesVector(bos, programState.getBufferVariables());
    SerializeRange(bos, programState.getAtomicCounterUniformRange());
    SerializeVariableLocationsVector(bos, "SecondaryOutputLocations",
                                     programState.getSecondaryOutputLocations());
    bos->addEntry("ActiveOutputVariables", programState.getActiveOutputVariables().to_ulong());
    for (GLenum outputVariableType : programState.getOutputVariableTypes())
    {
        bos->addEntry("OutputVariableType", outputVariableType);
    }
    bos->addEntry("DrawBufferTypeMask", programState.getDrawBufferTypeMask().to_ulong());
    bos->addEntry("BinaryRetrieveableHint", programState.hasBinaryRetrieveableHint());
    bos->addEntry("Separable", programState.isSeparable());
    bos->addEntry("EarlyFragmentTestsOptimization",
                  programState.hasEarlyFragmentTestsOptimization());
    bos->addEntry("NumViews", programState.getNumViews());
    bos->addEntry("DrawIDLocation", programState.getDrawIDLocation());
    bos->addEntry("BaseVertexLocation", programState.getBaseVertexLocation());
    bos->addEntry("BaseInstanceLocation", programState.getBaseInstanceLocation());
    SerializeProgramAliasedBindings(bos, programState.getUniformLocationBindings());
}

void SerializeProgramBindings(JsonSerializer *bos, const gl::ProgramBindings &programBindings)
{
    for (const auto &programBinding : programBindings)
    {
        bos->addEntry(programBinding.first, programBinding.second);
    }
}

void SerializeProgram(JsonSerializer *bos, gl::Program *program)
{
    GroupScope group(bos, "Program");
    SerializeProgramState(bos, program->getState());
    bos->addEntry("IsValidated", program->isValidated());
    SerializeProgramBindings(bos, program->getAttributeBindings());
    SerializeProgramAliasedBindings(bos, program->getFragmentOutputLocations());
    SerializeProgramAliasedBindings(bos, program->getFragmentOutputIndexes());
    bos->addEntry("IsLinked", program->isLinked());
    bos->addEntry("IsFlaggedForDeletion", program->isFlaggedForDeletion());
    bos->addEntry("RefCount", program->getRefCount());
    bos->addEntry("ID", program->id().value);
}

void SerializeImageDesc(JsonSerializer *bos, const gl::ImageDesc &imageDesc)
{
    GroupScope group(bos, "ImageDesc");
    SerializeExtents(bos, imageDesc.size);
    SerializeFormat(bos, imageDesc.format);
    bos->addEntry("Samples", imageDesc.samples);
    bos->addEntry("FixesSampleLocations", imageDesc.fixedSampleLocations);
    bos->addEntry("InitState", InitStateToString(imageDesc.initState));
}

void SerializeTextureState(JsonSerializer *bos, const gl::TextureState &textureState)
{
    bos->addEntry("Type", TextureTypeToString(textureState.getType()));
    SerializeSwizzleState(bos, textureState.getSwizzleState());
    SerializeSamplerState(bos, "TextureState", textureState.getSamplerState());
    bos->addEntry("SRGB", SrgbOverrideToString(textureState.getSRGBOverride()));
    bos->addEntry("BaseLevel", textureState.getBaseLevel());
    bos->addEntry("MaxLevel", textureState.getMaxLevel());
    bos->addEntry("DepthStencilTextureMode", textureState.getDepthStencilTextureMode());
    bos->addEntry("BeenBoundAsImage", textureState.hasBeenBoundAsImage());
    bos->addEntry("ImmutableFormat", textureState.getImmutableFormat());
    bos->addEntry("ImmutableLevels", textureState.getImmutableLevels());
    bos->addEntry("Usage", textureState.getUsage());
    const std::vector<gl::ImageDesc> &imageDescs = textureState.getImageDescs();
    for (const gl::ImageDesc &imageDesc : imageDescs)
    {
        SerializeImageDesc(bos, imageDesc);
    }
    SerializeRectangle(bos, "Crop", textureState.getCrop());

    bos->addEntry("GenerateMipmapHint", textureState.getGenerateMipmapHint());
    bos->addEntry("InitState", InitStateToString(textureState.getInitState()));
}

Result SerializeTextureData(JsonSerializer *bos,
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
        bos->addEntry("Texels", texelsPtr->data(), texelsPtr->size());
    }
    return Result::Continue;
}

Result SerializeTexture(const gl::Context *context,
                        JsonSerializer *bos,
                        ScratchBuffer *scratchBuffer,
                        gl::Texture *texture)
{
    GroupScope group(bos, "Texture ", texture->getId());
    SerializeTextureState(bos, texture->getState());
    bos->addEntry("Label", texture->getLabel());
    // FrameCapture can not serialize mBoundSurface and mBoundStream
    // because they are likely to change with each run
    ANGLE_TRY(SerializeTextureData(bos, context, texture, scratchBuffer));
    return Result::Continue;
}

void SerializeFormat(JsonSerializer *bos, const angle::Format *format)
{
    bos->addEntry("InternalFormat", format->glInternalFormat);
}

void SerializeVertexAttributeVector(JsonSerializer *bos,
                                    const std::vector<gl::VertexAttribute> &vertexAttributes)
{
    for (const gl::VertexAttribute &vertexAttribute : vertexAttributes)
    {
        GroupScope group(bos, "VertexAttribute@BindingIndex", vertexAttribute.bindingIndex);
        bos->addEntry("Enabled", vertexAttribute.enabled);
        ASSERT(vertexAttribute.format);
        SerializeFormat(bos, vertexAttribute.format);
        bos->addEntry("RelativeOffset", vertexAttribute.relativeOffset);
        bos->addEntry("VertexAttribArrayStride", vertexAttribute.vertexAttribArrayStride);
    }
}

void SerializeVertexBindingsVector(JsonSerializer *bos,
                                   const std::vector<gl::VertexBinding> &vertexBindings)
{
    for (const gl::VertexBinding &vertexBinding : vertexBindings)
    {
        GroupScope group(bos, "VertexBinding");
        bos->addEntry("Stride", vertexBinding.getStride());
        bos->addEntry("Divisor", vertexBinding.getDivisor());
        bos->addEntry("Offset", vertexBinding.getOffset());
        bos->addEntry("BufferID", vertexBinding.getBuffer().id().value);
        bos->addEntry("BoundAttributesMask", vertexBinding.getBoundAttributesMask().to_ulong());
    }
}

void SerializeVertexArrayState(JsonSerializer *bos, const gl::VertexArrayState &vertexArrayState)
{
    bos->addEntry("Label", vertexArrayState.getLabel());
    SerializeVertexAttributeVector(bos, vertexArrayState.getVertexAttributes());
    if (vertexArrayState.getElementArrayBuffer())
    {
        bos->addEntry("ElementArrayBufferID", vertexArrayState.getElementArrayBuffer()->id().value);
    }
    else
    {
        bos->addEntry("ElementArrayBufferID", 0);
    }
    SerializeVertexBindingsVector(bos, vertexArrayState.getVertexBindings());
    bos->addEntry("EnabledAttributesMask", vertexArrayState.getEnabledAttributesMask().to_ulong());
    bos->addEntry("VertexAttributesTypeMask",
                  vertexArrayState.getVertexAttributesTypeMask().to_ulong());
    bos->addEntry("ClientMemoryAttribsMask",
                  vertexArrayState.getClientMemoryAttribsMask().to_ulong());
    bos->addEntry("NullPointerClientMemoryAttribsMask",
                  vertexArrayState.getNullPointerClientMemoryAttribsMask().to_ulong());
}

void SerializeVertexArray(JsonSerializer *bos, gl::VertexArray *vertexArray)
{
    GroupScope group(bos, "VertexArray", vertexArray->id().value);
    SerializeVertexArrayState(bos, vertexArray->getState());
    bos->addEntry("BufferAccessValidationEnabled", vertexArray->isBufferAccessValidationEnabled());
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
