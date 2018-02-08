//
// Copyright (c) 2002-2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// GLES1on3.h: Defines GLES1 emulation rendering operations on top of a GLES3
// context. Used by Context.h.

#ifndef LIBANGLE_GLES1_EMULATION_ES_H_
#define LIBANGLE_GLES1_EMULATION_ES_H_

#include <unordered_map>

#include "angle_gl.h"
#include "common/angleutils.h"
#include "libANGLE/Context.h"
#include "libANGLE/ContextState.h"
#include "libANGLE/angletypes.h"

namespace gl
{

class GLES1on3
{
  public:
    GLES1on3(Context *context, State *glState);
    ~GLES1on3();

    // Interfaces to validation / context
    GLint vertexArrayIndex(GLenum type) const;
    void prepareDraw(GLenum mode);

  private:

    // The GLES3 Context/State on which we are emulating GLES1.
    Context *mContext;
    State *mGLState;

    // GLES1 emulation state
    struct GLES1DrawTexState
    {
        GLuint program;
        GLint xyzwNdcLoc;
        GLint texuvwhCropLoc;
        GLint texNdcDimsLoc;
        GLint samplerLoc;
    };

    struct LightingBuffer
    {
        GLint lightEnables[8];
        GLfloat lightAmbients[4 * 8];
        GLfloat lightDiffuses[4 * 8];
        GLfloat lightSpeculars[4 * 8];
        GLfloat lightPositions[4 * 8];
        GLfloat lightDirections[3 * 8];
        GLfloat spotlightExponents[8];
        GLfloat spotlightCutoffAngles[8];
        GLfloat attenuationConsts[8];
        GLfloat attenuationLinears[8];
        GLfloat attenuationQuadratics[8];
    };

    struct TexEnvBuffer
    {
        GLfloat textureMatrices[16 * 4];

        GLint modes[4];
        GLint combineRgbs[4];
        GLint combineAlphas[4];
        GLint src0rgbs[4];
        GLint src0alphas[4];
        GLint src1rgbs[4];
        GLint src1alphas[4];
        GLint src2rgbs[4];
        GLint src2alphas[4];
        GLint op0rgbs[4];
        GLint op0alphas[4];
        GLint op1rgbs[4];
        GLint op1alphas[4];
        GLint op2rgbs[4];
        GLint op2alphas[4];
        GLfloat envColors[4 * 4];
        GLfloat rgbScales[4];
        GLfloat alphaScales[4];
        GLint pointSpriteCoordReplaces[4];
    };

    struct GLES1DrawState
    {
        GLuint program;

        GLint projMatrixLoc;
        GLint modelviewMatrixLoc;
        GLint textureMatrixLoc;
        GLint modelviewInvTrLoc;

        GLint textureSampler0Loc;
        GLint textureCubeSampler0Loc;
        GLint textureSampler1Loc;
        GLint textureCubeSampler1Loc;
        GLint textureSampler2Loc;
        GLint textureCubeSampler2Loc;
        GLint textureSampler3Loc;
        GLint textureCubeSampler3Loc;

        GLint shadeModelFlatLoc;

        GLint enableTexture2DLoc;
        GLint enableTextureCubeMapLoc;
        GLint enableLightingLoc;
        GLint enableRescaleNormalLoc;
        GLint enableNormalizeLoc;
        GLint enableColorMaterialLoc;
        GLint enableFogLoc;
        GLint enableReflectionMapLoc;
        GLint enableAlphaTestLoc;
        GLint enableLogicOpLoc;

        GLint alphaFuncLoc;
        GLint alphaTestRefLoc;

        GLint logicOpLoc;

        GLint textureFormatLoc;
        GLint textureEnvModeLoc;
        GLint combineRgbLoc;
        GLint combineAlphaLoc;
        GLint src0rgbLoc;
        GLint src0alphaLoc;
        GLint src1rgbLoc;
        GLint src1alphaLoc;
        GLint src2rgbLoc;
        GLint src2alphaLoc;
        GLint op0rgbLoc;
        GLint op0alphaLoc;
        GLint op1rgbLoc;
        GLint op1alphaLoc;
        GLint op2rgbLoc;
        GLint op2alphaLoc;
        GLint textureEnvColorLoc;
        GLint rgbScaleLoc;
        GLint alphaScaleLoc;
        GLint pointSpriteCoordReplaceLoc;

        GLint materialAmbientLoc;
        GLint materialDiffuseLoc;
        GLint materialSpecularLoc;
        GLint materialEmissiveLoc;
        GLint materialSpecularExponentLoc;

        GLint lightModelSceneAmbientLoc;
        GLint lightModelTwoSidedLoc;

        GLint lightEnablesLoc;
        GLint lightAmbientsLoc;
        GLint lightDiffusesLoc;
        GLint lightSpecularsLoc;
        GLint lightPositionsLoc;
        GLint lightDirectionsLoc;
        GLint lightSpotlightExponentsLoc;
        GLint lightSpotlightCutoffAnglesLoc;
        GLint lightAttenuationConstsLoc;
        GLint lightAttenuationLinearsLoc;
        GLint lightAttenuationQuadraticsLoc;

        GLint fogModeLoc;
        GLint fogDensityLoc;
        GLint fogStartLoc;
        GLint fogEndLoc;
        GLint fogColorLoc;

        GLint pointRasterizationLoc;
        GLint pointSpriteEnabledLoc;
        GLint pointSmoothEnabledLoc;
        GLint pointSizeMinLoc;
        GLint pointSizeMaxLoc;
        GLint pointDistanceAttenuationLoc;

        GLint enableClipPlanesLoc;
        GLint clipPlaneEnablesLoc;
        GLint clipPlanesLoc;

        LightingBuffer lightingBuffer;
        TexEnvBuffer texEnvBuffer;
    };

    GLES1DrawTexState drawTex;
    GLES1DrawState mDrawState;
};

}  // namespace gl

#endif  // LIBANGLE_GLES1_EMULATION_ES_H_
