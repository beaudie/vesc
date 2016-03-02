//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VertexArray11:
//   Implementation of rx::VertexArray11.
//

#include "libANGLE/renderer/d3d/d3d11/VertexArray11.h"

#include "common/BitSetIterator.h"
#include "libANGLE/renderer/d3d/d3d11/Buffer11.h"

namespace rx
{

namespace
{
size_t GetAttribIndex(unsigned long dirtyBit)
{
    if (dirtyBit >= gl::VertexArray::DIRTY_BIT_ATTRIB_0_ENABLED &&
        dirtyBit < gl::VertexArray::DIRTY_BIT_ATTRIB_MAX_ENABLED)
    {
        return dirtyBit - gl::VertexArray::DIRTY_BIT_ATTRIB_0_ENABLED;
    }

    if (dirtyBit >= gl::VertexArray::DIRTY_BIT_ATTRIB_0_POINTER &&
        dirtyBit < gl::VertexArray::DIRTY_BIT_ATTRIB_MAX_POINTER)
    {
        return dirtyBit - gl::VertexArray::DIRTY_BIT_ATTRIB_0_POINTER;
    }

    ASSERT(dirtyBit >= gl::VertexArray::DIRTY_BIT_ATTRIB_0_DIVISOR &&
           dirtyBit < gl::VertexArray::DIRTY_BIT_ATTRIB_MAX_DIVISOR);
    return static_cast<size_t>(dirtyBit) - gl::VertexArray::DIRTY_BIT_ATTRIB_0_DIVISOR;
}
}  // anonymous namespace

VertexArray11::VertexArray11(const gl::VertexArray::Data &data, Renderer11 *renderer)
    : VertexArrayImpl(data),
      mRenderer(renderer),
      mAttributeStorageTypes(data.getVertexAttributes().size(), VertexStorageType::UNKNOWN),
      mTranslatedAttribs(data.getVertexAttributes().size()),
      mCurrentBuffers(data.getVertexAttributes().size())
{
    for (size_t attribIndex = 0; attribIndex < mCurrentBuffers.size(); ++attribIndex)
    {
        auto callback = [this, attribIndex]()
        {
            this->markBufferDataDirty(attribIndex);
        };
        mOnBufferDataDirty.push_back(callback);
    }
}

VertexArray11::~VertexArray11()
{
    for (auto &binding : mCurrentBuffers)
    {
        binding.set(nullptr);
    }
}

void VertexArray11::syncState(const gl::VertexArray::DirtyBits &dirtyBits)
{
    for (auto dirtyBit : angle::IterateBitSet(dirtyBits))
    {
        if (dirtyBit == gl::VertexArray::DIRTY_BIT_ELEMENT_ARRAY_BUFFER)
            continue;

        size_t attribIndex = GetAttribIndex(dirtyBit);
        updateVertexAttribStorage(attribIndex);
    }
}

void VertexArray11::updateVertexAttribStorage(size_t attribIndex)
{
    const auto &attrib = mData.getVertexAttribute(attribIndex);

    // Note: having an unchanged storage type doesn't mean the attribute is clean.
    auto oldStorageType = mAttributeStorageTypes[attribIndex];
    auto newStorageType = ClassifyAttributeStorage(attrib);

    mAttributeStorageTypes[attribIndex] = newStorageType;

    if (newStorageType == VertexStorageType::DYNAMIC)
    {
        if (oldStorageType != VertexStorageType::DYNAMIC)
        {
            mDynamicAttribIndexes.push_back(attribIndex);
        }
        return;
    }

    // TODO(jmadill): Notify state manager?
    if (std::count(mDirtyAttribIndexes.begin(), mDirtyAttribIndexes.end(), attribIndex) == 0)
    {
        mDirtyAttribIndexes.push_back(attribIndex);
    }

    if (oldStorageType == VertexStorageType::DYNAMIC)
    {
        auto foundIt =
            std::find(mDynamicAttribIndexes.begin(), mDynamicAttribIndexes.end(), attribIndex);
        ASSERT(foundIt != mDynamicAttribIndexes.end());
        mDynamicAttribIndexes.erase(foundIt);
    }

    gl::Buffer *oldBufferGL = mCurrentBuffers[attribIndex].get();
    gl::Buffer *newBufferGL = attrib.buffer.get();
    Buffer11 *oldBuffer11   = oldBufferGL ? GetImplAs<Buffer11>(oldBufferGL) : nullptr;
    Buffer11 *newBuffer11   = newBufferGL ? GetImplAs<Buffer11>(newBufferGL) : nullptr;

    if (oldBuffer11 != newBuffer11 || oldStorageType != newStorageType)
    {
        if (oldBuffer11 != nullptr)
        {
            if (oldStorageType == VertexStorageType::DIRECT)
            {
                oldBuffer11->removeDirectBufferDirtyCallback(&mOnBufferDataDirty[attribIndex]);
            }
            else if (oldStorageType == VertexStorageType::STATIC)
            {
                oldBuffer11->removeStaticBufferDirtyCallback(&mOnBufferDataDirty[attribIndex]);
            }
        }
        if (newBuffer11 != nullptr)
        {
            if (newStorageType == VertexStorageType::DIRECT)
            {
                newBuffer11->addDirectBufferDirtyCallback(&mOnBufferDataDirty[attribIndex]);
            }
            else if (newStorageType == VertexStorageType::STATIC)
            {
                newBuffer11->addStaticBufferDirtyCallback(&mOnBufferDataDirty[attribIndex]);
            }
        }
        mCurrentBuffers[attribIndex] = attrib.buffer;
    }
}

gl::Error VertexArray11::updateDirtyAndDynamicAttribs(VertexDataManager *vertexDataManager,
                                                      const gl::State &state,
                                                      GLint start,
                                                      GLsizei count,
                                                      GLsizei instances)
{
    const gl::Program *program = state.getProgram();
    const auto &attribs        = mData.getVertexAttributes();

    if (!mDirtyAttribIndexes.empty())
    {
        const std::vector<size_t> dirtyAttribs(std::move(mDirtyAttribIndexes));
        mDirtyAttribIndexes.clear();

        for (size_t dirtyAttribIndex : dirtyAttribs)
        {
            // Skip attrib locations the program doesn't use, saving for the next frame.
            // TODO(jmadill): Use bitsets?
            if (!program->isAttribLocationActive(dirtyAttribIndex))
            {
                mDirtyAttribIndexes.push_back(dirtyAttribIndex);
                continue;
            }

            auto *translatedAttrib = &mTranslatedAttribs[dirtyAttribIndex];
            const auto &currentValue =
                state.getVertexAttribCurrentValue(static_cast<unsigned int>(dirtyAttribIndex));

            // Record basic attrib info
            translatedAttrib->attribute        = &attribs[dirtyAttribIndex];
            translatedAttrib->currentValueType = currentValue.Type;
            translatedAttrib->divisor          = translatedAttrib->attribute->divisor;

            switch (mAttributeStorageTypes[dirtyAttribIndex])
            {
                case VertexStorageType::DIRECT:
                    VertexDataManager::StoreDirectAttrib(translatedAttrib, start);
                    break;
                case VertexStorageType::STATIC:
                {
                    auto error = VertexDataManager::StoreStaticAttrib(translatedAttrib, start,
                                                                      count, instances);
                    if (error.isError())
                    {
                        return error;
                    }
                    break;
                }
                case VertexStorageType::CURRENT_VALUE:
                {
                    auto error = vertexDataManager->storeCurrentValue(
                        currentValue, translatedAttrib, dirtyAttribIndex);
                    if (error.isError())
                    {
                        return error;
                    }
                    break;
                }
                default:
                    UNREACHABLE();
                    break;
            }
        }
    }

    if (!mDynamicAttribIndexes.empty())
    {
        for (size_t dynamicAttribIndex : mDynamicAttribIndexes)
        {
            auto *dynamicAttrib = &mTranslatedAttribs[dynamicAttribIndex];
            const auto &currentValue =
                state.getVertexAttribCurrentValue(static_cast<unsigned int>(dynamicAttribIndex));

            // Record basic attrib info
            dynamicAttrib->attribute        = &attribs[dynamicAttribIndex];
            dynamicAttrib->currentValueType = currentValue.Type;
            dynamicAttrib->divisor          = dynamicAttrib->attribute->divisor;
        }

        // TODO(jmadill): Filter out indexes the program doesn't use.
        return vertexDataManager->storeDynamicAttribs(&mTranslatedAttribs, mDynamicAttribIndexes,
                                                      start, count, instances);
    }

    return gl::Error(GL_NO_ERROR);
}

const std::vector<TranslatedAttribute> &VertexArray11::getTranslatedAttribs() const
{
    return mTranslatedAttribs;
}

void VertexArray11::markBufferDataDirty(size_t attribIndex)
{
    ASSERT(mAttributeStorageTypes[attribIndex] == VertexStorageType::DIRECT ||
           mAttributeStorageTypes[attribIndex] == VertexStorageType::STATIC);

    auto *dirtyList = &mDirtyAttribIndexes;
    if (std::find(dirtyList->begin(), dirtyList->end(), attribIndex) == dirtyList->end())
    {
        dirtyList->push_back(attribIndex);
    }
}

}  // namespace rx
