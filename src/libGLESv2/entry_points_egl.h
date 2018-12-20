//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points_egl.h : Defines the EGL entry points.

#ifndef LIBGLESV2_ENTRYPOINTS_EGL_H_
#define LIBGLESV2_ENTRYPOINTS_EGL_H_

#if defined(EGL_EGL_PROTOTYPES)
#error EGL functions would be multiple defined.
#endif  // defined(EGL_EGL_PROTOTYPES)

#include <EGL/egl.h>
#include <export.h>

extern "C" {

// EGL 1.0
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglChooseConfig(EGLDisplay dpy,
                                                    const EGLint *attrib_list,
                                                    EGLConfig *configs,
                                                    EGLint config_size,
                                                    EGLint *num_config);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglCopyBuffers(EGLDisplay dpy,
                                                   EGLSurface surface,
                                                   EGLNativePixmapType target);
ANGLE_EXPORT EGLContext EGLAPIENTRY eglCreateContext(EGLDisplay dpy,
                                                     EGLConfig config,
                                                     EGLContext share_context,
                                                     const EGLint *attrib_list);
ANGLE_EXPORT EGLSurface EGLAPIENTRY eglCreatePbufferSurface(EGLDisplay dpy,
                                                            EGLConfig config,
                                                            const EGLint *attrib_list);
ANGLE_EXPORT EGLSurface EGLAPIENTRY eglCreatePixmapSurface(EGLDisplay dpy,
                                                           EGLConfig config,
                                                           EGLNativePixmapType pixmap,
                                                           const EGLint *attrib_list);
ANGLE_EXPORT EGLSurface EGLAPIENTRY eglCreateWindowSurface(EGLDisplay dpy,
                                                           EGLConfig config,
                                                           EGLNativeWindowType win,
                                                           const EGLint *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglDestroyContext(EGLDisplay dpy, EGLContext ctx);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglDestroySurface(EGLDisplay dpy, EGLSurface surface);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglGetConfigAttrib(EGLDisplay dpy,
                                                       EGLConfig config,
                                                       EGLint attribute,
                                                       EGLint *value);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglGetConfigs(EGLDisplay dpy,
                                                  EGLConfig *configs,
                                                  EGLint config_size,
                                                  EGLint *num_config);
ANGLE_EXPORT EGLDisplay EGLAPIENTRY eglGetCurrentDisplay(void);
ANGLE_EXPORT EGLSurface EGLAPIENTRY eglGetCurrentSurface(EGLint readdraw);
ANGLE_EXPORT EGLDisplay EGLAPIENTRY eglGetDisplay(EGLNativeDisplayType display_id);
ANGLE_EXPORT EGLint EGLAPIENTRY eglGetError(void);
ANGLE_EXPORT __eglMustCastToProperFunctionPointerType EGLAPIENTRY
eglGetProcAddress(const char *procname);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglMakeCurrent(EGLDisplay dpy,
                                                   EGLSurface draw,
                                                   EGLSurface read,
                                                   EGLContext ctx);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglQueryContext(EGLDisplay dpy,
                                                    EGLContext ctx,
                                                    EGLint attribute,
                                                    EGLint *value);
ANGLE_EXPORT const char *EGLAPIENTRY eglQueryString(EGLDisplay dpy, EGLint name);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglQuerySurface(EGLDisplay dpy,
                                                    EGLSurface surface,
                                                    EGLint attribute,
                                                    EGLint *value);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglTerminate(EGLDisplay dpy);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglWaitGL(void);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglWaitNative(EGLint engine);

// EGL 1.1
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglBindTexImage(EGLDisplay dpy,
                                                    EGLSurface surface,
                                                    EGLint buffer);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglReleaseTexImage(EGLDisplay dpy,
                                                       EGLSurface surface,
                                                       EGLint buffer);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglSurfaceAttrib(EGLDisplay dpy,
                                                     EGLSurface surface,
                                                     EGLint attribute,
                                                     EGLint value);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglSwapInterval(EGLDisplay dpy, EGLint interval);

// EGL 1.2
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglBindAPI(EGLenum api);
ANGLE_EXPORT EGLenum EGLAPIENTRY eglQueryAPI(void);
ANGLE_EXPORT EGLSurface EGLAPIENTRY eglCreatePbufferFromClientBuffer(EGLDisplay dpy,
                                                                     EGLenum buftype,
                                                                     EGLClientBuffer buffer,
                                                                     EGLConfig config,
                                                                     const EGLint *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglReleaseThread(void);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglWaitClient(void);

// EGL 1.4
ANGLE_EXPORT EGLContext EGLAPIENTRY eglGetCurrentContext(void);

// EGL 1.5
ANGLE_EXPORT EGLSync EGLAPIENTRY eglCreateSync(EGLDisplay dpy,
                                               EGLenum type,
                                               const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglDestroySync(EGLDisplay dpy, EGLSync sync);
ANGLE_EXPORT EGLint EGLAPIENTRY eglClientWaitSync(EGLDisplay dpy,
                                                  EGLSync sync,
                                                  EGLint flags,
                                                  EGLTime timeout);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglGetSyncAttrib(EGLDisplay dpy,
                                                     EGLSync sync,
                                                     EGLint attribute,
                                                     EGLAttrib *value);
ANGLE_EXPORT EGLImage EGLAPIENTRY eglCreateImage(EGLDisplay dpy,
                                                 EGLContext ctx,
                                                 EGLenum target,
                                                 EGLClientBuffer buffer,
                                                 const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglDestroyImage(EGLDisplay dpy, EGLImage image);
ANGLE_EXPORT EGLDisplay EGLAPIENTRY eglGetPlatformDisplay(EGLenum platform,
                                                          void *native_display,
                                                          const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLSurface EGLAPIENTRY eglCreatePlatformWindowSurface(EGLDisplay dpy,
                                                                   EGLConfig config,
                                                                   void *native_window,
                                                                   const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLSurface EGLAPIENTRY eglCreatePlatformPixmapSurface(EGLDisplay dpy,
                                                                   EGLConfig config,
                                                                   void *native_pixmap,
                                                                   const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags);
}  // extern "C"

#endif  // LIBGLESV2_ENTRYPOINTS_EGL_H_
