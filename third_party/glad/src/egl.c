#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/egl.h>

#ifndef GLAD_IMPL_UTIL_C_
#define GLAD_IMPL_UTIL_C_

#ifdef _MSC_VER
#define GLAD_IMPL_UTIL_SSCANF sscanf_s
#else
#define GLAD_IMPL_UTIL_SSCANF sscanf
#endif

#endif /* GLAD_IMPL_UTIL_C_ */


int GLAD_EGL_VERSION_1_0 = 0;
int GLAD_EGL_VERSION_1_1 = 0;
int GLAD_EGL_VERSION_1_2 = 0;
int GLAD_EGL_VERSION_1_3 = 0;
int GLAD_EGL_VERSION_1_4 = 0;
int GLAD_EGL_VERSION_1_5 = 0;
int GLAD_EGL_ANDROID_blob_cache = 0;
int GLAD_EGL_ANDROID_create_native_client_buffer = 0;
int GLAD_EGL_ANDROID_framebuffer_target = 0;
int GLAD_EGL_ANDROID_front_buffer_auto_refresh = 0;
int GLAD_EGL_ANDROID_get_frame_timestamps = 0;
int GLAD_EGL_ANDROID_get_native_client_buffer = 0;
int GLAD_EGL_ANDROID_image_native_buffer = 0;
int GLAD_EGL_ANDROID_native_fence_sync = 0;
int GLAD_EGL_ANDROID_presentation_time = 0;
int GLAD_EGL_ANDROID_recordable = 0;
int GLAD_EGL_ANGLE_d3d_share_handle_client_buffer = 0;
int GLAD_EGL_ANGLE_device_d3d = 0;
int GLAD_EGL_ANGLE_query_surface_pointer = 0;
int GLAD_EGL_ANGLE_surface_d3d_texture_2d_share_handle = 0;
int GLAD_EGL_ANGLE_window_fixed_size = 0;
int GLAD_EGL_ARM_implicit_external_sync = 0;
int GLAD_EGL_ARM_pixmap_multisample_discard = 0;
int GLAD_EGL_EXT_bind_to_front = 0;
int GLAD_EGL_EXT_buffer_age = 0;
int GLAD_EGL_EXT_client_extensions = 0;
int GLAD_EGL_EXT_client_sync = 0;
int GLAD_EGL_EXT_compositor = 0;
int GLAD_EGL_EXT_create_context_robustness = 0;
int GLAD_EGL_EXT_device_base = 0;
int GLAD_EGL_EXT_device_drm = 0;
int GLAD_EGL_EXT_device_enumeration = 0;
int GLAD_EGL_EXT_device_openwf = 0;
int GLAD_EGL_EXT_device_query = 0;
int GLAD_EGL_EXT_gl_colorspace_bt2020_linear = 0;
int GLAD_EGL_EXT_gl_colorspace_bt2020_pq = 0;
int GLAD_EGL_EXT_gl_colorspace_display_p3 = 0;
int GLAD_EGL_EXT_gl_colorspace_display_p3_linear = 0;
int GLAD_EGL_EXT_gl_colorspace_scrgb = 0;
int GLAD_EGL_EXT_gl_colorspace_scrgb_linear = 0;
int GLAD_EGL_EXT_image_dma_buf_import = 0;
int GLAD_EGL_EXT_image_dma_buf_import_modifiers = 0;
int GLAD_EGL_EXT_image_gl_colorspace = 0;
int GLAD_EGL_EXT_image_implicit_sync_control = 0;
int GLAD_EGL_EXT_multiview_window = 0;
int GLAD_EGL_EXT_output_base = 0;
int GLAD_EGL_EXT_output_drm = 0;
int GLAD_EGL_EXT_output_openwf = 0;
int GLAD_EGL_EXT_pixel_format_float = 0;
int GLAD_EGL_EXT_platform_base = 0;
int GLAD_EGL_EXT_platform_device = 0;
int GLAD_EGL_EXT_platform_wayland = 0;
int GLAD_EGL_EXT_platform_x11 = 0;
int GLAD_EGL_EXT_protected_content = 0;
int GLAD_EGL_EXT_protected_surface = 0;
int GLAD_EGL_EXT_stream_consumer_egloutput = 0;
int GLAD_EGL_EXT_surface_CTA861_3_metadata = 0;
int GLAD_EGL_EXT_surface_SMPTE2086_metadata = 0;
int GLAD_EGL_EXT_swap_buffers_with_damage = 0;
int GLAD_EGL_EXT_sync_reuse = 0;
int GLAD_EGL_EXT_yuv_surface = 0;
int GLAD_EGL_HI_clientpixmap = 0;
int GLAD_EGL_HI_colorformats = 0;
int GLAD_EGL_IMG_context_priority = 0;
int GLAD_EGL_IMG_image_plane_attribs = 0;
int GLAD_EGL_KHR_cl_event = 0;
int GLAD_EGL_KHR_cl_event2 = 0;
int GLAD_EGL_KHR_client_get_all_proc_addresses = 0;
int GLAD_EGL_KHR_config_attribs = 0;
int GLAD_EGL_KHR_context_flush_control = 0;
int GLAD_EGL_KHR_create_context = 0;
int GLAD_EGL_KHR_create_context_no_error = 0;
int GLAD_EGL_KHR_debug = 0;
int GLAD_EGL_KHR_display_reference = 0;
int GLAD_EGL_KHR_fence_sync = 0;
int GLAD_EGL_KHR_get_all_proc_addresses = 0;
int GLAD_EGL_KHR_gl_colorspace = 0;
int GLAD_EGL_KHR_gl_renderbuffer_image = 0;
int GLAD_EGL_KHR_gl_texture_2D_image = 0;
int GLAD_EGL_KHR_gl_texture_3D_image = 0;
int GLAD_EGL_KHR_gl_texture_cubemap_image = 0;
int GLAD_EGL_KHR_image = 0;
int GLAD_EGL_KHR_image_base = 0;
int GLAD_EGL_KHR_image_pixmap = 0;
int GLAD_EGL_KHR_lock_surface = 0;
int GLAD_EGL_KHR_lock_surface2 = 0;
int GLAD_EGL_KHR_lock_surface3 = 0;
int GLAD_EGL_KHR_mutable_render_buffer = 0;
int GLAD_EGL_KHR_no_config_context = 0;
int GLAD_EGL_KHR_partial_update = 0;
int GLAD_EGL_KHR_platform_android = 0;
int GLAD_EGL_KHR_platform_gbm = 0;
int GLAD_EGL_KHR_platform_wayland = 0;
int GLAD_EGL_KHR_platform_x11 = 0;
int GLAD_EGL_KHR_reusable_sync = 0;
int GLAD_EGL_KHR_stream = 0;
int GLAD_EGL_KHR_stream_attrib = 0;
int GLAD_EGL_KHR_stream_consumer_gltexture = 0;
int GLAD_EGL_KHR_stream_cross_process_fd = 0;
int GLAD_EGL_KHR_stream_fifo = 0;
int GLAD_EGL_KHR_stream_producer_aldatalocator = 0;
int GLAD_EGL_KHR_stream_producer_eglsurface = 0;
int GLAD_EGL_KHR_surfaceless_context = 0;
int GLAD_EGL_KHR_swap_buffers_with_damage = 0;
int GLAD_EGL_KHR_vg_parent_image = 0;
int GLAD_EGL_KHR_wait_sync = 0;
int GLAD_EGL_MESA_drm_image = 0;
int GLAD_EGL_MESA_image_dma_buf_export = 0;
int GLAD_EGL_MESA_platform_gbm = 0;
int GLAD_EGL_MESA_platform_surfaceless = 0;
int GLAD_EGL_NOK_swap_region = 0;
int GLAD_EGL_NOK_swap_region2 = 0;
int GLAD_EGL_NOK_texture_from_pixmap = 0;
int GLAD_EGL_NV_3dvision_surface = 0;
int GLAD_EGL_NV_context_priority_realtime = 0;
int GLAD_EGL_NV_coverage_sample = 0;
int GLAD_EGL_NV_coverage_sample_resolve = 0;
int GLAD_EGL_NV_cuda_event = 0;
int GLAD_EGL_NV_depth_nonlinear = 0;
int GLAD_EGL_NV_device_cuda = 0;
int GLAD_EGL_NV_native_query = 0;
int GLAD_EGL_NV_post_convert_rounding = 0;
int GLAD_EGL_NV_post_sub_buffer = 0;
int GLAD_EGL_NV_robustness_video_memory_purge = 0;
int GLAD_EGL_NV_stream_consumer_gltexture_yuv = 0;
int GLAD_EGL_NV_stream_cross_display = 0;
int GLAD_EGL_NV_stream_cross_object = 0;
int GLAD_EGL_NV_stream_cross_partition = 0;
int GLAD_EGL_NV_stream_cross_process = 0;
int GLAD_EGL_NV_stream_cross_system = 0;
int GLAD_EGL_NV_stream_fifo_next = 0;
int GLAD_EGL_NV_stream_fifo_synchronous = 0;
int GLAD_EGL_NV_stream_flush = 0;
int GLAD_EGL_NV_stream_frame_limits = 0;
int GLAD_EGL_NV_stream_metadata = 0;
int GLAD_EGL_NV_stream_remote = 0;
int GLAD_EGL_NV_stream_reset = 0;
int GLAD_EGL_NV_stream_socket = 0;
int GLAD_EGL_NV_stream_socket_inet = 0;
int GLAD_EGL_NV_stream_socket_unix = 0;
int GLAD_EGL_NV_stream_sync = 0;
int GLAD_EGL_NV_sync = 0;
int GLAD_EGL_NV_system_time = 0;
int GLAD_EGL_TIZEN_image_native_buffer = 0;
int GLAD_EGL_TIZEN_image_native_surface = 0;



