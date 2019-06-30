//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// OverlayVk.h:
//    Defines the OverlayVk class and classes for various overlay elements.
//

#ifndef LIBANGLE_RENDERER_VULKAN_OVERLAYVK_H_
#define LIBANGLE_RENDERER_VULKAN_OVERLAYVK_H_

#include "common/PackedEnums.h"
#include "common/angleutils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{
class ContextVk;

namespace overlay_impl
{
class AppendItemDataHelper;
}

enum class OverlayType
{
    // Text types:

    // A total count of some event.
    Count,
    // A single line of ASCII text.  Retains content until changed.
    Text,
    // A per-second value.
    PerSecond,

    // Graph types:

    // A graph of the last N values.
    RunningGraph,
    // A histogram of the last N values (values between 0 and 1).
    RunningHistogram,

    InvalidEnum,
    EnumCount = InvalidEnum,
};

enum class OverlayId
{
    // Frames per second (PerSecond).
    FPS,
    // Last validation error (Text).
    LastValidationMessage,
    // Number of validation errors and warnings (Count).
    ValidationMessageCount,
    // Number of nodes in command graph (RunningGraph).
    CommandGraphSize,
    // Secondary Command Buffer pool memory waste (RunningHistogram).
    SecondaryCommandBufferPoolWaste,

    InvalidEnum,
    EnumCount = InvalidEnum,
};

class OverlayVk : angle::NonCopyable
{
  public:
    OverlayVk();
    ~OverlayVk();

    angle::Result init(ContextVk *contextVk);
    void destroy(VkDevice device);

    angle::Result onPresent(ContextVk *contextVk,
                            vk::ImageHelper *imageToPresent,
                            vk::ImageView *imageToPresentView);

    class OverlayItem
    {
      protected:
        OverlayType type;
        // Whether this item should be drawn.
        bool enabled = false;

        // For text items, size of the font.  This is a value in [0, overlay::kFontCount) which
        // determines the font size to use.
        int fontSize;

        // The area covered by the item, predetermined by the overlay class.  Negative values
        // indicate offset from the left/bottom of the image.
        int32_t coords[4];
        float color[4];

        friend class OverlayVk;
        friend class overlay_impl::AppendItemDataHelper;
    };

    class Count : public OverlayItem
    {
      public:
        void add(size_t n) { count += n; }
        void reset() { count = 0; }

      protected:
        size_t count = 0;

        friend class OverlayVk;
        friend class overlay_impl::AppendItemDataHelper;
    };

    class PerSecond : public Count
    {
      protected:
        size_t lastPerSecondCount = 0;

        friend class OverlayVk;
        friend class overlay_impl::AppendItemDataHelper;
    };

    class Text : public OverlayItem
    {
      public:
        void set(std::string &&str) { text = std::move(str); }

      protected:
        std::string text;

        friend class overlay_impl::AppendItemDataHelper;
    };

    class RunningGraph : public OverlayItem
    {
      public:
        // Out of line constructor to satisfy chromium-style.
        RunningGraph(size_t n);
        ~RunningGraph();
        void add(size_t n) { runningValues[lastValueIndex] += n; }
        void next()
        {
            lastValueIndex                = (lastValueIndex + 1) % runningValues.size();
            runningValues[lastValueIndex] = 0;
        }

      protected:
        std::vector<size_t> runningValues;
        size_t lastValueIndex = 0;
        Text description;

        friend class OverlayVk;
        friend class overlay_impl::AppendItemDataHelper;
    };

    class RunningHistogram : public RunningGraph
    {
      public:
        RunningHistogram(size_t n) : RunningGraph(n) {}
        void set(float n)
        {
            ASSERT(n >= 0.0f && n <= 1.0f);
            size_t rank = n == 1.0f ? runningValues.size() - 1
                                    : static_cast<size_t>(n * runningValues.size());

            runningValues[lastValueIndex] = rank;
        }
    };

    Text *getTextItem(OverlayId id) const { return getItem<Text, OverlayType::Text>(id); }
    Count *getCountItem(OverlayId id) const { return getItem<Count, OverlayType::Count>(id); }
    PerSecond *getPerSecondItem(OverlayId id) const
    {
        return getItem<PerSecond, OverlayType::PerSecond>(id);
    }
    RunningGraph *getRunningGraphItem(OverlayId id) const
    {
        return getItem<RunningGraph, OverlayType::RunningGraph>(id);
    }
    RunningHistogram *getRunningHistogramItem(OverlayId id) const
    {
        return getItem<RunningHistogram, OverlayType::RunningHistogram>(id);
    }

  private:
    template <typename Item, OverlayType Type>
    Item *getItem(OverlayId id) const
    {
        ASSERT(mOverlayItems[id] != nullptr);
        ASSERT(mOverlayItems[id]->type == Type);
        return reinterpret_cast<Item *>(mOverlayItems[id]);
    }
    void initFontData(uint8_t *fontData);
    void initOverlayItems();
    void enableOverlayItemsFromEnvironment();
    angle::Result createFont(ContextVk *contextVk);
    angle::Result cullItems(ContextVk *contextVk);
    void fillEnabledItemCoordinates(uint8_t *enabledItemsPtr);
    void fillItemData(uint8_t *textData, uint8_t *graphData);

    bool mSupportsSubgroupBallot;
    bool mSupportsSubgroupArithmetic;
    bool mRefreshCulledItems;

    uint32_t mEnabledItemCount;

    // Cached size of subgroup as accepted by UtilsVk, deduced from hardware subgroup size.
    uint32_t mSubgroupSize[2];

    // Cached size of last presented image.  If the size changes, culling is repeated.
    VkExtent2D mPresentImageExtent;

    vk::ImageHelper mFontImage;
    vk::ImageView mFontImageView;

    vk::ImageHelper mCulledItems;
    vk::ImageView mCulledItemsView;

    angle::PackedEnumMap<OverlayId, OverlayItem *> mOverlayItems;

    // Time tracking for PerSecond items.
    double mLastPerSecondUpdate;
};

class DummyOverlayVk
{
    DummyOverlayVk();
    ~DummyOverlayVk();

    angle::Result init(ContextVk *contextVk) { return angle::Result::Continue; }
    angle::Result onPresent(ContextVk *contextVk,
                            vk::ImageHelper *imageToPresent,
                            vk::ImageView *imageToPresentView)
    {
        return angle::Result::Continue;
    }

    class Dummy
    {
      public:
        void reset() const {}
        template <typename T>
        void set(T) const
        {}
        template <typename T>
        void add(T) const
        {}
        void next() const {}
    };

    const Dummy *getTextItem(OverlayId id) const { return &mDummy; }
    const Dummy *getCountItem(OverlayId id) const { return &mDummy; }
    const Dummy *getPerSecondItem(OverlayId id) const { return &mDummy; }
    const Dummy *getRunningGraphItem(OverlayId id) const { return &mDummy; }
    const Dummy *getRunningHistogramItem(OverlayId id) const { return &mDummy; }

  private:
    Dummy mDummy;
};

#if ANGLE_ENABLE_VULKAN_OVERLAY
using OverlayImplVk = OverlayVk;
#else   // !ANGLE_ENABLE_VULKAN_OVERLAY
using OverlayImplVk = DummyOverlayVk;
#endif  // ANGLE_ENABLE_VULKAN_OVERLAY

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_OVERLAYVK_H_
