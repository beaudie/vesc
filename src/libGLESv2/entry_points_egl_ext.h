//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points_egl_ext.h : Defines the EGL extension entry points.

#ifndef LIBGLESV2_ENTRYPOINTSEGLEXT_H_
#define LIBGLESV2_ENTRYPOINTSEGLEXT_H_

#if defined(EGL_EGL_PROTOTYPES) && EGL_EGL_PROTOTYPES
#    error EGL functions would be multiple defined.
#endif  // defined(EGL_EGL_PROTOTYPES) && EGL_EGL_PROTOTYPES

#if defined(EGL_EGLEXT_PROTOTYPES)
#    error EGL functions would be multiple defined.
#endif  // defined(EGL_EGLEXT_PROTOTYPES)

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <export.h>

extern "C" {

// EGL_ANGLE_query_surface_pointer
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglQuerySurfacePointerANGLE(EGLDisplay dpy,
                                                                EGLSurface surface,
                                                                EGLint attribute,
                                                                void **value);

// EGL_NV_post_sub_buffer
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglPostSubBufferNV(EGLDisplay dpy,
                                                       EGLSurface surface,
                                                       EGLint x,
                                                       EGLint y,
                                                       EGLint width,
                                                       EGLint height);

// EGL_EXT_platform_base
ANGLE_EXPORT EGLDisplay EGLAPIENTRY eglGetPlatformDisplayEXT(EGLenum platform,
                                                             void *native_display,
                                                             const EGLint *attrib_list);
ANGLE_EXPORT EGLSurface EGLAPIENTRY eglCreatePlatformWindowSurfaceEXT(EGLDisplay dpy,
                                                                      EGLConfig config,
                                                                      void *native_window,
                                                                      const EGLint *attrib_list);
ANGLE_EXPORT EGLSurface EGLAPIENTRY eglCreatePlatformPixmapSurfaceEXT(EGLDisplay dpy,
                                                                      EGLConfig config,
                                                                      void *native_pixmap,
                                                                      const EGLint *attrib_list);

// EGL_EXT_device_query
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglQueryDisplayAttribEXT(EGLDisplay dpy,
                                                             EGLint attribute,
                                                             EGLAttrib *value);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglQueryDeviceAttribEXT(EGLDeviceEXT device,
                                                            EGLint attribute,
                                                            EGLAttrib *value);
ANGLE_EXPORT const char *EGLAPIENTRY eglQueryDeviceStringEXT(EGLDeviceEXT device, EGLint name);

// EGL_KHR_image_base/EGL_KHR_image
ANGLE_EXPORT EGLImageKHR EGLAPIENTRY eglCreateImageKHR(EGLDisplay dpy,
                                                       EGLContext ctx,
                                                       EGLenum target,
                                                       EGLClientBuffer buffer,
                                                       const EGLint *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglDestroyImageKHR(EGLDisplay dpy, EGLImageKHR image);

// EGL_ANGLE_device_creation
ANGLE_EXPORT EGLDeviceEXT EGLAPIENTRY eglCreateDeviceANGLE(EGLint device_type,
                                                           void *native_device,
                                                           const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglReleaseDeviceANGLE(EGLDeviceEXT device);

// EGL_KHR_stream
ANGLE_EXPORT EGLStreamKHR EGLAPIENTRY eglCreateStreamKHR(EGLDisplay dpy, const EGLint *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglDestroyStreamKHR(EGLDisplay dpy, EGLStreamKHR stream);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglStreamAttribKHR(EGLDisplay dpy,
                                                       EGLStreamKHR stream,
                                                       EGLenum attribute,
                                                       EGLint value);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglQueryStreamKHR(EGLDisplay dpy,
                                                      EGLStreamKHR stream,
                                                      EGLenum attribute,
                                                      EGLint *value);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglQueryStreamu64KHR(EGLDisplay dpy,
                                                         EGLStreamKHR stream,
                                                         EGLenum attribute,
                                                         EGLuint64KHR *value);

// EGL_KHR_stream_consumer_gltexture
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglStreamConsumerGLTextureExternalKHR(EGLDisplay dpy,
                                                                          EGLStreamKHR stream);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglStreamConsumerAcquireKHR(EGLDisplay dpy,
                                                                EGLStreamKHR stream);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglStreamConsumerReleaseKHR(EGLDisplay dpy,
                                                                EGLStreamKHR stream);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY
eglStreamConsumerGLTextureExternalAttribsNV(EGLDisplay dpy,
                                            EGLStreamKHR stream,
                                            const EGLAttrib *attrib_list);

// EGL_ANGLE_stream_producer_d3d_texture
ANGLE_EXPORT EGLBoolean EGLAPIENTRY
eglCreateStreamProducerD3DTextureANGLE(EGLDisplay dpy,
                                       EGLStreamKHR stream,
                                       const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglStreamPostD3DTextureANGLE(EGLDisplay dpy,
                                                                 EGLStreamKHR stream,
                                                                 void *texture,
                                                                 const EGLAttrib *attrib_list);

// EGL_CHROMIUM_get_sync_values
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglGetSyncValuesCHROMIUM(EGLDisplay dpy,
                                                             EGLSurface surface,
                                                             EGLuint64KHR *ust,
                                                             EGLuint64KHR *msc,
                                                             EGLuint64KHR *sbc);

// EGL_KHR_swap_buffers_with_damage
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglSwapBuffersWithDamageKHR(EGLDisplay dpy,
                                                                EGLSurface surface,
                                                                EGLint *rects,
                                                                EGLint n_rects);

// EGL_ANDROID_presentation_time
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglPresentationTimeANDROID(EGLDisplay dpy,
                                                               EGLSurface surface,
                                                               EGLnsecsANDROID time);

// EGL_ANDRIOD_blob_cache
ANGLE_EXPORT void EGLAPIENTRY eglSetBlobCacheFuncsANDROID(EGLDisplay dpy,
                                                          EGLSetBlobFuncANDROID set,
                                                          EGLGetBlobFuncANDROID get);

// EGL_ANGLE_program_cache_control
ANGLE_EXPORT EGLint EGLAPIENTRY eglProgramCacheGetAttribANGLE(EGLDisplay dpy, EGLenum attrib);
ANGLE_EXPORT void EGLAPIENTRY eglProgramCacheQueryANGLE(EGLDisplay dpy,
                                                        EGLint index,
                                                        void *key,
                                                        EGLint *keysize,
                                                        void *binary,
                                                        EGLint *binarysize);
ANGLE_EXPORT void EGLAPIENTRY eglProgramCachePopulateANGLE(EGLDisplay dpy,
                                                           const void *key,
                                                           EGLint keysize,
                                                           const void *binary,
                                                           EGLint binarysize);
ANGLE_EXPORT EGLint EGLAPIENTRY eglProgramCacheResizeANGLE(EGLDisplay dpy,
                                                           EGLint limit,
                                                           EGLenum mode);

// EGL_KHR_debug
ANGLE_EXPORT EGLint EGLAPIENTRY eglDebugMessageControlKHR(EGLDEBUGPROCKHR callback,
                                                          const EGLAttrib *attrib_list);

ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglQueryDebugKHR(EGLint attribute, EGLAttrib *value);

ANGLE_EXPORT EGLint EGLAPIENTRY eglLabelObjectKHR(EGLDisplay display,
                                                  EGLenum objectType,
                                                  EGLObjectKHR object,
                                                  EGLLabelKHR label);

// ANDROID_get_frame_timestamps
ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglGetCompositorTimingSupportedANDROID(EGLDisplay dpy,
                                                                           EGLSurface surface,
                                                                           EGLint name);

ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglGetCompositorTimingANDROID(EGLDisplay dpy,
                                                                  EGLSurface surface,
                                                                  EGLint numTimestamps,
                                                                  const EGLint *names,
                                                                  EGLnsecsANDROID *values);

ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglGetNextFrameIdANDROID(EGLDisplay dpy,
                                                             EGLSurface surface,
                                                             EGLuint64KHR *frameId);

ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglGetFrameTimestampSupportedANDROID(EGLDisplay dpy,
                                                                         EGLSurface surface,
                                                                         EGLint timestamp);

ANGLE_EXPORT EGLBoolean EGLAPIENTRY eglGetFrameTimestampsANDROID(EGLDisplay dpy,
                                                                 EGLSurface surface,
                                                                 EGLuint64KHR frameId,
                                                                 EGLint numTimestamps,
                                                                 const EGLint *timestamps,
                                                                 EGLnsecsANDROID *values);

}  // extern "C"

#endif  // LIBGLESV2_ENTRYPOINTSEGLEXT_H_
