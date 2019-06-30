//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// OverlayVk.cpp:
//    Implements the OverlayVk class.
//

#include "libANGLE/renderer/vulkan/OverlayVk.h"
#include "common/system_utils.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/OverlayVk_font_autogen.h"

#include <numeric>

namespace rx
{
namespace
{
// A specialized time query implementation that duplicates some of `util/Timer.h`.  This avoids
// angle including util/ files.
#if defined(ANGLE_PLATFORM_WINDOWS)

#    include <windows.h>
double GetCurrentTime()
{
    LARGE_INTEGER frequency = {};
    QueryPerformanceFrequency(&frequency);

    LARGE_INTEGER curTime;
    QueryPerformanceCounter(&curTime);

    return static_cast<double>(curTime.QuadPart) / frequency.QuadPart;
}

#elif defined(ANGLE_PLATFORM_APPLE)

#    include <CoreServices/CoreServices.h>
#    include <mach/mach.h>
#    include <mach/mach_time.h>
double GetCurrentTime()
{
    mach_timebase_info_data_t timebaseInfo;
    mach_timebase_info(&timebaseInfo);

    double secondCoeff = timebaseInfo.numer * 1e-9 / timebaseInfo.denom;
    return secondCoeff * mach_absolute_time();
}

#else

double GetCurrentTime()
{
    struct timespec currentTime;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    return currentTime.tv_sec + currentTime.tv_nsec * 1e-9;
}

#endif

template <typename T>
class ScopedObject final : angle::NonCopyable
{
  public:
    ScopedObject(ContextVk *contextVk) : mContextVk(contextVk) {}
    ~ScopedObject() { mVar.release(mContextVk); }

    const T &get() const { return mVar; }
    T &get() { return mVar; }

    T &&release() { return std::move(mVar); }

  private:
    ContextVk *mContextVk;
    T mVar;
};

enum class OverlayInternalType
{
    Text,
    Graph,

