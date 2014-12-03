//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// libEGL.cpp: Implements the exported EGL functions.

#include "libGLESv2/entry_points_egl.h"
#include "libGLESv2/entry_points_egl_ext.h"
#include "libGLESv2/entry_points_gles2_ext.h"
#include "libGLESv2/entry_points_gles3_ext.h"

extern "C"
{

EGLBoolean EGLAPIENTRY eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
    return egl::ChooseConfig(dpy, attrib_list, configs, config_size, num_config);
}

EGLBoolean EGLAPIENTRY eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target)
{
    return egl::CopyBuffers(dpy, surface, target);
}

EGLContext EGLAPIENTRY eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list)
{
    return egl::CreateContext(dpy, config, share_context, attrib_list);
}

EGLSurface EGLAPIENTRY eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list)
{
    return egl::CreatePbufferSurface(dpy, config, attrib_list);
}

EGLSurface EGLAPIENTRY eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list)
{
    return egl::CreatePixmapSurface(dpy, config, pixmap, attrib_list);
}

EGLSurface EGLAPIENTRY eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list)
{
    return egl::CreateWindowSurface(dpy, config, win, attrib_list);
}

EGLBoolean EGLAPIENTRY eglDestroyContext(EGLDisplay dpy, EGLContext ctx)
{
    return egl::DestroyContext(dpy, ctx);
}

EGLBoolean EGLAPIENTRY eglDestroySurface(EGLDisplay dpy, EGLSurface surface)
{
    return egl::DestroySurface(dpy, surface);
}

EGLBoolean EGLAPIENTRY eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value)
{
    return egl::GetConfigAttrib(dpy, config, attribute, value);
}

EGLBoolean EGLAPIENTRY eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
    return egl::GetConfigs(dpy, configs, config_size, num_config);
}

EGLDisplay EGLAPIENTRY eglGetCurrentDisplay(void)
{
    return egl::GetCurrentDisplay();
}

EGLSurface EGLAPIENTRY eglGetCurrentSurface(EGLint readdraw)
{
    return egl::GetCurrentSurface(readdraw);
}

EGLDisplay EGLAPIENTRY eglGetDisplay(EGLNativeDisplayType display_id)
{
    return egl::GetDisplay(display_id);
}

EGLint EGLAPIENTRY eglGetError(void)
{
    return egl::GetError();
}

EGLBoolean EGLAPIENTRY eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor)
{
    return egl::Initialize(dpy, major, minor);
}

EGLBoolean EGLAPIENTRY eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
    return egl::MakeCurrent(dpy, draw, read, ctx);
}

EGLBoolean EGLAPIENTRY eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value)
{
    return egl::QueryContext(dpy, ctx, attribute, value);
}

const char* EGLAPIENTRY eglQueryString(EGLDisplay dpy, EGLint name)
{
    return egl::QueryString(dpy, name);
}

EGLBoolean EGLAPIENTRY eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value)
{
    return egl::QuerySurface(dpy, surface, attribute, value);
}

EGLBoolean EGLAPIENTRY eglSwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
    return egl::SwapBuffers(dpy, surface);
}

EGLBoolean EGLAPIENTRY eglTerminate(EGLDisplay dpy)
{
    return egl::Terminate(dpy);
}

EGLBoolean EGLAPIENTRY eglWaitGL(void)
{
    return egl::WaitGL();
}

EGLBoolean EGLAPIENTRY eglWaitNative(EGLint engine)
{
    return egl::WaitNative(engine);
}

EGLBoolean EGLAPIENTRY eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    return egl::BindTexImage(dpy, surface, buffer);
}

EGLBoolean EGLAPIENTRY eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    return egl::ReleaseTexImage(dpy, surface, buffer);
}

EGLBoolean EGLAPIENTRY eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value)
{
    return egl::SurfaceAttrib(dpy, surface, attribute, value);
}

EGLBoolean EGLAPIENTRY eglSwapInterval(EGLDisplay dpy, EGLint interval)
{
    return egl::SwapInterval(dpy, interval);
}

