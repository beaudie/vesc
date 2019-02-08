//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// libEGL.cpp: Implements the exported EGL functions.

#include "common/system_utils.h"

#include <memory>

#if !defined(ANGLE_USE_EGL_LOADER)
#    error Must be used with the EGL loader.
#endif  // defined(ANGLE_USE_EGL_LOADER)

#include "libEGL/egl_loader_autogen.h"

namespace
{
#if defined(ANGLE_USE_EGL_LOADER)
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
    angle::LoadEGL(GlobalLoad);
    if (!_eglGetPlatformDisplay)
    {
        fprintf(stderr, "Error loading EGL entry points.\n");
    }
    else
    {
        gLoaded = true;
    }
}
#else
void EnsureEGLLoaded() {}
#endif  // defined(ANGLE_USE_EGL_LOADER)
}  // anonymous namespace

extern "C" {

EGLBoolean EGLAPIENTRY eglChooseConfig(EGLDisplay dpy,
                                       const EGLint *attrib_list,
                                       EGLConfig *configs,
                                       EGLint config_size,
                                       EGLint *num_config)
{
    EnsureEGLLoaded();
    return _eglChooseConfig(dpy, attrib_list, configs, config_size, num_config);
}

EGLBoolean EGLAPIENTRY eglCopyBuffers(EGLDisplay dpy,
                                      EGLSurface surface,
                                      EGLNativePixmapType target)
{
    EnsureEGLLoaded();
    return _eglCopyBuffers(dpy, surface, target);
}

EGLContext EGLAPIENTRY eglCreateContext(EGLDisplay dpy,
                                        EGLConfig config,
                                        EGLContext share_context,
                                        const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return _eglCreateContext(dpy, config, share_context, attrib_list);
}

EGLSurface EGLAPIENTRY eglCreatePbufferSurface(EGLDisplay dpy,
                                               EGLConfig config,
                                               const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return _eglCreatePbufferSurface(dpy, config, attrib_list);
}

EGLSurface EGLAPIENTRY eglCreatePixmapSurface(EGLDisplay dpy,
                                              EGLConfig config,
                                              EGLNativePixmapType pixmap,
                                              const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return _eglCreatePixmapSurface(dpy, config, pixmap, attrib_list);
}

EGLSurface EGLAPIENTRY eglCreateWindowSurface(EGLDisplay dpy,
                                              EGLConfig config,
                                              EGLNativeWindowType win,
                                              const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return _eglCreateWindowSurface(dpy, config, win, attrib_list);
}

EGLBoolean EGLAPIENTRY eglDestroyContext(EGLDisplay dpy, EGLContext ctx)
{
    EnsureEGLLoaded();
    return _eglDestroyContext(dpy, ctx);
}

EGLBoolean EGLAPIENTRY eglDestroySurface(EGLDisplay dpy, EGLSurface surface)
{
    EnsureEGLLoaded();
    return _eglDestroySurface(dpy, surface);
}

EGLBoolean EGLAPIENTRY eglGetConfigAttrib(EGLDisplay dpy,
                                          EGLConfig config,
                                          EGLint attribute,
                                          EGLint *value)
{
    EnsureEGLLoaded();
    return _eglGetConfigAttrib(dpy, config, attribute, value);
}

EGLBoolean EGLAPIENTRY eglGetConfigs(EGLDisplay dpy,
                                     EGLConfig *configs,
                                     EGLint config_size,
                                     EGLint *num_config)
{
    EnsureEGLLoaded();
    return _eglGetConfigs(dpy, configs, config_size, num_config);
}

EGLDisplay EGLAPIENTRY eglGetCurrentDisplay(void)
{
    EnsureEGLLoaded();
    return _eglGetCurrentDisplay();
}

EGLSurface EGLAPIENTRY eglGetCurrentSurface(EGLint readdraw)
{
    EnsureEGLLoaded();
    return _eglGetCurrentSurface(readdraw);
}

EGLDisplay EGLAPIENTRY eglGetDisplay(EGLNativeDisplayType display_id)
{
    EnsureEGLLoaded();
    return _eglGetDisplay(display_id);
}

EGLint EGLAPIENTRY eglGetError(void)
{
    EnsureEGLLoaded();
    return _eglGetError();
}

EGLBoolean EGLAPIENTRY eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor)
{
    EnsureEGLLoaded();
    return _eglInitialize(dpy, major, minor);
}

