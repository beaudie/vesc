#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/gles1.h>

#ifndef GLAD_IMPL_UTIL_C_
#define GLAD_IMPL_UTIL_C_

#ifdef _MSC_VER
#define GLAD_IMPL_UTIL_SSCANF sscanf_s
#else
#define GLAD_IMPL_UTIL_SSCANF sscanf
#endif

#endif /* GLAD_IMPL_UTIL_C_ */


int GLAD_GL_VERSION_ES_CM_1_0 = 0;
int GLAD_GL_AMD_compressed_3DC_texture = 0;
int GLAD_GL_AMD_compressed_ATC_texture = 0;
int GLAD_GL_APPLE_copy_texture_levels = 0;
int GLAD_GL_APPLE_framebuffer_multisample = 0;
int GLAD_GL_APPLE_sync = 0;
int GLAD_GL_APPLE_texture_2D_limited_npot = 0;
int GLAD_GL_APPLE_texture_format_BGRA8888 = 0;
int GLAD_GL_APPLE_texture_max_level = 0;
int GLAD_GL_ARM_rgba8 = 0;
int GLAD_GL_EXT_blend_minmax = 0;
int GLAD_GL_EXT_debug_marker = 0;
int GLAD_GL_EXT_discard_framebuffer = 0;
int GLAD_GL_EXT_map_buffer_range = 0;
int GLAD_GL_EXT_multi_draw_arrays = 0;
int GLAD_GL_EXT_multisampled_render_to_texture = 0;
int GLAD_GL_EXT_read_format_bgra = 0;
int GLAD_GL_EXT_robustness = 0;
int GLAD_GL_EXT_sRGB = 0;
int GLAD_GL_EXT_texture_compression_dxt1 = 0;
int GLAD_GL_EXT_texture_filter_anisotropic = 0;
int GLAD_GL_EXT_texture_format_BGRA8888 = 0;
int GLAD_GL_EXT_texture_lod_bias = 0;
int GLAD_GL_EXT_texture_storage = 0;
int GLAD_GL_IMG_multisampled_render_to_texture = 0;
int GLAD_GL_IMG_read_format = 0;
int GLAD_GL_IMG_texture_compression_pvrtc = 0;
int GLAD_GL_IMG_texture_env_enhanced_fixed_function = 0;
int GLAD_GL_IMG_user_clip_plane = 0;
int GLAD_GL_KHR_debug = 0;
int GLAD_GL_NV_fence = 0;
int GLAD_GL_OES_EGL_image = 0;
int GLAD_GL_OES_EGL_image_external = 0;
int GLAD_GL_OES_blend_equation_separate = 0;
int GLAD_GL_OES_blend_func_separate = 0;
int GLAD_GL_OES_blend_subtract = 0;
int GLAD_GL_OES_byte_coordinates = 0;
int GLAD_GL_OES_compressed_ETC1_RGB8_sub_texture = 0;
int GLAD_GL_OES_compressed_ETC1_RGB8_texture = 0;
int GLAD_GL_OES_compressed_paletted_texture = 0;
int GLAD_GL_OES_depth24 = 0;
int GLAD_GL_OES_depth32 = 0;
int GLAD_GL_OES_draw_texture = 0;
int GLAD_GL_OES_element_index_uint = 0;
int GLAD_GL_OES_extended_matrix_palette = 0;
int GLAD_GL_OES_fbo_render_mipmap = 0;
int GLAD_GL_OES_fixed_point = 0;
int GLAD_GL_OES_framebuffer_object = 0;
int GLAD_GL_OES_mapbuffer = 0;
int GLAD_GL_OES_matrix_get = 0;
int GLAD_GL_OES_matrix_palette = 0;
int GLAD_GL_OES_packed_depth_stencil = 0;
int GLAD_GL_OES_point_size_array = 0;
int GLAD_GL_OES_point_sprite = 0;
int GLAD_GL_OES_query_matrix = 0;
int GLAD_GL_OES_read_format = 0;
int GLAD_GL_OES_required_internalformat = 0;
int GLAD_GL_OES_rgb8_rgba8 = 0;
int GLAD_GL_OES_single_precision = 0;
int GLAD_GL_OES_stencil1 = 0;
int GLAD_GL_OES_stencil4 = 0;
int GLAD_GL_OES_stencil8 = 0;
int GLAD_GL_OES_stencil_wrap = 0;
int GLAD_GL_OES_surfaceless_context = 0;
int GLAD_GL_OES_texture_cube_map = 0;
int GLAD_GL_OES_texture_env_crossbar = 0;
int GLAD_GL_OES_texture_mirrored_repeat = 0;
int GLAD_GL_OES_texture_npot = 0;
int GLAD_GL_OES_vertex_array_object = 0;
int GLAD_GL_QCOM_driver_control = 0;
int GLAD_GL_QCOM_extended_get = 0;
int GLAD_GL_QCOM_extended_get2 = 0;
int GLAD_GL_QCOM_perfmon_global_mode = 0;
int GLAD_GL_QCOM_tiled_rendering = 0;
int GLAD_GL_QCOM_writeonly_rendering = 0;



