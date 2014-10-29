//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Data.h: Container class for all GL relevant state, caps and objects

#ifndef LIBGLESV2_DATA_H_
#define LIBGLESV2_DATA_H_

#include "libGLESv2/State.h"

namespace gl
{

typedef std::map< GLenum, BindingPointer<Texture> > TextureMap;

struct Data final
{
  public:
    Data();
    ~Data();

    Texture *getSamplerTexture(unsigned int sampler, GLenum type) const;

    GLint clientVersion;
    State state;
    Caps caps;
    TextureCapsMap textureCaps;
    Extensions extensions;
    ResourceManager *resourceManager;

    TextureMap zeroTextures;

  private:
    DISALLOW_COPY_AND_ASSIGN(Data);
};

}

#endif // LIBGLESV2_DATA_H_
