//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexArray11.h: Defines the rx::VertexArray11 class which implements rx::VertexArrayImpl.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_VERTEXARRAY11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_VERTEXARRAY11_H_

#include "libANGLE/Framebuffer.h"
#include "libANGLE/renderer/VertexArrayImpl.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

namespace rx
{
class Renderer11;

class VertexArray11 : public VertexArrayImpl
{
  public:
    VertexArray11(const gl::VertexArrayState &data);
    ~VertexArray11() override;
    void destroy(const gl::Context *context) override;

    // Does not apply any state updates - these are done in syncStateForDraw which as access to
    // the DrawCallParams before a draw.
    gl::Error syncState(const gl::Context *context,
                        const gl::VertexArray::DirtyBits &dirtyBits,
                        const gl::VertexArray::DirtyAttribBitsArray &attribBits,
                        const gl::VertexArray::DirtyBindingBitsArray &bindingBits) override;

    // Applied buffer pointers are updated here.
    gl::Error syncStateForDraw(const gl::Context *context,
                               const gl::DrawCallParams &drawCallParams);

    // This will check the dynamic attribs mask.
    bool hasActiveDynamicAttrib(const gl::Context *context);

    const std::vector<TranslatedAttribute> &getTranslatedAttribs() const;

    Serial getCurrentStateSerial() const { return mCurrentStateSerial; }

    // In case of a multi-view program change, we have to update all attributes so that the divisor
    // is adjusted.
    void markAllAttributeDivisorsForAdjustment(int numViews);

    // Returns true if the element array buffer needs to be translated.
    gl::Error updateElementArrayStorage(const gl::Context *context,
                                        const gl::DrawCallParams &drawCallParams,
                                        bool restartEnabled);

    const TranslatedIndexData &getCachedIndexInfo() const;
    void updateCachedIndexInfo(const TranslatedIndexData &indexInfo);
    bool isCachedIndexInfoValid() const;

    GLenum getCachedDestinationIndexType() const;

    void onAttribBindingChanged(size_t attribIndex);

  private:
    void updateVertexAttribStorage(StateManager11 *stateManager,
                                   size_t attribIndex);
    gl::Error updateDirtyAttribs(const gl::Context *context,
                                 const gl::AttributesMask &activeDirtyAttribs);
    gl::Error updateDynamicAttribs(const gl::Context *context,
                                   VertexDataManager *vertexDataManager,
                                   const gl::DrawCallParams &drawCallParams,
                                   const gl::AttributesMask &activeDynamicAttribs);

    std::vector<VertexStorageType> mAttributeStorageTypes;
    std::vector<TranslatedAttribute> mTranslatedAttribs;

    // The mask of attributes marked as dynamic.
    gl::AttributesMask mDynamicAttribsMask;

    // A set of attributes we know are dirty, and need to be re-translated.
    gl::AttributesMask mAttribsToTranslate;

    Serial mCurrentStateSerial;

    // The numViews value used to adjust the divisor.
    int mAppliedNumViewsToDivisor;

    // If the index buffer needs re-streaming.
    Optional<GLenum> mLastDrawElementsType;
    Optional<const void *> mLastDrawElementsIndices;
    Optional<bool> mLastPrimitiveRestartEnabled;
    IndexStorageType mCurrentElementArrayStorage;
    Optional<TranslatedIndexData> mCachedIndexInfo;
    GLenum mCachedDestinationIndexType;

    // Records the mappings from each binding to all of the attributes that are using that binding.
    // It is used to label all the relevant attributes dirty when we are updating a dirty binding in
    // VertexArray11::syncStates().
    std::array<gl::AttributesMask, gl::MAX_VERTEX_ATTRIB_BINDINGS> mBoundAttributesMaskOnBindings;
    // Records the bindingIndex of each attribute in the last sync. It is used to make it easier to
    // label the attribute is no longer using the binding it used in the last sync.
    std::array<size_t, gl::MAX_VERTEX_ATTRIBS> mLastAttributeBindings;
};

}  // namespace rx

#endif // LIBANGLE_RENDERER_D3D_D3D11_VERTEXARRAY11_H_