PFNGLACTIVETEXTUREPROC glad_glActiveTexture = NULL;
PFNGLALPHAFUNCPROC glad_glAlphaFunc = NULL;
PFNGLALPHAFUNCXPROC glad_glAlphaFuncx = NULL;
PFNGLALPHAFUNCXOESPROC glad_glAlphaFuncxOES = NULL;
PFNGLBINDBUFFERPROC glad_glBindBuffer = NULL;
PFNGLBINDFRAMEBUFFEROESPROC glad_glBindFramebufferOES = NULL;
PFNGLBINDRENDERBUFFEROESPROC glad_glBindRenderbufferOES = NULL;
PFNGLBINDTEXTUREPROC glad_glBindTexture = NULL;
PFNGLBINDVERTEXARRAYPROC glad_glBindVertexArray = NULL;
PFNGLBINDVERTEXARRAYOESPROC glad_glBindVertexArrayOES = NULL;
PFNGLBLENDEQUATIONOESPROC glad_glBlendEquationOES = NULL;
PFNGLBLENDEQUATIONSEPARATEOESPROC glad_glBlendEquationSeparateOES = NULL;
PFNGLBLENDFUNCPROC glad_glBlendFunc = NULL;
PFNGLBLENDFUNCSEPARATEOESPROC glad_glBlendFuncSeparateOES = NULL;
PFNGLBUFFERDATAPROC glad_glBufferData = NULL;
PFNGLBUFFERSUBDATAPROC glad_glBufferSubData = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSOESPROC glad_glCheckFramebufferStatusOES = NULL;
PFNGLCLEARPROC glad_glClear = NULL;
PFNGLCLEARCOLORPROC glad_glClearColor = NULL;
PFNGLCLEARCOLORXPROC glad_glClearColorx = NULL;
PFNGLCLEARCOLORXOESPROC glad_glClearColorxOES = NULL;
PFNGLCLEARDEPTHFPROC glad_glClearDepthf = NULL;
PFNGLCLEARDEPTHFOESPROC glad_glClearDepthfOES = NULL;
PFNGLCLEARDEPTHXPROC glad_glClearDepthx = NULL;
PFNGLCLEARDEPTHXOESPROC glad_glClearDepthxOES = NULL;
PFNGLCLEARSTENCILPROC glad_glClearStencil = NULL;
PFNGLCLIENTACTIVETEXTUREPROC glad_glClientActiveTexture = NULL;
PFNGLCLIENTWAITSYNCPROC glad_glClientWaitSync = NULL;
PFNGLCLIENTWAITSYNCAPPLEPROC glad_glClientWaitSyncAPPLE = NULL;
PFNGLCLIPPLANEFPROC glad_glClipPlanef = NULL;
PFNGLCLIPPLANEFIMGPROC glad_glClipPlanefIMG = NULL;
PFNGLCLIPPLANEFOESPROC glad_glClipPlanefOES = NULL;
PFNGLCLIPPLANEXPROC glad_glClipPlanex = NULL;
PFNGLCLIPPLANEXIMGPROC glad_glClipPlanexIMG = NULL;
PFNGLCLIPPLANEXOESPROC glad_glClipPlanexOES = NULL;
PFNGLCOLOR4FPROC glad_glColor4f = NULL;
PFNGLCOLOR4UBPROC glad_glColor4ub = NULL;
PFNGLCOLOR4XPROC glad_glColor4x = NULL;
PFNGLCOLOR4XOESPROC glad_glColor4xOES = NULL;
PFNGLCOLORMASKPROC glad_glColorMask = NULL;
PFNGLCOLORPOINTERPROC glad_glColorPointer = NULL;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glad_glCompressedTexImage2D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glad_glCompressedTexSubImage2D = NULL;
PFNGLCOPYTEXIMAGE2DPROC glad_glCopyTexImage2D = NULL;
PFNGLCOPYTEXSUBIMAGE2DPROC glad_glCopyTexSubImage2D = NULL;
PFNGLCOPYTEXTURELEVELSAPPLEPROC glad_glCopyTextureLevelsAPPLE = NULL;
PFNGLCULLFACEPROC glad_glCullFace = NULL;
PFNGLCURRENTPALETTEMATRIXOESPROC glad_glCurrentPaletteMatrixOES = NULL;
PFNGLDELETEBUFFERSPROC glad_glDeleteBuffers = NULL;
PFNGLDELETEFENCESNVPROC glad_glDeleteFencesNV = NULL;
PFNGLDELETEFRAMEBUFFERSOESPROC glad_glDeleteFramebuffersOES = NULL;
PFNGLDELETERENDERBUFFERSOESPROC glad_glDeleteRenderbuffersOES = NULL;
PFNGLDELETESYNCPROC glad_glDeleteSync = NULL;
PFNGLDELETESYNCAPPLEPROC glad_glDeleteSyncAPPLE = NULL;
PFNGLDELETETEXTURESPROC glad_glDeleteTextures = NULL;
PFNGLDELETEVERTEXARRAYSPROC glad_glDeleteVertexArrays = NULL;
PFNGLDELETEVERTEXARRAYSOESPROC glad_glDeleteVertexArraysOES = NULL;
PFNGLDEPTHFUNCPROC glad_glDepthFunc = NULL;
PFNGLDEPTHMASKPROC glad_glDepthMask = NULL;
PFNGLDEPTHRANGEFPROC glad_glDepthRangef = NULL;
PFNGLDEPTHRANGEFOESPROC glad_glDepthRangefOES = NULL;
PFNGLDEPTHRANGEXPROC glad_glDepthRangex = NULL;
PFNGLDEPTHRANGEXOESPROC glad_glDepthRangexOES = NULL;
PFNGLDISABLEPROC glad_glDisable = NULL;
PFNGLDISABLECLIENTSTATEPROC glad_glDisableClientState = NULL;
PFNGLDISABLEDRIVERCONTROLQCOMPROC glad_glDisableDriverControlQCOM = NULL;
PFNGLDISCARDFRAMEBUFFEREXTPROC glad_glDiscardFramebufferEXT = NULL;
PFNGLDRAWARRAYSPROC glad_glDrawArrays = NULL;
PFNGLDRAWELEMENTSPROC glad_glDrawElements = NULL;
PFNGLDRAWTEXFOESPROC glad_glDrawTexfOES = NULL;
PFNGLDRAWTEXFVOESPROC glad_glDrawTexfvOES = NULL;
PFNGLDRAWTEXIOESPROC glad_glDrawTexiOES = NULL;
PFNGLDRAWTEXIVOESPROC glad_glDrawTexivOES = NULL;
PFNGLDRAWTEXSOESPROC glad_glDrawTexsOES = NULL;
PFNGLDRAWTEXSVOESPROC glad_glDrawTexsvOES = NULL;
PFNGLDRAWTEXXOESPROC glad_glDrawTexxOES = NULL;
PFNGLDRAWTEXXVOESPROC glad_glDrawTexxvOES = NULL;
PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC glad_glEGLImageTargetRenderbufferStorageOES = NULL;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glad_glEGLImageTargetTexture2DOES = NULL;
PFNGLENABLEPROC glad_glEnable = NULL;
PFNGLENABLECLIENTSTATEPROC glad_glEnableClientState = NULL;
PFNGLENABLEDRIVERCONTROLQCOMPROC glad_glEnableDriverControlQCOM = NULL;
PFNGLENDTILINGQCOMPROC glad_glEndTilingQCOM = NULL;
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
PFNGLFOGFPROC glad_glFogf = NULL;
PFNGLFOGFVPROC glad_glFogfv = NULL;
PFNGLFOGXPROC glad_glFogx = NULL;
PFNGLFOGXOESPROC glad_glFogxOES = NULL;
PFNGLFOGXVPROC glad_glFogxv = NULL;
PFNGLFOGXVOESPROC glad_glFogxvOES = NULL;
PFNGLFRAMEBUFFERRENDERBUFFEROESPROC glad_glFramebufferRenderbufferOES = NULL;
PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glad_glFramebufferTexture2DMultisampleEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC glad_glFramebufferTexture2DMultisampleIMG = NULL;
PFNGLFRAMEBUFFERTEXTURE2DOESPROC glad_glFramebufferTexture2DOES = NULL;
PFNGLFRONTFACEPROC glad_glFrontFace = NULL;
PFNGLFRUSTUMFPROC glad_glFrustumf = NULL;
PFNGLFRUSTUMFOESPROC glad_glFrustumfOES = NULL;
PFNGLFRUSTUMXPROC glad_glFrustumx = NULL;
PFNGLFRUSTUMXOESPROC glad_glFrustumxOES = NULL;
PFNGLGENBUFFERSPROC glad_glGenBuffers = NULL;
PFNGLGENFENCESNVPROC glad_glGenFencesNV = NULL;
PFNGLGENFRAMEBUFFERSOESPROC glad_glGenFramebuffersOES = NULL;
PFNGLGENRENDERBUFFERSOESPROC glad_glGenRenderbuffersOES = NULL;
PFNGLGENTEXTURESPROC glad_glGenTextures = NULL;
PFNGLGENVERTEXARRAYSPROC glad_glGenVertexArrays = NULL;
PFNGLGENVERTEXARRAYSOESPROC glad_glGenVertexArraysOES = NULL;
PFNGLGENERATEMIPMAPOESPROC glad_glGenerateMipmapOES = NULL;
PFNGLGETBOOLEANVPROC glad_glGetBooleanv = NULL;
PFNGLGETBUFFERPARAMETERIVPROC glad_glGetBufferParameteriv = NULL;
PFNGLGETBUFFERPOINTERVPROC glad_glGetBufferPointerv = NULL;
PFNGLGETBUFFERPOINTERVOESPROC glad_glGetBufferPointervOES = NULL;
PFNGLGETCLIPPLANEFPROC glad_glGetClipPlanef = NULL;
PFNGLGETCLIPPLANEFOESPROC glad_glGetClipPlanefOES = NULL;
PFNGLGETCLIPPLANEXPROC glad_glGetClipPlanex = NULL;
PFNGLGETCLIPPLANEXOESPROC glad_glGetClipPlanexOES = NULL;
PFNGLGETDRIVERCONTROLSTRINGQCOMPROC glad_glGetDriverControlStringQCOM = NULL;
PFNGLGETDRIVERCONTROLSQCOMPROC glad_glGetDriverControlsQCOM = NULL;
PFNGLGETERRORPROC glad_glGetError = NULL;
PFNGLGETFENCEIVNVPROC glad_glGetFenceivNV = NULL;
PFNGLGETFIXEDVPROC glad_glGetFixedv = NULL;
PFNGLGETFIXEDVOESPROC glad_glGetFixedvOES = NULL;
PFNGLGETFLOATVPROC glad_glGetFloatv = NULL;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOESPROC glad_glGetFramebufferAttachmentParameterivOES = NULL;
PFNGLGETGRAPHICSRESETSTATUSPROC glad_glGetGraphicsResetStatus = NULL;
PFNGLGETGRAPHICSRESETSTATUSEXTPROC glad_glGetGraphicsResetStatusEXT = NULL;
PFNGLGETINTEGER64VPROC glad_glGetInteger64v = NULL;
PFNGLGETINTEGER64VAPPLEPROC glad_glGetInteger64vAPPLE = NULL;
PFNGLGETINTEGERVPROC glad_glGetIntegerv = NULL;
PFNGLGETLIGHTFVPROC glad_glGetLightfv = NULL;
PFNGLGETLIGHTXVPROC glad_glGetLightxv = NULL;
PFNGLGETLIGHTXVOESPROC glad_glGetLightxvOES = NULL;
PFNGLGETMATERIALFVPROC glad_glGetMaterialfv = NULL;
PFNGLGETMATERIALXVPROC glad_glGetMaterialxv = NULL;
PFNGLGETMATERIALXVOESPROC glad_glGetMaterialxvOES = NULL;
PFNGLGETPOINTERVPROC glad_glGetPointerv = NULL;
PFNGLGETRENDERBUFFERPARAMETERIVOESPROC glad_glGetRenderbufferParameterivOES = NULL;
PFNGLGETSTRINGPROC glad_glGetString = NULL;
PFNGLGETSYNCIVPROC glad_glGetSynciv = NULL;
PFNGLGETSYNCIVAPPLEPROC glad_glGetSyncivAPPLE = NULL;
PFNGLGETTEXENVFVPROC glad_glGetTexEnvfv = NULL;
PFNGLGETTEXENVIVPROC glad_glGetTexEnviv = NULL;
PFNGLGETTEXENVXVPROC glad_glGetTexEnvxv = NULL;
PFNGLGETTEXENVXVOESPROC glad_glGetTexEnvxvOES = NULL;
PFNGLGETTEXGENFVOESPROC glad_glGetTexGenfvOES = NULL;
PFNGLGETTEXGENIVOESPROC glad_glGetTexGenivOES = NULL;
PFNGLGETTEXGENXVOESPROC glad_glGetTexGenxvOES = NULL;
PFNGLGETTEXPARAMETERFVPROC glad_glGetTexParameterfv = NULL;
PFNGLGETTEXPARAMETERIVPROC glad_glGetTexParameteriv = NULL;
PFNGLGETTEXPARAMETERXVPROC glad_glGetTexParameterxv = NULL;
PFNGLGETTEXPARAMETERXVOESPROC glad_glGetTexParameterxvOES = NULL;
PFNGLGETNUNIFORMFVPROC glad_glGetnUniformfv = NULL;
PFNGLGETNUNIFORMFVEXTPROC glad_glGetnUniformfvEXT = NULL;
PFNGLGETNUNIFORMIVPROC glad_glGetnUniformiv = NULL;
PFNGLGETNUNIFORMIVEXTPROC glad_glGetnUniformivEXT = NULL;
PFNGLHINTPROC glad_glHint = NULL;
PFNGLINSERTEVENTMARKEREXTPROC glad_glInsertEventMarkerEXT = NULL;
PFNGLISBUFFERPROC glad_glIsBuffer = NULL;
PFNGLISENABLEDPROC glad_glIsEnabled = NULL;
PFNGLISFENCENVPROC glad_glIsFenceNV = NULL;
PFNGLISFRAMEBUFFEROESPROC glad_glIsFramebufferOES = NULL;
PFNGLISRENDERBUFFEROESPROC glad_glIsRenderbufferOES = NULL;
PFNGLISSYNCPROC glad_glIsSync = NULL;
PFNGLISSYNCAPPLEPROC glad_glIsSyncAPPLE = NULL;
PFNGLISTEXTUREPROC glad_glIsTexture = NULL;
PFNGLISVERTEXARRAYPROC glad_glIsVertexArray = NULL;
PFNGLISVERTEXARRAYOESPROC glad_glIsVertexArrayOES = NULL;
PFNGLLIGHTMODELFPROC glad_glLightModelf = NULL;
PFNGLLIGHTMODELFVPROC glad_glLightModelfv = NULL;
PFNGLLIGHTMODELXPROC glad_glLightModelx = NULL;
PFNGLLIGHTMODELXOESPROC glad_glLightModelxOES = NULL;
PFNGLLIGHTMODELXVPROC glad_glLightModelxv = NULL;
PFNGLLIGHTMODELXVOESPROC glad_glLightModelxvOES = NULL;
PFNGLLIGHTFPROC glad_glLightf = NULL;
PFNGLLIGHTFVPROC glad_glLightfv = NULL;
PFNGLLIGHTXPROC glad_glLightx = NULL;
PFNGLLIGHTXOESPROC glad_glLightxOES = NULL;
PFNGLLIGHTXVPROC glad_glLightxv = NULL;
PFNGLLIGHTXVOESPROC glad_glLightxvOES = NULL;
PFNGLLINEWIDTHPROC glad_glLineWidth = NULL;
PFNGLLINEWIDTHXPROC glad_glLineWidthx = NULL;
PFNGLLINEWIDTHXOESPROC glad_glLineWidthxOES = NULL;
PFNGLLOADIDENTITYPROC glad_glLoadIdentity = NULL;
PFNGLLOADMATRIXFPROC glad_glLoadMatrixf = NULL;
PFNGLLOADMATRIXXPROC glad_glLoadMatrixx = NULL;
PFNGLLOADMATRIXXOESPROC glad_glLoadMatrixxOES = NULL;
PFNGLLOADPALETTEFROMMODELVIEWMATRIXOESPROC glad_glLoadPaletteFromModelViewMatrixOES = NULL;
PFNGLLOGICOPPROC glad_glLogicOp = NULL;
PFNGLMAPBUFFERPROC glad_glMapBuffer = NULL;
PFNGLMAPBUFFEROESPROC glad_glMapBufferOES = NULL;
PFNGLMAPBUFFERRANGEPROC glad_glMapBufferRange = NULL;
PFNGLMAPBUFFERRANGEEXTPROC glad_glMapBufferRangeEXT = NULL;
PFNGLMATERIALFPROC glad_glMaterialf = NULL;
PFNGLMATERIALFVPROC glad_glMaterialfv = NULL;
PFNGLMATERIALXPROC glad_glMaterialx = NULL;
PFNGLMATERIALXOESPROC glad_glMaterialxOES = NULL;
PFNGLMATERIALXVPROC glad_glMaterialxv = NULL;
PFNGLMATERIALXVOESPROC glad_glMaterialxvOES = NULL;
PFNGLMATRIXINDEXPOINTEROESPROC glad_glMatrixIndexPointerOES = NULL;
PFNGLMATRIXMODEPROC glad_glMatrixMode = NULL;
PFNGLMULTMATRIXFPROC glad_glMultMatrixf = NULL;
PFNGLMULTMATRIXXPROC glad_glMultMatrixx = NULL;
PFNGLMULTMATRIXXOESPROC glad_glMultMatrixxOES = NULL;
PFNGLMULTIDRAWARRAYSPROC glad_glMultiDrawArrays = NULL;
PFNGLMULTIDRAWARRAYSEXTPROC glad_glMultiDrawArraysEXT = NULL;
PFNGLMULTIDRAWELEMENTSPROC glad_glMultiDrawElements = NULL;
PFNGLMULTIDRAWELEMENTSEXTPROC glad_glMultiDrawElementsEXT = NULL;
PFNGLMULTITEXCOORD4FPROC glad_glMultiTexCoord4f = NULL;
PFNGLMULTITEXCOORD4XPROC glad_glMultiTexCoord4x = NULL;
PFNGLMULTITEXCOORD4XOESPROC glad_glMultiTexCoord4xOES = NULL;
PFNGLNORMAL3FPROC glad_glNormal3f = NULL;
PFNGLNORMAL3XPROC glad_glNormal3x = NULL;
PFNGLNORMAL3XOESPROC glad_glNormal3xOES = NULL;
PFNGLNORMALPOINTERPROC glad_glNormalPointer = NULL;
PFNGLORTHOFPROC glad_glOrthof = NULL;
PFNGLORTHOFOESPROC glad_glOrthofOES = NULL;
PFNGLORTHOXPROC glad_glOrthox = NULL;
PFNGLORTHOXOESPROC glad_glOrthoxOES = NULL;
PFNGLPIXELSTOREIPROC glad_glPixelStorei = NULL;
PFNGLPOINTPARAMETERFPROC glad_glPointParameterf = NULL;
PFNGLPOINTPARAMETERFVPROC glad_glPointParameterfv = NULL;
PFNGLPOINTPARAMETERXPROC glad_glPointParameterx = NULL;
PFNGLPOINTPARAMETERXOESPROC glad_glPointParameterxOES = NULL;
PFNGLPOINTPARAMETERXVPROC glad_glPointParameterxv = NULL;
PFNGLPOINTPARAMETERXVOESPROC glad_glPointParameterxvOES = NULL;
PFNGLPOINTSIZEPROC glad_glPointSize = NULL;
PFNGLPOINTSIZEPOINTEROESPROC glad_glPointSizePointerOES = NULL;
PFNGLPOINTSIZEXPROC glad_glPointSizex = NULL;
PFNGLPOINTSIZEXOESPROC glad_glPointSizexOES = NULL;
PFNGLPOLYGONOFFSETPROC glad_glPolygonOffset = NULL;
PFNGLPOLYGONOFFSETXPROC glad_glPolygonOffsetx = NULL;
PFNGLPOLYGONOFFSETXOESPROC glad_glPolygonOffsetxOES = NULL;
PFNGLPOPGROUPMARKEREXTPROC glad_glPopGroupMarkerEXT = NULL;
PFNGLPOPMATRIXPROC glad_glPopMatrix = NULL;
PFNGLPUSHGROUPMARKEREXTPROC glad_glPushGroupMarkerEXT = NULL;
PFNGLPUSHMATRIXPROC glad_glPushMatrix = NULL;
PFNGLQUERYMATRIXXOESPROC glad_glQueryMatrixxOES = NULL;
PFNGLREADPIXELSPROC glad_glReadPixels = NULL;
PFNGLREADNPIXELSPROC glad_glReadnPixels = NULL;
PFNGLREADNPIXELSEXTPROC glad_glReadnPixelsEXT = NULL;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glad_glRenderbufferStorageMultisample = NULL;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEAPPLEPROC glad_glRenderbufferStorageMultisampleAPPLE = NULL;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glad_glRenderbufferStorageMultisampleEXT = NULL;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMGPROC glad_glRenderbufferStorageMultisampleIMG = NULL;
PFNGLRENDERBUFFERSTORAGEOESPROC glad_glRenderbufferStorageOES = NULL;
PFNGLRESOLVEMULTISAMPLEFRAMEBUFFERAPPLEPROC glad_glResolveMultisampleFramebufferAPPLE = NULL;
PFNGLROTATEFPROC glad_glRotatef = NULL;
PFNGLROTATEXPROC glad_glRotatex = NULL;
PFNGLROTATEXOESPROC glad_glRotatexOES = NULL;
PFNGLSAMPLECOVERAGEPROC glad_glSampleCoverage = NULL;
PFNGLSAMPLECOVERAGEXPROC glad_glSampleCoveragex = NULL;
PFNGLSAMPLECOVERAGEXOESPROC glad_glSampleCoveragexOES = NULL;
PFNGLSCALEFPROC glad_glScalef = NULL;
PFNGLSCALEXPROC glad_glScalex = NULL;
PFNGLSCALEXOESPROC glad_glScalexOES = NULL;
PFNGLSCISSORPROC glad_glScissor = NULL;
PFNGLSETFENCENVPROC glad_glSetFenceNV = NULL;
PFNGLSHADEMODELPROC glad_glShadeModel = NULL;
PFNGLSTARTTILINGQCOMPROC glad_glStartTilingQCOM = NULL;
PFNGLSTENCILFUNCPROC glad_glStencilFunc = NULL;
PFNGLSTENCILMASKPROC glad_glStencilMask = NULL;
PFNGLSTENCILOPPROC glad_glStencilOp = NULL;
PFNGLTESTFENCENVPROC glad_glTestFenceNV = NULL;
PFNGLTEXCOORDPOINTERPROC glad_glTexCoordPointer = NULL;
PFNGLTEXENVFPROC glad_glTexEnvf = NULL;
PFNGLTEXENVFVPROC glad_glTexEnvfv = NULL;
PFNGLTEXENVIPROC glad_glTexEnvi = NULL;
PFNGLTEXENVIVPROC glad_glTexEnviv = NULL;
PFNGLTEXENVXPROC glad_glTexEnvx = NULL;
PFNGLTEXENVXOESPROC glad_glTexEnvxOES = NULL;
PFNGLTEXENVXVPROC glad_glTexEnvxv = NULL;
PFNGLTEXENVXVOESPROC glad_glTexEnvxvOES = NULL;
PFNGLTEXGENFOESPROC glad_glTexGenfOES = NULL;
PFNGLTEXGENFVOESPROC glad_glTexGenfvOES = NULL;
PFNGLTEXGENIOESPROC glad_glTexGeniOES = NULL;
PFNGLTEXGENIVOESPROC glad_glTexGenivOES = NULL;
PFNGLTEXGENXOESPROC glad_glTexGenxOES = NULL;
PFNGLTEXGENXVOESPROC glad_glTexGenxvOES = NULL;
PFNGLTEXIMAGE2DPROC glad_glTexImage2D = NULL;
PFNGLTEXPARAMETERFPROC glad_glTexParameterf = NULL;
PFNGLTEXPARAMETERFVPROC glad_glTexParameterfv = NULL;
PFNGLTEXPARAMETERIPROC glad_glTexParameteri = NULL;
PFNGLTEXPARAMETERIVPROC glad_glTexParameteriv = NULL;
PFNGLTEXPARAMETERXPROC glad_glTexParameterx = NULL;
PFNGLTEXPARAMETERXOESPROC glad_glTexParameterxOES = NULL;
PFNGLTEXPARAMETERXVPROC glad_glTexParameterxv = NULL;
PFNGLTEXPARAMETERXVOESPROC glad_glTexParameterxvOES = NULL;
PFNGLTEXSTORAGE1DPROC glad_glTexStorage1D = NULL;
PFNGLTEXSTORAGE1DEXTPROC glad_glTexStorage1DEXT = NULL;
PFNGLTEXSTORAGE2DPROC glad_glTexStorage2D = NULL;
PFNGLTEXSTORAGE2DEXTPROC glad_glTexStorage2DEXT = NULL;
PFNGLTEXSTORAGE3DPROC glad_glTexStorage3D = NULL;
PFNGLTEXSTORAGE3DEXTPROC glad_glTexStorage3DEXT = NULL;
PFNGLTEXSUBIMAGE2DPROC glad_glTexSubImage2D = NULL;
PFNGLTEXTURESTORAGE1DEXTPROC glad_glTextureStorage1DEXT = NULL;
PFNGLTEXTURESTORAGE2DEXTPROC glad_glTextureStorage2DEXT = NULL;
PFNGLTEXTURESTORAGE3DEXTPROC glad_glTextureStorage3DEXT = NULL;
PFNGLTRANSLATEFPROC glad_glTranslatef = NULL;
PFNGLTRANSLATEXPROC glad_glTranslatex = NULL;
PFNGLTRANSLATEXOESPROC glad_glTranslatexOES = NULL;
PFNGLUNMAPBUFFERPROC glad_glUnmapBuffer = NULL;
PFNGLUNMAPBUFFEROESPROC glad_glUnmapBufferOES = NULL;
PFNGLVERTEXPOINTERPROC glad_glVertexPointer = NULL;
PFNGLVIEWPORTPROC glad_glViewport = NULL;
PFNGLWAITSYNCPROC glad_glWaitSync = NULL;
PFNGLWAITSYNCAPPLEPROC glad_glWaitSyncAPPLE = NULL;
PFNGLWEIGHTPOINTEROESPROC glad_glWeightPointerOES = NULL;


