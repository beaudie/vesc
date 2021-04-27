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

template <typename T>
bool constexpr isIcdCompatible()
{
    constexpr T object{};
    return static_cast<const void *>(&object.mDispatch) == static_cast<const void *>(&object);
}

class Object
{
  private:
    // This has to be the first member to be OpenCL ICD compatible
    const cl_icd_dispatch *const mDispatch;

  protected:
    const cl_icd_dispatch &getDispatch() { return *mDispatch; }

  public:
    constexpr Object(const cl_icd_dispatch *dispatch) : mDispatch(dispatch) {}
    ~Object() = default;

    template <typename T>
    friend bool constexpr isIcdCompatible();
};

}  // namespace cl

#endif  // LIBANGLE_CLCONTEXT_H_
