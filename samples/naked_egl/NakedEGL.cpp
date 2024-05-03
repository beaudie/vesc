#include "common/string_utils.h"
#include "common/system_utils.h"
#include "util/EGLWindow.h"
#include "util/OSWindow.h"
#include "util/egl_loader_autogen.h"
#include "util/gles_loader_autogen.h"
#include "util/shader_utils.h"

bool load()
{
    auto egl = angle::OpenSharedLibrary(ANGLE_EGL_LIBRARY_NAME, angle::SearchType::ModuleDir);
    INFO() << "Loaded EGL: " << egl;
    PFNEGLGETPROCADDRESSPROC getProcAddress;
    egl->getAs("eglGetProcAddress", &getProcAddress);
    if (!getProcAddress)
    {
        fprintf(stderr, "Cannot load eglGetProcAddress\n");
        return false;
    }
    INFO() << "eglGetProcAddress() found";
    LoadUtilEGL(getProcAddress);
    INFO() << "EGL Loaded";

    LoadUtilGLES(eglGetProcAddress);
    INFO() << "GLES Loaded";

    return true;
}

EGLDisplay egl_display;

bool initialize()
{

    egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    INFO() << "EGL Display: " << egl_display;

    EGLint major, minor;
    if (!eglInitialize(egl_display, &major, &minor))
    {
        INFO() << "Unable to initialize EGL: " << eglGetError();
        return false;
    }

    INFO() << "EGL Version: " << major << "." << minor << " ("
           << eglQueryString(egl_display, EGL_VENDOR) << ")";

    const EGLint configAttribs[] = {EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
                                    EGL_BLUE_SIZE,    8,
                                    EGL_GREEN_SIZE,   8,
                                    EGL_RED_SIZE,     8,
                                    EGL_ALPHA_SIZE,   8,
                                    EGL_DEPTH_SIZE,   24,
                                    EGL_STENCIL_SIZE, 8,
                                    EGL_NONE};
    EGLint numConfigs;
    EGLConfig config;
    bool gotConfig = eglChooseConfig(egl_display, configAttribs, &config, 1, &numConfigs);
    if (!gotConfig || numConfigs == 0)
    {
        INFO() << "Failed to choose config (eglError: " << std::hex << eglGetError() << ")";
        return false;
    }
    if (!eglBindAPI(EGL_OPENGL_ES_API))
    {
        INFO() << "eglBindAPI() failed!";
        return false;
    }

    const EGLint pbufferAttribs[] = {
        EGL_WIDTH, 512, EGL_HEIGHT, 512, EGL_NONE,
    };
    auto egl_surface = eglCreatePbufferSurface(egl_display, config, pbufferAttribs);
    if (egl_surface == EGL_NO_SURFACE)
    {
        INFO() << "Unable to create EGL surface (eglError: " << eglGetError() << ")";
        return false;
    }

    std::vector<EGLint> ctxattr = {
        EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 1, EGL_NONE,
    };
    auto egl_context = eglCreateContext(egl_display, config, EGL_NO_CONTEXT, ctxattr.data());
    if (egl_context == EGL_NO_CONTEXT)
    {
        INFO() << "Unable to create EGL context (eglError: " << eglGetError() << ")";
        return false;
    }

    eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
    return true;
}

int main(int argc, char **argv)
{
    bool loaded = load();
    INFO() << "ANGLE loaded: " << loaded;

    bool initialized = initialize();
    INFO() << "ANGLE initialized: " << initialized;

    const char *vertSrc = R"(#version 300 es
precision highp float;
in vec4 position;
in vec4 vertex_color;
smooth out vec4 interp_color;

void main()
{
    gl_Position = position;
    interp_color = vertex_color;
}
)";
    const char *fragSrc = R"(#version 300 es
precision highp float;
smooth in vec4 interp_color;
out vec4 fragColor;

void main()
{
    fragColor = interp_color;
}
)";

    auto program = CompileProgram(vertSrc, fragSrc);
    INFO() << "Compiled program: " << program;

    // If I uncomment the following line, the program no longer crashes
    //    glDeleteProgram(program);

    eglTerminate(egl_display);

    return 0;
}