    InvalidEnum,
    EnumCount = InvalidEnum,
};

constexpr angle::PackedEnumMap<OverlayType, OverlayInternalType> kOverlayTypeToInternalMap = {
    {OverlayType::Count, OverlayInternalType::Text},
    {OverlayType::Text, OverlayInternalType::Text},
    {OverlayType::PerSecond, OverlayInternalType::Text},
    {OverlayType::RunningGraph, OverlayInternalType::Graph},
    {OverlayType::RunningHistogram, OverlayInternalType::Graph},
};

// Structures and limits matching uniform buffers in OverlayDraw.comp
constexpr size_t kMaxRenderableTextItems  = 32;
constexpr size_t kMaxRenderableGraphItems = 32;
constexpr size_t kMaxTextItemLength       = 256;
constexpr size_t kMaxGraphItemCount       = 64;

constexpr angle::PackedEnumMap<OverlayInternalType, size_t> kOverlayInternalTypeMaxItems = {
    {OverlayInternalType::Text, kMaxRenderableTextItems},
    {OverlayInternalType::Graph, kMaxRenderableGraphItems},
};

constexpr angle::PackedEnumMap<OverlayInternalType, size_t> kOverlayInternalTypeItemOffsets = {
    {OverlayInternalType::Text, 0},
    {OverlayInternalType::Graph, kMaxRenderableTextItems},
};

ANGLE_ENABLE_STRUCT_PADDING_WARNINGS

// Structure matching buffer in OverlayCull.comp.
struct ItemCoordinates
{
    uint32_t coordinates[kMaxRenderableTextItems + kMaxRenderableGraphItems][4];
};

// Structures matching buffers in OverlayDraw.comp.
struct TextItemData
{
    uint32_t coordinates[4];
    float color[4];
    uint32_t fontSize[3];
    uint32_t padding;
    uint8_t text[kMaxTextItemLength];
};

struct GraphItemData
{
    uint32_t coordinates[4];
    float color[4];
    uint32_t valueWidth;
    uint32_t padding[3];
    uint32_t values[kMaxGraphItemCount];
};

struct TextItems
{
    TextItemData items[kMaxRenderableTextItems];
};

struct GraphItems
{
    GraphItemData items[kMaxRenderableGraphItems];
};

ANGLE_DISABLE_STRUCT_PADDING_WARNINGS

uint32_t GetItemCoord(int32_t src, uint32_t extent)
{
    int32_t dst = src < 0 ? extent + src : src;

    return std::min<uint32_t>(std::max(dst, 0), extent - 1);
}

void GetItemCoordinates(const int32_t srcCoords[4],
                        const VkExtent2D &imageExtent,
                        uint32_t dstCoordsOut[4])
{
    dstCoordsOut[0] = GetItemCoord(srcCoords[0], imageExtent.width);
    dstCoordsOut[1] = GetItemCoord(srcCoords[1], imageExtent.height);
    dstCoordsOut[2] = GetItemCoord(srcCoords[2], imageExtent.width);
    dstCoordsOut[3] = GetItemCoord(srcCoords[3], imageExtent.height);
}

void GetItemColor(const float srcColor[4], float dstColor[4])
{
    memcpy(dstColor, srcColor, 4 * sizeof(dstColor[0]));
}

void GetTextFontSize(int srcFontSize, uint32_t dstFontSize[3])
{
    // .xy contains the font glyph width/height
    dstFontSize[0] = overlay::kFontGlyphWidths[srcFontSize];
    dstFontSize[1] = overlay::kFontGlyphHeights[srcFontSize];
    // .z contains the layer
    dstFontSize[2] = srcFontSize;
}

void GetGraphValueWidth(const int32_t srcCoords[4], size_t valueCount, uint32_t *dstValueWidth)
{
    const int32_t graphWidth = std::abs(srcCoords[2] - srcCoords[0]);

    // If valueCount doesn't divide graphWidth, the graph bars won't fit well in its frame.
    // Fix initOverlayItems() in that case.
    ASSERT(graphWidth % valueCount == 0);

    *dstValueWidth = graphWidth / valueCount;
}

void GetTextString(const std::string &src, uint8_t textOut[kMaxTextItemLength])
{
    for (size_t i = 0; i < src.length() && i < kMaxTextItemLength; ++i)
    {
        // The font image has 96 ASCII characters starting from ' '.
        textOut[i] = src[i] - ' ';
    }
}

void GetGraphValues(const std::vector<size_t> srcValues,
                    size_t startIndex,
                    float scale,
                    uint32_t valuesOut[kMaxGraphItemCount])
{
    ASSERT(srcValues.size() <= kMaxGraphItemCount);

    for (size_t i = 0; i < srcValues.size(); ++i)
    {
        size_t index = (startIndex + i) % srcValues.size();
        valuesOut[i] = static_cast<uint32_t>(srcValues[index] * scale);
    }
}

std::vector<size_t> CreateHistogram(const std::vector<size_t> values)
{
    std::vector<size_t> histogram(values.size(), 0);

    for (size_t rank : values)
    {
        ++histogram[rank];
    }

    return histogram;
}

using OverlayItemCounts  = angle::PackedEnumMap<OverlayInternalType, size_t>;
using AppendItemDataFunc = void (*)(const OverlayVk::OverlayItem *item,
                                    const VkExtent2D &imageExtent,
                                    TextItemData *textItem,
                                    GraphItemData *graphItem,
                                    OverlayItemCounts *itemCounts);
}  // namespace

namespace overlay_impl
{
class AppendItemDataHelper
{
  public:
    static void AppendFPS(const OverlayVk::OverlayItem *item,
                          const VkExtent2D &imageExtent,
                          TextItemData *textItem,
                          GraphItemData *graphItem,
                          OverlayItemCounts *itemCounts);
    static void AppendLastValidationMessage(const OverlayVk::OverlayItem *item,
                                            const VkExtent2D &imageExtent,
                                            TextItemData *textItem,
                                            GraphItemData *graphItem,
                                            OverlayItemCounts *itemCounts);
    static void AppendValidationMessageCount(const OverlayVk::OverlayItem *item,
                                             const VkExtent2D &imageExtent,
                                             TextItemData *textItem,
                                             GraphItemData *graphItem,
                                             OverlayItemCounts *itemCounts);
    static void AppendCommandGraphSize(const OverlayVk::OverlayItem *item,
                                       const VkExtent2D &imageExtent,
                                       TextItemData *textItem,
                                       GraphItemData *graphItem,
                                       OverlayItemCounts *itemCounts);
    static void AppendSecondaryCommandBufferPoolWaste(const OverlayVk::OverlayItem *item,
                                                      const VkExtent2D &imageExtent,
                                                      TextItemData *textItem,
                                                      GraphItemData *graphItem,
                                                      OverlayItemCounts *itemCounts);