EGLBoolean EGLAPIENTRY eglMakeCurrent(EGLDisplay dpy,
                                      EGLSurface draw,
                                      EGLSurface read,
                                      EGLContext ctx)
{
    EnsureEGLLoaded();
    return _eglMakeCurrent(dpy, draw, read, ctx);
}

EGLBoolean EGLAPIENTRY eglQueryContext(EGLDisplay dpy,
                                       EGLContext ctx,
                                       EGLint attribute,
                                       EGLint *value)
{
    EnsureEGLLoaded();
    return _eglQueryContext(dpy, ctx, attribute, value);
}

const char *EGLAPIENTRY eglQueryString(EGLDisplay dpy, EGLint name)
{
    EnsureEGLLoaded();
    return _eglQueryString(dpy, name);
}

EGLBoolean EGLAPIENTRY eglQuerySurface(EGLDisplay dpy,
                                       EGLSurface surface,
                                       EGLint attribute,
                                       EGLint *value)
{
    EnsureEGLLoaded();
    return _eglQuerySurface(dpy, surface, attribute, value);
}

EGLBoolean EGLAPIENTRY eglSwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
    EnsureEGLLoaded();
    return _eglSwapBuffers(dpy, surface);
}

EGLBoolean EGLAPIENTRY eglTerminate(EGLDisplay dpy)
{
    EnsureEGLLoaded();
    return _eglTerminate(dpy);
}

EGLBoolean EGLAPIENTRY eglWaitGL(void)
{
    EnsureEGLLoaded();
    return _eglWaitGL();
}

EGLBoolean EGLAPIENTRY eglWaitNative(EGLint engine)
{
    EnsureEGLLoaded();
    return _eglWaitNative(engine);
}

EGLBoolean EGLAPIENTRY eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    EnsureEGLLoaded();
    return _eglBindTexImage(dpy, surface, buffer);
}

EGLBoolean EGLAPIENTRY eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    EnsureEGLLoaded();
    return _eglReleaseTexImage(dpy, surface, buffer);
}

EGLBoolean EGLAPIENTRY eglSurfaceAttrib(EGLDisplay dpy,
                                        EGLSurface surface,
                                        EGLint attribute,
                                        EGLint value)
{
    EnsureEGLLoaded();
    return _eglSurfaceAttrib(dpy, surface, attribute, value);
}

EGLBoolean EGLAPIENTRY eglSwapInterval(EGLDisplay dpy, EGLint interval)
{
    EnsureEGLLoaded();
    return _eglSwapInterval(dpy, interval);
}

EGLBoolean EGLAPIENTRY eglBindAPI(EGLenum api)
{
    EnsureEGLLoaded();
    return _eglBindAPI(api);
}

EGLenum EGLAPIENTRY eglQueryAPI(void)
{
    EnsureEGLLoaded();
    return _eglQueryAPI();
}

EGLSurface EGLAPIENTRY eglCreatePbufferFromClientBuffer(EGLDisplay dpy,
                                                        EGLenum buftype,
                                                        EGLClientBuffer buffer,
                                                        EGLConfig config,
                                                        const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return _eglCreatePbufferFromClientBuffer(dpy, buftype, buffer, config, attrib_list);
}

EGLBoolean EGLAPIENTRY eglReleaseThread(void)
{
    EnsureEGLLoaded();
    return _eglReleaseThread();
}

EGLBoolean EGLAPIENTRY eglWaitClient(void)
{
    EnsureEGLLoaded();
    return _eglWaitClient();
}

EGLContext EGLAPIENTRY eglGetCurrentContext(void)
{
    EnsureEGLLoaded();
    return _eglGetCurrentContext();
}

EGLSync EGLAPIENTRY eglCreateSync(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return _eglCreateSync(dpy, type, attrib_list);
}

EGLBoolean EGLAPIENTRY eglDestroySync(EGLDisplay dpy, EGLSync sync)
{
    EnsureEGLLoaded();
    return _eglDestroySync(dpy, sync);
}

EGLint EGLAPIENTRY eglClientWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTime timeout)
{
    EnsureEGLLoaded();
    return _eglClientWaitSync(dpy, sync, flags, timeout);
}

EGLBoolean EGLAPIENTRY eglGetSyncAttrib(EGLDisplay dpy,
                                        EGLSync sync,
                                        EGLint attribute,
                                        EGLAttrib *value)
{
    EnsureEGLLoaded();
    return _eglGetSyncAttrib(dpy, sync, attribute, value);
}

