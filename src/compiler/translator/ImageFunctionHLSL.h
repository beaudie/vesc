//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageFunctionHLSL: Class for writing implementations of ESSL image functions into HLSL
// output. Some of the implementations are straightforward and just call the HLSL equivalent of the
// ESSL image function, others do more work to emulate ESSL image load, store or size query
// behavior.
//

#ifndef COMPILER_TRANSLATOR_IMAGEFUNCTIONHLSL_H_
#define COMPILER_TRANSLATOR_IMAGEFUNCTIONHLSL_H_

#include <set>

#include "GLSLANG/ShaderLang.h"
#include "compiler/translator/BaseTypes.h"
#include "compiler/translator/Common.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/Types.h"

namespace sh
{

class ImageFunctionHLSL final : angle::NonCopyable
{
  public:
    struct ImageFunction
    {
        // See ESSL 3.10.4 section 8.12 for reference about what the different methods below do.
        enum class Method
        {
            SIZE,
            LOAD,
            STORE
        };

        TString name() const;

        bool operator<(const ImageFunction &rhs) const;

        const char *getReturnType() const;

        TBasicType image;
        TLayoutImageInternalFormat imageInternalFormat;
        bool readOnly;
        Method method;
    };

    // Returns the name of the image function implementation to caller.
    // The name that's passed in is the name of the GLSL image function that it should implement.
    TString useImageFunction(const TString &name,
                             const TBasicType &type,
                             TLayoutImageInternalFormat imageInternalFormat,
                             bool readOnly);

    void imageFunctionHeader(TInfoSinkBase &out);

  private:
    using ImageFunctionSet = std::set<ImageFunction>;
    ImageFunctionSet mUsesImage;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_IMAGEFUNCTIONHLSL_H_
