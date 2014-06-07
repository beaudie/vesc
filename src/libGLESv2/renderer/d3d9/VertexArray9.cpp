#include "precompiled.h"
//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexArray9.cpp: Defines the rx::VertexArray9 class which implements rx::VertexArrayImpl.

#include "libGLESv2/renderer/d3d9/VertexArray9.h"
#include "libGLESv2/renderer/d3d9/Renderer9.h"
#include "libGLESv2/renderer/d3d9/renderer9_utils.h"
#include "libGLESv2/main.h"

namespace rx
{

VertexArray9::VertexArray9(rx::Renderer9 *renderer) : VertexArrayImpl()
{
    mRenderer = renderer;
}

VertexArray9::~VertexArray9()
{
}

void VertexArray9::setElementArrayBuffer(const gl::Buffer *buffer)
{

}

void VertexArray9::setAttribute(size_t idx, const gl::VertexAttribute &attr)
{

}

void VertexArray9::setAttributeDivisor(size_t idx, GLuint divisor)
{

}

void VertexArray9::enableAttribute(size_t idx, bool enabledState)
{

}

}
