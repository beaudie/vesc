//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// renderer11_utils.inl: Conversion functions and other utility routines
// specific to the OpenGL renderer.

#include "libANGLE/Error.h"
#include "libANGLE/renderer/gl/functionsgl.h"

namespace rx
{

namespace nativegl
{

gl::Error CheckForGLOutOfMemoryError(const FunctionsGL *functions)
{
    // Check for errors untill there is no longer an error and track if
    // a GL_OUT_OF_MEMORY was seen
    GLenum error = GL_NO_ERROR;
    bool sawOOM = false;

    do
    {
        error = functions->getError();

        // Should only expect to see NO_ERROR or OUT_OF_MEMORY
        ASSERT(error == GL_NO_ERROR || error == GL_OUT_OF_MEMORY);
        if (error == GL_OUT_OF_MEMORY)
        {
            sawOOM = true;
        }
    }
    while (error != GL_NO_ERROR);

    return gl::Error(sawOOM ? GL_OUT_OF_MEMORY : GL_NO_ERROR);
}

}

}