static void glad_gl_load_GL_VERSION_ES_CM_1_0( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_VERSION_ES_CM_1_0) return;
    glActiveTexture = (PFNGLACTIVETEXTUREPROC) load("glActiveTexture", userptr);
    glAlphaFunc = (PFNGLALPHAFUNCPROC) load("glAlphaFunc", userptr);
    glAlphaFuncx = (PFNGLALPHAFUNCXPROC) load("glAlphaFuncx", userptr);
    glBindBuffer = (PFNGLBINDBUFFERPROC) load("glBindBuffer", userptr);
    glBindTexture = (PFNGLBINDTEXTUREPROC) load("glBindTexture", userptr);
    glBlendFunc = (PFNGLBLENDFUNCPROC) load("glBlendFunc", userptr);
    glBufferData = (PFNGLBUFFERDATAPROC) load("glBufferData", userptr);
    glBufferSubData = (PFNGLBUFFERSUBDATAPROC) load("glBufferSubData", userptr);
    glClear = (PFNGLCLEARPROC) load("glClear", userptr);
    glClearColor = (PFNGLCLEARCOLORPROC) load("glClearColor", userptr);
    glClearColorx = (PFNGLCLEARCOLORXPROC) load("glClearColorx", userptr);
    glClearDepthf = (PFNGLCLEARDEPTHFPROC) load("glClearDepthf", userptr);
    glClearDepthx = (PFNGLCLEARDEPTHXPROC) load("glClearDepthx", userptr);
    glClearStencil = (PFNGLCLEARSTENCILPROC) load("glClearStencil", userptr);
    glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC) load("glClientActiveTexture", userptr);
    glClipPlanef = (PFNGLCLIPPLANEFPROC) load("glClipPlanef", userptr);
    glClipPlanex = (PFNGLCLIPPLANEXPROC) load("glClipPlanex", userptr);
    glColor4f = (PFNGLCOLOR4FPROC) load("glColor4f", userptr);
    glColor4ub = (PFNGLCOLOR4UBPROC) load("glColor4ub", userptr);
    glColor4x = (PFNGLCOLOR4XPROC) load("glColor4x", userptr);
    glColorMask = (PFNGLCOLORMASKPROC) load("glColorMask", userptr);
    glColorPointer = (PFNGLCOLORPOINTERPROC) load("glColorPointer", userptr);
    glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) load("glCompressedTexImage2D", userptr);
    glCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) load("glCompressedTexSubImage2D", userptr);
    glCopyTexImage2D = (PFNGLCOPYTEXIMAGE2DPROC) load("glCopyTexImage2D", userptr);
    glCopyTexSubImage2D = (PFNGLCOPYTEXSUBIMAGE2DPROC) load("glCopyTexSubImage2D", userptr);
    glCullFace = (PFNGLCULLFACEPROC) load("glCullFace", userptr);
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) load("glDeleteBuffers", userptr);
    glDeleteTextures = (PFNGLDELETETEXTURESPROC) load("glDeleteTextures", userptr);
    glDepthFunc = (PFNGLDEPTHFUNCPROC) load("glDepthFunc", userptr);
    glDepthMask = (PFNGLDEPTHMASKPROC) load("glDepthMask", userptr);
    glDepthRangef = (PFNGLDEPTHRANGEFPROC) load("glDepthRangef", userptr);
    glDepthRangex = (PFNGLDEPTHRANGEXPROC) load("glDepthRangex", userptr);
    glDisable = (PFNGLDISABLEPROC) load("glDisable", userptr);
    glDisableClientState = (PFNGLDISABLECLIENTSTATEPROC) load("glDisableClientState", userptr);
    glDrawArrays = (PFNGLDRAWARRAYSPROC) load("glDrawArrays", userptr);
    glDrawElements = (PFNGLDRAWELEMENTSPROC) load("glDrawElements", userptr);
    glEnable = (PFNGLENABLEPROC) load("glEnable", userptr);
    glEnableClientState = (PFNGLENABLECLIENTSTATEPROC) load("glEnableClientState", userptr);
    glFinish = (PFNGLFINISHPROC) load("glFinish", userptr);
    glFlush = (PFNGLFLUSHPROC) load("glFlush", userptr);
    glFogf = (PFNGLFOGFPROC) load("glFogf", userptr);
    glFogfv = (PFNGLFOGFVPROC) load("glFogfv", userptr);
    glFogx = (PFNGLFOGXPROC) load("glFogx", userptr);
    glFogxv = (PFNGLFOGXVPROC) load("glFogxv", userptr);
    glFrontFace = (PFNGLFRONTFACEPROC) load("glFrontFace", userptr);
    glFrustumf = (PFNGLFRUSTUMFPROC) load("glFrustumf", userptr);
    glFrustumx = (PFNGLFRUSTUMXPROC) load("glFrustumx", userptr);
    glGenBuffers = (PFNGLGENBUFFERSPROC) load("glGenBuffers", userptr);
    glGenTextures = (PFNGLGENTEXTURESPROC) load("glGenTextures", userptr);
    glGetBooleanv = (PFNGLGETBOOLEANVPROC) load("glGetBooleanv", userptr);
    glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC) load("glGetBufferParameteriv", userptr);
    glGetClipPlanef = (PFNGLGETCLIPPLANEFPROC) load("glGetClipPlanef", userptr);
    glGetClipPlanex = (PFNGLGETCLIPPLANEXPROC) load("glGetClipPlanex", userptr);
    glGetError = (PFNGLGETERRORPROC) load("glGetError", userptr);
    glGetFixedv = (PFNGLGETFIXEDVPROC) load("glGetFixedv", userptr);
    glGetFloatv = (PFNGLGETFLOATVPROC) load("glGetFloatv", userptr);
    glGetIntegerv = (PFNGLGETINTEGERVPROC) load("glGetIntegerv", userptr);
    glGetLightfv = (PFNGLGETLIGHTFVPROC) load("glGetLightfv", userptr);
    glGetLightxv = (PFNGLGETLIGHTXVPROC) load("glGetLightxv", userptr);
    glGetMaterialfv = (PFNGLGETMATERIALFVPROC) load("glGetMaterialfv", userptr);
    glGetMaterialxv = (PFNGLGETMATERIALXVPROC) load("glGetMaterialxv", userptr);
    glGetPointerv = (PFNGLGETPOINTERVPROC) load("glGetPointerv", userptr);
    glGetString = (PFNGLGETSTRINGPROC) load("glGetString", userptr);
    glGetTexEnvfv = (PFNGLGETTEXENVFVPROC) load("glGetTexEnvfv", userptr);
    glGetTexEnviv = (PFNGLGETTEXENVIVPROC) load("glGetTexEnviv", userptr);
    glGetTexEnvxv = (PFNGLGETTEXENVXVPROC) load("glGetTexEnvxv", userptr);
    glGetTexParameterfv = (PFNGLGETTEXPARAMETERFVPROC) load("glGetTexParameterfv", userptr);
    glGetTexParameteriv = (PFNGLGETTEXPARAMETERIVPROC) load("glGetTexParameteriv", userptr);
    glGetTexParameterxv = (PFNGLGETTEXPARAMETERXVPROC) load("glGetTexParameterxv", userptr);
    glHint = (PFNGLHINTPROC) load("glHint", userptr);
    glIsBuffer = (PFNGLISBUFFERPROC) load("glIsBuffer", userptr);
    glIsEnabled = (PFNGLISENABLEDPROC) load("glIsEnabled", userptr);
    glIsTexture = (PFNGLISTEXTUREPROC) load("glIsTexture", userptr);
    glLightModelf = (PFNGLLIGHTMODELFPROC) load("glLightModelf", userptr);
    glLightModelfv = (PFNGLLIGHTMODELFVPROC) load("glLightModelfv", userptr);
    glLightModelx = (PFNGLLIGHTMODELXPROC) load("glLightModelx", userptr);
    glLightModelxv = (PFNGLLIGHTMODELXVPROC) load("glLightModelxv", userptr);
    glLightf = (PFNGLLIGHTFPROC) load("glLightf", userptr);
    glLightfv = (PFNGLLIGHTFVPROC) load("glLightfv", userptr);
    glLightx = (PFNGLLIGHTXPROC) load("glLightx", userptr);
    glLightxv = (PFNGLLIGHTXVPROC) load("glLightxv", userptr);
    glLineWidth = (PFNGLLINEWIDTHPROC) load("glLineWidth", userptr);
    glLineWidthx = (PFNGLLINEWIDTHXPROC) load("glLineWidthx", userptr);
    glLoadIdentity = (PFNGLLOADIDENTITYPROC) load("glLoadIdentity", userptr);
    glLoadMatrixf = (PFNGLLOADMATRIXFPROC) load("glLoadMatrixf", userptr);
    glLoadMatrixx = (PFNGLLOADMATRIXXPROC) load("glLoadMatrixx", userptr);
    glLogicOp = (PFNGLLOGICOPPROC) load("glLogicOp", userptr);
    glMaterialf = (PFNGLMATERIALFPROC) load("glMaterialf", userptr);
    glMaterialfv = (PFNGLMATERIALFVPROC) load("glMaterialfv", userptr);
    glMaterialx = (PFNGLMATERIALXPROC) load("glMaterialx", userptr);
    glMaterialxv = (PFNGLMATERIALXVPROC) load("glMaterialxv", userptr);
    glMatrixMode = (PFNGLMATRIXMODEPROC) load("glMatrixMode", userptr);
    glMultMatrixf = (PFNGLMULTMATRIXFPROC) load("glMultMatrixf", userptr);
    glMultMatrixx = (PFNGLMULTMATRIXXPROC) load("glMultMatrixx", userptr);
    glMultiTexCoord4f = (PFNGLMULTITEXCOORD4FPROC) load("glMultiTexCoord4f", userptr);
    glMultiTexCoord4x = (PFNGLMULTITEXCOORD4XPROC) load("glMultiTexCoord4x", userptr);
    glNormal3f = (PFNGLNORMAL3FPROC) load("glNormal3f", userptr);
    glNormal3x = (PFNGLNORMAL3XPROC) load("glNormal3x", userptr);
    glNormalPointer = (PFNGLNORMALPOINTERPROC) load("glNormalPointer", userptr);
    glOrthof = (PFNGLORTHOFPROC) load("glOrthof", userptr);
    glOrthox = (PFNGLORTHOXPROC) load("glOrthox", userptr);
    glPixelStorei = (PFNGLPIXELSTOREIPROC) load("glPixelStorei", userptr);
    glPointParameterf = (PFNGLPOINTPARAMETERFPROC) load("glPointParameterf", userptr);
    glPointParameterfv = (PFNGLPOINTPARAMETERFVPROC) load("glPointParameterfv", userptr);
    glPointParameterx = (PFNGLPOINTPARAMETERXPROC) load("glPointParameterx", userptr);
    glPointParameterxv = (PFNGLPOINTPARAMETERXVPROC) load("glPointParameterxv", userptr);
    glPointSize = (PFNGLPOINTSIZEPROC) load("glPointSize", userptr);
    glPointSizex = (PFNGLPOINTSIZEXPROC) load("glPointSizex", userptr);
    glPolygonOffset = (PFNGLPOLYGONOFFSETPROC) load("glPolygonOffset", userptr);
    glPolygonOffsetx = (PFNGLPOLYGONOFFSETXPROC) load("glPolygonOffsetx", userptr);
    glPopMatrix = (PFNGLPOPMATRIXPROC) load("glPopMatrix", userptr);
    glPushMatrix = (PFNGLPUSHMATRIXPROC) load("glPushMatrix", userptr);
    glReadPixels = (PFNGLREADPIXELSPROC) load("glReadPixels", userptr);
    glRotatef = (PFNGLROTATEFPROC) load("glRotatef", userptr);
    glRotatex = (PFNGLROTATEXPROC) load("glRotatex", userptr);
    glSampleCoverage = (PFNGLSAMPLECOVERAGEPROC) load("glSampleCoverage", userptr);
    glSampleCoveragex = (PFNGLSAMPLECOVERAGEXPROC) load("glSampleCoveragex", userptr);
    glScalef = (PFNGLSCALEFPROC) load("glScalef", userptr);
    glScalex = (PFNGLSCALEXPROC) load("glScalex", userptr);
    glScissor = (PFNGLSCISSORPROC) load("glScissor", userptr);
    glShadeModel = (PFNGLSHADEMODELPROC) load("glShadeModel", userptr);
    glStencilFunc = (PFNGLSTENCILFUNCPROC) load("glStencilFunc", userptr);
    glStencilMask = (PFNGLSTENCILMASKPROC) load("glStencilMask", userptr);
    glStencilOp = (PFNGLSTENCILOPPROC) load("glStencilOp", userptr);
    glTexCoordPointer = (PFNGLTEXCOORDPOINTERPROC) load("glTexCoordPointer", userptr);
    glTexEnvf = (PFNGLTEXENVFPROC) load("glTexEnvf", userptr);
    glTexEnvfv = (PFNGLTEXENVFVPROC) load("glTexEnvfv", userptr);
    glTexEnvi = (PFNGLTEXENVIPROC) load("glTexEnvi", userptr);
    glTexEnviv = (PFNGLTEXENVIVPROC) load("glTexEnviv", userptr);
    glTexEnvx = (PFNGLTEXENVXPROC) load("glTexEnvx", userptr);
    glTexEnvxv = (PFNGLTEXENVXVPROC) load("glTexEnvxv", userptr);
    glTexImage2D = (PFNGLTEXIMAGE2DPROC) load("glTexImage2D", userptr);
    glTexParameterf = (PFNGLTEXPARAMETERFPROC) load("glTexParameterf", userptr);
    glTexParameterfv = (PFNGLTEXPARAMETERFVPROC) load("glTexParameterfv", userptr);
    glTexParameteri = (PFNGLTEXPARAMETERIPROC) load("glTexParameteri", userptr);
    glTexParameteriv = (PFNGLTEXPARAMETERIVPROC) load("glTexParameteriv", userptr);
    glTexParameterx = (PFNGLTEXPARAMETERXPROC) load("glTexParameterx", userptr);
    glTexParameterxv = (PFNGLTEXPARAMETERXVPROC) load("glTexParameterxv", userptr);
    glTexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC) load("glTexSubImage2D", userptr);
    glTranslatef = (PFNGLTRANSLATEFPROC) load("glTranslatef", userptr);
    glTranslatex = (PFNGLTRANSLATEXPROC) load("glTranslatex", userptr);
    glVertexPointer = (PFNGLVERTEXPOINTERPROC) load("glVertexPointer", userptr);
    glViewport = (PFNGLVIEWPORTPROC) load("glViewport", userptr);
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
static void glad_gl_load_GL_EXT_map_buffer_range( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_map_buffer_range) return;
    glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC) load("glFlushMappedBufferRange", userptr);
    glFlushMappedBufferRangeEXT = (PFNGLFLUSHMAPPEDBUFFERRANGEEXTPROC) load("glFlushMappedBufferRangeEXT", userptr);
    glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC) load("glMapBufferRange", userptr);
    glMapBufferRangeEXT = (PFNGLMAPBUFFERRANGEEXTPROC) load("glMapBufferRangeEXT", userptr);
}
static void glad_gl_load_GL_EXT_multi_draw_arrays( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_multi_draw_arrays) return;
    glMultiDrawArrays = (PFNGLMULTIDRAWARRAYSPROC) load("glMultiDrawArrays", userptr);
    glMultiDrawArraysEXT = (PFNGLMULTIDRAWARRAYSEXTPROC) load("glMultiDrawArraysEXT", userptr);
    glMultiDrawElements = (PFNGLMULTIDRAWELEMENTSPROC) load("glMultiDrawElements", userptr);
    glMultiDrawElementsEXT = (PFNGLMULTIDRAWELEMENTSEXTPROC) load("glMultiDrawElementsEXT", userptr);
}
static void glad_gl_load_GL_EXT_multisampled_render_to_texture( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_multisampled_render_to_texture) return;
    glFramebufferTexture2DMultisampleEXT = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC) load("glFramebufferTexture2DMultisampleEXT", userptr);
    glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC) load("glRenderbufferStorageMultisample", userptr);
    glRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) load("glRenderbufferStorageMultisampleEXT", userptr);
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
static void glad_gl_load_GL_IMG_multisampled_render_to_texture( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_IMG_multisampled_render_to_texture) return;
    glFramebufferTexture2DMultisampleIMG = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC) load("glFramebufferTexture2DMultisampleIMG", userptr);
    glRenderbufferStorageMultisampleIMG = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMGPROC) load("glRenderbufferStorageMultisampleIMG", userptr);
}
static void glad_gl_load_GL_IMG_user_clip_plane( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_IMG_user_clip_plane) return;
    glClipPlanefIMG = (PFNGLCLIPPLANEFIMGPROC) load("glClipPlanefIMG", userptr);
    glClipPlanexIMG = (PFNGLCLIPPLANEXIMGPROC) load("glClipPlanexIMG", userptr);
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
static void glad_gl_load_GL_OES_EGL_image( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_EGL_image) return;
    glEGLImageTargetRenderbufferStorageOES = (PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC) load("glEGLImageTargetRenderbufferStorageOES", userptr);
    glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) load("glEGLImageTargetTexture2DOES", userptr);
}
static void glad_gl_load_GL_OES_blend_equation_separate( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_blend_equation_separate) return;
    glBlendEquationSeparateOES = (PFNGLBLENDEQUATIONSEPARATEOESPROC) load("glBlendEquationSeparateOES", userptr);
}
static void glad_gl_load_GL_OES_blend_func_separate( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_blend_func_separate) return;
    glBlendFuncSeparateOES = (PFNGLBLENDFUNCSEPARATEOESPROC) load("glBlendFuncSeparateOES", userptr);
}
static void glad_gl_load_GL_OES_blend_subtract( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_blend_subtract) return;
    glBlendEquationOES = (PFNGLBLENDEQUATIONOESPROC) load("glBlendEquationOES", userptr);
}
static void glad_gl_load_GL_OES_draw_texture( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_draw_texture) return;
    glDrawTexfOES = (PFNGLDRAWTEXFOESPROC) load("glDrawTexfOES", userptr);
    glDrawTexfvOES = (PFNGLDRAWTEXFVOESPROC) load("glDrawTexfvOES", userptr);
    glDrawTexiOES = (PFNGLDRAWTEXIOESPROC) load("glDrawTexiOES", userptr);
    glDrawTexivOES = (PFNGLDRAWTEXIVOESPROC) load("glDrawTexivOES", userptr);
    glDrawTexsOES = (PFNGLDRAWTEXSOESPROC) load("glDrawTexsOES", userptr);
    glDrawTexsvOES = (PFNGLDRAWTEXSVOESPROC) load("glDrawTexsvOES", userptr);
    glDrawTexxOES = (PFNGLDRAWTEXXOESPROC) load("glDrawTexxOES", userptr);
    glDrawTexxvOES = (PFNGLDRAWTEXXVOESPROC) load("glDrawTexxvOES", userptr);
}
static void glad_gl_load_GL_OES_fixed_point( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_fixed_point) return;
    glAlphaFuncxOES = (PFNGLALPHAFUNCXOESPROC) load("glAlphaFuncxOES", userptr);
    glClearColorxOES = (PFNGLCLEARCOLORXOESPROC) load("glClearColorxOES", userptr);
    glClearDepthxOES = (PFNGLCLEARDEPTHXOESPROC) load("glClearDepthxOES", userptr);
    glClipPlanexOES = (PFNGLCLIPPLANEXOESPROC) load("glClipPlanexOES", userptr);
    glColor4xOES = (PFNGLCOLOR4XOESPROC) load("glColor4xOES", userptr);
    glDepthRangexOES = (PFNGLDEPTHRANGEXOESPROC) load("glDepthRangexOES", userptr);
    glFogxOES = (PFNGLFOGXOESPROC) load("glFogxOES", userptr);
    glFogxvOES = (PFNGLFOGXVOESPROC) load("glFogxvOES", userptr);
    glFrustumxOES = (PFNGLFRUSTUMXOESPROC) load("glFrustumxOES", userptr);
    glGetClipPlanexOES = (PFNGLGETCLIPPLANEXOESPROC) load("glGetClipPlanexOES", userptr);
    glGetFixedvOES = (PFNGLGETFIXEDVOESPROC) load("glGetFixedvOES", userptr);
    glGetLightxvOES = (PFNGLGETLIGHTXVOESPROC) load("glGetLightxvOES", userptr);
    glGetMaterialxvOES = (PFNGLGETMATERIALXVOESPROC) load("glGetMaterialxvOES", userptr);
    glGetTexEnvxvOES = (PFNGLGETTEXENVXVOESPROC) load("glGetTexEnvxvOES", userptr);
    glGetTexParameterxvOES = (PFNGLGETTEXPARAMETERXVOESPROC) load("glGetTexParameterxvOES", userptr);
    glLightModelxOES = (PFNGLLIGHTMODELXOESPROC) load("glLightModelxOES", userptr);
    glLightModelxvOES = (PFNGLLIGHTMODELXVOESPROC) load("glLightModelxvOES", userptr);
    glLightxOES = (PFNGLLIGHTXOESPROC) load("glLightxOES", userptr);
    glLightxvOES = (PFNGLLIGHTXVOESPROC) load("glLightxvOES", userptr);
    glLineWidthxOES = (PFNGLLINEWIDTHXOESPROC) load("glLineWidthxOES", userptr);
    glLoadMatrixxOES = (PFNGLLOADMATRIXXOESPROC) load("glLoadMatrixxOES", userptr);
    glMaterialxOES = (PFNGLMATERIALXOESPROC) load("glMaterialxOES", userptr);
    glMaterialxvOES = (PFNGLMATERIALXVOESPROC) load("glMaterialxvOES", userptr);
    glMultMatrixxOES = (PFNGLMULTMATRIXXOESPROC) load("glMultMatrixxOES", userptr);
    glMultiTexCoord4xOES = (PFNGLMULTITEXCOORD4XOESPROC) load("glMultiTexCoord4xOES", userptr);
    glNormal3xOES = (PFNGLNORMAL3XOESPROC) load("glNormal3xOES", userptr);
    glOrthoxOES = (PFNGLORTHOXOESPROC) load("glOrthoxOES", userptr);
    glPointParameterxOES = (PFNGLPOINTPARAMETERXOESPROC) load("glPointParameterxOES", userptr);
    glPointParameterxvOES = (PFNGLPOINTPARAMETERXVOESPROC) load("glPointParameterxvOES", userptr);
    glPointSizexOES = (PFNGLPOINTSIZEXOESPROC) load("glPointSizexOES", userptr);
    glPolygonOffsetxOES = (PFNGLPOLYGONOFFSETXOESPROC) load("glPolygonOffsetxOES", userptr);
    glRotatexOES = (PFNGLROTATEXOESPROC) load("glRotatexOES", userptr);
    glSampleCoveragexOES = (PFNGLSAMPLECOVERAGEXOESPROC) load("glSampleCoveragexOES", userptr);
    glScalexOES = (PFNGLSCALEXOESPROC) load("glScalexOES", userptr);
    glTexEnvxOES = (PFNGLTEXENVXOESPROC) load("glTexEnvxOES", userptr);
    glTexEnvxvOES = (PFNGLTEXENVXVOESPROC) load("glTexEnvxvOES", userptr);
    glTexParameterxOES = (PFNGLTEXPARAMETERXOESPROC) load("glTexParameterxOES", userptr);
    glTexParameterxvOES = (PFNGLTEXPARAMETERXVOESPROC) load("glTexParameterxvOES", userptr);
    glTranslatexOES = (PFNGLTRANSLATEXOESPROC) load("glTranslatexOES", userptr);
}
static void glad_gl_load_GL_OES_framebuffer_object( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_framebuffer_object) return;
    glBindFramebufferOES = (PFNGLBINDFRAMEBUFFEROESPROC) load("glBindFramebufferOES", userptr);
    glBindRenderbufferOES = (PFNGLBINDRENDERBUFFEROESPROC) load("glBindRenderbufferOES", userptr);
    glCheckFramebufferStatusOES = (PFNGLCHECKFRAMEBUFFERSTATUSOESPROC) load("glCheckFramebufferStatusOES", userptr);
    glDeleteFramebuffersOES = (PFNGLDELETEFRAMEBUFFERSOESPROC) load("glDeleteFramebuffersOES", userptr);
    glDeleteRenderbuffersOES = (PFNGLDELETERENDERBUFFERSOESPROC) load("glDeleteRenderbuffersOES", userptr);
    glFramebufferRenderbufferOES = (PFNGLFRAMEBUFFERRENDERBUFFEROESPROC) load("glFramebufferRenderbufferOES", userptr);
    glFramebufferTexture2DOES = (PFNGLFRAMEBUFFERTEXTURE2DOESPROC) load("glFramebufferTexture2DOES", userptr);
    glGenFramebuffersOES = (PFNGLGENFRAMEBUFFERSOESPROC) load("glGenFramebuffersOES", userptr);
    glGenRenderbuffersOES = (PFNGLGENRENDERBUFFERSOESPROC) load("glGenRenderbuffersOES", userptr);
    glGenerateMipmapOES = (PFNGLGENERATEMIPMAPOESPROC) load("glGenerateMipmapOES", userptr);
    glGetFramebufferAttachmentParameterivOES = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOESPROC) load("glGetFramebufferAttachmentParameterivOES", userptr);
    glGetRenderbufferParameterivOES = (PFNGLGETRENDERBUFFERPARAMETERIVOESPROC) load("glGetRenderbufferParameterivOES", userptr);
    glIsFramebufferOES = (PFNGLISFRAMEBUFFEROESPROC) load("glIsFramebufferOES", userptr);
    glIsRenderbufferOES = (PFNGLISRENDERBUFFEROESPROC) load("glIsRenderbufferOES", userptr);
    glRenderbufferStorageOES = (PFNGLRENDERBUFFERSTORAGEOESPROC) load("glRenderbufferStorageOES", userptr);
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
static void glad_gl_load_GL_OES_matrix_palette( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_matrix_palette) return;
    glCurrentPaletteMatrixOES = (PFNGLCURRENTPALETTEMATRIXOESPROC) load("glCurrentPaletteMatrixOES", userptr);
    glLoadPaletteFromModelViewMatrixOES = (PFNGLLOADPALETTEFROMMODELVIEWMATRIXOESPROC) load("glLoadPaletteFromModelViewMatrixOES", userptr);
    glMatrixIndexPointerOES = (PFNGLMATRIXINDEXPOINTEROESPROC) load("glMatrixIndexPointerOES", userptr);
    glWeightPointerOES = (PFNGLWEIGHTPOINTEROESPROC) load("glWeightPointerOES", userptr);
}
static void glad_gl_load_GL_OES_point_size_array( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_point_size_array) return;
    glPointSizePointerOES = (PFNGLPOINTSIZEPOINTEROESPROC) load("glPointSizePointerOES", userptr);
}
static void glad_gl_load_GL_OES_query_matrix( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_query_matrix) return;
    glQueryMatrixxOES = (PFNGLQUERYMATRIXXOESPROC) load("glQueryMatrixxOES", userptr);
}
static void glad_gl_load_GL_OES_single_precision( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_single_precision) return;
    glClearDepthf = (PFNGLCLEARDEPTHFPROC) load("glClearDepthf", userptr);
    glClearDepthfOES = (PFNGLCLEARDEPTHFOESPROC) load("glClearDepthfOES", userptr);
    glClipPlanefOES = (PFNGLCLIPPLANEFOESPROC) load("glClipPlanefOES", userptr);
    glDepthRangef = (PFNGLDEPTHRANGEFPROC) load("glDepthRangef", userptr);
    glDepthRangefOES = (PFNGLDEPTHRANGEFOESPROC) load("glDepthRangefOES", userptr);
    glFrustumfOES = (PFNGLFRUSTUMFOESPROC) load("glFrustumfOES", userptr);
    glGetClipPlanefOES = (PFNGLGETCLIPPLANEFOESPROC) load("glGetClipPlanefOES", userptr);
    glOrthofOES = (PFNGLORTHOFOESPROC) load("glOrthofOES", userptr);
}
static void glad_gl_load_GL_OES_texture_cube_map( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_texture_cube_map) return;
    glGetTexGenfvOES = (PFNGLGETTEXGENFVOESPROC) load("glGetTexGenfvOES", userptr);
    glGetTexGenivOES = (PFNGLGETTEXGENIVOESPROC) load("glGetTexGenivOES", userptr);
    glGetTexGenxvOES = (PFNGLGETTEXGENXVOESPROC) load("glGetTexGenxvOES", userptr);
    glTexGenfOES = (PFNGLTEXGENFOESPROC) load("glTexGenfOES", userptr);
    glTexGenfvOES = (PFNGLTEXGENFVOESPROC) load("glTexGenfvOES", userptr);
    glTexGeniOES = (PFNGLTEXGENIOESPROC) load("glTexGeniOES", userptr);
    glTexGenivOES = (PFNGLTEXGENIVOESPROC) load("glTexGenivOES", userptr);
    glTexGenxOES = (PFNGLTEXGENXOESPROC) load("glTexGenxOES", userptr);
    glTexGenxvOES = (PFNGLTEXGENXVOESPROC) load("glTexGenxvOES", userptr);
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

