//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DrawElementsPerfParams.h:
//   Parametrization for performance tests for ANGLE DrawElements call overhead.
//

#ifndef TESTS_PERF_TESTS_DRAW_ELEMENTS_PERF_PARAMS_H_
#define TESTS_PERF_TESTS_DRAW_ELEMENTS_PERF_PARAMS_H_

#include "DrawCallPerfParams.h"

#include <ostream>

#include "ANGLEPerfTest.h"

struct DrawElementsPerfParams final : public DrawCallPerfParams
{
    std::string suffix() const override;

    double runTimeSeconds   = 5.0;
    int numTris             = 2;
    GLenum type             = GL_UNSIGNED_INT;
    bool indexBufferChanged = false;
};

std::ostream &operator<<(std::ostream &os, const DrawElementsPerfParams &params);

DrawElementsPerfParams DrawElementsPerfWithChangedIndexBufferD3D11Params(bool useNullDevice,
                                                                         bool renderToTexture);

DrawElementsPerfParams DrawElementsPerfD3D11Params(bool useNullDevice, bool renderToTexture);

DrawElementsPerfParams DrawElementsPerfD3D9Params(bool useNullDevice, bool renderToTexture);

DrawElementsPerfParams DrawElementsPerfOpenGLParams(bool useNullDevice, bool renderToTexture);

DrawElementsPerfParams DrawElementsPerfValidationOnly();

#endif  // TESTS_PERF_TESTS_DRAW_ELEMENTS_PERF_PARAMS_H_
