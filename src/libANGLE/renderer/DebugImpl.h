//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DebugImpl.h: Defines the DebugImpl class for handling debug events and markers

#include <string>

namespace rx
{

class DebugImpl
{
  public:
    virtual void beginEvent(const std::string &msg) = 0;
    virtual void endEvent() = 0;

    virtual void setMarker(const std::string &msg) = 0;
};

}