EGLImage EGLAPIENTRY eglCreateImage(EGLDisplay dpy,
                                    EGLContext ctx,
                                    EGLenum target,
                                    EGLClientBuffer buffer,
                                    const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return _eglCreateImage(dpy, ctx, target, buffer, attrib_list);
}

EGLBoolean EGLAPIENTRY eglDestroyImage(EGLDisplay dpy, EGLImage image)
{
    EnsureEGLLoaded();
    return _eglDestroyImage(dpy, image);
}

EGLDisplay EGLAPIENTRY eglGetPlatformDisplay(EGLenum platform,
                                             void *native_display,
                                             const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return _eglGetPlatformDisplay(platform, native_display, attrib_list);
}

EGLSurface EGLAPIENTRY eglCreatePlatformWindowSurface(EGLDisplay dpy,
                                                      EGLConfig config,
                                                      void *native_window,
                                                      const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return _eglCreatePlatformWindowSurface(dpy, config, native_window, attrib_list);
}

EGLSurface EGLAPIENTRY eglCreatePlatformPixmapSurface(EGLDisplay dpy,
                                                      EGLConfig config,
                                                      void *native_pixmap,
                                                      const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return _eglCreatePlatformPixmapSurface(dpy, config, native_pixmap, attrib_list);
}

EGLBoolean EGLAPIENTRY eglWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags)
{
    EnsureEGLLoaded();
    return _eglWaitSync(dpy, sync, flags);
}

EGLBoolean EGLAPIENTRY eglQuerySurfacePointerANGLE(EGLDisplay dpy,
                                                   EGLSurface surface,
                                                   EGLint attribute,
                                                   void **value)
{
    EnsureEGLLoaded();
    return _eglQuerySurfacePointerANGLE(dpy, surface, attribute, value);
}

EGLBoolean EGLAPIENTRY eglPostSubBufferNV(EGLDisplay dpy,
                                          EGLSurface surface,
                                          EGLint x,
                                          EGLint y,
                                          EGLint width,
                                          EGLint height)
{
    EnsureEGLLoaded();
    return _eglPostSubBufferNV(dpy, surface, x, y, width, height);
}

EGLDisplay EGLAPIENTRY eglGetPlatformDisplayEXT(EGLenum platform,
                                                void *native_display,
                                                const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return _eglGetPlatformDisplayEXT(platform, native_display, attrib_list);
}

EGLSurface EGLAPIENTRY eglCreatePlatformWindowSurfaceEXT(EGLDisplay dpy,
                                                         EGLConfig config,
                                                         void *native_window,
                                                         const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return _eglCreatePlatformWindowSurfaceEXT(dpy, config, native_window, attrib_list);
}

EGLSurface EGLAPIENTRY eglCreatePlatformPixmapSurfaceEXT(EGLDisplay dpy,
                                                         EGLConfig config,
                                                         void *native_pixmap,
                                                         const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return _eglCreatePlatformPixmapSurfaceEXT(dpy, config, native_pixmap, attrib_list);
}

EGLBoolean EGLAPIENTRY eglQueryDisplayAttribEXT(EGLDisplay dpy, EGLint attribute, EGLAttrib *value)
{
    EnsureEGLLoaded();
    return _eglQueryDisplayAttribEXT(dpy, attribute, value);
}

EGLBoolean EGLAPIENTRY eglQueryDeviceAttribEXT(EGLDeviceEXT device,
                                               EGLint attribute,
                                               EGLAttrib *value)
{
    EnsureEGLLoaded();
    return _eglQueryDeviceAttribEXT(device, attribute, value);
}

const char *EGLAPIENTRY eglQueryDeviceStringEXT(EGLDeviceEXT device, EGLint name)
{
    EnsureEGLLoaded();
    return _eglQueryDeviceStringEXT(device, name);
}

EGLImageKHR EGLAPIENTRY eglCreateImageKHR(EGLDisplay dpy,
                                          EGLContext ctx,
                                          EGLenum target,
                                          EGLClientBuffer buffer,
                                          const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return _eglCreateImageKHR(dpy, ctx, target, buffer, attrib_list);
}

EGLBoolean EGLAPIENTRY eglDestroyImageKHR(EGLDisplay dpy, EGLImageKHR image)
{
    EnsureEGLLoaded();
    return _eglDestroyImageKHR(dpy, image);
}

EGLDeviceEXT EGLAPIENTRY eglCreateDeviceANGLE(EGLint device_type,
                                              void *native_device,
                                              const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return _eglCreateDeviceANGLE(device_type, native_device, attrib_list);
}