PFNEGLBINDAPIPROC glad_eglBindAPI = NULL;
PFNEGLBINDTEXIMAGEPROC glad_eglBindTexImage = NULL;
PFNEGLCHOOSECONFIGPROC glad_eglChooseConfig = NULL;
PFNEGLCLIENTSIGNALSYNCEXTPROC glad_eglClientSignalSyncEXT = NULL;
PFNEGLCLIENTWAITSYNCPROC glad_eglClientWaitSync = NULL;
PFNEGLCLIENTWAITSYNCKHRPROC glad_eglClientWaitSyncKHR = NULL;
PFNEGLCLIENTWAITSYNCNVPROC glad_eglClientWaitSyncNV = NULL;
PFNEGLCOMPOSITORBINDTEXWINDOWEXTPROC glad_eglCompositorBindTexWindowEXT = NULL;
PFNEGLCOMPOSITORSETCONTEXTATTRIBUTESEXTPROC glad_eglCompositorSetContextAttributesEXT = NULL;
PFNEGLCOMPOSITORSETCONTEXTLISTEXTPROC glad_eglCompositorSetContextListEXT = NULL;
PFNEGLCOMPOSITORSETSIZEEXTPROC glad_eglCompositorSetSizeEXT = NULL;
PFNEGLCOMPOSITORSETWINDOWATTRIBUTESEXTPROC glad_eglCompositorSetWindowAttributesEXT = NULL;
PFNEGLCOMPOSITORSETWINDOWLISTEXTPROC glad_eglCompositorSetWindowListEXT = NULL;
PFNEGLCOMPOSITORSWAPPOLICYEXTPROC glad_eglCompositorSwapPolicyEXT = NULL;
PFNEGLCOPYBUFFERSPROC glad_eglCopyBuffers = NULL;
PFNEGLCREATECONTEXTPROC glad_eglCreateContext = NULL;
PFNEGLCREATEDRMIMAGEMESAPROC glad_eglCreateDRMImageMESA = NULL;
PFNEGLCREATEFENCESYNCNVPROC glad_eglCreateFenceSyncNV = NULL;
PFNEGLCREATEIMAGEPROC glad_eglCreateImage = NULL;
PFNEGLCREATEIMAGEKHRPROC glad_eglCreateImageKHR = NULL;
PFNEGLCREATENATIVECLIENTBUFFERANDROIDPROC glad_eglCreateNativeClientBufferANDROID = NULL;
PFNEGLCREATEPBUFFERFROMCLIENTBUFFERPROC glad_eglCreatePbufferFromClientBuffer = NULL;
PFNEGLCREATEPBUFFERSURFACEPROC glad_eglCreatePbufferSurface = NULL;
PFNEGLCREATEPIXMAPSURFACEPROC glad_eglCreatePixmapSurface = NULL;
PFNEGLCREATEPIXMAPSURFACEHIPROC glad_eglCreatePixmapSurfaceHI = NULL;
PFNEGLCREATEPLATFORMPIXMAPSURFACEPROC glad_eglCreatePlatformPixmapSurface = NULL;
PFNEGLCREATEPLATFORMPIXMAPSURFACEEXTPROC glad_eglCreatePlatformPixmapSurfaceEXT = NULL;
PFNEGLCREATEPLATFORMWINDOWSURFACEPROC glad_eglCreatePlatformWindowSurface = NULL;
PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC glad_eglCreatePlatformWindowSurfaceEXT = NULL;
PFNEGLCREATESTREAMATTRIBKHRPROC glad_eglCreateStreamAttribKHR = NULL;
PFNEGLCREATESTREAMFROMFILEDESCRIPTORKHRPROC glad_eglCreateStreamFromFileDescriptorKHR = NULL;
PFNEGLCREATESTREAMKHRPROC glad_eglCreateStreamKHR = NULL;
PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC glad_eglCreateStreamProducerSurfaceKHR = NULL;
PFNEGLCREATESTREAMSYNCNVPROC glad_eglCreateStreamSyncNV = NULL;
PFNEGLCREATESYNCPROC glad_eglCreateSync = NULL;
PFNEGLCREATESYNC64KHRPROC glad_eglCreateSync64KHR = NULL;
PFNEGLCREATESYNCKHRPROC glad_eglCreateSyncKHR = NULL;
PFNEGLCREATEWINDOWSURFACEPROC glad_eglCreateWindowSurface = NULL;
PFNEGLDEBUGMESSAGECONTROLKHRPROC glad_eglDebugMessageControlKHR = NULL;
PFNEGLDESTROYCONTEXTPROC glad_eglDestroyContext = NULL;
PFNEGLDESTROYIMAGEPROC glad_eglDestroyImage = NULL;
PFNEGLDESTROYIMAGEKHRPROC glad_eglDestroyImageKHR = NULL;
PFNEGLDESTROYSTREAMKHRPROC glad_eglDestroyStreamKHR = NULL;
PFNEGLDESTROYSURFACEPROC glad_eglDestroySurface = NULL;
PFNEGLDESTROYSYNCPROC glad_eglDestroySync = NULL;
PFNEGLDESTROYSYNCKHRPROC glad_eglDestroySyncKHR = NULL;
PFNEGLDESTROYSYNCNVPROC glad_eglDestroySyncNV = NULL;
PFNEGLDUPNATIVEFENCEFDANDROIDPROC glad_eglDupNativeFenceFDANDROID = NULL;
PFNEGLEXPORTDMABUFIMAGEMESAPROC glad_eglExportDMABUFImageMESA = NULL;
PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC glad_eglExportDMABUFImageQueryMESA = NULL;
PFNEGLEXPORTDRMIMAGEMESAPROC glad_eglExportDRMImageMESA = NULL;
PFNEGLFENCENVPROC glad_eglFenceNV = NULL;
PFNEGLGETCOMPOSITORTIMINGANDROIDPROC glad_eglGetCompositorTimingANDROID = NULL;
PFNEGLGETCOMPOSITORTIMINGSUPPORTEDANDROIDPROC glad_eglGetCompositorTimingSupportedANDROID = NULL;
PFNEGLGETCONFIGATTRIBPROC glad_eglGetConfigAttrib = NULL;
PFNEGLGETCONFIGSPROC glad_eglGetConfigs = NULL;
PFNEGLGETCURRENTCONTEXTPROC glad_eglGetCurrentContext = NULL;
PFNEGLGETCURRENTDISPLAYPROC glad_eglGetCurrentDisplay = NULL;
PFNEGLGETCURRENTSURFACEPROC glad_eglGetCurrentSurface = NULL;
PFNEGLGETDISPLAYPROC glad_eglGetDisplay = NULL;
PFNEGLGETERRORPROC glad_eglGetError = NULL;
PFNEGLGETFRAMETIMESTAMPSUPPORTEDANDROIDPROC glad_eglGetFrameTimestampSupportedANDROID = NULL;
PFNEGLGETFRAMETIMESTAMPSANDROIDPROC glad_eglGetFrameTimestampsANDROID = NULL;
PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC glad_eglGetNativeClientBufferANDROID = NULL;
PFNEGLGETNEXTFRAMEIDANDROIDPROC glad_eglGetNextFrameIdANDROID = NULL;
PFNEGLGETOUTPUTLAYERSEXTPROC glad_eglGetOutputLayersEXT = NULL;
PFNEGLGETOUTPUTPORTSEXTPROC glad_eglGetOutputPortsEXT = NULL;
PFNEGLGETPLATFORMDISPLAYPROC glad_eglGetPlatformDisplay = NULL;
PFNEGLGETPLATFORMDISPLAYEXTPROC glad_eglGetPlatformDisplayEXT = NULL;
PFNEGLGETPROCADDRESSPROC glad_eglGetProcAddress = NULL;
PFNEGLGETSTREAMFILEDESCRIPTORKHRPROC glad_eglGetStreamFileDescriptorKHR = NULL;
PFNEGLGETSYNCATTRIBPROC glad_eglGetSyncAttrib = NULL;
PFNEGLGETSYNCATTRIBKHRPROC glad_eglGetSyncAttribKHR = NULL;
PFNEGLGETSYNCATTRIBNVPROC glad_eglGetSyncAttribNV = NULL;
PFNEGLGETSYSTEMTIMEFREQUENCYNVPROC glad_eglGetSystemTimeFrequencyNV = NULL;
PFNEGLGETSYSTEMTIMENVPROC glad_eglGetSystemTimeNV = NULL;
PFNEGLINITIALIZEPROC glad_eglInitialize = NULL;
PFNEGLLABELOBJECTKHRPROC glad_eglLabelObjectKHR = NULL;
PFNEGLLOCKSURFACEKHRPROC glad_eglLockSurfaceKHR = NULL;
PFNEGLMAKECURRENTPROC glad_eglMakeCurrent = NULL;
PFNEGLOUTPUTLAYERATTRIBEXTPROC glad_eglOutputLayerAttribEXT = NULL;
PFNEGLOUTPUTPORTATTRIBEXTPROC glad_eglOutputPortAttribEXT = NULL;
PFNEGLPOSTSUBBUFFERNVPROC glad_eglPostSubBufferNV = NULL;
PFNEGLPRESENTATIONTIMEANDROIDPROC glad_eglPresentationTimeANDROID = NULL;
PFNEGLQUERYAPIPROC glad_eglQueryAPI = NULL;
PFNEGLQUERYCONTEXTPROC glad_eglQueryContext = NULL;
PFNEGLQUERYDEBUGKHRPROC glad_eglQueryDebugKHR = NULL;
PFNEGLQUERYDEVICEATTRIBEXTPROC glad_eglQueryDeviceAttribEXT = NULL;
PFNEGLQUERYDEVICESTRINGEXTPROC glad_eglQueryDeviceStringEXT = NULL;
PFNEGLQUERYDEVICESEXTPROC glad_eglQueryDevicesEXT = NULL;
PFNEGLQUERYDISPLAYATTRIBEXTPROC glad_eglQueryDisplayAttribEXT = NULL;
PFNEGLQUERYDISPLAYATTRIBKHRPROC glad_eglQueryDisplayAttribKHR = NULL;
PFNEGLQUERYDISPLAYATTRIBNVPROC glad_eglQueryDisplayAttribNV = NULL;
PFNEGLQUERYDMABUFFORMATSEXTPROC glad_eglQueryDmaBufFormatsEXT = NULL;
PFNEGLQUERYDMABUFMODIFIERSEXTPROC glad_eglQueryDmaBufModifiersEXT = NULL;
PFNEGLQUERYNATIVEDISPLAYNVPROC glad_eglQueryNativeDisplayNV = NULL;
PFNEGLQUERYNATIVEPIXMAPNVPROC glad_eglQueryNativePixmapNV = NULL;
PFNEGLQUERYNATIVEWINDOWNVPROC glad_eglQueryNativeWindowNV = NULL;
PFNEGLQUERYOUTPUTLAYERATTRIBEXTPROC glad_eglQueryOutputLayerAttribEXT = NULL;
PFNEGLQUERYOUTPUTLAYERSTRINGEXTPROC glad_eglQueryOutputLayerStringEXT = NULL;
PFNEGLQUERYOUTPUTPORTATTRIBEXTPROC glad_eglQueryOutputPortAttribEXT = NULL;
PFNEGLQUERYOUTPUTPORTSTRINGEXTPROC glad_eglQueryOutputPortStringEXT = NULL;
PFNEGLQUERYSTREAMATTRIBKHRPROC glad_eglQueryStreamAttribKHR = NULL;
PFNEGLQUERYSTREAMKHRPROC glad_eglQueryStreamKHR = NULL;
PFNEGLQUERYSTREAMMETADATANVPROC glad_eglQueryStreamMetadataNV = NULL;
PFNEGLQUERYSTREAMTIMEKHRPROC glad_eglQueryStreamTimeKHR = NULL;
PFNEGLQUERYSTREAMU64KHRPROC glad_eglQueryStreamu64KHR = NULL;
PFNEGLQUERYSTRINGPROC glad_eglQueryString = NULL;
PFNEGLQUERYSURFACEPROC glad_eglQuerySurface = NULL;
PFNEGLQUERYSURFACE64KHRPROC glad_eglQuerySurface64KHR = NULL;
PFNEGLQUERYSURFACEPOINTERANGLEPROC glad_eglQuerySurfacePointerANGLE = NULL;
PFNEGLRELEASETEXIMAGEPROC glad_eglReleaseTexImage = NULL;
PFNEGLRELEASETHREADPROC glad_eglReleaseThread = NULL;
PFNEGLRESETSTREAMNVPROC glad_eglResetStreamNV = NULL;
PFNEGLSETBLOBCACHEFUNCSANDROIDPROC glad_eglSetBlobCacheFuncsANDROID = NULL;
PFNEGLSETDAMAGEREGIONKHRPROC glad_eglSetDamageRegionKHR = NULL;
PFNEGLSETSTREAMATTRIBKHRPROC glad_eglSetStreamAttribKHR = NULL;
PFNEGLSETSTREAMMETADATANVPROC glad_eglSetStreamMetadataNV = NULL;
PFNEGLSIGNALSYNCKHRPROC glad_eglSignalSyncKHR = NULL;
PFNEGLSIGNALSYNCNVPROC glad_eglSignalSyncNV = NULL;
PFNEGLSTREAMATTRIBKHRPROC glad_eglStreamAttribKHR = NULL;
PFNEGLSTREAMCONSUMERACQUIREATTRIBKHRPROC glad_eglStreamConsumerAcquireAttribKHR = NULL;
PFNEGLSTREAMCONSUMERACQUIREKHRPROC glad_eglStreamConsumerAcquireKHR = NULL;
PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALATTRIBSNVPROC glad_eglStreamConsumerGLTextureExternalAttribsNV = NULL;
PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALKHRPROC glad_eglStreamConsumerGLTextureExternalKHR = NULL;
PFNEGLSTREAMCONSUMEROUTPUTEXTPROC glad_eglStreamConsumerOutputEXT = NULL;
PFNEGLSTREAMCONSUMERRELEASEATTRIBKHRPROC glad_eglStreamConsumerReleaseAttribKHR = NULL;
PFNEGLSTREAMCONSUMERRELEASEKHRPROC glad_eglStreamConsumerReleaseKHR = NULL;
PFNEGLSTREAMFLUSHNVPROC glad_eglStreamFlushNV = NULL;
PFNEGLSURFACEATTRIBPROC glad_eglSurfaceAttrib = NULL;
PFNEGLSWAPBUFFERSPROC glad_eglSwapBuffers = NULL;
PFNEGLSWAPBUFFERSREGION2NOKPROC glad_eglSwapBuffersRegion2NOK = NULL;
PFNEGLSWAPBUFFERSREGIONNOKPROC glad_eglSwapBuffersRegionNOK = NULL;
PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC glad_eglSwapBuffersWithDamageEXT = NULL;
PFNEGLSWAPBUFFERSWITHDAMAGEKHRPROC glad_eglSwapBuffersWithDamageKHR = NULL;
PFNEGLSWAPINTERVALPROC glad_eglSwapInterval = NULL;
PFNEGLTERMINATEPROC glad_eglTerminate = NULL;
PFNEGLUNLOCKSURFACEKHRPROC glad_eglUnlockSurfaceKHR = NULL;
PFNEGLUNSIGNALSYNCEXTPROC glad_eglUnsignalSyncEXT = NULL;
PFNEGLWAITCLIENTPROC glad_eglWaitClient = NULL;
PFNEGLWAITGLPROC glad_eglWaitGL = NULL;
PFNEGLWAITNATIVEPROC glad_eglWaitNative = NULL;
PFNEGLWAITSYNCPROC glad_eglWaitSync = NULL;
PFNEGLWAITSYNCKHRPROC glad_eglWaitSyncKHR = NULL;


