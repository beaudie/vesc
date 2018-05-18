//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// GLES1State.cpp: Implements the GLES1State class, tracking state
// for GLES1 contexts.

#include "libANGLE/GLES1State.h"

#include "libANGLE/Context.h"

namespace {

static constexpr int kRGBA32FBytes = 4 * sizeof(GLfloat);

}  // anonymous namespace

namespace gl
{

TextureCoordF::TextureCoordF() = default;

TextureCoordF::TextureCoordF(float _s, float _t, float _r, float _q) : s(_s), t(_t), r(_r), q(_q)
{
}

bool TextureCoordF::operator==(const TextureCoordF &other) const
{
    return s == other.s && t == other.t && r == other.r && q == other.q;
}


GLES1State::GLES1State()
    : mGLState(nullptr),
      mVertexArrayEnabled(false),
      mNormalArrayEnabled(false),
      mColorArrayEnabled(false),
      mPointSizeArrayEnabled(false),
      mLineSmoothEnabled(false),
      mPointSmoothEnabled(false),
      mPointSpriteEnabled(false),
      mAlphaTestEnabled(false),
      mLogicOpEnabled(false),
      mLightingEnabled(false),
      mFogEnabled(false),
      mRescaleNormalEnabled(false),
      mNormalizeEnabled(false),
      mColorMaterialEnabled(false),
      mReflectionMapEnabled(false),
      mCurrentColor({0.0f, 0.0f, 0.0f, 0.0f}),
      mCurrentNormal({0.0f, 0.0f, 0.0f}),
      mClientActiveTexture(0),
      mMatrixMode(MatrixType::Modelview),
      mShadeModel(ShadingModel::Smooth),
      mAlphaTestFunc(AlphaTestFunc::AlwaysPass),
      mAlphaTestRef(0.0f),
      mLogicOp(LogicalOperation::Copy),
      mLineSmoothHint(HintSetting::DontCare),
      mPointSmoothHint(HintSetting::DontCare),
      mPerspectiveCorrectionHint(HintSetting::DontCare),
      mFogHint(HintSetting::DontCare)
{
}

GLES1State::~GLES1State() = default;

// Taken from the GLES 1.x spec which specifies all initial state values.
void GLES1State::initialize(const Context *context, const State *state)
{
    mGLState = state;

    const Caps &caps = context->getCaps();

    mTexUnitEnables.resize(caps.maxMultitextureUnits);
    for (auto &enables : mTexUnitEnables)
    {
        enables.reset();
    }

    mVertexArrayEnabled    = false;
    mNormalArrayEnabled    = false;
    mColorArrayEnabled     = false;
    mPointSizeArrayEnabled = false;
    mTexCoordArrayEnabled.resize(caps.maxMultitextureUnits, false);

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
    mReflectionMapEnabled = false;

    mMatrixMode = MatrixType::Modelview;

    mCurrentColor  = {1.0f, 1.0f, 1.0f, 1.0f};
    mCurrentNormal = {0.0f, 0.0f, 1.0f};
    mCurrentTextureCoords.resize(caps.maxMultitextureUnits);
    mClientActiveTexture = 0;

    mTextureEnvironments.resize(caps.maxMultitextureUnits);

    mModelviewMatrices.push_back(angle::Mat4());
    mProjectionMatrices.push_back(angle::Mat4());
    mTextureMatrices.resize(caps.maxMultitextureUnits);
    for (auto &stack : mTextureMatrices)
    {
        stack.push_back(angle::Mat4());
    }

    mMaterial.ambient  = {0.2f, 0.2f, 0.2f, 1.0f};
    mMaterial.diffuse  = {0.8f, 0.8f, 0.8f, 1.0f};
    mMaterial.specular = {0.0f, 0.0f, 0.0f, 1.0f};
    mMaterial.emissive = {0.0f, 0.0f, 0.0f, 1.0f};

    mMaterial.specularExponent = 0.0f;

    mLightModel.color    = {0.2f, 0.2f, 0.2f, 1.0f};
    mLightModel.twoSided = false;

    mLights.resize(caps.maxLights);

    // GL_LIGHT0 is special and has default state that avoids all-black renderings.
    mLights[0].diffuse  = {1.0f, 1.0f, 1.0f, 1.0f};
    mLights[0].specular = {1.0f, 1.0f, 1.0f, 1.0f};

    mFog.mode    = FogMode::Exp;
    mFog.density = 1.0f;
    mFog.start   = 0.0f;
    mFog.end     = 1.0f;

    mFog.color = {0.0f, 0.0f, 0.0f, 0.0f};

    mShadeModel = ShadingModel::Smooth;

    mAlphaTestFunc = AlphaTestFunc::AlwaysPass;
    mAlphaTestRef  = 0.0f;

    mLogicOp = LogicalOperation::Copy;

    mClipPlaneEnabled.resize(caps.maxClipPlanes, false);

    mClipPlanes.resize(caps.maxClipPlanes, angle::Vector4(0.0f, 0.0f, 0.0f, 0.0f));

    mPointParameters.pointSizeMin                = 0.1f;
    mPointParameters.pointSizeMax                = 100.0f;
    mPointParameters.pointFadeThresholdSize      = 0.1f;
    mPointParameters.pointDistanceAttenuation[0] = 1.0f;
    mPointParameters.pointDistanceAttenuation[1] = 0.0f;
    mPointParameters.pointDistanceAttenuation[2] = 0.0f;

    mPointParameters.pointSize = 1.0f;

    mLineSmoothHint            = HintSetting::DontCare;
    mPointSmoothHint           = HintSetting::DontCare;
    mPerspectiveCorrectionHint = HintSetting::DontCare;
    mFogHint                   = HintSetting::DontCare;
}

void GLES1State::setAlphaFunc(AlphaTestFunc func, GLfloat ref)
{
    mAlphaTestFunc = func;
    mAlphaTestRef  = ref;
}

void GLES1State::setClientTextureUnit(unsigned int unit)
{
    mClientActiveTexture = unit;
}

unsigned int GLES1State::getClientTextureUnit() const
{
    return mClientActiveTexture;
}

void GLES1State::setCurrentColor(const ColorF &color)
{
    mCurrentColor = color;
}

const ColorF &GLES1State::getCurrentColor() const
{
    return mCurrentColor;
}

void GLES1State::setCurrentNormal(const angle::Vector3 &normal)
{
    mCurrentNormal = normal;
}

const angle::Vector3 &GLES1State::getCurrentNormal() const
{
    return mCurrentNormal;
}

void GLES1State::setCurrentTextureCoords(unsigned int unit, const TextureCoordF &coords)
{
    mCurrentTextureCoords[unit] = coords;
}

const TextureCoordF &GLES1State::getCurrentTextureCoords(unsigned int unit) const
{
    return mCurrentTextureCoords[unit];
}

void GLES1State::setMatrixMode(MatrixType mode)
{
    mMatrixMode = mode;
}

MatrixType GLES1State::getMatrixMode() const
{
    return mMatrixMode;
}

void GLES1State::pushMatrix()
{
    auto &stack = currentMatrixStack();
    stack.push_back(stack.back());
}

void GLES1State::popMatrix()
{
    auto &stack = currentMatrixStack();
    stack.pop_back();
}

GLES1State::MatrixStack &GLES1State::currentMatrixStack()
{
    switch (mMatrixMode)
    {
        case MatrixType::Modelview:
            return mModelviewMatrices;
        case MatrixType::Projection:
            return mProjectionMatrices;
        case MatrixType::Texture:
            return mTextureMatrices[mGLState->getActiveSampler()];
        default:
            UNREACHABLE();
            return mModelviewMatrices;
    }
}

const GLES1State::MatrixStack &GLES1State::currentMatrixStack() const
{
    switch (mMatrixMode)
    {
        case MatrixType::Modelview:
            return mModelviewMatrices;
        case MatrixType::Projection:
            return mProjectionMatrices;
        case MatrixType::Texture:
            return mTextureMatrices[mGLState->getActiveSampler()];
        default:
            UNREACHABLE();
            return mModelviewMatrices;
    }
}

void GLES1State::loadMatrix(const angle::Mat4 &m)
{
    currentMatrixStack().back() = m;
}

void GLES1State::multMatrix(const angle::Mat4 &m)
{
    angle::Mat4 currentMatrix   = currentMatrixStack().back();
    currentMatrixStack().back() = currentMatrix.product(m);
}

void GLES1State::setClientStateEnabled(ClientVertexArrayType clientState, bool enable)
{
    switch (clientState)
    {
        case ClientVertexArrayType::Vertex:
            mVertexArrayEnabled = enable;
            break;
        case ClientVertexArrayType::Normal:
            mNormalArrayEnabled = enable;
            break;
        case ClientVertexArrayType::Color:
            mColorArrayEnabled = enable;
            break;
        case ClientVertexArrayType::PointSize:
            mPointSizeArrayEnabled = enable;
            break;
        case ClientVertexArrayType::TextureCoord:
            mTexCoordArrayEnabled[mClientActiveTexture] = enable;
            break;
        default:
            UNREACHABLE();
            break;
    }
}

bool GLES1State::isClientStateEnabled(ClientVertexArrayType clientState) const
{
    switch (clientState)
    {
        case ClientVertexArrayType::Vertex:
            return mVertexArrayEnabled;
        case ClientVertexArrayType::Normal:
            return mNormalArrayEnabled;
        case ClientVertexArrayType::Color:
            return mColorArrayEnabled;
        case ClientVertexArrayType::PointSize:
            return mPointSizeArrayEnabled;
        case ClientVertexArrayType::TextureCoord:
            return mTexCoordArrayEnabled[mClientActiveTexture];
        default:
            UNREACHABLE();
            return false;
    }
}

bool GLES1State::isTexCoordArrayEnabled(unsigned int unit) const
{
    ASSERT(unit < mTexCoordArrayEnabled.size());
    return mTexCoordArrayEnabled[unit];
}

bool GLES1State::isTextureTargetEnabled(unsigned int unit, const TextureType type) const
{
    return mTexUnitEnables[unit].test(type);
}

void GLES1State::setLightParameters(GLenum light, LightParameter pname, const GLfloat *params)
{
    uint32_t lightIndex = light - GL_LIGHT0;

    angle::Vector4 transformedPos;

    angle::Mat4 mv = mModelviewMatrices.back();

    switch (pname)
    {
        case LightParameter::Ambient:
            memcpy(mLights[lightIndex].ambient.ptr(), params, kRGBA32FBytes);
            break;
        case LightParameter::Diffuse:
            memcpy(mLights[lightIndex].diffuse.ptr(), params, kRGBA32FBytes);
            break;
        case LightParameter::Specular:
            memcpy(mLights[lightIndex].specular.ptr(), params, kRGBA32FBytes);
            break;
        case LightParameter::Position:
            transformedPos = mv.product(angle::Vector4(params[0], params[1], params[2], params[3]));
            mLights[lightIndex].position[0] = transformedPos[0];
            mLights[lightIndex].position[1] = transformedPos[1];
            mLights[lightIndex].position[2] = transformedPos[2];
            mLights[lightIndex].position[3] = transformedPos[3];
            break;
        case LightParameter::SpotDirection:
            transformedPos = mv.product(angle::Vector4(params[0], params[1], params[2], 0.0f));
            mLights[lightIndex].direction[0] = transformedPos[0];
            mLights[lightIndex].direction[1] = transformedPos[1];
            mLights[lightIndex].direction[2] = transformedPos[2];
            break;
        case LightParameter::SpotExponent:
            mLights[lightIndex].spotlightExponent = *params;
            break;
        case LightParameter::SpotCutoff:
            mLights[lightIndex].spotlightCutoffAngle = *params;
            break;
        case LightParameter::ConstantAttenuation:
            mLights[lightIndex].attenuationConst = *params;
            break;
        case LightParameter::LinearAttenuation:
            mLights[lightIndex].attenuationLinear = *params;
            break;
        case LightParameter::QuadraticAttenuation:
            mLights[lightIndex].attenuationQuadratic = *params;
            break;
        default:
            return;
    }
}

void GLES1State::getLightParameters(GLenum light, LightParameter pname, GLfloat *params) const
{
    uint32_t lightIndex = light - GL_LIGHT0;
    switch (pname)
    {
        case LightParameter::Ambient:
            memcpy(params, mLights[lightIndex].ambient.data(), kRGBA32FBytes);
            break;
        case LightParameter::Diffuse:
            memcpy(params, mLights[lightIndex].diffuse.data(), kRGBA32FBytes);
            break;
        case LightParameter::Specular:
            memcpy(params, mLights[lightIndex].specular.data(), kRGBA32FBytes);
            break;
        case LightParameter::Position:
            memcpy(params, mLights[lightIndex].position.data(), 4 * sizeof(GLfloat));
            break;
        case LightParameter::SpotDirection:
            memcpy(params, mLights[lightIndex].direction.data(), 3 * sizeof(GLfloat));
            break;
        case LightParameter::SpotExponent:
            *params = mLights[lightIndex].spotlightExponent;
            break;
        case LightParameter::SpotCutoff:
            *params = mLights[lightIndex].spotlightCutoffAngle;
            break;
        case LightParameter::ConstantAttenuation:
            *params = mLights[lightIndex].attenuationConst;
            break;
        case LightParameter::LinearAttenuation:
            *params = mLights[lightIndex].attenuationLinear;
            break;
        case LightParameter::QuadraticAttenuation:
            *params = mLights[lightIndex].attenuationQuadratic;
            break;
        default:
            break;
    }
}

void GLES1State::setLightModelParameters(GLenum pname, const GLfloat *params)
{
    switch (pname)
    {
        case GL_LIGHT_MODEL_AMBIENT:
            memcpy(mLightModel.color.ptr(), params, kRGBA32FBytes);
            break;
        case GL_LIGHT_MODEL_TWO_SIDE:
            mLightModel.twoSided = *params == 1.0f ? true : false;
            break;
        default:
            break;
    }
}

void GLES1State::getLightModelParameters(GLenum pname, GLfloat *params) const
{
    switch (pname)
    {
        case GL_LIGHT_MODEL_TWO_SIDE:
            *params = (float)mLightModel.twoSided;
            break;
        case GL_LIGHT_MODEL_AMBIENT:
            memcpy(params, mLightModel.color.data(), sizeof(mLightModel.color));
            break;
        default:
            break;
    }
}

bool GLES1State::isLightModelTwoSided() const
{
    return mLightModel.twoSided;
}

void GLES1State::setMaterialParameters(GLenum face, MaterialParameter pname, const GLfloat *params)
{
    switch (pname)
    {
        case MaterialParameter::Ambient:
            memcpy(mMaterial.ambient.ptr(), params, kRGBA32FBytes);
            break;
        case MaterialParameter::Diffuse:
            memcpy(mMaterial.diffuse.ptr(), params, kRGBA32FBytes);
            break;
        case MaterialParameter::AmbientAndDiffuse:
            memcpy(mMaterial.ambient.ptr(), params, kRGBA32FBytes);
            memcpy(mMaterial.diffuse.ptr(), params, kRGBA32FBytes);
            break;
        case MaterialParameter::Specular:
            memcpy(mMaterial.specular.ptr(), params, kRGBA32FBytes);
            break;
        case MaterialParameter::Emission:
            memcpy(mMaterial.emissive.ptr(), params, kRGBA32FBytes);
            break;
        case MaterialParameter::Shininess:
            mMaterial.specularExponent = *params;
            break;
        default:
            return;
    }
}

void GLES1State::getMaterialParameters(GLenum face, MaterialParameter pname, GLfloat *params) const
{
    switch (pname)
    {
        case MaterialParameter::Ambient:
            if (mColorMaterialEnabled)
            {
                memcpy(params, mCurrentColor.data(), kRGBA32FBytes);
            }
            else
            {
                memcpy(params, mMaterial.ambient.data(), kRGBA32FBytes);
            }
            break;
        case MaterialParameter::Diffuse:
            if (mColorMaterialEnabled)
            {
                memcpy(params, mCurrentColor.data(), kRGBA32FBytes);
            }
            else
            {
                memcpy(params, mMaterial.diffuse.data(), kRGBA32FBytes);
            }
            break;
        case MaterialParameter::Specular:
            memcpy(params, mMaterial.specular.data(), kRGBA32FBytes);
            break;
        case MaterialParameter::Emission:
            memcpy(params, mMaterial.emissive.data(), kRGBA32FBytes);
            break;
        case MaterialParameter::Shininess:
            *params = mMaterial.specularExponent;
            break;
        default:
            return;
    }
}

// static
unsigned int GLES1State::GetLightModelParameterCount(GLenum pname)
{
    switch (pname)
    {
        case GL_LIGHT_MODEL_AMBIENT:
            return 4;
        case GL_LIGHT_MODEL_TWO_SIDE:
            return 1;
        default:
            UNREACHABLE();
            return 0;
    }
}

// static
unsigned int GLES1State::GetLightParameterCount(LightParameter pname)
{
    switch (pname)
    {
        case LightParameter::Ambient:
        case LightParameter::Diffuse:
        case LightParameter::Specular:
        case LightParameter::Position:
            return 4;
        case LightParameter::SpotDirection:
            return 3;
        case LightParameter::SpotExponent:
        case LightParameter::SpotCutoff:
        case LightParameter::ConstantAttenuation:
        case LightParameter::LinearAttenuation:
        case LightParameter::QuadraticAttenuation:
            return 1;
        default:
            UNREACHABLE();
            return 0;
    }
}

// static
unsigned int GLES1State::GetMaterialParameterCount(MaterialParameter pname)
{
    switch (pname)
    {
        case MaterialParameter::Ambient:
        case MaterialParameter::Diffuse:
        case MaterialParameter::Specular:
        case MaterialParameter::Emission:
            return 4;
        case MaterialParameter::Shininess:
            return 1;
        default:
            UNREACHABLE();
            return 0;
    }
}

}  // namespace gl