EGLBoolean EGLAPIENTRY eglReleaseDeviceANGLE(EGLDeviceEXT device)
{
    EnsureEGLLoaded();
    return _eglReleaseDeviceANGLE(device);
}

__eglMustCastToProperFunctionPointerType EGLAPIENTRY eglGetProcAddress(const char *procname)
{
    EnsureEGLLoaded();
    return _eglGetProcAddress(procname);
}

EGLStreamKHR EGLAPIENTRY eglCreateStreamKHR(EGLDisplay dpy, const EGLint *attrib_list)
{
    EnsureEGLLoaded();
    return _eglCreateStreamKHR(dpy, attrib_list);
}

EGLBoolean EGLAPIENTRY eglDestroyStreamKHR(EGLDisplay dpy, EGLStreamKHR stream)
{
    EnsureEGLLoaded();
    return _eglDestroyStreamKHR(dpy, stream);
}

EGLBoolean EGLAPIENTRY eglStreamAttribKHR(EGLDisplay dpy,
                                          EGLStreamKHR stream,
                                          EGLenum attribute,
                                          EGLint value)
{
    EnsureEGLLoaded();
    return _eglStreamAttribKHR(dpy, stream, attribute, value);
}

EGLBoolean EGLAPIENTRY eglQueryStreamKHR(EGLDisplay dpy,
                                         EGLStreamKHR stream,
                                         EGLenum attribute,
                                         EGLint *value)
{
    EnsureEGLLoaded();
    return _eglQueryStreamKHR(dpy, stream, attribute, value);
}

EGLBoolean EGLAPIENTRY eglQueryStreamu64KHR(EGLDisplay dpy,
                                            EGLStreamKHR stream,
                                            EGLenum attribute,
                                            EGLuint64KHR *value)
{
    EnsureEGLLoaded();
    return _eglQueryStreamu64KHR(dpy, stream, attribute, value);
}

EGLBoolean EGLAPIENTRY eglStreamConsumerGLTextureExternalKHR(EGLDisplay dpy, EGLStreamKHR stream)
{
    EnsureEGLLoaded();
    return _eglStreamConsumerGLTextureExternalKHR(dpy, stream);
}

EGLBoolean EGLAPIENTRY eglStreamConsumerAcquireKHR(EGLDisplay dpy, EGLStreamKHR stream)
{
    EnsureEGLLoaded();
    return _eglStreamConsumerAcquireKHR(dpy, stream);
}

EGLBoolean EGLAPIENTRY eglStreamConsumerReleaseKHR(EGLDisplay dpy, EGLStreamKHR stream)
{
    EnsureEGLLoaded();
    return _eglStreamConsumerReleaseKHR(dpy, stream);
}

EGLBoolean EGLAPIENTRY eglStreamConsumerGLTextureExternalAttribsNV(EGLDisplay dpy,
                                                                   EGLStreamKHR stream,
                                                                   const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return _eglStreamConsumerGLTextureExternalAttribsNV(dpy, stream, attrib_list);
}

EGLBoolean EGLAPIENTRY eglCreateStreamProducerD3DTextureANGLE(EGLDisplay dpy,
                                                              EGLStreamKHR stream,
                                                              const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return _eglCreateStreamProducerD3DTextureANGLE(dpy, stream, attrib_list);
}

EGLBoolean EGLAPIENTRY eglStreamPostD3DTextureANGLE(EGLDisplay dpy,
                                                    EGLStreamKHR stream,
                                                    void *texture,
                                                    const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return _eglStreamPostD3DTextureANGLE(dpy, stream, texture, attrib_list);
}

EGLBoolean EGLAPIENTRY eglGetSyncValuesCHROMIUM(EGLDisplay dpy,
                                                EGLSurface surface,
                                                EGLuint64KHR *ust,
                                                EGLuint64KHR *msc,
                                                EGLuint64KHR *sbc)
{
    EnsureEGLLoaded();
    return _eglGetSyncValuesCHROMIUM(dpy, surface, ust, msc, sbc);
}

EGLBoolean EGLAPIENTRY eglSwapBuffersWithDamageKHR(EGLDisplay dpy,
                                                   EGLSurface surface,
                                                   EGLint *rects,
                                                   EGLint n_rects)
{
    EnsureEGLLoaded();
    return _eglSwapBuffersWithDamageKHR(dpy, surface, rects, n_rects);
}

