//
// Copyright (c) 2002-2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TEXTUREFUNCTIONHLSL_H_
#define COMPILER_TRANSLATOR_TEXTUREFUNCTIOnHLSL_H_

#include <set>

#include "compiler/translator/BaseTypes.h"
#include "compiler/translator/Common.h"
#include "compiler/translator/InfoSink.h"
#include "GLSLANG/ShaderLang.h"

namespace sh
{

class TextureFunctionHLSL : angle::NonCopyable
{
  public:
    struct TextureFunction
    {
        enum Method
        {
            IMPLICIT,  // Mipmap LOD determined implicitly (standard lookup)
            BIAS,
            LOD,
            LOD0,
            LOD0BIAS,
            SIZE,  // textureSize()
            FETCH,
            GRAD
        };

        TBasicType sampler;
        int coords;
        bool proj;
        bool offset;
        Method method;

        TString name() const;

        bool operator<(const TextureFunction &rhs) const;

        const char *getReturnType() const;
    };

    // Returns the texture function
    const TextureFunction &useTextureFunction(const TString &name,
                                              TBasicType samplerType,
                                              int coords,
                                              unsigned int argumentCount,
                                              bool lod0,
                                              sh::GLenum shaderType);

    void textureFunctionHeader(TInfoSinkBase &out, const ShShaderOutput outputType);

  private:
    typedef std::set<TextureFunction> TextureFunctionSet;
    TextureFunctionSet mUsesTexture;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TEXTUREFUNCTIONHLSL_H_
