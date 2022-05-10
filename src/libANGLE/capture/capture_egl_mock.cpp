//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// capture_egl.h:
//   EGL mock capture functions
//

#include "libANGLE/capture/capture_egl.h"

#if ANGLE_CAPTURE_ENABLED
#    error Frame capture must be disabled to include this file.
#endif  // ANGLE_CAPTURE_ENABLED

namespace angle
{

angle::CallCapture CaptureCreateNativeClientBufferANDROID(gl::Context *context,
                                                          const egl::AttributeMap &attribMap,
                                                          EGLClientBuffer eglClientBuffer)
{}
angle::CallCapture CaptureEGLCreateImage(gl::Context *context,
                                         EGLenum target,
                                         EGLClientBuffer buffer,
                                         const egl::AttributeMap &attributes,
                                         egl::Image *image)
{}

angle::CallCapture CaptureEGLDestroyImage(gl::Context *context,
                                          egl::Display *display,
                                          egl::Image *image)
{}

}  // namespace angle
