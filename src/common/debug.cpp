//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// debug.cpp: Debugging utilities.

#include "common/debug.h"
#include "common/platform.h"

#include <stdarg.h>
#include <vector>
#include <fstream>
#include <cstdio>

namespace gl
{
#if defined(ANGLE_ENABLE_PERF)
typedef void (WINAPI *PerfOutputFunction)(D3DCOLOR, LPCWSTR);
#else
typedef void (*PerfOutputFunction)(unsigned int, const wchar_t*);
#endif

static void output(bool traceFileDebugOnly, PerfOutputFunction perfFunc, const char *format, va_list vararg)
{
#if defined(ANGLE_ENABLE_PERF) || defined(ANGLE_ENABLE_TRACE)
    static std::vector<char> asciiMessageBuffer(512);

    // Attempt to just print to the current buffer
    int len = vsnprintf(&asciiMessageBuffer[0], asciiMessageBuffer.size(), format, vararg);
    if (len < 0 || static_cast<size_t>(len) >= asciiMessageBuffer.size())
    {
        // Buffer was not large enough, calculate the required size and resize the buffer
        len = vsnprintf(NULL, 0, format, vararg);
        asciiMessageBuffer.resize(len + 1);

        // Print again
        vsnprintf(&asciiMessageBuffer[0], asciiMessageBuffer.size(), format, vararg);
    }

    // NULL terminate the buffer to be safe
    asciiMessageBuffer[len] = '\0';
#endif

#if defined(ANGLE_ENABLE_PERF)
    if (perfActive())
    {
        // The perf function only accepts wide strings, widen the ascii message
        static std::wstring wideMessage;
        if (wideMessage.capacity() < asciiMessageBuffer.size())
        {
            wideMessage.reserve(asciiMessageBuffer.size());
        }

        wideMessage.assign(asciiMessageBuffer.begin(), asciiMessageBuffer.begin() + len);

        perfFunc(0, wideMessage.c_str());
    }
#endif // ANGLE_ENABLE_PERF

#if defined(ANGLE_ENABLE_TRACE)
#if defined(NDEBUG)
    if (traceFileDebugOnly)
    {
        return;
    }
#endif // NDEBUG

    static std::ofstream file(TRACE_OUTPUT_FILE, std::ofstream::app);
    if (file)
    {
        file.write(&asciiMessageBuffer[0], len);
        file.flush();
    }

#endif // ANGLE_ENABLE_TRACE
}

void trace(bool traceFileDebugOnly, const char *format, ...)
{
    va_list vararg;
    va_start(vararg, format);
#if defined(ANGLE_ENABLE_PERF)
    output(traceFileDebugOnly, D3DPERF_SetMarker, format, vararg);
#else
    output(traceFileDebugOnly, NULL, format, vararg);
#endif
    va_end(vararg);
}

bool perfActive()
{
#if defined(ANGLE_ENABLE_PERF)
    static bool active = D3DPERF_GetStatus() != 0;
    return active;
#else
    return false;
#endif
}

ScopedPerfEventHelper::ScopedPerfEventHelper(const char* format, ...)
{
#if defined(ANGLE_ENABLE_PERF)
#if !defined(ANGLE_ENABLE_TRACE)
    if (!perfActive())
    {
        return;
    }
#endif // !ANGLE_ENABLE_TRACE
    va_list vararg;
    va_start(vararg, format);
    output(true, reinterpret_cast<PerfOutputFunction>(D3DPERF_BeginEvent), format, vararg);
    va_end(vararg);
#endif // ANGLE_ENABLE_PERF
}

ScopedPerfEventHelper::~ScopedPerfEventHelper()
{
#if defined(ANGLE_ENABLE_PERF)
    if (perfActive())
    {
        D3DPERF_EndEvent();
    }
#endif
}

#if _DEBUG

#include <Windows.h>

class DialogTemplate
{
  public:
    LPCDLGTEMPLATE getTemplate()
    {
        return (LPCDLGTEMPLATE)&v[0];
    }

    void alignToDword()
    {
        if (v.size() % 4) write(NULL, 4 - (v.size() % 4));
    }

    void write(LPCVOID pvWrite, DWORD cbWrite)
    {
        v.insert(v.end(), cbWrite, 0);
        if (pvWrite) CopyMemory(&v[v.size() - cbWrite], pvWrite, cbWrite);
    }

    template<typename T> void write(T t)
    {
        write(&t, sizeof(T));
    }

