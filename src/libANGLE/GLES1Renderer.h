//
// Copyright (c) 2002-2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// GLES1Renderer.h: Defines GLES1 emulation rendering operations on top of a GLES3
// context. Used by Context.h.

#ifndef LIBANGLE_GLES1_RENDERER_H_
#define LIBANGLE_GLES1_RENDERER_H_

#include "angle_gl.h"
#include "common/angleutils.h"
#include "libANGLE/angletypes.h"

#include <string>
#include <unordered_map>

namespace gl
{

class Context;
class State;

class GLES1Renderer
{
  public:
    GLES1Renderer(Context *context, State *state);
    ~GLES1Renderer();

    Error prepareForDraw();

  private:

    using Mat4Uniform = float[16];

    GLuint compileShader(ShaderType shaderType, const char *src);
    GLuint linkProgram(GLuint vshader,
                       GLuint fshader,
                       const std::unordered_map<GLint, std::string> &attribLocs);
    bool initializeRendererProgram();

    Context *mContext;
    State *mGLState;

    static constexpr int kTexUnitCount = 4;

    bool mRendererProgramInitialized;

    struct GLES1ProgramState
    {
        GLint positionAttribIndex;
        GLint normalAttribIndex;
        GLint colorAttribIndex;
        GLint pointsizeAttribIndex;
        GLint texcoordAttribIndexBase;

        GLuint program;

        GLint projMatrixLoc;
        GLint modelviewMatrixLoc;
        GLint textureMatrixLoc;
        GLint modelviewInvTrLoc;

        std::array<GLint, kTexUnitCount> tex2DSamplerLocs;
        std::array<GLint, kTexUnitCount> texCubeSamplerLocs;

        GLint shadeModelFlatLoc;

        GLint enableTexture2DLoc;
        GLint enableTextureCubeMapLoc;
    };

    struct GLES1UniformBuffers
    {
        std::array<Mat4Uniform, kTexUnitCount> textureMatrices;
        std::array<GLint, kTexUnitCount> tex2DEnables;
        std::array<GLint, kTexUnitCount> texCubeEnables;
    };

    GLES1UniformBuffers mUniformBuffers;
    GLES1ProgramState mProgramState;
};

}  // namespace gl

#endif  // LIBANGLE_GLES1_RENDERER_H_
