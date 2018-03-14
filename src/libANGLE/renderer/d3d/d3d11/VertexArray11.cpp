//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VertexArray11:
//   Implementation of rx::VertexArray11.
//

#include "libANGLE/renderer/d3d/d3d11/VertexArray11.h"

#include "common/bitset_utils.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/d3d/IndexBuffer.h"
#include "libANGLE/renderer/d3d/d3d11/Buffer11.h"
#include "libANGLE/renderer/d3d/d3d11/Context11.h"

using namespace angle;

namespace rx
{
namespace
{
IndexStorageType ClassifyIndexStorage(const gl::State &glState,
                                      const gl::Buffer *elementArrayBuffer,
                                      GLenum elementType,
                                      GLenum destElementType,
                                      unsigned int offset)
{
    // No buffer bound means we are streaming from a client pointer.
    if (!elementArrayBuffer || !IsOffsetAligned(elementType, offset))
    {
        return IndexStorageType::Dynamic;
    }

    // The buffer can be used directly if the storage supports it and no translation needed.
    BufferD3D *bufferD3D = GetImplAs<BufferD3D>(elementArrayBuffer);
    if (bufferD3D->supportsDirectBinding() && destElementType == elementType)
    {
        return IndexStorageType::Direct;
    }

    // Use a static copy when available.
    StaticIndexBufferInterface *staticBuffer = bufferD3D->getStaticIndexBuffer();
    if (staticBuffer != nullptr)
    {
        return IndexStorageType::Static;
    }

    // Static buffer not available, fall back to streaming.
    return IndexStorageType::Dynamic;
}
}  // anonymous namespace

VertexArray11::VertexArray11(const gl::VertexArrayState &data)
    : VertexArrayImpl(data),
      mAttributeStorageTypes(data.getMaxAttribs(), VertexStorageType::CURRENT_VALUE),
      mTranslatedAttribs(data.getMaxAttribs()),
      mAppliedNumViewsToDivisor(1),
      mCurrentElementArrayStorage(IndexStorageType::Invalid),
      mCachedDestinationIndexType(GL_NONE)
{
}

VertexArray11::~VertexArray11()
{
}

void VertexArray11::destroy(const gl::Context *context)
{
}

void VertexArray11::syncState(const gl::Context *context,
                              const gl::VertexArray::DirtyBits &dirtyBits)
{
    ASSERT(dirtyBits.any());

    Renderer11 *renderer         = GetImplAs<Context11>(context)->getRenderer();
    StateManager11 *stateManager = renderer->getStateManager();

    // Generate a state serial. This serial is used in the program class to validate the cached
    // input layout, and skip recomputation in the fast path.
    mCurrentStateSerial = renderer->generateSerial();

    // TODO(jmadill): Individual attribute invalidation.
    stateManager->invalidateVertexBuffer();

    for (auto dirtyBit : dirtyBits)
    {
        if (dirtyBit == gl::VertexArray::DIRTY_BIT_ELEMENT_ARRAY_BUFFER)
        {
            mLastDrawElementsType.reset();
            mLastDrawElementsIndices.reset();
            mLastPrimitiveRestartEnabled.reset();
            mCachedIndexInfo.reset();
        }
        else
        {
            size_t index = gl::VertexArray::GetVertexIndexFromDirtyBit(dirtyBit);
            // TODO(jiawei.shao@intel.com): Vertex Attrib Bindings
            ASSERT(index == mState.getBindingIndexFromAttribIndex(index));
            mAttribsToUpdate.set(index);
        }
    }
}

gl::Error VertexArray11::syncStateInternal(const gl::Context *context,
                                           const gl::DrawCallParams &drawCallParams)
{
    Renderer11 *renderer         = GetImplAs<Context11>(context)->getRenderer();
    StateManager11 *stateManager = renderer->getStateManager();

    flushAttribUpdates(context);

    const gl::Program *program = context->getGLState().getProgram();
    ASSERT(program);
    mAppliedNumViewsToDivisor = (program->usesMultiview() ? program->getNumViews() : 1);

    if (mAttribsToTranslate.any())
    {
        ANGLE_TRY(updateDirtyAttribs(context, stateManager));
    }

    if (mDynamicAttribsMask.any())
    {
        ANGLE_TRY(updateDynamicAttribs(context, stateManager, drawCallParams));
    }

    if (drawCallParams.isDrawElements())
    {
        bool restartEnabled = context->getGLState().isPrimitiveRestartEnabled();
        if (!mLastDrawElementsType.valid() ||
            mLastDrawElementsType.value() != drawCallParams.type() ||
            mLastDrawElementsIndices.value() != drawCallParams.indices() ||
            mLastPrimitiveRestartEnabled.value() != restartEnabled)
        {
            mLastDrawElementsType        = drawCallParams.type();
            mLastDrawElementsIndices     = drawCallParams.indices();
            mLastPrimitiveRestartEnabled = restartEnabled;

            ANGLE_TRY(updateElementArrayStorage(context, drawCallParams, restartEnabled));
            stateManager->invalidateIndexBuffer();
        }
        else if (mCurrentElementArrayStorage == IndexStorageType::Dynamic)
        {
            stateManager->invalidateIndexBuffer();
        }
    }

    return gl::NoError();
}

void VertexArray11::flushAttribUpdates(const gl::Context *context)
{
    if (!mAttribsToUpdate.any())
    {
        return;
    }

    const auto &activeLocations =
        context->getGLState().getProgram()->getActiveAttribLocationsMask();

    // Skip attrib locations the program doesn't use.
    gl::AttributesMask activeToUpdate = mAttribsToUpdate & activeLocations;

    if (!activeToUpdate.any())
    {
        return;
    }

    for (auto toUpdateIndex : activeToUpdate)
    {
        mAttribsToUpdate.reset(toUpdateIndex);
        updateVertexAttribStorage(context, toUpdateIndex);
    }

    StateManager11 *stateManager = GetImplAs<Context11>(context)->getRenderer()->getStateManager();
    stateManager->invalidateShaders();
}

gl::Error VertexArray11::updateElementArrayStorage(const gl::Context *context,
                                                   const gl::DrawCallParams &drawCallParams,
                                                   bool restartEnabled)
{
    bool usePrimitiveRestartWorkaround =
        UsePrimitiveRestartWorkaround(restartEnabled, drawCallParams.type());

    ANGLE_TRY(GetIndexTranslationDestType(context, drawCallParams, usePrimitiveRestartWorkaround,
                                          &mCachedDestinationIndexType));

    unsigned int offset =
        static_cast<unsigned int>(reinterpret_cast<uintptr_t>(drawCallParams.indices()));

    mCurrentElementArrayStorage =
        ClassifyIndexStorage(context->getGLState(), mState.getElementArrayBuffer().get(),
                             drawCallParams.type(), mCachedDestinationIndexType, offset);

    return gl::NoError();
}

void VertexArray11::updateVertexAttribStorage(const gl::Context *context, size_t attribIndex)
{
    const gl::VertexAttribute &attrib = mState.getVertexAttribute(attribIndex);
    const gl::VertexBinding &binding  = mState.getBindingFromAttribIndex(attribIndex);

    // Note: having an unchanged storage type doesn't mean the attribute is clean.
    auto oldStorageType = mAttributeStorageTypes[attribIndex];
    auto newStorageType = ClassifyAttributeStorage(attrib, binding);

    mAttributeStorageTypes[attribIndex] = newStorageType;

    StateManager11 *stateManager = GetImplAs<Context11>(context)->getRenderer()->getStateManager();

    if (newStorageType == VertexStorageType::DYNAMIC)
    {
        if (oldStorageType != VertexStorageType::DYNAMIC)
        {
            // Sync dynamic attribs in a different set.
            mAttribsToTranslate.reset(attribIndex);
            mDynamicAttribsMask.set(attribIndex);
        }
    }
    else
    {
        mAttribsToTranslate.set(attribIndex);

        if (oldStorageType == VertexStorageType::DYNAMIC)
        {
            ASSERT(mDynamicAttribsMask[attribIndex]);
            mDynamicAttribsMask.reset(attribIndex);
        }
    }
    stateManager->invalidateVertexAttributeTranslation();

    if (oldStorageType != newStorageType)
    {
        if (newStorageType == VertexStorageType::CURRENT_VALUE)
        {
            stateManager->invalidateCurrentValueAttrib(attribIndex);
        }
    }
}

bool VertexArray11::hasActiveDynamicAttrib(const gl::Context *context)
{
    flushAttribUpdates(context);

    const auto &activeLocations =
        context->getGLState().getProgram()->getActiveAttribLocationsMask();
    ASSERT(!(mAttribsToUpdate & activeLocations).any());
    auto activeDynamicAttribs = (mDynamicAttribsMask & activeLocations);
    return activeDynamicAttribs.any();
}

gl::Error VertexArray11::updateDirtyAttribs(const gl::Context *context,
                                            StateManager11 *stateManager)
{
    const auto &glState         = context->getGLState();
    const auto &activeLocations = glState.getProgram()->getActiveAttribLocationsMask();
    const auto &attribs         = mState.getVertexAttributes();
    const auto &bindings        = mState.getVertexBindings();

    ASSERT(!(mAttribsToUpdate & activeLocations).any());

    stateManager->invalidateInputLayout();

    // Skip attrib locations the program doesn't use, saving for the next frame.
    gl::AttributesMask dirtyActiveAttribs = (mAttribsToTranslate & activeLocations);

    for (auto dirtyAttribIndex : dirtyActiveAttribs)
    {
        mAttribsToTranslate.reset(dirtyAttribIndex);

        auto *translatedAttrib   = &mTranslatedAttribs[dirtyAttribIndex];
        const auto &currentValue = glState.getVertexAttribCurrentValue(dirtyAttribIndex);

        // Record basic attrib info
        translatedAttrib->attribute        = &attribs[dirtyAttribIndex];
        translatedAttrib->binding          = &bindings[translatedAttrib->attribute->bindingIndex];
        translatedAttrib->currentValueType = currentValue.Type;
        translatedAttrib->divisor =
            translatedAttrib->binding->getDivisor() * mAppliedNumViewsToDivisor;

        switch (mAttributeStorageTypes[dirtyAttribIndex])
        {
            case VertexStorageType::DIRECT:
                VertexDataManager::StoreDirectAttrib(translatedAttrib);
                break;
            case VertexStorageType::STATIC:
            {
                ANGLE_TRY(VertexDataManager::StoreStaticAttrib(context, translatedAttrib));
                break;
            }
            case VertexStorageType::CURRENT_VALUE:
                // Current value attribs are managed by the StateManager11.
                break;
            default:
                UNREACHABLE();
                break;
        }
    }

    return gl::NoError();
}

gl::Error VertexArray11::updateDynamicAttribs(const gl::Context *context,
                                              StateManager11 *stateManager,
                                              const gl::DrawCallParams &drawCallParams)
{
    VertexDataManager *vertexDataManager = stateManager->getVertexDataManager();
    const auto &glState                  = context->getGLState();
    const auto &activeLocations          = glState.getProgram()->getActiveAttribLocationsMask();
    const auto &attribs                  = mState.getVertexAttributes();
    const auto &bindings                 = mState.getVertexBindings();

    ANGLE_TRY(drawCallParams.ensureIndexRangeResolved(context));
    stateManager->invalidateInputLayout();

    auto activeDynamicAttribs = (mDynamicAttribsMask & activeLocations);
    if (activeDynamicAttribs.none())
    {
        return gl::NoError();
    }

    for (auto dynamicAttribIndex : activeDynamicAttribs)
    {
        auto *dynamicAttrib      = &mTranslatedAttribs[dynamicAttribIndex];
        const auto &currentValue = glState.getVertexAttribCurrentValue(dynamicAttribIndex);

        // Record basic attrib info
        dynamicAttrib->attribute        = &attribs[dynamicAttribIndex];
        dynamicAttrib->binding          = &bindings[dynamicAttrib->attribute->bindingIndex];
        dynamicAttrib->currentValueType = currentValue.Type;
        dynamicAttrib->divisor = dynamicAttrib->binding->getDivisor() * mAppliedNumViewsToDivisor;
    }

    ANGLE_TRY(vertexDataManager->storeDynamicAttribs(
        context, &mTranslatedAttribs, activeDynamicAttribs, drawCallParams.firstVertex(),
        drawCallParams.vertexCount(), drawCallParams.instances()));

    return gl::NoError();
}

const std::vector<TranslatedAttribute> &VertexArray11::getTranslatedAttribs() const
{
    return mTranslatedAttribs;
}

void VertexArray11::clearDirtyAndPromoteDynamicAttribs(const gl::Context *context,
                                                       const gl::DrawCallParams &drawCallParams)
{
    const gl::State &state      = context->getGLState();
    const gl::Program *program  = state.getProgram();
    const auto &activeLocations = program->getActiveAttribLocationsMask();
    mAttribsToUpdate &= ~activeLocations;

    // Promote to static after we clear the dirty attributes, otherwise we can lose dirtyness.
    auto activeDynamicAttribs = (mDynamicAttribsMask & activeLocations);
    if (activeDynamicAttribs.any())
    {
        VertexDataManager::PromoteDynamicAttribs(context, mTranslatedAttribs, activeDynamicAttribs,
                                                 drawCallParams.vertexCount());
    }
}

void VertexArray11::markAllAttributeDivisorsForAdjustment(int numViews)
{
    if (mAppliedNumViewsToDivisor != numViews)
    {
        mAppliedNumViewsToDivisor = numViews;
        mAttribsToUpdate.set();
    }
}

const TranslatedIndexData &VertexArray11::getCachedIndexInfo() const
{
    ASSERT(mCachedIndexInfo.valid());
    return mCachedIndexInfo.value();
}

void VertexArray11::updateCachedIndexInfo(const TranslatedIndexData &indexInfo)
{
    mCachedIndexInfo = indexInfo;
}

bool VertexArray11::isCachedIndexInfoValid() const
{
    return mCachedIndexInfo.valid();
}

GLenum VertexArray11::getCachedDestinationIndexType() const
{
    return mCachedDestinationIndexType;
}

}  // namespace rx
