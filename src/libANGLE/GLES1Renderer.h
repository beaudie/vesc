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

namespace gl
{

class Context;
class State;

class GLES1Renderer
{
  public:
    GLES1Renderer(Context *context, State *state);
    ~GLES1Renderer();

    void prepareForDraw();

  private:
    Context *mContext;
    State *mGLState;

    static constexpr int kTexUnitCount = 4;

    int mPositionAttribIndex;
    int mNormalAttribIndex;
    int mColorAttribIndex;
    int mPointsizeAttribIndex;
    int mTexcoordAttribIndexBase;

    struct GLES1ProgramState
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
    };

    struct GLES1UniformBuffers
    {
        std::array<float, kTexUnitCount * 16> textureMatrices;
        std::array<int, kTexUnitCount> tex2DEnables;
        std::array<int, kTexUnitCount> texCubeEnables;
    };

    struct GLES1DrawState
    {
        GLES1UniformBuffers uniformBuffers;
        GLES1ProgramState programState;
    };

    GLES1DrawState mDrawState;
};

}  // namespace gl

#endif  // LIBANGLE_GLES1_RENDERER_H_
