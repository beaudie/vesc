//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Debug11.h: Defines the Debug11 class for handling debug events and markers

#include "common/platform.h"
#include "libANGLE/renderer/DebugImpl.h"

namespace rx
{

class Debug11 : public DebugImpl
{
  public:
    Debug11(ID3DUserDefinedAnnotation *userDefinedAnnotation);
    virtual ~Debug11();

    void beginEvent(const std::string &msg) override;
    void endEvent() override;

    void setMarker(const std::string &msg) override;

  private:
    ID3DUserDefinedAnnotation *mUserDefinedAnnotation;
};

}
