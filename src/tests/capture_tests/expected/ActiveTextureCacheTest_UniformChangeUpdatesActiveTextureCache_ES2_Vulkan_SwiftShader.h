#pragma once

#include <EGL/egl.h>
#include <stdint.h>

extern EGLDisplay gEGLDisplay;

// Public functions are declared in trace_fixture.h.

// Private Functions

void ReplayFrame1(void);
void ReplayFrame2(void);
void InitReplay(void);

// Global variables

extern const char *const glShaderSource_string_0[];
extern const char *const glShaderSource_string_1[];
