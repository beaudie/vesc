#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/gles2.h>

#ifndef GLAD_IMPL_UTIL_C_
#define GLAD_IMPL_UTIL_C_

#ifdef _MSC_VER
#define GLAD_IMPL_UTIL_SSCANF sscanf_s
#else
#define GLAD_IMPL_UTIL_SSCANF sscanf
#endif

#endif /* GLAD_IMPL_UTIL_C_ */


int GLAD_GL_ES_VERSION_2_0 = 0;
int GLAD_GL_ES_VERSION_3_0 = 0;
int GLAD_GL_ES_VERSION_3_1 = 0;
int GLAD_GL_ES_VERSION_3_2 = 0;
int GLAD_GL_AMD_compressed_3DC_texture = 0;
int GLAD_GL_AMD_compressed_ATC_texture = 0;
int GLAD_GL_AMD_framebuffer_multisample_advanced = 0;
int GLAD_GL_AMD_performance_monitor = 0;
int GLAD_GL_AMD_program_binary_Z400 = 0;
int GLAD_GL_ANDROID_extension_pack_es31a = 0;
int GLAD_GL_ANGLE_depth_texture = 0;
int GLAD_GL_ANGLE_framebuffer_blit = 0;
int GLAD_GL_ANGLE_framebuffer_multisample = 0;
int GLAD_GL_ANGLE_instanced_arrays = 0;
int GLAD_GL_ANGLE_pack_reverse_row_order = 0;
int GLAD_GL_ANGLE_program_binary = 0;
int GLAD_GL_ANGLE_texture_compression_dxt3 = 0;
int GLAD_GL_ANGLE_texture_compression_dxt5 = 0;
int GLAD_GL_ANGLE_texture_usage = 0;
int GLAD_GL_ANGLE_translated_shader_source = 0;
int GLAD_GL_APPLE_clip_distance = 0;
int GLAD_GL_APPLE_color_buffer_packed_float = 0;
int GLAD_GL_APPLE_copy_texture_levels = 0;
int GLAD_GL_APPLE_framebuffer_multisample = 0;
int GLAD_GL_APPLE_rgb_422 = 0;
int GLAD_GL_APPLE_sync = 0;
int GLAD_GL_APPLE_texture_format_BGRA8888 = 0;
int GLAD_GL_APPLE_texture_max_level = 0;
int GLAD_GL_APPLE_texture_packed_float = 0;
int GLAD_GL_ARM_mali_program_binary = 0;
int GLAD_GL_ARM_mali_shader_binary = 0;
int GLAD_GL_ARM_rgba8 = 0;
int GLAD_GL_ARM_shader_framebuffer_fetch = 0;
int GLAD_GL_ARM_shader_framebuffer_fetch_depth_stencil = 0;
int GLAD_GL_DMP_program_binary = 0;
int GLAD_GL_DMP_shader_binary = 0;
int GLAD_GL_EXT_EGL_image_array = 0;
int GLAD_GL_EXT_EGL_image_storage = 0;
int GLAD_GL_EXT_YUV_target = 0;
int GLAD_GL_EXT_base_instance = 0;
int GLAD_GL_EXT_blend_func_extended = 0;
int GLAD_GL_EXT_blend_minmax = 0;
int GLAD_GL_EXT_buffer_storage = 0;
int GLAD_GL_EXT_clear_texture = 0;
int GLAD_GL_EXT_clip_control = 0;
int GLAD_GL_EXT_clip_cull_distance = 0;
int GLAD_GL_EXT_color_buffer_float = 0;
int GLAD_GL_EXT_color_buffer_half_float = 0;
int GLAD_GL_EXT_conservative_depth = 0;
int GLAD_GL_EXT_copy_image = 0;
int GLAD_GL_EXT_debug_label = 0;
int GLAD_GL_EXT_debug_marker = 0;
int GLAD_GL_EXT_discard_framebuffer = 0;
int GLAD_GL_EXT_disjoint_timer_query = 0;
int GLAD_GL_EXT_draw_buffers = 0;
int GLAD_GL_EXT_draw_buffers_indexed = 0;
int GLAD_GL_EXT_draw_elements_base_vertex = 0;
int GLAD_GL_EXT_draw_instanced = 0;
int GLAD_GL_EXT_draw_transform_feedback = 0;
int GLAD_GL_EXT_external_buffer = 0;
int GLAD_GL_EXT_float_blend = 0;
int GLAD_GL_EXT_geometry_point_size = 0;
int GLAD_GL_EXT_geometry_shader = 0;
int GLAD_GL_EXT_gpu_shader5 = 0;
int GLAD_GL_EXT_instanced_arrays = 0;
int GLAD_GL_EXT_map_buffer_range = 0;
int GLAD_GL_EXT_memory_object = 0;
int GLAD_GL_EXT_memory_object_fd = 0;
int GLAD_GL_EXT_memory_object_win32 = 0;
int GLAD_GL_EXT_multi_draw_arrays = 0;
int GLAD_GL_EXT_multi_draw_indirect = 0;
int GLAD_GL_EXT_multisampled_compatibility = 0;
int GLAD_GL_EXT_multisampled_render_to_texture = 0;
int GLAD_GL_EXT_multiview_draw_buffers = 0;
int GLAD_GL_EXT_occlusion_query_boolean = 0;
int GLAD_GL_EXT_polygon_offset_clamp = 0;
int GLAD_GL_EXT_post_depth_coverage = 0;
int GLAD_GL_EXT_primitive_bounding_box = 0;
int GLAD_GL_EXT_protected_textures = 0;
int GLAD_GL_EXT_pvrtc_sRGB = 0;
int GLAD_GL_EXT_raster_multisample = 0;
int GLAD_GL_EXT_read_format_bgra = 0;
int GLAD_GL_EXT_render_snorm = 0;
int GLAD_GL_EXT_robustness = 0;
int GLAD_GL_EXT_sRGB = 0;
int GLAD_GL_EXT_sRGB_write_control = 0;
int GLAD_GL_EXT_semaphore = 0;
int GLAD_GL_EXT_semaphore_fd = 0;
int GLAD_GL_EXT_semaphore_win32 = 0;
int GLAD_GL_EXT_separate_shader_objects = 0;
int GLAD_GL_EXT_shader_framebuffer_fetch = 0;
int GLAD_GL_EXT_shader_framebuffer_fetch_non_coherent = 0;
int GLAD_GL_EXT_shader_group_vote = 0;
int GLAD_GL_EXT_shader_implicit_conversions = 0;
int GLAD_GL_EXT_shader_integer_mix = 0;
int GLAD_GL_EXT_shader_io_blocks = 0;
int GLAD_GL_EXT_shader_non_constant_global_initializers = 0;
int GLAD_GL_EXT_shader_pixel_local_storage = 0;
int GLAD_GL_EXT_shader_pixel_local_storage2 = 0;
int GLAD_GL_EXT_shader_texture_lod = 0;
int GLAD_GL_EXT_shadow_samplers = 0;
int GLAD_GL_EXT_sparse_texture = 0;
int GLAD_GL_EXT_sparse_texture2 = 0;
int GLAD_GL_EXT_tessellation_point_size = 0;
int GLAD_GL_EXT_tessellation_shader = 0;
int GLAD_GL_EXT_texture_border_clamp = 0;
int GLAD_GL_EXT_texture_buffer = 0;
int GLAD_GL_EXT_texture_compression_astc_decode_mode = 0;
int GLAD_GL_EXT_texture_compression_bptc = 0;
int GLAD_GL_EXT_texture_compression_dxt1 = 0;
int GLAD_GL_EXT_texture_compression_rgtc = 0;
int GLAD_GL_EXT_texture_compression_s3tc = 0;
int GLAD_GL_EXT_texture_compression_s3tc_srgb = 0;
int GLAD_GL_EXT_texture_cube_map_array = 0;
int GLAD_GL_EXT_texture_filter_anisotropic = 0;
int GLAD_GL_EXT_texture_filter_minmax = 0;
int GLAD_GL_EXT_texture_format_BGRA8888 = 0;
int GLAD_GL_EXT_texture_format_sRGB_override = 0;
int GLAD_GL_EXT_texture_mirror_clamp_to_edge = 0;
int GLAD_GL_EXT_texture_norm16 = 0;
int GLAD_GL_EXT_texture_rg = 0;
int GLAD_GL_EXT_texture_sRGB_R8 = 0;
int GLAD_GL_EXT_texture_sRGB_RG8 = 0;
int GLAD_GL_EXT_texture_sRGB_decode = 0;
int GLAD_GL_EXT_texture_storage = 0;
int GLAD_GL_EXT_texture_type_2_10_10_10_REV = 0;
int GLAD_GL_EXT_texture_view = 0;
int GLAD_GL_EXT_unpack_subimage = 0;
int GLAD_GL_EXT_win32_keyed_mutex = 0;
int GLAD_GL_EXT_window_rectangles = 0;
int GLAD_GL_FJ_shader_binary_GCCSO = 0;
int GLAD_GL_IMG_bindless_texture = 0;
int GLAD_GL_IMG_framebuffer_downsample = 0;
int GLAD_GL_IMG_multisampled_render_to_texture = 0;
int GLAD_GL_IMG_program_binary = 0;
int GLAD_GL_IMG_read_format = 0;
int GLAD_GL_IMG_shader_binary = 0;
int GLAD_GL_IMG_texture_compression_pvrtc = 0;
int GLAD_GL_IMG_texture_compression_pvrtc2 = 0;
int GLAD_GL_IMG_texture_filter_cubic = 0;
int GLAD_GL_INTEL_blackhole_render = 0;
int GLAD_GL_INTEL_conservative_rasterization = 0;
int GLAD_GL_INTEL_framebuffer_CMAA = 0;
int GLAD_GL_INTEL_performance_query = 0;
int GLAD_GL_KHR_blend_equation_advanced = 0;
int GLAD_GL_KHR_blend_equation_advanced_coherent = 0;
int GLAD_GL_KHR_context_flush_control = 0;
int GLAD_GL_KHR_debug = 0;
int GLAD_GL_KHR_no_error = 0;
int GLAD_GL_KHR_parallel_shader_compile = 0;
int GLAD_GL_KHR_robust_buffer_access_behavior = 0;
int GLAD_GL_KHR_robustness = 0;
int GLAD_GL_KHR_texture_compression_astc_hdr = 0;
int GLAD_GL_KHR_texture_compression_astc_ldr = 0;
int GLAD_GL_KHR_texture_compression_astc_sliced_3d = 0;
int GLAD_GL_MESA_framebuffer_flip_y = 0;
int GLAD_GL_MESA_program_binary_formats = 0;
int GLAD_GL_MESA_shader_integer_functions = 0;
int GLAD_GL_NVX_blend_equation_advanced_multi_draw_buffers = 0;
int GLAD_GL_NV_bindless_texture = 0;
int GLAD_GL_NV_blend_equation_advanced = 0;
int GLAD_GL_NV_blend_equation_advanced_coherent = 0;
int GLAD_GL_NV_blend_minmax_factor = 0;
int GLAD_GL_NV_clip_space_w_scaling = 0;
int GLAD_GL_NV_conditional_render = 0;
int GLAD_GL_NV_conservative_raster = 0;
int GLAD_GL_NV_conservative_raster_pre_snap = 0;
int GLAD_GL_NV_conservative_raster_pre_snap_triangles = 0;
int GLAD_GL_NV_copy_buffer = 0;
int GLAD_GL_NV_coverage_sample = 0;
int GLAD_GL_NV_depth_nonlinear = 0;
int GLAD_GL_NV_draw_buffers = 0;
int GLAD_GL_NV_draw_instanced = 0;
int GLAD_GL_NV_draw_vulkan_image = 0;
int GLAD_GL_NV_explicit_attrib_location = 0;
int GLAD_GL_NV_fbo_color_attachments = 0;
int GLAD_GL_NV_fence = 0;
int GLAD_GL_NV_fill_rectangle = 0;
int GLAD_GL_NV_fragment_coverage_to_color = 0;
int GLAD_GL_NV_fragment_shader_interlock = 0;
int GLAD_GL_NV_framebuffer_blit = 0;
int GLAD_GL_NV_framebuffer_mixed_samples = 0;
int GLAD_GL_NV_framebuffer_multisample = 0;
int GLAD_GL_NV_generate_mipmap_sRGB = 0;
int GLAD_GL_NV_geometry_shader_passthrough = 0;
int GLAD_GL_NV_gpu_shader5 = 0;
int GLAD_GL_NV_image_formats = 0;
int GLAD_GL_NV_instanced_arrays = 0;
int GLAD_GL_NV_internalformat_sample_query = 0;
int GLAD_GL_NV_memory_attachment = 0;
int GLAD_GL_NV_non_square_matrices = 0;
int GLAD_GL_NV_path_rendering = 0;
int GLAD_GL_NV_path_rendering_shared_edge = 0;
int GLAD_GL_NV_pixel_buffer_object = 0;
int GLAD_GL_NV_polygon_mode = 0;
int GLAD_GL_NV_read_buffer = 0;
int GLAD_GL_NV_read_buffer_front = 0;
int GLAD_GL_NV_read_depth = 0;
int GLAD_GL_NV_read_depth_stencil = 0;
int GLAD_GL_NV_read_stencil = 0;
int GLAD_GL_NV_sRGB_formats = 0;
int GLAD_GL_NV_sample_locations = 0;
int GLAD_GL_NV_sample_mask_override_coverage = 0;
int GLAD_GL_NV_shader_atomic_fp16_vector = 0;
int GLAD_GL_NV_shader_noperspective_interpolation = 0;
int GLAD_GL_NV_shadow_samplers_array = 0;
int GLAD_GL_NV_shadow_samplers_cube = 0;
int GLAD_GL_NV_stereo_view_rendering = 0;
int GLAD_GL_NV_texture_border_clamp = 0;
int GLAD_GL_NV_texture_compression_s3tc_update = 0;
int GLAD_GL_NV_texture_npot_2D_mipmap = 0;
int GLAD_GL_NV_viewport_array = 0;
int GLAD_GL_NV_viewport_array2 = 0;
int GLAD_GL_NV_viewport_swizzle = 0;
int GLAD_GL_OES_EGL_image = 0;
int GLAD_GL_OES_EGL_image_external = 0;
int GLAD_GL_OES_EGL_image_external_essl3 = 0;
int GLAD_GL_OES_compressed_ETC1_RGB8_sub_texture = 0;
int GLAD_GL_OES_compressed_ETC1_RGB8_texture = 0;
int GLAD_GL_OES_compressed_paletted_texture = 0;
int GLAD_GL_OES_copy_image = 0;
int GLAD_GL_OES_depth24 = 0;
int GLAD_GL_OES_depth32 = 0;
int GLAD_GL_OES_depth_texture = 0;
int GLAD_GL_OES_draw_buffers_indexed = 0;
int GLAD_GL_OES_draw_elements_base_vertex = 0;
int GLAD_GL_OES_element_index_uint = 0;
int GLAD_GL_OES_fbo_render_mipmap = 0;
int GLAD_GL_OES_fragment_precision_high = 0;
int GLAD_GL_OES_geometry_point_size = 0;
int GLAD_GL_OES_geometry_shader = 0;
int GLAD_GL_OES_get_program_binary = 0;
int GLAD_GL_OES_gpu_shader5 = 0;
int GLAD_GL_OES_mapbuffer = 0;
int GLAD_GL_OES_packed_depth_stencil = 0;
int GLAD_GL_OES_primitive_bounding_box = 0;
int GLAD_GL_OES_required_internalformat = 0;
int GLAD_GL_OES_rgb8_rgba8 = 0;
int GLAD_GL_OES_sample_shading = 0;
int GLAD_GL_OES_sample_variables = 0;
int GLAD_GL_OES_shader_image_atomic = 0;
int GLAD_GL_OES_shader_io_blocks = 0;
int GLAD_GL_OES_shader_multisample_interpolation = 0;
int GLAD_GL_OES_standard_derivatives = 0;
int GLAD_GL_OES_stencil1 = 0;
int GLAD_GL_OES_stencil4 = 0;
int GLAD_GL_OES_surfaceless_context = 0;
int GLAD_GL_OES_tessellation_point_size = 0;
int GLAD_GL_OES_tessellation_shader = 0;
int GLAD_GL_OES_texture_3D = 0;
int GLAD_GL_OES_texture_border_clamp = 0;
int GLAD_GL_OES_texture_buffer = 0;
int GLAD_GL_OES_texture_compression_astc = 0;
int GLAD_GL_OES_texture_cube_map_array = 0;
int GLAD_GL_OES_texture_float = 0;
int GLAD_GL_OES_texture_float_linear = 0;
int GLAD_GL_OES_texture_half_float = 0;
int GLAD_GL_OES_texture_half_float_linear = 0;
int GLAD_GL_OES_texture_npot = 0;
int GLAD_GL_OES_texture_stencil8 = 0;
int GLAD_GL_OES_texture_storage_multisample_2d_array = 0;
int GLAD_GL_OES_texture_view = 0;
int GLAD_GL_OES_vertex_array_object = 0;
int GLAD_GL_OES_vertex_half_float = 0;
int GLAD_GL_OES_vertex_type_10_10_10_2 = 0;
int GLAD_GL_OES_viewport_array = 0;
int GLAD_GL_OVR_multiview = 0;
int GLAD_GL_OVR_multiview2 = 0;
int GLAD_GL_OVR_multiview_multisampled_render_to_texture = 0;
int GLAD_GL_QCOM_alpha_test = 0;
int GLAD_GL_QCOM_binning_control = 0;
int GLAD_GL_QCOM_driver_control = 0;
int GLAD_GL_QCOM_extended_get = 0;
int GLAD_GL_QCOM_extended_get2 = 0;
int GLAD_GL_QCOM_framebuffer_foveated = 0;
int GLAD_GL_QCOM_perfmon_global_mode = 0;
int GLAD_GL_QCOM_shader_framebuffer_fetch_noncoherent = 0;
int GLAD_GL_QCOM_shader_framebuffer_fetch_rate = 0;
int GLAD_GL_QCOM_texture_foveated = 0;
int GLAD_GL_QCOM_texture_foveated_subsampled_layout = 0;
int GLAD_GL_QCOM_tiled_rendering = 0;
int GLAD_GL_QCOM_writeonly_rendering = 0;
int GLAD_GL_VIV_shader_binary = 0;