static void glad_egl_load_EGL_VERSION_1_0( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_VERSION_1_0) return;
    eglChooseConfig = (PFNEGLCHOOSECONFIGPROC) load("eglChooseConfig", userptr);
    eglCopyBuffers = (PFNEGLCOPYBUFFERSPROC) load("eglCopyBuffers", userptr);
    eglCreateContext = (PFNEGLCREATECONTEXTPROC) load("eglCreateContext", userptr);
    eglCreatePbufferSurface = (PFNEGLCREATEPBUFFERSURFACEPROC) load("eglCreatePbufferSurface", userptr);
    eglCreatePixmapSurface = (PFNEGLCREATEPIXMAPSURFACEPROC) load("eglCreatePixmapSurface", userptr);
    eglCreateWindowSurface = (PFNEGLCREATEWINDOWSURFACEPROC) load("eglCreateWindowSurface", userptr);
    eglDestroyContext = (PFNEGLDESTROYCONTEXTPROC) load("eglDestroyContext", userptr);
    eglDestroySurface = (PFNEGLDESTROYSURFACEPROC) load("eglDestroySurface", userptr);
    eglGetConfigAttrib = (PFNEGLGETCONFIGATTRIBPROC) load("eglGetConfigAttrib", userptr);
    eglGetConfigs = (PFNEGLGETCONFIGSPROC) load("eglGetConfigs", userptr);
    eglGetCurrentDisplay = (PFNEGLGETCURRENTDISPLAYPROC) load("eglGetCurrentDisplay", userptr);
    eglGetCurrentSurface = (PFNEGLGETCURRENTSURFACEPROC) load("eglGetCurrentSurface", userptr);
    eglGetDisplay = (PFNEGLGETDISPLAYPROC) load("eglGetDisplay", userptr);
    eglGetError = (PFNEGLGETERRORPROC) load("eglGetError", userptr);
    eglGetProcAddress = (PFNEGLGETPROCADDRESSPROC) load("eglGetProcAddress", userptr);
    eglInitialize = (PFNEGLINITIALIZEPROC) load("eglInitialize", userptr);
    eglMakeCurrent = (PFNEGLMAKECURRENTPROC) load("eglMakeCurrent", userptr);
    eglQueryContext = (PFNEGLQUERYCONTEXTPROC) load("eglQueryContext", userptr);
    eglQueryString = (PFNEGLQUERYSTRINGPROC) load("eglQueryString", userptr);
    eglQuerySurface = (PFNEGLQUERYSURFACEPROC) load("eglQuerySurface", userptr);
    eglSwapBuffers = (PFNEGLSWAPBUFFERSPROC) load("eglSwapBuffers", userptr);
    eglTerminate = (PFNEGLTERMINATEPROC) load("eglTerminate", userptr);
    eglWaitGL = (PFNEGLWAITGLPROC) load("eglWaitGL", userptr);
    eglWaitNative = (PFNEGLWAITNATIVEPROC) load("eglWaitNative", userptr);
}
static void glad_egl_load_EGL_VERSION_1_1( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_VERSION_1_1) return;
    eglBindTexImage = (PFNEGLBINDTEXIMAGEPROC) load("eglBindTexImage", userptr);
    eglReleaseTexImage = (PFNEGLRELEASETEXIMAGEPROC) load("eglReleaseTexImage", userptr);
    eglSurfaceAttrib = (PFNEGLSURFACEATTRIBPROC) load("eglSurfaceAttrib", userptr);
    eglSwapInterval = (PFNEGLSWAPINTERVALPROC) load("eglSwapInterval", userptr);
}
static void glad_egl_load_EGL_VERSION_1_2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_VERSION_1_2) return;
    eglBindAPI = (PFNEGLBINDAPIPROC) load("eglBindAPI", userptr);
    eglCreatePbufferFromClientBuffer = (PFNEGLCREATEPBUFFERFROMCLIENTBUFFERPROC) load("eglCreatePbufferFromClientBuffer", userptr);
    eglQueryAPI = (PFNEGLQUERYAPIPROC) load("eglQueryAPI", userptr);
    eglReleaseThread = (PFNEGLRELEASETHREADPROC) load("eglReleaseThread", userptr);
    eglWaitClient = (PFNEGLWAITCLIENTPROC) load("eglWaitClient", userptr);
}
static void glad_egl_load_EGL_VERSION_1_4( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_VERSION_1_4) return;
    eglGetCurrentContext = (PFNEGLGETCURRENTCONTEXTPROC) load("eglGetCurrentContext", userptr);
}
static void glad_egl_load_EGL_VERSION_1_5( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_VERSION_1_5) return;
    eglClientWaitSync = (PFNEGLCLIENTWAITSYNCPROC) load("eglClientWaitSync", userptr);
    eglCreateImage = (PFNEGLCREATEIMAGEPROC) load("eglCreateImage", userptr);
    eglCreatePlatformPixmapSurface = (PFNEGLCREATEPLATFORMPIXMAPSURFACEPROC) load("eglCreatePlatformPixmapSurface", userptr);
    eglCreatePlatformWindowSurface = (PFNEGLCREATEPLATFORMWINDOWSURFACEPROC) load("eglCreatePlatformWindowSurface", userptr);
    eglCreateSync = (PFNEGLCREATESYNCPROC) load("eglCreateSync", userptr);
    eglDestroyImage = (PFNEGLDESTROYIMAGEPROC) load("eglDestroyImage", userptr);
    eglDestroySync = (PFNEGLDESTROYSYNCPROC) load("eglDestroySync", userptr);
    eglGetPlatformDisplay = (PFNEGLGETPLATFORMDISPLAYPROC) load("eglGetPlatformDisplay", userptr);
    eglGetSyncAttrib = (PFNEGLGETSYNCATTRIBPROC) load("eglGetSyncAttrib", userptr);
    eglWaitSync = (PFNEGLWAITSYNCPROC) load("eglWaitSync", userptr);
}
static void glad_egl_load_EGL_ANDROID_blob_cache( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_ANDROID_blob_cache) return;
    eglSetBlobCacheFuncsANDROID = (PFNEGLSETBLOBCACHEFUNCSANDROIDPROC) load("eglSetBlobCacheFuncsANDROID", userptr);
}
static void glad_egl_load_EGL_ANDROID_create_native_client_buffer( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_ANDROID_create_native_client_buffer) return;
    eglCreateNativeClientBufferANDROID = (PFNEGLCREATENATIVECLIENTBUFFERANDROIDPROC) load("eglCreateNativeClientBufferANDROID", userptr);
}
static void glad_egl_load_EGL_ANDROID_get_frame_timestamps( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_ANDROID_get_frame_timestamps) return;
    eglGetCompositorTimingANDROID = (PFNEGLGETCOMPOSITORTIMINGANDROIDPROC) load("eglGetCompositorTimingANDROID", userptr);
    eglGetCompositorTimingSupportedANDROID = (PFNEGLGETCOMPOSITORTIMINGSUPPORTEDANDROIDPROC) load("eglGetCompositorTimingSupportedANDROID", userptr);
    eglGetFrameTimestampSupportedANDROID = (PFNEGLGETFRAMETIMESTAMPSUPPORTEDANDROIDPROC) load("eglGetFrameTimestampSupportedANDROID", userptr);
    eglGetFrameTimestampsANDROID = (PFNEGLGETFRAMETIMESTAMPSANDROIDPROC) load("eglGetFrameTimestampsANDROID", userptr);
    eglGetNextFrameIdANDROID = (PFNEGLGETNEXTFRAMEIDANDROIDPROC) load("eglGetNextFrameIdANDROID", userptr);
}
static void glad_egl_load_EGL_ANDROID_get_native_client_buffer( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_ANDROID_get_native_client_buffer) return;
    eglGetNativeClientBufferANDROID = (PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC) load("eglGetNativeClientBufferANDROID", userptr);
}
static void glad_egl_load_EGL_ANDROID_native_fence_sync( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_ANDROID_native_fence_sync) return;
    eglDupNativeFenceFDANDROID = (PFNEGLDUPNATIVEFENCEFDANDROIDPROC) load("eglDupNativeFenceFDANDROID", userptr);
}
static void glad_egl_load_EGL_ANDROID_presentation_time( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_ANDROID_presentation_time) return;
    eglPresentationTimeANDROID = (PFNEGLPRESENTATIONTIMEANDROIDPROC) load("eglPresentationTimeANDROID", userptr);
}
static void glad_egl_load_EGL_ANGLE_query_surface_pointer( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_ANGLE_query_surface_pointer) return;
    eglQuerySurfacePointerANGLE = (PFNEGLQUERYSURFACEPOINTERANGLEPROC) load("eglQuerySurfacePointerANGLE", userptr);
}
static void glad_egl_load_EGL_EXT_client_sync( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_EXT_client_sync) return;
    eglClientSignalSyncEXT = (PFNEGLCLIENTSIGNALSYNCEXTPROC) load("eglClientSignalSyncEXT", userptr);
}
static void glad_egl_load_EGL_EXT_compositor( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_EXT_compositor) return;
    eglCompositorBindTexWindowEXT = (PFNEGLCOMPOSITORBINDTEXWINDOWEXTPROC) load("eglCompositorBindTexWindowEXT", userptr);
    eglCompositorSetContextAttributesEXT = (PFNEGLCOMPOSITORSETCONTEXTATTRIBUTESEXTPROC) load("eglCompositorSetContextAttributesEXT", userptr);
    eglCompositorSetContextListEXT = (PFNEGLCOMPOSITORSETCONTEXTLISTEXTPROC) load("eglCompositorSetContextListEXT", userptr);
    eglCompositorSetSizeEXT = (PFNEGLCOMPOSITORSETSIZEEXTPROC) load("eglCompositorSetSizeEXT", userptr);
    eglCompositorSetWindowAttributesEXT = (PFNEGLCOMPOSITORSETWINDOWATTRIBUTESEXTPROC) load("eglCompositorSetWindowAttributesEXT", userptr);
    eglCompositorSetWindowListEXT = (PFNEGLCOMPOSITORSETWINDOWLISTEXTPROC) load("eglCompositorSetWindowListEXT", userptr);
    eglCompositorSwapPolicyEXT = (PFNEGLCOMPOSITORSWAPPOLICYEXTPROC) load("eglCompositorSwapPolicyEXT", userptr);
}
static void glad_egl_load_EGL_EXT_device_base( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_EXT_device_base) return;
    eglQueryDeviceAttribEXT = (PFNEGLQUERYDEVICEATTRIBEXTPROC) load("eglQueryDeviceAttribEXT", userptr);
    eglQueryDeviceStringEXT = (PFNEGLQUERYDEVICESTRINGEXTPROC) load("eglQueryDeviceStringEXT", userptr);
    eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC) load("eglQueryDevicesEXT", userptr);
    eglQueryDisplayAttribEXT = (PFNEGLQUERYDISPLAYATTRIBEXTPROC) load("eglQueryDisplayAttribEXT", userptr);
    eglQueryDisplayAttribKHR = (PFNEGLQUERYDISPLAYATTRIBKHRPROC) load("eglQueryDisplayAttribKHR", userptr);
}
static void glad_egl_load_EGL_EXT_device_enumeration( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_EXT_device_enumeration) return;
    eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC) load("eglQueryDevicesEXT", userptr);
}
static void glad_egl_load_EGL_EXT_device_query( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_EXT_device_query) return;
    eglQueryDeviceAttribEXT = (PFNEGLQUERYDEVICEATTRIBEXTPROC) load("eglQueryDeviceAttribEXT", userptr);
    eglQueryDeviceStringEXT = (PFNEGLQUERYDEVICESTRINGEXTPROC) load("eglQueryDeviceStringEXT", userptr);
    eglQueryDisplayAttribEXT = (PFNEGLQUERYDISPLAYATTRIBEXTPROC) load("eglQueryDisplayAttribEXT", userptr);
    eglQueryDisplayAttribKHR = (PFNEGLQUERYDISPLAYATTRIBKHRPROC) load("eglQueryDisplayAttribKHR", userptr);
}
static void glad_egl_load_EGL_EXT_image_dma_buf_import_modifiers( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_EXT_image_dma_buf_import_modifiers) return;
    eglQueryDmaBufFormatsEXT = (PFNEGLQUERYDMABUFFORMATSEXTPROC) load("eglQueryDmaBufFormatsEXT", userptr);
    eglQueryDmaBufModifiersEXT = (PFNEGLQUERYDMABUFMODIFIERSEXTPROC) load("eglQueryDmaBufModifiersEXT", userptr);
}
static void glad_egl_load_EGL_EXT_output_base( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_EXT_output_base) return;
    eglGetOutputLayersEXT = (PFNEGLGETOUTPUTLAYERSEXTPROC) load("eglGetOutputLayersEXT", userptr);
    eglGetOutputPortsEXT = (PFNEGLGETOUTPUTPORTSEXTPROC) load("eglGetOutputPortsEXT", userptr);
    eglOutputLayerAttribEXT = (PFNEGLOUTPUTLAYERATTRIBEXTPROC) load("eglOutputLayerAttribEXT", userptr);
    eglOutputPortAttribEXT = (PFNEGLOUTPUTPORTATTRIBEXTPROC) load("eglOutputPortAttribEXT", userptr);
    eglQueryOutputLayerAttribEXT = (PFNEGLQUERYOUTPUTLAYERATTRIBEXTPROC) load("eglQueryOutputLayerAttribEXT", userptr);
    eglQueryOutputLayerStringEXT = (PFNEGLQUERYOUTPUTLAYERSTRINGEXTPROC) load("eglQueryOutputLayerStringEXT", userptr);
    eglQueryOutputPortAttribEXT = (PFNEGLQUERYOUTPUTPORTATTRIBEXTPROC) load("eglQueryOutputPortAttribEXT", userptr);
    eglQueryOutputPortStringEXT = (PFNEGLQUERYOUTPUTPORTSTRINGEXTPROC) load("eglQueryOutputPortStringEXT", userptr);
}
static void glad_egl_load_EGL_EXT_platform_base( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_EXT_platform_base) return;
    eglCreatePlatformPixmapSurfaceEXT = (PFNEGLCREATEPLATFORMPIXMAPSURFACEEXTPROC) load("eglCreatePlatformPixmapSurfaceEXT", userptr);
    eglCreatePlatformWindowSurfaceEXT = (PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC) load("eglCreatePlatformWindowSurfaceEXT", userptr);
    eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC) load("eglGetPlatformDisplayEXT", userptr);
}
static void glad_egl_load_EGL_EXT_stream_consumer_egloutput( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_EXT_stream_consumer_egloutput) return;
    eglStreamConsumerOutputEXT = (PFNEGLSTREAMCONSUMEROUTPUTEXTPROC) load("eglStreamConsumerOutputEXT", userptr);
}
static void glad_egl_load_EGL_EXT_swap_buffers_with_damage( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_EXT_swap_buffers_with_damage) return;
    eglSwapBuffersWithDamageEXT = (PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC) load("eglSwapBuffersWithDamageEXT", userptr);
}
static void glad_egl_load_EGL_EXT_sync_reuse( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_EXT_sync_reuse) return;
    eglUnsignalSyncEXT = (PFNEGLUNSIGNALSYNCEXTPROC) load("eglUnsignalSyncEXT", userptr);
}
static void glad_egl_load_EGL_HI_clientpixmap( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_HI_clientpixmap) return;
    eglCreatePixmapSurfaceHI = (PFNEGLCREATEPIXMAPSURFACEHIPROC) load("eglCreatePixmapSurfaceHI", userptr);
}
static void glad_egl_load_EGL_KHR_cl_event2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_KHR_cl_event2) return;
    eglCreateSync = (PFNEGLCREATESYNCPROC) load("eglCreateSync", userptr);
    eglCreateSync64KHR = (PFNEGLCREATESYNC64KHRPROC) load("eglCreateSync64KHR", userptr);
}
static void glad_egl_load_EGL_KHR_debug( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_KHR_debug) return;
    eglDebugMessageControlKHR = (PFNEGLDEBUGMESSAGECONTROLKHRPROC) load("eglDebugMessageControlKHR", userptr);
    eglLabelObjectKHR = (PFNEGLLABELOBJECTKHRPROC) load("eglLabelObjectKHR", userptr);
    eglQueryDebugKHR = (PFNEGLQUERYDEBUGKHRPROC) load("eglQueryDebugKHR", userptr);
}
static void glad_egl_load_EGL_KHR_display_reference( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_KHR_display_reference) return;
    eglQueryDisplayAttribKHR = (PFNEGLQUERYDISPLAYATTRIBKHRPROC) load("eglQueryDisplayAttribKHR", userptr);
}
static void glad_egl_load_EGL_KHR_fence_sync( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_KHR_fence_sync) return;
    eglClientWaitSync = (PFNEGLCLIENTWAITSYNCPROC) load("eglClientWaitSync", userptr);
    eglClientWaitSyncKHR = (PFNEGLCLIENTWAITSYNCKHRPROC) load("eglClientWaitSyncKHR", userptr);
    eglCreateSyncKHR = (PFNEGLCREATESYNCKHRPROC) load("eglCreateSyncKHR", userptr);
    eglDestroySync = (PFNEGLDESTROYSYNCPROC) load("eglDestroySync", userptr);
    eglDestroySyncKHR = (PFNEGLDESTROYSYNCKHRPROC) load("eglDestroySyncKHR", userptr);
    eglGetSyncAttribKHR = (PFNEGLGETSYNCATTRIBKHRPROC) load("eglGetSyncAttribKHR", userptr);
}
static void glad_egl_load_EGL_KHR_image( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_KHR_image) return;
    eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) load("eglCreateImageKHR", userptr);
    eglDestroyImage = (PFNEGLDESTROYIMAGEPROC) load("eglDestroyImage", userptr);
    eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC) load("eglDestroyImageKHR", userptr);
}
static void glad_egl_load_EGL_KHR_image_base( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_KHR_image_base) return;
    eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) load("eglCreateImageKHR", userptr);
    eglDestroyImage = (PFNEGLDESTROYIMAGEPROC) load("eglDestroyImage", userptr);
    eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC) load("eglDestroyImageKHR", userptr);
}
static void glad_egl_load_EGL_KHR_lock_surface( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_KHR_lock_surface) return;
    eglLockSurfaceKHR = (PFNEGLLOCKSURFACEKHRPROC) load("eglLockSurfaceKHR", userptr);
    eglUnlockSurfaceKHR = (PFNEGLUNLOCKSURFACEKHRPROC) load("eglUnlockSurfaceKHR", userptr);
}
static void glad_egl_load_EGL_KHR_lock_surface3( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_KHR_lock_surface3) return;
    eglLockSurfaceKHR = (PFNEGLLOCKSURFACEKHRPROC) load("eglLockSurfaceKHR", userptr);
    eglQuerySurface64KHR = (PFNEGLQUERYSURFACE64KHRPROC) load("eglQuerySurface64KHR", userptr);
    eglUnlockSurfaceKHR = (PFNEGLUNLOCKSURFACEKHRPROC) load("eglUnlockSurfaceKHR", userptr);
}
static void glad_egl_load_EGL_KHR_partial_update( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_KHR_partial_update) return;
    eglSetDamageRegionKHR = (PFNEGLSETDAMAGEREGIONKHRPROC) load("eglSetDamageRegionKHR", userptr);
}
static void glad_egl_load_EGL_KHR_reusable_sync( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_KHR_reusable_sync) return;
    eglClientWaitSync = (PFNEGLCLIENTWAITSYNCPROC) load("eglClientWaitSync", userptr);
    eglClientWaitSyncKHR = (PFNEGLCLIENTWAITSYNCKHRPROC) load("eglClientWaitSyncKHR", userptr);
    eglCreateSyncKHR = (PFNEGLCREATESYNCKHRPROC) load("eglCreateSyncKHR", userptr);
    eglDestroySync = (PFNEGLDESTROYSYNCPROC) load("eglDestroySync", userptr);
    eglDestroySyncKHR = (PFNEGLDESTROYSYNCKHRPROC) load("eglDestroySyncKHR", userptr);
    eglGetSyncAttribKHR = (PFNEGLGETSYNCATTRIBKHRPROC) load("eglGetSyncAttribKHR", userptr);
    eglSignalSyncKHR = (PFNEGLSIGNALSYNCKHRPROC) load("eglSignalSyncKHR", userptr);
}
static void glad_egl_load_EGL_KHR_stream( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_KHR_stream) return;
    eglCreateStreamKHR = (PFNEGLCREATESTREAMKHRPROC) load("eglCreateStreamKHR", userptr);
    eglDestroyStreamKHR = (PFNEGLDESTROYSTREAMKHRPROC) load("eglDestroyStreamKHR", userptr);
    eglQueryStreamKHR = (PFNEGLQUERYSTREAMKHRPROC) load("eglQueryStreamKHR", userptr);
    eglQueryStreamu64KHR = (PFNEGLQUERYSTREAMU64KHRPROC) load("eglQueryStreamu64KHR", userptr);
    eglStreamAttribKHR = (PFNEGLSTREAMATTRIBKHRPROC) load("eglStreamAttribKHR", userptr);
}
static void glad_egl_load_EGL_KHR_stream_attrib( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_KHR_stream_attrib) return;
    eglCreateStreamAttribKHR = (PFNEGLCREATESTREAMATTRIBKHRPROC) load("eglCreateStreamAttribKHR", userptr);
    eglQueryStreamAttribKHR = (PFNEGLQUERYSTREAMATTRIBKHRPROC) load("eglQueryStreamAttribKHR", userptr);
    eglSetStreamAttribKHR = (PFNEGLSETSTREAMATTRIBKHRPROC) load("eglSetStreamAttribKHR", userptr);
    eglStreamConsumerAcquireAttribKHR = (PFNEGLSTREAMCONSUMERACQUIREATTRIBKHRPROC) load("eglStreamConsumerAcquireAttribKHR", userptr);
    eglStreamConsumerReleaseAttribKHR = (PFNEGLSTREAMCONSUMERRELEASEATTRIBKHRPROC) load("eglStreamConsumerReleaseAttribKHR", userptr);
}
static void glad_egl_load_EGL_KHR_stream_consumer_gltexture( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_KHR_stream_consumer_gltexture) return;
    eglStreamConsumerAcquireKHR = (PFNEGLSTREAMCONSUMERACQUIREKHRPROC) load("eglStreamConsumerAcquireKHR", userptr);
    eglStreamConsumerGLTextureExternalKHR = (PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALKHRPROC) load("eglStreamConsumerGLTextureExternalKHR", userptr);
    eglStreamConsumerReleaseKHR = (PFNEGLSTREAMCONSUMERRELEASEKHRPROC) load("eglStreamConsumerReleaseKHR", userptr);
}
static void glad_egl_load_EGL_KHR_stream_cross_process_fd( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_KHR_stream_cross_process_fd) return;
    eglCreateStreamFromFileDescriptorKHR = (PFNEGLCREATESTREAMFROMFILEDESCRIPTORKHRPROC) load("eglCreateStreamFromFileDescriptorKHR", userptr);
    eglGetStreamFileDescriptorKHR = (PFNEGLGETSTREAMFILEDESCRIPTORKHRPROC) load("eglGetStreamFileDescriptorKHR", userptr);
}
static void glad_egl_load_EGL_KHR_stream_fifo( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_KHR_stream_fifo) return;
    eglQueryStreamTimeKHR = (PFNEGLQUERYSTREAMTIMEKHRPROC) load("eglQueryStreamTimeKHR", userptr);
}
static void glad_egl_load_EGL_KHR_stream_producer_eglsurface( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_KHR_stream_producer_eglsurface) return;
    eglCreateStreamProducerSurfaceKHR = (PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC) load("eglCreateStreamProducerSurfaceKHR", userptr);
}
static void glad_egl_load_EGL_KHR_swap_buffers_with_damage( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_KHR_swap_buffers_with_damage) return;
    eglSwapBuffersWithDamageKHR = (PFNEGLSWAPBUFFERSWITHDAMAGEKHRPROC) load("eglSwapBuffersWithDamageKHR", userptr);
}
static void glad_egl_load_EGL_KHR_wait_sync( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_KHR_wait_sync) return;
    eglWaitSyncKHR = (PFNEGLWAITSYNCKHRPROC) load("eglWaitSyncKHR", userptr);
}
static void glad_egl_load_EGL_MESA_drm_image( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_MESA_drm_image) return;
    eglCreateDRMImageMESA = (PFNEGLCREATEDRMIMAGEMESAPROC) load("eglCreateDRMImageMESA", userptr);
    eglExportDRMImageMESA = (PFNEGLEXPORTDRMIMAGEMESAPROC) load("eglExportDRMImageMESA", userptr);
}
static void glad_egl_load_EGL_MESA_image_dma_buf_export( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_MESA_image_dma_buf_export) return;
    eglExportDMABUFImageMESA = (PFNEGLEXPORTDMABUFIMAGEMESAPROC) load("eglExportDMABUFImageMESA", userptr);
    eglExportDMABUFImageQueryMESA = (PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC) load("eglExportDMABUFImageQueryMESA", userptr);
}
static void glad_egl_load_EGL_NOK_swap_region( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_NOK_swap_region) return;
    eglSwapBuffersRegionNOK = (PFNEGLSWAPBUFFERSREGIONNOKPROC) load("eglSwapBuffersRegionNOK", userptr);
}
static void glad_egl_load_EGL_NOK_swap_region2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_NOK_swap_region2) return;
    eglSwapBuffersRegion2NOK = (PFNEGLSWAPBUFFERSREGION2NOKPROC) load("eglSwapBuffersRegion2NOK", userptr);
}
static void glad_egl_load_EGL_NV_native_query( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_NV_native_query) return;
    eglQueryNativeDisplayNV = (PFNEGLQUERYNATIVEDISPLAYNVPROC) load("eglQueryNativeDisplayNV", userptr);
    eglQueryNativePixmapNV = (PFNEGLQUERYNATIVEPIXMAPNVPROC) load("eglQueryNativePixmapNV", userptr);
    eglQueryNativeWindowNV = (PFNEGLQUERYNATIVEWINDOWNVPROC) load("eglQueryNativeWindowNV", userptr);
}
static void glad_egl_load_EGL_NV_post_sub_buffer( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_NV_post_sub_buffer) return;
    eglPostSubBufferNV = (PFNEGLPOSTSUBBUFFERNVPROC) load("eglPostSubBufferNV", userptr);
}
static void glad_egl_load_EGL_NV_stream_consumer_gltexture_yuv( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_NV_stream_consumer_gltexture_yuv) return;
    eglStreamConsumerGLTextureExternalAttribsNV = (PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALATTRIBSNVPROC) load("eglStreamConsumerGLTextureExternalAttribsNV", userptr);
}
static void glad_egl_load_EGL_NV_stream_flush( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_NV_stream_flush) return;
    eglStreamFlushNV = (PFNEGLSTREAMFLUSHNVPROC) load("eglStreamFlushNV", userptr);
}
static void glad_egl_load_EGL_NV_stream_metadata( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_NV_stream_metadata) return;
    eglQueryDisplayAttribKHR = (PFNEGLQUERYDISPLAYATTRIBKHRPROC) load("eglQueryDisplayAttribKHR", userptr);
    eglQueryDisplayAttribNV = (PFNEGLQUERYDISPLAYATTRIBNVPROC) load("eglQueryDisplayAttribNV", userptr);
    eglQueryStreamMetadataNV = (PFNEGLQUERYSTREAMMETADATANVPROC) load("eglQueryStreamMetadataNV", userptr);
    eglSetStreamMetadataNV = (PFNEGLSETSTREAMMETADATANVPROC) load("eglSetStreamMetadataNV", userptr);
}
static void glad_egl_load_EGL_NV_stream_reset( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_NV_stream_reset) return;
    eglResetStreamNV = (PFNEGLRESETSTREAMNVPROC) load("eglResetStreamNV", userptr);
}
static void glad_egl_load_EGL_NV_stream_sync( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_NV_stream_sync) return;
    eglCreateStreamSyncNV = (PFNEGLCREATESTREAMSYNCNVPROC) load("eglCreateStreamSyncNV", userptr);
}
static void glad_egl_load_EGL_NV_sync( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_NV_sync) return;
    eglClientWaitSyncNV = (PFNEGLCLIENTWAITSYNCNVPROC) load("eglClientWaitSyncNV", userptr);
    eglCreateFenceSyncNV = (PFNEGLCREATEFENCESYNCNVPROC) load("eglCreateFenceSyncNV", userptr);
    eglDestroySyncNV = (PFNEGLDESTROYSYNCNVPROC) load("eglDestroySyncNV", userptr);
    eglFenceNV = (PFNEGLFENCENVPROC) load("eglFenceNV", userptr);
    eglGetSyncAttribNV = (PFNEGLGETSYNCATTRIBNVPROC) load("eglGetSyncAttribNV", userptr);
    eglSignalSyncNV = (PFNEGLSIGNALSYNCNVPROC) load("eglSignalSyncNV", userptr);
}
static void glad_egl_load_EGL_NV_system_time( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_EGL_NV_system_time) return;
    eglGetSystemTimeFrequencyNV = (PFNEGLGETSYSTEMTIMEFREQUENCYNVPROC) load("eglGetSystemTimeFrequencyNV", userptr);
    eglGetSystemTimeNV = (PFNEGLGETSYSTEMTIMENVPROC) load("eglGetSystemTimeNV", userptr);
}



