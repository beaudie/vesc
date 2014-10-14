//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderbufferImpl.h: Defines the abstract class gl::RenderbufferImpl

#ifndef LIBGLESV2_RENDERER_RENDERBUFFERIMPL_H_
#define LIBGLESV2_RENDERER_RENDERBUFFERIMPL_H_

#include "angle_gl.h"

#include "common/angleutils.h"

namespace rx
{
class RenderTarget;

class RenderbufferImpl
{
public:
    RenderbufferImpl();

    virtual ~RenderbufferImpl() = 0;

    virtual RenderTarget *getRenderTarget();
    virtual void invalidate(GLint x, GLint y, GLsizei width, GLsizei height) = 0;

    virtual GLsizei getWidth() const;
    virtual GLsizei getHeight() const;
    virtual GLenum getInternalFormat() const;
    virtual GLenum getActualFormat() const;
    virtual GLsizei getSamples() const;

    virtual unsigned int getSerial() const;

    virtual bool isTexture() const;
    virtual unsigned int getTextureSerial() const;

    static unsigned int issueSerials(unsigned int count);

protected:
    GLsizei mWidth;
    GLsizei mHeight;
    GLenum mInternalFormat;
    GLenum mActualFormat;
    GLsizei mSamples;

private:
    DISALLOW_COPY_AND_ASSIGN(RenderbufferImpl);

    const unsigned int mSerial;

    static unsigned int mCurrentSerial;
};

}

#endif   // LIBGLESV2_RENDERER_RENDERBUFFERIMPL_H_