  private:
    static std::ostream &OutputPerSecond(std::ostream &out, const OverlayVk::PerSecond *perSecond);

    static std::ostream &OutputText(std::ostream &out, const OverlayVk::Text *text);

    static std::ostream &OutputCount(std::ostream &out, const OverlayVk::Count *count);

    static void AppendTextCommon(const OverlayVk::OverlayItem *item,
                                 const VkExtent2D &imageExtent,
                                 const std::string &text,
                                 TextItemData *textItem,
                                 OverlayItemCounts *itemCounts);

    static void AppendGraphCommon(const OverlayVk::OverlayItem *item,
                                  const VkExtent2D &imageExtent,
                                  const std::vector<size_t> runningValues,
                                  size_t startIndex,
                                  float scale,
                                  GraphItemData *graphItem,
                                  OverlayItemCounts *itemCounts);
};

void AppendItemDataHelper::AppendTextCommon(const OverlayVk::OverlayItem *item,
                                            const VkExtent2D &imageExtent,
                                            const std::string &text,
                                            TextItemData *textItem,
                                            OverlayItemCounts *itemCounts)
{
    GetItemCoordinates(item->coords, imageExtent, textItem->coordinates);
    GetItemColor(item->color, textItem->color);
    GetTextFontSize(item->fontSize, textItem->fontSize);
    GetTextString(text, textItem->text);

    ++(*itemCounts)[OverlayInternalType::Text];
}

void AppendItemDataHelper::AppendGraphCommon(const OverlayVk::OverlayItem *item,
                                             const VkExtent2D &imageExtent,
                                             const std::vector<size_t> runningValues,
                                             size_t startIndex,
                                             float scale,
                                             GraphItemData *graphItem,
                                             OverlayItemCounts *itemCounts)
{
    const OverlayVk::RunningGraph *itemAsGraph = static_cast<const OverlayVk::RunningGraph *>(item);

    GetItemCoordinates(item->coords, imageExtent, graphItem->coordinates);
    GetItemColor(item->color, graphItem->color);
    GetGraphValueWidth(item->coords, itemAsGraph->runningValues.size(), &graphItem->valueWidth);
    GetGraphValues(runningValues, startIndex, scale, graphItem->values);

    ++(*itemCounts)[OverlayInternalType::Graph];
}

void AppendItemDataHelper::AppendFPS(const OverlayVk::OverlayItem *item,
                                     const VkExtent2D &imageExtent,
                                     TextItemData *textItem,
                                     GraphItemData *graphItem,
                                     OverlayItemCounts *itemCounts)
{
    const OverlayVk::PerSecond *fps = static_cast<const OverlayVk::PerSecond *>(item);
    std::ostringstream text;
    text << "FPS: ";
    OutputPerSecond(text, fps);

    AppendTextCommon(item, imageExtent, text.str(), textItem, itemCounts);
}

void AppendItemDataHelper::AppendLastValidationMessage(const OverlayVk::OverlayItem *item,
                                                       const VkExtent2D &imageExtent,
                                                       TextItemData *textItem,
                                                       GraphItemData *graphItem,
                                                       OverlayItemCounts *itemCounts)
{
    const OverlayVk::Text *lastValidationMessage = static_cast<const OverlayVk::Text *>(item);
    std::ostringstream text;
    text << "Last VVL Message: ";
    OutputText(text, lastValidationMessage);

    AppendTextCommon(item, imageExtent, text.str(), textItem, itemCounts);
}

void AppendItemDataHelper::AppendValidationMessageCount(const OverlayVk::OverlayItem *item,
                                                        const VkExtent2D &imageExtent,
                                                        TextItemData *textItem,
                                                        GraphItemData *graphItem,
                                                        OverlayItemCounts *itemCounts)
{
    const OverlayVk::Count *validationMessageCount = static_cast<const OverlayVk::Count *>(item);
    std::ostringstream text;
    text << "VVL Message Count: ";
    OutputCount(text, validationMessageCount);

    AppendTextCommon(item, imageExtent, text.str(), textItem, itemCounts);
}

void AppendItemDataHelper::AppendCommandGraphSize(const OverlayVk::OverlayItem *item,
                                                  const VkExtent2D &imageExtent,
                                                  TextItemData *textItem,
                                                  GraphItemData *graphItem,
                                                  OverlayItemCounts *itemCounts)
{
    const OverlayVk::RunningGraph *commandGraphSize =
        static_cast<const OverlayVk::RunningGraph *>(item);

    const size_t maxValue     = *std::max_element(commandGraphSize->runningValues.begin(),
                                              commandGraphSize->runningValues.end());
    const int32_t graphHeight = std::abs(item->coords[3] - item->coords[1]);
    const float graphScale    = static_cast<float>(graphHeight) / maxValue;

    AppendGraphCommon(item, imageExtent, commandGraphSize->runningValues,
                      commandGraphSize->lastValueIndex + 1, graphScale, graphItem, itemCounts);

    if ((*itemCounts)[OverlayInternalType::Text] <
        kOverlayInternalTypeMaxItems[OverlayInternalType::Text])
    {
        std::ostringstream text;
        text << "Command Graph Size (Max: " << maxValue << ")";
        AppendTextCommon(&commandGraphSize->description, imageExtent, text.str(), textItem,
                         itemCounts);
    }
}

void AppendItemDataHelper::AppendSecondaryCommandBufferPoolWaste(const OverlayVk::OverlayItem *item,
                                                                 const VkExtent2D &imageExtent,
                                                                 TextItemData *textItem,
                                                                 GraphItemData *graphItem,
                                                                 OverlayItemCounts *itemCounts)
{
    const OverlayVk::RunningHistogram *secondaryCommandBufferPoolWaste =
        static_cast<const OverlayVk::RunningHistogram *>(item);

    std::vector<size_t> histogram = CreateHistogram(secondaryCommandBufferPoolWaste->runningValues);
    auto maxValueIter             = std::max_element(histogram.rbegin(), histogram.rend());
    const size_t maxValue         = *maxValueIter;
    const int32_t graphHeight     = std::abs(item->coords[3] - item->coords[1]);
    const float graphScale        = static_cast<float>(graphHeight) / maxValue;

    AppendGraphCommon(item, imageExtent, histogram, 0, graphScale, graphItem, itemCounts);

    if ((*itemCounts)[OverlayInternalType::Text] <
        kOverlayInternalTypeMaxItems[OverlayInternalType::Text])
    {
        std::ostringstream text;
        size_t peak        = std::distance(maxValueIter, histogram.rend() - 1);
        size_t peakPercent = (peak * 100 + 50) / histogram.size();

        text << "CB Pool Waste (Peak: " << peakPercent << "%)";
        AppendTextCommon(&secondaryCommandBufferPoolWaste->description, imageExtent, text.str(),
                         textItem, itemCounts);
    }
}

std::ostream &AppendItemDataHelper::OutputPerSecond(std::ostream &out,
                                                    const OverlayVk::PerSecond *perSecond)
{
    return out << perSecond->lastPerSecondCount;
}

std::ostream &AppendItemDataHelper::OutputText(std::ostream &out, const OverlayVk::Text *text)
{
    return out << text->text;
}

std::ostream &AppendItemDataHelper::OutputCount(std::ostream &out, const OverlayVk::Count *count)
{
    return out << count->count;
}
}  // namespace overlay_impl

namespace
{
constexpr angle::PackedEnumMap<OverlayId, AppendItemDataFunc> kOverlayIdToAppendDataFuncMap = {
    {OverlayId::FPS, overlay_impl::AppendItemDataHelper::AppendFPS},
    {OverlayId::LastValidationMessage,
     overlay_impl::AppendItemDataHelper::AppendLastValidationMessage},
    {OverlayId::ValidationMessageCount,
     overlay_impl::AppendItemDataHelper::AppendValidationMessageCount},
    {OverlayId::CommandGraphSize, overlay_impl::AppendItemDataHelper::AppendCommandGraphSize},
    {OverlayId::SecondaryCommandBufferPoolWaste,
     overlay_impl::AppendItemDataHelper::AppendSecondaryCommandBufferPoolWaste},
};
}

OverlayVk::OverlayVk()
    : mSupportsSubgroupBallot(false),
      mSupportsSubgroupArithmetic(false),
      mRefreshCulledItems(false),
      mEnabledItemCount(0),
      mOverlayItems{},
      mLastPerSecondUpdate(0)
{}
OverlayVk::~OverlayVk() = default;

void OverlayVk::enableOverlayItemsFromEnvironment()
{
    std::istringstream angleOverlayItems(angle::GetEnvironmentVar("ANGLE_OVERLAY"));

    std::set<std::string> enabledItems;
    std::string item;
    while (getline(angleOverlayItems, item, ':'))
    {
        enabledItems.insert(item);
    }

    constexpr std::pair<const char *, OverlayId> itemNames[] = {
        {"FPS", OverlayId::FPS},
        {"LastValidationMessage", OverlayId::LastValidationMessage},
        {"ValidationMessageCount", OverlayId::ValidationMessageCount},
        {"CommandGraphSize", OverlayId::CommandGraphSize},
        {"SecondaryCommandBufferPoolWaste", OverlayId::SecondaryCommandBufferPoolWaste},
    };
    for (const std::pair<const char *, OverlayId> &itemName : itemNames)
    {
        if (enabledItems.count(itemName.first) > 0)
        {
            mOverlayItems[itemName.second]->enabled = true;
            ++mEnabledItemCount;
        }
    }
}

angle::Result OverlayVk::createFont(ContextVk *contextVk)
{
    RendererVk *rendererVk = contextVk->getRenderer();

    // Create a buffer to stage font data upload.
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size =
        overlay::kFontCount * overlay::kFontImageWidth * overlay::kFontImageHeight;
    bufferCreateInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    ScopedObject<vk::BufferHelper> fontDataBuffer(contextVk);

    ANGLE_TRY(fontDataBuffer.get().init(contextVk, bufferCreateInfo,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

    uint8_t *fontData;
    ANGLE_TRY(fontDataBuffer.get().map(contextVk, &fontData));

    initFontData(fontData);

    ANGLE_TRY(fontDataBuffer.get().flush(contextVk, 0, fontDataBuffer.get().getSize()));
    fontDataBuffer.get().unmap(contextVk->getDevice());

    fontDataBuffer.get().onExternalWrite(VK_ACCESS_HOST_WRITE_BIT);

    // Create the font image.
    ANGLE_TRY(mFontImage.init(contextVk, gl::TextureType::_2D,
                              VkExtent3D{overlay::kFontImageWidth, overlay::kFontImageHeight, 1},
                              rendererVk->getFormat(angle::FormatID::R8_UNORM), 1,
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1,
                              overlay::kFontCount));
    ANGLE_TRY(mFontImage.initMemory(contextVk, rendererVk->getMemoryProperties(),
                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
    ANGLE_TRY(mFontImage.initImageView(contextVk, gl::TextureType::_2DArray,
                                       VK_IMAGE_ASPECT_COLOR_BIT, gl::SwizzleState(),
                                       &mFontImageView, 0, 1));

    // Copy font data from staging buffer.
    vk::CommandBuffer *fontDataUpload;
    ANGLE_TRY(mFontImage.recordCommands(contextVk, &fontDataUpload));

    fontDataBuffer.get().onRead(&mFontImage, VK_ACCESS_TRANSFER_READ_BIT);

    mFontImage.changeLayout(VK_IMAGE_ASPECT_COLOR_BIT, vk::ImageLayout::TransferDst,
                            fontDataUpload);

    VkBufferImageCopy copy           = {};
    copy.bufferRowLength             = overlay::kFontImageWidth;
    copy.bufferImageHeight           = overlay::kFontImageHeight;
    copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy.imageSubresource.layerCount = overlay::kFontCount;
    copy.imageExtent.width           = overlay::kFontImageWidth;
    copy.imageExtent.height          = overlay::kFontImageHeight;
    copy.imageExtent.depth           = 1;

    fontDataUpload->copyBufferToImage(fontDataBuffer.get().getBuffer().getHandle(),
                                      mFontImage.getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      1, &copy);

    mFontImage.changeLayout(VK_IMAGE_ASPECT_COLOR_BIT, vk::ImageLayout::ComputeShaderReadOnly,
                            fontDataUpload);

    return angle::Result::Continue;
}  // namespace rx

angle::Result OverlayVk::init(ContextVk *contextVk)
{
    RendererVk *rendererVk = contextVk->getRenderer();

    const VkPhysicalDeviceSubgroupProperties &subgroupProperties =
        rendererVk->getPhysicalDeviceSubgroupProperties();
    uint32_t subgroupSize = subgroupProperties.subgroupSize;

    // Currently, only subgroup sizes 32 and 64 are supported.
    if (subgroupSize != 32 && subgroupSize != 64)
    {
        return angle::Result::Continue;
    }

    mSubgroupSize[0] = 8;
    mSubgroupSize[1] = subgroupSize / 8;

    constexpr VkSubgroupFeatureFlags kSubgroupBallotOperations =
        VK_SUBGROUP_FEATURE_BASIC_BIT | VK_SUBGROUP_FEATURE_BALLOT_BIT;
    constexpr VkSubgroupFeatureFlags kSubgroupArithmeticOperations =
        VK_SUBGROUP_FEATURE_BASIC_BIT | VK_SUBGROUP_FEATURE_ARITHMETIC_BIT;

    // If not all operations used in the shaders are supported, disable the overlay.
    if ((subgroupProperties.supportedOperations & kSubgroupBallotOperations) ==
        kSubgroupBallotOperations)
    {
        mSupportsSubgroupBallot = true;
    }
    else if ((subgroupProperties.supportedOperations & kSubgroupArithmeticOperations) ==
             kSubgroupArithmeticOperations)
    {
        mSupportsSubgroupArithmetic = true;
    }

    ANGLE_TRY(createFont(contextVk));

    initOverlayItems();
    mLastPerSecondUpdate = GetCurrentTime();

    ASSERT(std::all_of(mOverlayItems.begin(), mOverlayItems.end(),
                       [](const OverlayItem *item) { return item != nullptr; }));

    enableOverlayItemsFromEnvironment();
    mRefreshCulledItems = true;

    return contextVk->flushImpl(nullptr);
}

void OverlayVk::destroy(VkDevice device)
{
    mCulledItems.destroy(device);
    mCulledItemsView.destroy(device);

    mFontImage.destroy(device);
    mFontImageView.destroy(device);

    for (OverlayItem *&item : mOverlayItems)
    {
        delete item;
        item = nullptr;
    }
}

void OverlayVk::fillEnabledItemCoordinates(uint8_t *enabledItemsPtr)
{
    ItemCoordinates *enabledItems = reinterpret_cast<ItemCoordinates *>(enabledItemsPtr);
    memset(enabledItems, 0, sizeof(*enabledItems));

    OverlayItemCounts itemCounts = {};

    for (const OverlayItem *item : mOverlayItems)
    {
        if (!item->enabled)
        {
            continue;
        }

        OverlayInternalType internalType = kOverlayTypeToInternalMap[item->type];
        ASSERT(internalType != OverlayInternalType::InvalidEnum);

        if (itemCounts[internalType] >= kOverlayInternalTypeMaxItems[internalType])
        {
            continue;
        }

        size_t writeIndex =
            kOverlayInternalTypeItemOffsets[internalType] + itemCounts[internalType]++;

        GetItemCoordinates(item->coords, mPresentImageExtent,
                           enabledItems->coordinates[writeIndex]);

        // Graph items have a text item attached as well.
        if (internalType == OverlayInternalType::Graph)
        {
            OverlayInternalType textType = OverlayInternalType::Text;
            if (itemCounts[textType] >= kOverlayInternalTypeMaxItems[textType])
            {
                continue;
            }

            const RunningGraph *itemAsGraph = static_cast<const RunningGraph *>(item);
            writeIndex = kOverlayInternalTypeItemOffsets[textType] + itemCounts[textType]++;

            GetItemCoordinates(itemAsGraph->description.coords, mPresentImageExtent,
                               enabledItems->coordinates[writeIndex]);
        }
    }
}

void OverlayVk::fillItemData(uint8_t *textData, uint8_t *graphData)
{
    TextItems *textItems   = reinterpret_cast<TextItems *>(textData);
    GraphItems *graphItems = reinterpret_cast<GraphItems *>(graphData);

    memset(textItems, overlay::kFontCharacters, sizeof(*textItems));
    memset(graphItems, 0, sizeof(*graphItems));

    OverlayItemCounts itemCounts = {};

    for (OverlayId id : angle::AllEnums<OverlayId>())
    {
        const OverlayItem *item = mOverlayItems[id];
        if (!item->enabled)
        {
            continue;
        }

        OverlayInternalType internalType = kOverlayTypeToInternalMap[item->type];
        ASSERT(internalType != OverlayInternalType::InvalidEnum);

        if (itemCounts[internalType] >= kOverlayInternalTypeMaxItems[internalType])
        {
            continue;
        }

        AppendItemDataFunc appendFunc = kOverlayIdToAppendDataFuncMap[id];
        appendFunc(item, mPresentImageExtent,
                   &textItems->items[itemCounts[OverlayInternalType::Text]],
                   &graphItems->items[itemCounts[OverlayInternalType::Graph]], &itemCounts);
    }
}

angle::Result OverlayVk::cullItems(ContextVk *contextVk)
{
    RendererVk *rendererVk = contextVk->getRenderer();

    // Release old culledItems image
    Serial currentSerial = contextVk->getCurrentQueueSerial();
    contextVk->releaseObject(currentSerial, &mCulledItems);
    contextVk->releaseObject(currentSerial, &mCulledItemsView);

    // Create a buffer to contain coordinates of enabled text and graph items.  This buffer will
    // be used to perform tiled culling and can be discarded immediately after.
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size               = sizeof(ItemCoordinates);
    bufferCreateInfo.usage              = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferCreateInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

    ScopedObject<vk::BufferHelper> enabledItemsBuffer(contextVk);

    ANGLE_TRY(enabledItemsBuffer.get().init(contextVk, bufferCreateInfo,
                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

    // Fill the buffer with coordinate information from enabled items.
    uint8_t *enabledItems;
    ANGLE_TRY(enabledItemsBuffer.get().map(contextVk, &enabledItems));
    fillEnabledItemCoordinates(enabledItems);
    ANGLE_TRY(enabledItemsBuffer.get().flush(contextVk, 0, enabledItemsBuffer.get().getSize()));
    enabledItemsBuffer.get().unmap(contextVk->getDevice());

    enabledItemsBuffer.get().onExternalWrite(VK_ACCESS_HOST_WRITE_BIT);

    // Allocate mCulledItem and its view.
    VkExtent3D culledItemsExtent = {
        UnsignedCeilDivide(mPresentImageExtent.width, mSubgroupSize[0]),
        UnsignedCeilDivide(mPresentImageExtent.height, mSubgroupSize[1]), 1};

    ANGLE_TRY(mCulledItems.init(contextVk, gl::TextureType::_2D, culledItemsExtent,
                                rendererVk->getFormat(angle::FormatID::R32G32_UINT), 1,
                                VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, 1));
    ANGLE_TRY(mCulledItems.initMemory(contextVk, rendererVk->getMemoryProperties(),
                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
    ANGLE_TRY(mCulledItems.initImageView(contextVk, gl::TextureType::_2D, VK_IMAGE_ASPECT_COLOR_BIT,
                                         gl::SwizzleState(), &mCulledItemsView, 0, 1));

    UtilsVk::OverlayCullParameters params;
    params.subgroupSize[0]            = mSubgroupSize[0];
    params.subgroupSize[1]            = mSubgroupSize[1];
    params.supportsSubgroupBallot     = mSupportsSubgroupBallot;
    params.supportsSubgroupArithmetic = mSupportsSubgroupArithmetic;

    return contextVk->getUtils().cullOverlayItems(contextVk, &enabledItemsBuffer.get(),
                                                  &mCulledItems, &mCulledItemsView, params);
}

angle::Result OverlayVk::onPresent(ContextVk *contextVk,
                                   vk::ImageHelper *imageToPresent,
                                   vk::ImageView *imageToPresentView)
{
    if (mEnabledItemCount == 0)
    {
        return angle::Result::Continue;
    }

    RendererVk *rendererVk = contextVk->getRenderer();

    // If the swapchain image doesn't support storage image, we can't output to it.
    VkFormatFeatureFlags featureBits = rendererVk->getImageFormatFeatureBits(
        imageToPresent->getFormat().vkImageFormat, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);
    if ((featureBits & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) == 0)
    {
        return angle::Result::Continue;
    }

    const VkExtent3D &imageExtent = imageToPresent->getExtents();

    mRefreshCulledItems = mRefreshCulledItems || mPresentImageExtent.width != imageExtent.width ||
                          mPresentImageExtent.height != imageExtent.height;

    if (mRefreshCulledItems)
    {
        mPresentImageExtent.width  = imageExtent.width;
        mPresentImageExtent.height = imageExtent.height;

        ANGLE_TRY(cullItems(contextVk));

        mRefreshCulledItems = false;
    }

    // Update per second values every second.
    double currentTime = GetCurrentTime();
    double timeDiff    = currentTime - mLastPerSecondUpdate;
    if (timeDiff >= 1.0)
    {
        for (OverlayItem *item : mOverlayItems)
        {
            if (item->type == OverlayType::PerSecond)
            {
                PerSecond *perSecond          = reinterpret_cast<PerSecond *>(item);
                perSecond->lastPerSecondCount = static_cast<size_t>(perSecond->count / timeDiff);
                perSecond->count              = 0;
            }
        }
        mLastPerSecondUpdate += 1.0;
    }

    ScopedObject<vk::BufferHelper> textDataBuffer(contextVk);
    ScopedObject<vk::BufferHelper> graphDataBuffer(contextVk);

    VkBufferCreateInfo textBufferCreateInfo = {};
    textBufferCreateInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    textBufferCreateInfo.size               = sizeof(TextItems);
    textBufferCreateInfo.usage              = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    textBufferCreateInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

    VkBufferCreateInfo graphBufferCreateInfo = textBufferCreateInfo;
    graphBufferCreateInfo.size               = sizeof(GraphItems);

    ANGLE_TRY(textDataBuffer.get().init(contextVk, textBufferCreateInfo,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    ANGLE_TRY(graphDataBuffer.get().init(contextVk, graphBufferCreateInfo,
                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

    uint8_t *textData;
    uint8_t *graphData;
    ANGLE_TRY(textDataBuffer.get().map(contextVk, &textData));
    ANGLE_TRY(graphDataBuffer.get().map(contextVk, &graphData));

    fillItemData(textData, graphData);

    ANGLE_TRY(textDataBuffer.get().flush(contextVk, 0, textDataBuffer.get().getSize()));
    ANGLE_TRY(graphDataBuffer.get().flush(contextVk, 0, graphDataBuffer.get().getSize()));
    textDataBuffer.get().unmap(contextVk->getDevice());
    graphDataBuffer.get().unmap(contextVk->getDevice());

    UtilsVk::OverlayDrawParameters params;
    params.subgroupSize[0] = mSubgroupSize[0];
    params.subgroupSize[1] = mSubgroupSize[1];

    return contextVk->getUtils().drawOverlay(
        contextVk, &textDataBuffer.get(), &graphDataBuffer.get(), &mFontImage, &mFontImageView,
        &mCulledItems, &mCulledItemsView, imageToPresent, imageToPresentView, params);
}

OverlayVk::RunningGraph::RunningGraph(size_t n) : runningValues(n, 0) {}
OverlayVk::RunningGraph::~RunningGraph() = default;

DummyOverlayVk::DummyOverlayVk()  = default;
DummyOverlayVk::~DummyOverlayVk() = default;

}  // namespace rx
