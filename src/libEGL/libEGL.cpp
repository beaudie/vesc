//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// libEGL.cpp: Implements the exported EGL functions.

#include "libEGL/egl_loader_autogen.h"

#include "common/debug.h"
#include "common/system_utils.h"

#include <memory>

namespace
{
bool gLoaded = false;
std::unique_ptr<angle::Library> gEntryPointsLib;

angle::GenericProc KHRONOS_APIENTRY GlobalLoad(const char *symbol)
{
    return reinterpret_cast<angle::GenericProc>(gEntryPointsLib->getSymbol(symbol));
}

void EnsureEGLLoaded()
{
    if (gLoaded)
        return;

    gEntryPointsLib.reset(angle::OpenSharedLibrary(ANGLE_GLESV2_LIBRARY_NAME));
    ASSERT(gEntryPointsLib);
    angle::LoadEGL(GlobalLoad);
    ASSERT(EGLGetPlatformDisplay);
    gLoaded = true;
}
}  // anonymous namespace

extern "C" {

EGLBoolean EGLAPIENTRY eglChooseConfig(EGLDisplay dpy,
                                       const EGLint *attrib_list,
                                       EGLConfig *configs,
                                       EGLint config_size,
                                       EGLint *num_config)
{
    EnsureEGLLoaded();
    return EGLChooseConfig(dpy, attrib_list, configs, config_size, num_config);
}

EGLBoolean EGLAPIENTRY eglCopyBuffers(EGLDisplay dpy,
                                      EGLSurface surface,
                                      EGLNativePixmapType target)
{
    EnsureEGLLoaded();
    return EGLCopyBuffers(dpy, surface, target);
}

EGLContext EGLAPIENTRY eglCreateContext(EGLDisplay dpy,
                                        EGLConfig config,
                                        EGLContext share_context,
                                        const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return EGLCreateContext(dpy, config, share_context, attrib_list);
}

EGLSurface EGLAPIENTRY eglCreatePbufferSurface(EGLDisplay dpy,
                                               EGLConfig config,
                                               const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return EGLCreatePbufferSurface(dpy, config, attrib_list);
}

EGLSurface EGLAPIENTRY eglCreatePixmapSurface(EGLDisplay dpy,
                                              EGLConfig config,
                                              EGLNativePixmapType pixmap,
                                              const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return EGLCreatePixmapSurface(dpy, config, pixmap, attrib_list);
}

EGLSurface EGLAPIENTRY eglCreateWindowSurface(EGLDisplay dpy,
                                              EGLConfig config,
                                              EGLNativeWindowType win,
                                              const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return EGLCreateWindowSurface(dpy, config, win, attrib_list);
}

EGLBoolean EGLAPIENTRY eglDestroyContext(EGLDisplay dpy, EGLContext ctx)
{
    EnsureEGLLoaded();
    return EGLDestroyContext(dpy, ctx);
}

EGLBoolean EGLAPIENTRY eglDestroySurface(EGLDisplay dpy, EGLSurface surface)
{
    EnsureEGLLoaded();
    return EGLDestroySurface(dpy, surface);
}

EGLBoolean EGLAPIENTRY eglGetConfigAttrib(EGLDisplay dpy,
                                          EGLConfig config,
                                          EGLint attribute,
                                          EGLint *value)
{
    EnsureEGLLoaded();
    return EGLGetConfigAttrib(dpy, config, attribute, value);
}

EGLBoolean EGLAPIENTRY eglGetConfigs(EGLDisplay dpy,
                                     EGLConfig *configs,
                                     EGLint config_size,
                                     EGLint *num_config)
{
    EnsureEGLLoaded();
    return EGLGetConfigs(dpy, configs, config_size, num_config);
}

EGLDisplay EGLAPIENTRY eglGetCurrentDisplay(void)
{
    EnsureEGLLoaded();
    return EGLGetCurrentDisplay();
}

EGLSurface EGLAPIENTRY eglGetCurrentSurface(EGLint readdraw)
{
    EnsureEGLLoaded();
    return EGLGetCurrentSurface(readdraw);
}

EGLDisplay EGLAPIENTRY eglGetDisplay(EGLNativeDisplayType display_id)
{
    EnsureEGLLoaded();
    return EGLGetDisplay(display_id);
}

EGLint EGLAPIENTRY eglGetError(void)
{
    EnsureEGLLoaded();
    return EGLGetError();
}

EGLBoolean EGLAPIENTRY eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor)
{
    EnsureEGLLoaded();
    return EGLInitialize(dpy, major, minor);
}