static int glad_egl_get_extensions(EGLDisplay display, const char **extensions) {
    *extensions = eglQueryString(display, EGL_EXTENSIONS);

    return extensions != NULL;
}

static int glad_egl_has_extension(const char *extensions, const char *ext) {
    const char *loc;
    const char *terminator;
    if(extensions == NULL) {
        return 0;
    }
    while(1) {
        loc = strstr(extensions, ext);
        if(loc == NULL) {
            return 0;
        }
        terminator = loc + strlen(ext);
        if((loc == extensions || *(loc - 1) == ' ') &&
            (*terminator == ' ' || *terminator == '\0')) {
            return 1;
        }
        extensions = terminator;
    }
}

static GLADapiproc glad_egl_get_proc_from_userptr(const char *name, void *userptr) {
    return (GLAD_GNUC_EXTENSION (GLADapiproc (*)(const char *name)) userptr)(name);
}

static int glad_egl_find_extensions_egl(EGLDisplay display) {
    const char *extensions;
    if (!glad_egl_get_extensions(display, &extensions)) return 0;

    GLAD_EGL_ANDROID_blob_cache = glad_egl_has_extension(extensions, "EGL_ANDROID_blob_cache");
    GLAD_EGL_ANDROID_create_native_client_buffer = glad_egl_has_extension(extensions, "EGL_ANDROID_create_native_client_buffer");
    GLAD_EGL_ANDROID_framebuffer_target = glad_egl_has_extension(extensions, "EGL_ANDROID_framebuffer_target");
    GLAD_EGL_ANDROID_front_buffer_auto_refresh = glad_egl_has_extension(extensions, "EGL_ANDROID_front_buffer_auto_refresh");
    GLAD_EGL_ANDROID_get_frame_timestamps = glad_egl_has_extension(extensions, "EGL_ANDROID_get_frame_timestamps");
    GLAD_EGL_ANDROID_get_native_client_buffer = glad_egl_has_extension(extensions, "EGL_ANDROID_get_native_client_buffer");
    GLAD_EGL_ANDROID_image_native_buffer = glad_egl_has_extension(extensions, "EGL_ANDROID_image_native_buffer");
    GLAD_EGL_ANDROID_native_fence_sync = glad_egl_has_extension(extensions, "EGL_ANDROID_native_fence_sync");
    GLAD_EGL_ANDROID_presentation_time = glad_egl_has_extension(extensions, "EGL_ANDROID_presentation_time");
    GLAD_EGL_ANDROID_recordable = glad_egl_has_extension(extensions, "EGL_ANDROID_recordable");
    GLAD_EGL_ANGLE_d3d_share_handle_client_buffer = glad_egl_has_extension(extensions, "EGL_ANGLE_d3d_share_handle_client_buffer");
    GLAD_EGL_ANGLE_device_d3d = glad_egl_has_extension(extensions, "EGL_ANGLE_device_d3d");
    GLAD_EGL_ANGLE_query_surface_pointer = glad_egl_has_extension(extensions, "EGL_ANGLE_query_surface_pointer");
    GLAD_EGL_ANGLE_surface_d3d_texture_2d_share_handle = glad_egl_has_extension(extensions, "EGL_ANGLE_surface_d3d_texture_2d_share_handle");
    GLAD_EGL_ANGLE_window_fixed_size = glad_egl_has_extension(extensions, "EGL_ANGLE_window_fixed_size");
    GLAD_EGL_ARM_implicit_external_sync = glad_egl_has_extension(extensions, "EGL_ARM_implicit_external_sync");
    GLAD_EGL_ARM_pixmap_multisample_discard = glad_egl_has_extension(extensions, "EGL_ARM_pixmap_multisample_discard");
    GLAD_EGL_EXT_bind_to_front = glad_egl_has_extension(extensions, "EGL_EXT_bind_to_front");
    GLAD_EGL_EXT_buffer_age = glad_egl_has_extension(extensions, "EGL_EXT_buffer_age");
    GLAD_EGL_EXT_client_extensions = glad_egl_has_extension(extensions, "EGL_EXT_client_extensions");
    GLAD_EGL_EXT_client_sync = glad_egl_has_extension(extensions, "EGL_EXT_client_sync");
    GLAD_EGL_EXT_compositor = glad_egl_has_extension(extensions, "EGL_EXT_compositor");
    GLAD_EGL_EXT_create_context_robustness = glad_egl_has_extension(extensions, "EGL_EXT_create_context_robustness");
    GLAD_EGL_EXT_device_base = glad_egl_has_extension(extensions, "EGL_EXT_device_base");
    GLAD_EGL_EXT_device_drm = glad_egl_has_extension(extensions, "EGL_EXT_device_drm");
    GLAD_EGL_EXT_device_enumeration = glad_egl_has_extension(extensions, "EGL_EXT_device_enumeration");
    GLAD_EGL_EXT_device_openwf = glad_egl_has_extension(extensions, "EGL_EXT_device_openwf");
    GLAD_EGL_EXT_device_query = glad_egl_has_extension(extensions, "EGL_EXT_device_query");
    GLAD_EGL_EXT_gl_colorspace_bt2020_linear = glad_egl_has_extension(extensions, "EGL_EXT_gl_colorspace_bt2020_linear");
    GLAD_EGL_EXT_gl_colorspace_bt2020_pq = glad_egl_has_extension(extensions, "EGL_EXT_gl_colorspace_bt2020_pq");
    GLAD_EGL_EXT_gl_colorspace_display_p3 = glad_egl_has_extension(extensions, "EGL_EXT_gl_colorspace_display_p3");
    GLAD_EGL_EXT_gl_colorspace_display_p3_linear = glad_egl_has_extension(extensions, "EGL_EXT_gl_colorspace_display_p3_linear");
    GLAD_EGL_EXT_gl_colorspace_scrgb = glad_egl_has_extension(extensions, "EGL_EXT_gl_colorspace_scrgb");
    GLAD_EGL_EXT_gl_colorspace_scrgb_linear = glad_egl_has_extension(extensions, "EGL_EXT_gl_colorspace_scrgb_linear");
    GLAD_EGL_EXT_image_dma_buf_import = glad_egl_has_extension(extensions, "EGL_EXT_image_dma_buf_import");
    GLAD_EGL_EXT_image_dma_buf_import_modifiers = glad_egl_has_extension(extensions, "EGL_EXT_image_dma_buf_import_modifiers");
    GLAD_EGL_EXT_image_gl_colorspace = glad_egl_has_extension(extensions, "EGL_EXT_image_gl_colorspace");
    GLAD_EGL_EXT_image_implicit_sync_control = glad_egl_has_extension(extensions, "EGL_EXT_image_implicit_sync_control");
    GLAD_EGL_EXT_multiview_window = glad_egl_has_extension(extensions, "EGL_EXT_multiview_window");
    GLAD_EGL_EXT_output_base = glad_egl_has_extension(extensions, "EGL_EXT_output_base");
    GLAD_EGL_EXT_output_drm = glad_egl_has_extension(extensions, "EGL_EXT_output_drm");
    GLAD_EGL_EXT_output_openwf = glad_egl_has_extension(extensions, "EGL_EXT_output_openwf");
    GLAD_EGL_EXT_pixel_format_float = glad_egl_has_extension(extensions, "EGL_EXT_pixel_format_float");
    GLAD_EGL_EXT_platform_base = glad_egl_has_extension(extensions, "EGL_EXT_platform_base");
    GLAD_EGL_EXT_platform_device = glad_egl_has_extension(extensions, "EGL_EXT_platform_device");
    GLAD_EGL_EXT_platform_wayland = glad_egl_has_extension(extensions, "EGL_EXT_platform_wayland");
    GLAD_EGL_EXT_platform_x11 = glad_egl_has_extension(extensions, "EGL_EXT_platform_x11");
    GLAD_EGL_EXT_protected_content = glad_egl_has_extension(extensions, "EGL_EXT_protected_content");
    GLAD_EGL_EXT_protected_surface = glad_egl_has_extension(extensions, "EGL_EXT_protected_surface");
    GLAD_EGL_EXT_stream_consumer_egloutput = glad_egl_has_extension(extensions, "EGL_EXT_stream_consumer_egloutput");
    GLAD_EGL_EXT_surface_CTA861_3_metadata = glad_egl_has_extension(extensions, "EGL_EXT_surface_CTA861_3_metadata");
    GLAD_EGL_EXT_surface_SMPTE2086_metadata = glad_egl_has_extension(extensions, "EGL_EXT_surface_SMPTE2086_metadata");
    GLAD_EGL_EXT_swap_buffers_with_damage = glad_egl_has_extension(extensions, "EGL_EXT_swap_buffers_with_damage");
    GLAD_EGL_EXT_sync_reuse = glad_egl_has_extension(extensions, "EGL_EXT_sync_reuse");
    GLAD_EGL_EXT_yuv_surface = glad_egl_has_extension(extensions, "EGL_EXT_yuv_surface");
    GLAD_EGL_HI_clientpixmap = glad_egl_has_extension(extensions, "EGL_HI_clientpixmap");
    GLAD_EGL_HI_colorformats = glad_egl_has_extension(extensions, "EGL_HI_colorformats");
    GLAD_EGL_IMG_context_priority = glad_egl_has_extension(extensions, "EGL_IMG_context_priority");
    GLAD_EGL_IMG_image_plane_attribs = glad_egl_has_extension(extensions, "EGL_IMG_image_plane_attribs");
    GLAD_EGL_KHR_cl_event = glad_egl_has_extension(extensions, "EGL_KHR_cl_event");
    GLAD_EGL_KHR_cl_event2 = glad_egl_has_extension(extensions, "EGL_KHR_cl_event2");
    GLAD_EGL_KHR_client_get_all_proc_addresses = glad_egl_has_extension(extensions, "EGL_KHR_client_get_all_proc_addresses");
    GLAD_EGL_KHR_config_attribs = glad_egl_has_extension(extensions, "EGL_KHR_config_attribs");
    GLAD_EGL_KHR_context_flush_control = glad_egl_has_extension(extensions, "EGL_KHR_context_flush_control");
    GLAD_EGL_KHR_create_context = glad_egl_has_extension(extensions, "EGL_KHR_create_context");
    GLAD_EGL_KHR_create_context_no_error = glad_egl_has_extension(extensions, "EGL_KHR_create_context_no_error");
    GLAD_EGL_KHR_debug = glad_egl_has_extension(extensions, "EGL_KHR_debug");
    GLAD_EGL_KHR_display_reference = glad_egl_has_extension(extensions, "EGL_KHR_display_reference");
    GLAD_EGL_KHR_fence_sync = glad_egl_has_extension(extensions, "EGL_KHR_fence_sync");
    GLAD_EGL_KHR_get_all_proc_addresses = glad_egl_has_extension(extensions, "EGL_KHR_get_all_proc_addresses");
    GLAD_EGL_KHR_gl_colorspace = glad_egl_has_extension(extensions, "EGL_KHR_gl_colorspace");
    GLAD_EGL_KHR_gl_renderbuffer_image = glad_egl_has_extension(extensions, "EGL_KHR_gl_renderbuffer_image");
    GLAD_EGL_KHR_gl_texture_2D_image = glad_egl_has_extension(extensions, "EGL_KHR_gl_texture_2D_image");
    GLAD_EGL_KHR_gl_texture_3D_image = glad_egl_has_extension(extensions, "EGL_KHR_gl_texture_3D_image");
    GLAD_EGL_KHR_gl_texture_cubemap_image = glad_egl_has_extension(extensions, "EGL_KHR_gl_texture_cubemap_image");
    GLAD_EGL_KHR_image = glad_egl_has_extension(extensions, "EGL_KHR_image");
    GLAD_EGL_KHR_image_base = glad_egl_has_extension(extensions, "EGL_KHR_image_base");
    GLAD_EGL_KHR_image_pixmap = glad_egl_has_extension(extensions, "EGL_KHR_image_pixmap");
    GLAD_EGL_KHR_lock_surface = glad_egl_has_extension(extensions, "EGL_KHR_lock_surface");
    GLAD_EGL_KHR_lock_surface2 = glad_egl_has_extension(extensions, "EGL_KHR_lock_surface2");
    GLAD_EGL_KHR_lock_surface3 = glad_egl_has_extension(extensions, "EGL_KHR_lock_surface3");
    GLAD_EGL_KHR_mutable_render_buffer = glad_egl_has_extension(extensions, "EGL_KHR_mutable_render_buffer");
    GLAD_EGL_KHR_no_config_context = glad_egl_has_extension(extensions, "EGL_KHR_no_config_context");
    GLAD_EGL_KHR_partial_update = glad_egl_has_extension(extensions, "EGL_KHR_partial_update");
    GLAD_EGL_KHR_platform_android = glad_egl_has_extension(extensions, "EGL_KHR_platform_android");
    GLAD_EGL_KHR_platform_gbm = glad_egl_has_extension(extensions, "EGL_KHR_platform_gbm");
    GLAD_EGL_KHR_platform_wayland = glad_egl_has_extension(extensions, "EGL_KHR_platform_wayland");
    GLAD_EGL_KHR_platform_x11 = glad_egl_has_extension(extensions, "EGL_KHR_platform_x11");
    GLAD_EGL_KHR_reusable_sync = glad_egl_has_extension(extensions, "EGL_KHR_reusable_sync");
    GLAD_EGL_KHR_stream = glad_egl_has_extension(extensions, "EGL_KHR_stream");
    GLAD_EGL_KHR_stream_attrib = glad_egl_has_extension(extensions, "EGL_KHR_stream_attrib");
    GLAD_EGL_KHR_stream_consumer_gltexture = glad_egl_has_extension(extensions, "EGL_KHR_stream_consumer_gltexture");
    GLAD_EGL_KHR_stream_cross_process_fd = glad_egl_has_extension(extensions, "EGL_KHR_stream_cross_process_fd");
    GLAD_EGL_KHR_stream_fifo = glad_egl_has_extension(extensions, "EGL_KHR_stream_fifo");
    GLAD_EGL_KHR_stream_producer_aldatalocator = glad_egl_has_extension(extensions, "EGL_KHR_stream_producer_aldatalocator");
    GLAD_EGL_KHR_stream_producer_eglsurface = glad_egl_has_extension(extensions, "EGL_KHR_stream_producer_eglsurface");
    GLAD_EGL_KHR_surfaceless_context = glad_egl_has_extension(extensions, "EGL_KHR_surfaceless_context");
    GLAD_EGL_KHR_swap_buffers_with_damage = glad_egl_has_extension(extensions, "EGL_KHR_swap_buffers_with_damage");
    GLAD_EGL_KHR_vg_parent_image = glad_egl_has_extension(extensions, "EGL_KHR_vg_parent_image");
    GLAD_EGL_KHR_wait_sync = glad_egl_has_extension(extensions, "EGL_KHR_wait_sync");
    GLAD_EGL_MESA_drm_image = glad_egl_has_extension(extensions, "EGL_MESA_drm_image");
    GLAD_EGL_MESA_image_dma_buf_export = glad_egl_has_extension(extensions, "EGL_MESA_image_dma_buf_export");
    GLAD_EGL_MESA_platform_gbm = glad_egl_has_extension(extensions, "EGL_MESA_platform_gbm");
    GLAD_EGL_MESA_platform_surfaceless = glad_egl_has_extension(extensions, "EGL_MESA_platform_surfaceless");
    GLAD_EGL_NOK_swap_region = glad_egl_has_extension(extensions, "EGL_NOK_swap_region");
    GLAD_EGL_NOK_swap_region2 = glad_egl_has_extension(extensions, "EGL_NOK_swap_region2");
    GLAD_EGL_NOK_texture_from_pixmap = glad_egl_has_extension(extensions, "EGL_NOK_texture_from_pixmap");
    GLAD_EGL_NV_3dvision_surface = glad_egl_has_extension(extensions, "EGL_NV_3dvision_surface");
    GLAD_EGL_NV_context_priority_realtime = glad_egl_has_extension(extensions, "EGL_NV_context_priority_realtime");
    GLAD_EGL_NV_coverage_sample = glad_egl_has_extension(extensions, "EGL_NV_coverage_sample");
    GLAD_EGL_NV_coverage_sample_resolve = glad_egl_has_extension(extensions, "EGL_NV_coverage_sample_resolve");
    GLAD_EGL_NV_cuda_event = glad_egl_has_extension(extensions, "EGL_NV_cuda_event");
    GLAD_EGL_NV_depth_nonlinear = glad_egl_has_extension(extensions, "EGL_NV_depth_nonlinear");
    GLAD_EGL_NV_device_cuda = glad_egl_has_extension(extensions, "EGL_NV_device_cuda");
    GLAD_EGL_NV_native_query = glad_egl_has_extension(extensions, "EGL_NV_native_query");
    GLAD_EGL_NV_post_convert_rounding = glad_egl_has_extension(extensions, "EGL_NV_post_convert_rounding");
    GLAD_EGL_NV_post_sub_buffer = glad_egl_has_extension(extensions, "EGL_NV_post_sub_buffer");
    GLAD_EGL_NV_robustness_video_memory_purge = glad_egl_has_extension(extensions, "EGL_NV_robustness_video_memory_purge");
    GLAD_EGL_NV_stream_consumer_gltexture_yuv = glad_egl_has_extension(extensions, "EGL_NV_stream_consumer_gltexture_yuv");
    GLAD_EGL_NV_stream_cross_display = glad_egl_has_extension(extensions, "EGL_NV_stream_cross_display");
    GLAD_EGL_NV_stream_cross_object = glad_egl_has_extension(extensions, "EGL_NV_stream_cross_object");
    GLAD_EGL_NV_stream_cross_partition = glad_egl_has_extension(extensions, "EGL_NV_stream_cross_partition");
    GLAD_EGL_NV_stream_cross_process = glad_egl_has_extension(extensions, "EGL_NV_stream_cross_process");
    GLAD_EGL_NV_stream_cross_system = glad_egl_has_extension(extensions, "EGL_NV_stream_cross_system");
    GLAD_EGL_NV_stream_fifo_next = glad_egl_has_extension(extensions, "EGL_NV_stream_fifo_next");
    GLAD_EGL_NV_stream_fifo_synchronous = glad_egl_has_extension(extensions, "EGL_NV_stream_fifo_synchronous");
    GLAD_EGL_NV_stream_flush = glad_egl_has_extension(extensions, "EGL_NV_stream_flush");
    GLAD_EGL_NV_stream_frame_limits = glad_egl_has_extension(extensions, "EGL_NV_stream_frame_limits");
    GLAD_EGL_NV_stream_metadata = glad_egl_has_extension(extensions, "EGL_NV_stream_metadata");
    GLAD_EGL_NV_stream_remote = glad_egl_has_extension(extensions, "EGL_NV_stream_remote");
    GLAD_EGL_NV_stream_reset = glad_egl_has_extension(extensions, "EGL_NV_stream_reset");
    GLAD_EGL_NV_stream_socket = glad_egl_has_extension(extensions, "EGL_NV_stream_socket");
    GLAD_EGL_NV_stream_socket_inet = glad_egl_has_extension(extensions, "EGL_NV_stream_socket_inet");
    GLAD_EGL_NV_stream_socket_unix = glad_egl_has_extension(extensions, "EGL_NV_stream_socket_unix");
    GLAD_EGL_NV_stream_sync = glad_egl_has_extension(extensions, "EGL_NV_stream_sync");
    GLAD_EGL_NV_sync = glad_egl_has_extension(extensions, "EGL_NV_sync");
    GLAD_EGL_NV_system_time = glad_egl_has_extension(extensions, "EGL_NV_system_time");
    GLAD_EGL_TIZEN_image_native_buffer = glad_egl_has_extension(extensions, "EGL_TIZEN_image_native_buffer");
    GLAD_EGL_TIZEN_image_native_surface = glad_egl_has_extension(extensions, "EGL_TIZEN_image_native_surface");

    return 1;
}

