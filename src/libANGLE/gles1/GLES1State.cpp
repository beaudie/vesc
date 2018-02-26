//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// GLES1State.cpp: Implements the GLES1State class, tracking state
// for GLES1 contexts.

#include "libANGLE/gles1/GLES1State.h"

#include "libANGLE/Context.h"

namespace gl
{

// Taken from the GLES 1.x spec which specifies all initial state values.
void GLES1State::initialize(const Context *context)
{

    const Caps &caps = context->getCaps();

    mMaxMultitextureUnits          = caps.maxMultitextureUnits;
    mMaxLights                     = caps.maxLights;
    mMaxModelviewMatrixStackDepth  = caps.maxModelviewMatrixStackDepth;
    mMaxProjectionMatrixStackDepth = caps.maxProjectionMatrixStackDepth;
    mMaxTextureMatrixStackDepth    = caps.maxTextureMatrixStackDepth;
    mMaxClipPlanes                 = caps.maxClipPlanes;

    mTexUnitEnables.resize(mMaxMultitextureUnits);
    for (auto &enables : mTexUnitEnables)
    {
        enables.enable2D      = false;
        enables.enableCubeMap = false;
    }

    mEnabledClientStates.vertex    = false;
    mEnabledClientStates.normal    = false;
    mEnabledClientStates.color     = false;
    mEnabledClientStates.pointSize = false;
    mEnabledClientStates.texCoord.resize(mMaxMultitextureUnits);
    for (int i = 0; i < mMaxMultitextureUnits; i++)
    {
        mEnabledClientStates.texCoord[i] = false;
    }

    mLineSmoothEnabled    = false;
    mPointSmoothEnabled   = false;
    mPointSpriteEnabled   = false;
    mLogicOpEnabled       = false;
    mAlphaTestEnabled     = false;
    mLightingEnabled      = false;
    mFogEnabled           = false;
    mRescaleNormalEnabled = false;
    mNormalizeEnabled     = false;
    mColorMaterialEnabled = false;
    mReflecitonMapEnabled = false;

    mShadeModel     = ShadingModel::Smooth;
    mCurrMatrixMode = MatrixType::Modelview;

    mCurrentColor  = {1.0f, 1.0f, 1.0f, 1.0f};
    mCurrentNormal = {0.0f, 0.0f, 1.0f};

    mCurrentTextureCoords.resize(mMaxMultitextureUnits);
    for (auto &texcoord : mCurrentTextureCoords)
    {
        memset(&texcoord, 0, sizeof(texcoord));
    }

    mTextureEnvironments.resize(mMaxMultitextureUnits);
    mTextureEnvironments.resize(mMaxMultitextureUnits);
    for (int i = 0; i < mMaxMultitextureUnits; i++)
    {
        auto &env = mTextureEnvironments[i];

        env.envMode      = TextureEnvMode::Modulate;
        env.combineRgb   = TextureCombine::Modulate;
        env.combineAlpha = TextureCombine::Modulate;

        env.src0rgb   = TextureSrc::Texture;
        env.src0alpha = TextureSrc::Texture;

        env.src1rgb   = TextureSrc::Previous;
        env.src1alpha = TextureSrc::Previous;

        env.src2rgb   = TextureSrc::Constant;
        env.src2alpha = TextureSrc::Constant;

        env.op0rgb   = TextureOp::SrcColor;
        env.op0alpha = TextureOp::SrcAlpha;

        env.op1rgb   = TextureOp::SrcColor;
        env.op1alpha = TextureOp::SrcAlpha;

        env.op2rgb   = TextureOp::SrcAlpha;
        env.op2alpha = TextureOp::SrcAlpha;

        env.envColor   = {0.0f, 0.0f, 0.0f, 0.0f};
        env.rgbScale   = 1.0;
        env.alphaScale = 1.0;

        env.pointSpriteCoordReplace = false;
    }

    mProjMatrices.resize(1, {});
    mModelviewMatrices.resize(1, {});
    mTextureMatrices.resize(mMaxMultitextureUnits);
    for (int i = 0; i < mMaxMultitextureUnits; i++)
    {
        mTextureMatrices[i].resize(1, {});
    }

    mMaterial.ambient  = {0.2f, 0.2f, 0.2f, 1.0f};
    mMaterial.diffuse  = {0.8f, 0.8f, 0.8f, 1.0f};
    mMaterial.specular = {0.0f, 0.0f, 0.0f, 1.0f};
    mMaterial.emissive = {0.0f, 0.0f, 0.0f, 1.0f};

    mMaterial.specularExponent = 0.0f;

    mLightModel.color    = {0.2f, 0.2f, 0.2f, 1.0f};
    mLightModel.twoSided = false;

    mLights.resize(mMaxLights, {});
    for (auto &light : mLights)
    {
        light.enabled = false;

        light.ambient   = {0.0f, 0.0f, 0.0f, 1.0f};
        light.diffuse   = {0.0f, 0.0f, 0.0f, 1.0f};
        light.specular  = {0.0f, 0.0f, 0.0f, 1.0f};
        light.position  = {0.0f, 0.0f, 1.0f, 0.0f};
        light.direction = {0.0f, 0.0f, -1.0f};

        light.spotlightExponent    = 0.0f;
        light.spotlightCutoffAngle = 180.0f;
        light.attenuationConst     = 1.0f;
        light.attenuationLinear    = 0.0f;
        light.attenuationQuadratic = 0.0f;
    }

    // GL_LIGHT0 is special and has default state that avoids all-black
    // renderings.
    mLights[0].diffuse  = {1.0f, 1.0f, 1.0f, 1.0f};
    mLights[0].specular = {1.0f, 1.0f, 1.0f, 1.0f};

    mFog.mode    = FogMode::Exp;
    mFog.density = 1.0f;
    mFog.start   = 0.0f;
    mFog.end     = 1.0f;

    mFog.color = {0.0f, 0.0f, 0.0f, 0.0f};

    mAlphaFunc    = AlphaTestFunc::Always;
    mAlphaTestRef = 0.0f;

    mClipPlaneEnabled.resize(mMaxClipPlanes, {});
    for (int i = 0; i < mMaxClipPlanes; i++)
    {
        mClipPlaneEnabled[i] = false;
    }

    mClipPlanes.resize(mMaxClipPlanes, {});
    for (auto &plane : mClipPlanes)
    {
        for (int i = 0; i < 4; i++)
        {
            plane[i] = 0.0f;
        }
    }

    mPointParameters.pointSizeMin                = 0.1f;
    mPointParameters.pointSizeMax                = 100.0f;
    mPointParameters.pointFadeThresholdSize      = 0.1f;
    mPointParameters.pointDistanceAttenuation[0] = 1.0f;
    mPointParameters.pointDistanceAttenuation[1] = 0.0f;
    mPointParameters.pointDistanceAttenuation[2] = 0.0f;

    mPointParameters.pointSize = 1.0f;

    mLogicOp = LogicalOperation::Copy;

    mVertexArrayStride       = 0;
    mNormalArrayStride       = 0;
    mColorArrayStride        = 0;
    mTextureCoordArrayStride = 0;

    mLineSmoothHint            = HintSetting::DontCare;
    mPointSmoothHint           = HintSetting::DontCare;
    mPerspectiveCorrectionHint = HintSetting::DontCare;
    mFogHint                   = HintSetting::DontCare;
}

}  // namespace gl
