#ifndef LIBGLESV2_RENDERER_D3D_D3DCOMPILER_H_
#define LIBGLESV2_RENDERER_D3D_D3DCOMPILER_H_

#include "common/angleutils.h"

#include <windows.h>

namespace gl
{
class InfoLog;
}

namespace rx
{

typedef void* ShaderBlob;
typedef void(*CompileFuncPtr)();

class D3DCompiler
{
  public:
    D3DCompiler();
    ~D3DCompiler();

    bool initialize();
    void release();

    ShaderBlob *compileToBinary(gl::InfoLog &infoLog, const char *hlsl, const char *profile,
                                unsigned int optimizationFlags, bool alternateFlags) const;

  private:
    DISALLOW_COPY_AND_ASSIGN(D3DCompiler);

    HMODULE mD3DCompilerModule;
    CompileFuncPtr mD3DCompileFunc;
};

}

#endif // LIBGLESV2_RENDERER_D3D_D3DCOMPILER_H_
