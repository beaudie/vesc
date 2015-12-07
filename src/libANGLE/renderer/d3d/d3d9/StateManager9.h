//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManager9.h: Defines a class for caching D3D9 state

#include "libANGLE/angletypes.h"
#include "libANGLE/Data.h"
#include "libANGLE/State.h"

namespace rx
{

class StateManager9 final : angle::NonCopyable
{
  public:
    StateManager9();
    ~StateManager9();

    void initialize(IDirect3DDevice9 *device, D3DADAPTER_IDENTIFIER9 *mAdapterIdentifier);

    void syncState(const gl::State &state, const gl::State::DirtyBits &dirtyBits);

    gl::Error setBlendState(const gl::Framebuffer *framebuffer,
                            const gl::BlendState &blendState,
                            const gl::ColorF &blendColor,
                            unsigned int sampleMask);

    void forceSetBlendState() { mBlendStateIsDirty = true; }
    VendorID getVendorId() const;

  private:
    void setBlendEnabled(bool enabled);
    void setBlendColor(const gl::BlendState &blendState, const gl::ColorF &blendColor);

    void setBlendFuncsEquations(const gl::BlendState &blendState);

    void setColorMask(const gl::Framebuffer *framebuffer,
                      bool red,
                      bool blue,
                      bool green,
                      bool alpha);

    void setSampleAlphaToCoverage(bool enabled);

    void setDither(bool dither);

    void setSampleMask(unsigned int sampleMask);

    // Currently applied blend state
    bool mBlendStateIsDirty;
    gl::BlendState mCurBlendState;
    gl::ColorF mCurBlendColor;
    unsigned int mCurSampleMask;

    IDirect3DDevice9 *mDevice;
    D3DADAPTER_IDENTIFIER9 *mAdapterIdentifier;
};
}