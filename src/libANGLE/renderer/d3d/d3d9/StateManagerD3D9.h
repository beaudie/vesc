//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManagerD3D11.h: Defines a class for caching D3D11 state

#ifndef LIBANGLE_RENDERER_D3D9_STATEMANAGERD3D9_H_
#define LIBANGLE_RENDERER_D3D9_STATEMANAGERD3D9_H_

#include "libANGLE/renderer/d3d/StateManagerD3D.h"

namespace rx
{

class StateManagerD3D9 final : public StateManagerD3D
{
  public:
    explicit StateManagerD3D9(IDirect3DDevice9 *device, D3DADAPTER_IDENTIFIER9 &adapterIdentifier);

    virtual ~StateManagerD3D9();

    virtual gl::Error setBlendState(const gl::Framebuffer *framebuffer,
                                    const gl::BlendState &blendState,
                                    const gl::ColorF &blendColor,
                                    unsigned int sampleMask,
                                    const gl::State::DirtyBits &dirtyBits);

    virtual gl::Error setDepthStencilState(const gl::DepthStencilState &depthStencilState,
                                           int stencilRef,
                                           int stencilBackRef,
                                           bool frontFaceCCW,
                                           const gl::State::DirtyBits &dirtyBits);

    virtual void setCurStencilSize(unsigned int size);

  private:
    static const D3DRENDERSTATETYPE D3DRS_CCW_STENCILREF       = D3DRS_STENCILREF;
    static const D3DRENDERSTATETYPE D3DRS_CCW_STENCILMASK      = D3DRS_STENCILMASK;
    static const D3DRENDERSTATETYPE D3DRS_CCW_STENCILWRITEMASK = D3DRS_STENCILWRITEMASK;

    VendorID getVendorId() const;

    // Blend state setting functions
    void setDepthMask(bool depthMask);
    void setDepthTestAndFunc(bool depthTest, GLenum depthFunc);
    void setStencilTestEnabled(bool stencilTest);
    void setStencilFuncsFront(GLenum stencilFunc,
                              GLuint stencilMask,
                              int stencilRef,
                              bool frontFaceCCW);
    void setStencilFuncsBack(GLenum stencilBackFunc,
                             GLuint stencilBackMask,
                             int stencilBackRef,
                             bool frontFaceCCW);

    void setStencilWriteMaskFront(GLuint stencilWritemask, bool frontFaceCCW);
    void setStencilWriteMaskBack(GLuint stencilBackWritemask, bool frontFaceCCW);
    void setStencilOpsFront(GLenum stencilFail,
                            GLenum stencilPassDepthFail,
                            GLenum stencilPassDepthPass,
                            bool frontFaceCCW);
    void setStencilOpsBack(GLenum stencilBackFail,
                           GLenum stencilBackPassDepthFail,
                           GLenum stencilBackPassDepthPass,
                           bool frontFaceCCW);

    // Depth stencil state setting functions
    void setBlendEnableFuncsEquations(const gl::BlendState &blendState,
                                      const gl::ColorF &blendColor);
    void setBlendColor(const gl::ColorF &blendColor, const gl::BlendState &blendState);
    void setBlendEnabled(bool blendEnabled);
    void setSampleAlphaToCoverageEnabled(bool sampleAlphaToCoverage);
    void setDitherEnabled(bool ditherEnabled);
    void setBlendColorMask(const gl::BlendState &blendState, const gl::Framebuffer *framebuffer);
    void setSampleMask(unsigned int sampleMask);

    IDirect3DDevice9 *mDevice;
    D3DADAPTER_IDENTIFIER9 &mAdapterIdentifier;

    // Blend State
    gl::BlendState mBlendState;
    gl::ColorF mBlendColor;

    // Depth Stencil State
    gl::DepthStencilState mDepthStencilState;
    int mStencilRef;
    int mStencilBackRef;
    unsigned int mMaxStencil;
};
}
#endif