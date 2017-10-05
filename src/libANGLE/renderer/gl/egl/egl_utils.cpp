//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// egl_utils.cpp: Utility routines specific to the EGL->EGL implementation.

#include "libANGLE/renderer/gl/egl/egl_utils.h"

#include "common/debug.h"

namespace rx
{

namespace native_egl
{

AttributeVector TrimAttributeMap(const egl::AttributeMap &attributes,
                                 const EGLint *nativeAttribs,
                                 size_t nativeAttribCount)
{
    ASSERT(nativeAttribs);

    AttributeVector result;
    for (size_t nativeAttribIndex = 0; nativeAttribIndex < nativeAttribCount; nativeAttribIndex++)
    {
        EGLint nativeAttrib = nativeAttribs[nativeAttribIndex];
        if (attributes.contains(nativeAttrib))
        {
            result.push_back(nativeAttrib);
            result.push_back(attributes.get(nativeAttrib));
        }
    }
    return result;
}

void FinalizeAttributeVector(AttributeVector *attributeVector)
{
    ASSERT(attributeVector);
    attributeVector->push_back(EGL_NONE);
}

}  // namespace egl

}  // namespace rx
