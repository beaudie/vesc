//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CompilerGL.h: Defines the class interface for CompilerGL.

#ifndef LIBANGLE_RENDERER_GL_COMPILERGL_H_
#define LIBANGLE_RENDERER_GL_COMPILERGL_H_

#include "libANGLE/renderer/CompilerImpl.h"

namespace angle
{
struct FeaturesGL;
}  // namespace angle

namespace gl
{
struct Extensions;
}  // namespace gl

namespace rx
{
class FunctionsGL;

class CompilerGL : public CompilerImpl
{
  public:
    CompilerGL(const FunctionsGL *, const gl::Extensions &, const angle::FeaturesGL &);
    ~CompilerGL() override {}

    ShShaderOutput getTranslatorOutputType() const override;

  private:
    ShShaderOutput mTranslatorOutputType;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_COMPILERGL_H_