static int glad_egl_find_core_egl(EGLDisplay display) {
    int major, minor;
    const char *version;

    if (display == NULL) {
        display = EGL_NO_DISPLAY; /* this is usually NULL, better safe than sorry */
    }
    if (display == EGL_NO_DISPLAY) {
        display = eglGetCurrentDisplay();
    }
#ifdef EGL_VERSION_1_4
    if (display == EGL_NO_DISPLAY) {
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    }
#endif
    if (display == EGL_NO_DISPLAY) {
        return 0;
    }

    version = eglQueryString(display, EGL_VERSION);
    (void) eglGetError();

    if (version == NULL) {
        major = 1;
        minor = 0;
    } else {
        GLAD_IMPL_UTIL_SSCANF(version, "%d.%d", &major, &minor);
    }

    GLAD_EGL_VERSION_1_0 = (major == 1 && minor >= 0) || major > 1;
    GLAD_EGL_VERSION_1_1 = (major == 1 && minor >= 1) || major > 1;
    GLAD_EGL_VERSION_1_2 = (major == 1 && minor >= 2) || major > 1;
    GLAD_EGL_VERSION_1_3 = (major == 1 && minor >= 3) || major > 1;
    GLAD_EGL_VERSION_1_4 = (major == 1 && minor >= 4) || major > 1;
    GLAD_EGL_VERSION_1_5 = (major == 1 && minor >= 5) || major > 1;

    return GLAD_MAKE_VERSION(major, minor);
}

