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

    gl::Error setBlendDepthRasterStates(const gl::State &glState, unsigned int sampleMask);

    void forceSetBlendState();
    void forceSetRasterState();
    void forceSetDepthStencilState();

    void updateDepthSizeIfChanged(bool depthStencilInitialized, unsigned int depthSize);
    void updateStencilSizeIfChanged(bool depthStencilInitialized, unsigned int stencilSize);

    void resetDirtyBits() { mDirtyBits.reset(); }
    VendorID getVendorId() const;

  private:
    // Blend state functions
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

    // Current raster state functions
    void setCullMode(bool cullFace, GLenum cullMode, GLenum frontFace);
    void setPolygonOffsetFill(bool polygonOffsetFill,
                              GLfloat polygonOffsetFactor,
                              GLfloat polygonOffsetUnits);

    // Depth stencil state functions
    void setStencilOpsFront(GLenum stencilFail,
                            GLenum stencilPassDepthFail,
                            GLenum stencilPassDepthPass,
                            bool frontFaceCCW);
    void setStencilOpsBack(GLenum stencilBackFail,
                           GLenum stencilBackPassDepthFail,
                           GLenum stencilBackPassDepthPass,
                           bool frontFaceCCW);
    void setStencilBackWriteMask(GLuint stencilBackWriteMask, bool frontFaceCCW);
    void setDepthFunc(bool depthTest, GLenum depthFunc);
    void setStencilTestEnabled(bool enabled);
    void setDepthMask(bool depthMask);
    void setStencilFuncsFront(GLenum stencilFunc,
                              GLuint stencilMask,
                              GLint stencilRef,
                              bool frontFaceCCW,
                              unsigned int maxStencil);
    void setStencilFuncsBack(GLenum stencilBackFunc,
                             GLuint stencilBackMask,
                             GLint stencilBackRef,
                             bool frontFaceCCW,
                             unsigned int maxStencil);
    void setStencilWriteMask(GLuint stencilWriteMask, bool frontFaceCCW);

    enum DirtyBitType
    {
        // Blend dirty bits
        DIRTY_BIT_BLEND_ENABLED,
        DIRTY_BIT_BLEND_COLOR,
        DIRTY_BIT_BLEND_FUNCS_EQUATIONS,
        DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE,
        DIRTY_BIT_COLOR_MASK,
        DIRTY_BIT_DITHER,
        DIRTY_BIT_SAMPLE_MASK,

        // Rasterizer dirty bits
        DIRTY_BIT_RASTER_CULL_MODE,
        DIRTY_BIT_RASTER_POLYGON_OFFSET,

        // Depth stencil dirty bits
        DIRTY_BIT_STENCIL_DEPTH_MASK,
        DIRTY_BIT_STENCIL_DEPTH_FUNC,
        DIRTY_BIT_STENCIL_TEST_ENABLED,
        DIRTY_BIT_STENCIL_FUNCS_FRONT,
        DIRTY_BIT_STENCIL_FUNCS_BACK,
        DIRTY_BIT_STENCIL_WRITEMASK_FRONT,
        DIRTY_BIT_STENCIL_WRITEMASK_BACK,
        DIRTY_BIT_STENCIL_OPS_FRONT,
        DIRTY_BIT_STENCIL_OPS_BACK,

        DIRTY_BIT_MAX
    };

    typedef std::bitset<DIRTY_BIT_MAX> DirtyBits;

    // Currently applied blend state
    gl::BlendState mCurBlendState;
    gl::ColorF mCurBlendColor;
    unsigned int mCurSampleMask;

    // Currently applied raster state
    gl::RasterizerState mCurRasterState;
    unsigned int mCurDepthSize;

    // Currently applied depth stencil state
    gl::DepthStencilState mCurDepthStencilState;
    int mCurStencilRef;
    int mCurStencilBackRef;
    bool mCurFrontFaceCCW;
    unsigned int mCurStencilSize;

    // FIXME: Unsupported by D3D9
    static const D3DRENDERSTATETYPE D3DRS_CCW_STENCILREF       = D3DRS_STENCILREF;
    static const D3DRENDERSTATETYPE D3DRS_CCW_STENCILMASK      = D3DRS_STENCILMASK;
    static const D3DRENDERSTATETYPE D3DRS_CCW_STENCILWRITEMASK = D3DRS_STENCILWRITEMASK;

    IDirect3DDevice9 *mDevice;
    D3DADAPTER_IDENTIFIER9 *mAdapterIdentifier;
    DirtyBits mDirtyBits;
};
}