PFNGLACQUIREKEYEDMUTEXWIN32EXTPROC glad_glAcquireKeyedMutexWin32EXT = NULL;
PFNGLACTIVESHADERPROGRAMPROC glad_glActiveShaderProgram = NULL;
PFNGLACTIVESHADERPROGRAMEXTPROC glad_glActiveShaderProgramEXT = NULL;
PFNGLACTIVETEXTUREPROC glad_glActiveTexture = NULL;
PFNGLALPHAFUNCQCOMPROC glad_glAlphaFuncQCOM = NULL;
PFNGLAPPLYFRAMEBUFFERATTACHMENTCMAAINTELPROC glad_glApplyFramebufferAttachmentCMAAINTEL = NULL;
PFNGLATTACHSHADERPROC glad_glAttachShader = NULL;
PFNGLBEGINCONDITIONALRENDERPROC glad_glBeginConditionalRender = NULL;
PFNGLBEGINCONDITIONALRENDERNVPROC glad_glBeginConditionalRenderNV = NULL;
PFNGLBEGINPERFMONITORAMDPROC glad_glBeginPerfMonitorAMD = NULL;
PFNGLBEGINPERFQUERYINTELPROC glad_glBeginPerfQueryINTEL = NULL;
PFNGLBEGINQUERYPROC glad_glBeginQuery = NULL;
PFNGLBEGINQUERYEXTPROC glad_glBeginQueryEXT = NULL;
PFNGLBEGINTRANSFORMFEEDBACKPROC glad_glBeginTransformFeedback = NULL;
PFNGLBINDATTRIBLOCATIONPROC glad_glBindAttribLocation = NULL;
PFNGLBINDBUFFERPROC glad_glBindBuffer = NULL;
PFNGLBINDBUFFERBASEPROC glad_glBindBufferBase = NULL;
PFNGLBINDBUFFERRANGEPROC glad_glBindBufferRange = NULL;
PFNGLBINDFRAGDATALOCATIONPROC glad_glBindFragDataLocation = NULL;
PFNGLBINDFRAGDATALOCATIONEXTPROC glad_glBindFragDataLocationEXT = NULL;
PFNGLBINDFRAGDATALOCATIONINDEXEDPROC glad_glBindFragDataLocationIndexed = NULL;
PFNGLBINDFRAGDATALOCATIONINDEXEDEXTPROC glad_glBindFragDataLocationIndexedEXT = NULL;
PFNGLBINDFRAMEBUFFERPROC glad_glBindFramebuffer = NULL;
PFNGLBINDIMAGETEXTUREPROC glad_glBindImageTexture = NULL;
PFNGLBINDPROGRAMPIPELINEPROC glad_glBindProgramPipeline = NULL;
PFNGLBINDPROGRAMPIPELINEEXTPROC glad_glBindProgramPipelineEXT = NULL;
PFNGLBINDRENDERBUFFERPROC glad_glBindRenderbuffer = NULL;
PFNGLBINDSAMPLERPROC glad_glBindSampler = NULL;
PFNGLBINDTEXTUREPROC glad_glBindTexture = NULL;
PFNGLBINDTRANSFORMFEEDBACKPROC glad_glBindTransformFeedback = NULL;
PFNGLBINDVERTEXARRAYPROC glad_glBindVertexArray = NULL;
PFNGLBINDVERTEXARRAYOESPROC glad_glBindVertexArrayOES = NULL;
PFNGLBINDVERTEXBUFFERPROC glad_glBindVertexBuffer = NULL;
PFNGLBLENDBARRIERPROC glad_glBlendBarrier = NULL;
PFNGLBLENDBARRIERKHRPROC glad_glBlendBarrierKHR = NULL;
PFNGLBLENDBARRIERNVPROC glad_glBlendBarrierNV = NULL;
PFNGLBLENDCOLORPROC glad_glBlendColor = NULL;
PFNGLBLENDEQUATIONPROC glad_glBlendEquation = NULL;
PFNGLBLENDEQUATIONSEPARATEPROC glad_glBlendEquationSeparate = NULL;
PFNGLBLENDEQUATIONSEPARATEIPROC glad_glBlendEquationSeparatei = NULL;
PFNGLBLENDEQUATIONSEPARATEIEXTPROC glad_glBlendEquationSeparateiEXT = NULL;
PFNGLBLENDEQUATIONSEPARATEIOESPROC glad_glBlendEquationSeparateiOES = NULL;
PFNGLBLENDEQUATIONIPROC glad_glBlendEquationi = NULL;
PFNGLBLENDEQUATIONIEXTPROC glad_glBlendEquationiEXT = NULL;
PFNGLBLENDEQUATIONIOESPROC glad_glBlendEquationiOES = NULL;
PFNGLBLENDFUNCPROC glad_glBlendFunc = NULL;
PFNGLBLENDFUNCSEPARATEPROC glad_glBlendFuncSeparate = NULL;
PFNGLBLENDFUNCSEPARATEIPROC glad_glBlendFuncSeparatei = NULL;
PFNGLBLENDFUNCSEPARATEIEXTPROC glad_glBlendFuncSeparateiEXT = NULL;
PFNGLBLENDFUNCSEPARATEIOESPROC glad_glBlendFuncSeparateiOES = NULL;
PFNGLBLENDFUNCIPROC glad_glBlendFunci = NULL;
PFNGLBLENDFUNCIEXTPROC glad_glBlendFunciEXT = NULL;
PFNGLBLENDFUNCIOESPROC glad_glBlendFunciOES = NULL;
PFNGLBLENDPARAMETERINVPROC glad_glBlendParameteriNV = NULL;
PFNGLBLITFRAMEBUFFERPROC glad_glBlitFramebuffer = NULL;
PFNGLBLITFRAMEBUFFERANGLEPROC glad_glBlitFramebufferANGLE = NULL;
PFNGLBLITFRAMEBUFFERNVPROC glad_glBlitFramebufferNV = NULL;
PFNGLBUFFERATTACHMEMORYNVPROC glad_glBufferAttachMemoryNV = NULL;
PFNGLBUFFERDATAPROC glad_glBufferData = NULL;
PFNGLBUFFERSTORAGEPROC glad_glBufferStorage = NULL;
PFNGLBUFFERSTORAGEEXTPROC glad_glBufferStorageEXT = NULL;
PFNGLBUFFERSTORAGEEXTERNALEXTPROC glad_glBufferStorageExternalEXT = NULL;
PFNGLBUFFERSTORAGEMEMEXTPROC glad_glBufferStorageMemEXT = NULL;
PFNGLBUFFERSUBDATAPROC glad_glBufferSubData = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glad_glCheckFramebufferStatus = NULL;
PFNGLCLEARPROC glad_glClear = NULL;
PFNGLCLEARBUFFERFIPROC glad_glClearBufferfi = NULL;
PFNGLCLEARBUFFERFVPROC glad_glClearBufferfv = NULL;
PFNGLCLEARBUFFERIVPROC glad_glClearBufferiv = NULL;
PFNGLCLEARBUFFERUIVPROC glad_glClearBufferuiv = NULL;
PFNGLCLEARCOLORPROC glad_glClearColor = NULL;
PFNGLCLEARDEPTHFPROC glad_glClearDepthf = NULL;
PFNGLCLEARPIXELLOCALSTORAGEUIEXTPROC glad_glClearPixelLocalStorageuiEXT = NULL;
PFNGLCLEARSTENCILPROC glad_glClearStencil = NULL;
PFNGLCLEARTEXIMAGEPROC glad_glClearTexImage = NULL;
PFNGLCLEARTEXIMAGEEXTPROC glad_glClearTexImageEXT = NULL;
PFNGLCLEARTEXSUBIMAGEPROC glad_glClearTexSubImage = NULL;
PFNGLCLEARTEXSUBIMAGEEXTPROC glad_glClearTexSubImageEXT = NULL;
PFNGLCLIENTWAITSYNCPROC glad_glClientWaitSync = NULL;
PFNGLCLIENTWAITSYNCAPPLEPROC glad_glClientWaitSyncAPPLE = NULL;
PFNGLCLIPCONTROLPROC glad_glClipControl = NULL;
PFNGLCLIPCONTROLEXTPROC glad_glClipControlEXT = NULL;
PFNGLCOLORMASKPROC glad_glColorMask = NULL;
PFNGLCOLORMASKIPROC glad_glColorMaski = NULL;
PFNGLCOLORMASKIEXTPROC glad_glColorMaskiEXT = NULL;
PFNGLCOLORMASKIOESPROC glad_glColorMaskiOES = NULL;
PFNGLCOMPILESHADERPROC glad_glCompileShader = NULL;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glad_glCompressedTexImage2D = NULL;
PFNGLCOMPRESSEDTEXIMAGE3DPROC glad_glCompressedTexImage3D = NULL;
PFNGLCOMPRESSEDTEXIMAGE3DOESPROC glad_glCompressedTexImage3DOES = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glad_glCompressedTexSubImage2D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC glad_glCompressedTexSubImage3D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC glad_glCompressedTexSubImage3DOES = NULL;
PFNGLCONSERVATIVERASTERPARAMETERINVPROC glad_glConservativeRasterParameteriNV = NULL;
PFNGLCOPYBUFFERSUBDATAPROC glad_glCopyBufferSubData = NULL;
PFNGLCOPYBUFFERSUBDATANVPROC glad_glCopyBufferSubDataNV = NULL;
PFNGLCOPYIMAGESUBDATAPROC glad_glCopyImageSubData = NULL;
PFNGLCOPYIMAGESUBDATAEXTPROC glad_glCopyImageSubDataEXT = NULL;
PFNGLCOPYIMAGESUBDATAOESPROC glad_glCopyImageSubDataOES = NULL;
PFNGLCOPYPATHNVPROC glad_glCopyPathNV = NULL;
PFNGLCOPYTEXIMAGE2DPROC glad_glCopyTexImage2D = NULL;
PFNGLCOPYTEXSUBIMAGE2DPROC glad_glCopyTexSubImage2D = NULL;
PFNGLCOPYTEXSUBIMAGE3DPROC glad_glCopyTexSubImage3D = NULL;
PFNGLCOPYTEXSUBIMAGE3DOESPROC glad_glCopyTexSubImage3DOES = NULL;
PFNGLCOPYTEXTURELEVELSAPPLEPROC glad_glCopyTextureLevelsAPPLE = NULL;
PFNGLCOVERFILLPATHINSTANCEDNVPROC glad_glCoverFillPathInstancedNV = NULL;
PFNGLCOVERFILLPATHNVPROC glad_glCoverFillPathNV = NULL;
PFNGLCOVERSTROKEPATHINSTANCEDNVPROC glad_glCoverStrokePathInstancedNV = NULL;
PFNGLCOVERSTROKEPATHNVPROC glad_glCoverStrokePathNV = NULL;
PFNGLCOVERAGEMASKNVPROC glad_glCoverageMaskNV = NULL;
PFNGLCOVERAGEMODULATIONNVPROC glad_glCoverageModulationNV = NULL;
PFNGLCOVERAGEMODULATIONTABLENVPROC glad_glCoverageModulationTableNV = NULL;
PFNGLCOVERAGEOPERATIONNVPROC glad_glCoverageOperationNV = NULL;
PFNGLCREATEMEMORYOBJECTSEXTPROC glad_glCreateMemoryObjectsEXT = NULL;
PFNGLCREATEPERFQUERYINTELPROC glad_glCreatePerfQueryINTEL = NULL;
PFNGLCREATEPROGRAMPROC glad_glCreateProgram = NULL;
PFNGLCREATESHADERPROC glad_glCreateShader = NULL;
PFNGLCREATESHADERPROGRAMVPROC glad_glCreateShaderProgramv = NULL;
PFNGLCREATESHADERPROGRAMVEXTPROC glad_glCreateShaderProgramvEXT = NULL;
PFNGLCULLFACEPROC glad_glCullFace = NULL;
PFNGLDEBUGMESSAGECALLBACKPROC glad_glDebugMessageCallback = NULL;
PFNGLDEBUGMESSAGECALLBACKKHRPROC glad_glDebugMessageCallbackKHR = NULL;
PFNGLDEBUGMESSAGECONTROLPROC glad_glDebugMessageControl = NULL;
PFNGLDEBUGMESSAGECONTROLKHRPROC glad_glDebugMessageControlKHR = NULL;
PFNGLDEBUGMESSAGEINSERTPROC glad_glDebugMessageInsert = NULL;
PFNGLDEBUGMESSAGEINSERTKHRPROC glad_glDebugMessageInsertKHR = NULL;
PFNGLDELETEBUFFERSPROC glad_glDeleteBuffers = NULL;
PFNGLDELETEFENCESNVPROC glad_glDeleteFencesNV = NULL;
PFNGLDELETEFRAMEBUFFERSPROC glad_glDeleteFramebuffers = NULL;
PFNGLDELETEMEMORYOBJECTSEXTPROC glad_glDeleteMemoryObjectsEXT = NULL;
PFNGLDELETEPATHSNVPROC glad_glDeletePathsNV = NULL;
PFNGLDELETEPERFMONITORSAMDPROC glad_glDeletePerfMonitorsAMD = NULL;
PFNGLDELETEPERFQUERYINTELPROC glad_glDeletePerfQueryINTEL = NULL;
PFNGLDELETEPROGRAMPROC glad_glDeleteProgram = NULL;
PFNGLDELETEPROGRAMPIPELINESPROC glad_glDeleteProgramPipelines = NULL;
PFNGLDELETEPROGRAMPIPELINESEXTPROC glad_glDeleteProgramPipelinesEXT = NULL;
PFNGLDELETEQUERIESPROC glad_glDeleteQueries = NULL;
PFNGLDELETEQUERIESEXTPROC glad_glDeleteQueriesEXT = NULL;
PFNGLDELETERENDERBUFFERSPROC glad_glDeleteRenderbuffers = NULL;
PFNGLDELETESAMPLERSPROC glad_glDeleteSamplers = NULL;
PFNGLDELETESEMAPHORESEXTPROC glad_glDeleteSemaphoresEXT = NULL;
PFNGLDELETESHADERPROC glad_glDeleteShader = NULL;
PFNGLDELETESYNCPROC glad_glDeleteSync = NULL;
PFNGLDELETESYNCAPPLEPROC glad_glDeleteSyncAPPLE = NULL;
PFNGLDELETETEXTURESPROC glad_glDeleteTextures = NULL;
PFNGLDELETETRANSFORMFEEDBACKSPROC glad_glDeleteTransformFeedbacks = NULL;
PFNGLDELETEVERTEXARRAYSPROC glad_glDeleteVertexArrays = NULL;
PFNGLDELETEVERTEXARRAYSOESPROC glad_glDeleteVertexArraysOES = NULL;
PFNGLDEPTHFUNCPROC glad_glDepthFunc = NULL;
PFNGLDEPTHMASKPROC glad_glDepthMask = NULL;
PFNGLDEPTHRANGEARRAYFVNVPROC glad_glDepthRangeArrayfvNV = NULL;
PFNGLDEPTHRANGEARRAYFVOESPROC glad_glDepthRangeArrayfvOES = NULL;
PFNGLDEPTHRANGEINDEXEDFNVPROC glad_glDepthRangeIndexedfNV = NULL;
PFNGLDEPTHRANGEINDEXEDFOESPROC glad_glDepthRangeIndexedfOES = NULL;
PFNGLDEPTHRANGEFPROC glad_glDepthRangef = NULL;
PFNGLDETACHSHADERPROC glad_glDetachShader = NULL;
PFNGLDISABLEPROC glad_glDisable = NULL;
PFNGLDISABLEDRIVERCONTROLQCOMPROC glad_glDisableDriverControlQCOM = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glad_glDisableVertexAttribArray = NULL;
PFNGLDISABLEIPROC glad_glDisablei = NULL;
PFNGLDISABLEIEXTPROC glad_glDisableiEXT = NULL;
PFNGLDISABLEINVPROC glad_glDisableiNV = NULL;
PFNGLDISABLEIOESPROC glad_glDisableiOES = NULL;
PFNGLDISCARDFRAMEBUFFEREXTPROC glad_glDiscardFramebufferEXT = NULL;
PFNGLDISPATCHCOMPUTEPROC glad_glDispatchCompute = NULL;
PFNGLDISPATCHCOMPUTEINDIRECTPROC glad_glDispatchComputeIndirect = NULL;
PFNGLDRAWARRAYSPROC glad_glDrawArrays = NULL;
PFNGLDRAWARRAYSINDIRECTPROC glad_glDrawArraysIndirect = NULL;
PFNGLDRAWARRAYSINSTANCEDPROC glad_glDrawArraysInstanced = NULL;
PFNGLDRAWARRAYSINSTANCEDANGLEPROC glad_glDrawArraysInstancedANGLE = NULL;
PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC glad_glDrawArraysInstancedBaseInstance = NULL;
PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEEXTPROC glad_glDrawArraysInstancedBaseInstanceEXT = NULL;
PFNGLDRAWARRAYSINSTANCEDEXTPROC glad_glDrawArraysInstancedEXT = NULL;
PFNGLDRAWARRAYSINSTANCEDNVPROC glad_glDrawArraysInstancedNV = NULL;
PFNGLDRAWBUFFERSPROC glad_glDrawBuffers = NULL;
PFNGLDRAWBUFFERSEXTPROC glad_glDrawBuffersEXT = NULL;
PFNGLDRAWBUFFERSINDEXEDEXTPROC glad_glDrawBuffersIndexedEXT = NULL;
PFNGLDRAWBUFFERSNVPROC glad_glDrawBuffersNV = NULL;
PFNGLDRAWELEMENTSPROC glad_glDrawElements = NULL;
PFNGLDRAWELEMENTSBASEVERTEXPROC glad_glDrawElementsBaseVertex = NULL;
PFNGLDRAWELEMENTSBASEVERTEXEXTPROC glad_glDrawElementsBaseVertexEXT = NULL;
PFNGLDRAWELEMENTSBASEVERTEXOESPROC glad_glDrawElementsBaseVertexOES = NULL;
PFNGLDRAWELEMENTSINDIRECTPROC glad_glDrawElementsIndirect = NULL;
PFNGLDRAWELEMENTSINSTANCEDPROC glad_glDrawElementsInstanced = NULL;
PFNGLDRAWELEMENTSINSTANCEDANGLEPROC glad_glDrawElementsInstancedANGLE = NULL;
PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC glad_glDrawElementsInstancedBaseInstance = NULL;
PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEEXTPROC glad_glDrawElementsInstancedBaseInstanceEXT = NULL;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC glad_glDrawElementsInstancedBaseVertex = NULL;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC glad_glDrawElementsInstancedBaseVertexBaseInstance = NULL;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEEXTPROC glad_glDrawElementsInstancedBaseVertexBaseInstanceEXT = NULL;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXEXTPROC glad_glDrawElementsInstancedBaseVertexEXT = NULL;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXOESPROC glad_glDrawElementsInstancedBaseVertexOES = NULL;
PFNGLDRAWELEMENTSINSTANCEDEXTPROC glad_glDrawElementsInstancedEXT = NULL;
PFNGLDRAWELEMENTSINSTANCEDNVPROC glad_glDrawElementsInstancedNV = NULL;
PFNGLDRAWRANGEELEMENTSPROC glad_glDrawRangeElements = NULL;
PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC glad_glDrawRangeElementsBaseVertex = NULL;
PFNGLDRAWRANGEELEMENTSBASEVERTEXEXTPROC glad_glDrawRangeElementsBaseVertexEXT = NULL;
PFNGLDRAWRANGEELEMENTSBASEVERTEXOESPROC glad_glDrawRangeElementsBaseVertexOES = NULL;
PFNGLDRAWTRANSFORMFEEDBACKPROC glad_glDrawTransformFeedback = NULL;
PFNGLDRAWTRANSFORMFEEDBACKEXTPROC glad_glDrawTransformFeedbackEXT = NULL;
PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC glad_glDrawTransformFeedbackInstanced = NULL;
PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDEXTPROC glad_glDrawTransformFeedbackInstancedEXT = NULL;
PFNGLDRAWVKIMAGENVPROC glad_glDrawVkImageNV = NULL;
PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC glad_glEGLImageTargetRenderbufferStorageOES = NULL;
PFNGLEGLIMAGETARGETTEXSTORAGEEXTPROC glad_glEGLImageTargetTexStorageEXT = NULL;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glad_glEGLImageTargetTexture2DOES = NULL;
PFNGLEGLIMAGETARGETTEXTURESTORAGEEXTPROC glad_glEGLImageTargetTextureStorageEXT = NULL;
PFNGLENABLEPROC glad_glEnable = NULL;
PFNGLENABLEDRIVERCONTROLQCOMPROC glad_glEnableDriverControlQCOM = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray = NULL;
PFNGLENABLEIPROC glad_glEnablei = NULL;
PFNGLENABLEIEXTPROC glad_glEnableiEXT = NULL;
PFNGLENABLEINVPROC glad_glEnableiNV = NULL;
PFNGLENABLEIOESPROC glad_glEnableiOES = NULL;
PFNGLENDCONDITIONALRENDERPROC glad_glEndConditionalRender = NULL;
PFNGLENDCONDITIONALRENDERNVPROC glad_glEndConditionalRenderNV = NULL;
PFNGLENDPERFMONITORAMDPROC glad_glEndPerfMonitorAMD = NULL;
PFNGLENDPERFQUERYINTELPROC glad_glEndPerfQueryINTEL = NULL;
PFNGLENDQUERYPROC glad_glEndQuery = NULL;
PFNGLENDQUERYEXTPROC glad_glEndQueryEXT = NULL;
PFNGLENDTILINGQCOMPROC glad_glEndTilingQCOM = NULL;
PFNGLENDTRANSFORMFEEDBACKPROC glad_glEndTransformFeedback = NULL;
PFNGLEXTGETBUFFERPOINTERVQCOMPROC glad_glExtGetBufferPointervQCOM = NULL;
PFNGLEXTGETBUFFERSQCOMPROC glad_glExtGetBuffersQCOM = NULL;
PFNGLEXTGETFRAMEBUFFERSQCOMPROC glad_glExtGetFramebuffersQCOM = NULL;
PFNGLEXTGETPROGRAMBINARYSOURCEQCOMPROC glad_glExtGetProgramBinarySourceQCOM = NULL;
PFNGLEXTGETPROGRAMSQCOMPROC glad_glExtGetProgramsQCOM = NULL;
PFNGLEXTGETRENDERBUFFERSQCOMPROC glad_glExtGetRenderbuffersQCOM = NULL;
PFNGLEXTGETSHADERSQCOMPROC glad_glExtGetShadersQCOM = NULL;
PFNGLEXTGETTEXLEVELPARAMETERIVQCOMPROC glad_glExtGetTexLevelParameterivQCOM = NULL;
PFNGLEXTGETTEXSUBIMAGEQCOMPROC glad_glExtGetTexSubImageQCOM = NULL;
PFNGLEXTGETTEXTURESQCOMPROC glad_glExtGetTexturesQCOM = NULL;
PFNGLEXTISPROGRAMBINARYQCOMPROC glad_glExtIsProgramBinaryQCOM = NULL;
PFNGLEXTTEXOBJECTSTATEOVERRIDEIQCOMPROC glad_glExtTexObjectStateOverrideiQCOM = NULL;
PFNGLFENCESYNCPROC glad_glFenceSync = NULL;
PFNGLFENCESYNCAPPLEPROC glad_glFenceSyncAPPLE = NULL;
PFNGLFINISHPROC glad_glFinish = NULL;
PFNGLFINISHFENCENVPROC glad_glFinishFenceNV = NULL;
PFNGLFLUSHPROC glad_glFlush = NULL;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC glad_glFlushMappedBufferRange = NULL;
PFNGLFLUSHMAPPEDBUFFERRANGEEXTPROC glad_glFlushMappedBufferRangeEXT = NULL;
PFNGLFRAGMENTCOVERAGECOLORNVPROC glad_glFragmentCoverageColorNV = NULL;
PFNGLFRAMEBUFFERFETCHBARRIEREXTPROC glad_glFramebufferFetchBarrierEXT = NULL;
PFNGLFRAMEBUFFERFETCHBARRIERQCOMPROC glad_glFramebufferFetchBarrierQCOM = NULL;
PFNGLFRAMEBUFFERFOVEATIONCONFIGQCOMPROC glad_glFramebufferFoveationConfigQCOM = NULL;
PFNGLFRAMEBUFFERFOVEATIONPARAMETERSQCOMPROC glad_glFramebufferFoveationParametersQCOM = NULL;
PFNGLFRAMEBUFFERPARAMETERIPROC glad_glFramebufferParameteri = NULL;
PFNGLFRAMEBUFFERPIXELLOCALSTORAGESIZEEXTPROC glad_glFramebufferPixelLocalStorageSizeEXT = NULL;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glad_glFramebufferRenderbuffer = NULL;
PFNGLFRAMEBUFFERSAMPLELOCATIONSFVNVPROC glad_glFramebufferSampleLocationsfvNV = NULL;
PFNGLFRAMEBUFFERTEXTUREPROC glad_glFramebufferTexture = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC glad_glFramebufferTexture2D = NULL;
PFNGLFRAMEBUFFERTEXTURE2DDOWNSAMPLEIMGPROC glad_glFramebufferTexture2DDownsampleIMG = NULL;
PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glad_glFramebufferTexture2DMultisampleEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC glad_glFramebufferTexture2DMultisampleIMG = NULL;
PFNGLFRAMEBUFFERTEXTURE3DOESPROC glad_glFramebufferTexture3DOES = NULL;
PFNGLFRAMEBUFFERTEXTUREEXTPROC glad_glFramebufferTextureEXT = NULL;
PFNGLFRAMEBUFFERTEXTURELAYERPROC glad_glFramebufferTextureLayer = NULL;
PFNGLFRAMEBUFFERTEXTURELAYERDOWNSAMPLEIMGPROC glad_glFramebufferTextureLayerDownsampleIMG = NULL;
PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVRPROC glad_glFramebufferTextureMultisampleMultiviewOVR = NULL;
PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC glad_glFramebufferTextureMultiviewOVR = NULL;
PFNGLFRAMEBUFFERTEXTUREOESPROC glad_glFramebufferTextureOES = NULL;
PFNGLFRONTFACEPROC glad_glFrontFace = NULL;
PFNGLGENBUFFERSPROC glad_glGenBuffers = NULL;
PFNGLGENFENCESNVPROC glad_glGenFencesNV = NULL;
PFNGLGENFRAMEBUFFERSPROC glad_glGenFramebuffers = NULL;
PFNGLGENPATHSNVPROC glad_glGenPathsNV = NULL;
PFNGLGENPERFMONITORSAMDPROC glad_glGenPerfMonitorsAMD = NULL;
PFNGLGENPROGRAMPIPELINESPROC glad_glGenProgramPipelines = NULL;
PFNGLGENPROGRAMPIPELINESEXTPROC glad_glGenProgramPipelinesEXT = NULL;
PFNGLGENQUERIESPROC glad_glGenQueries = NULL;
PFNGLGENQUERIESEXTPROC glad_glGenQueriesEXT = NULL;
PFNGLGENRENDERBUFFERSPROC glad_glGenRenderbuffers = NULL;
PFNGLGENSAMPLERSPROC glad_glGenSamplers = NULL;
PFNGLGENSEMAPHORESEXTPROC glad_glGenSemaphoresEXT = NULL;
PFNGLGENTEXTURESPROC glad_glGenTextures = NULL;
PFNGLGENTRANSFORMFEEDBACKSPROC glad_glGenTransformFeedbacks = NULL;
PFNGLGENVERTEXARRAYSPROC glad_glGenVertexArrays = NULL;
PFNGLGENVERTEXARRAYSOESPROC glad_glGenVertexArraysOES = NULL;
PFNGLGENERATEMIPMAPPROC glad_glGenerateMipmap = NULL;
PFNGLGETACTIVEATTRIBPROC glad_glGetActiveAttrib = NULL;
PFNGLGETACTIVEUNIFORMPROC glad_glGetActiveUniform = NULL;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glad_glGetActiveUniformBlockName = NULL;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC glad_glGetActiveUniformBlockiv = NULL;
PFNGLGETACTIVEUNIFORMSIVPROC glad_glGetActiveUniformsiv = NULL;
PFNGLGETATTACHEDSHADERSPROC glad_glGetAttachedShaders = NULL;
PFNGLGETATTRIBLOCATIONPROC glad_glGetAttribLocation = NULL;
PFNGLGETBOOLEANI_VPROC glad_glGetBooleani_v = NULL;
PFNGLGETBOOLEANVPROC glad_glGetBooleanv = NULL;
PFNGLGETBUFFERPARAMETERI64VPROC glad_glGetBufferParameteri64v = NULL;
PFNGLGETBUFFERPARAMETERIVPROC glad_glGetBufferParameteriv = NULL;
PFNGLGETBUFFERPOINTERVPROC glad_glGetBufferPointerv = NULL;
PFNGLGETBUFFERPOINTERVOESPROC glad_glGetBufferPointervOES = NULL;
PFNGLGETCOVERAGEMODULATIONTABLENVPROC glad_glGetCoverageModulationTableNV = NULL;
PFNGLGETDEBUGMESSAGELOGPROC glad_glGetDebugMessageLog = NULL;
PFNGLGETDEBUGMESSAGELOGKHRPROC glad_glGetDebugMessageLogKHR = NULL;
PFNGLGETDRIVERCONTROLSTRINGQCOMPROC glad_glGetDriverControlStringQCOM = NULL;
PFNGLGETDRIVERCONTROLSQCOMPROC glad_glGetDriverControlsQCOM = NULL;
PFNGLGETERRORPROC glad_glGetError = NULL;
PFNGLGETFENCEIVNVPROC glad_glGetFenceivNV = NULL;
PFNGLGETFIRSTPERFQUERYIDINTELPROC glad_glGetFirstPerfQueryIdINTEL = NULL;
PFNGLGETFLOATI_VPROC glad_glGetFloati_v = NULL;
PFNGLGETFLOATI_VNVPROC glad_glGetFloati_vNV = NULL;
PFNGLGETFLOATI_VOESPROC glad_glGetFloati_vOES = NULL;
PFNGLGETFLOATVPROC glad_glGetFloatv = NULL;
PFNGLGETFRAGDATAINDEXPROC glad_glGetFragDataIndex = NULL;
PFNGLGETFRAGDATAINDEXEXTPROC glad_glGetFragDataIndexEXT = NULL;
PFNGLGETFRAGDATALOCATIONPROC glad_glGetFragDataLocation = NULL;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glad_glGetFramebufferAttachmentParameteriv = NULL;
PFNGLGETFRAMEBUFFERPARAMETERIVPROC glad_glGetFramebufferParameteriv = NULL;
PFNGLGETFRAMEBUFFERPIXELLOCALSTORAGESIZEEXTPROC glad_glGetFramebufferPixelLocalStorageSizeEXT = NULL;
PFNGLGETGRAPHICSRESETSTATUSPROC glad_glGetGraphicsResetStatus = NULL;
PFNGLGETGRAPHICSRESETSTATUSEXTPROC glad_glGetGraphicsResetStatusEXT = NULL;
PFNGLGETGRAPHICSRESETSTATUSKHRPROC glad_glGetGraphicsResetStatusKHR = NULL;
PFNGLGETIMAGEHANDLENVPROC glad_glGetImageHandleNV = NULL;
PFNGLGETINTEGER64I_VPROC glad_glGetInteger64i_v = NULL;
PFNGLGETINTEGER64VPROC glad_glGetInteger64v = NULL;
PFNGLGETINTEGER64VAPPLEPROC glad_glGetInteger64vAPPLE = NULL;
PFNGLGETINTEGERI_VPROC glad_glGetIntegeri_v = NULL;
PFNGLGETINTEGERI_VEXTPROC glad_glGetIntegeri_vEXT = NULL;
PFNGLGETINTEGERVPROC glad_glGetIntegerv = NULL;
PFNGLGETINTERNALFORMATSAMPLEIVNVPROC glad_glGetInternalformatSampleivNV = NULL;
PFNGLGETINTERNALFORMATIVPROC glad_glGetInternalformativ = NULL;
PFNGLGETMEMORYOBJECTDETACHEDRESOURCESUIVNVPROC glad_glGetMemoryObjectDetachedResourcesuivNV = NULL;
PFNGLGETMEMORYOBJECTPARAMETERIVEXTPROC glad_glGetMemoryObjectParameterivEXT = NULL;
PFNGLGETMULTISAMPLEFVPROC glad_glGetMultisamplefv = NULL;
PFNGLGETNEXTPERFQUERYIDINTELPROC glad_glGetNextPerfQueryIdINTEL = NULL;
PFNGLGETOBJECTLABELPROC glad_glGetObjectLabel = NULL;
PFNGLGETOBJECTLABELEXTPROC glad_glGetObjectLabelEXT = NULL;
PFNGLGETOBJECTLABELKHRPROC glad_glGetObjectLabelKHR = NULL;
PFNGLGETOBJECTPTRLABELPROC glad_glGetObjectPtrLabel = NULL;
PFNGLGETOBJECTPTRLABELKHRPROC glad_glGetObjectPtrLabelKHR = NULL;
PFNGLGETPATHCOMMANDSNVPROC glad_glGetPathCommandsNV = NULL;
PFNGLGETPATHCOORDSNVPROC glad_glGetPathCoordsNV = NULL;
PFNGLGETPATHDASHARRAYNVPROC glad_glGetPathDashArrayNV = NULL;
PFNGLGETPATHLENGTHNVPROC glad_glGetPathLengthNV = NULL;
PFNGLGETPATHMETRICRANGENVPROC glad_glGetPathMetricRangeNV = NULL;
PFNGLGETPATHMETRICSNVPROC glad_glGetPathMetricsNV = NULL;
PFNGLGETPATHPARAMETERFVNVPROC glad_glGetPathParameterfvNV = NULL;
PFNGLGETPATHPARAMETERIVNVPROC glad_glGetPathParameterivNV = NULL;
PFNGLGETPATHSPACINGNVPROC glad_glGetPathSpacingNV = NULL;
PFNGLGETPERFCOUNTERINFOINTELPROC glad_glGetPerfCounterInfoINTEL = NULL;
PFNGLGETPERFMONITORCOUNTERDATAAMDPROC glad_glGetPerfMonitorCounterDataAMD = NULL;
PFNGLGETPERFMONITORCOUNTERINFOAMDPROC glad_glGetPerfMonitorCounterInfoAMD = NULL;
PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC glad_glGetPerfMonitorCounterStringAMD = NULL;
PFNGLGETPERFMONITORCOUNTERSAMDPROC glad_glGetPerfMonitorCountersAMD = NULL;
PFNGLGETPERFMONITORGROUPSTRINGAMDPROC glad_glGetPerfMonitorGroupStringAMD = NULL;
PFNGLGETPERFMONITORGROUPSAMDPROC glad_glGetPerfMonitorGroupsAMD = NULL;
PFNGLGETPERFQUERYDATAINTELPROC glad_glGetPerfQueryDataINTEL = NULL;
PFNGLGETPERFQUERYIDBYNAMEINTELPROC glad_glGetPerfQueryIdByNameINTEL = NULL;
PFNGLGETPERFQUERYINFOINTELPROC glad_glGetPerfQueryInfoINTEL = NULL;
PFNGLGETPOINTERVPROC glad_glGetPointerv = NULL;
PFNGLGETPOINTERVKHRPROC glad_glGetPointervKHR = NULL;
PFNGLGETPROGRAMBINARYPROC glad_glGetProgramBinary = NULL;
PFNGLGETPROGRAMBINARYOESPROC glad_glGetProgramBinaryOES = NULL;
PFNGLGETPROGRAMINFOLOGPROC glad_glGetProgramInfoLog = NULL;
PFNGLGETPROGRAMINTERFACEIVPROC glad_glGetProgramInterfaceiv = NULL;
PFNGLGETPROGRAMPIPELINEINFOLOGPROC glad_glGetProgramPipelineInfoLog = NULL;
PFNGLGETPROGRAMPIPELINEINFOLOGEXTPROC glad_glGetProgramPipelineInfoLogEXT = NULL;
PFNGLGETPROGRAMPIPELINEIVPROC glad_glGetProgramPipelineiv = NULL;
PFNGLGETPROGRAMPIPELINEIVEXTPROC glad_glGetProgramPipelineivEXT = NULL;
PFNGLGETPROGRAMRESOURCEINDEXPROC glad_glGetProgramResourceIndex = NULL;
PFNGLGETPROGRAMRESOURCELOCATIONPROC glad_glGetProgramResourceLocation = NULL;
PFNGLGETPROGRAMRESOURCELOCATIONINDEXEXTPROC glad_glGetProgramResourceLocationIndexEXT = NULL;
PFNGLGETPROGRAMRESOURCENAMEPROC glad_glGetProgramResourceName = NULL;
PFNGLGETPROGRAMRESOURCEFVNVPROC glad_glGetProgramResourcefvNV = NULL;
PFNGLGETPROGRAMRESOURCEIVPROC glad_glGetProgramResourceiv = NULL;
PFNGLGETPROGRAMIVPROC glad_glGetProgramiv = NULL;
PFNGLGETQUERYOBJECTI64VPROC glad_glGetQueryObjecti64v = NULL;
PFNGLGETQUERYOBJECTI64VEXTPROC glad_glGetQueryObjecti64vEXT = NULL;
PFNGLGETQUERYOBJECTIVPROC glad_glGetQueryObjectiv = NULL;
PFNGLGETQUERYOBJECTIVEXTPROC glad_glGetQueryObjectivEXT = NULL;
PFNGLGETQUERYOBJECTUI64VPROC glad_glGetQueryObjectui64v = NULL;
PFNGLGETQUERYOBJECTUI64VEXTPROC glad_glGetQueryObjectui64vEXT = NULL;
PFNGLGETQUERYOBJECTUIVPROC glad_glGetQueryObjectuiv = NULL;
PFNGLGETQUERYOBJECTUIVEXTPROC glad_glGetQueryObjectuivEXT = NULL;
PFNGLGETQUERYIVPROC glad_glGetQueryiv = NULL;
PFNGLGETQUERYIVEXTPROC glad_glGetQueryivEXT = NULL;
PFNGLGETRENDERBUFFERPARAMETERIVPROC glad_glGetRenderbufferParameteriv = NULL;
PFNGLGETSAMPLERPARAMETERIIVPROC glad_glGetSamplerParameterIiv = NULL;
PFNGLGETSAMPLERPARAMETERIIVEXTPROC glad_glGetSamplerParameterIivEXT = NULL;
PFNGLGETSAMPLERPARAMETERIIVOESPROC glad_glGetSamplerParameterIivOES = NULL;
PFNGLGETSAMPLERPARAMETERIUIVPROC glad_glGetSamplerParameterIuiv = NULL;
PFNGLGETSAMPLERPARAMETERIUIVEXTPROC glad_glGetSamplerParameterIuivEXT = NULL;
PFNGLGETSAMPLERPARAMETERIUIVOESPROC glad_glGetSamplerParameterIuivOES = NULL;
PFNGLGETSAMPLERPARAMETERFVPROC glad_glGetSamplerParameterfv = NULL;
PFNGLGETSAMPLERPARAMETERIVPROC glad_glGetSamplerParameteriv = NULL;
PFNGLGETSEMAPHOREPARAMETERUI64VEXTPROC glad_glGetSemaphoreParameterui64vEXT = NULL;
PFNGLGETSHADERINFOLOGPROC glad_glGetShaderInfoLog = NULL;
PFNGLGETSHADERPRECISIONFORMATPROC glad_glGetShaderPrecisionFormat = NULL;
PFNGLGETSHADERSOURCEPROC glad_glGetShaderSource = NULL;
PFNGLGETSHADERIVPROC glad_glGetShaderiv = NULL;
PFNGLGETSTRINGPROC glad_glGetString = NULL;
PFNGLGETSTRINGIPROC glad_glGetStringi = NULL;
PFNGLGETSYNCIVPROC glad_glGetSynciv = NULL;
PFNGLGETSYNCIVAPPLEPROC glad_glGetSyncivAPPLE = NULL;
PFNGLGETTEXLEVELPARAMETERFVPROC glad_glGetTexLevelParameterfv = NULL;
PFNGLGETTEXLEVELPARAMETERIVPROC glad_glGetTexLevelParameteriv = NULL;
PFNGLGETTEXPARAMETERIIVPROC glad_glGetTexParameterIiv = NULL;
PFNGLGETTEXPARAMETERIIVEXTPROC glad_glGetTexParameterIivEXT = NULL;
PFNGLGETTEXPARAMETERIIVOESPROC glad_glGetTexParameterIivOES = NULL;
PFNGLGETTEXPARAMETERIUIVPROC glad_glGetTexParameterIuiv = NULL;
PFNGLGETTEXPARAMETERIUIVEXTPROC glad_glGetTexParameterIuivEXT = NULL;
PFNGLGETTEXPARAMETERIUIVOESPROC glad_glGetTexParameterIuivOES = NULL;
PFNGLGETTEXPARAMETERFVPROC glad_glGetTexParameterfv = NULL;
PFNGLGETTEXPARAMETERIVPROC glad_glGetTexParameteriv = NULL;
PFNGLGETTEXTUREHANDLEARBPROC glad_glGetTextureHandleARB = NULL;
PFNGLGETTEXTUREHANDLEIMGPROC glad_glGetTextureHandleIMG = NULL;
PFNGLGETTEXTUREHANDLENVPROC glad_glGetTextureHandleNV = NULL;
PFNGLGETTEXTURESAMPLERHANDLEARBPROC glad_glGetTextureSamplerHandleARB = NULL;
PFNGLGETTEXTURESAMPLERHANDLEIMGPROC glad_glGetTextureSamplerHandleIMG = NULL;
PFNGLGETTEXTURESAMPLERHANDLENVPROC glad_glGetTextureSamplerHandleNV = NULL;
PFNGLGETTRANSFORMFEEDBACKVARYINGPROC glad_glGetTransformFeedbackVarying = NULL;
PFNGLGETTRANSLATEDSHADERSOURCEANGLEPROC glad_glGetTranslatedShaderSourceANGLE = NULL;
PFNGLGETUNIFORMBLOCKINDEXPROC glad_glGetUniformBlockIndex = NULL;
PFNGLGETUNIFORMINDICESPROC glad_glGetUniformIndices = NULL;
PFNGLGETUNIFORMLOCATIONPROC glad_glGetUniformLocation = NULL;
PFNGLGETUNIFORMFVPROC glad_glGetUniformfv = NULL;
PFNGLGETUNIFORMI64VNVPROC glad_glGetUniformi64vNV = NULL;
PFNGLGETUNIFORMIVPROC glad_glGetUniformiv = NULL;
PFNGLGETUNIFORMUIVPROC glad_glGetUniformuiv = NULL;
PFNGLGETUNSIGNEDBYTEI_VEXTPROC glad_glGetUnsignedBytei_vEXT = NULL;
PFNGLGETUNSIGNEDBYTEVEXTPROC glad_glGetUnsignedBytevEXT = NULL;
PFNGLGETVERTEXATTRIBIIVPROC glad_glGetVertexAttribIiv = NULL;
PFNGLGETVERTEXATTRIBIUIVPROC glad_glGetVertexAttribIuiv = NULL;
PFNGLGETVERTEXATTRIBPOINTERVPROC glad_glGetVertexAttribPointerv = NULL;
PFNGLGETVERTEXATTRIBFVPROC glad_glGetVertexAttribfv = NULL;
PFNGLGETVERTEXATTRIBIVPROC glad_glGetVertexAttribiv = NULL;
PFNGLGETVKPROCADDRNVPROC glad_glGetVkProcAddrNV = NULL;
PFNGLGETNUNIFORMFVPROC glad_glGetnUniformfv = NULL;
PFNGLGETNUNIFORMFVEXTPROC glad_glGetnUniformfvEXT = NULL;
PFNGLGETNUNIFORMFVKHRPROC glad_glGetnUniformfvKHR = NULL;
PFNGLGETNUNIFORMIVPROC glad_glGetnUniformiv = NULL;
PFNGLGETNUNIFORMIVEXTPROC glad_glGetnUniformivEXT = NULL;
PFNGLGETNUNIFORMIVKHRPROC glad_glGetnUniformivKHR = NULL;
PFNGLGETNUNIFORMUIVPROC glad_glGetnUniformuiv = NULL;
PFNGLGETNUNIFORMUIVKHRPROC glad_glGetnUniformuivKHR = NULL;
PFNGLHINTPROC glad_glHint = NULL;
PFNGLIMPORTMEMORYFDEXTPROC glad_glImportMemoryFdEXT = NULL;
PFNGLIMPORTMEMORYWIN32HANDLEEXTPROC glad_glImportMemoryWin32HandleEXT = NULL;
PFNGLIMPORTMEMORYWIN32NAMEEXTPROC glad_glImportMemoryWin32NameEXT = NULL;
PFNGLIMPORTSEMAPHOREFDEXTPROC glad_glImportSemaphoreFdEXT = NULL;
PFNGLIMPORTSEMAPHOREWIN32HANDLEEXTPROC glad_glImportSemaphoreWin32HandleEXT = NULL;
PFNGLIMPORTSEMAPHOREWIN32NAMEEXTPROC glad_glImportSemaphoreWin32NameEXT = NULL;
PFNGLINSERTEVENTMARKEREXTPROC glad_glInsertEventMarkerEXT = NULL;
PFNGLINTERPOLATEPATHSNVPROC glad_glInterpolatePathsNV = NULL;
PFNGLINVALIDATEFRAMEBUFFERPROC glad_glInvalidateFramebuffer = NULL;
PFNGLINVALIDATESUBFRAMEBUFFERPROC glad_glInvalidateSubFramebuffer = NULL;
PFNGLISBUFFERPROC glad_glIsBuffer = NULL;
PFNGLISENABLEDPROC glad_glIsEnabled = NULL;
PFNGLISENABLEDIPROC glad_glIsEnabledi = NULL;
PFNGLISENABLEDIEXTPROC glad_glIsEnablediEXT = NULL;
PFNGLISENABLEDINVPROC glad_glIsEnablediNV = NULL;
PFNGLISENABLEDIOESPROC glad_glIsEnablediOES = NULL;
PFNGLISFENCENVPROC glad_glIsFenceNV = NULL;
PFNGLISFRAMEBUFFERPROC glad_glIsFramebuffer = NULL;
PFNGLISIMAGEHANDLERESIDENTNVPROC glad_glIsImageHandleResidentNV = NULL;
PFNGLISMEMORYOBJECTEXTPROC glad_glIsMemoryObjectEXT = NULL;
PFNGLISPATHNVPROC glad_glIsPathNV = NULL;
PFNGLISPOINTINFILLPATHNVPROC glad_glIsPointInFillPathNV = NULL;
PFNGLISPOINTINSTROKEPATHNVPROC glad_glIsPointInStrokePathNV = NULL;
PFNGLISPROGRAMPROC glad_glIsProgram = NULL;
PFNGLISPROGRAMPIPELINEPROC glad_glIsProgramPipeline = NULL;
PFNGLISPROGRAMPIPELINEEXTPROC glad_glIsProgramPipelineEXT = NULL;
PFNGLISQUERYPROC glad_glIsQuery = NULL;
PFNGLISQUERYEXTPROC glad_glIsQueryEXT = NULL;
PFNGLISRENDERBUFFERPROC glad_glIsRenderbuffer = NULL;
PFNGLISSAMPLERPROC glad_glIsSampler = NULL;
PFNGLISSEMAPHOREEXTPROC glad_glIsSemaphoreEXT = NULL;
PFNGLISSHADERPROC glad_glIsShader = NULL;
PFNGLISSYNCPROC glad_glIsSync = NULL;
PFNGLISSYNCAPPLEPROC glad_glIsSyncAPPLE = NULL;
PFNGLISTEXTUREPROC glad_glIsTexture = NULL;
PFNGLISTEXTUREHANDLERESIDENTNVPROC glad_glIsTextureHandleResidentNV = NULL;
PFNGLISTRANSFORMFEEDBACKPROC glad_glIsTransformFeedback = NULL;
PFNGLISVERTEXARRAYPROC glad_glIsVertexArray = NULL;
PFNGLISVERTEXARRAYOESPROC glad_glIsVertexArrayOES = NULL;
PFNGLLABELOBJECTEXTPROC glad_glLabelObjectEXT = NULL;
PFNGLLINEWIDTHPROC glad_glLineWidth = NULL;
PFNGLLINKPROGRAMPROC glad_glLinkProgram = NULL;
PFNGLMAKEIMAGEHANDLENONRESIDENTNVPROC glad_glMakeImageHandleNonResidentNV = NULL;
PFNGLMAKEIMAGEHANDLERESIDENTNVPROC glad_glMakeImageHandleResidentNV = NULL;
PFNGLMAKETEXTUREHANDLENONRESIDENTNVPROC glad_glMakeTextureHandleNonResidentNV = NULL;
PFNGLMAKETEXTUREHANDLERESIDENTNVPROC glad_glMakeTextureHandleResidentNV = NULL;
PFNGLMAPBUFFERPROC glad_glMapBuffer = NULL;
PFNGLMAPBUFFEROESPROC glad_glMapBufferOES = NULL;
PFNGLMAPBUFFERRANGEPROC glad_glMapBufferRange = NULL;
PFNGLMAPBUFFERRANGEEXTPROC glad_glMapBufferRangeEXT = NULL;
PFNGLMATRIXFRUSTUMEXTPROC glad_glMatrixFrustumEXT = NULL;
PFNGLMATRIXLOAD3X2FNVPROC glad_glMatrixLoad3x2fNV = NULL;
PFNGLMATRIXLOAD3X3FNVPROC glad_glMatrixLoad3x3fNV = NULL;
PFNGLMATRIXLOADIDENTITYEXTPROC glad_glMatrixLoadIdentityEXT = NULL;
PFNGLMATRIXLOADTRANSPOSE3X3FNVPROC glad_glMatrixLoadTranspose3x3fNV = NULL;
PFNGLMATRIXLOADTRANSPOSEDEXTPROC glad_glMatrixLoadTransposedEXT = NULL;
PFNGLMATRIXLOADTRANSPOSEFEXTPROC glad_glMatrixLoadTransposefEXT = NULL;
PFNGLMATRIXLOADDEXTPROC glad_glMatrixLoaddEXT = NULL;
PFNGLMATRIXLOADFEXTPROC glad_glMatrixLoadfEXT = NULL;
PFNGLMATRIXMULT3X2FNVPROC glad_glMatrixMult3x2fNV = NULL;
PFNGLMATRIXMULT3X3FNVPROC glad_glMatrixMult3x3fNV = NULL;
PFNGLMATRIXMULTTRANSPOSE3X3FNVPROC glad_glMatrixMultTranspose3x3fNV = NULL;
PFNGLMATRIXMULTTRANSPOSEDEXTPROC glad_glMatrixMultTransposedEXT = NULL;
PFNGLMATRIXMULTTRANSPOSEFEXTPROC glad_glMatrixMultTransposefEXT = NULL;
PFNGLMATRIXMULTDEXTPROC glad_glMatrixMultdEXT = NULL;
PFNGLMATRIXMULTFEXTPROC glad_glMatrixMultfEXT = NULL;
PFNGLMATRIXORTHOEXTPROC glad_glMatrixOrthoEXT = NULL;
PFNGLMATRIXPOPEXTPROC glad_glMatrixPopEXT = NULL;
PFNGLMATRIXPUSHEXTPROC glad_glMatrixPushEXT = NULL;
PFNGLMATRIXROTATEDEXTPROC glad_glMatrixRotatedEXT = NULL;
PFNGLMATRIXROTATEFEXTPROC glad_glMatrixRotatefEXT = NULL;
PFNGLMATRIXSCALEDEXTPROC glad_glMatrixScaledEXT = NULL;
PFNGLMATRIXSCALEFEXTPROC glad_glMatrixScalefEXT = NULL;
PFNGLMATRIXTRANSLATEDEXTPROC glad_glMatrixTranslatedEXT = NULL;
PFNGLMATRIXTRANSLATEFEXTPROC glad_glMatrixTranslatefEXT = NULL;
PFNGLMAXSHADERCOMPILERTHREADSKHRPROC glad_glMaxShaderCompilerThreadsKHR = NULL;
PFNGLMEMORYBARRIERPROC glad_glMemoryBarrier = NULL;
PFNGLMEMORYBARRIERBYREGIONPROC glad_glMemoryBarrierByRegion = NULL;
PFNGLMEMORYOBJECTPARAMETERIVEXTPROC glad_glMemoryObjectParameterivEXT = NULL;
PFNGLMINSAMPLESHADINGPROC glad_glMinSampleShading = NULL;
PFNGLMINSAMPLESHADINGOESPROC glad_glMinSampleShadingOES = NULL;
PFNGLMULTIDRAWARRAYSPROC glad_glMultiDrawArrays = NULL;
PFNGLMULTIDRAWARRAYSEXTPROC glad_glMultiDrawArraysEXT = NULL;
PFNGLMULTIDRAWARRAYSINDIRECTPROC glad_glMultiDrawArraysIndirect = NULL;
PFNGLMULTIDRAWARRAYSINDIRECTEXTPROC glad_glMultiDrawArraysIndirectEXT = NULL;
PFNGLMULTIDRAWELEMENTSPROC glad_glMultiDrawElements = NULL;
PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC glad_glMultiDrawElementsBaseVertex = NULL;
PFNGLMULTIDRAWELEMENTSBASEVERTEXEXTPROC glad_glMultiDrawElementsBaseVertexEXT = NULL;
PFNGLMULTIDRAWELEMENTSEXTPROC glad_glMultiDrawElementsEXT = NULL;
PFNGLMULTIDRAWELEMENTSINDIRECTPROC glad_glMultiDrawElementsIndirect = NULL;
PFNGLMULTIDRAWELEMENTSINDIRECTEXTPROC glad_glMultiDrawElementsIndirectEXT = NULL;
PFNGLNAMEDBUFFERATTACHMEMORYNVPROC glad_glNamedBufferAttachMemoryNV = NULL;
PFNGLNAMEDBUFFERSTORAGEEXTERNALEXTPROC glad_glNamedBufferStorageExternalEXT = NULL;
PFNGLNAMEDBUFFERSTORAGEMEMEXTPROC glad_glNamedBufferStorageMemEXT = NULL;
PFNGLNAMEDFRAMEBUFFERSAMPLELOCATIONSFVNVPROC glad_glNamedFramebufferSampleLocationsfvNV = NULL;
PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEADVANCEDAMDPROC glad_glNamedRenderbufferStorageMultisampleAdvancedAMD = NULL;
PFNGLOBJECTLABELPROC glad_glObjectLabel = NULL;
PFNGLOBJECTLABELKHRPROC glad_glObjectLabelKHR = NULL;
PFNGLOBJECTPTRLABELPROC glad_glObjectPtrLabel = NULL;
PFNGLOBJECTPTRLABELKHRPROC glad_glObjectPtrLabelKHR = NULL;
PFNGLPATCHPARAMETERIPROC glad_glPatchParameteri = NULL;
PFNGLPATCHPARAMETERIEXTPROC glad_glPatchParameteriEXT = NULL;
PFNGLPATCHPARAMETERIOESPROC glad_glPatchParameteriOES = NULL;
PFNGLPATHCOMMANDSNVPROC glad_glPathCommandsNV = NULL;
PFNGLPATHCOORDSNVPROC glad_glPathCoordsNV = NULL;
PFNGLPATHCOVERDEPTHFUNCNVPROC glad_glPathCoverDepthFuncNV = NULL;
PFNGLPATHDASHARRAYNVPROC glad_glPathDashArrayNV = NULL;
PFNGLPATHGLYPHINDEXARRAYNVPROC glad_glPathGlyphIndexArrayNV = NULL;
PFNGLPATHGLYPHINDEXRANGENVPROC glad_glPathGlyphIndexRangeNV = NULL;
PFNGLPATHGLYPHRANGENVPROC glad_glPathGlyphRangeNV = NULL;
PFNGLPATHGLYPHSNVPROC glad_glPathGlyphsNV = NULL;
PFNGLPATHMEMORYGLYPHINDEXARRAYNVPROC glad_glPathMemoryGlyphIndexArrayNV = NULL;
PFNGLPATHPARAMETERFNVPROC glad_glPathParameterfNV = NULL;
PFNGLPATHPARAMETERFVNVPROC glad_glPathParameterfvNV = NULL;
PFNGLPATHPARAMETERINVPROC glad_glPathParameteriNV = NULL;
PFNGLPATHPARAMETERIVNVPROC glad_glPathParameterivNV = NULL;
PFNGLPATHSTENCILDEPTHOFFSETNVPROC glad_glPathStencilDepthOffsetNV = NULL;
PFNGLPATHSTENCILFUNCNVPROC glad_glPathStencilFuncNV = NULL;
PFNGLPATHSTRINGNVPROC glad_glPathStringNV = NULL;
PFNGLPATHSUBCOMMANDSNVPROC glad_glPathSubCommandsNV = NULL;
PFNGLPATHSUBCOORDSNVPROC glad_glPathSubCoordsNV = NULL;
PFNGLPAUSETRANSFORMFEEDBACKPROC glad_glPauseTransformFeedback = NULL;
PFNGLPIXELSTOREIPROC glad_glPixelStorei = NULL;
PFNGLPOINTALONGPATHNVPROC glad_glPointAlongPathNV = NULL;
PFNGLPOLYGONMODEPROC glad_glPolygonMode = NULL;
PFNGLPOLYGONMODENVPROC glad_glPolygonModeNV = NULL;
PFNGLPOLYGONOFFSETPROC glad_glPolygonOffset = NULL;
PFNGLPOLYGONOFFSETCLAMPPROC glad_glPolygonOffsetClamp = NULL;
PFNGLPOLYGONOFFSETCLAMPEXTPROC glad_glPolygonOffsetClampEXT = NULL;
PFNGLPOPDEBUGGROUPPROC glad_glPopDebugGroup = NULL;
PFNGLPOPDEBUGGROUPKHRPROC glad_glPopDebugGroupKHR = NULL;
PFNGLPOPGROUPMARKEREXTPROC glad_glPopGroupMarkerEXT = NULL;
PFNGLPRIMITIVEBOUNDINGBOXPROC glad_glPrimitiveBoundingBox = NULL;
PFNGLPRIMITIVEBOUNDINGBOXEXTPROC glad_glPrimitiveBoundingBoxEXT = NULL;
PFNGLPRIMITIVEBOUNDINGBOXOESPROC glad_glPrimitiveBoundingBoxOES = NULL;
PFNGLPROGRAMBINARYPROC glad_glProgramBinary = NULL;
PFNGLPROGRAMBINARYOESPROC glad_glProgramBinaryOES = NULL;
PFNGLPROGRAMPARAMETERIPROC glad_glProgramParameteri = NULL;
PFNGLPROGRAMPARAMETERIEXTPROC glad_glProgramParameteriEXT = NULL;
PFNGLPROGRAMPATHFRAGMENTINPUTGENNVPROC glad_glProgramPathFragmentInputGenNV = NULL;
PFNGLPROGRAMUNIFORM1FPROC glad_glProgramUniform1f = NULL;
PFNGLPROGRAMUNIFORM1FEXTPROC glad_glProgramUniform1fEXT = NULL;
PFNGLPROGRAMUNIFORM1FVPROC glad_glProgramUniform1fv = NULL;
PFNGLPROGRAMUNIFORM1FVEXTPROC glad_glProgramUniform1fvEXT = NULL;
PFNGLPROGRAMUNIFORM1IPROC glad_glProgramUniform1i = NULL;
PFNGLPROGRAMUNIFORM1I64NVPROC glad_glProgramUniform1i64NV = NULL;
PFNGLPROGRAMUNIFORM1I64VNVPROC glad_glProgramUniform1i64vNV = NULL;
PFNGLPROGRAMUNIFORM1IEXTPROC glad_glProgramUniform1iEXT = NULL;
PFNGLPROGRAMUNIFORM1IVPROC glad_glProgramUniform1iv = NULL;
PFNGLPROGRAMUNIFORM1IVEXTPROC glad_glProgramUniform1ivEXT = NULL;
PFNGLPROGRAMUNIFORM1UIPROC glad_glProgramUniform1ui = NULL;
PFNGLPROGRAMUNIFORM1UI64NVPROC glad_glProgramUniform1ui64NV = NULL;
PFNGLPROGRAMUNIFORM1UI64VNVPROC glad_glProgramUniform1ui64vNV = NULL;
PFNGLPROGRAMUNIFORM1UIEXTPROC glad_glProgramUniform1uiEXT = NULL;
PFNGLPROGRAMUNIFORM1UIVPROC glad_glProgramUniform1uiv = NULL;
PFNGLPROGRAMUNIFORM1UIVEXTPROC glad_glProgramUniform1uivEXT = NULL;
PFNGLPROGRAMUNIFORM2FPROC glad_glProgramUniform2f = NULL;
PFNGLPROGRAMUNIFORM2FEXTPROC glad_glProgramUniform2fEXT = NULL;
PFNGLPROGRAMUNIFORM2FVPROC glad_glProgramUniform2fv = NULL;
PFNGLPROGRAMUNIFORM2FVEXTPROC glad_glProgramUniform2fvEXT = NULL;
PFNGLPROGRAMUNIFORM2IPROC glad_glProgramUniform2i = NULL;
PFNGLPROGRAMUNIFORM2I64NVPROC glad_glProgramUniform2i64NV = NULL;
PFNGLPROGRAMUNIFORM2I64VNVPROC glad_glProgramUniform2i64vNV = NULL;
PFNGLPROGRAMUNIFORM2IEXTPROC glad_glProgramUniform2iEXT = NULL;
PFNGLPROGRAMUNIFORM2IVPROC glad_glProgramUniform2iv = NULL;
PFNGLPROGRAMUNIFORM2IVEXTPROC glad_glProgramUniform2ivEXT = NULL;
PFNGLPROGRAMUNIFORM2UIPROC glad_glProgramUniform2ui = NULL;
PFNGLPROGRAMUNIFORM2UI64NVPROC glad_glProgramUniform2ui64NV = NULL;
PFNGLPROGRAMUNIFORM2UI64VNVPROC glad_glProgramUniform2ui64vNV = NULL;
PFNGLPROGRAMUNIFORM2UIEXTPROC glad_glProgramUniform2uiEXT = NULL;
PFNGLPROGRAMUNIFORM2UIVPROC glad_glProgramUniform2uiv = NULL;
PFNGLPROGRAMUNIFORM2UIVEXTPROC glad_glProgramUniform2uivEXT = NULL;
PFNGLPROGRAMUNIFORM3FPROC glad_glProgramUniform3f = NULL;
PFNGLPROGRAMUNIFORM3FEXTPROC glad_glProgramUniform3fEXT = NULL;
PFNGLPROGRAMUNIFORM3FVPROC glad_glProgramUniform3fv = NULL;
PFNGLPROGRAMUNIFORM3FVEXTPROC glad_glProgramUniform3fvEXT = NULL;
PFNGLPROGRAMUNIFORM3IPROC glad_glProgramUniform3i = NULL;
PFNGLPROGRAMUNIFORM3I64NVPROC glad_glProgramUniform3i64NV = NULL;
PFNGLPROGRAMUNIFORM3I64VNVPROC glad_glProgramUniform3i64vNV = NULL;
PFNGLPROGRAMUNIFORM3IEXTPROC glad_glProgramUniform3iEXT = NULL;
PFNGLPROGRAMUNIFORM3IVPROC glad_glProgramUniform3iv = NULL;
PFNGLPROGRAMUNIFORM3IVEXTPROC glad_glProgramUniform3ivEXT = NULL;
PFNGLPROGRAMUNIFORM3UIPROC glad_glProgramUniform3ui = NULL;
PFNGLPROGRAMUNIFORM3UI64NVPROC glad_glProgramUniform3ui64NV = NULL;
PFNGLPROGRAMUNIFORM3UI64VNVPROC glad_glProgramUniform3ui64vNV = NULL;
PFNGLPROGRAMUNIFORM3UIEXTPROC glad_glProgramUniform3uiEXT = NULL;
PFNGLPROGRAMUNIFORM3UIVPROC glad_glProgramUniform3uiv = NULL;
PFNGLPROGRAMUNIFORM3UIVEXTPROC glad_glProgramUniform3uivEXT = NULL;
PFNGLPROGRAMUNIFORM4FPROC glad_glProgramUniform4f = NULL;
PFNGLPROGRAMUNIFORM4FEXTPROC glad_glProgramUniform4fEXT = NULL;
PFNGLPROGRAMUNIFORM4FVPROC glad_glProgramUniform4fv = NULL;
PFNGLPROGRAMUNIFORM4FVEXTPROC glad_glProgramUniform4fvEXT = NULL;
PFNGLPROGRAMUNIFORM4IPROC glad_glProgramUniform4i = NULL;
PFNGLPROGRAMUNIFORM4I64NVPROC glad_glProgramUniform4i64NV = NULL;
PFNGLPROGRAMUNIFORM4I64VNVPROC glad_glProgramUniform4i64vNV = NULL;
PFNGLPROGRAMUNIFORM4IEXTPROC glad_glProgramUniform4iEXT = NULL;
PFNGLPROGRAMUNIFORM4IVPROC glad_glProgramUniform4iv = NULL;
PFNGLPROGRAMUNIFORM4IVEXTPROC glad_glProgramUniform4ivEXT = NULL;
PFNGLPROGRAMUNIFORM4UIPROC glad_glProgramUniform4ui = NULL;
PFNGLPROGRAMUNIFORM4UI64NVPROC glad_glProgramUniform4ui64NV = NULL;
PFNGLPROGRAMUNIFORM4UI64VNVPROC glad_glProgramUniform4ui64vNV = NULL;
PFNGLPROGRAMUNIFORM4UIEXTPROC glad_glProgramUniform4uiEXT = NULL;
PFNGLPROGRAMUNIFORM4UIVPROC glad_glProgramUniform4uiv = NULL;
PFNGLPROGRAMUNIFORM4UIVEXTPROC glad_glProgramUniform4uivEXT = NULL;
PFNGLPROGRAMUNIFORMHANDLEUI64ARBPROC glad_glProgramUniformHandleui64ARB = NULL;
PFNGLPROGRAMUNIFORMHANDLEUI64IMGPROC glad_glProgramUniformHandleui64IMG = NULL;
PFNGLPROGRAMUNIFORMHANDLEUI64NVPROC glad_glProgramUniformHandleui64NV = NULL;
PFNGLPROGRAMUNIFORMHANDLEUI64VARBPROC glad_glProgramUniformHandleui64vARB = NULL;
PFNGLPROGRAMUNIFORMHANDLEUI64VIMGPROC glad_glProgramUniformHandleui64vIMG = NULL;
PFNGLPROGRAMUNIFORMHANDLEUI64VNVPROC glad_glProgramUniformHandleui64vNV = NULL;
PFNGLPROGRAMUNIFORMMATRIX2FVPROC glad_glProgramUniformMatrix2fv = NULL;
PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC glad_glProgramUniformMatrix2fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC glad_glProgramUniformMatrix2x3fv = NULL;
PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC glad_glProgramUniformMatrix2x3fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC glad_glProgramUniformMatrix2x4fv = NULL;
PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC glad_glProgramUniformMatrix2x4fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX3FVPROC glad_glProgramUniformMatrix3fv = NULL;
PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC glad_glProgramUniformMatrix3fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC glad_glProgramUniformMatrix3x2fv = NULL;
PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC glad_glProgramUniformMatrix3x2fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC glad_glProgramUniformMatrix3x4fv = NULL;
PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC glad_glProgramUniformMatrix3x4fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX4FVPROC glad_glProgramUniformMatrix4fv = NULL;
PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC glad_glProgramUniformMatrix4fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC glad_glProgramUniformMatrix4x2fv = NULL;
PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC glad_glProgramUniformMatrix4x2fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC glad_glProgramUniformMatrix4x3fv = NULL;
PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC glad_glProgramUniformMatrix4x3fvEXT = NULL;
PFNGLPUSHDEBUGGROUPPROC glad_glPushDebugGroup = NULL;
PFNGLPUSHDEBUGGROUPKHRPROC glad_glPushDebugGroupKHR = NULL;
PFNGLPUSHGROUPMARKEREXTPROC glad_glPushGroupMarkerEXT = NULL;
PFNGLQUERYCOUNTERPROC glad_glQueryCounter = NULL;
PFNGLQUERYCOUNTEREXTPROC glad_glQueryCounterEXT = NULL;
PFNGLRASTERSAMPLESEXTPROC glad_glRasterSamplesEXT = NULL;
PFNGLREADBUFFERPROC glad_glReadBuffer = NULL;
PFNGLREADBUFFERINDEXEDEXTPROC glad_glReadBufferIndexedEXT = NULL;
PFNGLREADBUFFERNVPROC glad_glReadBufferNV = NULL;
PFNGLREADPIXELSPROC glad_glReadPixels = NULL;
PFNGLREADNPIXELSPROC glad_glReadnPixels = NULL;
PFNGLREADNPIXELSEXTPROC glad_glReadnPixelsEXT = NULL;
PFNGLREADNPIXELSKHRPROC glad_glReadnPixelsKHR = NULL;
PFNGLRELEASEKEYEDMUTEXWIN32EXTPROC glad_glReleaseKeyedMutexWin32EXT = NULL;
PFNGLRELEASESHADERCOMPILERPROC glad_glReleaseShaderCompiler = NULL;
PFNGLRENDERBUFFERSTORAGEPROC glad_glRenderbufferStorage = NULL;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glad_glRenderbufferStorageMultisample = NULL;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC glad_glRenderbufferStorageMultisampleANGLE = NULL;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEAPPLEPROC glad_glRenderbufferStorageMultisampleAPPLE = NULL;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEADVANCEDAMDPROC glad_glRenderbufferStorageMultisampleAdvancedAMD = NULL;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glad_glRenderbufferStorageMultisampleEXT = NULL;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMGPROC glad_glRenderbufferStorageMultisampleIMG = NULL;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLENVPROC glad_glRenderbufferStorageMultisampleNV = NULL;
PFNGLRESETMEMORYOBJECTPARAMETERNVPROC glad_glResetMemoryObjectParameterNV = NULL;
PFNGLRESOLVEDEPTHVALUESNVPROC glad_glResolveDepthValuesNV = NULL;
PFNGLRESOLVEMULTISAMPLEFRAMEBUFFERAPPLEPROC glad_glResolveMultisampleFramebufferAPPLE = NULL;
PFNGLRESUMETRANSFORMFEEDBACKPROC glad_glResumeTransformFeedback = NULL;
PFNGLSAMPLECOVERAGEPROC glad_glSampleCoverage = NULL;
PFNGLSAMPLEMASKIPROC glad_glSampleMaski = NULL;
PFNGLSAMPLERPARAMETERIIVPROC glad_glSamplerParameterIiv = NULL;
PFNGLSAMPLERPARAMETERIIVEXTPROC glad_glSamplerParameterIivEXT = NULL;
PFNGLSAMPLERPARAMETERIIVOESPROC glad_glSamplerParameterIivOES = NULL;
PFNGLSAMPLERPARAMETERIUIVPROC glad_glSamplerParameterIuiv = NULL;
PFNGLSAMPLERPARAMETERIUIVEXTPROC glad_glSamplerParameterIuivEXT = NULL;
PFNGLSAMPLERPARAMETERIUIVOESPROC glad_glSamplerParameterIuivOES = NULL;
PFNGLSAMPLERPARAMETERFPROC glad_glSamplerParameterf = NULL;
PFNGLSAMPLERPARAMETERFVPROC glad_glSamplerParameterfv = NULL;
PFNGLSAMPLERPARAMETERIPROC glad_glSamplerParameteri = NULL;
PFNGLSAMPLERPARAMETERIVPROC glad_glSamplerParameteriv = NULL;
PFNGLSCISSORPROC glad_glScissor = NULL;
PFNGLSCISSORARRAYVPROC glad_glScissorArrayv = NULL;
PFNGLSCISSORARRAYVNVPROC glad_glScissorArrayvNV = NULL;
PFNGLSCISSORARRAYVOESPROC glad_glScissorArrayvOES = NULL;
PFNGLSCISSORINDEXEDPROC glad_glScissorIndexed = NULL;
PFNGLSCISSORINDEXEDNVPROC glad_glScissorIndexedNV = NULL;
PFNGLSCISSORINDEXEDOESPROC glad_glScissorIndexedOES = NULL;
PFNGLSCISSORINDEXEDVPROC glad_glScissorIndexedv = NULL;
PFNGLSCISSORINDEXEDVNVPROC glad_glScissorIndexedvNV = NULL;
PFNGLSCISSORINDEXEDVOESPROC glad_glScissorIndexedvOES = NULL;
PFNGLSELECTPERFMONITORCOUNTERSAMDPROC glad_glSelectPerfMonitorCountersAMD = NULL;
PFNGLSEMAPHOREPARAMETERUI64VEXTPROC glad_glSemaphoreParameterui64vEXT = NULL;
PFNGLSETFENCENVPROC glad_glSetFenceNV = NULL;
PFNGLSHADERBINARYPROC glad_glShaderBinary = NULL;
PFNGLSHADERSOURCEPROC glad_glShaderSource = NULL;
PFNGLSIGNALSEMAPHOREEXTPROC glad_glSignalSemaphoreEXT = NULL;
PFNGLSIGNALVKFENCENVPROC glad_glSignalVkFenceNV = NULL;
PFNGLSIGNALVKSEMAPHORENVPROC glad_glSignalVkSemaphoreNV = NULL;
PFNGLSTARTTILINGQCOMPROC glad_glStartTilingQCOM = NULL;
PFNGLSTENCILFILLPATHINSTANCEDNVPROC glad_glStencilFillPathInstancedNV = NULL;
PFNGLSTENCILFILLPATHNVPROC glad_glStencilFillPathNV = NULL;
PFNGLSTENCILFUNCPROC glad_glStencilFunc = NULL;
PFNGLSTENCILFUNCSEPARATEPROC glad_glStencilFuncSeparate = NULL;
PFNGLSTENCILMASKPROC glad_glStencilMask = NULL;
PFNGLSTENCILMASKSEPARATEPROC glad_glStencilMaskSeparate = NULL;
PFNGLSTENCILOPPROC glad_glStencilOp = NULL;
PFNGLSTENCILOPSEPARATEPROC glad_glStencilOpSeparate = NULL;
PFNGLSTENCILSTROKEPATHINSTANCEDNVPROC glad_glStencilStrokePathInstancedNV = NULL;
PFNGLSTENCILSTROKEPATHNVPROC glad_glStencilStrokePathNV = NULL;
PFNGLSTENCILTHENCOVERFILLPATHINSTANCEDNVPROC glad_glStencilThenCoverFillPathInstancedNV = NULL;
PFNGLSTENCILTHENCOVERFILLPATHNVPROC glad_glStencilThenCoverFillPathNV = NULL;
PFNGLSTENCILTHENCOVERSTROKEPATHINSTANCEDNVPROC glad_glStencilThenCoverStrokePathInstancedNV = NULL;
PFNGLSTENCILTHENCOVERSTROKEPATHNVPROC glad_glStencilThenCoverStrokePathNV = NULL;
PFNGLSUBPIXELPRECISIONBIASNVPROC glad_glSubpixelPrecisionBiasNV = NULL;
PFNGLTESTFENCENVPROC glad_glTestFenceNV = NULL;
PFNGLTEXATTACHMEMORYNVPROC glad_glTexAttachMemoryNV = NULL;
PFNGLTEXBUFFERPROC glad_glTexBuffer = NULL;
PFNGLTEXBUFFEREXTPROC glad_glTexBufferEXT = NULL;
PFNGLTEXBUFFEROESPROC glad_glTexBufferOES = NULL;
PFNGLTEXBUFFERRANGEPROC glad_glTexBufferRange = NULL;
PFNGLTEXBUFFERRANGEEXTPROC glad_glTexBufferRangeEXT = NULL;
PFNGLTEXBUFFERRANGEOESPROC glad_glTexBufferRangeOES = NULL;
PFNGLTEXIMAGE2DPROC glad_glTexImage2D = NULL;
PFNGLTEXIMAGE3DPROC glad_glTexImage3D = NULL;
PFNGLTEXIMAGE3DOESPROC glad_glTexImage3DOES = NULL;
PFNGLTEXPAGECOMMITMENTARBPROC glad_glTexPageCommitmentARB = NULL;
PFNGLTEXPAGECOMMITMENTEXTPROC glad_glTexPageCommitmentEXT = NULL;
PFNGLTEXPARAMETERIIVPROC glad_glTexParameterIiv = NULL;
PFNGLTEXPARAMETERIIVEXTPROC glad_glTexParameterIivEXT = NULL;
PFNGLTEXPARAMETERIIVOESPROC glad_glTexParameterIivOES = NULL;
PFNGLTEXPARAMETERIUIVPROC glad_glTexParameterIuiv = NULL;
PFNGLTEXPARAMETERIUIVEXTPROC glad_glTexParameterIuivEXT = NULL;
PFNGLTEXPARAMETERIUIVOESPROC glad_glTexParameterIuivOES = NULL;
PFNGLTEXPARAMETERFPROC glad_glTexParameterf = NULL;
PFNGLTEXPARAMETERFVPROC glad_glTexParameterfv = NULL;
PFNGLTEXPARAMETERIPROC glad_glTexParameteri = NULL;
PFNGLTEXPARAMETERIVPROC glad_glTexParameteriv = NULL;
PFNGLTEXSTORAGE1DPROC glad_glTexStorage1D = NULL;
PFNGLTEXSTORAGE1DEXTPROC glad_glTexStorage1DEXT = NULL;
PFNGLTEXSTORAGE2DPROC glad_glTexStorage2D = NULL;
PFNGLTEXSTORAGE2DEXTPROC glad_glTexStorage2DEXT = NULL;
PFNGLTEXSTORAGE2DMULTISAMPLEPROC glad_glTexStorage2DMultisample = NULL;
PFNGLTEXSTORAGE3DPROC glad_glTexStorage3D = NULL;
PFNGLTEXSTORAGE3DEXTPROC glad_glTexStorage3DEXT = NULL;
PFNGLTEXSTORAGE3DMULTISAMPLEPROC glad_glTexStorage3DMultisample = NULL;
PFNGLTEXSTORAGE3DMULTISAMPLEOESPROC glad_glTexStorage3DMultisampleOES = NULL;
PFNGLTEXSTORAGEMEM2DEXTPROC glad_glTexStorageMem2DEXT = NULL;
PFNGLTEXSTORAGEMEM2DMULTISAMPLEEXTPROC glad_glTexStorageMem2DMultisampleEXT = NULL;
PFNGLTEXSTORAGEMEM3DEXTPROC glad_glTexStorageMem3DEXT = NULL;
PFNGLTEXSTORAGEMEM3DMULTISAMPLEEXTPROC glad_glTexStorageMem3DMultisampleEXT = NULL;
PFNGLTEXSUBIMAGE2DPROC glad_glTexSubImage2D = NULL;
PFNGLTEXSUBIMAGE3DPROC glad_glTexSubImage3D = NULL;
PFNGLTEXSUBIMAGE3DOESPROC glad_glTexSubImage3DOES = NULL;
PFNGLTEXTUREATTACHMEMORYNVPROC glad_glTextureAttachMemoryNV = NULL;
PFNGLTEXTUREFOVEATIONPARAMETERSQCOMPROC glad_glTextureFoveationParametersQCOM = NULL;
PFNGLTEXTURESTORAGE1DEXTPROC glad_glTextureStorage1DEXT = NULL;
PFNGLTEXTURESTORAGE2DEXTPROC glad_glTextureStorage2DEXT = NULL;
PFNGLTEXTURESTORAGE3DEXTPROC glad_glTextureStorage3DEXT = NULL;
PFNGLTEXTURESTORAGEMEM2DEXTPROC glad_glTextureStorageMem2DEXT = NULL;
PFNGLTEXTURESTORAGEMEM2DMULTISAMPLEEXTPROC glad_glTextureStorageMem2DMultisampleEXT = NULL;
PFNGLTEXTURESTORAGEMEM3DEXTPROC glad_glTextureStorageMem3DEXT = NULL;
PFNGLTEXTURESTORAGEMEM3DMULTISAMPLEEXTPROC glad_glTextureStorageMem3DMultisampleEXT = NULL;
PFNGLTEXTUREVIEWPROC glad_glTextureView = NULL;
PFNGLTEXTUREVIEWEXTPROC glad_glTextureViewEXT = NULL;
PFNGLTEXTUREVIEWOESPROC glad_glTextureViewOES = NULL;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC glad_glTransformFeedbackVaryings = NULL;
PFNGLTRANSFORMPATHNVPROC glad_glTransformPathNV = NULL;
PFNGLUNIFORM1FPROC glad_glUniform1f = NULL;
PFNGLUNIFORM1FVPROC glad_glUniform1fv = NULL;
PFNGLUNIFORM1IPROC glad_glUniform1i = NULL;
PFNGLUNIFORM1I64NVPROC glad_glUniform1i64NV = NULL;
PFNGLUNIFORM1I64VNVPROC glad_glUniform1i64vNV = NULL;
PFNGLUNIFORM1IVPROC glad_glUniform1iv = NULL;
PFNGLUNIFORM1UIPROC glad_glUniform1ui = NULL;
PFNGLUNIFORM1UI64NVPROC glad_glUniform1ui64NV = NULL;
PFNGLUNIFORM1UI64VNVPROC glad_glUniform1ui64vNV = NULL;
PFNGLUNIFORM1UIVPROC glad_glUniform1uiv = NULL;
PFNGLUNIFORM2FPROC glad_glUniform2f = NULL;
PFNGLUNIFORM2FVPROC glad_glUniform2fv = NULL;
PFNGLUNIFORM2IPROC glad_glUniform2i = NULL;
PFNGLUNIFORM2I64NVPROC glad_glUniform2i64NV = NULL;
PFNGLUNIFORM2I64VNVPROC glad_glUniform2i64vNV = NULL;
PFNGLUNIFORM2IVPROC glad_glUniform2iv = NULL;
PFNGLUNIFORM2UIPROC glad_glUniform2ui = NULL;
PFNGLUNIFORM2UI64NVPROC glad_glUniform2ui64NV = NULL;
PFNGLUNIFORM2UI64VNVPROC glad_glUniform2ui64vNV = NULL;
PFNGLUNIFORM2UIVPROC glad_glUniform2uiv = NULL;
PFNGLUNIFORM3FPROC glad_glUniform3f = NULL;
PFNGLUNIFORM3FVPROC glad_glUniform3fv = NULL;
PFNGLUNIFORM3IPROC glad_glUniform3i = NULL;
PFNGLUNIFORM3I64NVPROC glad_glUniform3i64NV = NULL;
PFNGLUNIFORM3I64VNVPROC glad_glUniform3i64vNV = NULL;
PFNGLUNIFORM3IVPROC glad_glUniform3iv = NULL;
PFNGLUNIFORM3UIPROC glad_glUniform3ui = NULL;
PFNGLUNIFORM3UI64NVPROC glad_glUniform3ui64NV = NULL;
PFNGLUNIFORM3UI64VNVPROC glad_glUniform3ui64vNV = NULL;
PFNGLUNIFORM3UIVPROC glad_glUniform3uiv = NULL;
PFNGLUNIFORM4FPROC glad_glUniform4f = NULL;
PFNGLUNIFORM4FVPROC glad_glUniform4fv = NULL;
PFNGLUNIFORM4IPROC glad_glUniform4i = NULL;
PFNGLUNIFORM4I64NVPROC glad_glUniform4i64NV = NULL;
PFNGLUNIFORM4I64VNVPROC glad_glUniform4i64vNV = NULL;
PFNGLUNIFORM4IVPROC glad_glUniform4iv = NULL;
PFNGLUNIFORM4UIPROC glad_glUniform4ui = NULL;
PFNGLUNIFORM4UI64NVPROC glad_glUniform4ui64NV = NULL;
PFNGLUNIFORM4UI64VNVPROC glad_glUniform4ui64vNV = NULL;
PFNGLUNIFORM4UIVPROC glad_glUniform4uiv = NULL;
PFNGLUNIFORMBLOCKBINDINGPROC glad_glUniformBlockBinding = NULL;
PFNGLUNIFORMHANDLEUI64ARBPROC glad_glUniformHandleui64ARB = NULL;
PFNGLUNIFORMHANDLEUI64IMGPROC glad_glUniformHandleui64IMG = NULL;
PFNGLUNIFORMHANDLEUI64NVPROC glad_glUniformHandleui64NV = NULL;
PFNGLUNIFORMHANDLEUI64VARBPROC glad_glUniformHandleui64vARB = NULL;
PFNGLUNIFORMHANDLEUI64VIMGPROC glad_glUniformHandleui64vIMG = NULL;
PFNGLUNIFORMHANDLEUI64VNVPROC glad_glUniformHandleui64vNV = NULL;
PFNGLUNIFORMMATRIX2FVPROC glad_glUniformMatrix2fv = NULL;
PFNGLUNIFORMMATRIX2X3FVPROC glad_glUniformMatrix2x3fv = NULL;
PFNGLUNIFORMMATRIX2X3FVNVPROC glad_glUniformMatrix2x3fvNV = NULL;
PFNGLUNIFORMMATRIX2X4FVPROC glad_glUniformMatrix2x4fv = NULL;
PFNGLUNIFORMMATRIX2X4FVNVPROC glad_glUniformMatrix2x4fvNV = NULL;
PFNGLUNIFORMMATRIX3FVPROC glad_glUniformMatrix3fv = NULL;
PFNGLUNIFORMMATRIX3X2FVPROC glad_glUniformMatrix3x2fv = NULL;
PFNGLUNIFORMMATRIX3X2FVNVPROC glad_glUniformMatrix3x2fvNV = NULL;
PFNGLUNIFORMMATRIX3X4FVPROC glad_glUniformMatrix3x4fv = NULL;
PFNGLUNIFORMMATRIX3X4FVNVPROC glad_glUniformMatrix3x4fvNV = NULL;
PFNGLUNIFORMMATRIX4FVPROC glad_glUniformMatrix4fv = NULL;
PFNGLUNIFORMMATRIX4X2FVPROC glad_glUniformMatrix4x2fv = NULL;
PFNGLUNIFORMMATRIX4X2FVNVPROC glad_glUniformMatrix4x2fvNV = NULL;
PFNGLUNIFORMMATRIX4X3FVPROC glad_glUniformMatrix4x3fv = NULL;
PFNGLUNIFORMMATRIX4X3FVNVPROC glad_glUniformMatrix4x3fvNV = NULL;
PFNGLUNMAPBUFFERPROC glad_glUnmapBuffer = NULL;
PFNGLUNMAPBUFFEROESPROC glad_glUnmapBufferOES = NULL;
PFNGLUSEPROGRAMPROC glad_glUseProgram = NULL;
PFNGLUSEPROGRAMSTAGESPROC glad_glUseProgramStages = NULL;
PFNGLUSEPROGRAMSTAGESEXTPROC glad_glUseProgramStagesEXT = NULL;
PFNGLVALIDATEPROGRAMPROC glad_glValidateProgram = NULL;
PFNGLVALIDATEPROGRAMPIPELINEPROC glad_glValidateProgramPipeline = NULL;
PFNGLVALIDATEPROGRAMPIPELINEEXTPROC glad_glValidateProgramPipelineEXT = NULL;
PFNGLVERTEXATTRIB1FPROC glad_glVertexAttrib1f = NULL;
PFNGLVERTEXATTRIB1FVPROC glad_glVertexAttrib1fv = NULL;
PFNGLVERTEXATTRIB2FPROC glad_glVertexAttrib2f = NULL;
PFNGLVERTEXATTRIB2FVPROC glad_glVertexAttrib2fv = NULL;
PFNGLVERTEXATTRIB3FPROC glad_glVertexAttrib3f = NULL;
PFNGLVERTEXATTRIB3FVPROC glad_glVertexAttrib3fv = NULL;
PFNGLVERTEXATTRIB4FPROC glad_glVertexAttrib4f = NULL;
PFNGLVERTEXATTRIB4FVPROC glad_glVertexAttrib4fv = NULL;
PFNGLVERTEXATTRIBBINDINGPROC glad_glVertexAttribBinding = NULL;
PFNGLVERTEXATTRIBDIVISORPROC glad_glVertexAttribDivisor = NULL;
PFNGLVERTEXATTRIBDIVISORANGLEPROC glad_glVertexAttribDivisorANGLE = NULL;
PFNGLVERTEXATTRIBDIVISOREXTPROC glad_glVertexAttribDivisorEXT = NULL;
PFNGLVERTEXATTRIBDIVISORNVPROC glad_glVertexAttribDivisorNV = NULL;
PFNGLVERTEXATTRIBFORMATPROC glad_glVertexAttribFormat = NULL;
PFNGLVERTEXATTRIBI4IPROC glad_glVertexAttribI4i = NULL;
PFNGLVERTEXATTRIBI4IVPROC glad_glVertexAttribI4iv = NULL;
PFNGLVERTEXATTRIBI4UIPROC glad_glVertexAttribI4ui = NULL;
PFNGLVERTEXATTRIBI4UIVPROC glad_glVertexAttribI4uiv = NULL;
PFNGLVERTEXATTRIBIFORMATPROC glad_glVertexAttribIFormat = NULL;
PFNGLVERTEXATTRIBIPOINTERPROC glad_glVertexAttribIPointer = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glad_glVertexAttribPointer = NULL;
PFNGLVERTEXBINDINGDIVISORPROC glad_glVertexBindingDivisor = NULL;
PFNGLVIEWPORTPROC glad_glViewport = NULL;
PFNGLVIEWPORTARRAYVPROC glad_glViewportArrayv = NULL;
PFNGLVIEWPORTARRAYVNVPROC glad_glViewportArrayvNV = NULL;
PFNGLVIEWPORTARRAYVOESPROC glad_glViewportArrayvOES = NULL;
PFNGLVIEWPORTINDEXEDFPROC glad_glViewportIndexedf = NULL;
PFNGLVIEWPORTINDEXEDFNVPROC glad_glViewportIndexedfNV = NULL;
PFNGLVIEWPORTINDEXEDFOESPROC glad_glViewportIndexedfOES = NULL;
PFNGLVIEWPORTINDEXEDFVPROC glad_glViewportIndexedfv = NULL;
PFNGLVIEWPORTINDEXEDFVNVPROC glad_glViewportIndexedfvNV = NULL;
PFNGLVIEWPORTINDEXEDFVOESPROC glad_glViewportIndexedfvOES = NULL;
PFNGLVIEWPORTPOSITIONWSCALENVPROC glad_glViewportPositionWScaleNV = NULL;
PFNGLVIEWPORTSWIZZLENVPROC glad_glViewportSwizzleNV = NULL;
PFNGLWAITSEMAPHOREEXTPROC glad_glWaitSemaphoreEXT = NULL;
PFNGLWAITSYNCPROC glad_glWaitSync = NULL;
PFNGLWAITSYNCAPPLEPROC glad_glWaitSyncAPPLE = NULL;
PFNGLWAITVKSEMAPHORENVPROC glad_glWaitVkSemaphoreNV = NULL;
PFNGLWEIGHTPATHSNVPROC glad_glWeightPathsNV = NULL;
PFNGLWINDOWRECTANGLESEXTPROC glad_glWindowRectanglesEXT = NULL;