EGLBoolean EGLAPIENTRY eglMakeCurrent(EGLDisplay dpy,
                                      EGLSurface draw,
                                      EGLSurface read,
                                      EGLContext ctx)
{
    EnsureEGLLoaded();
    return EGLMakeCurrent(dpy, draw, read, ctx);
}

EGLBoolean EGLAPIENTRY eglQueryContext(EGLDisplay dpy,
                                       EGLContext ctx,
                                       EGLint attribute,
                                       EGLint *value)
{
    EnsureEGLLoaded();
    return EGLQueryContext(dpy, ctx, attribute, value);
}

const char *EGLAPIENTRY eglQueryString(EGLDisplay dpy, EGLint name)
{
    EnsureEGLLoaded();
    return EGLQueryString(dpy, name);
}

EGLBoolean EGLAPIENTRY eglQuerySurface(EGLDisplay dpy,
                                       EGLSurface surface,
                                       EGLint attribute,
                                       EGLint *value)
{
    EnsureEGLLoaded();
    return EGLQuerySurface(dpy, surface, attribute, value);
}

EGLBoolean EGLAPIENTRY eglSwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
    EnsureEGLLoaded();
    return EGLSwapBuffers(dpy, surface);
}

EGLBoolean EGLAPIENTRY eglTerminate(EGLDisplay dpy)
{
    EnsureEGLLoaded();
    return EGLTerminate(dpy);
}

EGLBoolean EGLAPIENTRY eglWaitGL(void)
{
    EnsureEGLLoaded();
    return EGLWaitGL();
}

EGLBoolean EGLAPIENTRY eglWaitNative(EGLint engine)
{
    EnsureEGLLoaded();
    return EGLWaitNative(engine);
}

EGLBoolean EGLAPIENTRY eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    EnsureEGLLoaded();
    return EGLBindTexImage(dpy, surface, buffer);
}

EGLBoolean EGLAPIENTRY eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    EnsureEGLLoaded();
    return EGLReleaseTexImage(dpy, surface, buffer);
}

EGLBoolean EGLAPIENTRY eglSurfaceAttrib(EGLDisplay dpy,
                                        EGLSurface surface,
                                        EGLint attribute,
                                        EGLint value)
{
    EnsureEGLLoaded();
    return EGLSurfaceAttrib(dpy, surface, attribute, value);
}

EGLBoolean EGLAPIENTRY eglSwapInterval(EGLDisplay dpy, EGLint interval)
{
    EnsureEGLLoaded();
    return EGLSwapInterval(dpy, interval);
}

EGLBoolean EGLAPIENTRY eglBindAPI(EGLenum api)
{
    EnsureEGLLoaded();
    return EGLBindAPI(api);
}

EGLenum EGLAPIENTRY eglQueryAPI(void)
{
    EnsureEGLLoaded();
    return EGLQueryAPI();
}

EGLSurface EGLAPIENTRY eglCreatePbufferFromClientBuffer(EGLDisplay dpy,
                                                        EGLenum buftype,
                                                        EGLClientBuffer buffer,
                                                        EGLConfig config,
                                                        const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return EGLCreatePbufferFromClientBuffer(dpy, buftype, buffer, config, attrib_list);
}

EGLBoolean EGLAPIENTRY eglReleaseThread(void)
{
    EnsureEGLLoaded();
    return EGLReleaseThread();
}

EGLBoolean EGLAPIENTRY eglWaitClient(void)
{
    EnsureEGLLoaded();
    return EGLWaitClient();
}

EGLContext EGLAPIENTRY eglGetCurrentContext(void)
{
    EnsureEGLLoaded();
    return EGLGetCurrentContext();
}

EGLSync EGLAPIENTRY eglCreateSync(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return EGLCreateSync(dpy, type, attrib_list);
}

EGLBoolean EGLAPIENTRY eglDestroySync(EGLDisplay dpy, EGLSync sync)
{
    EnsureEGLLoaded();
    return EGLDestroySync(dpy, sync);
}

EGLint EGLAPIENTRY eglClientWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTime timeout)
{
    EnsureEGLLoaded();
    return EGLClientWaitSync(dpy, sync, flags, timeout);
}

EGLBoolean EGLAPIENTRY eglGetSyncAttrib(EGLDisplay dpy,
                                        EGLSync sync,
                                        EGLint attribute,
                                        EGLAttrib *value)
{
    EnsureEGLLoaded();
    return EGLGetSyncAttrib(dpy, sync, attribute, value);
}

