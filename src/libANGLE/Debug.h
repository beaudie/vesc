//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Debug.h: Defines the Debug class for handling debug events and markers

#include <string>

namespace rx
{
class DebugImpl;
}

namespace gl
{

class Debug final
{
  public:
    Debug(rx::DebugImpl *impl);
    ~Debug();

    void beginEvent(const std::string &msg);
    void endEvent();

    void setMarker(const std::string &msg);

  private:
    rx::DebugImpl *mImplementation;
};

class ScopedEvent final
{
  public:
    ScopedEvent(Debug *debug, const char *msg, ...);
    ~ScopedEvent();

  private:
    Debug *mDebug;
};

}
