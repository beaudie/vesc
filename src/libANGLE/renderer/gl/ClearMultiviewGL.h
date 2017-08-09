//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ClearMultiviewLayeredGL:
//   A helper for clearing a subrange of layers in a multivuew layered framebuffer.
//

#ifndef LIBANGLE_RENDERER_GL_CLEARMULTIVIEWGL_H_
#define LIBANGLE_RENDERER_GL_CLEARMULTIVIEWGL_H_

#include "angle_gl.h"
#include "common/angleutils.h"
#include "libANGLE/Error.h"
#include "libANGLE/angletypes.h"

namespace gl
{
class FramebufferState;
}  // namespace gl

namespace rx
{
class FunctionsGL;
struct WorkaroundsGL;
class StateManagerGL;
class FramebufferGL;

// Enum containing the different types of Clear* commands.
enum class ClearCommandType
{
    Clear,
    ClearBufferfv,
    ClearBufferuiv,
    ClearBufferiv,
    ClearBufferfi
};

class ClearMultiviewGL : angle::NonCopyable
{
  public:
    ClearMultiviewGL(const FunctionsGL *functions,
                     const WorkaroundsGL &workarounds,
                     StateManagerGL *stateManager);
    ~ClearMultiviewGL();

    gl::Error clearLayeredFBO(const gl::FramebufferState &state,
                              ClearCommandType clearCommandType,
                              GLbitfield mask,
                              GLenum buffer,
                              GLint drawbuffer,
                              const uint8_t *values,
                              GLfloat depth,
                              GLint stencil);
    gl::Error clearSideBySideFBO(const gl::FramebufferState &state,
                                 const gl::Rectangle &scissorBase,
                                 ClearCommandType clearCommandType,
                                 GLbitfield mask,
                                 GLenum buffer,
                                 GLint drawbuffer,
                                 const uint8_t *values,
                                 GLfloat depth,
                                 GLint stencil);

    gl::Error initializeResources();

  private:
    void attachTextures(const gl::FramebufferState &state, int layer);
    void genericClear(ClearCommandType clearCommandType,
                      GLbitfield mask,
                      GLenum buffer,
                      GLint drawbuffer,
                      const uint8_t *values,
                      GLfloat depth,
                      GLint stencil);

    const FunctionsGL *mFunctions;
    const WorkaroundsGL &mWorkarounds;
    StateManagerGL *mStateManager;
    gl::DrawBufferMask mPrevDrawBufferMask;

    GLuint mFramebuffer;
};
}  // namespace rx;

#endif LIBANGLE_RENDERER_GL_CLEARMULTIVIEWGL_H_