EGLBoolean EGLAPIENTRY eglPresentationTimeANDROID(EGLDisplay dpy,
                                                  EGLSurface surface,
                                                  EGLnsecsANDROID time)
{
    EnsureEGLLoaded();
    return _eglPresentationTimeANDROID(dpy, surface, time);
}

EGLint EGLAPIENTRY eglProgramCacheGetAttribANGLE(EGLDisplay dpy, EGLenum attrib)
{
    EnsureEGLLoaded();
    return _eglProgramCacheGetAttribANGLE(dpy, attrib);
}

void EGLAPIENTRY eglProgramCacheQueryANGLE(EGLDisplay dpy,
                                           EGLint index,
                                           void *key,
                                           EGLint *keysize,
                                           void *binary,
                                           EGLint *binarysize)
{
    _eglProgramCacheQueryANGLE(dpy, index, key, keysize, binary, binarysize);
}

void EGLAPIENTRY eglProgramCachePopulateANGLE(EGLDisplay dpy,
                                              const void *key,
                                              EGLint keysize,
                                              const void *binary,
                                              EGLint binarysize)
{
    _eglProgramCachePopulateANGLE(dpy, key, keysize, binary, binarysize);
}

EGLint EGLAPIENTRY eglProgramCacheResizeANGLE(EGLDisplay dpy, EGLint limit, EGLenum mode)
{
    EnsureEGLLoaded();
    return _eglProgramCacheResizeANGLE(dpy, limit, mode);
}

EGLint EGLAPIENTRY eglDebugMessageControlKHR(EGLDEBUGPROCKHR callback, const EGLAttrib *attrib_list)
{
    EnsureEGLLoaded();
    return _eglDebugMessageControlKHR(callback, attrib_list);
}

EGLBoolean EGLAPIENTRY eglQueryDebugKHR(EGLint attribute, EGLAttrib *value)
{
    EnsureEGLLoaded();
    return _eglQueryDebugKHR(attribute, value);
}

EGLint EGLAPIENTRY eglLabelObjectKHR(EGLDisplay dpy,
                                     EGLenum objectType,
                                     EGLObjectKHR object,
                                     EGLLabelKHR label)
{
    EnsureEGLLoaded();
    return _eglLabelObjectKHR(dpy, objectType, object, label);
}

void EGLAPIENTRY eglSetBlobCacheFuncsANDROID(EGLDisplay dpy,
                                             EGLSetBlobFuncANDROID set,
                                             EGLGetBlobFuncANDROID get)
{
    EnsureEGLLoaded();
    return _eglSetBlobCacheFuncsANDROID(dpy, set, get);
}

EGLBoolean EGLAPIENTRY eglGetCompositorTimingSupportedANDROID(EGLDisplay dpy,
                                                              EGLSurface surface,
                                                              EGLint name)
{
    EnsureEGLLoaded();
    return _eglGetCompositorTimingSupportedANDROID(dpy, surface, name);
}

EGLBoolean EGLAPIENTRY eglGetCompositorTimingANDROID(EGLDisplay dpy,
                                                     EGLSurface surface,
                                                     EGLint numTimestamps,
                                                     const EGLint *names,
                                                     EGLnsecsANDROID *values)
{
    EnsureEGLLoaded();
    return _eglGetCompositorTimingANDROID(dpy, surface, numTimestamps, names, values);
}

EGLBoolean EGLAPIENTRY eglGetNextFrameIdANDROID(EGLDisplay dpy,
                                                EGLSurface surface,
                                                EGLuint64KHR *frameId)
{
    EnsureEGLLoaded();
    return _eglGetNextFrameIdANDROID(dpy, surface, frameId);
}

EGLBoolean EGLAPIENTRY eglGetFrameTimestampSupportedANDROID(EGLDisplay dpy,
                                                            EGLSurface surface,
                                                            EGLint timestamp)
{
    EnsureEGLLoaded();
    return _eglGetFrameTimestampSupportedANDROID(dpy, surface, timestamp);
}

EGLBoolean EGLAPIENTRY eglGetFrameTimestampsANDROID(EGLDisplay dpy,
                                                    EGLSurface surface,
                                                    EGLuint64KHR frameId,
                                                    EGLint numTimestamps,
                                                    const EGLint *timestamps,
                                                    EGLnsecsANDROID *values)
{
    EnsureEGLLoaded();
    return _eglGetFrameTimestampsANDROID(dpy, surface, frameId, numTimestamps, timestamps, values);
}
}  // extern "C"
