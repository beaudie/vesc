//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramBinaryD3D.h: Defines the rx::ProgramBinaryD3D class which implements rx::ProgramBinaryImpl.

#ifndef LIBGLESV2_RENDERER_PROGRAMBINARYD3D_H_
#define LIBGLESV2_RENDERER_PROGRAMBINARYD3D_H_

#include "libGLESv2/renderer/ProgramBinaryImpl.h"

namespace gl
{
struct LinkedUniform;
struct VertexFormat;
}

namespace rx
{

class UniformStorage;

class ProgramBinaryD3D : public ProgramBinaryImpl
{
  public:
    ProgramBinaryD3D(rx::Renderer *renderer);
    virtual ~ProgramBinaryD3D();

    static ProgramBinaryD3D *makeProgramBinaryD3D(ProgramBinaryImpl *impl);
    static const ProgramBinaryD3D *makeProgramBinaryD3D(const ProgramBinaryImpl *impl);

    Renderer *getRenderer() { return mRenderer; }
    DynamicHLSL *getDynamicHLSL() { return mDynamicHLSL; }

    // D3D only
    void initializeUniformStorage(const std::vector<gl::LinkedUniform*> &uniforms);

    const UniformStorage &getVertexUniformStorage() const { return *mVertexUniformStorage; }
    const UniformStorage &getFragmentUniformStorage() const { return *mFragmentUniformStorage; }

    void reset();

  private:
    DISALLOW_COPY_AND_ASSIGN(ProgramBinaryD3D);

    Renderer *mRenderer;
    DynamicHLSL *mDynamicHLSL;

    UniformStorage *mVertexUniformStorage;
    UniformStorage *mFragmentUniformStorage;
};

}

#endif // LIBGLESV2_RENDERER_PROGRAMBINARYD3D_H_