EGLImage EGLAPIENTRY eglCreateImage(EGLDisplay dpy,
                                    EGLContext ctx,
                                    EGLenum target,
                                    EGLClientBuffer buffer,
                                    const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return EGLCreateImage(dpy, ctx, target, buffer, attrib_list);
}

EGLBoolean EGLAPIENTRY eglDestroyImage(EGLDisplay dpy, EGLImage image)
{
    EnsureEGLLoaded();
    return EGLDestroyImage(dpy, image);
}

EGLDisplay EGLAPIENTRY eglGetPlatformDisplay(EGLenum platform,
                                             void *native_display,
                                             const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return EGLGetPlatformDisplay(platform, native_display, attrib_list);
}

EGLSurface EGLAPIENTRY eglCreatePlatformWindowSurface(EGLDisplay dpy,
                                                      EGLConfig config,
                                                      void *native_window,
                                                      const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return EGLCreatePlatformWindowSurface(dpy, config, native_window, attrib_list);
}

EGLSurface EGLAPIENTRY eglCreatePlatformPixmapSurface(EGLDisplay dpy,
                                                      EGLConfig config,
                                                      void *native_pixmap,
                                                      const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return EGLCreatePlatformPixmapSurface(dpy, config, native_pixmap, attrib_list);
}

EGLBoolean EGLAPIENTRY eglWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags)
{
    EnsureEGLLoaded();
    return EGLWaitSync(dpy, sync, flags);
}

EGLBoolean EGLAPIENTRY eglQuerySurfacePointerANGLE(EGLDisplay dpy,
                                                   EGLSurface surface,
                                                   EGLint attribute,
                                                   void **value)
{
    EnsureEGLLoaded();
    return EGLQuerySurfacePointerANGLE(dpy, surface, attribute, value);
}

EGLBoolean EGLAPIENTRY eglPostSubBufferNV(EGLDisplay dpy,
                                          EGLSurface surface,
                                          EGLint x,
                                          EGLint y,
                                          EGLint width,
                                          EGLint height)
{
    EnsureEGLLoaded();
    return EGLPostSubBufferNV(dpy, surface, x, y, width, height);
}

EGLDisplay EGLAPIENTRY eglGetPlatformDisplayEXT(EGLenum platform,
                                                void *native_display,
                                                const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return EGLGetPlatformDisplayEXT(platform, native_display, attrib_list);
}

EGLSurface EGLAPIENTRY eglCreatePlatformWindowSurfaceEXT(EGLDisplay dpy,
                                                         EGLConfig config,
                                                         void *native_window,
                                                         const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return EGLCreatePlatformWindowSurfaceEXT(dpy, config, native_window, attrib_list);
}

EGLSurface EGLAPIENTRY eglCreatePlatformPixmapSurfaceEXT(EGLDisplay dpy,
                                                         EGLConfig config,
                                                         void *native_pixmap,
                                                         const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return EGLCreatePlatformPixmapSurfaceEXT(dpy, config, native_pixmap, attrib_list);
}

EGLBoolean EGLAPIENTRY eglQueryDisplayAttribEXT(EGLDisplay dpy, EGLint attribute, EGLAttrib *value)
{
    EnsureEGLLoaded();
    return EGLQueryDisplayAttribEXT(dpy, attribute, value);
}

EGLBoolean EGLAPIENTRY eglQueryDeviceAttribEXT(EGLDeviceEXT device,
                                               EGLint attribute,
                                               EGLAttrib *value)
{
    EnsureEGLLoaded();
    return EGLQueryDeviceAttribEXT(device, attribute, value);
}

const char *EGLAPIENTRY eglQueryDeviceStringEXT(EGLDeviceEXT device, EGLint name)
{
    EnsureEGLLoaded();
    return EGLQueryDeviceStringEXT(device, name);
}

EGLImageKHR EGLAPIENTRY eglCreateImageKHR(EGLDisplay dpy,
                                          EGLContext ctx,
                                          EGLenum target,
                                          EGLClientBuffer buffer,
                                          const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return EGLCreateImageKHR(dpy, ctx, target, buffer, attrib_list);
}

EGLBoolean EGLAPIENTRY eglDestroyImageKHR(EGLDisplay dpy, EGLImageKHR image)
{
    EnsureEGLLoaded();
    return EGLDestroyImageKHR(dpy, image);
}

