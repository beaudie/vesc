//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManagerD3D.h: Defines a class for caching D3D state

#ifndef LIBANGLE_RENDERER_D3D_STATEMANAGERD3D_H_
#define LIBANGLE_RENDERER_D3D_STATEMANAGERD3D_H_

#include "libANGLE/angletypes.h"
#include "libANGLE/Data.h"
#include "libANGLE/State.h"

namespace rx
{

class StateManagerD3D : angle::NonCopyable
{
  public:
    explicit StateManagerD3D();

    virtual ~StateManagerD3D();

    void syncExternalDirtyBits(const gl::State::DirtyBits &dirtyBits);

    // TODO(dianx) Should be moved into syncstate after other states are moved in
    void resetExternalDirtyBits();

    gl::Error syncState(const gl::Data &data, const gl::State::DirtyBits &dirtyBits);

    virtual gl::Error setBlendState(const gl::Framebuffer *framebuffer,
                                    const gl::BlendState &blendState,
                                    const gl::ColorF &blendColor,
                                    unsigned int sampleMask,
                                    const gl::State::DirtyBits &dirtyBits) = 0;

    void forceSetBlendState();

  protected:
    static bool IsBlendStateDirty(const gl::State::DirtyBits &dirtyBits);

    // Blend State
    bool mForceSetBlendState;
    gl::BlendState mCurBlendState;
    gl::ColorF mCurBlendColor;
    unsigned int mCurSampleMask;

    // Copy of dirty bits in state. Synced on syncState.
    // Should be removed after all states are moved in
    gl::State::DirtyBits mExternalDirtyBits;
    static const gl::State::DirtyBits mBlendDirtyBits;

  private:
};
}  // namespace rx
#endif