    void writeString(LPCWSTR psz)
    {
        write(psz, (lstrlenW(psz) + 1) * sizeof(WCHAR));
    }

  private:
    std::vector<BYTE> v;
};

INT_PTR CALLBACK DebuggerWaitDialogProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
{
    switch (wm)
    {
      case WM_INITDIALOG: SetTimer(hwnd, 1, 100, NULL); return TRUE;
      case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL) EndDialog(hwnd, 0);
        break;
      case WM_TIMER: if (IsDebuggerPresent()) EndDialog(hwnd, 0);
    }
    return FALSE;
}

BOOL DebuggerWaitMessageBox(HWND hwnd, LPCWSTR pszMessage, LPCWSTR pszTitle)
{
    BOOL fSuccess = FALSE;
    HDC hdc = GetDC(NULL);
    if (hdc)
    {
        NONCLIENTMETRICSW ncm = { sizeof(ncm) };
        if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0))
        {
            DialogTemplate tmp;

            // Write out the extended dialog template header
            tmp.write<WORD>(1); // dialog version
            tmp.write<WORD>(0xFFFF); // extended dialog template
            tmp.write<DWORD>(0); // help ID
            tmp.write<DWORD>(0); // extended style
            tmp.write<DWORD>(WS_CAPTION | WS_SYSMENU | DS_SETFONT | DS_MODALFRAME);
            tmp.write<WORD>(2); // number of controls
            tmp.write<WORD>(32); // X
            tmp.write<WORD>(32); // Y
            tmp.write<WORD>(100); // width
            tmp.write<WORD>(40); // height
            tmp.writeString(L""); // no menu
            tmp.writeString(L""); // default dialog class
            tmp.writeString(pszTitle); // title

            // Next comes the font description.
            // See text for discussion of fancy formula.
            if (ncm.lfMessageFont.lfHeight < 0)
            {
                ncm.lfMessageFont.lfHeight = -MulDiv(ncm.lfMessageFont.lfHeight, 72, GetDeviceCaps(hdc, LOGPIXELSY));
            }
            tmp.write<WORD>((WORD)ncm.lfMessageFont.lfHeight); // point
            tmp.write<WORD>((WORD)ncm.lfMessageFont.lfWeight); // weight
            tmp.write<BYTE>(ncm.lfMessageFont.lfItalic); // Italic
            tmp.write<BYTE>(ncm.lfMessageFont.lfCharSet); // CharSet
            tmp.writeString(ncm.lfMessageFont.lfFaceName);

            // Then come the two controls.  First is the static text.
            tmp.alignToDword();
            tmp.write<DWORD>(0); // help id
            tmp.write<DWORD>(0); // window extended style
            tmp.write<DWORD>(WS_CHILD | WS_VISIBLE); // style
            tmp.write<WORD>(7); // x
            tmp.write<WORD>(7); // y
            tmp.write<WORD>(200-14); // width
            tmp.write<WORD>(80-7-14-7); // height
            tmp.write<DWORD>(-1); // control ID
            tmp.write<DWORD>(0x0082FFFF); // static
            tmp.writeString(pszMessage); // text
            tmp.write<WORD>(0); // no extra data

            // Second control is the OK button.
            tmp.alignToDword();
            tmp.write<DWORD>(0); // help id
            tmp.write<DWORD>(0); // window extended style
            tmp.write<DWORD>(WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_DEFPUSHBUTTON); // style
            tmp.write<WORD>(75); // x
            tmp.write<WORD>(80-7-14); // y
            tmp.write<WORD>(50); // width
            tmp.write<WORD>(14); // height
            tmp.write<DWORD>(IDCANCEL); // control ID
            tmp.write<DWORD>(0x0080FFFF); // static
            tmp.writeString(L"OK"); // text
            tmp.write<WORD>(0); // no extra data

            // Template is ready - go display it.
            fSuccess = DialogBoxIndirect(GetModuleHandle(NULL), tmp.getTemplate(), hwnd, DebuggerWaitDialogProc) >= 0;
        }
        ReleaseDC(NULL, hdc);
    }
    return fSuccess;
}

void WaitForDebugger()
{
    if (!IsDebuggerPresent())
    {
        DebuggerWaitMessageBox(NULL, L"Waiting for debugger...", L"ANGLE Debugger Attach Dialog");
    }
}

#else
void WaitForDebugger() {}
#endif // _DEBUG

}
