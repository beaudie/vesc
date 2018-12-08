//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points_egl_ext.h : Defines the EGL extension entry points.

#ifndef LIBGLESV2_ENTRYPOINTSEGLEXT_H_
#define LIBGLESV2_ENTRYPOINTSEGLEXT_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <export.h>

extern "C" {

// EGL_ANGLE_query_surface_pointer
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLQuerySurfacePointerANGLE(EGLDisplay dpy,
                                                                EGLSurface surface,
                                                                EGLint attribute,
                                                                void **value);

// EGL_NV_post_sub_buffer
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLPostSubBufferNV(EGLDisplay dpy,
                                                       EGLSurface surface,
                                                       EGLint x,
                                                       EGLint y,
                                                       EGLint width,
                                                       EGLint height);

// EGL_EXT_platform_base
ANGLE_EXPORT EGLDisplay EGLAPIENTRY EGLGetPlatformDisplayEXT(EGLenum platform,
                                                             void *native_display,
                                                             const EGLint *attrib_list);
ANGLE_EXPORT EGLSurface EGLAPIENTRY EGLCreatePlatformWindowSurfaceEXT(EGLDisplay dpy,
                                                                      EGLConfig config,
                                                                      void *native_window,
                                                                      const EGLint *attrib_list);
ANGLE_EXPORT EGLSurface EGLAPIENTRY EGLCreatePlatformPixmapSurfaceEXT(EGLDisplay dpy,
                                                                      EGLConfig config,
                                                                      void *native_pixmap,
                                                                      const EGLint *attrib_list);

// EGL_EXT_device_query
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLQueryDisplayAttribEXT(EGLDisplay dpy,
                                                             EGLint attribute,
                                                             EGLAttrib *value);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLQueryDeviceAttribEXT(EGLDeviceEXT device,
                                                            EGLint attribute,
                                                            EGLAttrib *value);
ANGLE_EXPORT const char *EGLAPIENTRY EGLQueryDeviceStringEXT(EGLDeviceEXT device, EGLint name);

// EGL_KHR_image_base/EGL_KHR_image
ANGLE_EXPORT EGLImageKHR EGLAPIENTRY EGLCreateImageKHR(EGLDisplay dpy,
                                                       EGLContext ctx,
                                                       EGLenum target,
                                                       EGLClientBuffer buffer,
                                                       const EGLint *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLDestroyImageKHR(EGLDisplay dpy, EGLImageKHR image);

// EGL_ANGLE_device_creation
ANGLE_EXPORT EGLDeviceEXT EGLAPIENTRY EGLCreateDeviceANGLE(EGLint device_type,
                                                           void *native_device,
                                                           const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLReleaseDeviceANGLE(EGLDeviceEXT device);

// EGL_KHR_stream
ANGLE_EXPORT EGLStreamKHR EGLAPIENTRY EGLCreateStreamKHR(EGLDisplay dpy, const EGLint *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLDestroyStreamKHR(EGLDisplay dpy, EGLStreamKHR stream);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLStreamAttribKHR(EGLDisplay dpy,
                                                       EGLStreamKHR stream,
                                                       EGLenum attribute,
                                                       EGLint value);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLQueryStreamKHR(EGLDisplay dpy,
                                                      EGLStreamKHR stream,
                                                      EGLenum attribute,
                                                      EGLint *value);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLQueryStreamu64KHR(EGLDisplay dpy,
                                                         EGLStreamKHR stream,
                                                         EGLenum attribute,
                                                         EGLuint64KHR *value);

// EGL_KHR_stream_consumer_gltexture
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLStreamConsumerGLTextureExternalKHR(EGLDisplay dpy,
                                                                          EGLStreamKHR stream);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLStreamConsumerAcquireKHR(EGLDisplay dpy,
                                                                EGLStreamKHR stream);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLStreamConsumerReleaseKHR(EGLDisplay dpy,
                                                                EGLStreamKHR stream);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY
EGLStreamConsumerGLTextureExternalAttribsNV(EGLDisplay dpy,
                                            EGLStreamKHR stream,
                                            const EGLAttrib *attrib_list);

// EGL_ANGLE_stream_producer_d3d_texture
ANGLE_EXPORT EGLBoolean EGLAPIENTRY
EGLCreateStreamProducerD3DTextureANGLE(EGLDisplay dpy,
                                       EGLStreamKHR stream,
                                       const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLStreamPostD3DTextureANGLE(EGLDisplay dpy,
                                                                 EGLStreamKHR stream,
                                                                 void *texture,
                                                                 const EGLAttrib *attrib_list);

// EGL_CHROMIUM_get_sync_values
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLGetSyncValuesCHROMIUM(EGLDisplay dpy,
                                                             EGLSurface surface,
                                                             EGLuint64KHR *ust,
                                                             EGLuint64KHR *msc,
                                                             EGLuint64KHR *sbc);

// EGL_KHR_swap_buffers_with_damage
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLSwapBuffersWithDamageKHR(EGLDisplay dpy,
                                                                EGLSurface surface,
                                                                EGLint *rects,
                                                                EGLint n_rects);

// EGL_ANDROID_presentation_time
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLPresentationTimeANDROID(EGLDisplay dpy,
                                                               EGLSurface surface,
                                                               EGLnsecsANDROID time);

// EGL_ANDRIOD_blob_cache
ANGLE_EXPORT void EGLAPIENTRY EGLSetBlobCacheFuncsANDROID(EGLDisplay dpy,
                                                          EGLSetBlobFuncANDROID set,
                                                          EGLGetBlobFuncANDROID get);

// EGL_ANGLE_program_cache_control
ANGLE_EXPORT EGLint EGLAPIENTRY EGLProgramCacheGetAttribANGLE(EGLDisplay dpy, EGLenum attrib);
ANGLE_EXPORT void EGLAPIENTRY EGLProgramCacheQueryANGLE(EGLDisplay dpy,
                                                        EGLint index,
                                                        void *key,
                                                        EGLint *keysize,
                                                        void *binary,
                                                        EGLint *binarysize);
ANGLE_EXPORT void EGLAPIENTRY EGLProgramCachePopulateANGLE(EGLDisplay dpy,
                                                           const void *key,
                                                           EGLint keysize,
                                                           const void *binary,
                                                           EGLint binarysize);
ANGLE_EXPORT EGLint EGLAPIENTRY EGLProgramCacheResizeANGLE(EGLDisplay dpy,
                                                           EGLint limit,
                                                           EGLenum mode);

// EGL_KHR_debug
ANGLE_EXPORT EGLint EGLAPIENTRY EGLDebugMessageControlKHR(EGLDEBUGPROCKHR callback,
                                                          const EGLAttrib *attrib_list);

ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLQueryDebugKHR(EGLint attribute, EGLAttrib *value);

ANGLE_EXPORT EGLint EGLAPIENTRY EGLLabelObjectKHR(EGLDisplay display,
                                                  EGLenum objectType,
                                                  EGLObjectKHR object,
                                                  EGLLabelKHR label);

// ANDROID_get_frame_timestamps
ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLGetCompositorTimingSupportedANDROID(EGLDisplay dpy,
                                                                           EGLSurface surface,
                                                                           EGLint name);

ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLGetCompositorTimingANDROID(EGLDisplay dpy,
                                                                  EGLSurface surface,
                                                                  EGLint numTimestamps,
                                                                  const EGLint *names,
                                                                  EGLnsecsANDROID *values);

ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLGetNextFrameIdANDROID(EGLDisplay dpy,
                                                             EGLSurface surface,
                                                             EGLuint64KHR *frameId);

ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLGetFrameTimestampSupportedANDROID(EGLDisplay dpy,
                                                                         EGLSurface surface,
                                                                         EGLint timestamp);

ANGLE_EXPORT EGLBoolean EGLAPIENTRY EGLGetFrameTimestampsANDROID(EGLDisplay dpy,
                                                                 EGLSurface surface,
                                                                 EGLuint64KHR frameId,
                                                                 EGLint numTimestamps,
                                                                 const EGLint *timestamps,
                                                                 EGLnsecsANDROID *values);

}  // extern "C"

#endif  // LIBGLESV2_ENTRYPOINTSEGLEXT_H_
