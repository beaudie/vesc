//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CLObject.h: Defines the cl::Object class, which is the base class of all ANGLE CL objects.

#ifndef LIBANGLE_CLOBJECT_H_
#define LIBANGLE_CLOBJECT_H_

#include "libANGLE/CLtypes.h"

struct _cl_icd_dispatch;
using cl_icd_dispatch = _cl_icd_dispatch;

namespace cl
{

class Object
{
  public:
    constexpr Object(const cl_icd_dispatch &dispatch) : mDispatch(&dispatch) {}
    ~Object() = default;

  protected:
    const cl_icd_dispatch &getDispatch() { return *mDispatch; }

  private:
    // This has to be the first member to be OpenCL ICD compatible
    const cl_icd_dispatch *const mDispatch;
};

}  // namespace cl

#endif  // LIBANGLE_CLCONTEXT_H_
