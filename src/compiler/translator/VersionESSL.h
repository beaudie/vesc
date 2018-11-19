//
// Copyright (c) 2002-2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_VERSIONESSL_H_
#define COMPILER_TRANSLATOR_VERSIONESSL_H_

#include "compiler/translator/tree_util/IntermTraverse.h"

#include "compiler/translator/Pragma.h"

namespace sh
{
static const int ESSL_VERSION_100 = 100;
static const int ESSL_VERSION_300 = 300;
static const int ESSL_VERSION_310 = 310;

int ShaderOutputTypeToESSLVersion(ShShaderOutput output);

class TVersionESSL : public TIntermTraverser
{
  public:
    TVersionESSL(sh::GLenum type, const TPragma &pragma, ShShaderOutput output);

    // If output is core profile, returns 150.
    // If output is legacy profile,
    //   Returns 120 if the following is used the shader:
    //   - "invariant",
    //   - "gl_PointCoord",
    //   - matrix/matrix constructors
    //   - array "out" parameters
    //   Else 110 is returned.
    int getVersion() const { return mVersion; }

  private:
    void ensureVersionIsAtLeast(int version);

    int mVersion;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_VERSIONELSL_H_
