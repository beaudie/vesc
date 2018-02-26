#pragma once

//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// GLES1State.h: Defines the GLES1State class holding the state of
// a GLES1 context.

#ifndef LIBANGLE_GLES1STATE_H_
#define LIBANGLE_GLES1STATE_H_

#include <unordered_set>

#include "common/matrix_utils.h"
#include "common/vector_utils.h"
#include "libANGLE/angletypes.h"

namespace gl
{

class Context;
class GLES1State
{
  public:
    GLES1State()  = default;
    ~GLES1State() = default;

    void initialize(const Context *context);

    int mMaxMultitextureUnits          = 0;
    int mMaxLights                     = 0;
    int mMaxModelviewMatrixStackDepth  = 0;
    int mMaxProjectionMatrixStackDepth = 0;
    int mMaxTextureMatrixStackDepth    = 0;
    int mMaxClipPlanes                 = 0;

    struct TextureEnables
    {
        bool enable2D      = false;
        bool enableCubeMap = false;
    };
    std::vector<TextureEnables> mTexUnitEnables = {};

    struct VertexArrayEnables
    {
        bool vertex                = false;
        bool normal                = false;
        bool color                 = false;
        bool pointSize             = false;
        std::vector<bool> texCoord = {};
    };
    VertexArrayEnables mEnabledClientStates = {};

    std::vector<bool> mClipPlaneEnabled = {};
    bool mLineSmoothEnabled             = false;
    bool mPointSmoothEnabled            = false;
    bool mPointSpriteEnabled            = false;
    bool mAlphaTestEnabled              = false;
    bool mLogicOpEnabled                = false;
    bool mLightingEnabled               = false;
    bool mFogEnabled                    = false;
    bool mRescaleNormalEnabled          = false;
    bool mNormalizeEnabled              = false;
    bool mColorMaterialEnabled          = false;
    bool mReflecitonMapEnabled          = false;

    ShadingModel mShadeModel                         = ShadingModel::Smooth;
    ColorF mCurrentColor                             = {};
    NormalF mCurrentNormal                           = {};
    std::vector<TextureCoordF> mCurrentTextureCoords = {};

    using MatrixStack                         = std::vector<angle::Mat4>;
    MatrixType mCurrMatrixMode                = MatrixType::Modelview;
    MatrixStack mProjMatrices                 = {};
    MatrixStack mModelviewMatrices            = {};
    std::vector<MatrixStack> mTextureMatrices = {};

    using TextureEnvironments                = std::vector<TextureEnvironmentParameters>;
    TextureEnvironments mTextureEnvironments = {};

    MaterialParameters mMaterial         = {};
    LightModelParameters mLightModel     = {};
    std::vector<LightParameters> mLights = {};
    FogParameters mFog                   = {};
    PointParameters mPointParameters     = {};

    AlphaTestFunc mAlphaFunc = AlphaTestFunc::Always;
    GLfloat mAlphaTestRef    = 0;

    std::vector<angle::Vector4> mClipPlanes = {};

    LogicalOperation mLogicOp = LogicalOperation::Copy;

    GLint mVertexArrayStride       = {};
    GLint mNormalArrayStride       = {};
    GLint mColorArrayStride        = {};
    GLint mTextureCoordArrayStride = {};

    HintSetting mLineSmoothHint            = HintSetting::DontCare;
    HintSetting mPointSmoothHint           = HintSetting::DontCare;
    HintSetting mPerspectiveCorrectionHint = HintSetting::DontCare;
    HintSetting mFogHint                   = HintSetting::DontCare;
};

}  // namespace gl

#endif  // LIBANGLE_GLES1STATE_H_