int gladLoadEGLUserPtr(EGLDisplay display, GLADuserptrloadfunc load, void* userptr) {
    int version;
    eglGetDisplay = (PFNEGLGETDISPLAYPROC) load("eglGetDisplay", userptr);
    eglGetCurrentDisplay = (PFNEGLGETCURRENTDISPLAYPROC) load("eglGetCurrentDisplay", userptr);
    eglQueryString = (PFNEGLQUERYSTRINGPROC) load("eglQueryString", userptr);
    eglGetError = (PFNEGLGETERRORPROC) load("eglGetError", userptr);
    if (eglGetDisplay == NULL || eglGetCurrentDisplay == NULL || eglQueryString == NULL || eglGetError == NULL) return 0;

    version = glad_egl_find_core_egl(display);
    if (!version) return 0;
    glad_egl_load_EGL_VERSION_1_0(load, userptr);
    glad_egl_load_EGL_VERSION_1_1(load, userptr);
    glad_egl_load_EGL_VERSION_1_2(load, userptr);
    glad_egl_load_EGL_VERSION_1_4(load, userptr);
    glad_egl_load_EGL_VERSION_1_5(load, userptr);

    if (!glad_egl_find_extensions_egl(display)) return 0;
    glad_egl_load_EGL_ANDROID_blob_cache(load, userptr);
    glad_egl_load_EGL_ANDROID_create_native_client_buffer(load, userptr);
    glad_egl_load_EGL_ANDROID_get_frame_timestamps(load, userptr);
    glad_egl_load_EGL_ANDROID_get_native_client_buffer(load, userptr);
    glad_egl_load_EGL_ANDROID_native_fence_sync(load, userptr);
    glad_egl_load_EGL_ANDROID_presentation_time(load, userptr);
    glad_egl_load_EGL_ANGLE_query_surface_pointer(load, userptr);
    glad_egl_load_EGL_EXT_client_sync(load, userptr);
    glad_egl_load_EGL_EXT_compositor(load, userptr);
    glad_egl_load_EGL_EXT_device_base(load, userptr);
    glad_egl_load_EGL_EXT_device_enumeration(load, userptr);
    glad_egl_load_EGL_EXT_device_query(load, userptr);
    glad_egl_load_EGL_EXT_image_dma_buf_import_modifiers(load, userptr);
    glad_egl_load_EGL_EXT_output_base(load, userptr);
    glad_egl_load_EGL_EXT_platform_base(load, userptr);
    glad_egl_load_EGL_EXT_stream_consumer_egloutput(load, userptr);
    glad_egl_load_EGL_EXT_swap_buffers_with_damage(load, userptr);
    glad_egl_load_EGL_EXT_sync_reuse(load, userptr);
    glad_egl_load_EGL_HI_clientpixmap(load, userptr);
    glad_egl_load_EGL_KHR_cl_event2(load, userptr);
    glad_egl_load_EGL_KHR_debug(load, userptr);
    glad_egl_load_EGL_KHR_display_reference(load, userptr);
    glad_egl_load_EGL_KHR_fence_sync(load, userptr);
    glad_egl_load_EGL_KHR_image(load, userptr);
    glad_egl_load_EGL_KHR_image_base(load, userptr);
    glad_egl_load_EGL_KHR_lock_surface(load, userptr);
    glad_egl_load_EGL_KHR_lock_surface3(load, userptr);
    glad_egl_load_EGL_KHR_partial_update(load, userptr);
    glad_egl_load_EGL_KHR_reusable_sync(load, userptr);
    glad_egl_load_EGL_KHR_stream(load, userptr);
    glad_egl_load_EGL_KHR_stream_attrib(load, userptr);
    glad_egl_load_EGL_KHR_stream_consumer_gltexture(load, userptr);
    glad_egl_load_EGL_KHR_stream_cross_process_fd(load, userptr);
    glad_egl_load_EGL_KHR_stream_fifo(load, userptr);
    glad_egl_load_EGL_KHR_stream_producer_eglsurface(load, userptr);
    glad_egl_load_EGL_KHR_swap_buffers_with_damage(load, userptr);
    glad_egl_load_EGL_KHR_wait_sync(load, userptr);
    glad_egl_load_EGL_MESA_drm_image(load, userptr);
    glad_egl_load_EGL_MESA_image_dma_buf_export(load, userptr);
    glad_egl_load_EGL_NOK_swap_region(load, userptr);
    glad_egl_load_EGL_NOK_swap_region2(load, userptr);
    glad_egl_load_EGL_NV_native_query(load, userptr);
    glad_egl_load_EGL_NV_post_sub_buffer(load, userptr);
    glad_egl_load_EGL_NV_stream_consumer_gltexture_yuv(load, userptr);
    glad_egl_load_EGL_NV_stream_flush(load, userptr);
    glad_egl_load_EGL_NV_stream_metadata(load, userptr);
    glad_egl_load_EGL_NV_stream_reset(load, userptr);
    glad_egl_load_EGL_NV_stream_sync(load, userptr);
    glad_egl_load_EGL_NV_sync(load, userptr);
    glad_egl_load_EGL_NV_system_time(load, userptr);

    return version;
}

