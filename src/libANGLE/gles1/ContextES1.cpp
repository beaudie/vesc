#include "libANGLE/gles1/ContextES1.h"

namespace gl {

static egl::AttributeMap getES1EngineAttribs(const egl::AttributeMap& attribs) {
    egl::AttributeMap res(attribs);
    res.insert(EGL_CONTEXT_CLIENT_VERSION, 3);
    res.insert(EGL_CONTEXT_MINOR_VERSION, 0);
    return res;
}

ContextES1::ContextES1(rx::EGLImplFactory *implFactory,
                       const egl::Config *config,
                       const Context *shareContext,
                       TextureManager *shareTextures,
                       MemoryProgramCache *memoryProgramCache,
                       const egl::AttributeMap &attribs,
                       const egl::DisplayExtensions &displayExtensions)
    : Context(implFactory, config, shareContext, shareTextures, memoryProgramCache,
              getES1EngineAttribs(attribs), displayExtensions) { }

// TODO: GLES1 versions of enable/disable, queries, texture unit funcs

} // namespace gl