static int glad_gl_find_extensions_gles1( int version) {
    const char *exts = NULL;
    unsigned int num_exts_i = 0;
    char **exts_i = NULL;
    if (!glad_gl_get_extensions(version, &exts, &num_exts_i, &exts_i)) return 0;

    GLAD_GL_AMD_compressed_3DC_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_AMD_compressed_3DC_texture");
    GLAD_GL_AMD_compressed_ATC_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_AMD_compressed_ATC_texture");
    GLAD_GL_APPLE_copy_texture_levels = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_APPLE_copy_texture_levels");
    GLAD_GL_APPLE_framebuffer_multisample = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_APPLE_framebuffer_multisample");
    GLAD_GL_APPLE_sync = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_APPLE_sync");
    GLAD_GL_APPLE_texture_2D_limited_npot = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_APPLE_texture_2D_limited_npot");
    GLAD_GL_APPLE_texture_format_BGRA8888 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_APPLE_texture_format_BGRA8888");
    GLAD_GL_APPLE_texture_max_level = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_APPLE_texture_max_level");
    GLAD_GL_ARM_rgba8 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_ARM_rgba8");
    GLAD_GL_EXT_blend_minmax = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_blend_minmax");
    GLAD_GL_EXT_debug_marker = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_debug_marker");
    GLAD_GL_EXT_discard_framebuffer = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_discard_framebuffer");
    GLAD_GL_EXT_map_buffer_range = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_map_buffer_range");
    GLAD_GL_EXT_multi_draw_arrays = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_multi_draw_arrays");
    GLAD_GL_EXT_multisampled_render_to_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_multisampled_render_to_texture");
    GLAD_GL_EXT_read_format_bgra = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_read_format_bgra");
    GLAD_GL_EXT_robustness = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_robustness");
    GLAD_GL_EXT_sRGB = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_sRGB");
    GLAD_GL_EXT_texture_compression_dxt1 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_compression_dxt1");
    GLAD_GL_EXT_texture_filter_anisotropic = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_filter_anisotropic");
    GLAD_GL_EXT_texture_format_BGRA8888 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_format_BGRA8888");
    GLAD_GL_EXT_texture_lod_bias = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_lod_bias");
    GLAD_GL_EXT_texture_storage = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_EXT_texture_storage");
    GLAD_GL_IMG_multisampled_render_to_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_IMG_multisampled_render_to_texture");
    GLAD_GL_IMG_read_format = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_IMG_read_format");
    GLAD_GL_IMG_texture_compression_pvrtc = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_IMG_texture_compression_pvrtc");
    GLAD_GL_IMG_texture_env_enhanced_fixed_function = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_IMG_texture_env_enhanced_fixed_function");
    GLAD_GL_IMG_user_clip_plane = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_IMG_user_clip_plane");
    GLAD_GL_KHR_debug = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_KHR_debug");
    GLAD_GL_NV_fence = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_NV_fence");
    GLAD_GL_OES_EGL_image = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_EGL_image");
    GLAD_GL_OES_EGL_image_external = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_EGL_image_external");
    GLAD_GL_OES_blend_equation_separate = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_blend_equation_separate");
    GLAD_GL_OES_blend_func_separate = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_blend_func_separate");
    GLAD_GL_OES_blend_subtract = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_blend_subtract");
    GLAD_GL_OES_byte_coordinates = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_byte_coordinates");
    GLAD_GL_OES_compressed_ETC1_RGB8_sub_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_compressed_ETC1_RGB8_sub_texture");
    GLAD_GL_OES_compressed_ETC1_RGB8_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_compressed_ETC1_RGB8_texture");
    GLAD_GL_OES_compressed_paletted_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_compressed_paletted_texture");
    GLAD_GL_OES_depth24 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_depth24");
    GLAD_GL_OES_depth32 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_depth32");
    GLAD_GL_OES_draw_texture = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_draw_texture");
    GLAD_GL_OES_element_index_uint = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_element_index_uint");
    GLAD_GL_OES_extended_matrix_palette = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_extended_matrix_palette");
    GLAD_GL_OES_fbo_render_mipmap = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_fbo_render_mipmap");
    GLAD_GL_OES_fixed_point = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_fixed_point");
    GLAD_GL_OES_framebuffer_object = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_framebuffer_object");
    GLAD_GL_OES_mapbuffer = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_mapbuffer");
    GLAD_GL_OES_matrix_get = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_matrix_get");
    GLAD_GL_OES_matrix_palette = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_matrix_palette");
    GLAD_GL_OES_packed_depth_stencil = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_packed_depth_stencil");
    GLAD_GL_OES_point_size_array = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_point_size_array");
    GLAD_GL_OES_point_sprite = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_point_sprite");
    GLAD_GL_OES_query_matrix = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_query_matrix");
    GLAD_GL_OES_read_format = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_read_format");
    GLAD_GL_OES_required_internalformat = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_required_internalformat");
    GLAD_GL_OES_rgb8_rgba8 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_rgb8_rgba8");
    GLAD_GL_OES_single_precision = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_single_precision");
    GLAD_GL_OES_stencil1 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_stencil1");
    GLAD_GL_OES_stencil4 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_stencil4");
    GLAD_GL_OES_stencil8 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_stencil8");
    GLAD_GL_OES_stencil_wrap = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_stencil_wrap");
    GLAD_GL_OES_surfaceless_context = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_surfaceless_context");
    GLAD_GL_OES_texture_cube_map = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_texture_cube_map");
    GLAD_GL_OES_texture_env_crossbar = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_texture_env_crossbar");
    GLAD_GL_OES_texture_mirrored_repeat = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_texture_mirrored_repeat");
    GLAD_GL_OES_texture_npot = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_texture_npot");
    GLAD_GL_OES_vertex_array_object = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_OES_vertex_array_object");
    GLAD_GL_QCOM_driver_control = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_QCOM_driver_control");
    GLAD_GL_QCOM_extended_get = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_QCOM_extended_get");
    GLAD_GL_QCOM_extended_get2 = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_QCOM_extended_get2");
    GLAD_GL_QCOM_perfmon_global_mode = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_QCOM_perfmon_global_mode");
    GLAD_GL_QCOM_tiled_rendering = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_QCOM_tiled_rendering");
    GLAD_GL_QCOM_writeonly_rendering = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_QCOM_writeonly_rendering");

    glad_gl_free_extensions(exts_i, num_exts_i);

    return 1;
}

