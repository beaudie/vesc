//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// GLES1Renderer.h: Defines GLES1 emulation rendering operations on top of a GLES3
// context. Used by Context.h.

#ifndef LIBANGLE_GLES1_RENDERER_H_
#define LIBANGLE_GLES1_RENDERER_H_

#include "GLES1State.h"
#include "angle_gl.h"
#include "common/angleutils.h"
#include "libANGLE/angletypes.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace gl
{
class Context;
class GLES1State;
class Program;
class State;
class Shader;
class ShaderProgramManager;

enum class GLES1StateEnables : uint64_t
{
    Lighting                  = 0,
    Fog                       = 1,
    ClipPlanes                = 2,
    DrawTexture               = 3,
    PointRasterization        = 4,
    PointSprite               = 5,
    RescaleNormal             = 6,
    Normalize                 = 7,
    AlphaTest                 = 8,
    ShadeModelFlat            = 9,
    ColorMaterial             = 10,
    LightModelTwoSided        = 11,
    PointSpriteCoordReplaces0 = 12,
    PointSpriteCoordReplaces1 = 13,
    PointSpriteCoordReplaces2 = 14,
    PointSpriteCoordReplaces3 = 15,
    Light0                    = 16,
    Light1                    = 17,
    Light2                    = 18,
    Light3                    = 19,
    Light4                    = 20,
    Light5                    = 21,
    Light6                    = 22,
    Light7                    = 23,
    ClipPlane0                = 24,
    ClipPlane1                = 25,
    ClipPlane2                = 26,
    ClipPlane3                = 27,
    ClipPlane4                = 28,
    ClipPlane5                = 29,

    InvalidEnum = 30,
    EnumCount   = 30,
};

using Mat4Uniform = float[16];
using Vec4Uniform = float[4];
using Vec3Uniform = float[3];

static constexpr int kTexUnitCount   = 4;
static constexpr int kLightCount     = 8;
static constexpr int kClipPlaneCount = 6;

struct GLES1ShaderStates
{
    GLES1ShaderStates();
    ~GLES1ShaderStates();

    using GLES1StateEnabledBitSet = angle::PackedEnumBitSet<GLES1StateEnables, uint64_t>;
    GLES1StateEnabledBitSet mGLES1StateEnabled;

    using BoolTexArray       = bool[kTexUnitCount];
    using IntTexArray        = int[kTexUnitCount];
    using FloatTexArray      = float[kTexUnitCount];
    using Vec4TexArray       = Vec4Uniform[kTexUnitCount];
    using BoolLightArray     = bool[kLightCount];
    using FloatLightArray    = float[kLightCount];
    using Vec3LightArray     = Vec3Uniform[kLightCount];
    using Vec4LightArray     = Vec4Uniform[kLightCount];
    using Vec4ClipPlaneArray = Vec4Uniform[kClipPlaneCount];

    BoolTexArray tex2DEnables   = {false, false, false, false};
    BoolTexArray texCubeEnables = {false, false, false, false};

    IntTexArray tex2DFormats = {GL_RGBA, GL_RGBA, GL_RGBA, GL_RGBA};

    Vec4TexArray texEnvColors;
    FloatTexArray texEnvRgbScales;
    FloatTexArray texEnvAlphaScales;

    IntTexArray texEnvModes;
    IntTexArray texCombineRgbs;
    IntTexArray texCombineAlphas;
    IntTexArray texCombineSrc0Rgbs;
    IntTexArray texCombineSrc0Alphas;
    IntTexArray texCombineSrc1Rgbs;
    IntTexArray texCombineSrc1Alphas;
    IntTexArray texCombineSrc2Rgbs;
    IntTexArray texCombineSrc2Alphas;
    IntTexArray texCombineOp0Rgbs;
    IntTexArray texCombineOp0Alphas;
    IntTexArray texCombineOp1Rgbs;
    IntTexArray texCombineOp1Alphas;
    IntTexArray texCombineOp2Rgbs;
    IntTexArray texCombineOp2Alphas;

    AlphaTestFunc mAlphaTestFunc;
    GLfloat mAlphaTestRef;

    // MaterialParameters mMaterial;
    Vec4Uniform materialAmbient;
    Vec4Uniform materialDiffuse;
    Vec4Uniform materialSpecular;
    Vec4Uniform materialEmissive;
    GLfloat materialSpecularExponent;

    // LightModelParameters mLightModel;
    Vec4Uniform lightModelColor;
    bool lightModelTwoSided;

    // LightParameters mLights[kLightCount];
    BoolLightArray lightEnabled;
    Vec4LightArray lightAmbient;
    Vec4LightArray lightDiffuse;
    Vec4LightArray lightSpecular;
    Vec4LightArray lightPosition;
    Vec3LightArray lightDirection;
    FloatLightArray lightSpotlightExponent;
    FloatLightArray lightSpotlightCutoffAngle;
    FloatLightArray lightAttenuationConst;
    FloatLightArray lightAttenuationLinear;
    FloatLightArray lightAttenuationQuadratic;

    // FogParameters mFog;
    FogMode fogMode;
    GLfloat fogDensity;
    GLfloat fogStart;
    GLfloat fogEnd;
    Vec4Uniform fogColor;

    // uniform vec4 clip_planes[kMaxClipPlanes];
    Vec4ClipPlaneArray clipPlanes;
};

class GLES1Renderer final : angle::NonCopyable
{
  public:
    GLES1Renderer();
    ~GLES1Renderer();

    void onDestroy(Context *context, State *state);

    angle::Result prepareForDraw(PrimitiveMode mode, Context *context, State *glState);

    static int VertexArrayIndex(ClientVertexArrayType type, const GLES1State &gles1);
    static ClientVertexArrayType VertexArrayType(int attribIndex);
    static int TexCoordArrayIndex(unsigned int unit);

    void drawTexture(Context *context,
                     State *glState,
                     float x,
                     float y,
                     float z,
                     float width,
                     float height);

  private:
    Shader *getShader(ShaderProgramID handle) const;
    Program *getProgram(ShaderProgramID handle) const;

    angle::Result compileShader(Context *context,
                                ShaderType shaderType,
                                const char *src,
                                ShaderProgramID *shaderOut);
    angle::Result linkProgram(Context *context,
                              State *glState,
                              ShaderProgramID vshader,
                              ShaderProgramID fshader,
                              const angle::HashMap<GLint, std::string> &attribLocs,
                              ShaderProgramID *programOut);
    angle::Result initializeRendererProgram(Context *context, State *glState);

    void setUniform1i(Context *context,
                      Program *programObject,
                      UniformLocation location,
                      GLint value);
    void setUniform1iv(Context *context,
                       Program *programObject,
                       UniformLocation location,
                       GLint count,
                       const GLint *value);
    void setUniformMatrix4fv(Program *programObject,
                             UniformLocation location,
                             GLint count,
                             GLboolean transpose,
                             const GLfloat *value);
    void setUniform4fv(Program *programObject,
                       UniformLocation location,
                       GLint count,
                       const GLfloat *value);
    void setUniform3fv(Program *programObject,
                       UniformLocation location,
                       GLint count,
                       const GLfloat *value);
    void setUniform2fv(Program *programObject,
                       UniformLocation location,
                       GLint count,
                       const GLfloat *value);
    void setUniform1f(Program *programObject, UniformLocation location, GLfloat value);
    void setUniform1fv(Program *programObject,
                       UniformLocation location,
                       GLint count,
                       const GLfloat *value);

    void setAttributesEnabled(Context *context, State *glState, AttributesMask mask);

    static constexpr int kVertexAttribIndex           = 0;
    static constexpr int kNormalAttribIndex           = 1;
    static constexpr int kColorAttribIndex            = 2;
    static constexpr int kPointSizeAttribIndex        = 3;
    static constexpr int kTextureCoordAttribIndexBase = 4;

    bool mRendererProgramInitialized;
    ShaderProgramManager *mShaderPrograms;

    GLES1ShaderStates mShaderStates;

    const char *getShaderBool(GLES1StateEnables state);
    void addShaderDefine(std::stringstream &outStream,
                         GLES1StateEnables state,
                         const char *enableString);
    void addShaderIntTexState(std::stringstream &outStream,
                              const char *texString,
                              GLES1ShaderStates::IntTexArray &texState);
    void addShaderVec4(std::stringstream &outStream, const char *name, Vec4Uniform &value);
    void addShaderVec4TexArray(std::stringstream &outStream,
                               const char *name,
                               GLES1ShaderStates::Vec4TexArray &value);
    void addFloatTexArray(std::stringstream &outStream,
                          const char *name,
                          GLES1ShaderStates::FloatTexArray &value);
    void addShaderVec3LightArray(std::stringstream &outStream,
                                 const char *name,
                                 GLES1ShaderStates::Vec3LightArray &value);
    void addShaderVec4LightArray(std::stringstream &outStream,
                                 const char *name,
                                 GLES1ShaderStates::Vec4LightArray &value);
    void addShaderVec4ClipPlaneArray(std::stringstream &outStream,
                                     const char *name,
                                     GLES1ShaderStates::Vec4ClipPlaneArray &value);
    void addFloatLightArray(std::stringstream &outStream,
                            const char *name,
                            GLES1ShaderStates::FloatLightArray &value);
    void addVertexShaderDefs(std::stringstream &outStream);
    void addFragmentShaderDefs(std::stringstream &outStream);

    struct GLES1ProgramState
    {
        ShaderProgramID program;

        UniformLocation projMatrixLoc;
        UniformLocation modelviewMatrixLoc;
        UniformLocation textureMatrixLoc;
        UniformLocation modelviewInvTrLoc;

        // Texturing
        std::array<UniformLocation, kTexUnitCount> tex2DSamplerLocs;
        std::array<UniformLocation, kTexUnitCount> texCubeSamplerLocs;

        // Point rasterization
        UniformLocation pointSizeMinLoc;
        UniformLocation pointSizeMaxLoc;
        UniformLocation pointDistanceAttenuationLoc;

        // Draw texture
        UniformLocation drawTextureCoordsLoc;
        UniformLocation drawTextureDimsLoc;
        UniformLocation drawTextureNormalizedCropRectLoc;
    };

    struct GLES1UniformBuffers
    {
        std::array<Mat4Uniform, kTexUnitCount> textureMatrices;

        // Texture crop rectangles
        std::array<Vec4Uniform, kTexUnitCount> texCropRects;
    };

    angle::HashMap<size_t, GLES1UniformBuffers> mUniformBuffers;
    angle::HashMap<size_t, GLES1ProgramState> mProgramStates;

    bool mDrawTextureEnabled      = false;
    GLfloat mDrawTextureCoords[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    GLfloat mDrawTextureDims[2]   = {0.0f, 0.0f};
};

}  // namespace gl

#endif  // LIBANGLE_GLES1_RENDERER_H_