int gladLoadEGL(EGLDisplay display, GLADloadfunc load) {
    return gladLoadEGLUserPtr(display, glad_egl_get_proc_from_userptr, GLAD_GNUC_EXTENSION (void*) load);
}


#ifdef GLAD_EGL

#ifndef GLAD_LOADER_LIBRARY_C_
#define GLAD_LOADER_LIBRARY_C_

#include <stddef.h>
#include <stdlib.h>

#if GLAD_PLATFORM_WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif


static void* glad_get_dlopen_handle(const char *lib_names[], int length) {
    void *handle = NULL;
    int i;

    for (i = 0; i < length; ++i) {
#if GLAD_PLATFORM_WIN32
  #if GLAD_PLATFORM_UWP
        size_t buffer_size = (strlen(lib_names[i]) + 1) * sizeof(WCHAR);
        LPWSTR buffer = (LPWSTR) malloc(buffer_size);
        if (buffer != NULL) {
            int ret = MultiByteToWideChar(CP_ACP, 0, lib_names[i], -1, buffer, buffer_size);
            if (ret != 0) {
                handle = (void*) LoadPackagedLibrary(buffer, 0);
            }
            free((void*) buffer);
        }
  #else
        handle = (void*) LoadLibraryA(lib_names[i]);
  #endif
#else
        handle = dlopen(lib_names[i], RTLD_LAZY | RTLD_LOCAL);
#endif
        if (handle != NULL) {
            return handle;
        }
    }

    return NULL;
}

