#include "precompiled.h"
//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexArray11.cpp: Defines the rx::VertexArray11 class which implements rx::VertexArrayImpl.

#include "libGLESv2/renderer/d3d11/VertexArray11.h"
#include "libGLESv2/renderer/d3d11/Renderer11.h"
#include "libGLESv2/renderer/d3d11/renderer11_utils.h"
#include "libGLESv2/main.h"

namespace rx
{

VertexArray11::VertexArray11(rx::Renderer11 *renderer) : VertexArrayImpl()
{
    mRenderer = renderer;
}

VertexArray11::~VertexArray11()
{
}

void VertexArray11::setElementArrayBuffer(const gl::Buffer *buffer)
{

}

void VertexArray11::setAttribute(size_t idx, const gl::VertexAttribute &attr)
{

}

void VertexArray11::setAttributeDivisor(size_t idx, GLuint divisor)
{

}

void VertexArray11::enableAttribute(size_t idx, bool enabledState)
{

}

}