static int glad_gl_find_core_gles1(void) {
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

    GLAD_GL_VERSION_ES_CM_1_0 = (major == 1 && minor >= 0) || major > 1;

    return GLAD_MAKE_VERSION(major, minor);
}

int gladLoadGLES1UserPtr( GLADuserptrloadfunc load, void *userptr) {
    int version;

    glGetString = (PFNGLGETSTRINGPROC) load("glGetString", userptr);
    if(glGetString == NULL) return 0;
    if(glGetString(GL_VERSION) == NULL) return 0;
    version = glad_gl_find_core_gles1();

    glad_gl_load_GL_VERSION_ES_CM_1_0(load, userptr);

    if (!glad_gl_find_extensions_gles1(version)) return 0;
    glad_gl_load_GL_APPLE_copy_texture_levels(load, userptr);
    glad_gl_load_GL_APPLE_framebuffer_multisample(load, userptr);
    glad_gl_load_GL_APPLE_sync(load, userptr);
    glad_gl_load_GL_EXT_debug_marker(load, userptr);
    glad_gl_load_GL_EXT_discard_framebuffer(load, userptr);
    glad_gl_load_GL_EXT_map_buffer_range(load, userptr);
    glad_gl_load_GL_EXT_multi_draw_arrays(load, userptr);
    glad_gl_load_GL_EXT_multisampled_render_to_texture(load, userptr);
    glad_gl_load_GL_EXT_robustness(load, userptr);
    glad_gl_load_GL_EXT_texture_storage(load, userptr);
    glad_gl_load_GL_IMG_multisampled_render_to_texture(load, userptr);
    glad_gl_load_GL_IMG_user_clip_plane(load, userptr);
    glad_gl_load_GL_NV_fence(load, userptr);
    glad_gl_load_GL_OES_EGL_image(load, userptr);
    glad_gl_load_GL_OES_blend_equation_separate(load, userptr);
    glad_gl_load_GL_OES_blend_func_separate(load, userptr);
    glad_gl_load_GL_OES_blend_subtract(load, userptr);
    glad_gl_load_GL_OES_draw_texture(load, userptr);
    glad_gl_load_GL_OES_fixed_point(load, userptr);
    glad_gl_load_GL_OES_framebuffer_object(load, userptr);
    glad_gl_load_GL_OES_mapbuffer(load, userptr);
    glad_gl_load_GL_OES_matrix_palette(load, userptr);
    glad_gl_load_GL_OES_point_size_array(load, userptr);
    glad_gl_load_GL_OES_query_matrix(load, userptr);
    glad_gl_load_GL_OES_single_precision(load, userptr);
    glad_gl_load_GL_OES_texture_cube_map(load, userptr);
    glad_gl_load_GL_OES_vertex_array_object(load, userptr);
    glad_gl_load_GL_QCOM_driver_control(load, userptr);
    glad_gl_load_GL_QCOM_extended_get(load, userptr);
    glad_gl_load_GL_QCOM_extended_get2(load, userptr);
    glad_gl_load_GL_QCOM_tiled_rendering(load, userptr);



    return version;
}