static void glad_close_dlopen_handle(void* handle) {
    if (handle != NULL) {
#if GLAD_PLATFORM_WIN32
        FreeLibrary((HMODULE) handle);
#else
        dlclose(handle);
#endif
    }
}

static GLADapiproc glad_dlsym_handle(void* handle, const char *name) {
    if (handle == NULL) {
        return NULL;
    }

#if GLAD_PLATFORM_WIN32
    return (GLADapiproc) GetProcAddress((HMODULE) handle, name);
#else
    return GLAD_GNUC_EXTENSION (GLADapiproc) dlsym(handle, name);
#endif
}

#endif /* GLAD_LOADER_LIBRARY_C_ */

struct _glad_egl_userptr {
    void *handle;
    PFNEGLGETPROCADDRESSPROC get_proc_address_ptr;
};

static GLADapiproc glad_egl_get_proc(const char* name, void *vuserptr) {
    struct _glad_egl_userptr userptr = *(struct _glad_egl_userptr*) vuserptr;
    GLADapiproc result = NULL;

    result = glad_dlsym_handle(userptr.handle, name);
    if (result == NULL) {
        result = GLAD_GNUC_EXTENSION (GLADapiproc) userptr.get_proc_address_ptr(name);
    }

    return result;
}

static void* _egl_handle = NULL;

int gladLoaderLoadEGL(EGLDisplay display) {
#if GLAD_PLATFORM_APPLE
    static const char *NAMES[] = {"libEGL.dylib"};
#elif GLAD_PLATFORM_WIN32
    static const char *NAMES[] = {"libEGL.dll", "EGL.dll"};
#else
    static const char *NAMES[] = {"libEGL.so.1", "libEGL.so"};
#endif

    int version = 0;
    int did_load = 0;
    struct _glad_egl_userptr userptr;

    if (_egl_handle == NULL) {
        _egl_handle = glad_get_dlopen_handle(NAMES, sizeof(NAMES) / sizeof(NAMES[0]));
        did_load = _egl_handle != NULL;
    }

    if (_egl_handle != NULL) {
        userptr.handle = _egl_handle;
        userptr.get_proc_address_ptr = (PFNEGLGETPROCADDRESSPROC) glad_dlsym_handle(_egl_handle, "eglGetProcAddress");
        if (userptr.get_proc_address_ptr != NULL) {
            version = gladLoadEGLUserPtr(display, glad_egl_get_proc, &userptr);
        }

        if (!version && did_load) {
            glad_close_dlopen_handle(_egl_handle);
            _egl_handle = NULL;
        }
    }

    return version;
}

void gladLoaderUnloadEGL() {
    if (_egl_handle != NULL) {
        glad_close_dlopen_handle(_egl_handle);
        _egl_handle = NULL;
    }
}

#endif /* GLAD_EGL */
