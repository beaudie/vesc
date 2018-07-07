//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramImpl.cpp: Defines the abstract rx::ProgramImpl classes.

#include "libANGLE/renderer/ProgramImpl.h"

namespace rx
{

class LinkEventSync : public gl::LinkEvent
{
  public:
    LinkEventSync(const gl::LinkResult &result) : mResult(result) {}
    gl::LinkResult wait() override { return mResult; }

  private:
    gl::LinkResult mResult;
};

gl::LinkEvent *ProgramImpl::asyncLink(const gl::Context *context,
                                      const gl::ProgramLinkedResources &resources,
                                      gl::InfoLog &infoLog)
{
    return new LinkEventSync(link(context, resources, infoLog));
}

void ProgramImpl::didLink(const gl::Context *context,
                          const gl::ProgramLinkedResources &resources,
                          gl::InfoLog &infoLog)
{
}

}  // namespace rx