int gladLoadGLES1( GLADloadfunc load) {
    return gladLoadGLES1UserPtr( glad_gl_get_proc_from_userptr, GLAD_GNUC_EXTENSION (void*) load);
}




#ifdef GLAD_GLES1

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

#include <glad/egl.h>

struct _glad_gles1_userptr {
    void *handle;
    PFNEGLGETPROCADDRESSPROC get_proc_address_ptr;
};


static GLADapiproc glad_gles1_get_proc(const char* name, void *vuserptr) {
    struct _glad_gles1_userptr userptr = *(struct _glad_gles1_userptr*) vuserptr;
    GLADapiproc result = NULL;

    result = glad_dlsym_handle(userptr.handle, name);
    if (result == NULL) {
        result = userptr.get_proc_address_ptr(name);
    }

    return result;
}

static void* _gles1_handle = NULL;

int gladLoaderLoadGLES1(void) {
#if GLAD_PLATFORM_APPLE
    static const char *NAMES[] = {"libGLESv1_CM.dylib"};
#elif GLAD_PLATFORM_WIN32
    static const char *NAMES[] = {"GLESv1_CM.dll", "libGLESv1_CM", "libGLES_CM.dll"};
#else
    static const char *NAMES[] = {"libGLESv1_CM.so.1", "libGLESv1_CM.so", "libGLES_CM.so.1"};
#endif

    int version = 0;
    int did_load = 0;
    struct _glad_gles1_userptr userptr;

    if (eglGetProcAddress == NULL) {
        return 0;
    }

    if (_gles1_handle == NULL) {
        _gles1_handle = glad_get_dlopen_handle(NAMES, sizeof(NAMES) / sizeof(NAMES[0]));
        did_load = _gles1_handle != NULL;
    }

    if (_gles1_handle != NULL) {
        userptr.handle = _gles1_handle;
        userptr.get_proc_address_ptr = eglGetProcAddress;

        version = gladLoadGLES1UserPtr(glad_gles1_get_proc, &userptr);

        if (!version && did_load) {
            glad_close_dlopen_handle(_gles1_handle);
            _gles1_handle = NULL;
        }
    }

    return version;
}


void gladLoaderUnloadGLES1(void) {
    if (_gles1_handle != NULL) {
        glad_close_dlopen_handle(_gles1_handle);
        _gles1_handle = NULL;
    }
}

#endif /* GLAD_GLES1 */
