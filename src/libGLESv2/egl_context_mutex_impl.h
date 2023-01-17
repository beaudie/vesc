#ifndef LIBGLESV2_EGL_CONTEXT_MUTEX_IMPL_H_
#define LIBGLESV2_EGL_CONTEXT_MUTEX_IMPL_H_

#include "libGLESv2/egl_context_mutex_autogen.h"

#include "libANGLE/validationEGL.h"

namespace egl
{
// Warning: Called before the Validation. Need to use GetContextIfValid() and TryGetContextMutex().

ContextMutex *GetContextMutex_ChooseConfig(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_CopyBuffers(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_CreateContext(Thread *thread,
                                            egl::Display *dpyPacked,
                                            gl::ContextID share_contextPacked)
{
    return TryGetActiveSharedContextMutex(GetContextIfValid(dpyPacked, share_contextPacked));
}

ContextMutex *GetContextMutex_CreatePbufferSurface(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_CreatePixmapSurface(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_CreateWindowSurface(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_DestroyContext(Thread *thread,
                                             egl::Display *dpyPacked,
                                             gl::ContextID ctxPacked)
{
    // Explicit lock in egl::Display::destroyContext()/makeCurrent()
    return nullptr;
}

ContextMutex *GetContextMutex_DestroySurface(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_GetConfigAttrib(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_GetConfigs(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_GetCurrentDisplay(Thread *thread)
{
    return nullptr;
}

ContextMutex *GetContextMutex_GetCurrentSurface(Thread *thread)
{
    return nullptr;
}

ContextMutex *GetContextMutex_GetDisplay(Thread *thread)
{
    return nullptr;
}

ContextMutex *GetContextMutex_GetError(Thread *thread)
{
    return nullptr;
}

ContextMutex *GetContextMutex_GetProcAddress(Thread *thread)
{
    return nullptr;
}

ContextMutex *GetContextMutex_Initialize(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_MakeCurrent(Thread *thread,
                                          egl::Display *dpyPacked,
                                          gl::ContextID ctxPacked)
{
    // Explicit lock in egl::Display::makeCurrent()
    return nullptr;
}

ContextMutex *GetContextMutex_QueryContext(Thread *thread,
                                           egl::Display *dpyPacked,
                                           gl::ContextID ctxPacked)
{
    // Lock not required - no shared data access
    return nullptr;
}

ContextMutex *GetContextMutex_QueryString(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_QuerySurface(Thread *thread, egl::Display *dpyPacked)
{
    // Explicit lock in egl::QuerySurfaceAttrib()
    return nullptr;
}

ContextMutex *GetContextMutex_SwapBuffers(Thread *thread, egl::Display *dpyPacked)
{
    return TryGetContextMutex(thread);
}

ContextMutex *GetContextMutex_Terminate(Thread *thread, egl::Display *dpyPacked)
{
    // Explicit lock in egl::Display::destroyContext()/makeCurrent()
    return nullptr;
}

ContextMutex *GetContextMutex_WaitGL(Thread *thread)
{
    return TryGetContextMutex(thread);
}

ContextMutex *GetContextMutex_WaitNative(Thread *thread)
{
    return TryGetContextMutex(thread);
}

// EGL 1.1
ContextMutex *GetContextMutex_BindTexImage(Thread *thread, egl::Display *dpyPacked)
{
    return TryGetContextMutex(thread);
}

ContextMutex *GetContextMutex_ReleaseTexImage(Thread *thread, egl::Display *dpyPacked)
{
    return TryGetContextMutex(thread);
}

ContextMutex *GetContextMutex_SurfaceAttrib(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_SwapInterval(Thread *thread, egl::Display *dpyPacked)
{
    // Only checked in Validation
    return nullptr;
}

// EGL 1.2
ContextMutex *GetContextMutex_BindAPI(Thread *thread)
{
    return nullptr;
}

ContextMutex *GetContextMutex_CreatePbufferFromClientBuffer(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_QueryAPI(Thread *thread)
{
    return nullptr;
}

ContextMutex *GetContextMutex_ReleaseThread(Thread *thread)
{
    // Explicit lock in egl::Display::makeCurrent()
    return nullptr;
}

ContextMutex *GetContextMutex_WaitClient(Thread *thread)
{
    return TryGetContextMutex(thread);
}

// EGL 1.4
ContextMutex *GetContextMutex_GetCurrentContext(Thread *thread)
{
    return nullptr;
}

// EGL 1.5
ContextMutex *GetContextMutex_ClientWaitSync(Thread *thread, egl::Display *dpyPacked)
{
    return TryGetContextMutex(thread);
}

ContextMutex *GetContextMutex_CreateImage(Thread *thread,
                                          egl::Display *dpyPacked,
                                          gl::ContextID ctxPacked)
{
    return TryGetActiveSharedContextMutex(GetContextIfValid(dpyPacked, ctxPacked));
}

ContextMutex *GetContextMutex_CreatePlatformPixmapSurface(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_CreatePlatformWindowSurface(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_CreateSync(Thread *thread, egl::Display *dpyPacked)
{
    return TryGetContextMutex(thread);
}

ContextMutex *GetContextMutex_DestroyImage(Thread *thread, egl::Display *dpyPacked)
{
    // Explicit lock in egl::Display::destroyImageImpl()
    return nullptr;
}

ContextMutex *GetContextMutex_DestroySync(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_GetPlatformDisplay(Thread *thread)
{
    return nullptr;
}

ContextMutex *GetContextMutex_GetSyncAttrib(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_WaitSync(Thread *thread, egl::Display *dpyPacked)
{
    return TryGetContextMutex(thread);
}

// EGL_ANDROID_blob_cache
ContextMutex *GetContextMutex_SetBlobCacheFuncsANDROID(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_ANDROID_create_native_client_buffer
ContextMutex *GetContextMutex_CreateNativeClientBufferANDROID(Thread *thread)
{
    return nullptr;
}

// EGL_ANDROID_get_frame_timestamps
ContextMutex *GetContextMutex_GetCompositorTimingSupportedANDROID(Thread *thread,
                                                                  egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_GetCompositorTimingANDROID(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_GetNextFrameIdANDROID(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_GetFrameTimestampSupportedANDROID(Thread *thread,
                                                                egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_GetFrameTimestampsANDROID(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_ANDROID_get_native_client_buffer
ContextMutex *GetContextMutex_GetNativeClientBufferANDROID(Thread *thread)
{
    return nullptr;
}

// EGL_ANDROID_native_fence_sync
ContextMutex *GetContextMutex_DupNativeFenceFDANDROID(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_ANDROID_presentation_time
ContextMutex *GetContextMutex_PresentationTimeANDROID(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_ANGLE_device_creation
ContextMutex *GetContextMutex_CreateDeviceANGLE(Thread *thread)
{
    return nullptr;
}

ContextMutex *GetContextMutex_ReleaseDeviceANGLE(Thread *thread)
{
    return nullptr;
}

// EGL_ANGLE_feature_control
ContextMutex *GetContextMutex_QueryStringiANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_QueryDisplayAttribANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_ANGLE_metal_shared_event_sync
ContextMutex *GetContextMutex_CopyMetalSharedEventANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_ANGLE_power_preference
ContextMutex *GetContextMutex_ReleaseHighPowerGPUANGLE(Thread *thread,
                                                       egl::Display *dpyPacked,
                                                       gl::ContextID ctxPacked)
{
    return TryGetContextMutex(GetContextIfValid(dpyPacked, ctxPacked));
}

ContextMutex *GetContextMutex_ReacquireHighPowerGPUANGLE(Thread *thread,
                                                         egl::Display *dpyPacked,
                                                         gl::ContextID ctxPacked)
{
    return TryGetContextMutex(GetContextIfValid(dpyPacked, ctxPacked));
}

ContextMutex *GetContextMutex_HandleGPUSwitchANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_ForceGPUSwitchANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_ANGLE_prepare_swap_buffers
ContextMutex *GetContextMutex_PrepareSwapBuffersANGLE(Thread *thread, egl::Display *dpyPacked)
{
    // Custom API, implemented in PrepareSwapBuffersANGLE().
    UNREACHABLE();
    return nullptr;
}

// EGL_ANGLE_program_cache_control
ContextMutex *GetContextMutex_ProgramCacheGetAttribANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_ProgramCacheQueryANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_ProgramCachePopulateANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_ProgramCacheResizeANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_ANGLE_query_surface_pointer
ContextMutex *GetContextMutex_QuerySurfacePointerANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_ANGLE_stream_producer_d3d_texture
ContextMutex *GetContextMutex_CreateStreamProducerD3DTextureANGLE(Thread *thread,
                                                                  egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_StreamPostD3DTextureANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_ANGLE_swap_with_frame_token
ContextMutex *GetContextMutex_SwapBuffersWithFrameTokenANGLE(Thread *thread,
                                                             egl::Display *dpyPacked)
{
    return TryGetContextMutex(thread);
}

// EGL_ANGLE_sync_control_rate
ContextMutex *GetContextMutex_GetMscRateANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_ANGLE_vulkan_image
ContextMutex *GetContextMutex_ExportVkImageANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_ANGLE_wait_until_work_scheduled
ContextMutex *GetContextMutex_WaitUntilWorkScheduledANGLE(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_CHROMIUM_sync_control
ContextMutex *GetContextMutex_GetSyncValuesCHROMIUM(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_EXT_device_query
ContextMutex *GetContextMutex_QueryDeviceAttribEXT(Thread *thread)
{
    return nullptr;
}

ContextMutex *GetContextMutex_QueryDeviceStringEXT(Thread *thread)
{
    return nullptr;
}

ContextMutex *GetContextMutex_QueryDisplayAttribEXT(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_EXT_image_dma_buf_import_modifiers
ContextMutex *GetContextMutex_QueryDmaBufFormatsEXT(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_QueryDmaBufModifiersEXT(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_EXT_platform_base
ContextMutex *GetContextMutex_CreatePlatformPixmapSurfaceEXT(Thread *thread,
                                                             egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_CreatePlatformWindowSurfaceEXT(Thread *thread,
                                                             egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_GetPlatformDisplayEXT(Thread *thread)
{
    return nullptr;
}

// EGL_KHR_debug
ContextMutex *GetContextMutex_DebugMessageControlKHR(Thread *thread)
{
    return nullptr;
}

ContextMutex *GetContextMutex_LabelObjectKHR(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_QueryDebugKHR(Thread *thread)
{
    return nullptr;
}

// EGL_KHR_fence_sync
ContextMutex *GetContextMutex_ClientWaitSyncKHR(Thread *thread, egl::Display *dpyPacked)
{
    return TryGetContextMutex(thread);
}

ContextMutex *GetContextMutex_CreateSyncKHR(Thread *thread, egl::Display *dpyPacked)
{
    return TryGetContextMutex(thread);
}

ContextMutex *GetContextMutex_DestroySyncKHR(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_GetSyncAttribKHR(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_KHR_image
ContextMutex *GetContextMutex_CreateImageKHR(Thread *thread,
                                             egl::Display *dpyPacked,
                                             gl::ContextID ctxPacked)
{
    return TryGetActiveSharedContextMutex(GetContextIfValid(dpyPacked, ctxPacked));
}

ContextMutex *GetContextMutex_DestroyImageKHR(Thread *thread, egl::Display *dpyPacked)
{
    // Explicit lock in egl::Display::destroyImageImpl()
    return nullptr;
}

// EGL_KHR_lock_surface3
ContextMutex *GetContextMutex_LockSurfaceKHR(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_QuerySurface64KHR(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_UnlockSurfaceKHR(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_KHR_partial_update
ContextMutex *GetContextMutex_SetDamageRegionKHR(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_KHR_reusable_sync
ContextMutex *GetContextMutex_SignalSyncKHR(Thread *thread, egl::Display *dpyPacked)
{
    return TryGetContextMutex(thread);
}

// EGL_KHR_stream
ContextMutex *GetContextMutex_CreateStreamKHR(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_DestroyStreamKHR(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_QueryStreamKHR(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_QueryStreamu64KHR(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

ContextMutex *GetContextMutex_StreamAttribKHR(Thread *thread, egl::Display *dpyPacked)
{
    return nullptr;
}

// EGL_KHR_stream_consumer_gltexture
ContextMutex *GetContextMutex_StreamConsumerAcquireKHR(Thread *thread, egl::Display *dpyPacked)
{
    return TryGetContextMutex(thread);
}

ContextMutex *GetContextMutex_StreamConsumerGLTextureExternalKHR(Thread *thread,
                                                                 egl::Display *dpyPacked)
{
    return TryGetContextMutex(thread);
}

ContextMutex *GetContextMutex_StreamConsumerReleaseKHR(Thread *thread, egl::Display *dpyPacked)
{
    return TryGetContextMutex(thread);
}

// EGL_KHR_swap_buffers_with_damage
ContextMutex *GetContextMutex_SwapBuffersWithDamageKHR(Thread *thread, egl::Display *dpyPacked)
{
    return TryGetContextMutex(thread);
}

// EGL_KHR_wait_sync
ContextMutex *GetContextMutex_WaitSyncKHR(Thread *thread, egl::Display *dpyPacked)
{
    return TryGetContextMutex(thread);
}

// EGL_NV_post_sub_buffer
ContextMutex *GetContextMutex_PostSubBufferNV(Thread *thread, egl::Display *dpyPacked)
{
    return TryGetContextMutex(thread);
}

// EGL_NV_stream_consumer_gltexture_yuv
ContextMutex *GetContextMutex_StreamConsumerGLTextureExternalAttribsNV(Thread *thread,
                                                                       egl::Display *dpyPacked)
{
    return TryGetContextMutex(thread);
}

}  // namespace egl

#endif  // LIBGLESV2_EGL_CONTEXT_MUTEX_IMPL_H_
