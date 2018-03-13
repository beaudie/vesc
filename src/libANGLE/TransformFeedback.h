//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_TRANSFORM_FEEDBACK_H_
#define LIBANGLE_TRANSFORM_FEEDBACK_H_

#include "libANGLE/RefCountObject.h"

#include "common/angleutils.h"
#include "common/mathutil.h"
#include "libANGLE/Debug.h"

#include "angle_gl.h"

namespace rx
{
class GLImplFactory;
class TransformFeedbackImpl;
}

namespace gl
{
class Buffer;
struct Caps;
class Context;
class Program;

class TransformFeedbackState final : angle::NonCopyable
{
  public:
    TransformFeedbackState(size_t maxIndexedBuffers);
    ~TransformFeedbackState();

    const OffsetBindingPointer<Buffer> &getIndexedBuffer(size_t idx) const;
    const std::vector<OffsetBindingPointer<Buffer>> &getIndexedBuffers() const;

  private:
    friend class TransformFeedback;

    std::string mLabel;

    bool mActive;
    GLenum mPrimitiveMode;
    bool mPaused;
    GLsizeiptr mVerticesDrawn;

    Program *mProgram;

    std::vector<OffsetBindingPointer<Buffer>> mIndexedBuffers;
};

class TransformFeedback final : public RefCountObject, public LabeledObject
{
  public:
    TransformFeedback(rx::GLImplFactory *implFactory, GLuint id, const Caps &caps);
    ~TransformFeedback() override;
    Error onDestroy(const Context *context) override;

    void setLabel(const std::string &label) override;
    const std::string &getLabel() const override;

    void begin(const Context *context, GLenum primitiveMode, Program *program);
    void end(const Context *context);
    void pause();
    void resume();

    bool isActive() const;
    bool isPaused() const;
    GLenum getPrimitiveMode() const;
    bool checkBufferSpaceForDraw(GLsizei count, GLsizei primcount) const;
    // Calculates the number of vertices that this draw call will write to the transform feedback
    // buffer, plus the number of vertices that were previously written since the last call to
    // BeginTransformFeedback (because vertices are written starting just after the last vertex
    // written by the previous draw). This is used to calculate whether there is enough space in the
    // transform feedback buffers. Returns false on integer overflow. This must be called every time
    // a transform feedback draw happens to keep track of how many vertices have been written to the
    // transform feedback buffers.
    void onVerticesDrawn(GLsizei count, GLsizei primcount);

    bool hasBoundProgram(GLuint program) const;

    void bindIndexedBuffer(const Context *context,
                           size_t index,
                           Buffer *buffer,
                           size_t offset,
                           size_t size);
    const OffsetBindingPointer<Buffer> &getIndexedBuffer(size_t index) const;
    size_t getIndexedBufferCount() const;

    // Returns true if any buffer bound to this object is also bound to another target.
    bool buffersBoundForOtherUse() const;

    void detachBuffer(const Context *context, GLuint bufferName);

    rx::TransformFeedbackImpl *getImplementation();
    const rx::TransformFeedbackImpl *getImplementation() const;

    void onBindingChanged(bool bound);

  private:
    void bindProgram(const Context *context, Program *program);
    angle::CheckedNumeric<GLsizeiptr> getVerticesNeededForDraw(GLsizei count,
                                                               GLsizei primcount) const;

    TransformFeedbackState mState;
    rx::TransformFeedbackImpl* mImplementation;
};

}

#endif // LIBANGLE_TRANSFORM_FEEDBACK_H_
