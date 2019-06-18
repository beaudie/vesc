//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// openGL32_wgl.cpp: Implements the exported WGL functions.

#include "openGL32_wgl.h"

#include "common/debug.h"
#include "common/event_tracer.h"
#include "common/utilities.h"
#include "common/version.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/EGLSync.h"
#include "libANGLE/Surface.h"
#include "libANGLE/Texture.h"
#include "libANGLE/Thread.h"
#include "libANGLE/queryutils.h"
#include "libANGLE/validationEGL.h"
#include "openGL32/global_state.h"
#include "openGL32/proc_table_wgl.h"

using namespace wgl;

namespace
{

bool CompareProc(const ProcEntry &a, const char *b)
{
    return strcmp(a.first, b) < 0;
}
}  // anonymous namespace

extern "C" {

// WGL 1.0
int GL_APIENTRY wglChoosePixelFormat(HDC hDc, const PIXELFORMATDESCRIPTOR *pPfd)
{
    UNIMPLEMENTED();
    return 0;
}

int GL_APIENTRY wglDescribePixelFormat(HDC hdc, int ipfd, UINT cjpfd, PIXELFORMATDESCRIPTOR *ppfd)
{
    UNIMPLEMENTED();
    return 0;
}

UINT GL_APIENTRY wglGetEnhMetaFilePixelFormat(HENHMETAFILE hemf,
                                              UINT cbBuffer,
                                              PIXELFORMATDESCRIPTOR *ppfd)
{
    UNIMPLEMENTED();
    return 0u;
}

int GL_APIENTRY wglGetPixelFormat(HDC hdc)
{
    UNIMPLEMENTED();
    return 0;
}

BOOL GL_APIENTRY wglSetPixelFormat(HDC hdc, int ipfd, const PIXELFORMATDESCRIPTOR *ppfd)
{
    UNIMPLEMENTED();
    return FALSE;
}

BOOL GL_APIENTRY wglSwapBuffers(HDC hdc)
{
    UNIMPLEMENTED();
    return FALSE;
}

BOOL GL_APIENTRY wglCopyContext(HGLRC hglrcSrc, HGLRC hglrcDst, UINT mask)
{
    UNIMPLEMENTED();
    return FALSE;
}

HGLRC GL_APIENTRY wglCreateContext(HDC hDc)
{
    UNIMPLEMENTED();
    return nullptr;
}

HGLRC GL_APIENTRY wglCreateLayerContext(HDC hDc, int level)
{
    UNIMPLEMENTED();
    return nullptr;
}

BOOL GL_APIENTRY wglDeleteContext(HGLRC oldContext)
{
    UNIMPLEMENTED();
    return FALSE;
}

BOOL GL_APIENTRY wglDescribeLayerPlane(HDC hDc,
                                       int pixelFormat,
                                       int layerPlane,
                                       UINT nBytes,
                                       LAYERPLANEDESCRIPTOR *plpd)
{
    UNIMPLEMENTED();
    return FALSE;
}

HGLRC GL_APIENTRY wglGetCurrentContext()
{
    UNIMPLEMENTED();
    return nullptr;
}

HDC GL_APIENTRY wglGetCurrentDC()
{
    UNIMPLEMENTED();
    return nullptr;
}

int GL_APIENTRY
wglGetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart, int cEntries, COLORREF *pcr)
{
    UNIMPLEMENTED();
    return 0;
}

PROC GL_APIENTRY wglGetProcAddress(LPCSTR lpszProc)
{
    printf("called getprocaddress");
    ANGLE_SCOPED_GLOBAL_LOCK();
    EVENT("(const char *procname = \"%s\")", lpszProc);
    egl::Thread *thread = egl::GetCurrentThread();

    ProcEntry *entry =
        std::lower_bound(&g_procTable[0], &g_procTable[g_numProcs], lpszProc, CompareProc);

    thread->setSuccess();

    if (entry == &g_procTable[g_numProcs] || strcmp(entry->first, lpszProc) != 0)
    {
        return nullptr;
    }

    return entry->second;
}

BOOL GL_APIENTRY wglMakeCurrent(HDC hDc, HGLRC newContext)
{
    UNIMPLEMENTED();
    return FALSE;
}

BOOL GL_APIENTRY wglRealizeLayerPalette(HDC hdc, int iLayerPlane, BOOL bRealize)
{
    UNIMPLEMENTED();
    return FALSE;
}

int GL_APIENTRY
wglSetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart, int cEntries, const COLORREF *pcr)
{
    UNIMPLEMENTED();
    return 0;
}

BOOL GL_APIENTRY wglShareLists(HGLRC hrcSrvShare, HGLRC hrcSrvSource)
{
    UNIMPLEMENTED();
    return FALSE;
}

BOOL GL_APIENTRY wglSwapLayerBuffers(HDC hdc, UINT fuFlags)
{
    UNIMPLEMENTED();
    return FALSE;
}

// BOOL GL_APIENTRY wglUseFontBitmaps(HDC hDC, DWORD first, DWORD count, DWORD listBase)
//{
//    UNIMPLEMENTED();
//    return FALSE;
//}

BOOL GL_APIENTRY wglUseFontBitmapsA(HDC hDC, DWORD first, DWORD count, DWORD listBase)
{
    UNIMPLEMENTED();
    return FALSE;
}

BOOL GL_APIENTRY wglUseFontBitmapsW(HDC hDC, DWORD first, DWORD count, DWORD listBase)
{
    UNIMPLEMENTED();
    return FALSE;
}

// BOOL GL_APIENTRY wglUseFontOutlines(HDC hDC,
//                                    DWORD first,
//                                    DWORD count,
//                                    DWORD listBase,
//                                    FLOAT deviation,
//                                    FLOAT extrusion,
//                                    int format,
//                                    LPGLYPHMETRICSFLOAT lpgmf)
//{
//    UNIMPLEMENTED();
//    return FALSE;
//}

BOOL GL_APIENTRY wglUseFontOutlinesA(HDC hDC,
                                     DWORD first,
                                     DWORD count,
                                     DWORD listBase,
                                     FLOAT deviation,
                                     FLOAT extrusion,
                                     int format,
                                     LPGLYPHMETRICSFLOAT lpgmf)
{
    UNIMPLEMENTED();
    return FALSE;
}

BOOL GL_APIENTRY wglUseFontOutlinesW(HDC hDC,
                                     DWORD first,
                                     DWORD count,
                                     DWORD listBase,
                                     FLOAT deviation,
                                     FLOAT extrusion,
                                     int format,
                                     LPGLYPHMETRICSFLOAT lpgmf)
{
    UNIMPLEMENTED();
    return FALSE;
}

}  // extern "C"