EGLDeviceEXT EGLAPIENTRY eglCreateDeviceANGLE(EGLint device_type,
                                              void *native_device,
                                              const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return EGLCreateDeviceANGLE(device_type, native_device, attrib_list);
}

EGLBoolean EGLAPIENTRY eglReleaseDeviceANGLE(EGLDeviceEXT device)
{
    EnsureEGLLoaded();
    return EGLReleaseDeviceANGLE(device);
}

__eglMustCastToProperFunctionPointerType EGLAPIENTRY eglGetProcAddress(const char *procname)
{
    EnsureEGLLoaded();
    return EGLGetProcAddress(procname);
}

EGLStreamKHR EGLAPIENTRY eglCreateStreamKHR(EGLDisplay dpy, const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return EGLCreateStreamKHR(dpy, attrib_list);
}

EGLBoolean EGLAPIENTRY eglDestroyStreamKHR(EGLDisplay dpy, EGLStreamKHR stream)
{
    EnsureEGLLoaded();
    return EGLDestroyStreamKHR(dpy, stream);
}

EGLBoolean EGLAPIENTRY eglStreamAttribKHR(EGLDisplay dpy,
                                          EGLStreamKHR stream,
                                          EGLenum attribute,
                                          EGLint value)
{
    EnsureEGLLoaded();
    return EGLStreamAttribKHR(dpy, stream, attribute, value);
}

EGLBoolean EGLAPIENTRY eglQueryStreamKHR(EGLDisplay dpy,
                                         EGLStreamKHR stream,
                                         EGLenum attribute,
                                         EGLint *value)
{
    EnsureEGLLoaded();
    return EGLQueryStreamKHR(dpy, stream, attribute, value);
}

EGLBoolean EGLAPIENTRY eglQueryStreamu64KHR(EGLDisplay dpy,
                                            EGLStreamKHR stream,
                                            EGLenum attribute,
                                            EGLuint64KHR *value)
{
    EnsureEGLLoaded();
    return EGLQueryStreamu64KHR(dpy, stream, attribute, value);
}

EGLBoolean EGLAPIENTRY eglStreamConsumerGLTextureExternalKHR(EGLDisplay dpy, EGLStreamKHR stream)
{
    EnsureEGLLoaded();
    return EGLStreamConsumerGLTextureExternalKHR(dpy, stream);
}

EGLBoolean EGLAPIENTRY eglStreamConsumerAcquireKHR(EGLDisplay dpy, EGLStreamKHR stream)
{
    EnsureEGLLoaded();
    return EGLStreamConsumerAcquireKHR(dpy, stream);
}

EGLBoolean EGLAPIENTRY eglStreamConsumerReleaseKHR(EGLDisplay dpy, EGLStreamKHR stream)
{
    EnsureEGLLoaded();
    return EGLStreamConsumerReleaseKHR(dpy, stream);
}

EGLBoolean EGLAPIENTRY eglStreamConsumerGLTextureExternalAttribsNV(EGLDisplay dpy,
                                                                   EGLStreamKHR stream,
                                                                   const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return EGLStreamConsumerGLTextureExternalAttribsNV(dpy, stream, attrib_list);
}

EGLBoolean EGLAPIENTRY eglCreateStreamProducerD3DTextureANGLE(EGLDisplay dpy,
                                                              EGLStreamKHR stream,
                                                              const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return EGLCreateStreamProducerD3DTextureANGLE(dpy, stream, attrib_list);
}

EGLBoolean EGLAPIENTRY eglStreamPostD3DTextureANGLE(EGLDisplay dpy,
                                                    EGLStreamKHR stream,
                                                    void *texture,
                                                    const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return EGLStreamPostD3DTextureANGLE(dpy, stream, texture, attrib_list);
}

EGLBoolean EGLAPIENTRY eglGetSyncValuesCHROMIUM(EGLDisplay dpy,
                                                EGLSurface surface,
                                                EGLuint64KHR *ust,
                                                EGLuint64KHR *msc,
                                                EGLuint64KHR *sbc)
{
    EnsureEGLLoaded();
    return EGLGetSyncValuesCHROMIUM(dpy, surface, ust, msc, sbc);
}

EGLBoolean EGLAPIENTRY eglSwapBuffersWithDamageKHR(EGLDisplay dpy,
                                                   EGLSurface surface,
                                                   EGLint *rects,
                                                   EGLint n_rects)
{
    EnsureEGLLoaded();
    return EGLSwapBuffersWithDamageKHR(dpy, surface, rects, n_rects);
}

