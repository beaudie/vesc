#include "ImageIndex.h"
//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ImageIndex.cpp: Implementation for ImageIndex methods.

#include "libANGLE/ImageIndex.h"
#include "libANGLE/Constants.h"
#include "common/utilities.h"

#include <tuple>

namespace gl
{

bool ImageIndex::is3D() const
{
    return type == TextureType::_3D || type == TextureType::_2DArray;
}

GLint ImageIndex::cubeMapFaceIndex() const
{
    return TextureTargetToCubeMapFaceIndex(target);
}

bool ImageIndex::valid() const
{
    return type != TextureType::InvalidEnum;
}

ImageIndex ImageIndex::Make2D(GLint mipIndex)
{
    return ImageIndex(TextureType::_2D, TextureTarget::_2D, mipIndex, ENTIRE_LEVEL, 1);
}

ImageIndex ImageIndex::MakeRectangle(GLint mipIndex)
{
    return ImageIndex(TextureType::Rectangle, TextureTarget::Rectangle, mipIndex,
                      ENTIRE_LEVEL, 1);
}

ImageIndex ImageIndex::MakeCube(TextureTarget target, GLint mipIndex)
{
    ASSERT(TextureTargetToType(target) == TextureType::CubeMap);
    return ImageIndex(TextureType::CubeMap, target, mipIndex, ENTIRE_LEVEL, 1);
}

ImageIndex ImageIndex::Make2DArray(GLint mipIndex, GLint layerIndex)
{
    return ImageIndex(TextureType::_2DArray, TextureTarget::_2DArray, mipIndex, layerIndex, 1);
}

ImageIndex ImageIndex::Make2DArrayRange(GLint mipIndex, GLint layerIndex, GLint numLayers)
{
    return ImageIndex(TextureType::_2DArray, TextureTarget::_2DArray, mipIndex, layerIndex, numLayers);
}

ImageIndex ImageIndex::Make3D(GLint mipIndex, GLint layerIndex)
{
    return ImageIndex(TextureType::_3D, TextureTarget::_3D, mipIndex, layerIndex, 1);
}

ImageIndex ImageIndex::MakeGeneric(TextureTarget target, GLint mipIndex)
{
    return ImageIndex(TextureTargetToType(target), target, mipIndex, ENTIRE_LEVEL, 1);
}

ImageIndex ImageIndex::Make2DMultisample()
{
    return ImageIndex(TextureType::_2DMultisample, TextureTarget::_2DMultisample, 0, ENTIRE_LEVEL, 1);
}

ImageIndex ImageIndex::MakeInvalid()
{
    return ImageIndex(TextureType::InvalidEnum, TextureTarget::InvalidEnum, -1, -1, -1);
}

bool operator<(const ImageIndex &a, const ImageIndex &b)
{
    uint8_t typeA = static_cast<uint8_t>(a.type);
    uint8_t typeB = static_cast<uint8_t>(b.type);
    uint8_t targetA = static_cast<uint8_t>(a.target);
    uint8_t targetB = static_cast<uint8_t>(b.target);
    return std::tie(typeA, targetA, a.mipIndex, a.layerIndex, a.numLayers) <
           std::tie(typeB, targetB, b.mipIndex, b.layerIndex, b.numLayers);
}

bool operator==(const ImageIndex &a, const ImageIndex &b)
{
    return std::tie(a.type, a.target, a.mipIndex, a.layerIndex, a.numLayers) ==
           std::tie(b.type, b.target, b.mipIndex, b.layerIndex, b.numLayers);
}

bool operator!=(const ImageIndex &a, const ImageIndex &b)
{
    return !(a == b);
}

ImageIndex::ImageIndex(TextureType typeIn,
                       TextureTarget targetIn,
                       GLint mipIndexIn,
                       GLint layerIndexIn,
                       GLint numLayersIn)
    : type(typeIn),
      target(targetIn),
      mipIndex(mipIndexIn),
      layerIndex(layerIndexIn),
      numLayers(numLayersIn)
{}

ImageIndexIterator::ImageIndexIterator(const ImageIndexIterator &other) = default;

ImageIndexIterator ImageIndexIterator::Make2D(GLint minMip, GLint maxMip)
{
    uint8_t target2D = static_cast<uint8_t>(TextureTarget::_2D);
    return ImageIndexIterator(
        TextureType::_2D, Range<uint8_t>(target2D, target2D), Range<GLint>(minMip, maxMip),
        Range<GLint>(ImageIndex::ENTIRE_LEVEL, ImageIndex::ENTIRE_LEVEL), nullptr);
}

ImageIndexIterator ImageIndexIterator::MakeRectangle(GLint minMip, GLint maxMip)
{
    uint8_t targetRect = static_cast<uint8_t>(TextureTarget::_2D);
    return ImageIndexIterator(TextureType::Rectangle,
                              Range<uint8_t>(targetRect, targetRect),
                              Range<GLint>(minMip, maxMip),
                              Range<GLint>(ImageIndex::ENTIRE_LEVEL, ImageIndex::ENTIRE_LEVEL),
                              nullptr);
}

ImageIndexIterator ImageIndexIterator::MakeCube(GLint minMip, GLint maxMip)
{
    return ImageIndexIterator(
        TextureType::CubeMap,
        Range<uint8_t>(static_cast<uint8_t>(TextureTarget::CubeMapNegativeX), static_cast<uint8_t>(TextureTarget::CubeMapPositiveZ)),
        Range<GLint>(minMip, maxMip),
        Range<GLint>(ImageIndex::ENTIRE_LEVEL, ImageIndex::ENTIRE_LEVEL), nullptr);
}

ImageIndexIterator ImageIndexIterator::Make3D(GLint minMip, GLint maxMip,
                                              GLint minLayer, GLint maxLayer)
{
    uint8_t target3D = static_cast<uint8_t>(TextureTarget::_3D);
    return ImageIndexIterator(TextureType::_3D, Range<uint8_t>(target3D, target3D),
                              Range<GLint>(minMip, maxMip), Range<GLint>(minLayer, maxLayer),
                              nullptr);
}

ImageIndexIterator ImageIndexIterator::Make2DArray(GLint minMip, GLint maxMip,
                                                   const GLsizei *layerCounts)
{
    uint8_t target2DArray = static_cast<uint8_t>(TextureTarget::_2DArray);
    return ImageIndexIterator(
        TextureType::_2DArray, Range<uint8_t>(target2DArray, target2DArray),
        Range<GLint>(minMip, maxMip), Range<GLint>(0, IMPLEMENTATION_MAX_2D_ARRAY_TEXTURE_LAYERS),
        layerCounts);
}

ImageIndexIterator ImageIndexIterator::Make2DMultisample()
{
    uint8_t target2DMS = static_cast<uint8_t>(TextureTarget::_2DMultisample);
    return ImageIndexIterator(
        TextureType::_2DMultisample,
        Range<uint8_t>(target2DMS, target2DMS), Range<GLint>(0, 0),
        Range<GLint>(ImageIndex::ENTIRE_LEVEL, ImageIndex::ENTIRE_LEVEL), nullptr);
}

ImageIndexIterator::ImageIndexIterator(TextureType type,
                                       const Range<uint8_t> &targetRange,
                                       const Range<GLint> &mipRange,
                                       const Range<GLint> &layerRange,
                                       const GLsizei *layerCounts)
    : mTargetRange(targetRange),
      mMipRange(mipRange),
      mLayerRange(layerRange),
      mLayerCounts(layerCounts),
      mCurrentIndex(type, static_cast<TextureTarget>(targetRange.low()), mipRange.low(), layerRange.low(), 1)
{}

GLint ImageIndexIterator::maxLayer() const
{
    if (mLayerCounts)
    {
        ASSERT(mCurrentIndex.hasLayer());
        return (mCurrentIndex.mipIndex < mMipRange.high()) ? mLayerCounts[mCurrentIndex.mipIndex]
                                                           : 0;
    }
    return mLayerRange.high();
}

ImageIndex ImageIndexIterator::next()
{
    ASSERT(hasNext());

    // Make a copy of the current index to return
    ImageIndex previousIndex = mCurrentIndex;

    // Iterate layers in the inner loop for now. We can add switchable
    // layer or mip iteration if we need it.

    if (static_cast<uint8_t>(mCurrentIndex.target) < mTargetRange.high())
    {
        mCurrentIndex.target = static_cast<TextureTarget>(static_cast<uint8_t>(mCurrentIndex.target) + 1);
    }
    else if (mCurrentIndex.hasLayer() && mCurrentIndex.layerIndex < maxLayer() - 1)
    {
        mCurrentIndex.target = static_cast<TextureTarget>(mTargetRange.low());
        mCurrentIndex.layerIndex++;
    }
    else if (mCurrentIndex.mipIndex < mMipRange.high() - 1)
    {
        mCurrentIndex.target     = static_cast<TextureTarget>(mTargetRange.low());
        mCurrentIndex.layerIndex = mLayerRange.low();
        mCurrentIndex.mipIndex++;
    }
    else
    {
        mCurrentIndex = ImageIndex::MakeInvalid();
    }

    return previousIndex;
}

ImageIndex ImageIndexIterator::current() const
{
    return mCurrentIndex;
}

bool ImageIndexIterator::hasNext() const
{
    return mCurrentIndex.valid();
}

}  // namespace gl
