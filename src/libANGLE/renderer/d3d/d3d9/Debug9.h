//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Debug9.h: Defines the Debug9 class for handling debug events and markers

#include "common/platform.h"
#include "libANGLE/renderer/DebugImpl.h"

namespace rx
{

class Debug9 : public DebugImpl
{
  public:
    Debug9();
    virtual ~Debug9();

    void beginEvent(const std::string &msg) override;
    void endEvent() override;

    void setMarker(const std::string &msg) override;
};

}
