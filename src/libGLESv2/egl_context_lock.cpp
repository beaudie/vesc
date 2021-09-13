#if SVDT_ENABLE_SHARED_CONTEXT_MUTEX
#    include "libGLESv2/egl_context_lock_autogen.h"
#endif

#if SVDT_ENABLE_SHARED_CONTEXT_MUTEX
namespace egl
{
// Warning: Called before the Validation. Need to use "TryGetContextLock()".

std::unique_lock<angle::GlobalMutex> GetContextLock_ChooseConfig(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_CopyBuffers(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_CreateContext(
    const Thread *thread,
    const gl::Context *share_contextPacked)
{
    return TryGetContextLock(share_contextPacked);
}

std::unique_lock<angle::GlobalMutex> GetContextLock_CreatePbufferSurface(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_CreatePixmapSurface(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_CreateWindowSurface(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_DestroyContext(const Thread *thread,
                                                                   const gl::Context *ctxPacked)
{
    return {};  // Explicit lock in egl::Display::destroyContextWithSurfaces()/makeCurrent()
}

std::unique_lock<angle::GlobalMutex> GetContextLock_DestroySurface(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_GetConfigAttrib(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_GetConfigs(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_GetCurrentDisplay(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_GetCurrentSurface(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_GetDisplay(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_GetError(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_GetProcAddress(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_Initialize(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_MakeCurrent(const Thread *thread,
                                                                const gl::Context *ctxPacked)
{
    return {};  // Explicit lock in egl::Display::makeCurrent()
}

std::unique_lock<angle::GlobalMutex> GetContextLock_QueryContext(const Thread *thread,
                                                                 const gl::Context *ctxPacked)
{
    return {};  // Lock not required - no shared data access
}

std::unique_lock<angle::GlobalMutex> GetContextLock_QueryString(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_QuerySurface(const Thread *thread)
{
    return {};  // Explicit lock in egl::QuerySurfaceAttrib()
}

std::unique_lock<angle::GlobalMutex> GetContextLock_SwapBuffers(const Thread *thread)
{
    return TryGetContextLock(thread);
}

std::unique_lock<angle::GlobalMutex> GetContextLock_Terminate(const Thread *thread)
{
    return {};  // Explicit lock in egl::Display::destroyContextWithSurfaces()/makeCurrent()
}

std::unique_lock<angle::GlobalMutex> GetContextLock_WaitGL(const Thread *thread)
{
    return TryGetContextLock(thread);
}

std::unique_lock<angle::GlobalMutex> GetContextLock_WaitNative(const Thread *thread)
{
    return TryGetContextLock(thread);
}

// EGL 1.1
std::unique_lock<angle::GlobalMutex> GetContextLock_BindTexImage(const Thread *thread)
{
    return TryGetContextLock(thread);
}

std::unique_lock<angle::GlobalMutex> GetContextLock_ReleaseTexImage(const Thread *thread)
{
    return TryGetContextLock(thread);
}

std::unique_lock<angle::GlobalMutex> GetContextLock_SurfaceAttrib(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_SwapInterval(const Thread *thread)
{
    return {};  // Only checked in Validation
}

// EGL 1.2
std::unique_lock<angle::GlobalMutex> GetContextLock_BindAPI(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_CreatePbufferFromClientBuffer(
    const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_QueryAPI(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_ReleaseThread(const Thread *thread)
{
    return {};  // Explicit lock in egl::Display::makeCurrent()
}

std::unique_lock<angle::GlobalMutex> GetContextLock_WaitClient(const Thread *thread)
{
    return TryGetContextLock(thread);
}

// EGL 1.4
std::unique_lock<angle::GlobalMutex> GetContextLock_GetCurrentContext(const Thread *thread)
{
    return {};
}

// EGL 1.5
std::unique_lock<angle::GlobalMutex> GetContextLock_ClientWaitSync(const Thread *thread)
{
    return TryGetContextLock(thread);
}

std::unique_lock<angle::GlobalMutex> GetContextLock_CreateImage(const Thread *thread,
                                                                const gl::Context *ctxPacked)
{
    return TryGetContextLock(ctxPacked);
}

std::unique_lock<angle::GlobalMutex> GetContextLock_CreatePlatformPixmapSurface(
    const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_CreatePlatformWindowSurface(
    const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_CreateSync(const Thread *thread)
{
    return TryGetContextLock(thread);
}

std::unique_lock<angle::GlobalMutex> GetContextLock_DestroyImage(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_DestroySync(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_GetPlatformDisplay(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_GetSyncAttrib(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_WaitSync(const Thread *thread)
{
    return TryGetContextLock(thread);
}

// EGL_ANDROID_blob_cache
std::unique_lock<angle::GlobalMutex> GetContextLock_SetBlobCacheFuncsANDROID(const Thread *thread)
{
    return {};
}

// EGL_ANDROID_create_native_client_buffer
std::unique_lock<angle::GlobalMutex> GetContextLock_CreateNativeClientBufferANDROID(
    const Thread *thread)
{
    return {};
}

// EGL_ANDROID_get_frame_timestamps
std::unique_lock<angle::GlobalMutex> GetContextLock_GetCompositorTimingSupportedANDROID(
    const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_GetCompositorTimingANDROID(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_GetNextFrameIdANDROID(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_GetFrameTimestampSupportedANDROID(
    const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_GetFrameTimestampsANDROID(const Thread *thread)
{
    return {};
}

// EGL_ANDROID_get_native_client_buffer
std::unique_lock<angle::GlobalMutex> GetContextLock_GetNativeClientBufferANDROID(
    const Thread *thread)
{
    return {};
}

// EGL_ANDROID_native_fence_sync
std::unique_lock<angle::GlobalMutex> GetContextLock_DupNativeFenceFDANDROID(const Thread *thread)
{
    return {};
}

// EGL_ANDROID_presentation_time
std::unique_lock<angle::GlobalMutex> GetContextLock_PresentationTimeANDROID(const Thread *thread)
{
    return {};
}

// EGL_ANGLE_device_creation
std::unique_lock<angle::GlobalMutex> GetContextLock_CreateDeviceANGLE(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_ReleaseDeviceANGLE(const Thread *thread)
{
    return {};
}

// EGL_ANGLE_feature_control
std::unique_lock<angle::GlobalMutex> GetContextLock_QueryStringiANGLE(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_QueryDisplayAttribANGLE(const Thread *thread)
{
    return {};
}

// EGL_ANGLE_power_preference
std::unique_lock<angle::GlobalMutex> GetContextLock_ReleaseHighPowerGPUANGLE(
    const Thread *thread,
    const gl::Context *ctxPacked)
{
    return TryGetContextLock(ctxPacked);
}

std::unique_lock<angle::GlobalMutex> GetContextLock_ReacquireHighPowerGPUANGLE(
    const Thread *thread,
    const gl::Context *ctxPacked)
{
    return TryGetContextLock(ctxPacked);
}

std::unique_lock<angle::GlobalMutex> GetContextLock_HandleGPUSwitchANGLE(const Thread *thread)
{
    return {};
}

// EGL_ANGLE_program_cache_control
std::unique_lock<angle::GlobalMutex> GetContextLock_ProgramCacheGetAttribANGLE(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_ProgramCacheQueryANGLE(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_ProgramCachePopulateANGLE(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_ProgramCacheResizeANGLE(const Thread *thread)
{
    return {};
}

// EGL_ANGLE_query_surface_pointer
std::unique_lock<angle::GlobalMutex> GetContextLock_QuerySurfacePointerANGLE(const Thread *thread)
{
    return {};
}

// EGL_ANGLE_stream_producer_d3d_texture
std::unique_lock<angle::GlobalMutex> GetContextLock_CreateStreamProducerD3DTextureANGLE(
    const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_StreamPostD3DTextureANGLE(const Thread *thread)
{
    return {};
}

// EGL_ANGLE_swap_with_frame_token
std::unique_lock<angle::GlobalMutex> GetContextLock_SwapBuffersWithFrameTokenANGLE(
    const Thread *thread)
{
    return TryGetContextLock(thread);
}

// EGL_ANGLE_sync_control_rate
std::unique_lock<angle::GlobalMutex> GetContextLock_GetMscRateANGLE(const Thread *thread)
{
    return {};
}

// EGL_CHROMIUM_sync_control
std::unique_lock<angle::GlobalMutex> GetContextLock_GetSyncValuesCHROMIUM(const Thread *thread)
{
    return {};
}

// EGL_EXT_device_query
std::unique_lock<angle::GlobalMutex> GetContextLock_QueryDeviceAttribEXT(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_QueryDeviceStringEXT(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_QueryDisplayAttribEXT(const Thread *thread)
{
    return {};
}

// EGL_EXT_platform_base
std::unique_lock<angle::GlobalMutex> GetContextLock_CreatePlatformPixmapSurfaceEXT(
    const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_CreatePlatformWindowSurfaceEXT(
    const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_GetPlatformDisplayEXT(const Thread *thread)
{
    return {};
}

// EGL_KHR_debug
std::unique_lock<angle::GlobalMutex> GetContextLock_DebugMessageControlKHR(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_LabelObjectKHR(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_QueryDebugKHR(const Thread *thread)
{
    return {};
}

// EGL_KHR_fence_sync
std::unique_lock<angle::GlobalMutex> GetContextLock_ClientWaitSyncKHR(const Thread *thread)
{
    return TryGetContextLock(thread);
}

std::unique_lock<angle::GlobalMutex> GetContextLock_CreateSyncKHR(const Thread *thread)
{
    return TryGetContextLock(thread);
}

std::unique_lock<angle::GlobalMutex> GetContextLock_DestroySyncKHR(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_GetSyncAttribKHR(const Thread *thread)
{
    return {};
}

// EGL_KHR_image
std::unique_lock<angle::GlobalMutex> GetContextLock_CreateImageKHR(const Thread *thread,
                                                                   const gl::Context *ctxPacked)
{
    return TryGetContextLock(ctxPacked);
}

std::unique_lock<angle::GlobalMutex> GetContextLock_DestroyImageKHR(const Thread *thread)
{
    return {};
}

// EGL_KHR_reusable_sync
std::unique_lock<angle::GlobalMutex> GetContextLock_SignalSyncKHR(const Thread *thread)
{
    return TryGetContextLock(thread);
}

// EGL_KHR_stream
std::unique_lock<angle::GlobalMutex> GetContextLock_CreateStreamKHR(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_DestroyStreamKHR(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_QueryStreamKHR(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_QueryStreamu64KHR(const Thread *thread)
{
    return {};
}

std::unique_lock<angle::GlobalMutex> GetContextLock_StreamAttribKHR(const Thread *thread)
{
    return {};
}

// EGL_KHR_stream_consumer_gltexture
std::unique_lock<angle::GlobalMutex> GetContextLock_StreamConsumerAcquireKHR(const Thread *thread)
{
    return TryGetContextLock(thread);
}

std::unique_lock<angle::GlobalMutex> GetContextLock_StreamConsumerGLTextureExternalKHR(
    const Thread *thread)
{
    return TryGetContextLock(thread);
}

std::unique_lock<angle::GlobalMutex> GetContextLock_StreamConsumerReleaseKHR(const Thread *thread)
{
    return TryGetContextLock(thread);
}

// EGL_KHR_swap_buffers_with_damage
std::unique_lock<angle::GlobalMutex> GetContextLock_SwapBuffersWithDamageKHR(const Thread *thread)
{
    return TryGetContextLock(thread);
}

// EGL_KHR_wait_sync
std::unique_lock<angle::GlobalMutex> GetContextLock_WaitSyncKHR(const Thread *thread)
{
    return TryGetContextLock(thread);
}

// EGL_NV_post_sub_buffer
std::unique_lock<angle::GlobalMutex> GetContextLock_PostSubBufferNV(const Thread *thread)
{
    return TryGetContextLock(thread);
}

// EGL_NV_stream_consumer_gltexture_yuv
std::unique_lock<angle::GlobalMutex> GetContextLock_StreamConsumerGLTextureExternalAttribsNV(
    const Thread *thread)
{
    return TryGetContextLock(thread);
}

}  // namespace egl
#endif  // SVDT_ENABLE_SHARED_CONTEXT_MUTEX