EGLBoolean EGLAPIENTRY eglBindAPI(EGLenum api)
{
    return egl::BindAPI(api);
}

EGLenum EGLAPIENTRY eglQueryAPI(void)
{
    return egl::QueryAPI();
}

EGLSurface EGLAPIENTRY eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list)
{
    return egl::CreatePbufferFromClientBuffer(dpy, buftype, buffer, config, attrib_list);
}

EGLBoolean EGLAPIENTRY eglReleaseThread(void)
{
    return egl::ReleaseThread();
}

EGLBoolean EGLAPIENTRY eglWaitClient(void)
{
    return egl::WaitClient();
}

EGLContext EGLAPIENTRY eglGetCurrentContext(void)
{
    return egl::GetCurrentContext();
}

EGLSync EGLAPIENTRY eglCreateSync(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list)
{
    return egl::CreateSync(dpy, type, attrib_list);
}

EGLBoolean EGLAPIENTRY eglDestroySync(EGLDisplay dpy, EGLSync sync)
{
    return egl::DestroySync(dpy, sync);
}

EGLint EGLAPIENTRY eglClientWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTime timeout)
{
    return egl::ClientWaitSync(dpy, sync, flags, timeout);
}

EGLBoolean EGLAPIENTRY eglGetSyncAttrib(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib *value)
{
    return egl::GetSyncAttrib(dpy, sync, attribute, value);
}

EGLDisplay EGLAPIENTRY eglGetPlatformDisplay(EGLenum platform, void *native_display, const EGLAttrib *attrib_list)
{
    return egl::GetPlatformDisplay(platform, native_display, attrib_list);
}

EGLSurface EGLAPIENTRY eglCreatePlatformWindowSurface(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLAttrib *attrib_list)
{
    return egl::CreatePlatformWindowSurface(dpy, config, native_window, attrib_list);
}

EGLSurface EGLAPIENTRY eglCreatePlatformPixmapSurface(EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLAttrib *attrib_list)
{
    return egl::CreatePlatformPixmapSurface(dpy, config, native_pixmap, attrib_list);
}

EGLBoolean EGLAPIENTRY eglWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags)
{
    return egl::WaitSync(dpy, sync, flags);
}

EGLBoolean EGLAPIENTRY eglQuerySurfacePointerANGLE(EGLDisplay dpy, EGLSurface surface, EGLint attribute, void **value)
{
    return egl::QuerySurfacePointerANGLE(dpy, surface, attribute, value);
}

EGLBoolean EGLAPIENTRY eglPostSubBufferNV(EGLDisplay dpy, EGLSurface surface, EGLint x, EGLint y, EGLint width, EGLint height)
{
    return egl::PostSubBufferNV(dpy, surface, x, y, width, height);
}

EGLDisplay EGLAPIENTRY eglGetPlatformDisplayEXT(EGLenum platform, void *native_display, const EGLint *attrib_list)
{
    return egl::GetPlatformDisplayEXT(platform, native_display, attrib_list);
}