static void glad_gl_load_GL_ES_VERSION_2_0( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_ES_VERSION_2_0) return;
    glActiveTexture = (PFNGLACTIVETEXTUREPROC) load("glActiveTexture", userptr);
    glAttachShader = (PFNGLATTACHSHADERPROC) load("glAttachShader", userptr);
    glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) load("glBindAttribLocation", userptr);
    glBindBuffer = (PFNGLBINDBUFFERPROC) load("glBindBuffer", userptr);
    glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) load("glBindFramebuffer", userptr);
    glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) load("glBindRenderbuffer", userptr);
    glBindTexture = (PFNGLBINDTEXTUREPROC) load("glBindTexture", userptr);
    glBlendColor = (PFNGLBLENDCOLORPROC) load("glBlendColor", userptr);
    glBlendEquation = (PFNGLBLENDEQUATIONPROC) load("glBlendEquation", userptr);
    glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC) load("glBlendEquationSeparate", userptr);
    glBlendFunc = (PFNGLBLENDFUNCPROC) load("glBlendFunc", userptr);
    glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC) load("glBlendFuncSeparate", userptr);
    glBufferData = (PFNGLBUFFERDATAPROC) load("glBufferData", userptr);
    glBufferSubData = (PFNGLBUFFERSUBDATAPROC) load("glBufferSubData", userptr);
    glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) load("glCheckFramebufferStatus", userptr);
    glClear = (PFNGLCLEARPROC) load("glClear", userptr);
    glClearColor = (PFNGLCLEARCOLORPROC) load("glClearColor", userptr);
    glClearDepthf = (PFNGLCLEARDEPTHFPROC) load("glClearDepthf", userptr);
    glClearStencil = (PFNGLCLEARSTENCILPROC) load("glClearStencil", userptr);
    glColorMask = (PFNGLCOLORMASKPROC) load("glColorMask", userptr);
    glCompileShader = (PFNGLCOMPILESHADERPROC) load("glCompileShader", userptr);
    glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) load("glCompressedTexImage2D", userptr);
    glCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) load("glCompressedTexSubImage2D", userptr);
    glCopyTexImage2D = (PFNGLCOPYTEXIMAGE2DPROC) load("glCopyTexImage2D", userptr);
    glCopyTexSubImage2D = (PFNGLCOPYTEXSUBIMAGE2DPROC) load("glCopyTexSubImage2D", userptr);
    glCreateProgram = (PFNGLCREATEPROGRAMPROC) load("glCreateProgram", userptr);
    glCreateShader = (PFNGLCREATESHADERPROC) load("glCreateShader", userptr);
    glCullFace = (PFNGLCULLFACEPROC) load("glCullFace", userptr);
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) load("glDeleteBuffers", userptr);
    glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) load("glDeleteFramebuffers", userptr);
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC) load("glDeleteProgram", userptr);
    glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) load("glDeleteRenderbuffers", userptr);
    glDeleteShader = (PFNGLDELETESHADERPROC) load("glDeleteShader", userptr);
    glDeleteTextures = (PFNGLDELETETEXTURESPROC) load("glDeleteTextures", userptr);
    glDepthFunc = (PFNGLDEPTHFUNCPROC) load("glDepthFunc", userptr);
    glDepthMask = (PFNGLDEPTHMASKPROC) load("glDepthMask", userptr);
    glDepthRangef = (PFNGLDEPTHRANGEFPROC) load("glDepthRangef", userptr);
    glDetachShader = (PFNGLDETACHSHADERPROC) load("glDetachShader", userptr);
    glDisable = (PFNGLDISABLEPROC) load("glDisable", userptr);
    glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) load("glDisableVertexAttribArray", userptr);
    glDrawArrays = (PFNGLDRAWARRAYSPROC) load("glDrawArrays", userptr);
    glDrawElements = (PFNGLDRAWELEMENTSPROC) load("glDrawElements", userptr);
    glEnable = (PFNGLENABLEPROC) load("glEnable", userptr);
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) load("glEnableVertexAttribArray", userptr);
    glFinish = (PFNGLFINISHPROC) load("glFinish", userptr);
    glFlush = (PFNGLFLUSHPROC) load("glFlush", userptr);
    glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) load("glFramebufferRenderbuffer", userptr);
    glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) load("glFramebufferTexture2D", userptr);
    glFrontFace = (PFNGLFRONTFACEPROC) load("glFrontFace", userptr);
    glGenBuffers = (PFNGLGENBUFFERSPROC) load("glGenBuffers", userptr);
    glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) load("glGenFramebuffers", userptr);
    glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) load("glGenRenderbuffers", userptr);
    glGenTextures = (PFNGLGENTEXTURESPROC) load("glGenTextures", userptr);
    glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) load("glGenerateMipmap", userptr);
    glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC) load("glGetActiveAttrib", userptr);
    glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC) load("glGetActiveUniform", userptr);
    glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC) load("glGetAttachedShaders", userptr);
    glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) load("glGetAttribLocation", userptr);
    glGetBooleanv = (PFNGLGETBOOLEANVPROC) load("glGetBooleanv", userptr);
    glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC) load("glGetBufferParameteriv", userptr);
    glGetError = (PFNGLGETERRORPROC) load("glGetError", userptr);
    glGetFloatv = (PFNGLGETFLOATVPROC) load("glGetFloatv", userptr);
    glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) load("glGetFramebufferAttachmentParameteriv", userptr);
    glGetIntegerv = (PFNGLGETINTEGERVPROC) load("glGetIntegerv", userptr);
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) load("glGetProgramInfoLog", userptr);
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC) load("glGetProgramiv", userptr);
    glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC) load("glGetRenderbufferParameteriv", userptr);
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) load("glGetShaderInfoLog", userptr);
    glGetShaderPrecisionFormat = (PFNGLGETSHADERPRECISIONFORMATPROC) load("glGetShaderPrecisionFormat", userptr);
    glGetShaderSource = (PFNGLGETSHADERSOURCEPROC) load("glGetShaderSource", userptr);
    glGetShaderiv = (PFNGLGETSHADERIVPROC) load("glGetShaderiv", userptr);
    glGetString = (PFNGLGETSTRINGPROC) load("glGetString", userptr);
    glGetTexParameterfv = (PFNGLGETTEXPARAMETERFVPROC) load("glGetTexParameterfv", userptr);
    glGetTexParameteriv = (PFNGLGETTEXPARAMETERIVPROC) load("glGetTexParameteriv", userptr);
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) load("glGetUniformLocation", userptr);
    glGetUniformfv = (PFNGLGETUNIFORMFVPROC) load("glGetUniformfv", userptr);
    glGetUniformiv = (PFNGLGETUNIFORMIVPROC) load("glGetUniformiv", userptr);
    glGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC) load("glGetVertexAttribPointerv", userptr);
    glGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC) load("glGetVertexAttribfv", userptr);
    glGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC) load("glGetVertexAttribiv", userptr);
    glHint = (PFNGLHINTPROC) load("glHint", userptr);
    glIsBuffer = (PFNGLISBUFFERPROC) load("glIsBuffer", userptr);
    glIsEnabled = (PFNGLISENABLEDPROC) load("glIsEnabled", userptr);
    glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC) load("glIsFramebuffer", userptr);
    glIsProgram = (PFNGLISPROGRAMPROC) load("glIsProgram", userptr);
    glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC) load("glIsRenderbuffer", userptr);
    glIsShader = (PFNGLISSHADERPROC) load("glIsShader", userptr);
    glIsTexture = (PFNGLISTEXTUREPROC) load("glIsTexture", userptr);
    glLineWidth = (PFNGLLINEWIDTHPROC) load("glLineWidth", userptr);
    glLinkProgram = (PFNGLLINKPROGRAMPROC) load("glLinkProgram", userptr);
    glPixelStorei = (PFNGLPIXELSTOREIPROC) load("glPixelStorei", userptr);
    glPolygonOffset = (PFNGLPOLYGONOFFSETPROC) load("glPolygonOffset", userptr);
    glReadPixels = (PFNGLREADPIXELSPROC) load("glReadPixels", userptr);
    glReleaseShaderCompiler = (PFNGLRELEASESHADERCOMPILERPROC) load("glReleaseShaderCompiler", userptr);
    glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) load("glRenderbufferStorage", userptr);
    glSampleCoverage = (PFNGLSAMPLECOVERAGEPROC) load("glSampleCoverage", userptr);
    glScissor = (PFNGLSCISSORPROC) load("glScissor", userptr);
    glShaderBinary = (PFNGLSHADERBINARYPROC) load("glShaderBinary", userptr);
    glShaderSource = (PFNGLSHADERSOURCEPROC) load("glShaderSource", userptr);
    glStencilFunc = (PFNGLSTENCILFUNCPROC) load("glStencilFunc", userptr);
    glStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC) load("glStencilFuncSeparate", userptr);
    glStencilMask = (PFNGLSTENCILMASKPROC) load("glStencilMask", userptr);
    glStencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC) load("glStencilMaskSeparate", userptr);
    glStencilOp = (PFNGLSTENCILOPPROC) load("glStencilOp", userptr);
    glStencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC) load("glStencilOpSeparate", userptr);
    glTexImage2D = (PFNGLTEXIMAGE2DPROC) load("glTexImage2D", userptr);
    glTexParameterf = (PFNGLTEXPARAMETERFPROC) load("glTexParameterf", userptr);
    glTexParameterfv = (PFNGLTEXPARAMETERFVPROC) load("glTexParameterfv", userptr);
    glTexParameteri = (PFNGLTEXPARAMETERIPROC) load("glTexParameteri", userptr);
    glTexParameteriv = (PFNGLTEXPARAMETERIVPROC) load("glTexParameteriv", userptr);
    glTexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC) load("glTexSubImage2D", userptr);
    glUniform1f = (PFNGLUNIFORM1FPROC) load("glUniform1f", userptr);
    glUniform1fv = (PFNGLUNIFORM1FVPROC) load("glUniform1fv", userptr);
    glUniform1i = (PFNGLUNIFORM1IPROC) load("glUniform1i", userptr);
    glUniform1iv = (PFNGLUNIFORM1IVPROC) load("glUniform1iv", userptr);
    glUniform2f = (PFNGLUNIFORM2FPROC) load("glUniform2f", userptr);
    glUniform2fv = (PFNGLUNIFORM2FVPROC) load("glUniform2fv", userptr);
    glUniform2i = (PFNGLUNIFORM2IPROC) load("glUniform2i", userptr);
    glUniform2iv = (PFNGLUNIFORM2IVPROC) load("glUniform2iv", userptr);
    glUniform3f = (PFNGLUNIFORM3FPROC) load("glUniform3f", userptr);
    glUniform3fv = (PFNGLUNIFORM3FVPROC) load("glUniform3fv", userptr);
    glUniform3i = (PFNGLUNIFORM3IPROC) load("glUniform3i", userptr);
    glUniform3iv = (PFNGLUNIFORM3IVPROC) load("glUniform3iv", userptr);
    glUniform4f = (PFNGLUNIFORM4FPROC) load("glUniform4f", userptr);
    glUniform4fv = (PFNGLUNIFORM4FVPROC) load("glUniform4fv", userptr);
    glUniform4i = (PFNGLUNIFORM4IPROC) load("glUniform4i", userptr);
    glUniform4iv = (PFNGLUNIFORM4IVPROC) load("glUniform4iv", userptr);
    glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC) load("glUniformMatrix2fv", userptr);
    glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC) load("glUniformMatrix3fv", userptr);
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) load("glUniformMatrix4fv", userptr);
    glUseProgram = (PFNGLUSEPROGRAMPROC) load("glUseProgram", userptr);
    glValidateProgram = (PFNGLVALIDATEPROGRAMPROC) load("glValidateProgram", userptr);
    glVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC) load("glVertexAttrib1f", userptr);
    glVertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC) load("glVertexAttrib1fv", userptr);
    glVertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC) load("glVertexAttrib2f", userptr);
    glVertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC) load("glVertexAttrib2fv", userptr);
    glVertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC) load("glVertexAttrib3f", userptr);
    glVertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC) load("glVertexAttrib3fv", userptr);
    glVertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC) load("glVertexAttrib4f", userptr);
    glVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC) load("glVertexAttrib4fv", userptr);
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) load("glVertexAttribPointer", userptr);
    glViewport = (PFNGLVIEWPORTPROC) load("glViewport", userptr);
}
static void glad_gl_load_GL_ES_VERSION_3_0( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_ES_VERSION_3_0) return;
    glBeginQuery = (PFNGLBEGINQUERYPROC) load("glBeginQuery", userptr);
    glBeginTransformFeedback = (PFNGLBEGINTRANSFORMFEEDBACKPROC) load("glBeginTransformFeedback", userptr);
    glBindBufferBase = (PFNGLBINDBUFFERBASEPROC) load("glBindBufferBase", userptr);
    glBindBufferRange = (PFNGLBINDBUFFERRANGEPROC) load("glBindBufferRange", userptr);
    glBindSampler = (PFNGLBINDSAMPLERPROC) load("glBindSampler", userptr);
    glBindTransformFeedback = (PFNGLBINDTRANSFORMFEEDBACKPROC) load("glBindTransformFeedback", userptr);
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) load("glBindVertexArray", userptr);
    glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC) load("glBlitFramebuffer", userptr);
    glClearBufferfi = (PFNGLCLEARBUFFERFIPROC) load("glClearBufferfi", userptr);
    glClearBufferfv = (PFNGLCLEARBUFFERFVPROC) load("glClearBufferfv", userptr);
    glClearBufferiv = (PFNGLCLEARBUFFERIVPROC) load("glClearBufferiv", userptr);
    glClearBufferuiv = (PFNGLCLEARBUFFERUIVPROC) load("glClearBufferuiv", userptr);
    glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC) load("glClientWaitSync", userptr);
    glCompressedTexImage3D = (PFNGLCOMPRESSEDTEXIMAGE3DPROC) load("glCompressedTexImage3D", userptr);
    glCompressedTexSubImage3D = (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC) load("glCompressedTexSubImage3D", userptr);
    glCopyBufferSubData = (PFNGLCOPYBUFFERSUBDATAPROC) load("glCopyBufferSubData", userptr);
    glCopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3DPROC) load("glCopyTexSubImage3D", userptr);
    glDeleteQueries = (PFNGLDELETEQUERIESPROC) load("glDeleteQueries", userptr);
    glDeleteSamplers = (PFNGLDELETESAMPLERSPROC) load("glDeleteSamplers", userptr);
    glDeleteSync = (PFNGLDELETESYNCPROC) load("glDeleteSync", userptr);
    glDeleteTransformFeedbacks = (PFNGLDELETETRANSFORMFEEDBACKSPROC) load("glDeleteTransformFeedbacks", userptr);
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) load("glDeleteVertexArrays", userptr);
    glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC) load("glDrawArraysInstanced", userptr);
    glDrawBuffers = (PFNGLDRAWBUFFERSPROC) load("glDrawBuffers", userptr);
    glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC) load("glDrawElementsInstanced", userptr);
    glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC) load("glDrawRangeElements", userptr);
    glEndQuery = (PFNGLENDQUERYPROC) load("glEndQuery", userptr);
    glEndTransformFeedback = (PFNGLENDTRANSFORMFEEDBACKPROC) load("glEndTransformFeedback", userptr);
    glFenceSync = (PFNGLFENCESYNCPROC) load("glFenceSync", userptr);
    glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC) load("glFlushMappedBufferRange", userptr);
    glFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC) load("glFramebufferTextureLayer", userptr);
    glGenQueries = (PFNGLGENQUERIESPROC) load("glGenQueries", userptr);
    glGenSamplers = (PFNGLGENSAMPLERSPROC) load("glGenSamplers", userptr);
    glGenTransformFeedbacks = (PFNGLGENTRANSFORMFEEDBACKSPROC) load("glGenTransformFeedbacks", userptr);
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) load("glGenVertexArrays", userptr);
    glGetActiveUniformBlockName = (PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC) load("glGetActiveUniformBlockName", userptr);
    glGetActiveUniformBlockiv = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC) load("glGetActiveUniformBlockiv", userptr);
    glGetActiveUniformsiv = (PFNGLGETACTIVEUNIFORMSIVPROC) load("glGetActiveUniformsiv", userptr);
    glGetBufferParameteri64v = (PFNGLGETBUFFERPARAMETERI64VPROC) load("glGetBufferParameteri64v", userptr);
    glGetBufferPointerv = (PFNGLGETBUFFERPOINTERVPROC) load("glGetBufferPointerv", userptr);
    glGetFragDataLocation = (PFNGLGETFRAGDATALOCATIONPROC) load("glGetFragDataLocation", userptr);
    glGetInteger64i_v = (PFNGLGETINTEGER64I_VPROC) load("glGetInteger64i_v", userptr);
    glGetInteger64v = (PFNGLGETINTEGER64VPROC) load("glGetInteger64v", userptr);
    glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC) load("glGetIntegeri_v", userptr);
    glGetInternalformativ = (PFNGLGETINTERNALFORMATIVPROC) load("glGetInternalformativ", userptr);
    glGetProgramBinary = (PFNGLGETPROGRAMBINARYPROC) load("glGetProgramBinary", userptr);
    glGetQueryObjectuiv = (PFNGLGETQUERYOBJECTUIVPROC) load("glGetQueryObjectuiv", userptr);
    glGetQueryiv = (PFNGLGETQUERYIVPROC) load("glGetQueryiv", userptr);
    glGetSamplerParameterfv = (PFNGLGETSAMPLERPARAMETERFVPROC) load("glGetSamplerParameterfv", userptr);
    glGetSamplerParameteriv = (PFNGLGETSAMPLERPARAMETERIVPROC) load("glGetSamplerParameteriv", userptr);
    glGetStringi = (PFNGLGETSTRINGIPROC) load("glGetStringi", userptr);
    glGetSynciv = (PFNGLGETSYNCIVPROC) load("glGetSynciv", userptr);
    glGetTransformFeedbackVarying = (PFNGLGETTRANSFORMFEEDBACKVARYINGPROC) load("glGetTransformFeedbackVarying", userptr);
    glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC) load("glGetUniformBlockIndex", userptr);
    glGetUniformIndices = (PFNGLGETUNIFORMINDICESPROC) load("glGetUniformIndices", userptr);
    glGetUniformuiv = (PFNGLGETUNIFORMUIVPROC) load("glGetUniformuiv", userptr);
    glGetVertexAttribIiv = (PFNGLGETVERTEXATTRIBIIVPROC) load("glGetVertexAttribIiv", userptr);
    glGetVertexAttribIuiv = (PFNGLGETVERTEXATTRIBIUIVPROC) load("glGetVertexAttribIuiv", userptr);
    glInvalidateFramebuffer = (PFNGLINVALIDATEFRAMEBUFFERPROC) load("glInvalidateFramebuffer", userptr);
    glInvalidateSubFramebuffer = (PFNGLINVALIDATESUBFRAMEBUFFERPROC) load("glInvalidateSubFramebuffer", userptr);
    glIsQuery = (PFNGLISQUERYPROC) load("glIsQuery", userptr);
    glIsSampler = (PFNGLISSAMPLERPROC) load("glIsSampler", userptr);
    glIsSync = (PFNGLISSYNCPROC) load("glIsSync", userptr);
    glIsTransformFeedback = (PFNGLISTRANSFORMFEEDBACKPROC) load("glIsTransformFeedback", userptr);
    glIsVertexArray = (PFNGLISVERTEXARRAYPROC) load("glIsVertexArray", userptr);
    glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC) load("glMapBufferRange", userptr);
    glPauseTransformFeedback = (PFNGLPAUSETRANSFORMFEEDBACKPROC) load("glPauseTransformFeedback", userptr);
    glProgramBinary = (PFNGLPROGRAMBINARYPROC) load("glProgramBinary", userptr);
    glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC) load("glProgramParameteri", userptr);
    glReadBuffer = (PFNGLREADBUFFERPROC) load("glReadBuffer", userptr);
    glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC) load("glRenderbufferStorageMultisample", userptr);
    glResumeTransformFeedback = (PFNGLRESUMETRANSFORMFEEDBACKPROC) load("glResumeTransformFeedback", userptr);
    glSamplerParameterf = (PFNGLSAMPLERPARAMETERFPROC) load("glSamplerParameterf", userptr);
    glSamplerParameterfv = (PFNGLSAMPLERPARAMETERFVPROC) load("glSamplerParameterfv", userptr);
    glSamplerParameteri = (PFNGLSAMPLERPARAMETERIPROC) load("glSamplerParameteri", userptr);
    glSamplerParameteriv = (PFNGLSAMPLERPARAMETERIVPROC) load("glSamplerParameteriv", userptr);
    glTexImage3D = (PFNGLTEXIMAGE3DPROC) load("glTexImage3D", userptr);
    glTexStorage2D = (PFNGLTEXSTORAGE2DPROC) load("glTexStorage2D", userptr);
    glTexStorage3D = (PFNGLTEXSTORAGE3DPROC) load("glTexStorage3D", userptr);
    glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC) load("glTexSubImage3D", userptr);
    glTransformFeedbackVaryings = (PFNGLTRANSFORMFEEDBACKVARYINGSPROC) load("glTransformFeedbackVaryings", userptr);
    glUniform1ui = (PFNGLUNIFORM1UIPROC) load("glUniform1ui", userptr);
    glUniform1uiv = (PFNGLUNIFORM1UIVPROC) load("glUniform1uiv", userptr);
    glUniform2ui = (PFNGLUNIFORM2UIPROC) load("glUniform2ui", userptr);
    glUniform2uiv = (PFNGLUNIFORM2UIVPROC) load("glUniform2uiv", userptr);
    glUniform3ui = (PFNGLUNIFORM3UIPROC) load("glUniform3ui", userptr);
    glUniform3uiv = (PFNGLUNIFORM3UIVPROC) load("glUniform3uiv", userptr);
    glUniform4ui = (PFNGLUNIFORM4UIPROC) load("glUniform4ui", userptr);
    glUniform4uiv = (PFNGLUNIFORM4UIVPROC) load("glUniform4uiv", userptr);
    glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC) load("glUniformBlockBinding", userptr);
    glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC) load("glUniformMatrix2x3fv", userptr);
    glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC) load("glUniformMatrix2x4fv", userptr);
    glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC) load("glUniformMatrix3x2fv", userptr);
    glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC) load("glUniformMatrix3x4fv", userptr);
    glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC) load("glUniformMatrix4x2fv", userptr);
    glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC) load("glUniformMatrix4x3fv", userptr);
    glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) load("glUnmapBuffer", userptr);
    glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC) load("glVertexAttribDivisor", userptr);
    glVertexAttribI4i = (PFNGLVERTEXATTRIBI4IPROC) load("glVertexAttribI4i", userptr);
    glVertexAttribI4iv = (PFNGLVERTEXATTRIBI4IVPROC) load("glVertexAttribI4iv", userptr);
    glVertexAttribI4ui = (PFNGLVERTEXATTRIBI4UIPROC) load("glVertexAttribI4ui", userptr);
    glVertexAttribI4uiv = (PFNGLVERTEXATTRIBI4UIVPROC) load("glVertexAttribI4uiv", userptr);
    glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC) load("glVertexAttribIPointer", userptr);
    glWaitSync = (PFNGLWAITSYNCPROC) load("glWaitSync", userptr);
}
static void glad_gl_load_GL_ES_VERSION_3_1( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_ES_VERSION_3_1) return;
    glActiveShaderProgram = (PFNGLACTIVESHADERPROGRAMPROC) load("glActiveShaderProgram", userptr);
    glBindImageTexture = (PFNGLBINDIMAGETEXTUREPROC) load("glBindImageTexture", userptr);
    glBindProgramPipeline = (PFNGLBINDPROGRAMPIPELINEPROC) load("glBindProgramPipeline", userptr);
    glBindVertexBuffer = (PFNGLBINDVERTEXBUFFERPROC) load("glBindVertexBuffer", userptr);
    glCreateShaderProgramv = (PFNGLCREATESHADERPROGRAMVPROC) load("glCreateShaderProgramv", userptr);
    glDeleteProgramPipelines = (PFNGLDELETEPROGRAMPIPELINESPROC) load("glDeleteProgramPipelines", userptr);
    glDispatchCompute = (PFNGLDISPATCHCOMPUTEPROC) load("glDispatchCompute", userptr);
    glDispatchComputeIndirect = (PFNGLDISPATCHCOMPUTEINDIRECTPROC) load("glDispatchComputeIndirect", userptr);
    glDrawArraysIndirect = (PFNGLDRAWARRAYSINDIRECTPROC) load("glDrawArraysIndirect", userptr);
    glDrawElementsIndirect = (PFNGLDRAWELEMENTSINDIRECTPROC) load("glDrawElementsIndirect", userptr);
    glFramebufferParameteri = (PFNGLFRAMEBUFFERPARAMETERIPROC) load("glFramebufferParameteri", userptr);
    glGenProgramPipelines = (PFNGLGENPROGRAMPIPELINESPROC) load("glGenProgramPipelines", userptr);
    glGetBooleani_v = (PFNGLGETBOOLEANI_VPROC) load("glGetBooleani_v", userptr);
    glGetFramebufferParameteriv = (PFNGLGETFRAMEBUFFERPARAMETERIVPROC) load("glGetFramebufferParameteriv", userptr);
    glGetMultisamplefv = (PFNGLGETMULTISAMPLEFVPROC) load("glGetMultisamplefv", userptr);
    glGetProgramInterfaceiv = (PFNGLGETPROGRAMINTERFACEIVPROC) load("glGetProgramInterfaceiv", userptr);
    glGetProgramPipelineInfoLog = (PFNGLGETPROGRAMPIPELINEINFOLOGPROC) load("glGetProgramPipelineInfoLog", userptr);
    glGetProgramPipelineiv = (PFNGLGETPROGRAMPIPELINEIVPROC) load("glGetProgramPipelineiv", userptr);
    glGetProgramResourceIndex = (PFNGLGETPROGRAMRESOURCEINDEXPROC) load("glGetProgramResourceIndex", userptr);
    glGetProgramResourceLocation = (PFNGLGETPROGRAMRESOURCELOCATIONPROC) load("glGetProgramResourceLocation", userptr);
    glGetProgramResourceName = (PFNGLGETPROGRAMRESOURCENAMEPROC) load("glGetProgramResourceName", userptr);
    glGetProgramResourceiv = (PFNGLGETPROGRAMRESOURCEIVPROC) load("glGetProgramResourceiv", userptr);
    glGetTexLevelParameterfv = (PFNGLGETTEXLEVELPARAMETERFVPROC) load("glGetTexLevelParameterfv", userptr);
    glGetTexLevelParameteriv = (PFNGLGETTEXLEVELPARAMETERIVPROC) load("glGetTexLevelParameteriv", userptr);
    glIsProgramPipeline = (PFNGLISPROGRAMPIPELINEPROC) load("glIsProgramPipeline", userptr);
    glMemoryBarrier = (PFNGLMEMORYBARRIERPROC) load("glMemoryBarrier", userptr);
    glMemoryBarrierByRegion = (PFNGLMEMORYBARRIERBYREGIONPROC) load("glMemoryBarrierByRegion", userptr);
    glProgramUniform1f = (PFNGLPROGRAMUNIFORM1FPROC) load("glProgramUniform1f", userptr);
    glProgramUniform1fv = (PFNGLPROGRAMUNIFORM1FVPROC) load("glProgramUniform1fv", userptr);
    glProgramUniform1i = (PFNGLPROGRAMUNIFORM1IPROC) load("glProgramUniform1i", userptr);
    glProgramUniform1iv = (PFNGLPROGRAMUNIFORM1IVPROC) load("glProgramUniform1iv", userptr);
    glProgramUniform1ui = (PFNGLPROGRAMUNIFORM1UIPROC) load("glProgramUniform1ui", userptr);
    glProgramUniform1uiv = (PFNGLPROGRAMUNIFORM1UIVPROC) load("glProgramUniform1uiv", userptr);
    glProgramUniform2f = (PFNGLPROGRAMUNIFORM2FPROC) load("glProgramUniform2f", userptr);
    glProgramUniform2fv = (PFNGLPROGRAMUNIFORM2FVPROC) load("glProgramUniform2fv", userptr);
    glProgramUniform2i = (PFNGLPROGRAMUNIFORM2IPROC) load("glProgramUniform2i", userptr);
    glProgramUniform2iv = (PFNGLPROGRAMUNIFORM2IVPROC) load("glProgramUniform2iv", userptr);
    glProgramUniform2ui = (PFNGLPROGRAMUNIFORM2UIPROC) load("glProgramUniform2ui", userptr);
    glProgramUniform2uiv = (PFNGLPROGRAMUNIFORM2UIVPROC) load("glProgramUniform2uiv", userptr);
    glProgramUniform3f = (PFNGLPROGRAMUNIFORM3FPROC) load("glProgramUniform3f", userptr);
    glProgramUniform3fv = (PFNGLPROGRAMUNIFORM3FVPROC) load("glProgramUniform3fv", userptr);
    glProgramUniform3i = (PFNGLPROGRAMUNIFORM3IPROC) load("glProgramUniform3i", userptr);
    glProgramUniform3iv = (PFNGLPROGRAMUNIFORM3IVPROC) load("glProgramUniform3iv", userptr);
    glProgramUniform3ui = (PFNGLPROGRAMUNIFORM3UIPROC) load("glProgramUniform3ui", userptr);
    glProgramUniform3uiv = (PFNGLPROGRAMUNIFORM3UIVPROC) load("glProgramUniform3uiv", userptr);
    glProgramUniform4f = (PFNGLPROGRAMUNIFORM4FPROC) load("glProgramUniform4f", userptr);
    glProgramUniform4fv = (PFNGLPROGRAMUNIFORM4FVPROC) load("glProgramUniform4fv", userptr);
    glProgramUniform4i = (PFNGLPROGRAMUNIFORM4IPROC) load("glProgramUniform4i", userptr);
    glProgramUniform4iv = (PFNGLPROGRAMUNIFORM4IVPROC) load("glProgramUniform4iv", userptr);
    glProgramUniform4ui = (PFNGLPROGRAMUNIFORM4UIPROC) load("glProgramUniform4ui", userptr);
    glProgramUniform4uiv = (PFNGLPROGRAMUNIFORM4UIVPROC) load("glProgramUniform4uiv", userptr);
    glProgramUniformMatrix2fv = (PFNGLPROGRAMUNIFORMMATRIX2FVPROC) load("glProgramUniformMatrix2fv", userptr);
    glProgramUniformMatrix2x3fv = (PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC) load("glProgramUniformMatrix2x3fv", userptr);
    glProgramUniformMatrix2x4fv = (PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC) load("glProgramUniformMatrix2x4fv", userptr);
    glProgramUniformMatrix3fv = (PFNGLPROGRAMUNIFORMMATRIX3FVPROC) load("glProgramUniformMatrix3fv", userptr);
    glProgramUniformMatrix3x2fv = (PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC) load("glProgramUniformMatrix3x2fv", userptr);
    glProgramUniformMatrix3x4fv = (PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC) load("glProgramUniformMatrix3x4fv", userptr);
    glProgramUniformMatrix4fv = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC) load("glProgramUniformMatrix4fv", userptr);
    glProgramUniformMatrix4x2fv = (PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC) load("glProgramUniformMatrix4x2fv", userptr);
    glProgramUniformMatrix4x3fv = (PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC) load("glProgramUniformMatrix4x3fv", userptr);
    glSampleMaski = (PFNGLSAMPLEMASKIPROC) load("glSampleMaski", userptr);
    glTexStorage2DMultisample = (PFNGLTEXSTORAGE2DMULTISAMPLEPROC) load("glTexStorage2DMultisample", userptr);
    glUseProgramStages = (PFNGLUSEPROGRAMSTAGESPROC) load("glUseProgramStages", userptr);
    glValidateProgramPipeline = (PFNGLVALIDATEPROGRAMPIPELINEPROC) load("glValidateProgramPipeline", userptr);
    glVertexAttribBinding = (PFNGLVERTEXATTRIBBINDINGPROC) load("glVertexAttribBinding", userptr);
    glVertexAttribFormat = (PFNGLVERTEXATTRIBFORMATPROC) load("glVertexAttribFormat", userptr);
    glVertexAttribIFormat = (PFNGLVERTEXATTRIBIFORMATPROC) load("glVertexAttribIFormat", userptr);
    glVertexBindingDivisor = (PFNGLVERTEXBINDINGDIVISORPROC) load("glVertexBindingDivisor", userptr);
}
static void glad_gl_load_GL_ES_VERSION_3_2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_ES_VERSION_3_2) return;
    glBlendBarrier = (PFNGLBLENDBARRIERPROC) load("glBlendBarrier", userptr);
    glBlendEquationSeparatei = (PFNGLBLENDEQUATIONSEPARATEIPROC) load("glBlendEquationSeparatei", userptr);
    glBlendEquationi = (PFNGLBLENDEQUATIONIPROC) load("glBlendEquationi", userptr);
    glBlendFuncSeparatei = (PFNGLBLENDFUNCSEPARATEIPROC) load("glBlendFuncSeparatei", userptr);
    glBlendFunci = (PFNGLBLENDFUNCIPROC) load("glBlendFunci", userptr);
    glColorMaski = (PFNGLCOLORMASKIPROC) load("glColorMaski", userptr);
    glCopyImageSubData = (PFNGLCOPYIMAGESUBDATAPROC) load("glCopyImageSubData", userptr);
    glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC) load("glDebugMessageCallback", userptr);
    glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC) load("glDebugMessageControl", userptr);
    glDebugMessageInsert = (PFNGLDEBUGMESSAGEINSERTPROC) load("glDebugMessageInsert", userptr);
    glDisablei = (PFNGLDISABLEIPROC) load("glDisablei", userptr);
    glDrawElementsBaseVertex = (PFNGLDRAWELEMENTSBASEVERTEXPROC) load("glDrawElementsBaseVertex", userptr);
    glDrawElementsInstancedBaseVertex = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC) load("glDrawElementsInstancedBaseVertex", userptr);
    glDrawRangeElementsBaseVertex = (PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC) load("glDrawRangeElementsBaseVertex", userptr);
    glEnablei = (PFNGLENABLEIPROC) load("glEnablei", userptr);
    glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC) load("glFramebufferTexture", userptr);
    glGetDebugMessageLog = (PFNGLGETDEBUGMESSAGELOGPROC) load("glGetDebugMessageLog", userptr);
    glGetGraphicsResetStatus = (PFNGLGETGRAPHICSRESETSTATUSPROC) load("glGetGraphicsResetStatus", userptr);
    glGetObjectLabel = (PFNGLGETOBJECTLABELPROC) load("glGetObjectLabel", userptr);
    glGetObjectPtrLabel = (PFNGLGETOBJECTPTRLABELPROC) load("glGetObjectPtrLabel", userptr);
    glGetPointerv = (PFNGLGETPOINTERVPROC) load("glGetPointerv", userptr);
    glGetSamplerParameterIiv = (PFNGLGETSAMPLERPARAMETERIIVPROC) load("glGetSamplerParameterIiv", userptr);
    glGetSamplerParameterIuiv = (PFNGLGETSAMPLERPARAMETERIUIVPROC) load("glGetSamplerParameterIuiv", userptr);
    glGetTexParameterIiv = (PFNGLGETTEXPARAMETERIIVPROC) load("glGetTexParameterIiv", userptr);
    glGetTexParameterIuiv = (PFNGLGETTEXPARAMETERIUIVPROC) load("glGetTexParameterIuiv", userptr);
    glGetnUniformfv = (PFNGLGETNUNIFORMFVPROC) load("glGetnUniformfv", userptr);
    glGetnUniformiv = (PFNGLGETNUNIFORMIVPROC) load("glGetnUniformiv", userptr);
    glGetnUniformuiv = (PFNGLGETNUNIFORMUIVPROC) load("glGetnUniformuiv", userptr);
    glIsEnabledi = (PFNGLISENABLEDIPROC) load("glIsEnabledi", userptr);
    glMinSampleShading = (PFNGLMINSAMPLESHADINGPROC) load("glMinSampleShading", userptr);
    glObjectLabel = (PFNGLOBJECTLABELPROC) load("glObjectLabel", userptr);
    glObjectPtrLabel = (PFNGLOBJECTPTRLABELPROC) load("glObjectPtrLabel", userptr);
    glPatchParameteri = (PFNGLPATCHPARAMETERIPROC) load("glPatchParameteri", userptr);
    glPopDebugGroup = (PFNGLPOPDEBUGGROUPPROC) load("glPopDebugGroup", userptr);
    glPrimitiveBoundingBox = (PFNGLPRIMITIVEBOUNDINGBOXPROC) load("glPrimitiveBoundingBox", userptr);
    glPushDebugGroup = (PFNGLPUSHDEBUGGROUPPROC) load("glPushDebugGroup", userptr);
    glReadnPixels = (PFNGLREADNPIXELSPROC) load("glReadnPixels", userptr);
    glSamplerParameterIiv = (PFNGLSAMPLERPARAMETERIIVPROC) load("glSamplerParameterIiv", userptr);
    glSamplerParameterIuiv = (PFNGLSAMPLERPARAMETERIUIVPROC) load("glSamplerParameterIuiv", userptr);
    glTexBuffer = (PFNGLTEXBUFFERPROC) load("glTexBuffer", userptr);
    glTexBufferRange = (PFNGLTEXBUFFERRANGEPROC) load("glTexBufferRange", userptr);
    glTexParameterIiv = (PFNGLTEXPARAMETERIIVPROC) load("glTexParameterIiv", userptr);
    glTexParameterIuiv = (PFNGLTEXPARAMETERIUIVPROC) load("glTexParameterIuiv", userptr);
    glTexStorage3DMultisample = (PFNGLTEXSTORAGE3DMULTISAMPLEPROC) load("glTexStorage3DMultisample", userptr);
}
static void glad_gl_load_GL_AMD_framebuffer_multisample_advanced( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_AMD_framebuffer_multisample_advanced) return;
    glNamedRenderbufferStorageMultisampleAdvancedAMD = (PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEADVANCEDAMDPROC) load("glNamedRenderbufferStorageMultisampleAdvancedAMD", userptr);
    glRenderbufferStorageMultisampleAdvancedAMD = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEADVANCEDAMDPROC) load("glRenderbufferStorageMultisampleAdvancedAMD", userptr);
}
static void glad_gl_load_GL_AMD_performance_monitor( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_AMD_performance_monitor) return;
    glBeginPerfMonitorAMD = (PFNGLBEGINPERFMONITORAMDPROC) load("glBeginPerfMonitorAMD", userptr);
    glDeletePerfMonitorsAMD = (PFNGLDELETEPERFMONITORSAMDPROC) load("glDeletePerfMonitorsAMD", userptr);
    glEndPerfMonitorAMD = (PFNGLENDPERFMONITORAMDPROC) load("glEndPerfMonitorAMD", userptr);
    glGenPerfMonitorsAMD = (PFNGLGENPERFMONITORSAMDPROC) load("glGenPerfMonitorsAMD", userptr);
    glGetPerfMonitorCounterDataAMD = (PFNGLGETPERFMONITORCOUNTERDATAAMDPROC) load("glGetPerfMonitorCounterDataAMD", userptr);
    glGetPerfMonitorCounterInfoAMD = (PFNGLGETPERFMONITORCOUNTERINFOAMDPROC) load("glGetPerfMonitorCounterInfoAMD", userptr);
    glGetPerfMonitorCounterStringAMD = (PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC) load("glGetPerfMonitorCounterStringAMD", userptr);
    glGetPerfMonitorCountersAMD = (PFNGLGETPERFMONITORCOUNTERSAMDPROC) load("glGetPerfMonitorCountersAMD", userptr);
    glGetPerfMonitorGroupStringAMD = (PFNGLGETPERFMONITORGROUPSTRINGAMDPROC) load("glGetPerfMonitorGroupStringAMD", userptr);
    glGetPerfMonitorGroupsAMD = (PFNGLGETPERFMONITORGROUPSAMDPROC) load("glGetPerfMonitorGroupsAMD", userptr);
    glSelectPerfMonitorCountersAMD = (PFNGLSELECTPERFMONITORCOUNTERSAMDPROC) load("glSelectPerfMonitorCountersAMD", userptr);
}
static void glad_gl_load_GL_ANGLE_framebuffer_blit( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_ANGLE_framebuffer_blit) return;
    glBlitFramebufferANGLE = (PFNGLBLITFRAMEBUFFERANGLEPROC) load("glBlitFramebufferANGLE", userptr);
}
static void glad_gl_load_GL_ANGLE_framebuffer_multisample( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_ANGLE_framebuffer_multisample) return;
    glRenderbufferStorageMultisampleANGLE = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC) load("glRenderbufferStorageMultisampleANGLE", userptr);
}
static void glad_gl_load_GL_ANGLE_instanced_arrays( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_ANGLE_instanced_arrays) return;
    glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC) load("glDrawArraysInstanced", userptr);
    glDrawArraysInstancedANGLE = (PFNGLDRAWARRAYSINSTANCEDANGLEPROC) load("glDrawArraysInstancedANGLE", userptr);
    glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC) load("glDrawElementsInstanced", userptr);
    glDrawElementsInstancedANGLE = (PFNGLDRAWELEMENTSINSTANCEDANGLEPROC) load("glDrawElementsInstancedANGLE", userptr);
    glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC) load("glVertexAttribDivisor", userptr);
    glVertexAttribDivisorANGLE = (PFNGLVERTEXATTRIBDIVISORANGLEPROC) load("glVertexAttribDivisorANGLE", userptr);
}
static void glad_gl_load_GL_ANGLE_translated_shader_source( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_ANGLE_translated_shader_source) return;
    glGetTranslatedShaderSourceANGLE = (PFNGLGETTRANSLATEDSHADERSOURCEANGLEPROC) load("glGetTranslatedShaderSourceANGLE", userptr);
}
static void glad_gl_load_GL_APPLE_copy_texture_levels( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_APPLE_copy_texture_levels) return;
    glCopyTextureLevelsAPPLE = (PFNGLCOPYTEXTURELEVELSAPPLEPROC) load("glCopyTextureLevelsAPPLE", userptr);
}
static void glad_gl_load_GL_APPLE_framebuffer_multisample( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_APPLE_framebuffer_multisample) return;
    glRenderbufferStorageMultisampleAPPLE = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEAPPLEPROC) load("glRenderbufferStorageMultisampleAPPLE", userptr);
    glResolveMultisampleFramebufferAPPLE = (PFNGLRESOLVEMULTISAMPLEFRAMEBUFFERAPPLEPROC) load("glResolveMultisampleFramebufferAPPLE", userptr);
}
static void glad_gl_load_GL_APPLE_sync( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_APPLE_sync) return;
    glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC) load("glClientWaitSync", userptr);
    glClientWaitSyncAPPLE = (PFNGLCLIENTWAITSYNCAPPLEPROC) load("glClientWaitSyncAPPLE", userptr);
    glDeleteSync = (PFNGLDELETESYNCPROC) load("glDeleteSync", userptr);
    glDeleteSyncAPPLE = (PFNGLDELETESYNCAPPLEPROC) load("glDeleteSyncAPPLE", userptr);
    glFenceSync = (PFNGLFENCESYNCPROC) load("glFenceSync", userptr);
    glFenceSyncAPPLE = (PFNGLFENCESYNCAPPLEPROC) load("glFenceSyncAPPLE", userptr);
    glGetInteger64v = (PFNGLGETINTEGER64VPROC) load("glGetInteger64v", userptr);
    glGetInteger64vAPPLE = (PFNGLGETINTEGER64VAPPLEPROC) load("glGetInteger64vAPPLE", userptr);
    glGetSynciv = (PFNGLGETSYNCIVPROC) load("glGetSynciv", userptr);
    glGetSyncivAPPLE = (PFNGLGETSYNCIVAPPLEPROC) load("glGetSyncivAPPLE", userptr);
    glIsSync = (PFNGLISSYNCPROC) load("glIsSync", userptr);
    glIsSyncAPPLE = (PFNGLISSYNCAPPLEPROC) load("glIsSyncAPPLE", userptr);
    glWaitSync = (PFNGLWAITSYNCPROC) load("glWaitSync", userptr);
    glWaitSyncAPPLE = (PFNGLWAITSYNCAPPLEPROC) load("glWaitSyncAPPLE", userptr);
}
static void glad_gl_load_GL_EXT_EGL_image_storage( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_EGL_image_storage) return;
    glEGLImageTargetTexStorageEXT = (PFNGLEGLIMAGETARGETTEXSTORAGEEXTPROC) load("glEGLImageTargetTexStorageEXT", userptr);
    glEGLImageTargetTextureStorageEXT = (PFNGLEGLIMAGETARGETTEXTURESTORAGEEXTPROC) load("glEGLImageTargetTextureStorageEXT", userptr);
}
static void glad_gl_load_GL_EXT_base_instance( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_base_instance) return;
    glDrawArraysInstancedBaseInstance = (PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC) load("glDrawArraysInstancedBaseInstance", userptr);
    glDrawArraysInstancedBaseInstanceEXT = (PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEEXTPROC) load("glDrawArraysInstancedBaseInstanceEXT", userptr);
    glDrawElementsInstancedBaseInstance = (PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC) load("glDrawElementsInstancedBaseInstance", userptr);
    glDrawElementsInstancedBaseInstanceEXT = (PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEEXTPROC) load("glDrawElementsInstancedBaseInstanceEXT", userptr);
    glDrawElementsInstancedBaseVertexBaseInstance = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC) load("glDrawElementsInstancedBaseVertexBaseInstance", userptr);
    glDrawElementsInstancedBaseVertexBaseInstanceEXT = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEEXTPROC) load("glDrawElementsInstancedBaseVertexBaseInstanceEXT", userptr);
}
static void glad_gl_load_GL_EXT_blend_func_extended( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_blend_func_extended) return;
    glBindFragDataLocation = (PFNGLBINDFRAGDATALOCATIONPROC) load("glBindFragDataLocation", userptr);
    glBindFragDataLocationEXT = (PFNGLBINDFRAGDATALOCATIONEXTPROC) load("glBindFragDataLocationEXT", userptr);
    glBindFragDataLocationIndexed = (PFNGLBINDFRAGDATALOCATIONINDEXEDPROC) load("glBindFragDataLocationIndexed", userptr);
    glBindFragDataLocationIndexedEXT = (PFNGLBINDFRAGDATALOCATIONINDEXEDEXTPROC) load("glBindFragDataLocationIndexedEXT", userptr);
    glGetFragDataIndex = (PFNGLGETFRAGDATAINDEXPROC) load("glGetFragDataIndex", userptr);
    glGetFragDataIndexEXT = (PFNGLGETFRAGDATAINDEXEXTPROC) load("glGetFragDataIndexEXT", userptr);
    glGetProgramResourceLocationIndexEXT = (PFNGLGETPROGRAMRESOURCELOCATIONINDEXEXTPROC) load("glGetProgramResourceLocationIndexEXT", userptr);
}
static void glad_gl_load_GL_EXT_buffer_storage( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_buffer_storage) return;
    glBufferStorage = (PFNGLBUFFERSTORAGEPROC) load("glBufferStorage", userptr);
    glBufferStorageEXT = (PFNGLBUFFERSTORAGEEXTPROC) load("glBufferStorageEXT", userptr);
}
static void glad_gl_load_GL_EXT_clear_texture( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_clear_texture) return;
    glClearTexImage = (PFNGLCLEARTEXIMAGEPROC) load("glClearTexImage", userptr);
    glClearTexImageEXT = (PFNGLCLEARTEXIMAGEEXTPROC) load("glClearTexImageEXT", userptr);
    glClearTexSubImage = (PFNGLCLEARTEXSUBIMAGEPROC) load("glClearTexSubImage", userptr);
    glClearTexSubImageEXT = (PFNGLCLEARTEXSUBIMAGEEXTPROC) load("glClearTexSubImageEXT", userptr);
}
static void glad_gl_load_GL_EXT_clip_control( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_clip_control) return;
    glClipControl = (PFNGLCLIPCONTROLPROC) load("glClipControl", userptr);
    glClipControlEXT = (PFNGLCLIPCONTROLEXTPROC) load("glClipControlEXT", userptr);
}
static void glad_gl_load_GL_EXT_copy_image( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_copy_image) return;
    glCopyImageSubData = (PFNGLCOPYIMAGESUBDATAPROC) load("glCopyImageSubData", userptr);
    glCopyImageSubDataEXT = (PFNGLCOPYIMAGESUBDATAEXTPROC) load("glCopyImageSubDataEXT", userptr);
}
static void glad_gl_load_GL_EXT_debug_label( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_debug_label) return;
    glGetObjectLabelEXT = (PFNGLGETOBJECTLABELEXTPROC) load("glGetObjectLabelEXT", userptr);
    glLabelObjectEXT = (PFNGLLABELOBJECTEXTPROC) load("glLabelObjectEXT", userptr);
}
static void glad_gl_load_GL_EXT_debug_marker( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_debug_marker) return;
    glInsertEventMarkerEXT = (PFNGLINSERTEVENTMARKEREXTPROC) load("glInsertEventMarkerEXT", userptr);
    glPopGroupMarkerEXT = (PFNGLPOPGROUPMARKEREXTPROC) load("glPopGroupMarkerEXT", userptr);
    glPushGroupMarkerEXT = (PFNGLPUSHGROUPMARKEREXTPROC) load("glPushGroupMarkerEXT", userptr);
}
static void glad_gl_load_GL_EXT_discard_framebuffer( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_discard_framebuffer) return;
    glDiscardFramebufferEXT = (PFNGLDISCARDFRAMEBUFFEREXTPROC) load("glDiscardFramebufferEXT", userptr);
}
static void glad_gl_load_GL_EXT_disjoint_timer_query( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_disjoint_timer_query) return;
    glBeginQueryEXT = (PFNGLBEGINQUERYEXTPROC) load("glBeginQueryEXT", userptr);
    glDeleteQueriesEXT = (PFNGLDELETEQUERIESEXTPROC) load("glDeleteQueriesEXT", userptr);
    glEndQueryEXT = (PFNGLENDQUERYEXTPROC) load("glEndQueryEXT", userptr);
    glGenQueriesEXT = (PFNGLGENQUERIESEXTPROC) load("glGenQueriesEXT", userptr);
    glGetQueryObjecti64v = (PFNGLGETQUERYOBJECTI64VPROC) load("glGetQueryObjecti64v", userptr);
    glGetQueryObjecti64vEXT = (PFNGLGETQUERYOBJECTI64VEXTPROC) load("glGetQueryObjecti64vEXT", userptr);
    glGetQueryObjectiv = (PFNGLGETQUERYOBJECTIVPROC) load("glGetQueryObjectiv", userptr);
    glGetQueryObjectivEXT = (PFNGLGETQUERYOBJECTIVEXTPROC) load("glGetQueryObjectivEXT", userptr);
    glGetQueryObjectui64v = (PFNGLGETQUERYOBJECTUI64VPROC) load("glGetQueryObjectui64v", userptr);
    glGetQueryObjectui64vEXT = (PFNGLGETQUERYOBJECTUI64VEXTPROC) load("glGetQueryObjectui64vEXT", userptr);
    glGetQueryObjectuivEXT = (PFNGLGETQUERYOBJECTUIVEXTPROC) load("glGetQueryObjectuivEXT", userptr);
    glGetQueryivEXT = (PFNGLGETQUERYIVEXTPROC) load("glGetQueryivEXT", userptr);
    glIsQueryEXT = (PFNGLISQUERYEXTPROC) load("glIsQueryEXT", userptr);
    glQueryCounter = (PFNGLQUERYCOUNTERPROC) load("glQueryCounter", userptr);
    glQueryCounterEXT = (PFNGLQUERYCOUNTEREXTPROC) load("glQueryCounterEXT", userptr);
}
static void glad_gl_load_GL_EXT_draw_buffers( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_draw_buffers) return;
    glDrawBuffers = (PFNGLDRAWBUFFERSPROC) load("glDrawBuffers", userptr);
    glDrawBuffersEXT = (PFNGLDRAWBUFFERSEXTPROC) load("glDrawBuffersEXT", userptr);
}
static void glad_gl_load_GL_EXT_draw_buffers_indexed( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_draw_buffers_indexed) return;
    glBlendEquationSeparatei = (PFNGLBLENDEQUATIONSEPARATEIPROC) load("glBlendEquationSeparatei", userptr);
    glBlendEquationSeparateiEXT = (PFNGLBLENDEQUATIONSEPARATEIEXTPROC) load("glBlendEquationSeparateiEXT", userptr);
    glBlendEquationi = (PFNGLBLENDEQUATIONIPROC) load("glBlendEquationi", userptr);
    glBlendEquationiEXT = (PFNGLBLENDEQUATIONIEXTPROC) load("glBlendEquationiEXT", userptr);
    glBlendFuncSeparatei = (PFNGLBLENDFUNCSEPARATEIPROC) load("glBlendFuncSeparatei", userptr);
    glBlendFuncSeparateiEXT = (PFNGLBLENDFUNCSEPARATEIEXTPROC) load("glBlendFuncSeparateiEXT", userptr);
    glBlendFunci = (PFNGLBLENDFUNCIPROC) load("glBlendFunci", userptr);
    glBlendFunciEXT = (PFNGLBLENDFUNCIEXTPROC) load("glBlendFunciEXT", userptr);
    glColorMaski = (PFNGLCOLORMASKIPROC) load("glColorMaski", userptr);
    glColorMaskiEXT = (PFNGLCOLORMASKIEXTPROC) load("glColorMaskiEXT", userptr);
    glDisablei = (PFNGLDISABLEIPROC) load("glDisablei", userptr);
    glDisableiEXT = (PFNGLDISABLEIEXTPROC) load("glDisableiEXT", userptr);
    glEnablei = (PFNGLENABLEIPROC) load("glEnablei", userptr);
    glEnableiEXT = (PFNGLENABLEIEXTPROC) load("glEnableiEXT", userptr);
    glIsEnabledi = (PFNGLISENABLEDIPROC) load("glIsEnabledi", userptr);
    glIsEnablediEXT = (PFNGLISENABLEDIEXTPROC) load("glIsEnablediEXT", userptr);
}
static void glad_gl_load_GL_EXT_draw_elements_base_vertex( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_draw_elements_base_vertex) return;
    glDrawElementsBaseVertex = (PFNGLDRAWELEMENTSBASEVERTEXPROC) load("glDrawElementsBaseVertex", userptr);
    glDrawElementsBaseVertexEXT = (PFNGLDRAWELEMENTSBASEVERTEXEXTPROC) load("glDrawElementsBaseVertexEXT", userptr);
    glDrawElementsInstancedBaseVertex = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC) load("glDrawElementsInstancedBaseVertex", userptr);
    glDrawElementsInstancedBaseVertexEXT = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXEXTPROC) load("glDrawElementsInstancedBaseVertexEXT", userptr);
    glDrawRangeElementsBaseVertex = (PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC) load("glDrawRangeElementsBaseVertex", userptr);
    glDrawRangeElementsBaseVertexEXT = (PFNGLDRAWRANGEELEMENTSBASEVERTEXEXTPROC) load("glDrawRangeElementsBaseVertexEXT", userptr);
    glMultiDrawElementsBaseVertex = (PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC) load("glMultiDrawElementsBaseVertex", userptr);
    glMultiDrawElementsBaseVertexEXT = (PFNGLMULTIDRAWELEMENTSBASEVERTEXEXTPROC) load("glMultiDrawElementsBaseVertexEXT", userptr);
}
static void glad_gl_load_GL_EXT_draw_instanced( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_draw_instanced) return;
    glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC) load("glDrawArraysInstanced", userptr);
    glDrawArraysInstancedEXT = (PFNGLDRAWARRAYSINSTANCEDEXTPROC) load("glDrawArraysInstancedEXT", userptr);
    glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC) load("glDrawElementsInstanced", userptr);
    glDrawElementsInstancedEXT = (PFNGLDRAWELEMENTSINSTANCEDEXTPROC) load("glDrawElementsInstancedEXT", userptr);
}
static void glad_gl_load_GL_EXT_draw_transform_feedback( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_draw_transform_feedback) return;
    glDrawTransformFeedback = (PFNGLDRAWTRANSFORMFEEDBACKPROC) load("glDrawTransformFeedback", userptr);
    glDrawTransformFeedbackEXT = (PFNGLDRAWTRANSFORMFEEDBACKEXTPROC) load("glDrawTransformFeedbackEXT", userptr);
    glDrawTransformFeedbackInstanced = (PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC) load("glDrawTransformFeedbackInstanced", userptr);
    glDrawTransformFeedbackInstancedEXT = (PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDEXTPROC) load("glDrawTransformFeedbackInstancedEXT", userptr);
}
static void glad_gl_load_GL_EXT_external_buffer( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_external_buffer) return;
    glBufferStorageExternalEXT = (PFNGLBUFFERSTORAGEEXTERNALEXTPROC) load("glBufferStorageExternalEXT", userptr);
    glNamedBufferStorageExternalEXT = (PFNGLNAMEDBUFFERSTORAGEEXTERNALEXTPROC) load("glNamedBufferStorageExternalEXT", userptr);
}
static void glad_gl_load_GL_EXT_geometry_shader( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_geometry_shader) return;
    glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC) load("glFramebufferTexture", userptr);
    glFramebufferTextureEXT = (PFNGLFRAMEBUFFERTEXTUREEXTPROC) load("glFramebufferTextureEXT", userptr);
}
static void glad_gl_load_GL_EXT_instanced_arrays( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_instanced_arrays) return;
    glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC) load("glDrawArraysInstanced", userptr);
    glDrawArraysInstancedEXT = (PFNGLDRAWARRAYSINSTANCEDEXTPROC) load("glDrawArraysInstancedEXT", userptr);
    glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC) load("glDrawElementsInstanced", userptr);
    glDrawElementsInstancedEXT = (PFNGLDRAWELEMENTSINSTANCEDEXTPROC) load("glDrawElementsInstancedEXT", userptr);
    glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC) load("glVertexAttribDivisor", userptr);
    glVertexAttribDivisorEXT = (PFNGLVERTEXATTRIBDIVISOREXTPROC) load("glVertexAttribDivisorEXT", userptr);
}
static void glad_gl_load_GL_EXT_map_buffer_range( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_map_buffer_range) return;
    glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC) load("glFlushMappedBufferRange", userptr);
    glFlushMappedBufferRangeEXT = (PFNGLFLUSHMAPPEDBUFFERRANGEEXTPROC) load("glFlushMappedBufferRangeEXT", userptr);
    glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC) load("glMapBufferRange", userptr);
    glMapBufferRangeEXT = (PFNGLMAPBUFFERRANGEEXTPROC) load("glMapBufferRangeEXT", userptr);
}
static void glad_gl_load_GL_EXT_memory_object( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_memory_object) return;
    glBufferStorageMemEXT = (PFNGLBUFFERSTORAGEMEMEXTPROC) load("glBufferStorageMemEXT", userptr);
    glCreateMemoryObjectsEXT = (PFNGLCREATEMEMORYOBJECTSEXTPROC) load("glCreateMemoryObjectsEXT", userptr);
    glDeleteMemoryObjectsEXT = (PFNGLDELETEMEMORYOBJECTSEXTPROC) load("glDeleteMemoryObjectsEXT", userptr);
    glGetMemoryObjectParameterivEXT = (PFNGLGETMEMORYOBJECTPARAMETERIVEXTPROC) load("glGetMemoryObjectParameterivEXT", userptr);
    glGetUnsignedBytei_vEXT = (PFNGLGETUNSIGNEDBYTEI_VEXTPROC) load("glGetUnsignedBytei_vEXT", userptr);
    glGetUnsignedBytevEXT = (PFNGLGETUNSIGNEDBYTEVEXTPROC) load("glGetUnsignedBytevEXT", userptr);
    glIsMemoryObjectEXT = (PFNGLISMEMORYOBJECTEXTPROC) load("glIsMemoryObjectEXT", userptr);
    glMemoryObjectParameterivEXT = (PFNGLMEMORYOBJECTPARAMETERIVEXTPROC) load("glMemoryObjectParameterivEXT", userptr);
    glNamedBufferStorageMemEXT = (PFNGLNAMEDBUFFERSTORAGEMEMEXTPROC) load("glNamedBufferStorageMemEXT", userptr);
    glTexStorageMem2DEXT = (PFNGLTEXSTORAGEMEM2DEXTPROC) load("glTexStorageMem2DEXT", userptr);
    glTexStorageMem2DMultisampleEXT = (PFNGLTEXSTORAGEMEM2DMULTISAMPLEEXTPROC) load("glTexStorageMem2DMultisampleEXT", userptr);
    glTexStorageMem3DEXT = (PFNGLTEXSTORAGEMEM3DEXTPROC) load("glTexStorageMem3DEXT", userptr);
    glTexStorageMem3DMultisampleEXT = (PFNGLTEXSTORAGEMEM3DMULTISAMPLEEXTPROC) load("glTexStorageMem3DMultisampleEXT", userptr);
    glTextureStorageMem2DEXT = (PFNGLTEXTURESTORAGEMEM2DEXTPROC) load("glTextureStorageMem2DEXT", userptr);
    glTextureStorageMem2DMultisampleEXT = (PFNGLTEXTURESTORAGEMEM2DMULTISAMPLEEXTPROC) load("glTextureStorageMem2DMultisampleEXT", userptr);
    glTextureStorageMem3DEXT = (PFNGLTEXTURESTORAGEMEM3DEXTPROC) load("glTextureStorageMem3DEXT", userptr);
    glTextureStorageMem3DMultisampleEXT = (PFNGLTEXTURESTORAGEMEM3DMULTISAMPLEEXTPROC) load("glTextureStorageMem3DMultisampleEXT", userptr);
}
static void glad_gl_load_GL_EXT_memory_object_fd( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_memory_object_fd) return;
    glImportMemoryFdEXT = (PFNGLIMPORTMEMORYFDEXTPROC) load("glImportMemoryFdEXT", userptr);
}
static void glad_gl_load_GL_EXT_memory_object_win32( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_memory_object_win32) return;
    glImportMemoryWin32HandleEXT = (PFNGLIMPORTMEMORYWIN32HANDLEEXTPROC) load("glImportMemoryWin32HandleEXT", userptr);
    glImportMemoryWin32NameEXT = (PFNGLIMPORTMEMORYWIN32NAMEEXTPROC) load("glImportMemoryWin32NameEXT", userptr);
}
static void glad_gl_load_GL_EXT_multi_draw_arrays( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_multi_draw_arrays) return;
    glMultiDrawArrays = (PFNGLMULTIDRAWARRAYSPROC) load("glMultiDrawArrays", userptr);
    glMultiDrawArraysEXT = (PFNGLMULTIDRAWARRAYSEXTPROC) load("glMultiDrawArraysEXT", userptr);
    glMultiDrawElements = (PFNGLMULTIDRAWELEMENTSPROC) load("glMultiDrawElements", userptr);
    glMultiDrawElementsEXT = (PFNGLMULTIDRAWELEMENTSEXTPROC) load("glMultiDrawElementsEXT", userptr);
}
static void glad_gl_load_GL_EXT_multi_draw_indirect( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_multi_draw_indirect) return;
    glMultiDrawArraysIndirect = (PFNGLMULTIDRAWARRAYSINDIRECTPROC) load("glMultiDrawArraysIndirect", userptr);
    glMultiDrawArraysIndirectEXT = (PFNGLMULTIDRAWARRAYSINDIRECTEXTPROC) load("glMultiDrawArraysIndirectEXT", userptr);
    glMultiDrawElementsIndirect = (PFNGLMULTIDRAWELEMENTSINDIRECTPROC) load("glMultiDrawElementsIndirect", userptr);
    glMultiDrawElementsIndirectEXT = (PFNGLMULTIDRAWELEMENTSINDIRECTEXTPROC) load("glMultiDrawElementsIndirectEXT", userptr);
}
static void glad_gl_load_GL_EXT_multisampled_render_to_texture( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_multisampled_render_to_texture) return;
    glFramebufferTexture2DMultisampleEXT = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC) load("glFramebufferTexture2DMultisampleEXT", userptr);
    glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC) load("glRenderbufferStorageMultisample", userptr);
    glRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) load("glRenderbufferStorageMultisampleEXT", userptr);
}
static void glad_gl_load_GL_EXT_multiview_draw_buffers( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_multiview_draw_buffers) return;
    glDrawBuffersIndexedEXT = (PFNGLDRAWBUFFERSINDEXEDEXTPROC) load("glDrawBuffersIndexedEXT", userptr);
    glGetIntegeri_vEXT = (PFNGLGETINTEGERI_VEXTPROC) load("glGetIntegeri_vEXT", userptr);
    glReadBufferIndexedEXT = (PFNGLREADBUFFERINDEXEDEXTPROC) load("glReadBufferIndexedEXT", userptr);
}
static void glad_gl_load_GL_EXT_occlusion_query_boolean( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_occlusion_query_boolean) return;
    glBeginQueryEXT = (PFNGLBEGINQUERYEXTPROC) load("glBeginQueryEXT", userptr);
    glDeleteQueriesEXT = (PFNGLDELETEQUERIESEXTPROC) load("glDeleteQueriesEXT", userptr);
    glEndQueryEXT = (PFNGLENDQUERYEXTPROC) load("glEndQueryEXT", userptr);
    glGenQueriesEXT = (PFNGLGENQUERIESEXTPROC) load("glGenQueriesEXT", userptr);
    glGetQueryObjectuivEXT = (PFNGLGETQUERYOBJECTUIVEXTPROC) load("glGetQueryObjectuivEXT", userptr);
    glGetQueryivEXT = (PFNGLGETQUERYIVEXTPROC) load("glGetQueryivEXT", userptr);
    glIsQueryEXT = (PFNGLISQUERYEXTPROC) load("glIsQueryEXT", userptr);
}
static void glad_gl_load_GL_EXT_polygon_offset_clamp( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_polygon_offset_clamp) return;
    glPolygonOffsetClamp = (PFNGLPOLYGONOFFSETCLAMPPROC) load("glPolygonOffsetClamp", userptr);
    glPolygonOffsetClampEXT = (PFNGLPOLYGONOFFSETCLAMPEXTPROC) load("glPolygonOffsetClampEXT", userptr);
}
static void glad_gl_load_GL_EXT_primitive_bounding_box( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_primitive_bounding_box) return;
    glPrimitiveBoundingBox = (PFNGLPRIMITIVEBOUNDINGBOXPROC) load("glPrimitiveBoundingBox", userptr);
    glPrimitiveBoundingBoxEXT = (PFNGLPRIMITIVEBOUNDINGBOXEXTPROC) load("glPrimitiveBoundingBoxEXT", userptr);
}
static void glad_gl_load_GL_EXT_raster_multisample( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_raster_multisample) return;
    glRasterSamplesEXT = (PFNGLRASTERSAMPLESEXTPROC) load("glRasterSamplesEXT", userptr);
}
static void glad_gl_load_GL_EXT_robustness( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_robustness) return;
    glGetGraphicsResetStatus = (PFNGLGETGRAPHICSRESETSTATUSPROC) load("glGetGraphicsResetStatus", userptr);
    glGetGraphicsResetStatusEXT = (PFNGLGETGRAPHICSRESETSTATUSEXTPROC) load("glGetGraphicsResetStatusEXT", userptr);
    glGetnUniformfv = (PFNGLGETNUNIFORMFVPROC) load("glGetnUniformfv", userptr);
    glGetnUniformfvEXT = (PFNGLGETNUNIFORMFVEXTPROC) load("glGetnUniformfvEXT", userptr);
    glGetnUniformiv = (PFNGLGETNUNIFORMIVPROC) load("glGetnUniformiv", userptr);
    glGetnUniformivEXT = (PFNGLGETNUNIFORMIVEXTPROC) load("glGetnUniformivEXT", userptr);
    glReadnPixels = (PFNGLREADNPIXELSPROC) load("glReadnPixels", userptr);
    glReadnPixelsEXT = (PFNGLREADNPIXELSEXTPROC) load("glReadnPixelsEXT", userptr);
}
static void glad_gl_load_GL_EXT_semaphore( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_semaphore) return;
    glDeleteSemaphoresEXT = (PFNGLDELETESEMAPHORESEXTPROC) load("glDeleteSemaphoresEXT", userptr);
    glGenSemaphoresEXT = (PFNGLGENSEMAPHORESEXTPROC) load("glGenSemaphoresEXT", userptr);
    glGetSemaphoreParameterui64vEXT = (PFNGLGETSEMAPHOREPARAMETERUI64VEXTPROC) load("glGetSemaphoreParameterui64vEXT", userptr);
    glGetUnsignedBytei_vEXT = (PFNGLGETUNSIGNEDBYTEI_VEXTPROC) load("glGetUnsignedBytei_vEXT", userptr);
    glGetUnsignedBytevEXT = (PFNGLGETUNSIGNEDBYTEVEXTPROC) load("glGetUnsignedBytevEXT", userptr);
    glIsSemaphoreEXT = (PFNGLISSEMAPHOREEXTPROC) load("glIsSemaphoreEXT", userptr);
    glSemaphoreParameterui64vEXT = (PFNGLSEMAPHOREPARAMETERUI64VEXTPROC) load("glSemaphoreParameterui64vEXT", userptr);
    glSignalSemaphoreEXT = (PFNGLSIGNALSEMAPHOREEXTPROC) load("glSignalSemaphoreEXT", userptr);
    glWaitSemaphoreEXT = (PFNGLWAITSEMAPHOREEXTPROC) load("glWaitSemaphoreEXT", userptr);
}
static void glad_gl_load_GL_EXT_semaphore_fd( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_semaphore_fd) return;
    glImportSemaphoreFdEXT = (PFNGLIMPORTSEMAPHOREFDEXTPROC) load("glImportSemaphoreFdEXT", userptr);
}
static void glad_gl_load_GL_EXT_semaphore_win32( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_semaphore_win32) return;
    glImportSemaphoreWin32HandleEXT = (PFNGLIMPORTSEMAPHOREWIN32HANDLEEXTPROC) load("glImportSemaphoreWin32HandleEXT", userptr);
    glImportSemaphoreWin32NameEXT = (PFNGLIMPORTSEMAPHOREWIN32NAMEEXTPROC) load("glImportSemaphoreWin32NameEXT", userptr);
}
static void glad_gl_load_GL_EXT_separate_shader_objects( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_separate_shader_objects) return;
    glActiveShaderProgramEXT = (PFNGLACTIVESHADERPROGRAMEXTPROC) load("glActiveShaderProgramEXT", userptr);
    glBindProgramPipelineEXT = (PFNGLBINDPROGRAMPIPELINEEXTPROC) load("glBindProgramPipelineEXT", userptr);
    glCreateShaderProgramvEXT = (PFNGLCREATESHADERPROGRAMVEXTPROC) load("glCreateShaderProgramvEXT", userptr);
    glDeleteProgramPipelinesEXT = (PFNGLDELETEPROGRAMPIPELINESEXTPROC) load("glDeleteProgramPipelinesEXT", userptr);
    glGenProgramPipelinesEXT = (PFNGLGENPROGRAMPIPELINESEXTPROC) load("glGenProgramPipelinesEXT", userptr);
    glGetProgramPipelineInfoLogEXT = (PFNGLGETPROGRAMPIPELINEINFOLOGEXTPROC) load("glGetProgramPipelineInfoLogEXT", userptr);
    glGetProgramPipelineivEXT = (PFNGLGETPROGRAMPIPELINEIVEXTPROC) load("glGetProgramPipelineivEXT", userptr);
    glIsProgramPipelineEXT = (PFNGLISPROGRAMPIPELINEEXTPROC) load("glIsProgramPipelineEXT", userptr);
    glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC) load("glProgramParameteri", userptr);
    glProgramParameteriEXT = (PFNGLPROGRAMPARAMETERIEXTPROC) load("glProgramParameteriEXT", userptr);
    glProgramUniform1f = (PFNGLPROGRAMUNIFORM1FPROC) load("glProgramUniform1f", userptr);
    glProgramUniform1fEXT = (PFNGLPROGRAMUNIFORM1FEXTPROC) load("glProgramUniform1fEXT", userptr);
    glProgramUniform1fv = (PFNGLPROGRAMUNIFORM1FVPROC) load("glProgramUniform1fv", userptr);
    glProgramUniform1fvEXT = (PFNGLPROGRAMUNIFORM1FVEXTPROC) load("glProgramUniform1fvEXT", userptr);
    glProgramUniform1i = (PFNGLPROGRAMUNIFORM1IPROC) load("glProgramUniform1i", userptr);
    glProgramUniform1iEXT = (PFNGLPROGRAMUNIFORM1IEXTPROC) load("glProgramUniform1iEXT", userptr);
    glProgramUniform1iv = (PFNGLPROGRAMUNIFORM1IVPROC) load("glProgramUniform1iv", userptr);
    glProgramUniform1ivEXT = (PFNGLPROGRAMUNIFORM1IVEXTPROC) load("glProgramUniform1ivEXT", userptr);
    glProgramUniform1ui = (PFNGLPROGRAMUNIFORM1UIPROC) load("glProgramUniform1ui", userptr);
    glProgramUniform1uiEXT = (PFNGLPROGRAMUNIFORM1UIEXTPROC) load("glProgramUniform1uiEXT", userptr);
    glProgramUniform1uiv = (PFNGLPROGRAMUNIFORM1UIVPROC) load("glProgramUniform1uiv", userptr);
    glProgramUniform1uivEXT = (PFNGLPROGRAMUNIFORM1UIVEXTPROC) load("glProgramUniform1uivEXT", userptr);
    glProgramUniform2f = (PFNGLPROGRAMUNIFORM2FPROC) load("glProgramUniform2f", userptr);
    glProgramUniform2fEXT = (PFNGLPROGRAMUNIFORM2FEXTPROC) load("glProgramUniform2fEXT", userptr);
    glProgramUniform2fv = (PFNGLPROGRAMUNIFORM2FVPROC) load("glProgramUniform2fv", userptr);
    glProgramUniform2fvEXT = (PFNGLPROGRAMUNIFORM2FVEXTPROC) load("glProgramUniform2fvEXT", userptr);
    glProgramUniform2i = (PFNGLPROGRAMUNIFORM2IPROC) load("glProgramUniform2i", userptr);
    glProgramUniform2iEXT = (PFNGLPROGRAMUNIFORM2IEXTPROC) load("glProgramUniform2iEXT", userptr);
    glProgramUniform2iv = (PFNGLPROGRAMUNIFORM2IVPROC) load("glProgramUniform2iv", userptr);
    glProgramUniform2ivEXT = (PFNGLPROGRAMUNIFORM2IVEXTPROC) load("glProgramUniform2ivEXT", userptr);
    glProgramUniform2ui = (PFNGLPROGRAMUNIFORM2UIPROC) load("glProgramUniform2ui", userptr);
    glProgramUniform2uiEXT = (PFNGLPROGRAMUNIFORM2UIEXTPROC) load("glProgramUniform2uiEXT", userptr);
    glProgramUniform2uiv = (PFNGLPROGRAMUNIFORM2UIVPROC) load("glProgramUniform2uiv", userptr);
    glProgramUniform2uivEXT = (PFNGLPROGRAMUNIFORM2UIVEXTPROC) load("glProgramUniform2uivEXT", userptr);
    glProgramUniform3f = (PFNGLPROGRAMUNIFORM3FPROC) load("glProgramUniform3f", userptr);
    glProgramUniform3fEXT = (PFNGLPROGRAMUNIFORM3FEXTPROC) load("glProgramUniform3fEXT", userptr);
    glProgramUniform3fv = (PFNGLPROGRAMUNIFORM3FVPROC) load("glProgramUniform3fv", userptr);
    glProgramUniform3fvEXT = (PFNGLPROGRAMUNIFORM3FVEXTPROC) load("glProgramUniform3fvEXT", userptr);
    glProgramUniform3i = (PFNGLPROGRAMUNIFORM3IPROC) load("glProgramUniform3i", userptr);
    glProgramUniform3iEXT = (PFNGLPROGRAMUNIFORM3IEXTPROC) load("glProgramUniform3iEXT", userptr);
    glProgramUniform3iv = (PFNGLPROGRAMUNIFORM3IVPROC) load("glProgramUniform3iv", userptr);
    glProgramUniform3ivEXT = (PFNGLPROGRAMUNIFORM3IVEXTPROC) load("glProgramUniform3ivEXT", userptr);
    glProgramUniform3ui = (PFNGLPROGRAMUNIFORM3UIPROC) load("glProgramUniform3ui", userptr);
    glProgramUniform3uiEXT = (PFNGLPROGRAMUNIFORM3UIEXTPROC) load("glProgramUniform3uiEXT", userptr);
    glProgramUniform3uiv = (PFNGLPROGRAMUNIFORM3UIVPROC) load("glProgramUniform3uiv", userptr);
    glProgramUniform3uivEXT = (PFNGLPROGRAMUNIFORM3UIVEXTPROC) load("glProgramUniform3uivEXT", userptr);
    glProgramUniform4f = (PFNGLPROGRAMUNIFORM4FPROC) load("glProgramUniform4f", userptr);
    glProgramUniform4fEXT = (PFNGLPROGRAMUNIFORM4FEXTPROC) load("glProgramUniform4fEXT", userptr);
    glProgramUniform4fv = (PFNGLPROGRAMUNIFORM4FVPROC) load("glProgramUniform4fv", userptr);
    glProgramUniform4fvEXT = (PFNGLPROGRAMUNIFORM4FVEXTPROC) load("glProgramUniform4fvEXT", userptr);
    glProgramUniform4i = (PFNGLPROGRAMUNIFORM4IPROC) load("glProgramUniform4i", userptr);
    glProgramUniform4iEXT = (PFNGLPROGRAMUNIFORM4IEXTPROC) load("glProgramUniform4iEXT", userptr);
    glProgramUniform4iv = (PFNGLPROGRAMUNIFORM4IVPROC) load("glProgramUniform4iv", userptr);
    glProgramUniform4ivEXT = (PFNGLPROGRAMUNIFORM4IVEXTPROC) load("glProgramUniform4ivEXT", userptr);
    glProgramUniform4ui = (PFNGLPROGRAMUNIFORM4UIPROC) load("glProgramUniform4ui", userptr);
    glProgramUniform4uiEXT = (PFNGLPROGRAMUNIFORM4UIEXTPROC) load("glProgramUniform4uiEXT", userptr);
    glProgramUniform4uiv = (PFNGLPROGRAMUNIFORM4UIVPROC) load("glProgramUniform4uiv", userptr);
    glProgramUniform4uivEXT = (PFNGLPROGRAMUNIFORM4UIVEXTPROC) load("glProgramUniform4uivEXT", userptr);
    glProgramUniformMatrix2fv = (PFNGLPROGRAMUNIFORMMATRIX2FVPROC) load("glProgramUniformMatrix2fv", userptr);
    glProgramUniformMatrix2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC) load("glProgramUniformMatrix2fvEXT", userptr);
    glProgramUniformMatrix2x3fv = (PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC) load("glProgramUniformMatrix2x3fv", userptr);
    glProgramUniformMatrix2x3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC) load("glProgramUniformMatrix2x3fvEXT", userptr);
    glProgramUniformMatrix2x4fv = (PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC) load("glProgramUniformMatrix2x4fv", userptr);
    glProgramUniformMatrix2x4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC) load("glProgramUniformMatrix2x4fvEXT", userptr);
    glProgramUniformMatrix3fv = (PFNGLPROGRAMUNIFORMMATRIX3FVPROC) load("glProgramUniformMatrix3fv", userptr);
    glProgramUniformMatrix3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC) load("glProgramUniformMatrix3fvEXT", userptr);
    glProgramUniformMatrix3x2fv = (PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC) load("glProgramUniformMatrix3x2fv", userptr);
    glProgramUniformMatrix3x2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC) load("glProgramUniformMatrix3x2fvEXT", userptr);
    glProgramUniformMatrix3x4fv = (PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC) load("glProgramUniformMatrix3x4fv", userptr);
    glProgramUniformMatrix3x4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC) load("glProgramUniformMatrix3x4fvEXT", userptr);
    glProgramUniformMatrix4fv = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC) load("glProgramUniformMatrix4fv", userptr);
    glProgramUniformMatrix4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC) load("glProgramUniformMatrix4fvEXT", userptr);
    glProgramUniformMatrix4x2fv = (PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC) load("glProgramUniformMatrix4x2fv", userptr);
    glProgramUniformMatrix4x2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC) load("glProgramUniformMatrix4x2fvEXT", userptr);
    glProgramUniformMatrix4x3fv = (PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC) load("glProgramUniformMatrix4x3fv", userptr);
    glProgramUniformMatrix4x3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC) load("glProgramUniformMatrix4x3fvEXT", userptr);
    glUseProgramStagesEXT = (PFNGLUSEPROGRAMSTAGESEXTPROC) load("glUseProgramStagesEXT", userptr);
    glValidateProgramPipelineEXT = (PFNGLVALIDATEPROGRAMPIPELINEEXTPROC) load("glValidateProgramPipelineEXT", userptr);
}
static void glad_gl_load_GL_EXT_shader_framebuffer_fetch_non_coherent( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_shader_framebuffer_fetch_non_coherent) return;
    glFramebufferFetchBarrierEXT = (PFNGLFRAMEBUFFERFETCHBARRIEREXTPROC) load("glFramebufferFetchBarrierEXT", userptr);
}
static void glad_gl_load_GL_EXT_shader_pixel_local_storage2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_shader_pixel_local_storage2) return;
    glClearPixelLocalStorageuiEXT = (PFNGLCLEARPIXELLOCALSTORAGEUIEXTPROC) load("glClearPixelLocalStorageuiEXT", userptr);
    glFramebufferPixelLocalStorageSizeEXT = (PFNGLFRAMEBUFFERPIXELLOCALSTORAGESIZEEXTPROC) load("glFramebufferPixelLocalStorageSizeEXT", userptr);
    glGetFramebufferPixelLocalStorageSizeEXT = (PFNGLGETFRAMEBUFFERPIXELLOCALSTORAGESIZEEXTPROC) load("glGetFramebufferPixelLocalStorageSizeEXT", userptr);
}
static void glad_gl_load_GL_EXT_sparse_texture( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_sparse_texture) return;
    glTexPageCommitmentARB = (PFNGLTEXPAGECOMMITMENTARBPROC) load("glTexPageCommitmentARB", userptr);
    glTexPageCommitmentEXT = (PFNGLTEXPAGECOMMITMENTEXTPROC) load("glTexPageCommitmentEXT", userptr);
}
static void glad_gl_load_GL_EXT_tessellation_shader( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_tessellation_shader) return;
    glPatchParameteri = (PFNGLPATCHPARAMETERIPROC) load("glPatchParameteri", userptr);
    glPatchParameteriEXT = (PFNGLPATCHPARAMETERIEXTPROC) load("glPatchParameteriEXT", userptr);
}
static void glad_gl_load_GL_EXT_texture_border_clamp( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_texture_border_clamp) return;
    glGetSamplerParameterIiv = (PFNGLGETSAMPLERPARAMETERIIVPROC) load("glGetSamplerParameterIiv", userptr);
    glGetSamplerParameterIivEXT = (PFNGLGETSAMPLERPARAMETERIIVEXTPROC) load("glGetSamplerParameterIivEXT", userptr);
    glGetSamplerParameterIuiv = (PFNGLGETSAMPLERPARAMETERIUIVPROC) load("glGetSamplerParameterIuiv", userptr);
    glGetSamplerParameterIuivEXT = (PFNGLGETSAMPLERPARAMETERIUIVEXTPROC) load("glGetSamplerParameterIuivEXT", userptr);
    glGetTexParameterIiv = (PFNGLGETTEXPARAMETERIIVPROC) load("glGetTexParameterIiv", userptr);
    glGetTexParameterIivEXT = (PFNGLGETTEXPARAMETERIIVEXTPROC) load("glGetTexParameterIivEXT", userptr);
    glGetTexParameterIuiv = (PFNGLGETTEXPARAMETERIUIVPROC) load("glGetTexParameterIuiv", userptr);
    glGetTexParameterIuivEXT = (PFNGLGETTEXPARAMETERIUIVEXTPROC) load("glGetTexParameterIuivEXT", userptr);
    glSamplerParameterIiv = (PFNGLSAMPLERPARAMETERIIVPROC) load("glSamplerParameterIiv", userptr);
    glSamplerParameterIivEXT = (PFNGLSAMPLERPARAMETERIIVEXTPROC) load("glSamplerParameterIivEXT", userptr);
    glSamplerParameterIuiv = (PFNGLSAMPLERPARAMETERIUIVPROC) load("glSamplerParameterIuiv", userptr);
    glSamplerParameterIuivEXT = (PFNGLSAMPLERPARAMETERIUIVEXTPROC) load("glSamplerParameterIuivEXT", userptr);
    glTexParameterIiv = (PFNGLTEXPARAMETERIIVPROC) load("glTexParameterIiv", userptr);
    glTexParameterIivEXT = (PFNGLTEXPARAMETERIIVEXTPROC) load("glTexParameterIivEXT", userptr);
    glTexParameterIuiv = (PFNGLTEXPARAMETERIUIVPROC) load("glTexParameterIuiv", userptr);
    glTexParameterIuivEXT = (PFNGLTEXPARAMETERIUIVEXTPROC) load("glTexParameterIuivEXT", userptr);
}
static void glad_gl_load_GL_EXT_texture_buffer( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_texture_buffer) return;
    glTexBuffer = (PFNGLTEXBUFFERPROC) load("glTexBuffer", userptr);
    glTexBufferEXT = (PFNGLTEXBUFFEREXTPROC) load("glTexBufferEXT", userptr);
    glTexBufferRange = (PFNGLTEXBUFFERRANGEPROC) load("glTexBufferRange", userptr);
    glTexBufferRangeEXT = (PFNGLTEXBUFFERRANGEEXTPROC) load("glTexBufferRangeEXT", userptr);
}
static void glad_gl_load_GL_EXT_texture_storage( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_texture_storage) return;
    glTexStorage1D = (PFNGLTEXSTORAGE1DPROC) load("glTexStorage1D", userptr);
    glTexStorage1DEXT = (PFNGLTEXSTORAGE1DEXTPROC) load("glTexStorage1DEXT", userptr);
    glTexStorage2D = (PFNGLTEXSTORAGE2DPROC) load("glTexStorage2D", userptr);
    glTexStorage2DEXT = (PFNGLTEXSTORAGE2DEXTPROC) load("glTexStorage2DEXT", userptr);
    glTexStorage3D = (PFNGLTEXSTORAGE3DPROC) load("glTexStorage3D", userptr);
    glTexStorage3DEXT = (PFNGLTEXSTORAGE3DEXTPROC) load("glTexStorage3DEXT", userptr);
    glTextureStorage1DEXT = (PFNGLTEXTURESTORAGE1DEXTPROC) load("glTextureStorage1DEXT", userptr);
    glTextureStorage2DEXT = (PFNGLTEXTURESTORAGE2DEXTPROC) load("glTextureStorage2DEXT", userptr);
    glTextureStorage3DEXT = (PFNGLTEXTURESTORAGE3DEXTPROC) load("glTextureStorage3DEXT", userptr);
}
static void glad_gl_load_GL_EXT_texture_view( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_texture_view) return;
    glTextureView = (PFNGLTEXTUREVIEWPROC) load("glTextureView", userptr);
    glTextureViewEXT = (PFNGLTEXTUREVIEWEXTPROC) load("glTextureViewEXT", userptr);
}
static void glad_gl_load_GL_EXT_win32_keyed_mutex( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_win32_keyed_mutex) return;
    glAcquireKeyedMutexWin32EXT = (PFNGLACQUIREKEYEDMUTEXWIN32EXTPROC) load("glAcquireKeyedMutexWin32EXT", userptr);
    glReleaseKeyedMutexWin32EXT = (PFNGLRELEASEKEYEDMUTEXWIN32EXTPROC) load("glReleaseKeyedMutexWin32EXT", userptr);
}
static void glad_gl_load_GL_EXT_window_rectangles( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_window_rectangles) return;
    glWindowRectanglesEXT = (PFNGLWINDOWRECTANGLESEXTPROC) load("glWindowRectanglesEXT", userptr);
}
static void glad_gl_load_GL_IMG_bindless_texture( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_IMG_bindless_texture) return;
    glGetTextureHandleARB = (PFNGLGETTEXTUREHANDLEARBPROC) load("glGetTextureHandleARB", userptr);
    glGetTextureHandleIMG = (PFNGLGETTEXTUREHANDLEIMGPROC) load("glGetTextureHandleIMG", userptr);
    glGetTextureSamplerHandleARB = (PFNGLGETTEXTURESAMPLERHANDLEARBPROC) load("glGetTextureSamplerHandleARB", userptr);
    glGetTextureSamplerHandleIMG = (PFNGLGETTEXTURESAMPLERHANDLEIMGPROC) load("glGetTextureSamplerHandleIMG", userptr);
    glProgramUniformHandleui64ARB = (PFNGLPROGRAMUNIFORMHANDLEUI64ARBPROC) load("glProgramUniformHandleui64ARB", userptr);
    glProgramUniformHandleui64IMG = (PFNGLPROGRAMUNIFORMHANDLEUI64IMGPROC) load("glProgramUniformHandleui64IMG", userptr);
    glProgramUniformHandleui64vARB = (PFNGLPROGRAMUNIFORMHANDLEUI64VARBPROC) load("glProgramUniformHandleui64vARB", userptr);
    glProgramUniformHandleui64vIMG = (PFNGLPROGRAMUNIFORMHANDLEUI64VIMGPROC) load("glProgramUniformHandleui64vIMG", userptr);
    glUniformHandleui64ARB = (PFNGLUNIFORMHANDLEUI64ARBPROC) load("glUniformHandleui64ARB", userptr);
    glUniformHandleui64IMG = (PFNGLUNIFORMHANDLEUI64IMGPROC) load("glUniformHandleui64IMG", userptr);
    glUniformHandleui64vARB = (PFNGLUNIFORMHANDLEUI64VARBPROC) load("glUniformHandleui64vARB", userptr);
    glUniformHandleui64vIMG = (PFNGLUNIFORMHANDLEUI64VIMGPROC) load("glUniformHandleui64vIMG", userptr);
}
static void glad_gl_load_GL_IMG_framebuffer_downsample( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_IMG_framebuffer_downsample) return;
    glFramebufferTexture2DDownsampleIMG = (PFNGLFRAMEBUFFERTEXTURE2DDOWNSAMPLEIMGPROC) load("glFramebufferTexture2DDownsampleIMG", userptr);
    glFramebufferTextureLayerDownsampleIMG = (PFNGLFRAMEBUFFERTEXTURELAYERDOWNSAMPLEIMGPROC) load("glFramebufferTextureLayerDownsampleIMG", userptr);
}
static void glad_gl_load_GL_IMG_multisampled_render_to_texture( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_IMG_multisampled_render_to_texture) return;
    glFramebufferTexture2DMultisampleIMG = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC) load("glFramebufferTexture2DMultisampleIMG", userptr);
    glRenderbufferStorageMultisampleIMG = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMGPROC) load("glRenderbufferStorageMultisampleIMG", userptr);
}
static void glad_gl_load_GL_INTEL_framebuffer_CMAA( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_INTEL_framebuffer_CMAA) return;
    glApplyFramebufferAttachmentCMAAINTEL = (PFNGLAPPLYFRAMEBUFFERATTACHMENTCMAAINTELPROC) load("glApplyFramebufferAttachmentCMAAINTEL", userptr);
}
static void glad_gl_load_GL_INTEL_performance_query( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_INTEL_performance_query) return;
    glBeginPerfQueryINTEL = (PFNGLBEGINPERFQUERYINTELPROC) load("glBeginPerfQueryINTEL", userptr);
    glCreatePerfQueryINTEL = (PFNGLCREATEPERFQUERYINTELPROC) load("glCreatePerfQueryINTEL", userptr);
    glDeletePerfQueryINTEL = (PFNGLDELETEPERFQUERYINTELPROC) load("glDeletePerfQueryINTEL", userptr);
    glEndPerfQueryINTEL = (PFNGLENDPERFQUERYINTELPROC) load("glEndPerfQueryINTEL", userptr);
    glGetFirstPerfQueryIdINTEL = (PFNGLGETFIRSTPERFQUERYIDINTELPROC) load("glGetFirstPerfQueryIdINTEL", userptr);
    glGetNextPerfQueryIdINTEL = (PFNGLGETNEXTPERFQUERYIDINTELPROC) load("glGetNextPerfQueryIdINTEL", userptr);
    glGetPerfCounterInfoINTEL = (PFNGLGETPERFCOUNTERINFOINTELPROC) load("glGetPerfCounterInfoINTEL", userptr);
    glGetPerfQueryDataINTEL = (PFNGLGETPERFQUERYDATAINTELPROC) load("glGetPerfQueryDataINTEL", userptr);
    glGetPerfQueryIdByNameINTEL = (PFNGLGETPERFQUERYIDBYNAMEINTELPROC) load("glGetPerfQueryIdByNameINTEL", userptr);
    glGetPerfQueryInfoINTEL = (PFNGLGETPERFQUERYINFOINTELPROC) load("glGetPerfQueryInfoINTEL", userptr);
}
static void glad_gl_load_GL_KHR_blend_equation_advanced( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_KHR_blend_equation_advanced) return;
    glBlendBarrier = (PFNGLBLENDBARRIERPROC) load("glBlendBarrier", userptr);
    glBlendBarrierKHR = (PFNGLBLENDBARRIERKHRPROC) load("glBlendBarrierKHR", userptr);
}
static void glad_gl_load_GL_KHR_debug( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_KHR_debug) return;
    glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC) load("glDebugMessageCallback", userptr);
    glDebugMessageCallbackKHR = (PFNGLDEBUGMESSAGECALLBACKKHRPROC) load("glDebugMessageCallbackKHR", userptr);
    glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC) load("glDebugMessageControl", userptr);
    glDebugMessageControlKHR = (PFNGLDEBUGMESSAGECONTROLKHRPROC) load("glDebugMessageControlKHR", userptr);
    glDebugMessageInsert = (PFNGLDEBUGMESSAGEINSERTPROC) load("glDebugMessageInsert", userptr);
    glDebugMessageInsertKHR = (PFNGLDEBUGMESSAGEINSERTKHRPROC) load("glDebugMessageInsertKHR", userptr);
    glGetDebugMessageLog = (PFNGLGETDEBUGMESSAGELOGPROC) load("glGetDebugMessageLog", userptr);
    glGetDebugMessageLogKHR = (PFNGLGETDEBUGMESSAGELOGKHRPROC) load("glGetDebugMessageLogKHR", userptr);
    glGetObjectLabel = (PFNGLGETOBJECTLABELPROC) load("glGetObjectLabel", userptr);
    glGetObjectLabelKHR = (PFNGLGETOBJECTLABELKHRPROC) load("glGetObjectLabelKHR", userptr);
    glGetObjectPtrLabel = (PFNGLGETOBJECTPTRLABELPROC) load("glGetObjectPtrLabel", userptr);
    glGetObjectPtrLabelKHR = (PFNGLGETOBJECTPTRLABELKHRPROC) load("glGetObjectPtrLabelKHR", userptr);
    glGetPointerv = (PFNGLGETPOINTERVPROC) load("glGetPointerv", userptr);
    glGetPointervKHR = (PFNGLGETPOINTERVKHRPROC) load("glGetPointervKHR", userptr);
    glObjectLabel = (PFNGLOBJECTLABELPROC) load("glObjectLabel", userptr);
    glObjectLabelKHR = (PFNGLOBJECTLABELKHRPROC) load("glObjectLabelKHR", userptr);
    glObjectPtrLabel = (PFNGLOBJECTPTRLABELPROC) load("glObjectPtrLabel", userptr);
    glObjectPtrLabelKHR = (PFNGLOBJECTPTRLABELKHRPROC) load("glObjectPtrLabelKHR", userptr);
    glPopDebugGroup = (PFNGLPOPDEBUGGROUPPROC) load("glPopDebugGroup", userptr);
    glPopDebugGroupKHR = (PFNGLPOPDEBUGGROUPKHRPROC) load("glPopDebugGroupKHR", userptr);
    glPushDebugGroup = (PFNGLPUSHDEBUGGROUPPROC) load("glPushDebugGroup", userptr);
    glPushDebugGroupKHR = (PFNGLPUSHDEBUGGROUPKHRPROC) load("glPushDebugGroupKHR", userptr);
}
static void glad_gl_load_GL_KHR_parallel_shader_compile( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_KHR_parallel_shader_compile) return;
    glMaxShaderCompilerThreadsKHR = (PFNGLMAXSHADERCOMPILERTHREADSKHRPROC) load("glMaxShaderCompilerThreadsKHR", userptr);
}
static void glad_gl_load_GL_KHR_robustness( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_KHR_robustness) return;
    glGetGraphicsResetStatus = (PFNGLGETGRAPHICSRESETSTATUSPROC) load("glGetGraphicsResetStatus", userptr);
    glGetGraphicsResetStatusKHR = (PFNGLGETGRAPHICSRESETSTATUSKHRPROC) load("glGetGraphicsResetStatusKHR", userptr);
    glGetnUniformfv = (PFNGLGETNUNIFORMFVPROC) load("glGetnUniformfv", userptr);
    glGetnUniformfvKHR = (PFNGLGETNUNIFORMFVKHRPROC) load("glGetnUniformfvKHR", userptr);
    glGetnUniformiv = (PFNGLGETNUNIFORMIVPROC) load("glGetnUniformiv", userptr);
    glGetnUniformivKHR = (PFNGLGETNUNIFORMIVKHRPROC) load("glGetnUniformivKHR", userptr);
    glGetnUniformuiv = (PFNGLGETNUNIFORMUIVPROC) load("glGetnUniformuiv", userptr);
    glGetnUniformuivKHR = (PFNGLGETNUNIFORMUIVKHRPROC) load("glGetnUniformuivKHR", userptr);
    glReadnPixels = (PFNGLREADNPIXELSPROC) load("glReadnPixels", userptr);
    glReadnPixelsKHR = (PFNGLREADNPIXELSKHRPROC) load("glReadnPixelsKHR", userptr);
}
static void glad_gl_load_GL_NV_bindless_texture( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_bindless_texture) return;
    glGetImageHandleNV = (PFNGLGETIMAGEHANDLENVPROC) load("glGetImageHandleNV", userptr);
    glGetTextureHandleNV = (PFNGLGETTEXTUREHANDLENVPROC) load("glGetTextureHandleNV", userptr);
    glGetTextureSamplerHandleNV = (PFNGLGETTEXTURESAMPLERHANDLENVPROC) load("glGetTextureSamplerHandleNV", userptr);
    glIsImageHandleResidentNV = (PFNGLISIMAGEHANDLERESIDENTNVPROC) load("glIsImageHandleResidentNV", userptr);
    glIsTextureHandleResidentNV = (PFNGLISTEXTUREHANDLERESIDENTNVPROC) load("glIsTextureHandleResidentNV", userptr);
    glMakeImageHandleNonResidentNV = (PFNGLMAKEIMAGEHANDLENONRESIDENTNVPROC) load("glMakeImageHandleNonResidentNV", userptr);
    glMakeImageHandleResidentNV = (PFNGLMAKEIMAGEHANDLERESIDENTNVPROC) load("glMakeImageHandleResidentNV", userptr);
    glMakeTextureHandleNonResidentNV = (PFNGLMAKETEXTUREHANDLENONRESIDENTNVPROC) load("glMakeTextureHandleNonResidentNV", userptr);
    glMakeTextureHandleResidentNV = (PFNGLMAKETEXTUREHANDLERESIDENTNVPROC) load("glMakeTextureHandleResidentNV", userptr);
    glProgramUniformHandleui64NV = (PFNGLPROGRAMUNIFORMHANDLEUI64NVPROC) load("glProgramUniformHandleui64NV", userptr);
    glProgramUniformHandleui64vNV = (PFNGLPROGRAMUNIFORMHANDLEUI64VNVPROC) load("glProgramUniformHandleui64vNV", userptr);
    glUniformHandleui64NV = (PFNGLUNIFORMHANDLEUI64NVPROC) load("glUniformHandleui64NV", userptr);
    glUniformHandleui64vNV = (PFNGLUNIFORMHANDLEUI64VNVPROC) load("glUniformHandleui64vNV", userptr);
}
static void glad_gl_load_GL_NV_blend_equation_advanced( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_blend_equation_advanced) return;
    glBlendBarrier = (PFNGLBLENDBARRIERPROC) load("glBlendBarrier", userptr);
    glBlendBarrierNV = (PFNGLBLENDBARRIERNVPROC) load("glBlendBarrierNV", userptr);
    glBlendParameteriNV = (PFNGLBLENDPARAMETERINVPROC) load("glBlendParameteriNV", userptr);
}
static void glad_gl_load_GL_NV_clip_space_w_scaling( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_clip_space_w_scaling) return;
    glViewportPositionWScaleNV = (PFNGLVIEWPORTPOSITIONWSCALENVPROC) load("glViewportPositionWScaleNV", userptr);
}
static void glad_gl_load_GL_NV_conditional_render( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_conditional_render) return;
    glBeginConditionalRender = (PFNGLBEGINCONDITIONALRENDERPROC) load("glBeginConditionalRender", userptr);
    glBeginConditionalRenderNV = (PFNGLBEGINCONDITIONALRENDERNVPROC) load("glBeginConditionalRenderNV", userptr);
    glEndConditionalRender = (PFNGLENDCONDITIONALRENDERPROC) load("glEndConditionalRender", userptr);
    glEndConditionalRenderNV = (PFNGLENDCONDITIONALRENDERNVPROC) load("glEndConditionalRenderNV", userptr);
}
static void glad_gl_load_GL_NV_conservative_raster( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_conservative_raster) return;
    glSubpixelPrecisionBiasNV = (PFNGLSUBPIXELPRECISIONBIASNVPROC) load("glSubpixelPrecisionBiasNV", userptr);
}
static void glad_gl_load_GL_NV_conservative_raster_pre_snap_triangles( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_conservative_raster_pre_snap_triangles) return;
    glConservativeRasterParameteriNV = (PFNGLCONSERVATIVERASTERPARAMETERINVPROC) load("glConservativeRasterParameteriNV", userptr);
}
static void glad_gl_load_GL_NV_copy_buffer( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_copy_buffer) return;
    glCopyBufferSubData = (PFNGLCOPYBUFFERSUBDATAPROC) load("glCopyBufferSubData", userptr);
    glCopyBufferSubDataNV = (PFNGLCOPYBUFFERSUBDATANVPROC) load("glCopyBufferSubDataNV", userptr);
}
static void glad_gl_load_GL_NV_coverage_sample( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_coverage_sample) return;
    glCoverageMaskNV = (PFNGLCOVERAGEMASKNVPROC) load("glCoverageMaskNV", userptr);
    glCoverageOperationNV = (PFNGLCOVERAGEOPERATIONNVPROC) load("glCoverageOperationNV", userptr);
}
static void glad_gl_load_GL_NV_draw_buffers( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_draw_buffers) return;
    glDrawBuffersNV = (PFNGLDRAWBUFFERSNVPROC) load("glDrawBuffersNV", userptr);
}
static void glad_gl_load_GL_NV_draw_instanced( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_draw_instanced) return;
    glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC) load("glDrawArraysInstanced", userptr);
    glDrawArraysInstancedNV = (PFNGLDRAWARRAYSINSTANCEDNVPROC) load("glDrawArraysInstancedNV", userptr);
    glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC) load("glDrawElementsInstanced", userptr);
    glDrawElementsInstancedNV = (PFNGLDRAWELEMENTSINSTANCEDNVPROC) load("glDrawElementsInstancedNV", userptr);
}
static void glad_gl_load_GL_NV_draw_vulkan_image( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_draw_vulkan_image) return;
    glDrawVkImageNV = (PFNGLDRAWVKIMAGENVPROC) load("glDrawVkImageNV", userptr);
    glGetVkProcAddrNV = (PFNGLGETVKPROCADDRNVPROC) load("glGetVkProcAddrNV", userptr);
    glSignalVkFenceNV = (PFNGLSIGNALVKFENCENVPROC) load("glSignalVkFenceNV", userptr);
    glSignalVkSemaphoreNV = (PFNGLSIGNALVKSEMAPHORENVPROC) load("glSignalVkSemaphoreNV", userptr);
    glWaitVkSemaphoreNV = (PFNGLWAITVKSEMAPHORENVPROC) load("glWaitVkSemaphoreNV", userptr);
}
static void glad_gl_load_GL_NV_fence( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_fence) return;
    glDeleteFencesNV = (PFNGLDELETEFENCESNVPROC) load("glDeleteFencesNV", userptr);
    glFinishFenceNV = (PFNGLFINISHFENCENVPROC) load("glFinishFenceNV", userptr);
    glGenFencesNV = (PFNGLGENFENCESNVPROC) load("glGenFencesNV", userptr);
    glGetFenceivNV = (PFNGLGETFENCEIVNVPROC) load("glGetFenceivNV", userptr);
    glIsFenceNV = (PFNGLISFENCENVPROC) load("glIsFenceNV", userptr);
    glSetFenceNV = (PFNGLSETFENCENVPROC) load("glSetFenceNV", userptr);
    glTestFenceNV = (PFNGLTESTFENCENVPROC) load("glTestFenceNV", userptr);
}
static void glad_gl_load_GL_NV_fragment_coverage_to_color( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_fragment_coverage_to_color) return;
    glFragmentCoverageColorNV = (PFNGLFRAGMENTCOVERAGECOLORNVPROC) load("glFragmentCoverageColorNV", userptr);
}
static void glad_gl_load_GL_NV_framebuffer_blit( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_framebuffer_blit) return;
    glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC) load("glBlitFramebuffer", userptr);
    glBlitFramebufferNV = (PFNGLBLITFRAMEBUFFERNVPROC) load("glBlitFramebufferNV", userptr);
}
static void glad_gl_load_GL_NV_framebuffer_mixed_samples( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_framebuffer_mixed_samples) return;
    glCoverageModulationNV = (PFNGLCOVERAGEMODULATIONNVPROC) load("glCoverageModulationNV", userptr);
    glCoverageModulationTableNV = (PFNGLCOVERAGEMODULATIONTABLENVPROC) load("glCoverageModulationTableNV", userptr);
    glGetCoverageModulationTableNV = (PFNGLGETCOVERAGEMODULATIONTABLENVPROC) load("glGetCoverageModulationTableNV", userptr);
    glRasterSamplesEXT = (PFNGLRASTERSAMPLESEXTPROC) load("glRasterSamplesEXT", userptr);
}
static void glad_gl_load_GL_NV_framebuffer_multisample( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_framebuffer_multisample) return;
    glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC) load("glRenderbufferStorageMultisample", userptr);
    glRenderbufferStorageMultisampleNV = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLENVPROC) load("glRenderbufferStorageMultisampleNV", userptr);
}
static void glad_gl_load_GL_NV_gpu_shader5( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_gpu_shader5) return;
    glGetUniformi64vNV = (PFNGLGETUNIFORMI64VNVPROC) load("glGetUniformi64vNV", userptr);
    glProgramUniform1i64NV = (PFNGLPROGRAMUNIFORM1I64NVPROC) load("glProgramUniform1i64NV", userptr);
    glProgramUniform1i64vNV = (PFNGLPROGRAMUNIFORM1I64VNVPROC) load("glProgramUniform1i64vNV", userptr);
    glProgramUniform1ui64NV = (PFNGLPROGRAMUNIFORM1UI64NVPROC) load("glProgramUniform1ui64NV", userptr);
    glProgramUniform1ui64vNV = (PFNGLPROGRAMUNIFORM1UI64VNVPROC) load("glProgramUniform1ui64vNV", userptr);
    glProgramUniform2i64NV = (PFNGLPROGRAMUNIFORM2I64NVPROC) load("glProgramUniform2i64NV", userptr);
    glProgramUniform2i64vNV = (PFNGLPROGRAMUNIFORM2I64VNVPROC) load("glProgramUniform2i64vNV", userptr);
    glProgramUniform2ui64NV = (PFNGLPROGRAMUNIFORM2UI64NVPROC) load("glProgramUniform2ui64NV", userptr);
    glProgramUniform2ui64vNV = (PFNGLPROGRAMUNIFORM2UI64VNVPROC) load("glProgramUniform2ui64vNV", userptr);
    glProgramUniform3i64NV = (PFNGLPROGRAMUNIFORM3I64NVPROC) load("glProgramUniform3i64NV", userptr);
    glProgramUniform3i64vNV = (PFNGLPROGRAMUNIFORM3I64VNVPROC) load("glProgramUniform3i64vNV", userptr);
    glProgramUniform3ui64NV = (PFNGLPROGRAMUNIFORM3UI64NVPROC) load("glProgramUniform3ui64NV", userptr);
    glProgramUniform3ui64vNV = (PFNGLPROGRAMUNIFORM3UI64VNVPROC) load("glProgramUniform3ui64vNV", userptr);
    glProgramUniform4i64NV = (PFNGLPROGRAMUNIFORM4I64NVPROC) load("glProgramUniform4i64NV", userptr);
    glProgramUniform4i64vNV = (PFNGLPROGRAMUNIFORM4I64VNVPROC) load("glProgramUniform4i64vNV", userptr);
    glProgramUniform4ui64NV = (PFNGLPROGRAMUNIFORM4UI64NVPROC) load("glProgramUniform4ui64NV", userptr);
    glProgramUniform4ui64vNV = (PFNGLPROGRAMUNIFORM4UI64VNVPROC) load("glProgramUniform4ui64vNV", userptr);
    glUniform1i64NV = (PFNGLUNIFORM1I64NVPROC) load("glUniform1i64NV", userptr);
    glUniform1i64vNV = (PFNGLUNIFORM1I64VNVPROC) load("glUniform1i64vNV", userptr);
    glUniform1ui64NV = (PFNGLUNIFORM1UI64NVPROC) load("glUniform1ui64NV", userptr);
    glUniform1ui64vNV = (PFNGLUNIFORM1UI64VNVPROC) load("glUniform1ui64vNV", userptr);
    glUniform2i64NV = (PFNGLUNIFORM2I64NVPROC) load("glUniform2i64NV", userptr);
    glUniform2i64vNV = (PFNGLUNIFORM2I64VNVPROC) load("glUniform2i64vNV", userptr);
    glUniform2ui64NV = (PFNGLUNIFORM2UI64NVPROC) load("glUniform2ui64NV", userptr);
    glUniform2ui64vNV = (PFNGLUNIFORM2UI64VNVPROC) load("glUniform2ui64vNV", userptr);
    glUniform3i64NV = (PFNGLUNIFORM3I64NVPROC) load("glUniform3i64NV", userptr);
    glUniform3i64vNV = (PFNGLUNIFORM3I64VNVPROC) load("glUniform3i64vNV", userptr);
    glUniform3ui64NV = (PFNGLUNIFORM3UI64NVPROC) load("glUniform3ui64NV", userptr);
    glUniform3ui64vNV = (PFNGLUNIFORM3UI64VNVPROC) load("glUniform3ui64vNV", userptr);
    glUniform4i64NV = (PFNGLUNIFORM4I64NVPROC) load("glUniform4i64NV", userptr);
    glUniform4i64vNV = (PFNGLUNIFORM4I64VNVPROC) load("glUniform4i64vNV", userptr);
    glUniform4ui64NV = (PFNGLUNIFORM4UI64NVPROC) load("glUniform4ui64NV", userptr);
    glUniform4ui64vNV = (PFNGLUNIFORM4UI64VNVPROC) load("glUniform4ui64vNV", userptr);
}
static void glad_gl_load_GL_NV_instanced_arrays( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_instanced_arrays) return;
    glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC) load("glVertexAttribDivisor", userptr);
    glVertexAttribDivisorNV = (PFNGLVERTEXATTRIBDIVISORNVPROC) load("glVertexAttribDivisorNV", userptr);
}
static void glad_gl_load_GL_NV_internalformat_sample_query( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_internalformat_sample_query) return;
    glGetInternalformatSampleivNV = (PFNGLGETINTERNALFORMATSAMPLEIVNVPROC) load("glGetInternalformatSampleivNV", userptr);
}
static void glad_gl_load_GL_NV_memory_attachment( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_memory_attachment) return;
    glBufferAttachMemoryNV = (PFNGLBUFFERATTACHMEMORYNVPROC) load("glBufferAttachMemoryNV", userptr);
    glGetMemoryObjectDetachedResourcesuivNV = (PFNGLGETMEMORYOBJECTDETACHEDRESOURCESUIVNVPROC) load("glGetMemoryObjectDetachedResourcesuivNV", userptr);
    glNamedBufferAttachMemoryNV = (PFNGLNAMEDBUFFERATTACHMEMORYNVPROC) load("glNamedBufferAttachMemoryNV", userptr);
    glResetMemoryObjectParameterNV = (PFNGLRESETMEMORYOBJECTPARAMETERNVPROC) load("glResetMemoryObjectParameterNV", userptr);
    glTexAttachMemoryNV = (PFNGLTEXATTACHMEMORYNVPROC) load("glTexAttachMemoryNV", userptr);
    glTextureAttachMemoryNV = (PFNGLTEXTUREATTACHMEMORYNVPROC) load("glTextureAttachMemoryNV", userptr);
}
static void glad_gl_load_GL_NV_non_square_matrices( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_non_square_matrices) return;
    glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC) load("glUniformMatrix2x3fv", userptr);
    glUniformMatrix2x3fvNV = (PFNGLUNIFORMMATRIX2X3FVNVPROC) load("glUniformMatrix2x3fvNV", userptr);
    glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC) load("glUniformMatrix2x4fv", userptr);
    glUniformMatrix2x4fvNV = (PFNGLUNIFORMMATRIX2X4FVNVPROC) load("glUniformMatrix2x4fvNV", userptr);
    glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC) load("glUniformMatrix3x2fv", userptr);
    glUniformMatrix3x2fvNV = (PFNGLUNIFORMMATRIX3X2FVNVPROC) load("glUniformMatrix3x2fvNV", userptr);
    glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC) load("glUniformMatrix3x4fv", userptr);
    glUniformMatrix3x4fvNV = (PFNGLUNIFORMMATRIX3X4FVNVPROC) load("glUniformMatrix3x4fvNV", userptr);
    glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC) load("glUniformMatrix4x2fv", userptr);
    glUniformMatrix4x2fvNV = (PFNGLUNIFORMMATRIX4X2FVNVPROC) load("glUniformMatrix4x2fvNV", userptr);
    glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC) load("glUniformMatrix4x3fv", userptr);
    glUniformMatrix4x3fvNV = (PFNGLUNIFORMMATRIX4X3FVNVPROC) load("glUniformMatrix4x3fvNV", userptr);
}
static void glad_gl_load_GL_NV_path_rendering( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_path_rendering) return;
    glCopyPathNV = (PFNGLCOPYPATHNVPROC) load("glCopyPathNV", userptr);
    glCoverFillPathInstancedNV = (PFNGLCOVERFILLPATHINSTANCEDNVPROC) load("glCoverFillPathInstancedNV", userptr);
    glCoverFillPathNV = (PFNGLCOVERFILLPATHNVPROC) load("glCoverFillPathNV", userptr);
    glCoverStrokePathInstancedNV = (PFNGLCOVERSTROKEPATHINSTANCEDNVPROC) load("glCoverStrokePathInstancedNV", userptr);
    glCoverStrokePathNV = (PFNGLCOVERSTROKEPATHNVPROC) load("glCoverStrokePathNV", userptr);
    glDeletePathsNV = (PFNGLDELETEPATHSNVPROC) load("glDeletePathsNV", userptr);
    glGenPathsNV = (PFNGLGENPATHSNVPROC) load("glGenPathsNV", userptr);
    glGetPathCommandsNV = (PFNGLGETPATHCOMMANDSNVPROC) load("glGetPathCommandsNV", userptr);
    glGetPathCoordsNV = (PFNGLGETPATHCOORDSNVPROC) load("glGetPathCoordsNV", userptr);
    glGetPathDashArrayNV = (PFNGLGETPATHDASHARRAYNVPROC) load("glGetPathDashArrayNV", userptr);
    glGetPathLengthNV = (PFNGLGETPATHLENGTHNVPROC) load("glGetPathLengthNV", userptr);
    glGetPathMetricRangeNV = (PFNGLGETPATHMETRICRANGENVPROC) load("glGetPathMetricRangeNV", userptr);
    glGetPathMetricsNV = (PFNGLGETPATHMETRICSNVPROC) load("glGetPathMetricsNV", userptr);
    glGetPathParameterfvNV = (PFNGLGETPATHPARAMETERFVNVPROC) load("glGetPathParameterfvNV", userptr);
    glGetPathParameterivNV = (PFNGLGETPATHPARAMETERIVNVPROC) load("glGetPathParameterivNV", userptr);
    glGetPathSpacingNV = (PFNGLGETPATHSPACINGNVPROC) load("glGetPathSpacingNV", userptr);
    glGetProgramResourcefvNV = (PFNGLGETPROGRAMRESOURCEFVNVPROC) load("glGetProgramResourcefvNV", userptr);
    glInterpolatePathsNV = (PFNGLINTERPOLATEPATHSNVPROC) load("glInterpolatePathsNV", userptr);
    glIsPathNV = (PFNGLISPATHNVPROC) load("glIsPathNV", userptr);
    glIsPointInFillPathNV = (PFNGLISPOINTINFILLPATHNVPROC) load("glIsPointInFillPathNV", userptr);
    glIsPointInStrokePathNV = (PFNGLISPOINTINSTROKEPATHNVPROC) load("glIsPointInStrokePathNV", userptr);
    glMatrixFrustumEXT = (PFNGLMATRIXFRUSTUMEXTPROC) load("glMatrixFrustumEXT", userptr);
    glMatrixLoad3x2fNV = (PFNGLMATRIXLOAD3X2FNVPROC) load("glMatrixLoad3x2fNV", userptr);
    glMatrixLoad3x3fNV = (PFNGLMATRIXLOAD3X3FNVPROC) load("glMatrixLoad3x3fNV", userptr);
    glMatrixLoadIdentityEXT = (PFNGLMATRIXLOADIDENTITYEXTPROC) load("glMatrixLoadIdentityEXT", userptr);
    glMatrixLoadTranspose3x3fNV = (PFNGLMATRIXLOADTRANSPOSE3X3FNVPROC) load("glMatrixLoadTranspose3x3fNV", userptr);
    glMatrixLoadTransposedEXT = (PFNGLMATRIXLOADTRANSPOSEDEXTPROC) load("glMatrixLoadTransposedEXT", userptr);
    glMatrixLoadTransposefEXT = (PFNGLMATRIXLOADTRANSPOSEFEXTPROC) load("glMatrixLoadTransposefEXT", userptr);
    glMatrixLoaddEXT = (PFNGLMATRIXLOADDEXTPROC) load("glMatrixLoaddEXT", userptr);
    glMatrixLoadfEXT = (PFNGLMATRIXLOADFEXTPROC) load("glMatrixLoadfEXT", userptr);
    glMatrixMult3x2fNV = (PFNGLMATRIXMULT3X2FNVPROC) load("glMatrixMult3x2fNV", userptr);
    glMatrixMult3x3fNV = (PFNGLMATRIXMULT3X3FNVPROC) load("glMatrixMult3x3fNV", userptr);
    glMatrixMultTranspose3x3fNV = (PFNGLMATRIXMULTTRANSPOSE3X3FNVPROC) load("glMatrixMultTranspose3x3fNV", userptr);
    glMatrixMultTransposedEXT = (PFNGLMATRIXMULTTRANSPOSEDEXTPROC) load("glMatrixMultTransposedEXT", userptr);
    glMatrixMultTransposefEXT = (PFNGLMATRIXMULTTRANSPOSEFEXTPROC) load("glMatrixMultTransposefEXT", userptr);
    glMatrixMultdEXT = (PFNGLMATRIXMULTDEXTPROC) load("glMatrixMultdEXT", userptr);
    glMatrixMultfEXT = (PFNGLMATRIXMULTFEXTPROC) load("glMatrixMultfEXT", userptr);
    glMatrixOrthoEXT = (PFNGLMATRIXORTHOEXTPROC) load("glMatrixOrthoEXT", userptr);
    glMatrixPopEXT = (PFNGLMATRIXPOPEXTPROC) load("glMatrixPopEXT", userptr);
    glMatrixPushEXT = (PFNGLMATRIXPUSHEXTPROC) load("glMatrixPushEXT", userptr);
    glMatrixRotatedEXT = (PFNGLMATRIXROTATEDEXTPROC) load("glMatrixRotatedEXT", userptr);
    glMatrixRotatefEXT = (PFNGLMATRIXROTATEFEXTPROC) load("glMatrixRotatefEXT", userptr);
    glMatrixScaledEXT = (PFNGLMATRIXSCALEDEXTPROC) load("glMatrixScaledEXT", userptr);
    glMatrixScalefEXT = (PFNGLMATRIXSCALEFEXTPROC) load("glMatrixScalefEXT", userptr);
    glMatrixTranslatedEXT = (PFNGLMATRIXTRANSLATEDEXTPROC) load("glMatrixTranslatedEXT", userptr);
    glMatrixTranslatefEXT = (PFNGLMATRIXTRANSLATEFEXTPROC) load("glMatrixTranslatefEXT", userptr);
    glPathCommandsNV = (PFNGLPATHCOMMANDSNVPROC) load("glPathCommandsNV", userptr);
    glPathCoordsNV = (PFNGLPATHCOORDSNVPROC) load("glPathCoordsNV", userptr);
    glPathCoverDepthFuncNV = (PFNGLPATHCOVERDEPTHFUNCNVPROC) load("glPathCoverDepthFuncNV", userptr);
    glPathDashArrayNV = (PFNGLPATHDASHARRAYNVPROC) load("glPathDashArrayNV", userptr);
    glPathGlyphIndexArrayNV = (PFNGLPATHGLYPHINDEXARRAYNVPROC) load("glPathGlyphIndexArrayNV", userptr);
    glPathGlyphIndexRangeNV = (PFNGLPATHGLYPHINDEXRANGENVPROC) load("glPathGlyphIndexRangeNV", userptr);
    glPathGlyphRangeNV = (PFNGLPATHGLYPHRANGENVPROC) load("glPathGlyphRangeNV", userptr);
    glPathGlyphsNV = (PFNGLPATHGLYPHSNVPROC) load("glPathGlyphsNV", userptr);
    glPathMemoryGlyphIndexArrayNV = (PFNGLPATHMEMORYGLYPHINDEXARRAYNVPROC) load("glPathMemoryGlyphIndexArrayNV", userptr);
    glPathParameterfNV = (PFNGLPATHPARAMETERFNVPROC) load("glPathParameterfNV", userptr);
    glPathParameterfvNV = (PFNGLPATHPARAMETERFVNVPROC) load("glPathParameterfvNV", userptr);
    glPathParameteriNV = (PFNGLPATHPARAMETERINVPROC) load("glPathParameteriNV", userptr);
    glPathParameterivNV = (PFNGLPATHPARAMETERIVNVPROC) load("glPathParameterivNV", userptr);
    glPathStencilDepthOffsetNV = (PFNGLPATHSTENCILDEPTHOFFSETNVPROC) load("glPathStencilDepthOffsetNV", userptr);
    glPathStencilFuncNV = (PFNGLPATHSTENCILFUNCNVPROC) load("glPathStencilFuncNV", userptr);
    glPathStringNV = (PFNGLPATHSTRINGNVPROC) load("glPathStringNV", userptr);
    glPathSubCommandsNV = (PFNGLPATHSUBCOMMANDSNVPROC) load("glPathSubCommandsNV", userptr);
    glPathSubCoordsNV = (PFNGLPATHSUBCOORDSNVPROC) load("glPathSubCoordsNV", userptr);
    glPointAlongPathNV = (PFNGLPOINTALONGPATHNVPROC) load("glPointAlongPathNV", userptr);
    glProgramPathFragmentInputGenNV = (PFNGLPROGRAMPATHFRAGMENTINPUTGENNVPROC) load("glProgramPathFragmentInputGenNV", userptr);
    glStencilFillPathInstancedNV = (PFNGLSTENCILFILLPATHINSTANCEDNVPROC) load("glStencilFillPathInstancedNV", userptr);
    glStencilFillPathNV = (PFNGLSTENCILFILLPATHNVPROC) load("glStencilFillPathNV", userptr);
    glStencilStrokePathInstancedNV = (PFNGLSTENCILSTROKEPATHINSTANCEDNVPROC) load("glStencilStrokePathInstancedNV", userptr);
    glStencilStrokePathNV = (PFNGLSTENCILSTROKEPATHNVPROC) load("glStencilStrokePathNV", userptr);
    glStencilThenCoverFillPathInstancedNV = (PFNGLSTENCILTHENCOVERFILLPATHINSTANCEDNVPROC) load("glStencilThenCoverFillPathInstancedNV", userptr);
    glStencilThenCoverFillPathNV = (PFNGLSTENCILTHENCOVERFILLPATHNVPROC) load("glStencilThenCoverFillPathNV", userptr);
    glStencilThenCoverStrokePathInstancedNV = (PFNGLSTENCILTHENCOVERSTROKEPATHINSTANCEDNVPROC) load("glStencilThenCoverStrokePathInstancedNV", userptr);
    glStencilThenCoverStrokePathNV = (PFNGLSTENCILTHENCOVERSTROKEPATHNVPROC) load("glStencilThenCoverStrokePathNV", userptr);
    glTransformPathNV = (PFNGLTRANSFORMPATHNVPROC) load("glTransformPathNV", userptr);
    glWeightPathsNV = (PFNGLWEIGHTPATHSNVPROC) load("glWeightPathsNV", userptr);
}
static void glad_gl_load_GL_NV_polygon_mode( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_polygon_mode) return;
    glPolygonMode = (PFNGLPOLYGONMODEPROC) load("glPolygonMode", userptr);
    glPolygonModeNV = (PFNGLPOLYGONMODENVPROC) load("glPolygonModeNV", userptr);
}
static void glad_gl_load_GL_NV_read_buffer( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_read_buffer) return;
    glReadBufferNV = (PFNGLREADBUFFERNVPROC) load("glReadBufferNV", userptr);
}
static void glad_gl_load_GL_NV_sample_locations( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_sample_locations) return;
    glFramebufferSampleLocationsfvNV = (PFNGLFRAMEBUFFERSAMPLELOCATIONSFVNVPROC) load("glFramebufferSampleLocationsfvNV", userptr);
    glNamedFramebufferSampleLocationsfvNV = (PFNGLNAMEDFRAMEBUFFERSAMPLELOCATIONSFVNVPROC) load("glNamedFramebufferSampleLocationsfvNV", userptr);
    glResolveDepthValuesNV = (PFNGLRESOLVEDEPTHVALUESNVPROC) load("glResolveDepthValuesNV", userptr);
}
static void glad_gl_load_GL_NV_viewport_array( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_viewport_array) return;
    glDepthRangeArrayfvNV = (PFNGLDEPTHRANGEARRAYFVNVPROC) load("glDepthRangeArrayfvNV", userptr);
    glDepthRangeIndexedfNV = (PFNGLDEPTHRANGEINDEXEDFNVPROC) load("glDepthRangeIndexedfNV", userptr);
    glDisablei = (PFNGLDISABLEIPROC) load("glDisablei", userptr);
    glDisableiNV = (PFNGLDISABLEINVPROC) load("glDisableiNV", userptr);
    glEnablei = (PFNGLENABLEIPROC) load("glEnablei", userptr);
    glEnableiNV = (PFNGLENABLEINVPROC) load("glEnableiNV", userptr);
    glGetFloati_v = (PFNGLGETFLOATI_VPROC) load("glGetFloati_v", userptr);
    glGetFloati_vNV = (PFNGLGETFLOATI_VNVPROC) load("glGetFloati_vNV", userptr);
    glIsEnabledi = (PFNGLISENABLEDIPROC) load("glIsEnabledi", userptr);
    glIsEnablediNV = (PFNGLISENABLEDINVPROC) load("glIsEnablediNV", userptr);
    glScissorArrayv = (PFNGLSCISSORARRAYVPROC) load("glScissorArrayv", userptr);
    glScissorArrayvNV = (PFNGLSCISSORARRAYVNVPROC) load("glScissorArrayvNV", userptr);
    glScissorIndexed = (PFNGLSCISSORINDEXEDPROC) load("glScissorIndexed", userptr);
    glScissorIndexedNV = (PFNGLSCISSORINDEXEDNVPROC) load("glScissorIndexedNV", userptr);
    glScissorIndexedv = (PFNGLSCISSORINDEXEDVPROC) load("glScissorIndexedv", userptr);
    glScissorIndexedvNV = (PFNGLSCISSORINDEXEDVNVPROC) load("glScissorIndexedvNV", userptr);
    glViewportArrayv = (PFNGLVIEWPORTARRAYVPROC) load("glViewportArrayv", userptr);
    glViewportArrayvNV = (PFNGLVIEWPORTARRAYVNVPROC) load("glViewportArrayvNV", userptr);
    glViewportIndexedf = (PFNGLVIEWPORTINDEXEDFPROC) load("glViewportIndexedf", userptr);
    glViewportIndexedfNV = (PFNGLVIEWPORTINDEXEDFNVPROC) load("glViewportIndexedfNV", userptr);
    glViewportIndexedfv = (PFNGLVIEWPORTINDEXEDFVPROC) load("glViewportIndexedfv", userptr);
    glViewportIndexedfvNV = (PFNGLVIEWPORTINDEXEDFVNVPROC) load("glViewportIndexedfvNV", userptr);
}
static void glad_gl_load_GL_NV_viewport_swizzle( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_NV_viewport_swizzle) return;
    glViewportSwizzleNV = (PFNGLVIEWPORTSWIZZLENVPROC) load("glViewportSwizzleNV", userptr);
}
static void glad_gl_load_GL_OES_EGL_image( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_EGL_image) return;
    glEGLImageTargetRenderbufferStorageOES = (PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC) load("glEGLImageTargetRenderbufferStorageOES", userptr);
    glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) load("glEGLImageTargetTexture2DOES", userptr);
}
static void glad_gl_load_GL_OES_copy_image( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_copy_image) return;
    glCopyImageSubData = (PFNGLCOPYIMAGESUBDATAPROC) load("glCopyImageSubData", userptr);
    glCopyImageSubDataOES = (PFNGLCOPYIMAGESUBDATAOESPROC) load("glCopyImageSubDataOES", userptr);
}
static void glad_gl_load_GL_OES_draw_buffers_indexed( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_draw_buffers_indexed) return;
    glBlendEquationSeparatei = (PFNGLBLENDEQUATIONSEPARATEIPROC) load("glBlendEquationSeparatei", userptr);
    glBlendEquationSeparateiOES = (PFNGLBLENDEQUATIONSEPARATEIOESPROC) load("glBlendEquationSeparateiOES", userptr);
    glBlendEquationi = (PFNGLBLENDEQUATIONIPROC) load("glBlendEquationi", userptr);
    glBlendEquationiOES = (PFNGLBLENDEQUATIONIOESPROC) load("glBlendEquationiOES", userptr);
    glBlendFuncSeparatei = (PFNGLBLENDFUNCSEPARATEIPROC) load("glBlendFuncSeparatei", userptr);
    glBlendFuncSeparateiOES = (PFNGLBLENDFUNCSEPARATEIOESPROC) load("glBlendFuncSeparateiOES", userptr);
    glBlendFunci = (PFNGLBLENDFUNCIPROC) load("glBlendFunci", userptr);
    glBlendFunciOES = (PFNGLBLENDFUNCIOESPROC) load("glBlendFunciOES", userptr);
    glColorMaski = (PFNGLCOLORMASKIPROC) load("glColorMaski", userptr);
    glColorMaskiOES = (PFNGLCOLORMASKIOESPROC) load("glColorMaskiOES", userptr);
    glDisablei = (PFNGLDISABLEIPROC) load("glDisablei", userptr);
    glDisableiOES = (PFNGLDISABLEIOESPROC) load("glDisableiOES", userptr);
    glEnablei = (PFNGLENABLEIPROC) load("glEnablei", userptr);
    glEnableiOES = (PFNGLENABLEIOESPROC) load("glEnableiOES", userptr);
    glIsEnabledi = (PFNGLISENABLEDIPROC) load("glIsEnabledi", userptr);
    glIsEnablediOES = (PFNGLISENABLEDIOESPROC) load("glIsEnablediOES", userptr);
}
static void glad_gl_load_GL_OES_draw_elements_base_vertex( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_draw_elements_base_vertex) return;
    glDrawElementsBaseVertex = (PFNGLDRAWELEMENTSBASEVERTEXPROC) load("glDrawElementsBaseVertex", userptr);
    glDrawElementsBaseVertexOES = (PFNGLDRAWELEMENTSBASEVERTEXOESPROC) load("glDrawElementsBaseVertexOES", userptr);
    glDrawElementsInstancedBaseVertex = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC) load("glDrawElementsInstancedBaseVertex", userptr);
    glDrawElementsInstancedBaseVertexOES = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXOESPROC) load("glDrawElementsInstancedBaseVertexOES", userptr);
    glDrawRangeElementsBaseVertex = (PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC) load("glDrawRangeElementsBaseVertex", userptr);
    glDrawRangeElementsBaseVertexOES = (PFNGLDRAWRANGEELEMENTSBASEVERTEXOESPROC) load("glDrawRangeElementsBaseVertexOES", userptr);
    glMultiDrawElementsBaseVertex = (PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC) load("glMultiDrawElementsBaseVertex", userptr);
    glMultiDrawElementsBaseVertexEXT = (PFNGLMULTIDRAWELEMENTSBASEVERTEXEXTPROC) load("glMultiDrawElementsBaseVertexEXT", userptr);
}
static void glad_gl_load_GL_OES_geometry_shader( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_geometry_shader) return;
    glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC) load("glFramebufferTexture", userptr);
    glFramebufferTextureOES = (PFNGLFRAMEBUFFERTEXTUREOESPROC) load("glFramebufferTextureOES", userptr);
}
static void glad_gl_load_GL_OES_get_program_binary( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_get_program_binary) return;
    glGetProgramBinary = (PFNGLGETPROGRAMBINARYPROC) load("glGetProgramBinary", userptr);
    glGetProgramBinaryOES = (PFNGLGETPROGRAMBINARYOESPROC) load("glGetProgramBinaryOES", userptr);
    glProgramBinary = (PFNGLPROGRAMBINARYPROC) load("glProgramBinary", userptr);
    glProgramBinaryOES = (PFNGLPROGRAMBINARYOESPROC) load("glProgramBinaryOES", userptr);
}
static void glad_gl_load_GL_OES_mapbuffer( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_mapbuffer) return;
    glGetBufferPointerv = (PFNGLGETBUFFERPOINTERVPROC) load("glGetBufferPointerv", userptr);
    glGetBufferPointervOES = (PFNGLGETBUFFERPOINTERVOESPROC) load("glGetBufferPointervOES", userptr);
    glMapBuffer = (PFNGLMAPBUFFERPROC) load("glMapBuffer", userptr);
    glMapBufferOES = (PFNGLMAPBUFFEROESPROC) load("glMapBufferOES", userptr);
    glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) load("glUnmapBuffer", userptr);
    glUnmapBufferOES = (PFNGLUNMAPBUFFEROESPROC) load("glUnmapBufferOES", userptr);
}
static void glad_gl_load_GL_OES_primitive_bounding_box( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_primitive_bounding_box) return;
    glPrimitiveBoundingBox = (PFNGLPRIMITIVEBOUNDINGBOXPROC) load("glPrimitiveBoundingBox", userptr);
    glPrimitiveBoundingBoxOES = (PFNGLPRIMITIVEBOUNDINGBOXOESPROC) load("glPrimitiveBoundingBoxOES", userptr);
}
static void glad_gl_load_GL_OES_sample_shading( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_sample_shading) return;
    glMinSampleShading = (PFNGLMINSAMPLESHADINGPROC) load("glMinSampleShading", userptr);
    glMinSampleShadingOES = (PFNGLMINSAMPLESHADINGOESPROC) load("glMinSampleShadingOES", userptr);
}
static void glad_gl_load_GL_OES_tessellation_shader( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_tessellation_shader) return;
    glPatchParameteri = (PFNGLPATCHPARAMETERIPROC) load("glPatchParameteri", userptr);
    glPatchParameteriOES = (PFNGLPATCHPARAMETERIOESPROC) load("glPatchParameteriOES", userptr);
}
static void glad_gl_load_GL_OES_texture_3D( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_texture_3D) return;
    glCompressedTexImage3DOES = (PFNGLCOMPRESSEDTEXIMAGE3DOESPROC) load("glCompressedTexImage3DOES", userptr);
    glCompressedTexSubImage3DOES = (PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC) load("glCompressedTexSubImage3DOES", userptr);
    glCopyTexSubImage3DOES = (PFNGLCOPYTEXSUBIMAGE3DOESPROC) load("glCopyTexSubImage3DOES", userptr);
    glFramebufferTexture3DOES = (PFNGLFRAMEBUFFERTEXTURE3DOESPROC) load("glFramebufferTexture3DOES", userptr);
    glTexImage3DOES = (PFNGLTEXIMAGE3DOESPROC) load("glTexImage3DOES", userptr);
    glTexSubImage3DOES = (PFNGLTEXSUBIMAGE3DOESPROC) load("glTexSubImage3DOES", userptr);
}
static void glad_gl_load_GL_OES_texture_border_clamp( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_texture_border_clamp) return;
    glGetSamplerParameterIiv = (PFNGLGETSAMPLERPARAMETERIIVPROC) load("glGetSamplerParameterIiv", userptr);
    glGetSamplerParameterIivOES = (PFNGLGETSAMPLERPARAMETERIIVOESPROC) load("glGetSamplerParameterIivOES", userptr);
    glGetSamplerParameterIuiv = (PFNGLGETSAMPLERPARAMETERIUIVPROC) load("glGetSamplerParameterIuiv", userptr);
    glGetSamplerParameterIuivOES = (PFNGLGETSAMPLERPARAMETERIUIVOESPROC) load("glGetSamplerParameterIuivOES", userptr);
    glGetTexParameterIiv = (PFNGLGETTEXPARAMETERIIVPROC) load("glGetTexParameterIiv", userptr);
    glGetTexParameterIivOES = (PFNGLGETTEXPARAMETERIIVOESPROC) load("glGetTexParameterIivOES", userptr);
    glGetTexParameterIuiv = (PFNGLGETTEXPARAMETERIUIVPROC) load("glGetTexParameterIuiv", userptr);
    glGetTexParameterIuivOES = (PFNGLGETTEXPARAMETERIUIVOESPROC) load("glGetTexParameterIuivOES", userptr);
    glSamplerParameterIiv = (PFNGLSAMPLERPARAMETERIIVPROC) load("glSamplerParameterIiv", userptr);
    glSamplerParameterIivOES = (PFNGLSAMPLERPARAMETERIIVOESPROC) load("glSamplerParameterIivOES", userptr);
    glSamplerParameterIuiv = (PFNGLSAMPLERPARAMETERIUIVPROC) load("glSamplerParameterIuiv", userptr);
    glSamplerParameterIuivOES = (PFNGLSAMPLERPARAMETERIUIVOESPROC) load("glSamplerParameterIuivOES", userptr);
    glTexParameterIiv = (PFNGLTEXPARAMETERIIVPROC) load("glTexParameterIiv", userptr);
    glTexParameterIivOES = (PFNGLTEXPARAMETERIIVOESPROC) load("glTexParameterIivOES", userptr);
    glTexParameterIuiv = (PFNGLTEXPARAMETERIUIVPROC) load("glTexParameterIuiv", userptr);
    glTexParameterIuivOES = (PFNGLTEXPARAMETERIUIVOESPROC) load("glTexParameterIuivOES", userptr);
}
static void glad_gl_load_GL_OES_texture_buffer( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_texture_buffer) return;
    glTexBuffer = (PFNGLTEXBUFFERPROC) load("glTexBuffer", userptr);
    glTexBufferOES = (PFNGLTEXBUFFEROESPROC) load("glTexBufferOES", userptr);
    glTexBufferRange = (PFNGLTEXBUFFERRANGEPROC) load("glTexBufferRange", userptr);
    glTexBufferRangeOES = (PFNGLTEXBUFFERRANGEOESPROC) load("glTexBufferRangeOES", userptr);
}
static void glad_gl_load_GL_OES_texture_storage_multisample_2d_array( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_texture_storage_multisample_2d_array) return;
    glTexStorage3DMultisample = (PFNGLTEXSTORAGE3DMULTISAMPLEPROC) load("glTexStorage3DMultisample", userptr);
    glTexStorage3DMultisampleOES = (PFNGLTEXSTORAGE3DMULTISAMPLEOESPROC) load("glTexStorage3DMultisampleOES", userptr);
}
static void glad_gl_load_GL_OES_texture_view( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_texture_view) return;
    glTextureView = (PFNGLTEXTUREVIEWPROC) load("glTextureView", userptr);
    glTextureViewOES = (PFNGLTEXTUREVIEWOESPROC) load("glTextureViewOES", userptr);
}
static void glad_gl_load_GL_OES_vertex_array_object( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_vertex_array_object) return;
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) load("glBindVertexArray", userptr);
    glBindVertexArrayOES = (PFNGLBINDVERTEXARRAYOESPROC) load("glBindVertexArrayOES", userptr);
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) load("glDeleteVertexArrays", userptr);
    glDeleteVertexArraysOES = (PFNGLDELETEVERTEXARRAYSOESPROC) load("glDeleteVertexArraysOES", userptr);
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) load("glGenVertexArrays", userptr);
    glGenVertexArraysOES = (PFNGLGENVERTEXARRAYSOESPROC) load("glGenVertexArraysOES", userptr);
    glIsVertexArray = (PFNGLISVERTEXARRAYPROC) load("glIsVertexArray", userptr);
    glIsVertexArrayOES = (PFNGLISVERTEXARRAYOESPROC) load("glIsVertexArrayOES", userptr);
}
static void glad_gl_load_GL_OES_viewport_array( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_viewport_array) return;
    glDepthRangeArrayfvOES = (PFNGLDEPTHRANGEARRAYFVOESPROC) load("glDepthRangeArrayfvOES", userptr);
    glDepthRangeIndexedfOES = (PFNGLDEPTHRANGEINDEXEDFOESPROC) load("glDepthRangeIndexedfOES", userptr);
    glDisablei = (PFNGLDISABLEIPROC) load("glDisablei", userptr);
    glDisableiOES = (PFNGLDISABLEIOESPROC) load("glDisableiOES", userptr);
    glEnablei = (PFNGLENABLEIPROC) load("glEnablei", userptr);
    glEnableiOES = (PFNGLENABLEIOESPROC) load("glEnableiOES", userptr);
    glGetFloati_v = (PFNGLGETFLOATI_VPROC) load("glGetFloati_v", userptr);
    glGetFloati_vOES = (PFNGLGETFLOATI_VOESPROC) load("glGetFloati_vOES", userptr);
    glIsEnabledi = (PFNGLISENABLEDIPROC) load("glIsEnabledi", userptr);
    glIsEnablediOES = (PFNGLISENABLEDIOESPROC) load("glIsEnablediOES", userptr);
    glScissorArrayv = (PFNGLSCISSORARRAYVPROC) load("glScissorArrayv", userptr);
    glScissorArrayvOES = (PFNGLSCISSORARRAYVOESPROC) load("glScissorArrayvOES", userptr);
    glScissorIndexed = (PFNGLSCISSORINDEXEDPROC) load("glScissorIndexed", userptr);
    glScissorIndexedOES = (PFNGLSCISSORINDEXEDOESPROC) load("glScissorIndexedOES", userptr);
    glScissorIndexedv = (PFNGLSCISSORINDEXEDVPROC) load("glScissorIndexedv", userptr);
    glScissorIndexedvOES = (PFNGLSCISSORINDEXEDVOESPROC) load("glScissorIndexedvOES", userptr);
    glViewportArrayv = (PFNGLVIEWPORTARRAYVPROC) load("glViewportArrayv", userptr);
    glViewportArrayvOES = (PFNGLVIEWPORTARRAYVOESPROC) load("glViewportArrayvOES", userptr);
    glViewportIndexedf = (PFNGLVIEWPORTINDEXEDFPROC) load("glViewportIndexedf", userptr);
    glViewportIndexedfOES = (PFNGLVIEWPORTINDEXEDFOESPROC) load("glViewportIndexedfOES", userptr);
    glViewportIndexedfv = (PFNGLVIEWPORTINDEXEDFVPROC) load("glViewportIndexedfv", userptr);
    glViewportIndexedfvOES = (PFNGLVIEWPORTINDEXEDFVOESPROC) load("glViewportIndexedfvOES", userptr);
}
static void glad_gl_load_GL_OVR_multiview( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OVR_multiview) return;
    glFramebufferTextureMultiviewOVR = (PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC) load("glFramebufferTextureMultiviewOVR", userptr);
}
static void glad_gl_load_GL_OVR_multiview_multisampled_render_to_texture( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OVR_multiview_multisampled_render_to_texture) return;
    glFramebufferTextureMultisampleMultiviewOVR = (PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVRPROC) load("glFramebufferTextureMultisampleMultiviewOVR", userptr);
}
static void glad_gl_load_GL_QCOM_alpha_test( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_QCOM_alpha_test) return;
    glAlphaFuncQCOM = (PFNGLALPHAFUNCQCOMPROC) load("glAlphaFuncQCOM", userptr);
}
static void glad_gl_load_GL_QCOM_driver_control( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_QCOM_driver_control) return;
    glDisableDriverControlQCOM = (PFNGLDISABLEDRIVERCONTROLQCOMPROC) load("glDisableDriverControlQCOM", userptr);
    glEnableDriverControlQCOM = (PFNGLENABLEDRIVERCONTROLQCOMPROC) load("glEnableDriverControlQCOM", userptr);
    glGetDriverControlStringQCOM = (PFNGLGETDRIVERCONTROLSTRINGQCOMPROC) load("glGetDriverControlStringQCOM", userptr);
    glGetDriverControlsQCOM = (PFNGLGETDRIVERCONTROLSQCOMPROC) load("glGetDriverControlsQCOM", userptr);
}
static void glad_gl_load_GL_QCOM_extended_get( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_QCOM_extended_get) return;
    glExtGetBufferPointervQCOM = (PFNGLEXTGETBUFFERPOINTERVQCOMPROC) load("glExtGetBufferPointervQCOM", userptr);
    glExtGetBuffersQCOM = (PFNGLEXTGETBUFFERSQCOMPROC) load("glExtGetBuffersQCOM", userptr);
    glExtGetFramebuffersQCOM = (PFNGLEXTGETFRAMEBUFFERSQCOMPROC) load("glExtGetFramebuffersQCOM", userptr);
    glExtGetRenderbuffersQCOM = (PFNGLEXTGETRENDERBUFFERSQCOMPROC) load("glExtGetRenderbuffersQCOM", userptr);
    glExtGetTexLevelParameterivQCOM = (PFNGLEXTGETTEXLEVELPARAMETERIVQCOMPROC) load("glExtGetTexLevelParameterivQCOM", userptr);
    glExtGetTexSubImageQCOM = (PFNGLEXTGETTEXSUBIMAGEQCOMPROC) load("glExtGetTexSubImageQCOM", userptr);
    glExtGetTexturesQCOM = (PFNGLEXTGETTEXTURESQCOMPROC) load("glExtGetTexturesQCOM", userptr);
    glExtTexObjectStateOverrideiQCOM = (PFNGLEXTTEXOBJECTSTATEOVERRIDEIQCOMPROC) load("glExtTexObjectStateOverrideiQCOM", userptr);
}
static void glad_gl_load_GL_QCOM_extended_get2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_QCOM_extended_get2) return;
    glExtGetProgramBinarySourceQCOM = (PFNGLEXTGETPROGRAMBINARYSOURCEQCOMPROC) load("glExtGetProgramBinarySourceQCOM", userptr);
    glExtGetProgramsQCOM = (PFNGLEXTGETPROGRAMSQCOMPROC) load("glExtGetProgramsQCOM", userptr);
    glExtGetShadersQCOM = (PFNGLEXTGETSHADERSQCOMPROC) load("glExtGetShadersQCOM", userptr);
    glExtIsProgramBinaryQCOM = (PFNGLEXTISPROGRAMBINARYQCOMPROC) load("glExtIsProgramBinaryQCOM", userptr);
}
static void glad_gl_load_GL_QCOM_framebuffer_foveated( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_QCOM_framebuffer_foveated) return;
    glFramebufferFoveationConfigQCOM = (PFNGLFRAMEBUFFERFOVEATIONCONFIGQCOMPROC) load("glFramebufferFoveationConfigQCOM", userptr);
    glFramebufferFoveationParametersQCOM = (PFNGLFRAMEBUFFERFOVEATIONPARAMETERSQCOMPROC) load("glFramebufferFoveationParametersQCOM", userptr);
}
static void glad_gl_load_GL_QCOM_shader_framebuffer_fetch_noncoherent( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_QCOM_shader_framebuffer_fetch_noncoherent) return;
    glFramebufferFetchBarrierQCOM = (PFNGLFRAMEBUFFERFETCHBARRIERQCOMPROC) load("glFramebufferFetchBarrierQCOM", userptr);
}
static void glad_gl_load_GL_QCOM_texture_foveated( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_QCOM_texture_foveated) return;
    glTextureFoveationParametersQCOM = (PFNGLTEXTUREFOVEATIONPARAMETERSQCOMPROC) load("glTextureFoveationParametersQCOM", userptr);
}
static void glad_gl_load_GL_QCOM_tiled_rendering( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_QCOM_tiled_rendering) return;
    glEndTilingQCOM = (PFNGLENDTILINGQCOMPROC) load("glEndTilingQCOM", userptr);
    glStartTilingQCOM = (PFNGLSTARTTILINGQCOMPROC) load("glStartTilingQCOM", userptr);
}



