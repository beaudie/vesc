//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// GLES1State.h: Defines the GLES1State class holding the state of
// a GLES1 context.

#ifndef LIBANGLE_GLES1STATE_H_
#define LIBANGLE_GLES1STATE_H_

#include <unordered_set>

#include "common/angleutils.h"
#include "common/matrix_utils.h"
#include "common/vector_utils.h"
#include "libANGLE/angletypes.h"

namespace gl
{

class Context;
class GLES1State final : angle::NonCopyable
{
  public:
    GLES1State();
    ~GLES1State();

    void initialize(const Context *context);

  private:
    // All initial state values come from the
    // OpenGL ES 1.1 spec.
    struct TextureEnables
    {
        bool enable2D      = false;
        bool enableCubeMap = false;
    };
    std::vector<TextureEnables> mTexUnitEnables;

    // Table 6.4, 6.5 (IsEnabled)
    bool mVertexArrayEnabled;
    bool mNormalArrayEnabled;
    bool mColorArrayEnabled;
    bool mPointSizeArrayEnabled;
    std::vector<bool> mTexCoordArrayEnabled;

    // Table 6.7-6.16 (IsEnabled)
    std::vector<bool> mClipPlaneEnabled;
    bool mLineSmoothEnabled;
    bool mPointSmoothEnabled;
    bool mPointSpriteEnabled;
    bool mAlphaTestEnabled;
    bool mLogicOpEnabled;
    bool mLightingEnabled;
    bool mFogEnabled;
    bool mRescaleNormalEnabled;
    bool mNormalizeEnabled;
    bool mColorMaterialEnabled;
    bool mReflectionMapEnabled;

    // Table 6.3
    ColorF mCurrentColor;
    NormalF mCurrentNormal;
    std::vector<TextureCoordF> mCurrentTextureCoords;

    // Table 6.7
    using MatrixStack = std::vector<angle::Mat4>;
    MatrixType mCurrMatrixMode;
    MatrixStack mProjMatrices;
    MatrixStack mModelviewMatrices;
    std::vector<MatrixStack> mTextureMatrices;

    // Table 6.15
    using TextureEnvironments = std::vector<TextureEnvironmentParameters>;
    TextureEnvironments mTextureEnvironments;

    // Table 6.9, 2.8
    MaterialParameters mMaterial;
    LightModelParameters mLightModel;

    // Table 6.10
    std::vector<LightParameters> mLights;

    // Table 6.8
    FogParameters mFog;
    ShadingModel mShadeModel;

    // Table 6.11
    PointParameters mPointParameters;

    // Table 6.16
    AlphaTestFunc mAlphaFunc;
    GLfloat mAlphaTestRef;
    LogicalOperation mLogicOp;

    // Table 6.7
    std::vector<angle::Vector4> mClipPlanes;

    // Table 6.19
    HintSetting mLineSmoothHint;
    HintSetting mPointSmoothHint;
    HintSetting mPerspectiveCorrectionHint;
    HintSetting mFogHint;
};

}  // namespace gl

#endif  // LIBANGLE_GLES1STATE_H_