__eglMustCastToProperFunctionPointerType __stdcall eglGetProcAddress(const char *procname)
{
    struct Extension
    {
        const char *name;
        __eglMustCastToProperFunctionPointerType address;
    };

    static const Extension extensions[] =
    {
        { "eglQuerySurfacePointerANGLE", (__eglMustCastToProperFunctionPointerType)egl::QuerySurfacePointerANGLE },
        { "eglPostSubBufferNV", (__eglMustCastToProperFunctionPointerType)egl::PostSubBufferNV },
        { "eglGetPlatformDisplayEXT", (__eglMustCastToProperFunctionPointerType)egl::GetPlatformDisplayEXT },
        { "glBlitFramebufferANGLE", (__eglMustCastToProperFunctionPointerType)gl::BlitFramebufferANGLE },
        { "glRenderbufferStorageMultisampleANGLE", (__eglMustCastToProperFunctionPointerType)gl::RenderbufferStorageMultisampleANGLE },
        { "glDeleteFencesNV", (__eglMustCastToProperFunctionPointerType)gl::DeleteFencesNV },
        { "glGenFencesNV", (__eglMustCastToProperFunctionPointerType)gl::GenFencesNV },
        { "glIsFenceNV", (__eglMustCastToProperFunctionPointerType)gl::IsFenceNV },
        { "glTestFenceNV", (__eglMustCastToProperFunctionPointerType)gl::TestFenceNV },
        { "glGetFenceivNV", (__eglMustCastToProperFunctionPointerType)gl::GetFenceivNV },
        { "glFinishFenceNV", (__eglMustCastToProperFunctionPointerType)gl::FinishFenceNV },
        { "glSetFenceNV", (__eglMustCastToProperFunctionPointerType)gl::SetFenceNV },
        { "glGetTranslatedShaderSourceANGLE", (__eglMustCastToProperFunctionPointerType)gl::GetTranslatedShaderSourceANGLE },
        { "glTexStorage2DEXT", (__eglMustCastToProperFunctionPointerType)gl::TexStorage2DEXT },
        { "glGetGraphicsResetStatusEXT", (__eglMustCastToProperFunctionPointerType)gl::GetGraphicsResetStatusEXT },
        { "glReadnPixelsEXT", (__eglMustCastToProperFunctionPointerType)gl::ReadnPixelsEXT },
        { "glGetnUniformfvEXT", (__eglMustCastToProperFunctionPointerType)gl::GetnUniformfvEXT },
        { "glGetnUniformivEXT", (__eglMustCastToProperFunctionPointerType)gl::GetnUniformivEXT },
        { "glGenQueriesEXT", (__eglMustCastToProperFunctionPointerType)gl::GenQueriesEXT },
        { "glDeleteQueriesEXT", (__eglMustCastToProperFunctionPointerType)gl::DeleteQueriesEXT },
        { "glIsQueryEXT", (__eglMustCastToProperFunctionPointerType)gl::IsQueryEXT },
        { "glBeginQueryEXT", (__eglMustCastToProperFunctionPointerType)gl::BeginQueryEXT },
        { "glEndQueryEXT", (__eglMustCastToProperFunctionPointerType)gl::EndQueryEXT },
        { "glGetQueryivEXT", (__eglMustCastToProperFunctionPointerType)gl::GetQueryivEXT },
        { "glGetQueryObjectuivEXT", (__eglMustCastToProperFunctionPointerType)gl::GetQueryObjectuivEXT },
        { "glDrawBuffersEXT", (__eglMustCastToProperFunctionPointerType)gl::DrawBuffersEXT },
        { "glVertexAttribDivisorANGLE", (__eglMustCastToProperFunctionPointerType)gl::VertexAttribDivisorANGLE },
        { "glDrawArraysInstancedANGLE", (__eglMustCastToProperFunctionPointerType)gl::DrawArraysInstancedANGLE },
        { "glDrawElementsInstancedANGLE", (__eglMustCastToProperFunctionPointerType)gl::DrawElementsInstancedANGLE },
        { "glGetProgramBinaryOES", (__eglMustCastToProperFunctionPointerType)gl::GetProgramBinaryOES },
        { "glProgramBinaryOES", (__eglMustCastToProperFunctionPointerType)gl::ProgramBinaryOES },
        { "glGetBufferPointervOES", (__eglMustCastToProperFunctionPointerType)gl::GetBufferPointervOES },
        { "glMapBufferOES", (__eglMustCastToProperFunctionPointerType)gl::MapBufferOES },
        { "glUnmapBufferOES", (__eglMustCastToProperFunctionPointerType)gl::UnmapBufferOES },
        { "glMapBufferRangeEXT", (__eglMustCastToProperFunctionPointerType)gl::MapBufferRangeEXT },
        { "glFlushMappedBufferRangeEXT", (__eglMustCastToProperFunctionPointerType)gl::FlushMappedBufferRangeEXT },
        { "", NULL },
    };

    for (const Extension *extension = &extensions[0]; extension->address != nullptr; extension++)
    {
        if (strcmp(procname, extension->name) == 0)
        {
            return reinterpret_cast<__eglMustCastToProperFunctionPointerType>(extension->address);
        }
    }

    return NULL;
}

}
