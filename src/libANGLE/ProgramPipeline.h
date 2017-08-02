//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramPipeline.h: Defines the gl::ProgramPipeline class.
// Implements GL program pipeline objects and related functionality.
// [OpenGL ES 3.1] section 7.4 page 105.

#ifndef LIBANGLE_PROGRAMPIPELINE_H_
#define LIBANGLE_PROGRAMPIPELINE_H_

#include "common/angleutils.h"
#include "libANGLE/Debug.h"
#include "libANGLE/Error.h"
#include "libANGLE/IndexRangeCache.h"
#include "libANGLE/RefCountObject.h"

namespace rx
{
class GLImplFactory;
class ProgramPipelineImpl;
};

namespace gl
{
struct Caps;
class Context;
class ContextState;
class ProgramPipeline;
class State;

class ProgramPipelineState final : angle::NonCopyable
{
  public:
    ProgramPipelineState();
    ~ProgramPipelineState();

    const std::string &getLabel();

  private:
    friend class ProgramPipeline;

    std::string mLabel;
};

class ProgramPipeline final : public RefCountObject, public LabeledObject
{
  public:
    ProgramPipeline(rx::GLImplFactory *factory, GLuint handle);
    ~ProgramPipeline() override;

    void onDestroy(const Context *context) {}

    GLuint id() const { return mHandle; }

    void setLabel(const std::string &label) override;
    const std::string &getLabel() const override;

    rx::ProgramPipelineImpl *getImplementation() const;

  private:
    rx::ProgramPipelineImpl *mProgramPipeline;

    ProgramPipelineState mState;

    const GLuint mHandle;
};
}

#endif  // LIBANGLE_PROGRAMPIPELINE_H_