EGLBoolean EGLAPIENTRY eglPresentationTimeANDROID(EGLDisplay dpy,
                                                  EGLSurface surface,
                                                  EGLnsecsANDROID time)
{
    EnsureEGLLoaded();
    return EGLPresentationTimeANDROID(dpy, surface, time);
}

EGLint EGLAPIENTRY eglProgramCacheGetAttribANGLE(EGLDisplay dpy, EGLenum attrib)
{
    EnsureEGLLoaded();
    return EGLProgramCacheGetAttribANGLE(dpy, attrib);
}

void EGLAPIENTRY eglProgramCacheQueryANGLE(EGLDisplay dpy,
                                           EGLint index,
                                           void *key,
                                           EGLint *keysize,
                                           void *binary,
                                           EGLint *binarysize)
{
    EGLProgramCacheQueryANGLE(dpy, index, key, keysize, binary, binarysize);
}

void EGLAPIENTRY eglProgramCachePopulateANGLE(EGLDisplay dpy,
                                              const void *key,
                                              EGLint keysize,
                                              const void *binary,
                                              EGLint binarysize)
{
    EGLProgramCachePopulateANGLE(dpy, key, keysize, binary, binarysize);
}

EGLint EGLAPIENTRY eglProgramCacheResizeANGLE(EGLDisplay dpy, EGLint limit, EGLenum mode)
{
    EnsureEGLLoaded();
    return EGLProgramCacheResizeANGLE(dpy, limit, mode);
}

EGLint EGLAPIENTRY eglDebugMessageControlKHR(EGLDEBUGPROCKHR callback, const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return EGLDebugMessageControlKHR(callback, attrib_list);
}

EGLBoolean EGLAPIENTRY eglQueryDebugKHR(EGLint attribute, EGLAttrib *value)
{
    EnsureEGLLoaded();
    return EGLQueryDebugKHR(attribute, value);
}

EGLint EGLAPIENTRY eglLabelObjectKHR(EGLDisplay dpy,
                                     EGLenum objectType,
                                     EGLObjectKHR object,
                                     EGLLabelKHR label)
{
    EnsureEGLLoaded();
    return EGLLabelObjectKHR(dpy, objectType, object, label);
}

void EGLAPIENTRY eglSetBlobCacheFuncsANDROID(EGLDisplay dpy,
                                             EGLSetBlobFuncANDROID set,
                                             EGLGetBlobFuncANDROID get)
{
    EnsureEGLLoaded();
    return EGLSetBlobCacheFuncsANDROID(dpy, set, get);
}

EGLBoolean EGLAPIENTRY eglGetCompositorTimingSupportedANDROID(EGLDisplay dpy,
                                                              EGLSurface surface,
                                                              EGLint name)
{
    EnsureEGLLoaded();
    return EGLGetCompositorTimingSupportedANDROID(dpy, surface, name);
}

EGLBoolean EGLAPIENTRY eglGetCompositorTimingANDROID(EGLDisplay dpy,
                                                     EGLSurface surface,
                                                     EGLint numTimestamps,
                                                     const EGLint *names,
                                                     EGLnsecsANDROID *values)
{
    EnsureEGLLoaded();
    return EGLGetCompositorTimingANDROID(dpy, surface, numTimestamps, names, values);
}

EGLBoolean EGLAPIENTRY eglGetNextFrameIdANDROID(EGLDisplay dpy,
                                                EGLSurface surface,
                                                EGLuint64KHR *frameId)
{
    EnsureEGLLoaded();
    return EGLGetNextFrameIdANDROID(dpy, surface, frameId);
}

EGLBoolean EGLAPIENTRY eglGetFrameTimestampSupportedANDROID(EGLDisplay dpy,
                                                            EGLSurface surface,
                                                            EGLint timestamp)
{
    EnsureEGLLoaded();
    return EGLGetFrameTimestampSupportedANDROID(dpy, surface, timestamp);
}

EGLBoolean EGLAPIENTRY eglGetFrameTimestampsANDROID(EGLDisplay dpy,
                                                    EGLSurface surface,
                                                    EGLuint64KHR frameId,
                                                    EGLint numTimestamps,
                                                    const EGLint *timestamps,
                                                    EGLnsecsANDROID *values)
{
    EnsureEGLLoaded();
    return EGLGetFrameTimestampsANDROID(dpy, surface, frameId, numTimestamps, timestamps, values);
}
}  // extern "C"
