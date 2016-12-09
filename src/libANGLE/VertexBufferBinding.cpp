//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Implementation of vertex buffer binding point in ES3.1
//

#include "libANGLE/VertexBufferBinding.h"

namespace gl
{

VertexBufferBinding::VertexBufferBinding()
    : index(0), stride(0), divisor(0), offset(0), attribIndex(0)
{
}

}  // namespace gl