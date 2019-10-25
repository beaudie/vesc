//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TransformFeedbackGL.h: Defines the class interface for TransformFeedbackGL.

#ifndef LIBANGLE_RENDERER_GL_TRANSFORMFEEDBACKGL_H_
#define LIBANGLE_RENDERER_GL_TRANSFORMFEEDBACKGL_H_

#include "libANGLE/renderer/TransformFeedbackImpl.h"

namespace rx
{

class TransformFeedbackGL : public TransformFeedbackImpl
{
  public:
    TransformFeedbackGL(const gl::TransformFeedbackState &state, GLuint transformFeedbackID);
    ~TransformFeedbackGL() override;

    void onDestroy(const gl::Context *context) override;

    angle::Result begin(const gl::Context *context, gl::PrimitiveMode primitiveMode) override;
    angle::Result end(const gl::Context *context) override;
    angle::Result pause(const gl::Context *context) override;
    angle::Result resume(const gl::Context *context) override;

    angle::Result bindIndexedBuffer(const gl::Context *context,
                                    size_t index,
                                    const gl::OffsetBindingPointer<gl::Buffer> &binding) override;

    GLuint getTransformFeedbackID() const;

    angle::Result syncActiveState(const gl::Context *context,
                                  bool active,
                                  gl::PrimitiveMode primitiveMode);
    angle::Result syncPausedState(const gl::Context *context, bool paused);

  private:
    GLuint mTransformFeedbackID;

    bool mIsActive;
    bool mIsPaused;
    GLuint mActiveProgram;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_TRANSFORMFEEDBACKGL_H_
