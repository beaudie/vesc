//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points_egl.h : Defines the EGL entry points.

#ifndef LIBGLESV2_ENTRYPOINTSEGL_H_
#define LIBGLESV2_ENTRYPOINTSEGL_H_

#include <EGL/egl.h>
#include <export.h>

extern "C" {

// EGL 1.0
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLChooseConfig(EGLDisplay dpy,
                                                    const EGLint *attrib_list,
                                                    EGLConfig *configs,
                                                    EGLint config_size,
                                                    EGLint *num_config);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLCopyBuffers(EGLDisplay dpy,
                                                   EGLSurface surface,
                                                   EGLNativePixmapType target);
ANGLE_EXPORT EGLContext EGLAPIENTRY EGLCreateContext(EGLDisplay dpy,
                                                     EGLConfig config,
                                                     EGLContext share_context,
                                                     const EGLint *attrib_list);
ANGLE_EXPORT EGLSurface EGLAPIENTRY EGLCreatePbufferSurface(EGLDisplay dpy,
                                                            EGLConfig config,
                                                            const EGLint *attrib_list);
ANGLE_EXPORT EGLSurface EGLAPIENTRY EGLCreatePixmapSurface(EGLDisplay dpy,
                                                           EGLConfig config,
                                                           EGLNativePixmapType pixmap,
                                                           const EGLint *attrib_list);
ANGLE_EXPORT EGLSurface EGLAPIENTRY EGLCreateWindowSurface(EGLDisplay dpy,
                                                           EGLConfig config,
                                                           EGLNativeWindowType win,
                                                           const EGLint *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLDestroyContext(EGLDisplay dpy, EGLContext ctx);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLDestroySurface(EGLDisplay dpy, EGLSurface surface);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLGetConfigAttrib(EGLDisplay dpy,
                                                       EGLConfig config,
                                                       EGLint attribute,
                                                       EGLint *value);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLGetConfigs(EGLDisplay dpy,
                                                  EGLConfig *configs,
                                                  EGLint config_size,
                                                  EGLint *num_config);
ANGLE_EXPORT EGLDisplay EGLAPIENTRY EGLGetCurrentDisplay(void);
ANGLE_EXPORT EGLSurface EGLAPIENTRY EGLGetCurrentSurface(EGLint readdraw);
ANGLE_EXPORT EGLDisplay EGLAPIENTRY EGLGetDisplay(EGLNativeDisplayType display_id);
ANGLE_EXPORT EGLint EGLAPIENTRY EGLGetError(void);
ANGLE_EXPORT __eglMustCastToProperFunctionPointerType EGLAPIENTRY
EGLGetProcAddress(const char *procname);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLMakeCurrent(EGLDisplay dpy,
                                                   EGLSurface draw,
                                                   EGLSurface read,
                                                   EGLContext ctx);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLQueryContext(EGLDisplay dpy,
                                                    EGLContext ctx,
                                                    EGLint attribute,
                                                    EGLint *value);
ANGLE_EXPORT const char *EGLAPIENTRY EGLQueryString(EGLDisplay dpy, EGLint name);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLQuerySurface(EGLDisplay dpy,
                                                    EGLSurface surface,
                                                    EGLint attribute,
                                                    EGLint *value);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLSwapBuffers(EGLDisplay dpy, EGLSurface surface);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLTerminate(EGLDisplay dpy);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLWaitGL(void);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLWaitNative(EGLint engine);

// EGL 1.1
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLBindTexImage(EGLDisplay dpy,
                                                    EGLSurface surface,
                                                    EGLint buffer);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLReleaseTexImage(EGLDisplay dpy,
                                                       EGLSurface surface,
                                                       EGLint buffer);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLSurfaceAttrib(EGLDisplay dpy,
                                                     EGLSurface surface,
                                                     EGLint attribute,
                                                     EGLint value);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLSwapInterval(EGLDisplay dpy, EGLint interval);

// EGL 1.2
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLBindAPI(EGLenum api);
ANGLE_EXPORT EGLenum EGLAPIENTRY EGLQueryAPI(void);
ANGLE_EXPORT EGLSurface EGLAPIENTRY EGLCreatePbufferFromClientBuffer(EGLDisplay dpy,
                                                                     EGLenum buftype,
                                                                     EGLClientBuffer buffer,
                                                                     EGLConfig config,
                                                                     const EGLint *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLReleaseThread(void);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLWaitClient(void);

// EGL 1.4
ANGLE_EXPORT EGLContext EGLAPIENTRY EGLGetCurrentContext(void);

// EGL 1.5
ANGLE_EXPORT EGLSync EGLAPIENTRY EGLCreateSync(EGLDisplay dpy,
                                               EGLenum type,
                                               const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLDestroySync(EGLDisplay dpy, EGLSync sync);
ANGLE_EXPORT EGLint EGLAPIENTRY EGLClientWaitSync(EGLDisplay dpy,
                                                  EGLSync sync,
                                                  EGLint flags,
                                                  EGLTime timeout);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLGetSyncAttrib(EGLDisplay dpy,
                                                     EGLSync sync,
                                                     EGLint attribute,
                                                     EGLAttrib *value);
ANGLE_EXPORT EGLImage EGLAPIENTRY EGLCreateImage(EGLDisplay dpy,
                                                 EGLContext ctx,
                                                 EGLenum target,
                                                 EGLClientBuffer buffer,
                                                 const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLDestroyImage(EGLDisplay dpy, EGLImage image);
ANGLE_EXPORT EGLDisplay EGLAPIENTRY EGLGetPlatformDisplay(EGLenum platform,
                                                          void *native_display,
                                                          const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLSurface EGLAPIENTRY EGLCreatePlatformWindowSurface(EGLDisplay dpy,
                                                                   EGLConfig config,
                                                                   void *native_window,
                                                                   const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLSurface EGLAPIENTRY EGLCreatePlatformPixmapSurface(EGLDisplay dpy,
                                                                   EGLConfig config,
                                                                   void *native_pixmap,
                                                                   const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags);
}  // extern "C"

#endif  // LIBGLESV2_ENTRYPOINTSEGL_H_