#if defined(GL_ES_VERSION_3_0) || defined(GL_VERSION_3_0)
#define GLAD_GL_IS_SOME_NEW_VERSION 1
#else
#define GLAD_GL_IS_SOME_NEW_VERSION 0
#endif

static int glad_gl_get_extensions( int version, const char **out_exts, unsigned int *out_num_exts_i, char ***out_exts_i) {
#if GLAD_GL_IS_SOME_NEW_VERSION
    if(GLAD_VERSION_MAJOR(version) < 3) {
#else
    (void) version;
    (void) out_num_exts_i;
    (void) out_exts_i;
#endif
        if (glGetString == NULL) {
            return 0;
        }
        *out_exts = (const char *)glGetString(GL_EXTENSIONS);
#if GLAD_GL_IS_SOME_NEW_VERSION
    } else {
        unsigned int index = 0;
        unsigned int num_exts_i = 0;
        char **exts_i = NULL;
        if (glGetStringi == NULL || glGetIntegerv == NULL) {
            return 0;
        }
        glGetIntegerv(GL_NUM_EXTENSIONS, (int*) &num_exts_i);
        if (num_exts_i > 0) {
            exts_i = (char **) malloc(num_exts_i * (sizeof *exts_i));
        }
        if (exts_i == NULL) {
            return 0;
        }
        for(index = 0; index < num_exts_i; index++) {
            const char *gl_str_tmp = (const char*) glGetStringi(GL_EXTENSIONS, index);
            size_t len = strlen(gl_str_tmp) + 1;

            char *local_str = (char*) malloc(len * sizeof(char));
            if(local_str != NULL) {
                memcpy(local_str, gl_str_tmp, len * sizeof(char));
            }

            exts_i[index] = local_str;
        }

        *out_num_exts_i = num_exts_i;
        *out_exts_i = exts_i;
    }
#endif
    return 1;
}
static void glad_gl_free_extensions(char **exts_i, unsigned int num_exts_i) {
    if (exts_i != NULL) {
        unsigned int index;
        for(index = 0; index < num_exts_i; index++) {
            free((void *) (exts_i[index]));
        }
        free((void *)exts_i);
        exts_i = NULL;
    }
}
static int glad_gl_has_extension(int version, const char *exts, unsigned int num_exts_i, char **exts_i, const char *ext) {
    if(GLAD_VERSION_MAJOR(version) < 3 || !GLAD_GL_IS_SOME_NEW_VERSION) {
        const char *extensions;
        const char *loc;
        const char *terminator;
        extensions = exts;
        if(extensions == NULL || ext == NULL) {
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
    } else {
        unsigned int index;
        for(index = 0; index < num_exts_i; index++) {
            const char *e = exts_i[index];
            if(strcmp(e, ext) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

static GLADapiproc glad_gl_get_proc_from_userptr(const char* name, void *userptr) {
    return (GLAD_GNUC_EXTENSION (GLADapiproc (*)(const char *name)) userptr)(name);
}

static int glad_gl_find_extensions_gles2( int version) {
    const char *exts = NULL;
    unsigned int num_exts_i = 0;
    char **exts_i = NULL;
    if (!glad_gl_get_extensions(version, &exts, &num_exts_i, &exts_i)) return 0;

    GLAD_GL_AMD_compressed_3DC_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_AMD_compressed_3DC_texture");
    GLAD_GL_AMD_compressed_ATC_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_AMD_compressed_ATC_texture");
    GLAD_GL_AMD_framebuffer_multisample_advanced = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_AMD_framebuffer_multisample_advanced");
    GLAD_GL_AMD_performance_monitor = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_AMD_performance_monitor");
    GLAD_GL_AMD_program_binary_Z400 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_AMD_program_binary_Z400");
    GLAD_GL_ANDROID_extension_pack_es31a = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_ANDROID_extension_pack_es31a");
    GLAD_GL_ANGLE_depth_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_ANGLE_depth_texture");
    GLAD_GL_ANGLE_framebuffer_blit = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_ANGLE_framebuffer_blit");
    GLAD_GL_ANGLE_framebuffer_multisample = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_ANGLE_framebuffer_multisample");
    GLAD_GL_ANGLE_instanced_arrays = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_ANGLE_instanced_arrays");
    GLAD_GL_ANGLE_pack_reverse_row_order = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_ANGLE_pack_reverse_row_order");
    GLAD_GL_ANGLE_program_binary = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_ANGLE_program_binary");
    GLAD_GL_ANGLE_texture_compression_dxt3 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_ANGLE_texture_compression_dxt3");
    GLAD_GL_ANGLE_texture_compression_dxt5 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_ANGLE_texture_compression_dxt5");
    GLAD_GL_ANGLE_texture_usage = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_ANGLE_texture_usage");
    GLAD_GL_ANGLE_translated_shader_source = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_ANGLE_translated_shader_source");
    GLAD_GL_APPLE_clip_distance = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_APPLE_clip_distance");
    GLAD_GL_APPLE_color_buffer_packed_float = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_APPLE_color_buffer_packed_float");
    GLAD_GL_APPLE_copy_texture_levels = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_APPLE_copy_texture_levels");
    GLAD_GL_APPLE_framebuffer_multisample = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_APPLE_framebuffer_multisample");
    GLAD_GL_APPLE_rgb_422 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_APPLE_rgb_422");
    GLAD_GL_APPLE_sync = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_APPLE_sync");
    GLAD_GL_APPLE_texture_format_BGRA8888 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_APPLE_texture_format_BGRA8888");
    GLAD_GL_APPLE_texture_max_level = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_APPLE_texture_max_level");
    GLAD_GL_APPLE_texture_packed_float = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_APPLE_texture_packed_float");
    GLAD_GL_ARM_mali_program_binary = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_ARM_mali_program_binary");
    GLAD_GL_ARM_mali_shader_binary = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_ARM_mali_shader_binary");
    GLAD_GL_ARM_rgba8 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_ARM_rgba8");
    GLAD_GL_ARM_shader_framebuffer_fetch = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_ARM_shader_framebuffer_fetch");
    GLAD_GL_ARM_shader_framebuffer_fetch_depth_stencil = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_ARM_shader_framebuffer_fetch_depth_stencil");
    GLAD_GL_DMP_program_binary = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_DMP_program_binary");
    GLAD_GL_DMP_shader_binary = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_DMP_shader_binary");
    GLAD_GL_EXT_EGL_image_array = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_EGL_image_array");
    GLAD_GL_EXT_EGL_image_storage = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_EGL_image_storage");
    GLAD_GL_EXT_YUV_target = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_YUV_target");
    GLAD_GL_EXT_base_instance = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_base_instance");
    GLAD_GL_EXT_blend_func_extended = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_blend_func_extended");
    GLAD_GL_EXT_blend_minmax = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_blend_minmax");
    GLAD_GL_EXT_buffer_storage = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_buffer_storage");
    GLAD_GL_EXT_clear_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_clear_texture");
    GLAD_GL_EXT_clip_control = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_clip_control");
    GLAD_GL_EXT_clip_cull_distance = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_clip_cull_distance");
    GLAD_GL_EXT_color_buffer_float = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_color_buffer_float");
    GLAD_GL_EXT_color_buffer_half_float = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_color_buffer_half_float");
    GLAD_GL_EXT_conservative_depth = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_conservative_depth");
    GLAD_GL_EXT_copy_image = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_copy_image");
    GLAD_GL_EXT_debug_label = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_debug_label");
    GLAD_GL_EXT_debug_marker = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_debug_marker");
    GLAD_GL_EXT_discard_framebuffer = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_discard_framebuffer");
    GLAD_GL_EXT_disjoint_timer_query = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_disjoint_timer_query");
    GLAD_GL_EXT_draw_buffers = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_draw_buffers");
    GLAD_GL_EXT_draw_buffers_indexed = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_draw_buffers_indexed");
    GLAD_GL_EXT_draw_elements_base_vertex = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_draw_elements_base_vertex");
    GLAD_GL_EXT_draw_instanced = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_draw_instanced");
    GLAD_GL_EXT_draw_transform_feedback = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_draw_transform_feedback");
    GLAD_GL_EXT_external_buffer = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_external_buffer");
    GLAD_GL_EXT_float_blend = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_float_blend");
    GLAD_GL_EXT_geometry_point_size = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_geometry_point_size");
    GLAD_GL_EXT_geometry_shader = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_geometry_shader");
    GLAD_GL_EXT_gpu_shader5 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_gpu_shader5");
    GLAD_GL_EXT_instanced_arrays = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_instanced_arrays");
    GLAD_GL_EXT_map_buffer_range = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_map_buffer_range");
    GLAD_GL_EXT_memory_object = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_memory_object");
    GLAD_GL_EXT_memory_object_fd = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_memory_object_fd");
    GLAD_GL_EXT_memory_object_win32 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_memory_object_win32");
    GLAD_GL_EXT_multi_draw_arrays = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_multi_draw_arrays");
    GLAD_GL_EXT_multi_draw_indirect = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_multi_draw_indirect");
    GLAD_GL_EXT_multisampled_compatibility = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_multisampled_compatibility");
    GLAD_GL_EXT_multisampled_render_to_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_multisampled_render_to_texture");
    GLAD_GL_EXT_multiview_draw_buffers = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_multiview_draw_buffers");
    GLAD_GL_EXT_occlusion_query_boolean = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_occlusion_query_boolean");
    GLAD_GL_EXT_polygon_offset_clamp = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_polygon_offset_clamp");
    GLAD_GL_EXT_post_depth_coverage = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_post_depth_coverage");
    GLAD_GL_EXT_primitive_bounding_box = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_primitive_bounding_box");
    GLAD_GL_EXT_protected_textures = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_protected_textures");
    GLAD_GL_EXT_pvrtc_sRGB = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_pvrtc_sRGB");
    GLAD_GL_EXT_raster_multisample = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_raster_multisample");
    GLAD_GL_EXT_read_format_bgra = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_read_format_bgra");
    GLAD_GL_EXT_render_snorm = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_render_snorm");
    GLAD_GL_EXT_robustness = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_robustness");
    GLAD_GL_EXT_sRGB = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_sRGB");
    GLAD_GL_EXT_sRGB_write_control = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_sRGB_write_control");
    GLAD_GL_EXT_semaphore = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_semaphore");
    GLAD_GL_EXT_semaphore_fd = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_semaphore_fd");
    GLAD_GL_EXT_semaphore_win32 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_semaphore_win32");
    GLAD_GL_EXT_separate_shader_objects = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_separate_shader_objects");
    GLAD_GL_EXT_shader_framebuffer_fetch = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_shader_framebuffer_fetch");
    GLAD_GL_EXT_shader_framebuffer_fetch_non_coherent = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_shader_framebuffer_fetch_non_coherent");
    GLAD_GL_EXT_shader_group_vote = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_shader_group_vote");
    GLAD_GL_EXT_shader_implicit_conversions = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_shader_implicit_conversions");
    GLAD_GL_EXT_shader_integer_mix = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_shader_integer_mix");
    GLAD_GL_EXT_shader_io_blocks = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_shader_io_blocks");
    GLAD_GL_EXT_shader_non_constant_global_initializers = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_shader_non_constant_global_initializers");
    GLAD_GL_EXT_shader_pixel_local_storage = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_shader_pixel_local_storage");
    GLAD_GL_EXT_shader_pixel_local_storage2 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_shader_pixel_local_storage2");
    GLAD_GL_EXT_shader_texture_lod = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_shader_texture_lod");
    GLAD_GL_EXT_shadow_samplers = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_shadow_samplers");
    GLAD_GL_EXT_sparse_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_sparse_texture");
    GLAD_GL_EXT_sparse_texture2 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_sparse_texture2");
    GLAD_GL_EXT_tessellation_point_size = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_tessellation_point_size");
    GLAD_GL_EXT_tessellation_shader = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_tessellation_shader");
    GLAD_GL_EXT_texture_border_clamp = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_border_clamp");
    GLAD_GL_EXT_texture_buffer = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_buffer");
    GLAD_GL_EXT_texture_compression_astc_decode_mode = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_compression_astc_decode_mode");
    GLAD_GL_EXT_texture_compression_bptc = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_compression_bptc");
    GLAD_GL_EXT_texture_compression_dxt1 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_compression_dxt1");
    GLAD_GL_EXT_texture_compression_rgtc = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_compression_rgtc");
    GLAD_GL_EXT_texture_compression_s3tc = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_compression_s3tc");
    GLAD_GL_EXT_texture_compression_s3tc_srgb = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_compression_s3tc_srgb");
    GLAD_GL_EXT_texture_cube_map_array = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_cube_map_array");
    GLAD_GL_EXT_texture_filter_anisotropic = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_filter_anisotropic");
    GLAD_GL_EXT_texture_filter_minmax = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_filter_minmax");
    GLAD_GL_EXT_texture_format_BGRA8888 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_format_BGRA8888");
    GLAD_GL_EXT_texture_format_sRGB_override = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_format_sRGB_override");
    GLAD_GL_EXT_texture_mirror_clamp_to_edge = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_mirror_clamp_to_edge");
    GLAD_GL_EXT_texture_norm16 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_norm16");
    GLAD_GL_EXT_texture_rg = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_rg");
    GLAD_GL_EXT_texture_sRGB_R8 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_sRGB_R8");
    GLAD_GL_EXT_texture_sRGB_RG8 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_sRGB_RG8");
    GLAD_GL_EXT_texture_sRGB_decode = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_sRGB_decode");
    GLAD_GL_EXT_texture_storage = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_storage");
    GLAD_GL_EXT_texture_type_2_10_10_10_REV = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_type_2_10_10_10_REV");
    GLAD_GL_EXT_texture_view = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_view");
    GLAD_GL_EXT_unpack_subimage = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_unpack_subimage");
    GLAD_GL_EXT_win32_keyed_mutex = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_win32_keyed_mutex");
    GLAD_GL_EXT_window_rectangles = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_window_rectangles");
    GLAD_GL_FJ_shader_binary_GCCSO = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_FJ_shader_binary_GCCSO");
    GLAD_GL_IMG_bindless_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_IMG_bindless_texture");
    GLAD_GL_IMG_framebuffer_downsample = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_IMG_framebuffer_downsample");
    GLAD_GL_IMG_multisampled_render_to_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_IMG_multisampled_render_to_texture");
    GLAD_GL_IMG_program_binary = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_IMG_program_binary");
    GLAD_GL_IMG_read_format = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_IMG_read_format");
    GLAD_GL_IMG_shader_binary = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_IMG_shader_binary");
    GLAD_GL_IMG_texture_compression_pvrtc = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_IMG_texture_compression_pvrtc");
    GLAD_GL_IMG_texture_compression_pvrtc2 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_IMG_texture_compression_pvrtc2");
    GLAD_GL_IMG_texture_filter_cubic = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_IMG_texture_filter_cubic");
    GLAD_GL_INTEL_blackhole_render = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_INTEL_blackhole_render");
    GLAD_GL_INTEL_conservative_rasterization = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_INTEL_conservative_rasterization");
    GLAD_GL_INTEL_framebuffer_CMAA = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_INTEL_framebuffer_CMAA");
    GLAD_GL_INTEL_performance_query = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_INTEL_performance_query");
    GLAD_GL_KHR_blend_equation_advanced = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_KHR_blend_equation_advanced");
    GLAD_GL_KHR_blend_equation_advanced_coherent = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_KHR_blend_equation_advanced_coherent");
    GLAD_GL_KHR_context_flush_control = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_KHR_context_flush_control");
    GLAD_GL_KHR_debug = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_KHR_debug");
    GLAD_GL_KHR_no_error = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_KHR_no_error");
    GLAD_GL_KHR_parallel_shader_compile = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_KHR_parallel_shader_compile");
    GLAD_GL_KHR_robust_buffer_access_behavior = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_KHR_robust_buffer_access_behavior");
    GLAD_GL_KHR_robustness = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_KHR_robustness");
    GLAD_GL_KHR_texture_compression_astc_hdr = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_KHR_texture_compression_astc_hdr");
    GLAD_GL_KHR_texture_compression_astc_ldr = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_KHR_texture_compression_astc_ldr");
    GLAD_GL_KHR_texture_compression_astc_sliced_3d = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_KHR_texture_compression_astc_sliced_3d");
    GLAD_GL_MESA_framebuffer_flip_y = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_MESA_framebuffer_flip_y");
    GLAD_GL_MESA_program_binary_formats = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_MESA_program_binary_formats");
    GLAD_GL_MESA_shader_integer_functions = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_MESA_shader_integer_functions");
    GLAD_GL_NVX_blend_equation_advanced_multi_draw_buffers = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NVX_blend_equation_advanced_multi_draw_buffers");
    GLAD_GL_NV_bindless_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_bindless_texture");
    GLAD_GL_NV_blend_equation_advanced = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_blend_equation_advanced");
    GLAD_GL_NV_blend_equation_advanced_coherent = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_blend_equation_advanced_coherent");
    GLAD_GL_NV_blend_minmax_factor = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_blend_minmax_factor");
    GLAD_GL_NV_clip_space_w_scaling = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_clip_space_w_scaling");
    GLAD_GL_NV_conditional_render = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_conditional_render");
    GLAD_GL_NV_conservative_raster = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_conservative_raster");
    GLAD_GL_NV_conservative_raster_pre_snap = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_conservative_raster_pre_snap");
    GLAD_GL_NV_conservative_raster_pre_snap_triangles = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_conservative_raster_pre_snap_triangles");
    GLAD_GL_NV_copy_buffer = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_copy_buffer");
    GLAD_GL_NV_coverage_sample = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_coverage_sample");
    GLAD_GL_NV_depth_nonlinear = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_depth_nonlinear");
    GLAD_GL_NV_draw_buffers = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_draw_buffers");
    GLAD_GL_NV_draw_instanced = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_draw_instanced");
    GLAD_GL_NV_draw_vulkan_image = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_draw_vulkan_image");
    GLAD_GL_NV_explicit_attrib_location = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_explicit_attrib_location");
    GLAD_GL_NV_fbo_color_attachments = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_fbo_color_attachments");
    GLAD_GL_NV_fence = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_fence");
    GLAD_GL_NV_fill_rectangle = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_fill_rectangle");
    GLAD_GL_NV_fragment_coverage_to_color = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_fragment_coverage_to_color");
    GLAD_GL_NV_fragment_shader_interlock = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_fragment_shader_interlock");
    GLAD_GL_NV_framebuffer_blit = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_framebuffer_blit");
    GLAD_GL_NV_framebuffer_mixed_samples = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_framebuffer_mixed_samples");
    GLAD_GL_NV_framebuffer_multisample = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_framebuffer_multisample");
    GLAD_GL_NV_generate_mipmap_sRGB = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_generate_mipmap_sRGB");
    GLAD_GL_NV_geometry_shader_passthrough = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_geometry_shader_passthrough");
    GLAD_GL_NV_gpu_shader5 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_gpu_shader5");
    GLAD_GL_NV_image_formats = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_image_formats");
    GLAD_GL_NV_instanced_arrays = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_instanced_arrays");
    GLAD_GL_NV_internalformat_sample_query = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_internalformat_sample_query");
    GLAD_GL_NV_memory_attachment = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_memory_attachment");
    GLAD_GL_NV_non_square_matrices = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_non_square_matrices");
    GLAD_GL_NV_path_rendering = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_path_rendering");
    GLAD_GL_NV_path_rendering_shared_edge = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_path_rendering_shared_edge");
    GLAD_GL_NV_pixel_buffer_object = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_pixel_buffer_object");
    GLAD_GL_NV_polygon_mode = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_polygon_mode");
    GLAD_GL_NV_read_buffer = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_read_buffer");
    GLAD_GL_NV_read_buffer_front = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_read_buffer_front");
    GLAD_GL_NV_read_depth = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_read_depth");
    GLAD_GL_NV_read_depth_stencil = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_read_depth_stencil");
    GLAD_GL_NV_read_stencil = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_read_stencil");
    GLAD_GL_NV_sRGB_formats = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_sRGB_formats");
    GLAD_GL_NV_sample_locations = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_sample_locations");
    GLAD_GL_NV_sample_mask_override_coverage = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_sample_mask_override_coverage");
    GLAD_GL_NV_shader_atomic_fp16_vector = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_shader_atomic_fp16_vector");
    GLAD_GL_NV_shader_noperspective_interpolation = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_shader_noperspective_interpolation");
    GLAD_GL_NV_shadow_samplers_array = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_shadow_samplers_array");
    GLAD_GL_NV_shadow_samplers_cube = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_shadow_samplers_cube");
    GLAD_GL_NV_stereo_view_rendering = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_stereo_view_rendering");
    GLAD_GL_NV_texture_border_clamp = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_texture_border_clamp");
    GLAD_GL_NV_texture_compression_s3tc_update = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_texture_compression_s3tc_update");
    GLAD_GL_NV_texture_npot_2D_mipmap = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_texture_npot_2D_mipmap");
    GLAD_GL_NV_viewport_array = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_viewport_array");
    GLAD_GL_NV_viewport_array2 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_viewport_array2");
    GLAD_GL_NV_viewport_swizzle = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_viewport_swizzle");
    GLAD_GL_OES_EGL_image = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_EGL_image");
    GLAD_GL_OES_EGL_image_external = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_EGL_image_external");
    GLAD_GL_OES_EGL_image_external_essl3 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_EGL_image_external_essl3");
    GLAD_GL_OES_compressed_ETC1_RGB8_sub_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_compressed_ETC1_RGB8_sub_texture");
    GLAD_GL_OES_compressed_ETC1_RGB8_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_compressed_ETC1_RGB8_texture");
    GLAD_GL_OES_compressed_paletted_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_compressed_paletted_texture");
    GLAD_GL_OES_copy_image = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_copy_image");
    GLAD_GL_OES_depth24 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_depth24");
    GLAD_GL_OES_depth32 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_depth32");
    GLAD_GL_OES_depth_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_depth_texture");
    GLAD_GL_OES_draw_buffers_indexed = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_draw_buffers_indexed");
    GLAD_GL_OES_draw_elements_base_vertex = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_draw_elements_base_vertex");
    GLAD_GL_OES_element_index_uint = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_element_index_uint");
    GLAD_GL_OES_fbo_render_mipmap = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_fbo_render_mipmap");
    GLAD_GL_OES_fragment_precision_high = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_fragment_precision_high");
    GLAD_GL_OES_geometry_point_size = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_geometry_point_size");
    GLAD_GL_OES_geometry_shader = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_geometry_shader");
    GLAD_GL_OES_get_program_binary = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_get_program_binary");
    GLAD_GL_OES_gpu_shader5 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_gpu_shader5");
    GLAD_GL_OES_mapbuffer = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_mapbuffer");
    GLAD_GL_OES_packed_depth_stencil = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_packed_depth_stencil");
    GLAD_GL_OES_primitive_bounding_box = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_primitive_bounding_box");
    GLAD_GL_OES_required_internalformat = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_required_internalformat");
    GLAD_GL_OES_rgb8_rgba8 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_rgb8_rgba8");
    GLAD_GL_OES_sample_shading = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_sample_shading");
    GLAD_GL_OES_sample_variables = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_sample_variables");
    GLAD_GL_OES_shader_image_atomic = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_shader_image_atomic");
    GLAD_GL_OES_shader_io_blocks = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_shader_io_blocks");
    GLAD_GL_OES_shader_multisample_interpolation = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_shader_multisample_interpolation");
    GLAD_GL_OES_standard_derivatives = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_standard_derivatives");
    GLAD_GL_OES_stencil1 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_stencil1");
    GLAD_GL_OES_stencil4 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_stencil4");
    GLAD_GL_OES_surfaceless_context = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_surfaceless_context");
    GLAD_GL_OES_tessellation_point_size = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_tessellation_point_size");
    GLAD_GL_OES_tessellation_shader = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_tessellation_shader");
    GLAD_GL_OES_texture_3D = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_texture_3D");
    GLAD_GL_OES_texture_border_clamp = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_texture_border_clamp");
    GLAD_GL_OES_texture_buffer = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_texture_buffer");
    GLAD_GL_OES_texture_compression_astc = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_texture_compression_astc");
    GLAD_GL_OES_texture_cube_map_array = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_texture_cube_map_array");
    GLAD_GL_OES_texture_float = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_texture_float");
    GLAD_GL_OES_texture_float_linear = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_texture_float_linear");
    GLAD_GL_OES_texture_half_float = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_texture_half_float");
    GLAD_GL_OES_texture_half_float_linear = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_texture_half_float_linear");
    GLAD_GL_OES_texture_npot = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_texture_npot");
    GLAD_GL_OES_texture_stencil8 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_texture_stencil8");
    GLAD_GL_OES_texture_storage_multisample_2d_array = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_texture_storage_multisample_2d_array");
    GLAD_GL_OES_texture_view = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_texture_view");
    GLAD_GL_OES_vertex_array_object = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_vertex_array_object");
    GLAD_GL_OES_vertex_half_float = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_vertex_half_float");
    GLAD_GL_OES_vertex_type_10_10_10_2 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_vertex_type_10_10_10_2");
    GLAD_GL_OES_viewport_array = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_viewport_array");
    GLAD_GL_OVR_multiview = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OVR_multiview");
    GLAD_GL_OVR_multiview2 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OVR_multiview2");
    GLAD_GL_OVR_multiview_multisampled_render_to_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OVR_multiview_multisampled_render_to_texture");
    GLAD_GL_QCOM_alpha_test = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_QCOM_alpha_test");
    GLAD_GL_QCOM_binning_control = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_QCOM_binning_control");
    GLAD_GL_QCOM_driver_control = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_QCOM_driver_control");
    GLAD_GL_QCOM_extended_get = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_QCOM_extended_get");
    GLAD_GL_QCOM_extended_get2 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_QCOM_extended_get2");
    GLAD_GL_QCOM_framebuffer_foveated = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_QCOM_framebuffer_foveated");
    GLAD_GL_QCOM_perfmon_global_mode = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_QCOM_perfmon_global_mode");
    GLAD_GL_QCOM_shader_framebuffer_fetch_noncoherent = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_QCOM_shader_framebuffer_fetch_noncoherent");
    GLAD_GL_QCOM_shader_framebuffer_fetch_rate = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_QCOM_shader_framebuffer_fetch_rate");
    GLAD_GL_QCOM_texture_foveated = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_QCOM_texture_foveated");
    GLAD_GL_QCOM_texture_foveated_subsampled_layout = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_QCOM_texture_foveated_subsampled_layout");
    GLAD_GL_QCOM_tiled_rendering = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_QCOM_tiled_rendering");
    GLAD_GL_QCOM_writeonly_rendering = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_QCOM_writeonly_rendering");
    GLAD_GL_VIV_shader_binary = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_VIV_shader_binary");

    glad_gl_free_extensions(exts_i, num_exts_i);

    return 1;
}

static int glad_gl_find_core_gles2(void) {
    int i, major, minor;
    const char* version;
    const char* prefixes[] = {
        "OpenGL ES-CM ",
        "OpenGL ES-CL ",
        "OpenGL ES ",
        NULL
    };
    version = (const char*) glGetString(GL_VERSION);
    if (!version) return 0;
    for (i = 0;  prefixes[i];  i++) {
        const size_t length = strlen(prefixes[i]);
        if (strncmp(version, prefixes[i], length) == 0) {
            version += length;
            break;
        }
    }

    GLAD_IMPL_UTIL_SSCANF(version, "%d.%d", &major, &minor);

    GLAD_GL_ES_VERSION_2_0 = (major == 2 && minor >= 0) || major > 2;
    GLAD_GL_ES_VERSION_3_0 = (major == 3 && minor >= 0) || major > 3;
    GLAD_GL_ES_VERSION_3_1 = (major == 3 && minor >= 1) || major > 3;
    GLAD_GL_ES_VERSION_3_2 = (major == 3 && minor >= 2) || major > 3;

    return GLAD_MAKE_VERSION(major, minor);
}

int gladLoadGLES2UserPtr( GLADuserptrloadfunc load, void *userptr) {
    int version;

    glGetString = (PFNGLGETSTRINGPROC) load("glGetString", userptr);
    if(glGetString == NULL) return 0;
    if(glGetString(GL_VERSION) == NULL) return 0;
    version = glad_gl_find_core_gles2();

    glad_gl_load_GL_ES_VERSION_2_0(load, userptr);
    glad_gl_load_GL_ES_VERSION_3_0(load, userptr);
    glad_gl_load_GL_ES_VERSION_3_1(load, userptr);
    glad_gl_load_GL_ES_VERSION_3_2(load, userptr);

    if (!glad_gl_find_extensions_gles2(version)) return 0;
    glad_gl_load_GL_AMD_framebuffer_multisample_advanced(load, userptr);
    glad_gl_load_GL_AMD_performance_monitor(load, userptr);
    glad_gl_load_GL_ANGLE_framebuffer_blit(load, userptr);
    glad_gl_load_GL_ANGLE_framebuffer_multisample(load, userptr);
    glad_gl_load_GL_ANGLE_instanced_arrays(load, userptr);
    glad_gl_load_GL_ANGLE_translated_shader_source(load, userptr);
    glad_gl_load_GL_APPLE_copy_texture_levels(load, userptr);
    glad_gl_load_GL_APPLE_framebuffer_multisample(load, userptr);
    glad_gl_load_GL_APPLE_sync(load, userptr);
    glad_gl_load_GL_EXT_EGL_image_storage(load, userptr);
    glad_gl_load_GL_EXT_base_instance(load, userptr);
    glad_gl_load_GL_EXT_blend_func_extended(load, userptr);
    glad_gl_load_GL_EXT_buffer_storage(load, userptr);
    glad_gl_load_GL_EXT_clear_texture(load, userptr);
    glad_gl_load_GL_EXT_clip_control(load, userptr);
    glad_gl_load_GL_EXT_copy_image(load, userptr);
    glad_gl_load_GL_EXT_debug_label(load, userptr);
    glad_gl_load_GL_EXT_debug_marker(load, userptr);
    glad_gl_load_GL_EXT_discard_framebuffer(load, userptr);
    glad_gl_load_GL_EXT_disjoint_timer_query(load, userptr);
    glad_gl_load_GL_EXT_draw_buffers(load, userptr);
    glad_gl_load_GL_EXT_draw_buffers_indexed(load, userptr);
    glad_gl_load_GL_EXT_draw_elements_base_vertex(load, userptr);
    glad_gl_load_GL_EXT_draw_instanced(load, userptr);
    glad_gl_load_GL_EXT_draw_transform_feedback(load, userptr);
    glad_gl_load_GL_EXT_external_buffer(load, userptr);
    glad_gl_load_GL_EXT_geometry_shader(load, userptr);
    glad_gl_load_GL_EXT_instanced_arrays(load, userptr);
    glad_gl_load_GL_EXT_map_buffer_range(load, userptr);
    glad_gl_load_GL_EXT_memory_object(load, userptr);
    glad_gl_load_GL_EXT_memory_object_fd(load, userptr);
    glad_gl_load_GL_EXT_memory_object_win32(load, userptr);
    glad_gl_load_GL_EXT_multi_draw_arrays(load, userptr);
    glad_gl_load_GL_EXT_multi_draw_indirect(load, userptr);
    glad_gl_load_GL_EXT_multisampled_render_to_texture(load, userptr);
    glad_gl_load_GL_EXT_multiview_draw_buffers(load, userptr);
    glad_gl_load_GL_EXT_occlusion_query_boolean(load, userptr);
    glad_gl_load_GL_EXT_polygon_offset_clamp(load, userptr);
    glad_gl_load_GL_EXT_primitive_bounding_box(load, userptr);
    glad_gl_load_GL_EXT_raster_multisample(load, userptr);
    glad_gl_load_GL_EXT_robustness(load, userptr);
    glad_gl_load_GL_EXT_semaphore(load, userptr);
    glad_gl_load_GL_EXT_semaphore_fd(load, userptr);
    glad_gl_load_GL_EXT_semaphore_win32(load, userptr);
    glad_gl_load_GL_EXT_separate_shader_objects(load, userptr);
    glad_gl_load_GL_EXT_shader_framebuffer_fetch_non_coherent(load, userptr);
    glad_gl_load_GL_EXT_shader_pixel_local_storage2(load, userptr);
    glad_gl_load_GL_EXT_sparse_texture(load, userptr);
    glad_gl_load_GL_EXT_tessellation_shader(load, userptr);
    glad_gl_load_GL_EXT_texture_border_clamp(load, userptr);
    glad_gl_load_GL_EXT_texture_buffer(load, userptr);
    glad_gl_load_GL_EXT_texture_storage(load, userptr);
    glad_gl_load_GL_EXT_texture_view(load, userptr);
    glad_gl_load_GL_EXT_win32_keyed_mutex(load, userptr);
    glad_gl_load_GL_EXT_window_rectangles(load, userptr);
    glad_gl_load_GL_IMG_bindless_texture(load, userptr);
    glad_gl_load_GL_IMG_framebuffer_downsample(load, userptr);
    glad_gl_load_GL_IMG_multisampled_render_to_texture(load, userptr);
    glad_gl_load_GL_INTEL_framebuffer_CMAA(load, userptr);
    glad_gl_load_GL_INTEL_performance_query(load, userptr);
    glad_gl_load_GL_KHR_blend_equation_advanced(load, userptr);
    glad_gl_load_GL_KHR_debug(load, userptr);
    glad_gl_load_GL_KHR_parallel_shader_compile(load, userptr);
    glad_gl_load_GL_KHR_robustness(load, userptr);
    glad_gl_load_GL_NV_bindless_texture(load, userptr);
    glad_gl_load_GL_NV_blend_equation_advanced(load, userptr);
    glad_gl_load_GL_NV_clip_space_w_scaling(load, userptr);
    glad_gl_load_GL_NV_conditional_render(load, userptr);
    glad_gl_load_GL_NV_conservative_raster(load, userptr);
    glad_gl_load_GL_NV_conservative_raster_pre_snap_triangles(load, userptr);
    glad_gl_load_GL_NV_copy_buffer(load, userptr);
    glad_gl_load_GL_NV_coverage_sample(load, userptr);
    glad_gl_load_GL_NV_draw_buffers(load, userptr);
    glad_gl_load_GL_NV_draw_instanced(load, userptr);
    glad_gl_load_GL_NV_draw_vulkan_image(load, userptr);
    glad_gl_load_GL_NV_fence(load, userptr);
    glad_gl_load_GL_NV_fragment_coverage_to_color(load, userptr);
    glad_gl_load_GL_NV_framebuffer_blit(load, userptr);
    glad_gl_load_GL_NV_framebuffer_mixed_samples(load, userptr);
    glad_gl_load_GL_NV_framebuffer_multisample(load, userptr);
    glad_gl_load_GL_NV_gpu_shader5(load, userptr);
    glad_gl_load_GL_NV_instanced_arrays(load, userptr);
    glad_gl_load_GL_NV_internalformat_sample_query(load, userptr);
    glad_gl_load_GL_NV_memory_attachment(load, userptr);
    glad_gl_load_GL_NV_non_square_matrices(load, userptr);
    glad_gl_load_GL_NV_path_rendering(load, userptr);
    glad_gl_load_GL_NV_polygon_mode(load, userptr);
    glad_gl_load_GL_NV_read_buffer(load, userptr);
    glad_gl_load_GL_NV_sample_locations(load, userptr);
    glad_gl_load_GL_NV_viewport_array(load, userptr);
    glad_gl_load_GL_NV_viewport_swizzle(load, userptr);
    glad_gl_load_GL_OES_EGL_image(load, userptr);
    glad_gl_load_GL_OES_copy_image(load, userptr);
    glad_gl_load_GL_OES_draw_buffers_indexed(load, userptr);
    glad_gl_load_GL_OES_draw_elements_base_vertex(load, userptr);
    glad_gl_load_GL_OES_geometry_shader(load, userptr);
    glad_gl_load_GL_OES_get_program_binary(load, userptr);
    glad_gl_load_GL_OES_mapbuffer(load, userptr);
    glad_gl_load_GL_OES_primitive_bounding_box(load, userptr);
    glad_gl_load_GL_OES_sample_shading(load, userptr);
    glad_gl_load_GL_OES_tessellation_shader(load, userptr);
    glad_gl_load_GL_OES_texture_3D(load, userptr);
    glad_gl_load_GL_OES_texture_border_clamp(load, userptr);
    glad_gl_load_GL_OES_texture_buffer(load, userptr);
    glad_gl_load_GL_OES_texture_storage_multisample_2d_array(load, userptr);
    glad_gl_load_GL_OES_texture_view(load, userptr);
    glad_gl_load_GL_OES_vertex_array_object(load, userptr);
    glad_gl_load_GL_OES_viewport_array(load, userptr);
    glad_gl_load_GL_OVR_multiview(load, userptr);
    glad_gl_load_GL_OVR_multiview_multisampled_render_to_texture(load, userptr);
    glad_gl_load_GL_QCOM_alpha_test(load, userptr);
    glad_gl_load_GL_QCOM_driver_control(load, userptr);
    glad_gl_load_GL_QCOM_extended_get(load, userptr);
    glad_gl_load_GL_QCOM_extended_get2(load, userptr);
    glad_gl_load_GL_QCOM_framebuffer_foveated(load, userptr);
    glad_gl_load_GL_QCOM_shader_framebuffer_fetch_noncoherent(load, userptr);
    glad_gl_load_GL_QCOM_texture_foveated(load, userptr);
    glad_gl_load_GL_QCOM_tiled_rendering(load, userptr);



    return version;
}


int gladLoadGLES2( GLADloadfunc load) {
    return gladLoadGLES2UserPtr( glad_gl_get_proc_from_userptr, GLAD_GNUC_EXTENSION (void*) load);
}




#ifdef GLAD_GLES2

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

#if GLAD_PLATFORM_EMSCRIPTEN
  typedef void* (GLAD_API_PTR *PFNEGLGETPROCADDRESSPROC)(const char *name);
  extern void* emscripten_GetProcAddress(const char *name);
#else
  #include <glad/egl.h>
#endif


struct _glad_gles2_userptr {
    void *handle;
    PFNEGLGETPROCADDRESSPROC get_proc_address_ptr;
};


static GLADapiproc glad_gles2_get_proc(const char* name, void *vuserptr) {
    struct _glad_gles2_userptr userptr = *(struct _glad_gles2_userptr*) vuserptr;
    GLADapiproc result = NULL;

#if !GLAD_PLATFORM_EMSCRIPTEN
    result = glad_dlsym_handle(userptr.handle, name);
#endif
    if (result == NULL) {
        result = userptr.get_proc_address_ptr(name);
    }

    return result;
}

static void* _gles2_handle = NULL;

int gladLoaderLoadGLES2(void) {
#if GLAD_PLATFORM_EMSCRIPTEN
#elif GLAD_PLATFORM_APPLE
    static const char *NAMES[] = {"libGLESv2.dylib"};
#elif GLAD_PLATFORM_WIN32
    static const char *NAMES[] = {"GLESv2.dll", "libGLESv2.dll"};
#else
    static const char *NAMES[] = {"libGLESv2.so.2", "libGLESv2.so"};
#endif

    int version = 0;
    int did_load = 0;
    struct _glad_gles2_userptr userptr;

#if GLAD_PLATFORM_EMSCRIPTEN
    userptr.get_proc_address_ptr = emscripten_GetProcAddress;
    version = gladLoadGLES2UserPtr(glad_gles2_get_proc, &userptr);
#else
    if (eglGetProcAddress == NULL) {
        return 0;
    }

    if (_gles2_handle == NULL) {
        _gles2_handle = glad_get_dlopen_handle(NAMES, sizeof(NAMES) / sizeof(NAMES[0]));
        did_load = _gles2_handle != NULL;
    }

    if (_gles2_handle != NULL) {
        userptr.handle = _gles2_handle;
        userptr.get_proc_address_ptr = eglGetProcAddress;

        version = gladLoadGLES2UserPtr(glad_gles2_get_proc, &userptr);

        if (!version && did_load) {
            glad_close_dlopen_handle(_gles2_handle);
            _gles2_handle = NULL;
        }
    }
#endif

    return version;
}


void gladLoaderUnloadGLES2(void) {
    if (_gles2_handle != NULL) {
        glad_close_dlopen_handle(_gles2_handle);
        _gles2_handle = NULL;
    }
}

#endif /* GLAD_GLES2 */
